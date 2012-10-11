cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wold-style-cast -std=gnu++0x")

include_directories(
.
../..
../../..
${GTEST_DIR}/include
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
