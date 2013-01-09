cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${${ORIGINAL_CXX_FLAGS}}")

include_directories(. include ${GTEST_INCLUDE_DIR})

# gmock-all.cc includes all other sources
add_library(gmock STATIC src/gmock-all.cc)
