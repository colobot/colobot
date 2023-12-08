target_sources(Colobot-Base PRIVATE
    gl33_device.cpp
    gl33_device.h
    gl33_object_renderer.cpp
    gl33_object_renderer.h
    gl33_particle_renderer.cpp
    gl33_particle_renderer.h
    gl33_terrain_renderer.cpp
    gl33_terrain_renderer.h
    gl33_shadow_renderer.cpp
    gl33_shadow_renderer.h
    gl33_ui_renderer.cpp
    gl33_ui_renderer.h
    glframebuffer.cpp
    glframebuffer.h
    glutil.cpp
    glutil.h
)

if(COLOBOT_DEVELOPMENT_MODE)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders/gl33
        DESTINATION ${PROJECT_BINARY_DIR}/data/shaders
    )
endif()

install(DIRECTORY shaders/gl33 DESTINATION ${COLOBOT_INSTALL_DATA_DIR}/shaders)
