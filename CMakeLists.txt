cmake_minimum_required(VERSION 3.7)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Project
project(Gamelink)

# Global settings. Note we're compiling without exceptions to show that the library
# works without exception support.

add_compile_definitions("FMT_HEADER_ONLY=1" "JSON_NOEXCEPTION" "_HAS_EXCEPTIONS=0")
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "/EHs-c- /GR-")
else()
    set(CMAKE_CXX_FLAGS "-Wall -fno-exceptions")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
endif()

include_directories(.)
include_directories(include)
include_directories(third_party)

## Grab all the source files
file(GLOB SCHEMA_SOURCES "schema/*.cpp")
file(GLOB SDK_TEST_SOURCES "test/*.cpp")
file(GLOB SDK_SOURCES "src/*.cpp")
file(GLOB C_SOURCES "c_library/*.cpp")

## Unit tests
add_executable(tests ${SCHEMA_SOURCES} ${SDK_TEST_SOURCES} ${SDK_SOURCES} ${C_SOURCES})
target_include_directories(tests PRIVATE "test" "c_library")
target_compile_definitions(tests PRIVATE "MUXY_GAMELINK_CUSTOM_STRING_TYPE=ConstrainedString" "MUXY_GAMELINK_CUSTOM_LOCK_TYPE=ConstrainedLock" "MUXY_GAMELINK_CUSTOM_STRING_INCLUDE=\"constrained_types.h\"")

# Amalgamation
set(AMALGAM_IN "${CMAKE_CURRENT_SOURCE_DIR}/amalgamate.in")
set(AMALGAM_OUT "${CMAKE_CURRENT_SOURCE_DIR}/amalgamate.sh")

set(AMALGAM_SOURCES ${SCHEMA_SOURCES} ${SDK_SOURCES})

string(REPLACE ";" " " AMALGAM_PUBLIC_HEADERS "${PUBLIC_HEADERS}")
string(REPLACE ";" " " AMALGAM_PUBLIC_SOURCES "${AMALGAM_SOURCES}")

configure_file(${AMALGAM_IN} ${AMALGAM_OUT} @ONLY)
add_custom_target(amalgam ALL 
    COMMAND 
    ${AMALGAM_OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating Amalgam file")

# Amalgamation compilation checked
add_executable(amalgam_compile_check "amalgam_check.cpp")
add_dependencies(amalgam_compile_check amalgam)

# Shared library 
add_library(cgamelink SHARED ${SDK_SOURCES} ${SCHEMA_SOURCES} ${C_SOURCES})
target_compile_definitions(cgamelink PRIVATE "MUXY_CLIB_EXPORT")

add_executable(cgamelink_compile_check "clibrary_check.cpp")
target_link_libraries(cgamelink_compile_check cgamelink)
add_dependencies(cgamelink_compile_check cgamelink)

## Examples
find_package(PkgConfig)
pkg_check_modules(LIB_WEBSOCKETS libwebsockets>=4.0.0)

if(LIB_WEBSOCKETS_INCLUDE_DIRS)
    add_subdirectory("examples/01-authenticate")
    add_subdirectory("examples/02-voting")
    add_subdirectory("examples/03-state")
    add_subdirectory("examples/04-broadcast")

    add_custom_target(examples ALL)
    add_dependencies(authenticate amalgam)
    add_dependencies(voting amalgam)
    add_dependencies(state amalgam)
    add_dependencies(broadcast amalgam)
    add_dependencies(examples authenticate voting state broadcast)
endif()

# Documentation
find_package(Doxygen)

file(GLOB SCHEMA_PUBLIC_HEADERS "schema/*.h")
file(GLOB SRC_PUBLIC_HEADERS "include/*.h")

set(PUBLIC_HEADERS ${SCHEMA_PUBLIC_HEADERS} ${SRC_PUBLIC_HEADERS})
set(DOXYGEN_INDEX_FILE "doxygen/html/index.html")
set(DOXYFILE_IN "${CMAKE_CURRENT_SOURCE_DIR}/doxygen/Doxyfile.in")
set(DOXYFILE_OUT "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile")
SET(DOXYGEN_STRIP_PATH ${CMAKE_CURRENT_SOURCE_DIR})
string(REPLACE ";" " " DOXYGEN_INPUTS "${PUBLIC_HEADERS}")
set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/doxygen")

configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

add_custom_command(OUTPUT ${DOXYGEN_INDEX_FILE}
                   DEPENDS ${PUBLIC_HEADERS}
                   COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                   MAIN_DEPENDENCY ${DOXYFILE_OUT} ${DOXYFILE_IN}
                   COMMENT "Generating docs")

add_custom_target(doxygen DEPENDS ${DOXYGEN_INDEX_FILE})

find_program(SPHINX_EXECUTABLE
             NAMES sphinx-build
             DOC "Path to sphinx-build executable")

set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/docs)
set(SPHINX_BUILD ${CMAKE_CURRENT_SOURCE_DIR}/docs/sphinx)

add_custom_target(sphinx
    COMMAND
    ${SPHINX_EXECUTABLE} -b html
    -Dbreathe_projects.gamelink=${DOXYGEN_OUTPUT_DIRECTORY}/xml
    ${SPHINX_SOURCE} ${SPHINX_BUILD}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating sphinx documentation")

add_dependencies(sphinx doxygen)