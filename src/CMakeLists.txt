# Compile flags as defined in global CMakeLists
set(CMAKE_CXX_FLAGS "${COLOBOT_CXX_FLAGS} ${MXE_CFLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE ${COLOBOT_CXX_FLAGS_RELEASE})
set(CMAKE_CXX_FLAGS_DEBUG ${COLOBOT_CXX_FLAGS_DEBUG})


# Subdirectories

add_subdirectory(CBot)

add_subdirectory(tools)

add_subdirectory(po)

if(${DESKTOP})
    add_subdirectory(desktop)
endif()


# Optional libraries
set(OPTIONAL_LIBS "")
set(OPTIONAL_INCLUDES "")

if (${OPENAL_SOUND})
    set(OPTIONAL_LIBS ${OPENAL_LIBRARY})
    set(OPTIONAL_INCLUDES ${OPENAL_INCLUDE_DIR})
endif()

# Additional libraries per platform
if (${MXE}) # MXE requires special treatment
    set(PLATFORM_LIBS ${MXE_LIBS})  
elseif (${PLATFORM_WINDOWS})
    # because it isn't included in standard linking libraries
    set(PLATFORM_LIBS "-lintl")
elseif(${PLATFORM_LINUX})
    # for clock_gettime
    set(PLATFORM_LIBS "-lrt -lX11")
endif()


# Configure file
configure_file(common/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/common/config.h)

set(OPENAL_SRC "")

if (${OPENAL_SOUND})
    set(OPENAL_SRC
        sound/oalsound/alsound.cpp
        sound/oalsound/buffer.cpp
        sound/oalsound/channel.cpp
    )
endif()

# Platform-dependent implementation of system.h
if (${PLATFORM_WINDOWS})
    set(SYSTEM_CPP_MODULE "system_windows.cpp")
elseif(${PLATFORM_LINUX})
    set(SYSTEM_CPP_MODULE "system_linux.cpp")
else()
    set(SYSTEM_CPP_MODULE "system_other.cpp")
endif()

# Source files
set(SOURCES
app/app.cpp
app/main.cpp
app/system.cpp
app/${SYSTEM_CPP_MODULE}
common/event.cpp
common/image.cpp
common/iman.cpp
common/logger.cpp
common/misc.cpp
common/profile.cpp
common/restext.cpp
common/stringutils.cpp
graphics/core/color.cpp
graphics/engine/camera.cpp
graphics/engine/cloud.cpp
graphics/engine/engine.cpp
graphics/engine/lightman.cpp
graphics/engine/lightning.cpp
graphics/engine/modelfile.cpp
graphics/engine/modelmanager.cpp
graphics/engine/particle.cpp
graphics/engine/planet.cpp
graphics/engine/pyro.cpp
graphics/engine/terrain.cpp
graphics/engine/text.cpp
graphics/engine/water.cpp
graphics/opengl/gldevice.cpp
object/auto/auto.cpp
object/auto/autobase.cpp
object/auto/autoconvert.cpp
object/auto/autoderrick.cpp
object/auto/autodestroyer.cpp
object/auto/autoegg.cpp
object/auto/autoenergy.cpp
object/auto/autofactory.cpp
object/auto/autoflag.cpp
object/auto/autohuston.cpp
object/auto/autoinfo.cpp
object/auto/autojostle.cpp
object/auto/autokid.cpp
object/auto/autolabo.cpp
object/auto/automush.cpp
object/auto/autonest.cpp
object/auto/autonuclear.cpp
object/auto/autopara.cpp
object/auto/autoportico.cpp
object/auto/autoradar.cpp
object/auto/autorepair.cpp
object/auto/autoresearch.cpp
object/auto/autoroot.cpp
object/auto/autosafe.cpp
object/auto/autostation.cpp
object/auto/autotower.cpp
object/brain.cpp
object/mainmovie.cpp
object/motion/motion.cpp
object/motion/motionant.cpp
object/motion/motionbee.cpp
object/motion/motionhuman.cpp
object/motion/motionmother.cpp
object/motion/motionspider.cpp
object/motion/motiontoto.cpp
object/motion/motionvehicle.cpp
object/motion/motionworm.cpp
object/object.cpp
object/robotmain.cpp
object/objman.cpp
object/task/task.cpp
object/task/taskadvance.cpp
object/task/taskbuild.cpp
object/task/taskfire.cpp
object/task/taskfireant.cpp
object/task/taskflag.cpp
object/task/taskgoto.cpp
object/task/taskgungoal.cpp
object/task/taskinfo.cpp
object/task/taskmanager.cpp
object/task/taskmanip.cpp
object/task/taskpen.cpp
object/task/taskrecover.cpp
object/task/taskreset.cpp
object/task/tasksearch.cpp
object/task/taskshield.cpp
object/task/taskspiderexplo.cpp
object/task/tasktake.cpp
object/task/taskterraform.cpp
object/task/taskturn.cpp
object/task/taskwait.cpp
physics/physics.cpp
script/cbottoken.cpp
script/cmdtoken.cpp
script/script.cpp
ui/button.cpp
ui/check.cpp
ui/color.cpp
ui/compass.cpp
ui/control.cpp
ui/displayinfo.cpp
ui/displaytext.cpp
ui/edit.cpp
ui/editvalue.cpp
ui/gauge.cpp
ui/group.cpp
ui/image.cpp
ui/interface.cpp
ui/key.cpp
ui/label.cpp
ui/list.cpp
ui/maindialog.cpp
ui/mainmap.cpp
ui/mainshort.cpp
ui/map.cpp
ui/scroll.cpp
ui/shortcut.cpp
ui/slider.cpp
ui/studio.cpp
ui/target.cpp
ui/window.cpp
${OPENAL_SRC}
)


set(LIBS
CBot
clipboard
${SDL_LIBRARY}
${SDLIMAGE_LIBRARY}
${SDLTTF_LIBRARY}
${OPENGL_LIBRARY}
${PNG_LIBRARIES}
${GLEW_LIBRARY}
${Boost_LIBRARIES}
${LIBSNDFILE_LIBRARY}
${OPTIONAL_LIBS}
${PLATFORM_LIBS}
)

# Local
include_directories(
.
..
${CMAKE_CURRENT_BINARY_DIR}
)

# System
include_directories(
SYSTEM
${SDL_INCLUDE_DIR}
${SDLIMAGE_INCLUDE_DIR}
${SDLTTF_INCLUDE_DIR}
${PNG_INCLUDE_DIRS}
${GLEW_INCLUDE_PATH}
${Boost_INCLUDE_DIRS}
${LIBSNDFILE_INCLUDE_DIR}
${OPTIONAL_INCLUDE_DIRS}
${CLIPBOARD_DIR}
)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/CBot)

add_executable(colobot ${SOURCES})

target_link_libraries(colobot ${LIBS})

install(TARGETS colobot RUNTIME DESTINATION ${COLOBOT_INSTALL_BIN_DIR})
set_target_properties(colobot PROPERTIES INSTALL_RPATH ${COLOBOT_INSTALL_LIB_DIR})
