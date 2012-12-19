cmake_minimum_required(VERSION 2.8)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE debug)
endif(NOT CMAKE_BUILD_TYPE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wold-style-cast -std=gnu++0x")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

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
${GTEST_INCLUDE_DIR}
)

add_executable(modelfile_test ${MODELFILE_TEST_SOURCES})

target_link_libraries(modelfile_test gtest)

add_test(modelfile_test modelfile_test)
