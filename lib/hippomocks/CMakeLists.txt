cmake_minimum_required(VERSION 3.16)

add_library(hippomocks INTERFACE)
target_include_directories(hippomocks SYSTEM INTERFACE ${CMAKE_CURRENT_LIST_DIR})
