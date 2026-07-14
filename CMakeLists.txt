cmake_minimum_required(VERSION 3.20)

project(MuxyGameLink VERSION 0.2.0 LANGUAGES C CXX)

option(GAMELINK_BUILD_TESTS "Build the GameLink unit tests" ON)
option(GAMELINK_BUILD_EXAMPLES "Compile the GameLink examples" ON)
option(GAMELINK_BUILD_SHARED "Build the C ABI shared library" ON)
option(BUILD_INTEGRATION_TESTS "Compile network integration tests" OFF)
option(GENERATE_COVERAGE "Enable Clang source coverage for tests" OFF)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

add_compile_definitions(FMT_HEADER_ONLY=1 JSON_NOEXCEPTION _HAS_EXCEPTIONS=0)
if(MSVC)
    add_compile_options(/W4 /EHs-c- /GR-)
else()
    add_compile_options(-Wall -fno-exceptions -Wno-unknown-pragmas -Wno-unused-variable)
endif()

file(GLOB SCHEMA_SOURCES CONFIGURE_DEPENDS "schema/*.cpp")
file(GLOB SDK_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB C_SOURCES CONFIGURE_DEPENDS "c_library/*.cpp")
file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "test/*.cpp")
file(GLOB WEBSOCKET_SOURCES CONFIGURE_DEPENDS "websocket_network/*.cpp")
file(GLOB SCHEMA_PUBLIC_HEADERS CONFIGURE_DEPENDS "schema/*.h")
file(GLOB SDK_PUBLIC_HEADERS CONFIGURE_DEPENDS "include/*.h")
set(PUBLIC_HEADERS ${SCHEMA_PUBLIC_HEADERS} ${SDK_PUBLIC_HEADERS})

set(GAMELINK_INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party"
)

if(GAMELINK_BUILD_TESTS)
    enable_testing()
    add_executable(gamelink_tests ${SCHEMA_SOURCES} ${TEST_SOURCES} ${SDK_SOURCES} ${C_SOURCES})
    target_include_directories(gamelink_tests PRIVATE ${GAMELINK_INCLUDE_DIRS} test c_library)
    target_compile_definitions(gamelink_tests PRIVATE
        MUXY_GAMELINK_CUSTOM_STRING_TYPE=ConstrainedString
        MUXY_GAMELINK_CUSTOM_LOCK_TYPE=ConstrainedLock
        MUXY_GAMELINK_CUSTOM_STRING_INCLUDE="constrained_types.h"
    )
    target_link_libraries(gamelink_tests PRIVATE Threads::Threads)

    if(BUILD_INTEGRATION_TESTS)
        find_package(CURL REQUIRED)
        target_sources(gamelink_tests PRIVATE ${WEBSOCKET_SOURCES})
        target_compile_definitions(gamelink_tests PRIVATE MUXY_GAMELINK_ENABLE_INTEGRATION_TESTS=1)
        target_include_directories(gamelink_tests PRIVATE websocket_network)
        target_link_libraries(gamelink_tests PRIVATE CURL::libcurl)
    endif()

    if(GENERATE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(gamelink_tests PRIVATE -fprofile-instr-generate -fcoverage-mapping)
        target_link_options(gamelink_tests PRIVATE -fprofile-instr-generate)
    endif()

    add_test(NAME gamelink-tests COMMAND gamelink_tests)
endif()

set(AMALGAM_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/amalgamate.sh")
set(AMALGAM_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(AMALGAM_HEADER "${AMALGAM_GENERATED_DIR}/gamelink_single.hpp")
set(AMALGAM_SOURCES ${SCHEMA_SOURCES} ${SDK_SOURCES})
string(REPLACE ";" " " AMALGAM_PUBLIC_SOURCES "${AMALGAM_SOURCES}")
configure_file(amalgamate.in "${AMALGAM_SCRIPT}" @ONLY)

add_custom_command(
    OUTPUT "${AMALGAM_HEADER}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${AMALGAM_GENERATED_DIR}"
    COMMAND bash "${AMALGAM_SCRIPT}" "${AMALGAM_HEADER}"
    DEPENDS amalgamate.in ${PUBLIC_HEADERS} ${AMALGAM_SOURCES} include/config.h include/config_footer.h third_party/nlohmann/json.hpp
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    VERBATIM
)
add_custom_target(amalgam ALL DEPENDS "${AMALGAM_HEADER}")

add_executable(amalgam_compile_check amalgam_check.cpp)
add_dependencies(amalgam_compile_check amalgam)
target_include_directories(amalgam_compile_check PRIVATE "${AMALGAM_GENERATED_DIR}")
target_link_libraries(amalgam_compile_check PRIVATE Threads::Threads)

add_custom_target(amalgam_drift
    COMMAND "${CMAKE_COMMAND}" -E compare_files "${AMALGAM_HEADER}" "${CMAKE_CURRENT_SOURCE_DIR}/gamelink_single.hpp"
    DEPENDS amalgam
    COMMENT "Checking committed amalgamated header"
)

if(GAMELINK_BUILD_SHARED)
    add_library(cgamelink SHARED ${SDK_SOURCES} ${SCHEMA_SOURCES} ${C_SOURCES})
    target_include_directories(cgamelink PUBLIC ${GAMELINK_INCLUDE_DIRS} c_library)
    target_compile_definitions(cgamelink PRIVATE MUXY_CLIB_EXPORT)
    target_link_libraries(cgamelink PRIVATE Threads::Threads)

    add_executable(cgamelink_compile_check clibrary_check.cpp)
    target_include_directories(cgamelink_compile_check PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(cgamelink_compile_check PRIVATE cgamelink)
endif()

if(GAMELINK_BUILD_EXAMPLES)
    find_package(CURL REQUIRED)
    add_subdirectory(examples)
endif()

find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    set(DOXYGEN_INDEX_FILE "${CMAKE_CURRENT_BINARY_DIR}/doxygen/html/index.html")
    set(DOXYFILE_OUT "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")
    set(DOXYGEN_STRIP_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
    set(DOXYGEN_INPUTS ${PUBLIC_HEADERS})
    set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/doxygen")
    configure_file(doxygen/Doxyfile.in "${DOXYFILE_OUT}" @ONLY)
    add_custom_command(
        OUTPUT "${DOXYGEN_INDEX_FILE}"
        COMMAND "${DOXYGEN_EXECUTABLE}" "${DOXYFILE_OUT}"
        DEPENDS ${PUBLIC_HEADERS} "${DOXYFILE_OUT}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        VERBATIM
    )
    add_custom_target(doxygen DEPENDS "${DOXYGEN_INDEX_FILE}")
endif()
