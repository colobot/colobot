cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE debug)
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -O0 -std=c++11 -rdynamic")

add_executable(plugin_test plugin_test.cpp ../../common/iman.cpp ../../common/logger.cpp ../pluginloader.cpp)

include_directories("../../")

target_link_libraries(plugin_test ltdl)
