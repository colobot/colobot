# CBot library is built separately
add_subdirectory(CBot)

# Tools directory is built separately
add_subdirectory(tools)

add_subdirectory(po)

# Tests
if(${TESTS})
    add_subdirectory(common/test)
    add_subdirectory(graphics/engine/test)
    add_subdirectory(ui/test)
    add_subdirectory(math/test)
endif()


# Optional libraries
set(OPTIONAL_LIBS "")
set(OPTIONAL_INCLUDE_DIRS "")

if(${USE_GLEW} AND NOT ${MXE})
    set(OPTIONAL_LIBS ${OPTIONAL_LIBS} ${GLEW_LIBRARY})
    set(OPTIONAL_INCLUDE_DIRS ${OPTIONAL_INCLUDE_DIRS} ${GLEW_INCLUDE_PATH})
endif()


# Additional libraries per platform
set(PLATFORM_LIBS "")
set(OPENAL_LIBS "")

if (${OPENAL_SOUND})
    if (${MXE})
	set(OPENAL_LIBS
	    ${CMAKE_FIND_ROOT_PATH}/lib/libOpenAL32.a
	    ${CMAKE_FIND_ROOT_PATH}/lib/libalut.a
	)
    else()
	set(OPENAL_LIBS
	    openal
	    alut
	)
    endif()
endif()

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
configure_file(common/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/common/config.h)

set(OPENAL_SRC "")

if (${OPENAL_SOUND})
    set(OPENAL_SRC
	sound/oalsound/alsound.cpp
	sound/oalsound/buffer.cpp
	sound/oalsound/channel.cpp
    )
endif()

# Source files
set(SOURCES
app/app.cpp
app/main.cpp
app/system.cpp
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
${SDL_LIBRARY}
${SDLIMAGE_LIBRARY}
${SDLTTF_LIBRARY}
${OPENGL_LIBRARY}
${PNG_LIBRARIES}
${OPTIONAL_LIBS}
${PLATFORM_LIBS}
${Boost_LIBRARIES}
CBot
${OPENAL_LIBS}
)

include_directories(
.
..
${CMAKE_CURRENT_BINARY_DIR}
${SDL_INCLUDE_DIR}
${SDLIMAGE_INCLUDE_DIR}
${SDLTTF_INCLUDE_DIR}
${PNG_INCLUDE_DIRS}
${OPTIONAL_INCLUDE_DIRS}
${Boost_INCLUDE_DIRS}
)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/CBot)

add_executable(colobot ${SOURCES})

target_link_libraries(colobot ${LIBS})

install(TARGETS colobot RUNTIME DESTINATION ${COLOBOT_INSTALL_BIN_DIR})
set_target_properties(colobot PROPERTIES INSTALL_RPATH ${COLOBOT_INSTALL_LIB_DIR})
