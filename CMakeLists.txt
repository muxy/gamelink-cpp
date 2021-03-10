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

file(GLOB SOURCES "schema/*.cpp")
file(GLOB TEST_SOURCES "schema/test/*.cpp")

add_executable(schema_tests ${TEST_SOURCES} ${SOURCES})

find_package(PkgConfig)
pkg_check_modules(LIB_WEBSOCKETS REQUIRED libwebsockets>=4.0.0)
find_package(OpenSSL REQUIRED)

include_directories(include)
include_directories(third_party)
include_directories(${OPENSSL_INCLUDE_DIR} ${LIB_WEBSOCKETS_INCLUDE_DIRS})

add_executable(demo demo.cpp websocket.cpp ${SOURCES})
target_link_libraries(demo ${LIB_WEBSOCKETS_LIBRARIES})
