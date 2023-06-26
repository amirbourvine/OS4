cmake_minimum_required(VERSION 3.21)
project(OS4)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(OS4
        tests/header_1.h
        malloc_1.cpp
        malloc_2.cpp
        tests/test_1.cpp)
