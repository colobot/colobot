# CBot shared library
add_subdirectory(CBot)

set(SOURCES
old/d3dapp.cpp
old/joystick.cpp
old/blitz.cpp
old/camera.cpp
old/cloud.cpp
old/light.cpp
old/model.cpp
old/particule.cpp
old/planet.cpp
old/pyro.cpp
old/terrain.cpp
old/text.cpp
old/water.cpp
old/d3dengine.cpp
old/d3denum.cpp
old/d3dframe.cpp
old/d3dtextr.cpp
old/d3dutil.cpp
old/d3dmath.cpp
old/math3d.cpp
app/app.cpp
common/event.cpp
common/iman.cpp
common/metafile.cpp
common/misc.cpp
common/modfile.cpp
common/profile.cpp
common/restext.cpp
graphics/common/camera.cpp
graphics/common/cloud.cpp
graphics/common/color.cpp
graphics/common/engine.cpp
graphics/common/light.cpp
graphics/common/lightning.cpp
graphics/common/model.cpp
graphics/common/particle.cpp
graphics/common/planet.cpp
graphics/common/pyro.cpp
graphics/common/terrain.cpp
graphics/common/text.cpp
graphics/common/water.cpp
graphics/opengl/gldevice.cpp
graphics/opengl/glengine.cpp
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
sound/sound.cpp
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
)

add_definitions(-DSTRICT -DD3D_OVERLOADS)

# Change to DirectX SDK directory
set(DXSDK_DIR "c:/dxsdk")

include_directories(${DXSDK_DIR}/include .)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/CBot ${DXSDK_DIR}/lib)

set( LIBS -lkernel32 -luser32 -lgdi32 -lwinspool -lcomdlg32
    -ladvapi32 -lshell32 -lole32 -loleaut32 -luuid -lodbc32
    -lodbccp32 -lwinmm
    ${DXSDK_DIR}/lib/ddraw.lib ${DXSDK_DIR}/lib/dinput.lib ${DXSDK_DIR}/lib/dxguid.lib ${DXSDK_DIR}/lib/d3d8.lib ${DXSDK_DIR}/lib/dsound.lib )

# To build with libwine:
#  include_directories(/usr/include/wine/windows /usr/include/wine/msvcrt)
#  set(LIBS -lwine)

add_executable(colobot ${SOURCES})

target_link_libraries(colobot CBot ${LIBS})

