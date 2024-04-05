##
# Main CMake project file
# Contains global options and definitions
##

cmake_minimum_required(VERSION 3.21)

cmake_policy(SET CMP0025 NEW)

project(colobot
    VERSION 0.2.1.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

##
# Project version
##
set(COLOBOT_VERSION_CODENAME "Gold")

# Used on official releases
#set(COLOBOT_VERSION_RELEASE_CODENAME "-alpha")
# Used on unreleased, development builds
set(COLOBOT_VERSION_UNRELEASED "+alpha")

# Append git characteristics to version
if(DEFINED COLOBOT_VERSION_UNRELEASED)
    if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
        find_package(Git)
        execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
                        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
                        OUTPUT_VARIABLE GIT_BRANCH
                        OUTPUT_STRIP_TRAILING_WHITESPACE)
        execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
                        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
                        OUTPUT_VARIABLE GIT_REVISION
                        OUTPUT_STRIP_TRAILING_WHITESPACE)
        set(COLOBOT_VERSION_UNRELEASED "${COLOBOT_VERSION_UNRELEASED}-git-${GIT_BRANCH}~r${GIT_REVISION}")
        set(COLOBOT_VERSION_DISPLAY "git-${GIT_BRANCH}~r${GIT_REVISION}")
    else()
        set(COLOBOT_VERSION_DISPLAY "${COLOBOT_VERSION_CODENAME}-${COLOBOT_VERSION_UNRELEASED}")
    endif()
else()
    set(COLOBOT_VERSION_DISPLAY "${CMAKE_PROJECT_VERSION}${COLOBOT_VERSION_RELEASE_CODENAME}")
endif()

set(COLOBOT_VERSION_FULL "${CMAKE_PROJECT_VERSION}${COLOBOT_VERSION_UNRELEASED}${COLOBOT_VERSION_RELEASE_CODENAME}")
message(STATUS "Building Colobot \"${COLOBOT_VERSION_CODENAME}\" (${COLOBOT_VERSION_FULL})")

set(BUILD_NUMBER 0)
if(NOT "$ENV{BUILD_NUMBER}" STREQUAL "")
    set(BUILD_NUMBER "$ENV{BUILD_NUMBER}")
    message(STATUS "CI build #${BUILD_NUMBER}")
endif()


##
# Platform detection and some related checks
##
include("${PROJECT_SOURCE_DIR}/cmake/systems/${CMAKE_SYSTEM_NAME}.cmake" OPTIONAL RESULT_VARIABLE SYSTEM_INCLUDE)

if ("${SYSTEM_INCLUDE}" EQUAL NOTFOUND)
    include("${PROJECT_SOURCE_DIR}/cmake/systems/Other.cmake")
endif()

##
# Build options
##

# Build targets should be placed in the root build directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Include cmake directory with some additional scripts
list(PREPEND CMAKE_MODULE_PATH 
    ${PROJECT_SOURCE_DIR}/cmake
)

# Compiler detection
include("${PROJECT_SOURCE_DIR}/cmake/compilers/${CMAKE_CXX_COMPILER_ID}.cmake" OPTIONAL RESULT_VARIABLE COMPILER_INCLUDE)

if ("${COMPILER_INCLUDE}" EQUAL NOTFOUND)
    message(WARNING "Your C++ compiler doesn't seem to be supported.")
endif()


# Global compile flags
# These are specific to GCC/MinGW/clang; for other compilers, change as necessary
# The flags are used throughout src/ and test/ subdirs

