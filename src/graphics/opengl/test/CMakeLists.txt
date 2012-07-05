cmake_minimum_required(VERSION 2.8)

find_package(OpenGL REQUIRED)
find_package(SDL REQUIRED)
find_package(SDL_image REQUIRED)
find_package(PNG REQUIRED)

set(CMAKE_BUILD_TYPE debug)
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -O0")


set(SOURCES
../gldevice.cpp
../../common/device.cpp
../../../common/logger.cpp
../../../common/image.cpp
texture_test.cpp
)

include_directories(../../../)

set(LIBS
${SDL_LIBRARY}
${SDLIMAGE_LIBRARY}
${OPENGL_LIBRARY}
${PNG_LIBRARIES}
)

add_executable(texture_test ${SOURCES})

target_link_libraries(texture_test ${LIBS})
