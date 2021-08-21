# Compile flags as defined in global CMakeLists
set(CMAKE_CXX_FLAGS "${COLOBOT_CXX_FLAGS} ${MXE_CFLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE ${COLOBOT_CXX_FLAGS_RELEASE})
set(CMAKE_CXX_FLAGS_DEBUG ${COLOBOT_CXX_FLAGS_DEBUG})

# Groups source files for IDE project
macro(group_sources SOURCES GROUP_ROOT)
    foreach(SOURCE_FILE ${SOURCES})
        get_filename_component(GROUP "${SOURCE_FILE}" PATH)
        string(REPLACE "/" "\\" GROUP "${GROUP}")
        set(GROUP "${GROUP_ROOT}\\${GROUP}")
        source_group("${GROUP}" FILES "${SOURCE_FILE}")
    endforeach()
endmacro()

# Subdirectories

add_subdirectory(CBot)

if(TOOLS)
    add_subdirectory(tools)
endif()

add_subdirectory(graphics/opengl/shaders)


# Optional libraries
set(OPTIONAL_LIBS "")
set(OPTIONAL_INCLUDES "")

if(OPENAL_SOUND)
    set(OPTIONAL_LIBS ${OPENAL_LIBRARY})
    set(OPTIONAL_INCLUDES ${OPENAL_INCLUDE_DIR})
endif()

if(WINGETOPT)
    set(OPTIONAL_LIBS ${OPTIONAL_LIBS} wingetopt)
    set(OPTIONAL_INCLUDES ${OPTIONAL_INCLUDES} ${WINGETOPT_INCLUDE_DIR})
endif()

# Additional libraries per platform
if(MXE) # MXE requires special treatment
    set(PLATFORM_LIBS ${MXE_LIBS})
elseif(PLATFORM_WINDOWS)
    # because it isn't included in standard linking libraries
    if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        find_library(LIBINTL_LIBRARY NAMES intl.lib libintl)

        if(${MSVC_STATIC})
            if (${OPENAL_SOUND})
                find_library(FLAC_LIBRARY NAMES flac)
                find_library(VORBIS_LIBRARY NAMES vorbis)
                find_library(VORBISENC_LIBRARY NAMES vorbisenc)
                find_library(OGG_LIBRARY NAMES ogg)
                find_library(OPUS_LIBRARY NAMES opus)
                set(OPENAL_MSVC_LIBS
                    ${FLAC_LIBRARY}
                    ${VORBIS_LIBRARY}
                    ${VORBISENC_LIBRARY}
                    ${OGG_LIBRARY}
                    ${OPUS_LIBRARY}
                )
            endif()

            find_library(BZ2_LIBRARY NAMES bz2)
            find_library(JPEG_LIBRARY NAMES jpeg)
            find_library(TIFF_LIBRARY NAMES tiff)
            find_library(LZMA_LIBRARY NAMES lzma)
            find_library(FREETYPE_LIBRARY NAMES freetype)
            find_library(ICONV_LIBRARY NAMES iconv)
            find_library(CHARSET_LIBRARY NAMES charset)
            find_library(BROTLICOMMON_LIBRARY NAMES brotlicommon-static)
            find_library(BROTLIENC_LIBRARY NAMES brotlienc-static)
            find_library(BROTLIDEC_LIBRARY NAMES brotlidec-static)
            set(MSVC_LIBS
                ${LIBINTL_LIBRARY}
                ${OPENAL_MSVC_LIBS}
                ${JPEG_LIBRARY}
                ${TIFF_LIBRARY}
                ${BZ2_LIBRARY}
                ${LZMA_LIBRARY}
                ${FREETYPE_LIBRARY}
                ${ICONV_LIBRARY}
                ${CHARSET_LIBRARY}
                ${BROTLICOMMON_LIBRARY}
                ${BROTLIENC_LIBRARY}
                ${BROTLIDEC_LIBRARY}
                winmm.lib
                dxguid.lib
                imm32.lib
                ole32.lib
                oleaut32.lib
                version.lib
                wsock32.lib
                ws2_32.lib
                setupapi.lib
            )
        else(${MSVC_STATIC})
            set(MSVC_LIBS ${LIBINTL_LIBRARY})
        endif()

        set(PLATFORM_LIBS ${MSVC_LIBS})
    else()
        set(PLATFORM_LIBS "-lintl")
    endif()
elseif(PLATFORM_GNU)
    set(PLATFORM_LIBS "")
elseif(PLATFORM_LINUX)
    # for clock_gettime
    set(PLATFORM_LIBS "-lrt")
elseif(PLATFORM_MACOSX)
    find_library(LIBINTL_LIBRARY NAMES intl libintl)
    find_path(LIBINTL_INCLUDE_PATH NAMES libintl.h)
    set(PLATFORM_LIBS ${LIBINTL_LIBRARY})
elseif(PLATFORM_FREEBSD)
    find_library(LIBINTL_LIBRARY NAMES intl libintl)
    find_path(LIBINTL_INCLUDE_PATH NAMES libintl.h)
    set(PLATFORM_LIBS ${LIBINTL_LIBRARY})
endif()


# Configure file
configure_file(common/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/common/config.h)
configure_file(common/version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/common/version.h)

set(OPENAL_SRC "")

if(OPENAL_SOUND)
    set(OPENAL_SRC
        sound/oalsound/alsound.cpp
        sound/oalsound/buffer.cpp
        sound/oalsound/channel.cpp
        sound/oalsound/check.cpp
        sound/oalsound/alsound.h
        sound/oalsound/buffer.h
        sound/oalsound/channel.h
        sound/oalsound/check.h
    )
endif()

set(RES_FILES "")

if(PLATFORM_WINDOWS)
    set(RES_FILES "../desktop/colobot.rc")
endif()

# Source files
set(BASE_SOURCES
    ${OPENAL_SRC}
    app/app.cpp
    app/app.h
    app/controller.cpp
    app/controller.h
    app/input.cpp
    app/input.h
    app/modman.cpp
    app/modman.h
    app/pathman.cpp
    app/pathman.h
    app/pausemanager.cpp
    app/pausemanager.h
    app/signal_handlers.cpp
    app/signal_handlers.h
    common/config_file.cpp
    common/config_file.h
    common/error.h
    common/event.cpp
    common/event.h
    common/font_loader.h
    common/font_loader.cpp
    common/global.h
    common/image.cpp
    common/image.h
    common/ioutils.h
    common/key.cpp
    common/key.h
    common/language.cpp
    common/language.h
    common/logger.cpp
    common/logger.h
    common/make_unique.h
    common/profiler.cpp
    common/profiler.h
    common/regex_utils.cpp
    common/regex_utils.h
    common/resources/inputstream.cpp
    common/resources/inputstream.h
    common/resources/inputstreambuffer.cpp
    common/resources/inputstreambuffer.h
    common/resources/outputstream.cpp
    common/resources/outputstream.h
    common/resources/outputstreambuffer.cpp
    common/resources/outputstreambuffer.h
    common/resources/resourcemanager.cpp
    common/resources/resourcemanager.h
    common/resources/sdl_file_wrapper.cpp
    common/resources/sdl_file_wrapper.h
    common/resources/sdl_memory_wrapper.cpp
    common/resources/sdl_memory_wrapper.h
    common/resources/sndfile_wrapper.cpp
    common/resources/sndfile_wrapper.h
    common/restext.cpp
    common/restext.h
    common/system/system.cpp
    common/system/system.h
    common/system/system_other.cpp
    common/system/system_other.h
    common/system/${SYSTEM_CPP_MODULE}
    common/system/${SYSTEM_H_MODULE}
    common/settings.cpp
    common/settings.h
    common/singleton.h
    common/stringutils.cpp
    common/stringutils.h
    common/thread/resource_owning_thread.h
    common/thread/sdl_cond_wrapper.h
    common/thread/sdl_mutex_wrapper.h
    common/thread/thread.h
    common/thread/worker_thread.h
    graphics/core/color.cpp
    graphics/core/color.h
    graphics/core/device.h
    graphics/core/framebuffer.cpp
    graphics/core/framebuffer.h
    graphics/core/light.h
    graphics/core/material.h
    graphics/core/nulldevice.cpp
    graphics/core/nulldevice.h
    graphics/core/texture.h
    graphics/core/type.cpp
    graphics/core/type.h
    graphics/core/vertex.h
    graphics/engine/camera.cpp
    graphics/engine/camera.h
    graphics/engine/cloud.cpp
    graphics/engine/cloud.h
    graphics/engine/engine.cpp
    graphics/engine/engine.h
    graphics/engine/lightman.cpp
    graphics/engine/lightman.h
    graphics/engine/lightning.cpp
    graphics/engine/lightning.h
    graphics/engine/oldmodelmanager.cpp
    graphics/engine/oldmodelmanager.h
    graphics/engine/particle.cpp
    graphics/engine/particle.h
    graphics/engine/planet.cpp
    graphics/engine/planet.h
    graphics/engine/pyro.cpp
    graphics/engine/pyro.h
    graphics/engine/pyro_manager.cpp
    graphics/engine/pyro_manager.h
    graphics/engine/pyro_type.h
    graphics/engine/terrain.cpp
    graphics/engine/terrain.h
    graphics/engine/text.cpp
    graphics/engine/text.h
    graphics/engine/water.cpp
    graphics/engine/water.h
    graphics/model/model.cpp
    graphics/model/model.h
    graphics/model/model_crash_sphere.h
    graphics/model/model_format.h
    graphics/model/model_input.cpp
    graphics/model/model_input.h
    graphics/model/model_io_exception.h
    graphics/model/model_io_structs.h
    graphics/model/model_manager.cpp
    graphics/model/model_manager.h
    graphics/model/model_mesh.cpp
    graphics/model/model_mesh.h
    graphics/model/model_output.cpp
    graphics/model/model_output.h
    graphics/model/model_shadow_spot.h
    graphics/model/model_triangle.h
    graphics/opengl/gl14device.cpp
    graphics/opengl/gl14device.h
    graphics/opengl/gl21device.cpp
    graphics/opengl/gl21device.h
    graphics/opengl/gl33device.cpp
    graphics/opengl/gl33device.h
    graphics/opengl/glframebuffer.cpp
    graphics/opengl/glframebuffer.h
    graphics/opengl/glutil.cpp
    graphics/opengl/glutil.h
    level/build_type.h
    level/level_category.cpp
    level/level_category.h
    level/mainmovie.cpp
    level/mainmovie.h
    level/parser/parser.cpp
    level/parser/parser.h
    level/parser/parserexceptions.cpp
    level/parser/parserexceptions.h
    level/parser/parserline.cpp
    level/parser/parserline.h
    level/parser/parserparam.cpp
    level/parser/parserparam.h
    level/player_profile.cpp
    level/player_profile.h
    level/research_type.h
    level/robotmain.cpp
    level/robotmain.h
    level/scene_conditions.cpp
    level/scene_conditions.h
    level/scoreboard.cpp
    level/scoreboard.h
    math/all.h
    math/const.h
    math/func.h
    math/geometry.h
    math/half.cpp
    math/half.h
    math/intpoint.h
    math/matrix.h
    math/point.h
    math/sphere.h
    math/vector.h
    object/auto/auto.cpp
    object/auto/auto.h
    object/auto/autobase.cpp
    object/auto/autobase.h
    object/auto/autoconvert.cpp
    object/auto/autoconvert.h
    object/auto/autoderrick.cpp
    object/auto/autoderrick.h
    object/auto/autodestroyer.cpp
    object/auto/autodestroyer.h
    object/auto/autoegg.cpp
    object/auto/autoegg.h
    object/auto/autofactory.cpp
    object/auto/autofactory.h
    object/auto/autoflag.cpp
    object/auto/autoflag.h
    object/auto/autohouston.cpp
    object/auto/autohouston.h
    object/auto/autojostle.cpp
    object/auto/autojostle.h
    object/auto/autokid.cpp
    object/auto/autokid.h
    object/auto/autolabo.cpp
    object/auto/autolabo.h
    object/auto/automush.cpp
    object/auto/automush.h
    object/auto/autonest.cpp
    object/auto/autonest.h
    object/auto/autonuclearplant.cpp
    object/auto/autonuclearplant.h
    object/auto/autoportico.cpp
    object/auto/autoportico.h
    object/auto/autopowercaptor.cpp
    object/auto/autopowercaptor.h
    object/auto/autopowerplant.cpp
    object/auto/autopowerplant.h
    object/auto/autopowerstation.cpp
    object/auto/autopowerstation.h
    object/auto/autoradar.cpp
    object/auto/autoradar.h
    object/auto/autorepair.cpp
    object/auto/autorepair.h
    object/auto/autoresearch.cpp
    object/auto/autoresearch.h
    object/auto/autoroot.cpp
    object/auto/autoroot.h
    object/auto/autotower.cpp
    object/auto/autotower.h
    object/auto/autovault.cpp
    object/auto/autovault.h
    object/crash_sphere.h
    object/drive_type.cpp
    object/drive_type.h
    object/implementation/power_container_impl.cpp
    object/implementation/power_container_impl.h
    object/implementation/program_storage_impl.cpp
    object/implementation/program_storage_impl.h
    object/implementation/programmable_impl.cpp
    object/implementation/programmable_impl.h
    object/implementation/task_executor_impl.cpp
    object/implementation/task_executor_impl.h
    object/interface/carrier_object.h
    object/interface/controllable_object.h
    object/interface/damageable_object.h
    object/interface/destroyable_object.h
    object/interface/flying_object.h
    object/interface/fragile_object.h
    object/interface/interactive_object.h
    object/interface/jet_flying_object.h
    object/interface/jostleable_object.h
    object/interface/movable_object.h
    object/interface/power_container_object.h
    object/interface/powered_object.h
    object/interface/program_storage_object.h
    object/interface/programmable_object.h
    object/interface/ranged_object.h
    object/interface/shielded_auto_regen_object.h
    object/interface/shielded_object.h
    object/interface/task_executor_object.h
    object/interface/trace_drawing_object.cpp
    object/interface/trace_drawing_object.h
    object/interface/transportable_object.h
    object/mission_type.h
    object/motion/motion.cpp
    object/motion/motion.h
    object/motion/motionant.cpp
    object/motion/motionant.h
    object/motion/motionbee.cpp
    object/motion/motionbee.h
    object/motion/motionhuman.cpp
    object/motion/motionhuman.h
    object/motion/motionlevelcontroller.cpp
    object/motion/motionlevelcontroller.h
    object/motion/motionqueen.cpp
    object/motion/motionqueen.h
    object/motion/motionspider.cpp
    object/motion/motionspider.h
    object/motion/motiontoto.cpp
    object/motion/motiontoto.h
    object/motion/motionvehicle.cpp
    object/motion/motionvehicle.h
    object/motion/motionworm.cpp
    object/motion/motionworm.h
    object/object.cpp
    object/object.h
    object/object_create_exception.h
    object/object_create_params.h
    object/object_factory.cpp
    object/object_factory.h
    object/object_interface_type.h
    object/object_manager.cpp
    object/object_manager.h
    object/object_type.cpp
    object/object_type.h
    object/old_object.cpp
    object/old_object.h
    object/old_object_interface.cpp
    object/old_object_interface.h
    object/subclass/base_alien.cpp
    object/subclass/base_alien.h
    object/subclass/base_building.cpp
    object/subclass/base_building.h
    object/subclass/base_robot.cpp
    object/subclass/base_robot.h
    object/subclass/base_vehicle.cpp
    object/subclass/base_vehicle.h
    object/subclass/exchange_post.cpp
    object/subclass/exchange_post.h
    object/subclass/shielder.cpp
    object/subclass/shielder.h
    object/subclass/static_object.cpp
    object/subclass/static_object.h
    object/task/task.cpp
    object/task/task.h
    object/task/taskadvance.cpp
    object/task/taskadvance.h
    object/task/taskbuild.cpp
    object/task/taskbuild.h
    object/task/taskdeletemark.cpp
    object/task/taskdeletemark.h
    object/task/taskfire.cpp
    object/task/taskfire.h
    object/task/taskfireant.cpp
    object/task/taskfireant.h
    object/task/taskflag.cpp
    object/task/taskflag.h
    object/task/taskgoto.cpp
    object/task/taskgoto.h
    object/task/taskgungoal.cpp
    object/task/taskgungoal.h
    object/task/taskinfo.cpp
    object/task/taskinfo.h
    object/task/taskmanip.cpp
    object/task/taskmanip.h
    object/task/taskpen.cpp
    object/task/taskpen.h
    object/task/taskrecover.cpp
    object/task/taskrecover.h
    object/task/tasksearch.cpp
    object/task/tasksearch.h
    object/task/taskshield.cpp
    object/task/taskshield.h
    object/task/taskspiderexplo.cpp
    object/task/taskspiderexplo.h
    object/task/tasktake.cpp
    object/task/tasktake.h
    object/task/taskterraform.cpp
    object/task/taskterraform.h
    object/task/taskturn.cpp
    object/task/taskturn.h
    object/task/taskwait.cpp
    object/task/taskwait.h
    object/tool_type.cpp
    object/tool_type.h
    physics/physics.cpp
    physics/physics.h
    script/cbottoken.cpp
    script/cbottoken.h
    script/script.cpp
    script/script.h
    script/scriptfunc.cpp
    script/scriptfunc.h
    sound/sound.cpp
    sound/sound.h
    sound/sound_type.cpp
    sound/sound_type.h
    ui/controls/button.cpp
    ui/controls/button.h
    ui/controls/check.cpp
    ui/controls/check.h
    ui/controls/color.cpp
    ui/controls/color.h
    ui/controls/control.cpp
    ui/controls/control.h
    ui/controls/edit.cpp
    ui/controls/edit.h
    ui/controls/editvalue.cpp
    ui/controls/editvalue.h
    ui/controls/enumslider.cpp
    ui/controls/enumslider.h
    ui/controls/gauge.cpp
    ui/controls/gauge.h
    ui/controls/group.cpp
    ui/controls/group.h
    ui/controls/image.cpp
    ui/controls/image.h
    ui/controls/interface.cpp
    ui/controls/interface.h
    ui/controls/key.cpp
    ui/controls/key.h
    ui/controls/label.cpp
    ui/controls/label.h
    ui/controls/list.cpp
    ui/controls/list.h
    ui/controls/map.cpp
    ui/controls/map.h
    ui/controls/scroll.cpp
    ui/controls/scroll.h
    ui/controls/shortcut.cpp
    ui/controls/shortcut.h
    ui/controls/slider.cpp
    ui/controls/slider.h
    ui/controls/target.cpp
    ui/controls/target.h
    ui/controls/window.cpp
    ui/controls/window.h
    ui/debug_menu.cpp
    ui/debug_menu.h
    ui/displayinfo.cpp
    ui/displayinfo.h
    ui/displaytext.cpp
    ui/displaytext.h
    ui/filedialog.cpp
    ui/filedialog.h
    ui/maindialog.cpp
    ui/maindialog.h
    ui/mainmap.cpp
    ui/mainmap.h
    ui/mainshort.cpp
    ui/mainshort.h
    ui/mainui.cpp
    ui/mainui.h
    ui/object_interface.cpp
    ui/object_interface.h
    ui/particles_generator.cpp
    ui/particles_generator.h
    ui/screen/screen.cpp
    ui/screen/screen.h
    ui/screen/screen_apperance.cpp
    ui/screen/screen_apperance.h
    ui/screen/screen_io.cpp
    ui/screen/screen_io.h
    ui/screen/screen_io_read.cpp
    ui/screen/screen_io_read.h
    ui/screen/screen_io_write.cpp
    ui/screen/screen_io_write.h
    ui/screen/screen_level_list.cpp
    ui/screen/screen_level_list.h
    ui/screen/screen_loading.cpp
    ui/screen/screen_loading.h
    ui/screen/screen_main_menu.cpp
    ui/screen/screen_main_menu.h
    ui/screen/screen_mod_list.cpp
    ui/screen/screen_mod_list.h
    ui/screen/screen_player_select.cpp
    ui/screen/screen_player_select.h
    ui/screen/screen_quit.cpp
    ui/screen/screen_quit.h
    ui/screen/screen_setup.cpp
    ui/screen/screen_setup.h
    ui/screen/screen_setup_controls.cpp
    ui/screen/screen_setup_controls.h
    ui/screen/screen_setup_display.cpp
    ui/screen/screen_setup_display.h
    ui/screen/screen_setup_game.cpp
    ui/screen/screen_setup_game.h
    ui/screen/screen_setup_graphics.cpp
    ui/screen/screen_setup_graphics.h
    ui/screen/screen_setup_sound.cpp
    ui/screen/screen_setup_sound.h
    ui/screen/screen_welcome.cpp
    ui/screen/screen_welcome.h
    ui/studio.cpp
    ui/studio.h
)

set(MAIN_SOURCES
    app/main.cpp
    ${RES_FILES}
)

# Libraries
set(LIBS
    CBot
    localename
    ${SDL2_LIBRARY}
    ${SDL2_IMAGE_LIBRARY}
    ${SDL2_TTF_LIBRARY}
    ${OPENGL_LIBRARY}
    ${PNG_LIBRARIES}
    ${GLEW_LIBRARY}
    ${Boost_LIBRARIES}
    ${LIBSNDFILE_LIBRARY}
    ${OPTIONAL_LIBS}
    ${PLATFORM_LIBS}
    ${PHYSFS_LIBRARY}
)

set(COLOBOT_LIBS ${LIBS} PARENT_SCOPE)


# Includes
set(LOCAL_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(SYSTEM_INCLUDES
    ${SDL2_INCLUDE_DIR}
    ${SDL2_IMAGE_INCLUDE_DIR}
    ${SDL2_TTF_INCLUDE_DIR}
    ${PNG_INCLUDE_DIRS}
    ${GLEW_INCLUDE_PATH}
    ${Boost_INCLUDE_DIRS}
    ${LIBSNDFILE_INCLUDE_DIR}
    ${LOCALENAME_INCLUDE_DIR}
    ${PHYSFS_INCLUDE_PATH}
    ${LIBINTL_INCLUDE_PATH}
    ${OPTIONAL_INCLUDES}
)

set(COLOBOT_LOCAL_INCLUDES ${LOCAL_INCLUDES} PARENT_SCOPE)
set(COLOBOT_SYSTEM_INCLUDES ${SYSTEM_INCLUDES} PARENT_SCOPE)

include_directories(${LOCAL_INCLUDES})
include_directories(SYSTEM ${SYSTEM_INCLUDES})


# Link directories
link_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/CBot
)


# Targets

add_library(colobotbase STATIC ${BASE_SOURCES})

add_executable(colobot ${MAIN_SOURCES})
target_link_libraries(colobot colobotbase ${LIBS})

install(TARGETS colobot RUNTIME DESTINATION ${COLOBOT_INSTALL_BIN_DIR})
if(NOT CBOT_STATIC)
    set_target_properties(colobot PROPERTIES INSTALL_RPATH ${COLOBOT_INSTALL_LIB_DIR})
endif()

if(COLOBOT_LINT_BUILD)
    add_fake_header_sources("src")
endif()

group_sources("${BASE_SOURCES}" "Source Files")
