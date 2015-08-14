cmake_minimum_required(VERSION 2.8)

include_directories(. include)

add_definitions(-DGTEST_HAS_TR1_TUPLE=0)

# gtest-all.cc includes all other sources
add_library(gtest STATIC src/gtest-all.cc)
