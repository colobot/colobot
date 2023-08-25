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

add_custom_target(CopyShaders ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/gl33
        ${CMAKE_SOURCE_DIR}/data/shaders/gl33
)

install(DIRECTORY shaders/gl33 DESTINATION ${COLOBOT_INSTALL_DATA_DIR}/shaders)
