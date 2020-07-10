cmake_minimum_required(VERSION 3.17)

add_library(localename STATIC localename.c)
target_include_directories(localename PUBLIC ${CMAKE_CURRENT_LIST_DIR})
