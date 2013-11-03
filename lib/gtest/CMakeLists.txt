cmake_minimum_required(VERSION 2.8)

include_directories(. include)

# gtest-all.cc includes all other sources
add_library(gtest STATIC src/gtest-all.cc)
