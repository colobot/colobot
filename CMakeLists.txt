##
# Main CMake project file
# Contains global options and definitions
##

cmake_minimum_required(VERSION 3.16)

project(colobot C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

##
# Project version
##
set(COLOBOT_VERSION_CODENAME "Gold")
set(COLOBOT_VERSION_MAJOR    0)
set(COLOBOT_VERSION_MINOR    2)
set(COLOBOT_VERSION_REVISION 2)

# Used on official releases
set(COLOBOT_VERSION_RELEASE_CODENAME "-alpha")
# Used on unreleased, development builds
#set(COLOBOT_VERSION_UNRELEASED "+alpha")

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
    set(COLOBOT_VERSION_DISPLAY "${COLOBOT_VERSION_MAJOR}.${COLOBOT_VERSION_MINOR}.${COLOBOT_VERSION_REVISION}${COLOBOT_VERSION_RELEASE_CODENAME}")
endif()

set(COLOBOT_VERSION_FULL "${COLOBOT_VERSION_MAJOR}.${COLOBOT_VERSION_MINOR}.${COLOBOT_VERSION_REVISION}${COLOBOT_VERSION_UNRELEASED}${COLOBOT_VERSION_RELEASE_CODENAME}")
message(STATUS "Building Colobot \"${COLOBOT_VERSION_CODENAME}\" (${COLOBOT_VERSION_FULL})")

set(BUILD_NUMBER 0)
if(NOT "$ENV{BUILD_NUMBER}" STREQUAL "")
    set(BUILD_NUMBER "$ENV{BUILD_NUMBER}")
    message(STATUS "CI build #${BUILD_NUMBER}")
endif()


##
# Platform detection and some related checks
##
if("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
    message(STATUS "Build for Windows system")
    set(PLATFORM_WINDOWS 1)
    set(PLATFORM_GNU     0)
    set(PLATFORM_LINUX   0)
    set(PLATFORM_MACOSX  0)
    set(PLATFORM_FREEBSD 0)
    set(PLATFORM_OTHER   0)

    # Platform-dependent implementation of system.h
    set(SYSTEM_CPP_MODULE "system_windows.cpp")
    set(SYSTEM_H_MODULE "system_windows.h")
elseif("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    message(STATUS "Build for Linux system")
    set(PLATFORM_WINDOWS 0)
    set(PLATFORM_LINUX   1)
    set(PLATFORM_GNU     1)
    set(PLATFORM_MACOSX  0)
    set(PLATFORM_FREEBSD 0)
    set(PLATFORM_OTHER   0)

    # Platform-dependent implementation of system.h
    set(SYSTEM_CPP_MODULE "system_linux.cpp")
    set(SYSTEM_H_MODULE "system_linux.h")
elseif("${CMAKE_SYSTEM_NAME}" MATCHES "kFreeBSD" OR "${CMAKE_SYSTEM_NAME}" STREQUAL "GNU")
    message(STATUS "Build for kFreeBSD system")
    set(PLATFORM_WINDOWS 0)
    set(PLATFORM_LINUX   0)
    set(PLATFORM_GNU     1)
    set(PLATFORM_MACOSX  0)
    set(PLATFORM_FREEBSD 0)
    set(PLATFORM_OTHER   0)

    # Platform-dependent implementation of system.h
    set(SYSTEM_CPP_MODULE "system_other.cpp")
    set(SYSTEM_H_MODULE "system_other.h")
elseif("${CMAKE_SYSTEM_NAME}" MATCHES "Darwin")
    message(STATUS "Build for Mac OSX system")
    set(PLATFORM_WINDOWS 0)
    set(PLATFORM_LINUX   0)
    set(PLATFORM_GNU     0)
    set(PLATFORM_MACOSX  1)
    set(PLATFORM_OTHER   0)
    set(PLATFORM_FREEBSD 0)

    # Platform-dependent implementation of system.h
    set(SYSTEM_CPP_MODULE "system_macosx.cpp")
    set(SYSTEM_H_MODULE "system_macosx.h")
    # Fix compilation errors in MacOS SDK files
    set(CMAKE_CXX_FLAGS "${NORMAL_CXX_FLAGS} -Wno-nullability-extension -Wno-nullability-completeness -Wno-expansion-to-defined -Wno-four-char-constants -Wno-gnu-zero-variadic-macro-arguments -Wno-variadic-macros -Wno-zero-length-array")
    # To avoid CMake warning
    set(CMAKE_MACOSX_RPATH 1)
elseif("${CMAKE_SYSTEM_NAME}" MATCHES "FreeBSD")
    message(STATUS "Build for FreeBSD system")
    set(PLATFORM_WINDOWS 0)
    set(PLATFORM_LINUX   0)
    set(PLATFORM_GNU     0)
    set(PLATFORM_MACOSX  0)
    set(PLATFORM_FREEBSD 1)
    set(PLATFORM_OTHER   0)

    # Platform-dependent implementation of system.h
    # On FreeBSD we can use *_other
    set(SYSTEM_CPP_MODULE "system_other.cpp")
    set(SYSTEM_H_MODULE "system_other.h")
    # To avoid CMake warning
    set(CMAKE_MACOSX_RPATH 1)
else()
    message(STATUS "Build for other system")
    set(PLATFORM_WINDOWS 0)
    set(PLATFORM_LINUX   0)
    set(PLATFORM_GNU     0)
    set(PLATFORM_MACOSX  0)
    set(PLATFORM_FREEBSD 0)
    set(PLATFORM_OTHER   1)

    # Platform-dependent implementation of system.h
    set(SYSTEM_CPP_MODULE "system_other.cpp")
    set(SYSTEM_H_MODULE "system_other.h")
endif()


##
# Build options
##

# Build targets should be placed in the root build directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Include cmake directory with some additional scripts
set(CMAKE_MODULE_PATH "${colobot_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Compiler detection
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.7)
        message(FATAL_ERROR "${PROJECT_NAME} requires GCC 4.7 or greater.")
    endif()

    message(STATUS "Detected GCC version 4.7+")

    set(NORMAL_CXX_FLAGS "-Wall -Wold-style-cast -pedantic-errors -Wmissing-declarations")
    set(NORMAL_CXX_FLAGS "${NORMAL_CXX_FLAGS} -Wno-error=deprecated-declarations") # updated version of physfs is not available on some platforms so we keep using deprecated functions, see #958

    if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0)
        set(NORMAL_CXX_FLAGS "${NORMAL_CXX_FLAGS} -Wsuggest-override")
    endif()

    set(RELEASE_CXX_FLAGS "-O2")
    set(DEBUG_CXX_FLAGS "-g -O0")
    set(TEST_CXX_FLAGS "-pthread")
    add_definitions(-DNOEXCEPT=noexcept -DHAVE_DEMANGLE)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.1)
        message(FATAL_ERROR "${PROJECT_NAME} requires Clang 3.1 or greater.")
    endif()

    message(STATUS "Detected Clang version 3.1+")

    if (${PLATFORM_FREEBSD})
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=bfd")
    endif()

    set(NORMAL_CXX_FLAGS "-Wall -Wold-style-cast -pedantic-errors -Wmissing-prototypes")
    set(NORMAL_CXX_FLAGS "${NORMAL_CXX_FLAGS} -Wno-error=deprecated-declarations") # updated version of physfs is not available on some platforms so we keep using deprecated functions, see #958
    set(RELEASE_CXX_FLAGS "-O2")
    set(DEBUG_CXX_FLAGS "-g -O0")
    set(TEST_CXX_FLAGS "-pthread")
    add_definitions(-DNOEXCEPT=noexcept -DHAVE_DEMANGLE)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    message(STATUS "Detected MSVC compiler")

    # Disable some useless warnings
    set(NORMAL_CXX_FLAGS "/wd\"4244\" /wd\"4309\" /wd\"4800\" /wd\"4996\" /wd\"4351\"")
    if(USE_STATIC_RUNTIME)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    else()
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    endif()
    set(TEST_CXX_FLAGS "")
    add_definitions(-DNOEXCEPT= -DHAS_MSVC_EXCEPTION_BUG)

    # Increase the stack size to 8 MB (the default is 1 MB), needed for CBOT
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:8388608")

    # Needed for Debug information (it's set to "No" by default for some reason)
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS} /DEBUG /NODEFAULTLIB:MSVCRTD /NODEFAULTLIB:LIBCMT")
    set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS} /DEBUG")
