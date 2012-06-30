# CMake project file for compiling with GCC/MinGW

cmake_minimum_required(VERSION 2.8)

project(colobot C CXX)

# Required packages
find_package(OpenGL REQUIRED)
find_package(SDL REQUIRED)
find_package(SDL_image REQUIRED)

# TODO: check for SDL version. Should be >= 1.2.10

# Build with debugging symbols
set(CMAKE_BUILD_TYPE debug)

# Global compile flags
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -O0")

# Subdirectory with sources
add_subdirectory(src bin)
