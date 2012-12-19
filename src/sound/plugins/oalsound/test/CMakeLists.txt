cmake_minimum_required(VERSION 2.8)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE debug)
endif(NOT CMAKE_BUILD_TYPE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -rdynamic")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

add_executable(plugin_test plugin_test.cpp ../../../../common/iman.cpp ../../../../common/logger.cpp ../../../../plugins/pluginloader.cpp)

include_directories(".")
include_directories("../../../../")

target_link_libraries(plugin_test ${LTDL_LIBRARY})