else()
    message(FATAL_ERROR "Your C++ compiler doesn't seem to be supported.")
endif()


# Global compile flags
# These are specific to GCC/MinGW/clang; for other compilers, change as necessary
# The flags are used throughout src/ and test/ subdirs

# Special flags for boost
add_definitions(-DBOOST_NO_SCOPED_ENUMS -DBOOST_NO_CXX11_SCOPED_ENUMS)

set(COLOBOT_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${NORMAL_CXX_FLAGS}")
set(COLOBOT_CXX_FLAGS_RELEASE "${RELEASE_CXX_FLAGS}")
set(COLOBOT_CXX_FLAGS_DEBUG "${DEBUG_CXX_FLAGS}")

# Flags for gtest
set(COLOBOT_GTEST_CXX_FLAGS "${TEST_CXX_FLAGS}")


# Asserts can be enabled/disabled regardless of build type
option(ASSERTS "Enable assert()s" ON)

# Development build can be enabled/disabled regardless of build type
option(DEV_BUILD "Enable development build (enables some debugging tools, local setting paths, etc.)" OFF)

# Official build - changes text on the crash screen
# PLEASE DO NOT USE ON UNOFFICIAL BUILDS. Thanks.
option(OFFICIAL_COLOBOT_BUILD "Official build (changes crash screen text)" OFF)

# Hide console on Windows (useful for release builds)
option(HIDE_CONSOLE "Hide console" OFF)

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

# This is useful in case you want to use static boost libraries
option(BOOST_STATIC "Link with static boost libraries" OFF)

# This is useful on Windows, if linking against standard GLEW dll fails
option(GLEW_STATIC "Link statically with GLEW" OFF)

# Link statically with LibSndFile
option(SNDFILE_STATIC "Link statically with LibSndFile" OFF)

# Sometimes helpful if there is a different version of gtest installed on system vs bundled
option(FORCE_BUNDLED_GTEST "Force the use of bundled gtest" OFF)

# This is for use with colobot-lint tool
option(COLOBOT_LINT_BUILD "Generate some additional CMake targets for use with colobot-lint" OFF)

# Default build type if not given is debug
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Build type not specified - assuming debug")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

# Warn about development build
if(DEV_BUILD)
    message(STATUS "Building with development extensions")
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
find_package(PhysFS REQUIRED)

set(Boost_USE_STATIC_LIBS ${BOOST_STATIC})
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME ${USE_STATIC_RUNTIME})

