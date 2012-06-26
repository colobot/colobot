# CBot shared library
add_subdirectory(CBot)

set(SOURCES
autobase.cpp
autoconvert.cpp
auto.cpp
autoderrick.cpp
autodestroyer.cpp
autoegg.cpp
autoenergy.cpp
autofactory.cpp
autoflag.cpp
autohuston.cpp
autoinfo.cpp
autojostle.cpp
autokid.cpp
autolabo.cpp
automush.cpp
autonest.cpp
autonuclear.cpp
autopara.cpp
autoportico.cpp
autoradar.cpp
autorepair.cpp
autoresearch.cpp
autoroot.cpp
autosafe.cpp
autostation.cpp
autotower.cpp
blitz.cpp
brain.cpp
button.cpp
camera.cpp
cbottoken.cpp
check.cpp
cloud.cpp
cmdtoken.cpp
color.cpp
compass.cpp
control.cpp
d3dapp.cpp
d3dengine.cpp
d3denum.cpp
d3dframe.cpp
d3dmath.cpp
d3dtextr.cpp
d3dutil.cpp
displayinfo.cpp
displaytext.cpp
edit.cpp
editvalue.cpp
event.cpp
gauge.cpp
group.cpp
image.cpp
iman.cpp
interface.cpp
joystick.cpp
key.cpp
label.cpp
light.cpp
list.cpp
maindialog.cpp
mainmap.cpp
mainmovie.cpp
mainshort.cpp
map.cpp
math3d.cpp
metafile.cpp
misc.cpp
model.cpp
modfile.cpp
motionant.cpp
motionbee.cpp
motion.cpp
motionhuman.cpp
motionmother.cpp
motionspider.cpp
motiontoto.cpp
motionvehicle.cpp
motionworm.cpp
object.cpp
particule.cpp
physics.cpp
planet.cpp
profile.cpp
pyro.cpp
restext.cpp
robotmain.cpp
script.cpp
scroll.cpp
shortcut.cpp
slider.cpp
sound.cpp
studio.cpp
target.cpp
taskadvance.cpp
taskbuild.cpp
task.cpp
taskfireant.cpp
taskfire.cpp
taskflag.cpp
taskgoto.cpp
taskgungoal.cpp
taskinfo.cpp
taskmanager.cpp
taskmanip.cpp
taskpen.cpp
taskrecover.cpp
taskreset.cpp
tasksearch.cpp
taskshield.cpp
taskspiderexplo.cpp
tasktake.cpp
taskterraform.cpp
taskturn.cpp
taskwait.cpp
terrain.cpp
text.cpp
water.cpp
window.cpp
)

# Change to DirectX SDK directory
set(DXSDK_DIR "c:/dev/dxsdk")

include_directories(${DXSDK_DIR}/include)
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

