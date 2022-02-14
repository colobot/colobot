cmake_minimum_required(VERSION 3.16)

add_library(wingetopt STATIC src/getopt.c src/getopt.h)
target_include_directories(wingetopt SYSTEM PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src)
