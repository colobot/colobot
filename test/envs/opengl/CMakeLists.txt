set(SRC_DIR ${colobot_SOURCE_DIR}/src)

configure_file(${SRC_DIR}/common/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/common/config.h)


set(TEXTURE_SOURCES
${SRC_DIR}/graphics/opengl/gldevice.cpp
${SRC_DIR}/common/logger.cpp
${SRC_DIR}/common/image.cpp
texture_test.cpp
)

set(MODEL_SOURCES
${SRC_DIR}/graphics/opengl/gldevice.cpp
${SRC_DIR}/graphics/engine/modelfile.cpp
${SRC_DIR}/common/logger.cpp
${SRC_DIR}/common/image.cpp
${SRC_DIR}/common/iman.cpp
${SRC_DIR}/common/stringutils.cpp
${SRC_DIR}/app/system.cpp
model_test.cpp
)

set(TRANSFORM_SOURCES
${SRC_DIR}/graphics/opengl/gldevice.cpp
${SRC_DIR}/common/logger.cpp
${SRC_DIR}/common/image.cpp
${SRC_DIR}/common/iman.cpp
${SRC_DIR}/app/system.cpp
transform_test.cpp
)

set(LIGHT_SOURCES
${SRC_DIR}/graphics/opengl/gldevice.cpp
${SRC_DIR}/common/logger.cpp
${SRC_DIR}/common/image.cpp
${SRC_DIR}/common/iman.cpp
${SRC_DIR}/app/system.cpp
light_test.cpp
)

include_directories(${SRC_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set(LIBS
${SDL_LIBRARY}
${SDLIMAGE_LIBRARY}
${OPENGL_LIBRARY}
${GLEW_LIBRARY}
${PNG_LIBRARIES}
${ADD_LIBS}
)

add_executable(texture_test ${TEXTURE_SOURCES})
target_link_libraries(texture_test ${LIBS})

add_executable(model_test ${MODEL_SOURCES})
target_link_libraries(model_test ${LIBS})

add_executable(transform_test ${TRANSFORM_SOURCES})
target_link_libraries(transform_test ${LIBS})

add_executable(light_test ${LIGHT_SOURCES})
target_link_libraries(light_test ${LIBS})