set(COLOBOT_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${NORMAL_CXX_FLAGS}")
set(COLOBOT_CXX_FLAGS_RELEASE "${RELEASE_CXX_FLAGS}")
set(COLOBOT_CXX_FLAGS_DEBUG "${DEBUG_CXX_FLAGS}")

# Flags for gtest
set(COLOBOT_GTEST_CXX_FLAGS "${TEST_CXX_FLAGS}")


# Asserts can be enabled/disabled regardless of build type
option(ASSERTS "Enable assert()s" ON)

# Enables development mode
option(COLOBOT_DEVELOPMENT_MODE "Setup project for development (e.g. copies data content to build directory for execution)" OFF)

# Official build - changes text on the crash screen
# PLEASE DO NOT USE ON UNOFFICIAL BUILDS. Thanks.
option(COLOBOT_OFFICIAL_BUILD "Official build (changes crash screen text)" OFF)

# Hardcode relative paths instead of absolute paths
option(USE_RELATIVE_PATHS "Generate relative paths from absolute paths" OFF)

# Portable build - load all data from the base directory
option(PORTABLE "Portable build" OFF)

# Portable saves - suitable for e.g. putting the whole game on external storage and moving your saves with it
option(PORTABLE_SAVES "Portable saves" OFF)

# Building tests can be enabled/disabled
option(TESTS "Build tests" OFF)

# Building tool programs can be enabled/disabled
option(TOOLS "Build tool programs" OFF)

# Generate desktop files, manpage, etc.
option(DESKTOP "Generate desktop files, manpages, etc" ON)

# Doxygen docs are optional for installation
option(INSTALL_DOCS "Install Doxygen-generated documentation" OFF)

# Build OpenAL sound support
option(OPENAL_SOUND "Build OpenAL sound support" ON)

# Link runtime library statically (currently only works for MSVC)
option(USE_STATIC_RUNTIME "Link the runtime library statically" OFF)

# CBot can also be a static library
option(CBOT_STATIC "Build CBot as static libary" OFF)

# This is useful on Windows, if linking against standard GLEW dll fails
option(GLEW_STATIC "Link statically with GLEW" OFF)

# Link statically with LibSndFile
option(SNDFILE_STATIC "Link statically with LibSndFile" OFF)

# Sometimes helpful if there is a different version of gtest installed on system vs bundled
option(FORCE_BUNDLED_GTEST "Force the use of bundled gtest" OFF)

# This is for use with colobot-lint tool
option(COLOBOT_LINT_BUILD "Generate some additional CMake targets for use with colobot-lint" OFF)

# Enable this if your compiler has issues using char8_t
option(COLOBOT_CHAR8_T_OVERRIDE "Override char8_t usage for compatibility reasons" OFF)

# Default build type if not given is debug
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Build type not specified - assuming debug")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

##
# Additional settings for MSYS
##
include("${colobot_SOURCE_DIR}/cmake/msys.cmake")

##
# Additional functions for colobot-lint
##
include("${colobot_SOURCE_DIR}/cmake/colobot-lint.cmake")

##
# Searching for packages
##

find_package(OpenGL 1.4 REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(PNG 1.2 REQUIRED)
find_package(Gettext REQUIRED)
find_package(Intl REQUIRED)
find_package(PhysFS REQUIRED)
find_package(glm CONFIG REQUIRED)

# Add target alias glm::glm for older versions of the library
if(NOT TARGET glm::glm)
    add_library(glm::glm ALIAS glm)
endif()

find_package(nlohmann_json 3.10 QUIET)

if(NOT nlohmann_json_FOUND)
    message(STATUS "Using nlohmann_json git submodule")
    set(JSON_BuildTests OFF CACHE INTERNAL "")
    add_subdirectory(lib/json)
endif()

set(GLEW_USE_STATIC_LIBS ${GLEW_STATIC})
find_package(GLEW REQUIRED)

if (OPENAL_SOUND)
    find_package(OpenAL REQUIRED)

    if (NOT TARGET OpenAL::OpenAL)
        add_library(OpenAL::OpenAL UNKNOWN IMPORTED)

        set_target_properties(OpenAL::OpenAL PROPERTIES
            IMPORTED_LOCATION "${OPENAL_LIBRARY}")
        set_target_properties(OpenAL::OpenAL PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${OPENAL_INCLUDE_DIR}")
    endif()
endif()

find_package(SndFile REQUIRED)

if(NOT ASSERTS)
    add_definitions(-DNDEBUG)
endif()

if(TESTS)
    add_definitions(-DTESTS -DTEST_VIRTUAL=virtual)
else()
    add_definitions(-DTEST_VIRTUAL=)
endif()

##
# MSVC specific settings
##
set(WINGETOPT 0)
if(MSVC)
    message(STATUS "Adding MSVC-specific options")

    set(WINGETOPT 1) # use wingetopt library
endif()

##
# Localename
##
add_subdirectory(lib/localename)


##
# Wingetopt
##
if(WINGETOPT)
    add_subdirectory(lib/wingetopt)
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

# Installation paths defined before compiling sources
if (PORTABLE)
    set(COLOBOT_INSTALL_BIN_DIR ${CMAKE_INSTALL_PREFIX}/)
    set(COLOBOT_INSTALL_LIB_DIR ${CMAKE_INSTALL_PREFIX}/)
    set(COLOBOT_INSTALL_DATA_DIR ${CMAKE_INSTALL_PREFIX}/data)
    set(COLOBOT_INSTALL_I18N_DIR ${CMAKE_INSTALL_PREFIX}/lang)
    set(COLOBOT_INSTALL_DOC_DIR ${CMAKE_INSTALL_PREFIX}/doc)

    set(USE_RELATIVE_PATHS ON)
else()
    set(COLOBOT_INSTALL_BIN_DIR ${CMAKE_INSTALL_PREFIX}/${PLATFORM_INSTALL_BIN_DIR})
    set(COLOBOT_INSTALL_LIB_DIR ${CMAKE_INSTALL_PREFIX}/${PLATFORM_INSTALL_LIB_DIR})
    set(COLOBOT_INSTALL_DATA_DIR ${CMAKE_INSTALL_PREFIX}/${PLATFORM_INSTALL_DATA_DIR})
    set(COLOBOT_INSTALL_I18N_DIR ${CMAKE_INSTALL_PREFIX}/${PLATFORM_INSTALL_I18N_DIR})
    set(COLOBOT_INSTALL_DOC_DIR ${CMAKE_INSTALL_PREFIX}/${PLATFORM_INSTALL_DOC_DIR})
endif()

# Generate relative paths from absolute paths
if(USE_RELATIVE_PATHS)
    message(STATUS "Generating relative paths")
    file(RELATIVE_PATH COLOBOT_DATA_DIR ${COLOBOT_INSTALL_BIN_DIR} ${COLOBOT_INSTALL_DATA_DIR})
    file(RELATIVE_PATH COLOBOT_I18N_DIR ${COLOBOT_INSTALL_BIN_DIR} ${COLOBOT_INSTALL_I18N_DIR})

    add_definitions(-DUSE_RELATIVE_PATHS)
else()
    set(COLOBOT_DATA_DIR ${COLOBOT_INSTALL_DATA_DIR})
    set(COLOBOT_I18N_DIR ${COLOBOT_INSTALL_I18N_DIR})
endif()

# Subdirectory with common implementation
add_subdirectory(colobot-common)

# Add CBot
add_subdirectory(CBot)

# Add base Colobot implementation
add_subdirectory(colobot-base)

# Add Colobot executable
add_subdirectory(colobot-app)

add_subdirectory(po)

if(TOOLS)
    add_subdirectory(tools)
endif()

if(DESKTOP)
    add_subdirectory(desktop)
endif()

if(TESTS)
    # Google Test library
    find_package(GTest 1.11.0 QUIET)
    if(NOT(FORCE_BUNDLED_GTEST) AND GTEST_FOUND)
        message(STATUS "Using system gtest library in ${GTEST_BOTH_LIBRARIES}")
    elseif(EXISTS "${colobot_SOURCE_DIR}/lib/googletest/googletest/CMakeLists.txt")
        message(STATUS "Using gtest git submodule")
        set(GOOGLETEST_VERSION "1.11.0")
        add_subdirectory("${colobot_SOURCE_DIR}/lib/googletest/googletest" "lib/googletest/googletest")
        # Add aliases so target names are compatible with the find_package above
        add_library(GTest::GTest ALIAS gtest)
        add_library(GTest::Main ALIAS gtest_main)
    else()
        message(FATAL_ERROR "Could not find gtest, cannot enable testing")
    endif()

    # Hippomocks library
    add_subdirectory(${colobot_SOURCE_DIR}/lib/hippomocks)

    # Tests targets
    enable_testing()
    include(GoogleTest)
    add_subdirectory(test)
endif()


##
# Installation
##

# Data: check if the submodule handles its own installation
if(EXISTS "${CMAKE_SOURCE_DIR}/data/CMakeLists.txt")
    message(STATUS "Data directory will install itself.")
    add_subdirectory(data)
else()
    message(WARNING "Data directory is not available; make sure colobot-data is installed in ${COLOBOT_INSTALL_DATA_DIR}.")
endif()

# Documentation
if(INSTALL_DOCS AND DOXYGEN_FOUND AND DOXYGEN_DOT_FOUND)
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc/ DESTINATION ${COLOBOT_INSTALL_DOC_DIR} OPTIONAL)
endif()
