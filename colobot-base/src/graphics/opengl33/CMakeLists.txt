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
    set(SHADERS
        lighting.glsl
        object_fs.glsl
        object_vs.glsl
        particle_fs.glsl
        particle_vs.glsl
        preamble.glsl
        shadow_fs.glsl
        shadow_vs.glsl
        shadow.glsl
        terrain_fs.glsl
        terrain_vs.glsl
        ui_fs.glsl
        ui_vs.glsl
    )

    add_custom_target(UpdateShaders ALL)

    foreach(shader ${SHADERS})
        add_custom_command(
            TARGET UpdateShaders POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/shaders/gl33/${shader}"
                "${PROJECT_BINARY_DIR}/data/shaders/gl33/${shader}"
        )
    endforeach()
endif()

install(DIRECTORY shaders/gl33 DESTINATION ${COLOBOT_INSTALL_DATA_DIR}/shaders)
