cmake_minimum_required(VERSION 2.8)

find_package(OpenGL REQUIRED)
find_package(SDL REQUIRED)
find_package(SDL_image REQUIRED)
find_package(PNG REQUIRED)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE debug)
endif(NOT CMAKE_BUILD_TYPE)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

set(ADD_LIBS "")

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(PLATFORM_WINDOWS 1)
  set(PLATFORM_LINUX   0)
  set(PLATFORM_OTHER   0)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(PLATFORM_WINDOWS 0)
  set(PLATFORM_LINUX   1)
  set(PLATFORM_OTHER   0)
  set(ADD_LIBS "-lrt")
else()
  set(PLATFORM_WINDOWS 0)
  set(PLATFORM_LINUX   0)
  set(PLATFORM_OTHER   1)
endif()

configure_file(../../../common/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/common/config.h)


set(TEXTURE_SOURCES
../gldevice.cpp
../../../common/logger.cpp
../../../common/image.cpp
texture_test.cpp
)

set(MODEL_SOURCES
../gldevice.cpp
../../engine/modelfile.cpp
../../../common/logger.cpp
../../../common/image.cpp
../../../common/iman.cpp
../../../common/stringutils.cpp
../../../app/system.cpp
model_test.cpp
)

set(TRANSFORM_SOURCES
../gldevice.cpp
../../../common/logger.cpp
../../../common/image.cpp
../../../common/iman.cpp
../../../app/system.cpp
transform_test.cpp
)

set(LIGHT_SOURCES
../gldevice.cpp
../../../common/logger.cpp
../../../common/image.cpp
../../../common/iman.cpp
../../../app/system.cpp
light_test.cpp
)

include_directories(../../../ ${CMAKE_CURRENT_BINARY_DIR})

set(LIBS
${SDL_LIBRARY}
${SDLIMAGE_LIBRARY}
${OPENGL_LIBRARY}
${PNG_LIBRARIES}
${ADD_LIBS}
)

add_executable(texture_test ${TEXTURE_SOURCES})
target_link_libraries(texture_test ${LIBS})

# Temporarily disabling because of dependencies on CEngine et al.
#add_executable(model_test ${MODEL_SOURCES})
#target_link_libraries(model_test ${LIBS})

add_executable(transform_test ${TRANSFORM_SOURCES})
target_link_libraries(transform_test ${LIBS})

add_executable(light_test ${LIGHT_SOURCES})
target_link_libraries(light_test ${LIBS})
