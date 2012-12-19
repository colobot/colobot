cmake_minimum_required(VERSION 2.8)

project(CBot_console C CXX)

# Build with debugging symbols
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE debug)
endif(NOT CMAKE_BUILD_TYPE)

# Global compile flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wold-style-cast -std=gnu++0x")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

# Include cmake directory
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${colobot_SOURCE_DIR}/cmake")

add_subdirectory(src bin)
