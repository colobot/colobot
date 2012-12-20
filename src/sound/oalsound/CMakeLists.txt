cmake_minimum_required(VERSION 2.8)

set(SOURCES
    alsound.cpp
    buffer.cpp
    channel.cpp
)

SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++0x -fPIC")
SET (CMAKE_CXX_FLAGS_DEBUG "-g")

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
