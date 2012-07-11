cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE debug)
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -O0")

include_directories(. ../../..)
add_executable(modelfile_test modelfile_test.cpp ../modelfile.cpp ../../../common/stringutils.cpp ../../../common/iman.cpp)
