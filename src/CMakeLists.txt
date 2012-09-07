# CBot shared library is built separately
add_subdirectory(CBot)


# Configure options
option(DEBUG "Enable debug output" ON)

set(PLATFORM_LIBS "")

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(PLATFORM_WINDOWS 1)
  set(PLATFORM_LINUX   0)
  set(PLATFORM_OTHER   0)
  # On Windows, GLEW is required
  if (${USE_GLEW} MATCHES "auto")
    set(USE_GLEW 1)
  endif()
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(PLATFORM_WINDOWS 0)
  set(PLATFORM_LINUX   1)
  set(PLATFORM_OTHER   0)
  # On Linux, we should be fine without GLEW
  if (${USE_GLEW} MATCHES "auto")
    set(USE_GLEW 0)
  endif()
  # for clock_gettime
  set(PLATFORM_LIBS "-lrt")
else()
  set(PLATFORM_WINDOWS 0)
  set(PLATFORM_LINUX   0)
  set(PLATFORM_OTHER   1)
  # Use GLEW to be safe
  if (${USE_GLEW} MATCHES "auto")
    set(USE_GLEW 1)
  endif()
endif()

set(OPTIONAL_LIBS "")
set(OPTIONAL_INCLUDE_DIRS "")

if(${USE_GLEW} EQUAL 1)
  find_package(GLEW REQUIRED)
  set(OPTIONAL_LIBS ${OPTIONAL_LIBS} ${GLEW_LIBRARY})
  set(OPTIONAL_INCLUDE_DIRS ${OPTIONAL_INCLUDE_DIRS} ${GLEW_INCLUDE_PATH})
endif()


# Configure file
configure_file(common/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/common/config.h)


# Source files
# Commented out files are still dependent on DirectX or WinAPI

set(SOURCES
app/app.cpp
app/main.cpp
app/system.cpp
common/event.cpp
common/image.cpp
common/logger.cpp
common/iman.cpp
# common/metafile.cpp
# common/misc.cpp
# common/modfile.cpp
# common/profile.cpp
# common/restext.cpp
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
# object/auto/auto.cpp
# object/auto/autobase.cpp
# object/auto/autoconvert.cpp
# object/auto/autoderrick.cpp
# object/auto/autodestroyer.cpp
# object/auto/autoegg.cpp
# object/auto/autoenergy.cpp
# object/auto/autofactory.cpp
# object/auto/autoflag.cpp
# object/auto/autohuston.cpp
# object/auto/autoinfo.cpp
# object/auto/autojostle.cpp
# object/auto/autokid.cpp
# object/auto/autolabo.cpp
# object/auto/automush.cpp
# object/auto/autonest.cpp
# object/auto/autonuclear.cpp
# object/auto/autopara.cpp
# object/auto/autoportico.cpp
# object/auto/autoradar.cpp
# object/auto/autorepair.cpp
# object/auto/autoresearch.cpp
# object/auto/autoroot.cpp
# object/auto/autosafe.cpp
# object/auto/autostation.cpp
# object/auto/autotower.cpp
# object/brain.cpp
# object/mainmovie.cpp
# object/motion/motion.cpp
# object/motion/motionant.cpp
# object/motion/motionbee.cpp
# object/motion/motionhuman.cpp
# object/motion/motionmother.cpp
# object/motion/motionspider.cpp
# object/motion/motiontoto.cpp
# object/motion/motionvehicle.cpp
# object/motion/motionworm.cpp
object/object.cpp
# object/robotmain.cpp
# object/task/task.cpp
# object/task/taskadvance.cpp
# object/task/taskbuild.cpp
# object/task/taskfire.cpp
# object/task/taskfireant.cpp
# object/task/taskflag.cpp
# object/task/taskgoto.cpp
# object/task/taskgungoal.cpp
# object/task/taskinfo.cpp
# object/task/taskmanager.cpp
# object/task/taskmanip.cpp
# object/task/taskpen.cpp
# object/task/taskrecover.cpp
# object/task/taskreset.cpp
# object/task/tasksearch.cpp
# object/task/taskshield.cpp
# object/task/taskspiderexplo.cpp
# object/task/tasktake.cpp
# object/task/taskterraform.cpp
# object/task/taskturn.cpp
# object/task/taskwait.cpp
physics/physics.cpp
script/cbottoken.cpp
script/cmdtoken.cpp
script/script.cpp
# sound/sound.cpp
# ui/button.cpp
# ui/check.cpp
# ui/color.cpp
# ui/compass.cpp
# ui/control.cpp
# ui/displayinfo.cpp
# ui/displaytext.cpp
# ui/edit.cpp
# ui/editvalue.cpp
# ui/gauge.cpp
# ui/group.cpp
# ui/image.cpp
# ui/interface.cpp
# ui/key.cpp
# ui/label.cpp
# ui/list.cpp
# ui/maindialog.cpp
# ui/mainmap.cpp
# ui/mainshort.cpp
# ui/map.cpp
# ui/scroll.cpp
# ui/shortcut.cpp
# ui/slider.cpp
# ui/studio.cpp
# ui/target.cpp
# ui/window.cpp
)

set(LIBS
${SDL_LIBRARY}
${SDLIMAGE_LIBRARY}
${SDLTTF_LIBRARY}
${OPENGL_LIBRARY}
${PNG_LIBRARIES}
${OPTIONAL_LIBS}
${PLATFORM_LIBS}
CBot
)

include_directories(. ${CMAKE_CURRENT_BINARY_DIR}
${SDL_INCLUDE_DIR}
${SDL_IMAGE_INCLUDE_DIR}
${SDLTTF_INCLUDE_DIR}
${PNG_INCLUDE_DIRS}
${OPTIONAL_INCLUDE_DIRS}
..
)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/CBot)

add_executable(colobot ${SOURCES})

target_link_libraries(colobot ${LIBS})
