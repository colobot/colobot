# CMake project file for compiling with GCC/MinGW

cmake_minimum_required(VERSION 2.8)

project(colobot C CXX)

# Required packages
find_package(OpenGL 1.4 REQUIRED)
find_package(SDL 1.2.10 REQUIRED)
find_package(SDL_image 1.2 REQUIRED)
find_package(SDL_ttf 2.0 REQUIRED)
find_package(PNG 1.2 REQUIRED)

# GLEW requirement depends on platform
# By default it is auto detected
# This setting may be used to override
# Possible values:
# - auto -> determine automatically
# - 1 -> always enable
# - 0 -> always disable
set(USE_GLEW auto)

# Build with debugging symbols
set(CMAKE_BUILD_TYPE debug)

# Global compile flags
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -Wall -Wold-style-cast -std=gnu++0x")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wold-style-cast -std=gnu++0x")

# Include cmake directory
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${colobot_SOURCE_DIR}/cmake")

# Subdirectory with sources
add_subdirectory(src bin lib)
