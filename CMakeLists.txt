cmake_minimum_required(VERSION 3.11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Project
project(schema)

# Settings
set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g")

include_directories(.)
include_directories(third_party)
include_directories(schema/test)

file(GLOB SOURCES "schema/*.cpp")
file(GLOB TEST_SOURCES "schema/test/*.cpp")

add_executable(schema_tests ${TEST_SOURCES} ${SOURCES})