cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE debug)
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -O0 -std=c++11")

include_directories("../../")
include_directories("../../../")

#add_executable(image_test ../image.cpp image_test.cpp)
add_executable(profile_test ../profile.cpp profile_test.cpp)

add_test(profile_test ./profile_test)
