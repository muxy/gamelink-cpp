cmake_minimum_required(VERSION 3.11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Project
project(Gamelink)

# Settings
set(CMAKE_CXX_FLAGS "-Wall -DFMT_HEADER_ONLY=1 -pthread")
set(CMAKE_CXX_FLAGS_DEBUG "-g")

include_directories(.)
include_directories(third_party)
include_directories(schema/test)
include_directories(src/test)

## Grab all the source files
file(GLOB SCHEMA_SOURCES "schema/*.cpp")
file(GLOB SCHEMA_TEST_SOURCES "schema/test/*.cpp")
file(GLOB SDK_TEST_SOURCES "src/test/*.cpp")
file(GLOB SDK_SOURCES "src/*.cpp")
file(GLOB DEMO_SOURCES "*.cpp")

# Schema unit tests
add_executable(tests ${SCHEMA_TEST_SOURCES} ${SCHEMA_SOURCES} ${SDK_TEST_SOURCES} ${SDK_SOURCES})

# SDK demo app
find_package(PkgConfig)
find_package(Doxygen)
pkg_check_modules(LIB_WEBSOCKETS REQUIRED libwebsockets>=4.0.0)
find_package(OpenSSL REQUIRED)

include_directories(include)
include_directories(third_party)
include_directories(${OPENSSL_INCLUDE_DIR} ${LIB_WEBSOCKETS_INCLUDE_DIRS})
link_directories(${LIB_WEBSOCKETS_LIBRARY_DIRS})

add_executable(demo ${DEMO_SOURCES})
target_link_libraries(demo ${LIB_WEBSOCKETS_LIBRARIES})

# Doc
file(GLOB SCHEMA_PUBLIC_HEADERS "schema/*.h")
file(GLOB SRC_PUBLIC_HEADERS "src/*.hpp")

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

add_custom_target(Doxygen ALL DEPENDS ${DOXYGEN_INDEX_FILE})

find_program(SPHINX_EXECUTABLE
             NAMES sphinx-build
             DOC "Path to sphinx-build executable")

set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/docs)
set(SPHINX_BUILD ${CMAKE_CURRENT_SOURCE_DIR}/docs/sphinx)

add_custom_target(Sphinx ALL
    COMMAND
    ${SPHINX_EXECUTABLE} -b html
    -Dbreathe_projects.gamelink=${DOXYGEN_OUTPUT_DIRECTORY}/xml
    ${SPHINX_SOURCE} ${SPHINX_BUILD}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating sphinx documentation")


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