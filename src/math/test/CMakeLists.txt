cmake_minimum_required(VERSION 2.8)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE debug)
endif(NOT CMAKE_BUILD_TYPE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wold-style-cast -std=gnu++0x")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

include_directories(
.
../..
../../..
${GTEST_INCLUDE_DIR}
)

add_executable(matrix_test matrix_test.cpp)
target_link_libraries(matrix_test gtest)

add_executable(vector_test vector_test.cpp)
target_link_libraries(vector_test gtest)

add_executable(geometry_test geometry_test.cpp)
target_link_libraries(geometry_test gtest)

add_test(matrix_test matrix_test)
add_test(vector_test vector_test)
add_test(geometry_test geometry_test)
