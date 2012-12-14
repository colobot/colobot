cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE debug)
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -O0 -std=c++11 -rdynamic")

add_executable(manager_test manager_test.cpp ../../common/logger.cpp ../../common/profile.cpp ../../common/iman.cpp ../pluginmanager.cpp ../pluginloader.cpp)

include_directories(".")
include_directories("../../")
include_directories("../../../")

target_link_libraries(manager_test ${LTDL_LIBRARY})
