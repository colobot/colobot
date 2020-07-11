cmake_minimum_required(VERSION 3.17)

add_library(hippomocks INTERFACE)
target_include_directories(hippomocks INTERFACE ${CMAKE_CURRENT_LIST_DIR})
