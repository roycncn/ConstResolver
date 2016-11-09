cmake_minimum_required(VERSION 3.6)
project(ConstResolver)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    ConstResolver.cpp)

add_executable(ConstResolver ${SOURCE_FILES})