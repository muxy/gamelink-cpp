cmake_minimum_required(VERSION 3.11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Project
project(GamelinkExamples)

# Settings
set(CMAKE_CXX_FLAGS "-Wall -DFMT_HEADER_ONLY=1 -pthread")
set(CMAKE_CXX_FLAGS_DEBUG "-g")

include_directories(..)
include_directories(../third_party)
include_directories(../websocket_network)

file(GLOB DEMO_SOURCES "../websocket_network/websocket.cpp")

find_package(PkgConfig)
pkg_check_modules(LIB_WEBSOCKETS REQUIRED libwebsockets>=4.0.0)
find_package(OpenSSL REQUIRED)

add_executable(voting 02-voting/main.cpp ${DEMO_SOURCES})
target_include_directories(voting PRIVATE ${OPENSSL_INCLUDE_DIR} ${LIB_WEBSOCKETS_INCLUDE_DIRS})
target_link_directories(voting PRIVATE ${LIB_WEBSOCKETS_LIBRARY_DIRS})
target_link_libraries(voting ${LIB_WEBSOCKETS_LIBRARIES})
