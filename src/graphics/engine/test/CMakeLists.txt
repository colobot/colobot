cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wold-style-cast -std=gnu++0x")

set(MODELFILE_TEST_SOURCES
modelfile_test.cpp
../modelfile.cpp
../../../common/logger.cpp
../../../common/stringutils.cpp
../../../common/iman.cpp
)

add_definitions(-DMODELFILE_NO_ENGINE)

include_directories(
.
../../..
${GTEST_DIR}/include
)

add_executable(modelfile_test ${MODELFILE_TEST_SOURCES})

target_link_libraries(modelfile_test gtest)

add_test(modelfile_test modelfile_test)
