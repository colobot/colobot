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


add_executable(image_test ../image.cpp image_test.cpp)
target_link_libraries(image_test ${SDL_LIBRARY} ${SDLIMAGE_LIBRARY} ${PNG_LIBRARIES})

#add_executable(profile_test ../profile.cpp ../logger.cpp profile_test.cpp)
#target_link_libraries(profile_test gtest ${Boost_LIBRARIES})

#add_test(profile_test ./profile_test)
