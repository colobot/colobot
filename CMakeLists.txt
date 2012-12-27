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
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE debug)
endif()

# Global compile flags
# These are specific to GCC/MinGW; for other compilers, change as necessary
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wold-style-cast -std=gnu++0x")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

# Asserts can be enabled/disabled regardless of build type
option(ASSERTS "Enable assert()s" ON)

# Building tests can be enabled/disabled
option(TESTS "Enable tests" ON)

# CBot can also be a static library
option(CBOT_STATIC "Build CBot as static libary" OFF)

# Doxygen docs are optional for installation
option(INSTALL_DOCS "Install Doxygen-generated documentation" OFF)

# Build openal sound support
option(OPENAL_SOUND "Build openal sound support" OFF)

##
# Required packages
##

find_package(OpenGL 1.4 REQUIRED)
find_package(SDL 1.2.10 REQUIRED)
find_package(SDL_image 1.2 REQUIRED)
find_package(SDL_ttf 2.0 REQUIRED)
find_package(PNG 1.2 REQUIRED)
find_package(Gettext REQUIRED)

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
# Doxygen docs
##

find_package(Doxygen)

if(DOXYGEN_FOUND AND DOXYGEN_DOT_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc
                      ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
                      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                      COMMENT "Generating API documentation with Doxygen" VERBATIM)
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

# Installation paths defined before compiling sources
set(COLOBOT_INSTALL_BIN_DIR games CACHE PATH "Colobot binary directory")
set(COLOBOT_INSTALL_DATA_DIR share/games/colobot CACHE PATH "Colobot shared data directory")
set(COLOBOT_INSTALL_LIB_DIR lib/colobot CACHE PATH "Colobot libraries directory")
set(COLOBOT_INSTALL_DOC_DIR share/doc/colobot CACHE PATH "Colobot documentation directory")
set(COLOBOT_INSTALL_I18N_DIR share/locale CACHE PATH "Colobot translations directory")

# Subdirectory with sources
add_subdirectory(src bin)


##
# Installation
##

file(GLOB DATA_FILES "data/*")

# Data
install(DIRECTORY data/ DESTINATION ${COLOBOT_INSTALL_DATA_DIR})

# Documentation
if(INSTALL_DOCS AND DOXYGEN_FOUND AND DOXYGEN_DOT_FOUND)
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc/ DESTINATION ${COLOBOT_INSTALL_DOC_DIR} OPTIONAL)
endif()
