cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE debug)
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -O0")

add_executable(matrix_test matrix_test.cpp)
add_executable(vector_test vector_test.cpp)

enable_testing()

add_test(matrix_test ./matrix_test)
add_test(vector_test ./vector_test)

# 'make check' will compile the required test programs
# Note that 'make test' will still fail without compiled programs
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS matrix_test vector_test)

# Files to be removed in distclean
set(REMOVE_FILES
 CMakeFiles Testing cmake_install.cmake CMakeCache.txt CTestTestfile.cmake Makefile
 ./matrix_test
 ./vector_test
)

add_custom_target(distclean COMMAND rm -rf ${REMOVE_FILES})