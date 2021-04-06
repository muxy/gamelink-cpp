cmake_minimum_required(VERSION 3.11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# Project
project(GamelinkExamples)

# Settings
set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g")

include_directories(../..)
include_directories(../shared)

list(APPEND WEBSOCKET_SOURCES "../../websocket_network/websocket.cpp")

find_package(PkgConfig)
pkg_check_modules(LIB_WEBSOCKETS REQUIRED libwebsockets>=4.0.0)
find_package(OpenSSL REQUIRED)

set(TARGET_NAME state)

add_executable(${TARGET_NAME} main.cpp ${WEBSOCKET_SOURCES})
set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(${TARGET_NAME} PRIVATE ${OPENSSL_INCLUDE_DIR} ${LIB_WEBSOCKETS_INCLUDE_DIRS})
target_link_directories(${TARGET_NAME} PRIVATE ${LIB_WEBSOCKETS_LIBRARY_DIRS})
target_link_libraries(${TARGET_NAME} ${LIB_WEBSOCKETS_LIBRARIES})