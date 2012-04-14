# CMake project file for compiling with MinGW

cmake_minimum_required(VERSION 2.8)

project(colobot CXX)

set(CMAKE_BUILD_TYPE debug)

# Currently compiles only with -fpermissive

set(CMAKE_CXX_FLAGS_RELEASE "-fpermissive -O2")
set(CMAKE_CXX_FLAGS_DEBUG "-fpermissive -w -g -O0")

add_subdirectory(src bin)
