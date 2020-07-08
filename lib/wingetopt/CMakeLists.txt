cmake_minimum_required(VERSION 3.17)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
add_library(wingetopt STATIC src/getopt.c src/getopt.h)

