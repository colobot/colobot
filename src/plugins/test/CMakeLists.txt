cmake_minimum_required(VERSION 2.8)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE debug)
endif(NOT CMAKE_BUILD_TYPE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -rdynamic")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

add_executable(manager_test manager_test.cpp ../../common/logger.cpp ../../common/profile.cpp ../../common/iman.cpp ../pluginmanager.cpp ../pluginloader.cpp)

include_directories(".")
include_directories("../../")
include_directories("../../../")

target_link_libraries(manager_test ${LTDL_LIBRARY})
