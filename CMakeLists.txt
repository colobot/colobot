##
# Main CMake project file
# Contains global options and definitions
##

cmake_minimum_required(VERSION 2.8)

project(colobot C CXX)

# Include cmake directory with some additional scripts
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${colobot_SOURCE_DIR}/cmake")


##
# Build options
##

# Global build type
set(CMAKE_BUILD_TYPE debug)

# Global compile flags
# These are specific to GCC/MinGW; for other compilers, change as necessary
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -Wall -Wold-style-cast -std=gnu++0x")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wold-style-cast -std=gnu++0x")

# Asserts can be enabled/disabled regardless of build type
option(ASSERTS "Enable assert()s" ON)

# Building tests can be enabled/disabled
option(TESTS "Enable tests" ON)

# CBot can also be a static library
option(CBOT_STATIC "Build CBot as static libary" OFF)


##
# Required packages
##

find_package(OpenGL 1.4 REQUIRED)
find_package(SDL 1.2.10 REQUIRED)
find_package(SDL_image 1.2 REQUIRED)
find_package(SDL_ttf 2.0 REQUIRED)
find_package(PNG 1.2 REQUIRED)
find_package(LTDL 2.4.2 REQUIRED)

set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)

set(Boost_ADDITIONALVERSION "1.51" "1.51.0")
find_package(Boost COMPONENTS system filesystem regex REQUIRED)

# GLEW requirement depends on platform
# By default it is auto detected
# This setting may be used to override
# Possible values:
# - auto -> determine automatically
# - 1 -> always enable
# - 0 -> always disable
set(USE_GLEW auto)
# This is useful on Windows, if linking against standard GLEW dll fails
option(GLEW_STATIC "Link statically with GLEW" OFF)


##
# Additional settings to use when cross-compiling with MXE (http://mxe.cc/)
##

include("${colobot_SOURCE_DIR}/cmake/mxe.cmake")


##
# Platform detection and some related checks
##

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    message(STATUS "Windows system detected")
    set(PLATFORM_WINDOWS 1)
    set(PLATFORM_LINUX   0)
    set(PLATFORM_OTHER   0)

    # On Windows, GLEW is required
    if (${USE_GLEW} MATCHES "auto")
        set(USE_GLEW 1)
    endif()
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    message(STATUS "Linux system detected")
    set(PLATFORM_WINDOWS 0)
    set(PLATFORM_LINUX   1)
    set(PLATFORM_OTHER   0)

    # On Linux, we should be fine without GLEW
    if (${USE_GLEW} MATCHES "auto")
        set(USE_GLEW 0)
    endif()
else()
    message(STATUS "Other system detected")
    set(PLATFORM_WINDOWS 0)
    set(PLATFORM_LINUX   0)
    set(PLATFORM_OTHER   1)

    # Use GLEW to be safe
    if (${USE_GLEW} MATCHES "auto")
        set(USE_GLEW 1)
    endif()
endif()


if(${USE_GLEW})
    find_package(GLEW REQUIRED)
endif()

if(NOT ${ASSERTS})
    add_definitions(-DNDEBUG)
endif()

if(${TESTS})
    add_definitions(-DTEST_VIRTUAL=virtual)
    enable_testing()
else()
    add_definitions(-DTEST_VIRTUAL)
endif()


##
# Targets
##

if(${TESTS})
    # Google Test library
    find_path(GTEST_SRC_DIR NAMES src/gtest.cc src/gtest-all.cc PATHS /usr/src PATH_SUFFIXES gtest)
    find_path(GTEST_INCLUDE_DIR gtest/gtest.h PATHS /usr/include)
    if(GTEST_SRC_DIR AND GTEST_INCLUDE_DIR)
        message(STATUS "Using system gtest library in ${GTEST_SRC_DIR}")
    else()
        message(STATUS "Using bundled gtest library")
        set(GTEST_SRC_DIR ${colobot_SOURCE_DIR}/lib/gtest)
        set(GTEST_INCLUDE_DIR ${colobot_SOURCE_DIR}/lib/gtest/include)
    endif()

    add_subdirectory(${GTEST_SRC_DIR} bin/gtest)

    # Google Mock library
    find_path(GMOCK_SRC_DIR NAMES src/gmock.cc src/gmock-all.cc PATHS /usr/src PATH_SUFFIXES gmock)
    find_path(GMOCK_INCLUDE_DIR gmock/gmock.h PATHS /usr/include)
    if(GMOCK_SRC_DIR AND GMOCK_INCLUDE_DIR)
        message(STATUS "Using system gmock library in ${GMOCK_SRC_DIR}")

        include_directories(${GMOCK_SRC_DIR})
        # gmock-all.cc includes all other sources
        add_library(gmock STATIC ${GMOCK_SRC_DIR}/src/gmock-all.cc)
    else()
        message(STATUS "Using bundled gmock library")
        set(GMOCK_SRC_DIR ${colobot_SOURCE_DIR}/lib/gmock)
        set(GMOCK_INCLUDE_DIR ${colobot_SOURCE_DIR}/lib/gmock/include)
        add_subdirectory(${GMOCK_SRC_DIR} bin/gmock)
    endif()

endif()

# Subdirectory with sources
add_subdirectory(src bin)

# TODO: provide data files as git submodule and setup correct data path
install_files(/share/games/colobot ../data)
