cmake_minimum_required(VERSION 2.8)

add_library(localename STATIC localename.c)
target_include_directories(localename PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
