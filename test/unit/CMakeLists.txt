set(SRC_DIR ${colobot_SOURCE_DIR}/src)

# Additional libraries per platform
if (${MXE}) # MXE requires special treatment
    set(PLATFORM_LIBS ${MXE_LIBS})
elseif (${PLATFORM_WINDOWS})
    # because it isn't included in standard linking libraries
    set(PLATFORM_LIBS "-lintl")
elseif(${PLATFORM_LINUX})
    # for clock_gettime
    set(PLATFORM_LIBS "-lrt")
endif()


# Configure file
configure_file(${SRC_DIR}/common/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/common/config.h)

# Platform-dependent implementation of system.h
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(SYSTEM_CPP_MODULE "system_windows.cpp")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(SYSTEM_CPP_MODULE "system_linux.cpp")
else()
    set(SYSTEM_CPP_MODULE "system_other.cpp")
endif()

# Code sources
set(COLOBOT_SOURCES
${SRC_DIR}/app/app.cpp
${SRC_DIR}/app/system.cpp
${SRC_DIR}/app/${SYSTEM_CPP_MODULE}
${SRC_DIR}/common/event.cpp
${SRC_DIR}/common/image.cpp
${SRC_DIR}/common/iman.cpp
${SRC_DIR}/common/logger.cpp
${SRC_DIR}/common/misc.cpp
${SRC_DIR}/common/profile.cpp
${SRC_DIR}/common/restext.cpp
${SRC_DIR}/common/stringutils.cpp
${SRC_DIR}/graphics/core/color.cpp
${SRC_DIR}/graphics/engine/camera.cpp
${SRC_DIR}/graphics/engine/cloud.cpp
${SRC_DIR}/graphics/engine/engine.cpp
${SRC_DIR}/graphics/engine/lightman.cpp
${SRC_DIR}/graphics/engine/lightning.cpp
${SRC_DIR}/graphics/engine/modelfile.cpp
${SRC_DIR}/graphics/engine/modelmanager.cpp
${SRC_DIR}/graphics/engine/particle.cpp
${SRC_DIR}/graphics/engine/planet.cpp
${SRC_DIR}/graphics/engine/pyro.cpp
${SRC_DIR}/graphics/engine/terrain.cpp
${SRC_DIR}/graphics/engine/text.cpp
${SRC_DIR}/graphics/engine/water.cpp
${SRC_DIR}/graphics/opengl/gldevice.cpp
${SRC_DIR}/object/auto/auto.cpp
${SRC_DIR}/object/auto/autobase.cpp
${SRC_DIR}/object/auto/autoconvert.cpp
${SRC_DIR}/object/auto/autoderrick.cpp
${SRC_DIR}/object/auto/autodestroyer.cpp
${SRC_DIR}/object/auto/autoegg.cpp
${SRC_DIR}/object/auto/autoenergy.cpp
${SRC_DIR}/object/auto/autofactory.cpp
${SRC_DIR}/object/auto/autoflag.cpp
${SRC_DIR}/object/auto/autohuston.cpp
${SRC_DIR}/object/auto/autoinfo.cpp
${SRC_DIR}/object/auto/autojostle.cpp
${SRC_DIR}/object/auto/autokid.cpp
${SRC_DIR}/object/auto/autolabo.cpp
${SRC_DIR}/object/auto/automush.cpp
${SRC_DIR}/object/auto/autonest.cpp
${SRC_DIR}/object/auto/autonuclear.cpp
${SRC_DIR}/object/auto/autopara.cpp
${SRC_DIR}/object/auto/autoportico.cpp
${SRC_DIR}/object/auto/autoradar.cpp
${SRC_DIR}/object/auto/autorepair.cpp
${SRC_DIR}/object/auto/autoresearch.cpp
${SRC_DIR}/object/auto/autoroot.cpp
${SRC_DIR}/object/auto/autosafe.cpp
${SRC_DIR}/object/auto/autostation.cpp
${SRC_DIR}/object/auto/autotower.cpp
${SRC_DIR}/object/brain.cpp
${SRC_DIR}/object/mainmovie.cpp
${SRC_DIR}/object/motion/motion.cpp
${SRC_DIR}/object/motion/motionant.cpp
${SRC_DIR}/object/motion/motionbee.cpp
${SRC_DIR}/object/motion/motionhuman.cpp
${SRC_DIR}/object/motion/motionmother.cpp
${SRC_DIR}/object/motion/motionspider.cpp
${SRC_DIR}/object/motion/motiontoto.cpp
${SRC_DIR}/object/motion/motionvehicle.cpp
${SRC_DIR}/object/motion/motionworm.cpp
${SRC_DIR}/object/object.cpp
${SRC_DIR}/object/robotmain.cpp
${SRC_DIR}/object/task/task.cpp
${SRC_DIR}/object/task/taskadvance.cpp
${SRC_DIR}/object/task/taskbuild.cpp
${SRC_DIR}/object/task/taskfire.cpp
${SRC_DIR}/object/task/taskfireant.cpp
${SRC_DIR}/object/task/taskflag.cpp
${SRC_DIR}/object/task/taskgoto.cpp
${SRC_DIR}/object/task/taskgungoal.cpp
${SRC_DIR}/object/task/taskinfo.cpp
${SRC_DIR}/object/task/taskmanager.cpp
${SRC_DIR}/object/task/taskmanip.cpp
${SRC_DIR}/object/task/taskpen.cpp
${SRC_DIR}/object/task/taskrecover.cpp
${SRC_DIR}/object/task/taskreset.cpp
${SRC_DIR}/object/task/tasksearch.cpp
${SRC_DIR}/object/task/taskshield.cpp
${SRC_DIR}/object/task/taskspiderexplo.cpp
${SRC_DIR}/object/task/tasktake.cpp
${SRC_DIR}/object/task/taskterraform.cpp
${SRC_DIR}/object/task/taskturn.cpp
${SRC_DIR}/object/task/taskwait.cpp
${SRC_DIR}/physics/physics.cpp
${SRC_DIR}/script/cbottoken.cpp
${SRC_DIR}/script/cmdtoken.cpp
${SRC_DIR}/script/script.cpp
${SRC_DIR}/ui/button.cpp
${SRC_DIR}/ui/check.cpp
${SRC_DIR}/ui/color.cpp
${SRC_DIR}/ui/compass.cpp
${SRC_DIR}/ui/control.cpp
${SRC_DIR}/ui/displayinfo.cpp
${SRC_DIR}/ui/displaytext.cpp
${SRC_DIR}/ui/edit.cpp
${SRC_DIR}/ui/editvalue.cpp
${SRC_DIR}/ui/gauge.cpp
${SRC_DIR}/ui/group.cpp
${SRC_DIR}/ui/image.cpp
${SRC_DIR}/ui/interface.cpp
${SRC_DIR}/ui/key.cpp
${SRC_DIR}/ui/label.cpp
${SRC_DIR}/ui/list.cpp
${SRC_DIR}/ui/maindialog.cpp
${SRC_DIR}/ui/mainmap.cpp
${SRC_DIR}/ui/mainshort.cpp
${SRC_DIR}/ui/map.cpp
${SRC_DIR}/ui/scroll.cpp
${SRC_DIR}/ui/shortcut.cpp
${SRC_DIR}/ui/slider.cpp
${SRC_DIR}/ui/studio.cpp
${SRC_DIR}/ui/target.cpp
${SRC_DIR}/ui/window.cpp
)

