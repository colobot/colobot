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
${GMOCK_INCLUDE_DIR}
)


add_executable(edit_test 
    ../../common/event.cpp
    ../../common/logger.cpp
    ../../common/misc.cpp
    ../../common/iman.cpp
    ../../common/stringutils.cpp
    ../../graphics/engine/text.cpp
    ../button.cpp
    ../control.cpp
    ../edit.cpp
    ../scroll.cpp
    stubs/app_stub.cpp
    stubs/engine_stub.cpp
    stubs/particle_stub.cpp
    stubs/restext_stub.cpp
    stubs/robotmain_stub.cpp
    edit_test.cpp)
target_link_libraries(edit_test gtest gmock ${SDL_LIBRARY} ${SDLTTF_LIBRARY})

add_test(edit_test ./edit_test)