set(Boost_ADDITIONALVERSION "1.51" "1.51.0")
find_package(Boost COMPONENTS system filesystem regex REQUIRED)

set(GLEW_USE_STATIC_LIBS ${GLEW_STATIC})
find_package(GLEW REQUIRED)

if (OPENAL_SOUND)
    find_package(OpenAL REQUIRED)
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

if(DEV_BUILD)
    add_definitions(-DDEV_BUILD)
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
if(PORTABLE)
    set(COLOBOT_INSTALL_BIN_DIR ${CMAKE_INSTALL_PREFIX}/ CACHE PATH "Colobot binary directory")
    set(COLOBOT_INSTALL_LIB_DIR ${CMAKE_INSTALL_PREFIX}/ CACHE PATH "Colobot libraries directory")
    set(COLOBOT_INSTALL_DATA_DIR ${CMAKE_INSTALL_PREFIX}/data CACHE PATH "Colobot shared data directory")
    set(COLOBOT_INSTALL_I18N_DIR ${CMAKE_INSTALL_PREFIX}/lang CACHE PATH "Colobot translations directory")
    set(COLOBOT_INSTALL_DOC_DIR ${CMAKE_INSTALL_PREFIX}/doc CACHE PATH "Colobot documentation directory")
    set(USE_RELATIVE_PATHS ON)
elseif(PLATFORM_WINDOWS)
    set(COLOBOT_INSTALL_BIN_DIR ${CMAKE_INSTALL_PREFIX}/ CACHE PATH "Colobot binary directory")
    set(COLOBOT_INSTALL_LIB_DIR ${CMAKE_INSTALL_PREFIX}/ CACHE PATH "Colobot libraries directory")
    set(COLOBOT_INSTALL_DATA_DIR ${CMAKE_INSTALL_PREFIX}/data CACHE PATH "Colobot shared data directory")
    set(COLOBOT_INSTALL_I18N_DIR ${CMAKE_INSTALL_PREFIX}/lang CACHE PATH "Colobot translations directory")
    set(COLOBOT_INSTALL_DOC_DIR ${CMAKE_INSTALL_PREFIX}/doc CACHE PATH "Colobot documentation directory")
elseif(PLATFORM_MACOSX)
    set(COLOBOT_INSTALL_BIN_DIR ../MacOS CACHE STRING "Colobot binary directory")
    set(COLOBOT_INSTALL_LIB_DIR ../MacOS CACHE STRING "Colobot libraries directory")
    set(COLOBOT_INSTALL_DATA_DIR . CACHE STRING "Colobot shared data directory")
    set(COLOBOT_INSTALL_I18N_DIR i18n CACHE SRING "Colobot translations directory")
    set(COLOBOT_INSTALL_DOC_DIR doc CACHE STRING "Colobot documentation directory")
else()
    set(COLOBOT_INSTALL_BIN_DIR ${CMAKE_INSTALL_PREFIX}/games CACHE PATH "Colobot binary directory")
    set(COLOBOT_INSTALL_LIB_DIR ${CMAKE_INSTALL_PREFIX}/lib/colobot CACHE PATH "Colobot libraries directory")
    set(COLOBOT_INSTALL_DATA_DIR ${CMAKE_INSTALL_PREFIX}/share/games/colobot CACHE PATH "Colobot shared data directory")
    set(COLOBOT_INSTALL_I18N_DIR ${CMAKE_INSTALL_PREFIX}/share/locale CACHE PATH "Colobot translations directory")
    set(COLOBOT_INSTALL_DOC_DIR ${CMAKE_INSTALL_PREFIX}/share/doc/colobot CACHE PATH "Colobot documentation directory")
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

# Subdirectory with sources
add_subdirectory(src)

add_subdirectory(po)

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
