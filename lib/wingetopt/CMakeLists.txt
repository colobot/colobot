cmake_minimum_required(VERSION 3.17)

add_library(wingetopt STATIC src/getopt.c src/getopt.h)
target_include_directories(wingetopt PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src)
