cmake_minimum_required(VERSION 2.8)

set(SOURCES
    alsound.cpp
    buffer.cpp
    channel.cpp
    ../../../common/logger.cpp
)

SET (CMAKE_CXX_FLAGS "-Wall  -g -std=c++0x")

include(FindPkgConfig)
include(FindOpenAL)
pkg_check_modules(OPENAL_LIB REQUIRED openal)

set(OPENAL_LIBRARIES
    openal
    alut
)


include_directories(../../..)
include_directories(.)
add_library(openalsound SHARED ${SOURCES})
target_link_libraries(openalsound ${OPENAL_LIBRARIES})
