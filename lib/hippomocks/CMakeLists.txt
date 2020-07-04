cmake_minimum_required(VERSION 3.13)

add_library(hippomocks INTERFACE)
target_include_directories(hippomocks SYSTEM INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