set(OPENAL_SOURCES "")

if (${OPENAL_SOUND})
    set(OPENAL_SOURCES
        ${SRC_DIR}/sound/oalsound/alsound.cpp
        ${SRC_DIR}/sound/oalsound/buffer.cpp
        ${SRC_DIR}/sound/oalsound/channel.cpp
    )
endif()

# Optional libraries
set(OPTIONAL_LIBS "")

if (${OPENAL_SOUND})
    if (${PLATFORM_WINDOWS})
        set(OPTIONAL_LIBS
            OpenAL32
        )
    else()
        set(OPTIONAL_LIBS
            openal
        )
    endif()
endif()


# Platform-dependent tests
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    #TODO: set(PLATFORM_TESTS app/system_windows_test.cpp)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(PLATFORM_TESTS app/system_linux_test.cpp)
else()
    #TODO: set(PLATFORM_TESTS app/system_other_test.cpp)
endif()

# Tests
set(UT_SOURCES
main.cpp
graphics/engine/lightman_test.cpp
math/geometry_test.cpp
math/matrix_test.cpp
math/vector_test.cpp
${PLATFORM_TESTS}
)

include_directories(
${CMAKE_CURRENT_BINARY_DIR}
${SRC_DIR}
${GTEST_INCLUDE_DIR}
${GMOCK_INCLUDE_DIR}
.
common
math
)

set(LIBS
gtest
gmock
CBot
${SDL_LIBRARY}
${SDLIMAGE_LIBRARY}
${SDLTTF_LIBRARY}
${OPENGL_LIBRARY}
${PNG_LIBRARIES}
${GLEW_LIBRARY}
${Boost_LIBRARIES}
${OPTIONAL_LIBS}
${PLATFORM_LIBS}
${LIBSNDFILE_LIBRARY}
)

add_executable(colobot_ut ${COLOBOT_SOURCES} ${UT_SOURCES} ${OPENAL_SOURCES})
target_link_libraries(colobot_ut ${LIBS})

add_test(colobot_ut ./colobot_ut)

# TODO: change the unit cases to independent automated tests to be included in colobot_ut
add_subdirectory(common)
add_subdirectory(ui)
