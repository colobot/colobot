cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE debug)
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -O0")

add_executable(matrix_test matrix_test.cpp)

enable_testing()

add_test(matrix_test ./matrix_test)

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS matrix_test)

add_custom_target(distclean COMMAND rm -rf ./matrix_test CMakeFiles Testing cmake_install.cmake CMakeCache.txt CTestTestfile.cmake Makefile)