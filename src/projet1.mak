# Microsoft Developer Studio Generated NMAKE File, Based on projet1.dsp
!IF "$(CFG)" == ""
CFG=projet1 - Win32 Debug
!MESSAGE No configuration specified. Defaulting to projet1 - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "projet1 - Win32 Release" && "$(CFG)" !=\
 "projet1 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "projet1.mak" CFG="projet1 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "projet1 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "projet1 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "projet1 - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\projet1.exe"

!ELSE 

ALL : "$(OUTDIR)\projet1.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\auto.obj"
	-@erase "$(INTDIR)\autobase.obj"
	-@erase "$(INTDIR)\autoconvert.obj"
	-@erase "$(INTDIR)\autoderrick.obj"
	-@erase "$(INTDIR)\autodestroyer.obj"
	-@erase "$(INTDIR)\autoegg.obj"
	-@erase "$(INTDIR)\autoenergy.obj"
	-@erase "$(INTDIR)\autofactory.obj"
	-@erase "$(INTDIR)\autoflag.obj"
	-@erase "$(INTDIR)\autohuston.obj"
	-@erase "$(INTDIR)\autoinfo.obj"
	-@erase "$(INTDIR)\autojostle.obj"
	-@erase "$(INTDIR)\autokid.obj"
	-@erase "$(INTDIR)\autolabo.obj"
	-@erase "$(INTDIR)\automush.obj"
	-@erase "$(INTDIR)\autonest.obj"
	-@erase "$(INTDIR)\autonuclear.obj"
	-@erase "$(INTDIR)\autopara.obj"
	-@erase "$(INTDIR)\autoportico.obj"
	-@erase "$(INTDIR)\autoradar.obj"
	-@erase "$(INTDIR)\autorepair.obj"
	-@erase "$(INTDIR)\autoresearch.obj"
	-@erase "$(INTDIR)\autoroot.obj"
	-@erase "$(INTDIR)\autosafe.obj"
	-@erase "$(INTDIR)\autostation.obj"
	-@erase "$(INTDIR)\autotower.obj"
	-@erase "$(INTDIR)\blitz.obj"
	-@erase "$(INTDIR)\brain.obj"
	-@erase "$(INTDIR)\button.obj"
	-@erase "$(INTDIR)\camera.obj"
	-@erase "$(INTDIR)\cbottoken.obj"
	-@erase "$(INTDIR)\check.obj"
	-@erase "$(INTDIR)\cloud.obj"
	-@erase "$(INTDIR)\cmdtoken.obj"
	-@erase "$(INTDIR)\color.obj"
	-@erase "$(INTDIR)\compass.obj"
	-@erase "$(INTDIR)\control.obj"
	-@erase "$(INTDIR)\d3dapp.obj"
	-@erase "$(INTDIR)\d3dengine.obj"
	-@erase "$(INTDIR)\d3denum.obj"
	-@erase "$(INTDIR)\d3dframe.obj"
	-@erase "$(INTDIR)\d3dmath.obj"
	-@erase "$(INTDIR)\d3dtextr.obj"
	-@erase "$(INTDIR)\d3dutil.obj"
	-@erase "$(INTDIR)\displayinfo.obj"
	-@erase "$(INTDIR)\displaytext.obj"
	-@erase "$(INTDIR)\edit.obj"
	-@erase "$(INTDIR)\editvalue.obj"
	-@erase "$(INTDIR)\event.obj"
	-@erase "$(INTDIR)\gauge.obj"
	-@erase "$(INTDIR)\group.obj"
	-@erase "$(INTDIR)\image.obj"
	-@erase "$(INTDIR)\iman.obj"
	-@erase "$(INTDIR)\interface.obj"
	-@erase "$(INTDIR)\joystick.obj"
	-@erase "$(INTDIR)\key.obj"
	-@erase "$(INTDIR)\label.obj"
	-@erase "$(INTDIR)\light.obj"
	-@erase "$(INTDIR)\list.obj"
	-@erase "$(INTDIR)\maindialog.obj"
	-@erase "$(INTDIR)\mainmap.obj"
	-@erase "$(INTDIR)\mainmovie.obj"
	-@erase "$(INTDIR)\mainshort.obj"
	-@erase "$(INTDIR)\map.obj"
	-@erase "$(INTDIR)\math3d.obj"
	-@erase "$(INTDIR)\metafile.obj"
	-@erase "$(INTDIR)\misc.obj"
	-@erase "$(INTDIR)\model.obj"
	-@erase "$(INTDIR)\modfile.obj"
	-@erase "$(INTDIR)\motion.obj"
	-@erase "$(INTDIR)\motionant.obj"
	-@erase "$(INTDIR)\motionbee.obj"
	-@erase "$(INTDIR)\motionhuman.obj"
	-@erase "$(INTDIR)\motionmother.obj"
	-@erase "$(INTDIR)\motionspider.obj"
	-@erase "$(INTDIR)\motiontoto.obj"
	-@erase "$(INTDIR)\motionvehicle.obj"
	-@erase "$(INTDIR)\motionworm.obj"
	-@erase "$(INTDIR)\object.obj"
	-@erase "$(INTDIR)\particule.obj"
	-@erase "$(INTDIR)\physics.obj"
	-@erase "$(INTDIR)\planet.obj"
	-@erase "$(INTDIR)\profile.obj"
	-@erase "$(INTDIR)\pyro.obj"
	-@erase "$(INTDIR)\restext.obj"
	-@erase "$(INTDIR)\robotmain.obj"
	-@erase "$(INTDIR)\script.obj"
	-@erase "$(INTDIR)\scroll.obj"
	-@erase "$(INTDIR)\shortcut.obj"
	-@erase "$(INTDIR)\slider.obj"
	-@erase "$(INTDIR)\sound.obj"
	-@erase "$(INTDIR)\studio.obj"
	-@erase "$(INTDIR)\target.obj"
	-@erase "$(INTDIR)\task.obj"
	-@erase "$(INTDIR)\taskadvance.obj"
	-@erase "$(INTDIR)\taskbuild.obj"
	-@erase "$(INTDIR)\taskfire.obj"
	-@erase "$(INTDIR)\taskfireant.obj"
	-@erase "$(INTDIR)\taskflag.obj"
	-@erase "$(INTDIR)\taskgoto.obj"
	-@erase "$(INTDIR)\taskgungoal.obj"
	-@erase "$(INTDIR)\taskinfo.obj"
	-@erase "$(INTDIR)\taskmanager.obj"
	-@erase "$(INTDIR)\taskmanip.obj"
	-@erase "$(INTDIR)\taskpen.obj"
	-@erase "$(INTDIR)\taskrecover.obj"
	-@erase "$(INTDIR)\taskreset.obj"
	-@erase "$(INTDIR)\tasksearch.obj"
	-@erase "$(INTDIR)\taskshield.obj"
	-@erase "$(INTDIR)\taskspiderexplo.obj"
	-@erase "$(INTDIR)\tasktake.obj"
	-@erase "$(INTDIR)\taskterraform.obj"
	-@erase "$(INTDIR)\taskturn.obj"
	-@erase "$(INTDIR)\taskwait.obj"
	-@erase "$(INTDIR)\terrain.obj"
	-@erase "$(INTDIR)\text.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(INTDIR)\water.obj"
	-@erase "$(INTDIR)\window.obj"
	-@erase "$(INTDIR)\winmain.res"
	-@erase "$(OUTDIR)\projet1.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /Zi /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)\projet1.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Release/
CPP_SBRS=.

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o NUL /win32 
RSC=rc.exe
RSC_PROJ=/l 0x100c /fo"$(INTDIR)\winmain.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\projet1.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib winmm.lib c:\dx7sdk\lib\ddraw.lib c:\dx7sdk\lib\dinput.lib\
 c:\dx7sdk\lib\dxguid.lib c:\dx7sdk\lib\d3dx.lib c:\dx7sdk\lib\dsound.lib\
 cbot\cbot.lib /nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)\projet1.pdb" /machine:I386 /out:"$(OUTDIR)\projet1.exe" 
LINK32_OBJS= \
	"$(INTDIR)\auto.obj" \
	"$(INTDIR)\autobase.obj" \
	"$(INTDIR)\autoconvert.obj" \
	"$(INTDIR)\autoderrick.obj" \
	"$(INTDIR)\autodestroyer.obj" \
	"$(INTDIR)\autoegg.obj" \
	"$(INTDIR)\autoenergy.obj" \
	"$(INTDIR)\autofactory.obj" \
	"$(INTDIR)\autoflag.obj" \
	"$(INTDIR)\autohuston.obj" \
	"$(INTDIR)\autoinfo.obj" \
	"$(INTDIR)\autojostle.obj" \
	"$(INTDIR)\autokid.obj" \
	"$(INTDIR)\autolabo.obj" \
	"$(INTDIR)\automush.obj" \
	"$(INTDIR)\autonest.obj" \
	"$(INTDIR)\autonuclear.obj" \
	"$(INTDIR)\autopara.obj" \
	"$(INTDIR)\autoportico.obj" \
	"$(INTDIR)\autoradar.obj" \
	"$(INTDIR)\autorepair.obj" \
	"$(INTDIR)\autoresearch.obj" \
	"$(INTDIR)\autoroot.obj" \
	"$(INTDIR)\autosafe.obj" \
	"$(INTDIR)\autostation.obj" \
	"$(INTDIR)\autotower.obj" \
	"$(INTDIR)\blitz.obj" \
	"$(INTDIR)\brain.obj" \
	"$(INTDIR)\button.obj" \
	"$(INTDIR)\camera.obj" \
	"$(INTDIR)\cbottoken.obj" \
	"$(INTDIR)\check.obj" \
	"$(INTDIR)\cloud.obj" \
	"$(INTDIR)\cmdtoken.obj" \
	"$(INTDIR)\color.obj" \
	"$(INTDIR)\compass.obj" \
	"$(INTDIR)\control.obj" \
	"$(INTDIR)\d3dapp.obj" \
	"$(INTDIR)\d3dengine.obj" \
	"$(INTDIR)\d3denum.obj" \
	"$(INTDIR)\d3dframe.obj" \
	"$(INTDIR)\d3dmath.obj" \
	"$(INTDIR)\d3dtextr.obj" \
	"$(INTDIR)\d3dutil.obj" \
	"$(INTDIR)\displayinfo.obj" \
	"$(INTDIR)\displaytext.obj" \
	"$(INTDIR)\edit.obj" \
	"$(INTDIR)\editvalue.obj" \
	"$(INTDIR)\event.obj" \
	"$(INTDIR)\gauge.obj" \
	"$(INTDIR)\group.obj" \
	"$(INTDIR)\image.obj" \
	"$(INTDIR)\iman.obj" \
	"$(INTDIR)\interface.obj" \
	"$(INTDIR)\joystick.obj" \
	"$(INTDIR)\key.obj" \
	"$(INTDIR)\label.obj" \
	"$(INTDIR)\light.obj" \
	"$(INTDIR)\list.obj" \
	"$(INTDIR)\maindialog.obj" \
	"$(INTDIR)\mainmap.obj" \
	"$(INTDIR)\mainmovie.obj" \
	"$(INTDIR)\mainshort.obj" \
	"$(INTDIR)\map.obj" \
	"$(INTDIR)\math3d.obj" \
	"$(INTDIR)\metafile.obj" \
	"$(INTDIR)\misc.obj" \
	"$(INTDIR)\model.obj" \
	"$(INTDIR)\modfile.obj" \
	"$(INTDIR)\motion.obj" \
	"$(INTDIR)\motionant.obj" \
	"$(INTDIR)\motionbee.obj" \
	"$(INTDIR)\motionhuman.obj" \
	"$(INTDIR)\motionmother.obj" \
	"$(INTDIR)\motionspider.obj" \
	"$(INTDIR)\motiontoto.obj" \
	"$(INTDIR)\motionvehicle.obj" \
	"$(INTDIR)\motionworm.obj" \
	"$(INTDIR)\object.obj" \
	"$(INTDIR)\particule.obj" \
	"$(INTDIR)\physics.obj" \
	"$(INTDIR)\planet.obj" \
	"$(INTDIR)\profile.obj" \
	"$(INTDIR)\pyro.obj" \
	"$(INTDIR)\restext.obj" \
	"$(INTDIR)\robotmain.obj" \
	"$(INTDIR)\script.obj" \
	"$(INTDIR)\scroll.obj" \
	"$(INTDIR)\shortcut.obj" \
	"$(INTDIR)\slider.obj" \
	"$(INTDIR)\sound.obj" \
	"$(INTDIR)\studio.obj" \
	"$(INTDIR)\target.obj" \
	"$(INTDIR)\task.obj" \
	"$(INTDIR)\taskadvance.obj" \
	"$(INTDIR)\taskbuild.obj" \
	"$(INTDIR)\taskfire.obj" \
	"$(INTDIR)\taskfireant.obj" \
	"$(INTDIR)\taskflag.obj" \
	"$(INTDIR)\taskgoto.obj" \
	"$(INTDIR)\taskgungoal.obj" \
	"$(INTDIR)\taskinfo.obj" \
	"$(INTDIR)\taskmanager.obj" \
	"$(INTDIR)\taskmanip.obj" \
	"$(INTDIR)\taskpen.obj" \
	"$(INTDIR)\taskrecover.obj" \
	"$(INTDIR)\taskreset.obj" \
	"$(INTDIR)\tasksearch.obj" \
	"$(INTDIR)\taskshield.obj" \
	"$(INTDIR)\taskspiderexplo.obj" \
	"$(INTDIR)\tasktake.obj" \
	"$(INTDIR)\taskterraform.obj" \
	"$(INTDIR)\taskturn.obj" \
	"$(INTDIR)\taskwait.obj" \
	"$(INTDIR)\terrain.obj" \
	"$(INTDIR)\text.obj" \
	"$(INTDIR)\water.obj" \
	"$(INTDIR)\window.obj" \
	"$(INTDIR)\winmain.res"

"$(OUTDIR)\projet1.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\projet1.exe" "$(OUTDIR)\projet1.bsc"

!ELSE 

ALL : "$(OUTDIR)\projet1.exe" "$(OUTDIR)\projet1.bsc"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\auto.obj"
	-@erase "$(INTDIR)\auto.sbr"
	-@erase "$(INTDIR)\autobase.obj"
	-@erase "$(INTDIR)\autobase.sbr"
	-@erase "$(INTDIR)\autoconvert.obj"
	-@erase "$(INTDIR)\autoconvert.sbr"
	-@erase "$(INTDIR)\autoderrick.obj"
	-@erase "$(INTDIR)\autoderrick.sbr"
	-@erase "$(INTDIR)\autodestroyer.obj"
	-@erase "$(INTDIR)\autodestroyer.sbr"
	-@erase "$(INTDIR)\autoegg.obj"
	-@erase "$(INTDIR)\autoegg.sbr"
	-@erase "$(INTDIR)\autoenergy.obj"
	-@erase "$(INTDIR)\autoenergy.sbr"
	-@erase "$(INTDIR)\autofactory.obj"
	-@erase "$(INTDIR)\autofactory.sbr"
	-@erase "$(INTDIR)\autoflag.obj"
	-@erase "$(INTDIR)\autoflag.sbr"
	-@erase "$(INTDIR)\autohuston.obj"
	-@erase "$(INTDIR)\autohuston.sbr"
	-@erase "$(INTDIR)\autoinfo.obj"
	-@erase "$(INTDIR)\autoinfo.sbr"
	-@erase "$(INTDIR)\autojostle.obj"
	-@erase "$(INTDIR)\autojostle.sbr"
	-@erase "$(INTDIR)\autokid.obj"
	-@erase "$(INTDIR)\autokid.sbr"
	-@erase "$(INTDIR)\autolabo.obj"
	-@erase "$(INTDIR)\autolabo.sbr"
	-@erase "$(INTDIR)\automush.obj"
	-@erase "$(INTDIR)\automush.sbr"
	-@erase "$(INTDIR)\autonest.obj"
	-@erase "$(INTDIR)\autonest.sbr"
	-@erase "$(INTDIR)\autonuclear.obj"
	-@erase "$(INTDIR)\autonuclear.sbr"
	-@erase "$(INTDIR)\autopara.obj"
	-@erase "$(INTDIR)\autopara.sbr"
	-@erase "$(INTDIR)\autoportico.obj"
	-@erase "$(INTDIR)\autoportico.sbr"
	-@erase "$(INTDIR)\autoradar.obj"
	-@erase "$(INTDIR)\autoradar.sbr"
	-@erase "$(INTDIR)\autorepair.obj"
	-@erase "$(INTDIR)\autorepair.sbr"
	-@erase "$(INTDIR)\autoresearch.obj"
	-@erase "$(INTDIR)\autoresearch.sbr"
	-@erase "$(INTDIR)\autoroot.obj"
	-@erase "$(INTDIR)\autoroot.sbr"
	-@erase "$(INTDIR)\autosafe.obj"
	-@erase "$(INTDIR)\autosafe.sbr"
	-@erase "$(INTDIR)\autostation.obj"
	-@erase "$(INTDIR)\autostation.sbr"
	-@erase "$(INTDIR)\autotower.obj"
	-@erase "$(INTDIR)\autotower.sbr"
	-@erase "$(INTDIR)\blitz.obj"
	-@erase "$(INTDIR)\blitz.sbr"
	-@erase "$(INTDIR)\brain.obj"
	-@erase "$(INTDIR)\brain.sbr"
	-@erase "$(INTDIR)\button.obj"
	-@erase "$(INTDIR)\button.sbr"
	-@erase "$(INTDIR)\camera.obj"
	-@erase "$(INTDIR)\camera.sbr"
	-@erase "$(INTDIR)\cbottoken.obj"
	-@erase "$(INTDIR)\cbottoken.sbr"
	-@erase "$(INTDIR)\check.obj"
	-@erase "$(INTDIR)\check.sbr"
	-@erase "$(INTDIR)\cloud.obj"
	-@erase "$(INTDIR)\cloud.sbr"
	-@erase "$(INTDIR)\cmdtoken.obj"
	-@erase "$(INTDIR)\cmdtoken.sbr"
	-@erase "$(INTDIR)\color.obj"
	-@erase "$(INTDIR)\color.sbr"
	-@erase "$(INTDIR)\compass.obj"
	-@erase "$(INTDIR)\compass.sbr"
	-@erase "$(INTDIR)\control.obj"
	-@erase "$(INTDIR)\control.sbr"
	-@erase "$(INTDIR)\d3dapp.obj"
	-@erase "$(INTDIR)\d3dapp.sbr"
	-@erase "$(INTDIR)\d3dengine.obj"
	-@erase "$(INTDIR)\d3dengine.sbr"
	-@erase "$(INTDIR)\d3denum.obj"
	-@erase "$(INTDIR)\d3denum.sbr"
	-@erase "$(INTDIR)\d3dframe.obj"
	-@erase "$(INTDIR)\d3dframe.sbr"
	-@erase "$(INTDIR)\d3dmath.obj"
	-@erase "$(INTDIR)\d3dmath.sbr"
	-@erase "$(INTDIR)\d3dtextr.obj"
	-@erase "$(INTDIR)\d3dtextr.sbr"
	-@erase "$(INTDIR)\d3dutil.obj"
	-@erase "$(INTDIR)\d3dutil.sbr"
	-@erase "$(INTDIR)\displayinfo.obj"
	-@erase "$(INTDIR)\displayinfo.sbr"
	-@erase "$(INTDIR)\displaytext.obj"
	-@erase "$(INTDIR)\displaytext.sbr"
	-@erase "$(INTDIR)\edit.obj"
	-@erase "$(INTDIR)\edit.sbr"
	-@erase "$(INTDIR)\editvalue.obj"
	-@erase "$(INTDIR)\editvalue.sbr"
	-@erase "$(INTDIR)\event.obj"
	-@erase "$(INTDIR)\event.sbr"
	-@erase "$(INTDIR)\gauge.obj"
	-@erase "$(INTDIR)\gauge.sbr"
	-@erase "$(INTDIR)\group.obj"
	-@erase "$(INTDIR)\group.sbr"
	-@erase "$(INTDIR)\image.obj"
	-@erase "$(INTDIR)\image.sbr"
	-@erase "$(INTDIR)\iman.obj"
	-@erase "$(INTDIR)\iman.sbr"
	-@erase "$(INTDIR)\interface.obj"
	-@erase "$(INTDIR)\interface.sbr"
	-@erase "$(INTDIR)\joystick.obj"
	-@erase "$(INTDIR)\joystick.sbr"
	-@erase "$(INTDIR)\key.obj"
	-@erase "$(INTDIR)\key.sbr"
	-@erase "$(INTDIR)\label.obj"
	-@erase "$(INTDIR)\label.sbr"
	-@erase "$(INTDIR)\light.obj"
	-@erase "$(INTDIR)\light.sbr"
	-@erase "$(INTDIR)\list.obj"
	-@erase "$(INTDIR)\list.sbr"
	-@erase "$(INTDIR)\maindialog.obj"
	-@erase "$(INTDIR)\maindialog.sbr"
	-@erase "$(INTDIR)\mainmap.obj"
	-@erase "$(INTDIR)\mainmap.sbr"
	-@erase "$(INTDIR)\mainmovie.obj"
	-@erase "$(INTDIR)\mainmovie.sbr"
	-@erase "$(INTDIR)\mainshort.obj"
	-@erase "$(INTDIR)\mainshort.sbr"
	-@erase "$(INTDIR)\map.obj"
	-@erase "$(INTDIR)\map.sbr"
	-@erase "$(INTDIR)\math3d.obj"
	-@erase "$(INTDIR)\math3d.sbr"
	-@erase "$(INTDIR)\metafile.obj"
	-@erase "$(INTDIR)\metafile.sbr"
	-@erase "$(INTDIR)\misc.obj"
	-@erase "$(INTDIR)\misc.sbr"
	-@erase "$(INTDIR)\model.obj"
	-@erase "$(INTDIR)\model.sbr"
	-@erase "$(INTDIR)\modfile.obj"
	-@erase "$(INTDIR)\modfile.sbr"
	-@erase "$(INTDIR)\motion.obj"
	-@erase "$(INTDIR)\motion.sbr"
	-@erase "$(INTDIR)\motionant.obj"
	-@erase "$(INTDIR)\motionant.sbr"
	-@erase "$(INTDIR)\motionbee.obj"
	-@erase "$(INTDIR)\motionbee.sbr"
	-@erase "$(INTDIR)\motionhuman.obj"
	-@erase "$(INTDIR)\motionhuman.sbr"
	-@erase "$(INTDIR)\motionmother.obj"
	-@erase "$(INTDIR)\motionmother.sbr"
	-@erase "$(INTDIR)\motionspider.obj"
	-@erase "$(INTDIR)\motionspider.sbr"
	-@erase "$(INTDIR)\motiontoto.obj"
	-@erase "$(INTDIR)\motiontoto.sbr"
	-@erase "$(INTDIR)\motionvehicle.obj"
	-@erase "$(INTDIR)\motionvehicle.sbr"
	-@erase "$(INTDIR)\motionworm.obj"
	-@erase "$(INTDIR)\motionworm.sbr"
	-@erase "$(INTDIR)\object.obj"
	-@erase "$(INTDIR)\object.sbr"
	-@erase "$(INTDIR)\particule.obj"
	-@erase "$(INTDIR)\particule.sbr"
	-@erase "$(INTDIR)\physics.obj"
	-@erase "$(INTDIR)\physics.sbr"
	-@erase "$(INTDIR)\planet.obj"
	-@erase "$(INTDIR)\planet.sbr"
	-@erase "$(INTDIR)\profile.obj"
	-@erase "$(INTDIR)\profile.sbr"
	-@erase "$(INTDIR)\pyro.obj"
	-@erase "$(INTDIR)\pyro.sbr"
	-@erase "$(INTDIR)\restext.obj"
	-@erase "$(INTDIR)\restext.sbr"
	-@erase "$(INTDIR)\robotmain.obj"
	-@erase "$(INTDIR)\robotmain.sbr"
	-@erase "$(INTDIR)\script.obj"
	-@erase "$(INTDIR)\script.sbr"
	-@erase "$(INTDIR)\scroll.obj"
	-@erase "$(INTDIR)\scroll.sbr"
	-@erase "$(INTDIR)\shortcut.obj"
	-@erase "$(INTDIR)\shortcut.sbr"
	-@erase "$(INTDIR)\slider.obj"
	-@erase "$(INTDIR)\slider.sbr"
	-@erase "$(INTDIR)\sound.obj"
	-@erase "$(INTDIR)\sound.sbr"
	-@erase "$(INTDIR)\studio.obj"
	-@erase "$(INTDIR)\studio.sbr"
	-@erase "$(INTDIR)\target.obj"
	-@erase "$(INTDIR)\target.sbr"
	-@erase "$(INTDIR)\task.obj"
	-@erase "$(INTDIR)\task.sbr"
	-@erase "$(INTDIR)\taskadvance.obj"
	-@erase "$(INTDIR)\taskadvance.sbr"
	-@erase "$(INTDIR)\taskbuild.obj"
	-@erase "$(INTDIR)\taskbuild.sbr"
	-@erase "$(INTDIR)\taskfire.obj"
	-@erase "$(INTDIR)\taskfire.sbr"
	-@erase "$(INTDIR)\taskfireant.obj"
	-@erase "$(INTDIR)\taskfireant.sbr"
	-@erase "$(INTDIR)\taskflag.obj"
	-@erase "$(INTDIR)\taskflag.sbr"
	-@erase "$(INTDIR)\taskgoto.obj"
	-@erase "$(INTDIR)\taskgoto.sbr"
	-@erase "$(INTDIR)\taskgungoal.obj"
	-@erase "$(INTDIR)\taskgungoal.sbr"
	-@erase "$(INTDIR)\taskinfo.obj"
	-@erase "$(INTDIR)\taskinfo.sbr"
	-@erase "$(INTDIR)\taskmanager.obj"
	-@erase "$(INTDIR)\taskmanager.sbr"
	-@erase "$(INTDIR)\taskmanip.obj"
	-@erase "$(INTDIR)\taskmanip.sbr"
	-@erase "$(INTDIR)\taskpen.obj"
	-@erase "$(INTDIR)\taskpen.sbr"
	-@erase "$(INTDIR)\taskrecover.obj"
	-@erase "$(INTDIR)\taskrecover.sbr"
	-@erase "$(INTDIR)\taskreset.obj"
	-@erase "$(INTDIR)\taskreset.sbr"
	-@erase "$(INTDIR)\tasksearch.obj"
	-@erase "$(INTDIR)\tasksearch.sbr"
	-@erase "$(INTDIR)\taskshield.obj"
	-@erase "$(INTDIR)\taskshield.sbr"
	-@erase "$(INTDIR)\taskspiderexplo.obj"
	-@erase "$(INTDIR)\taskspiderexplo.sbr"
	-@erase "$(INTDIR)\tasktake.obj"
	-@erase "$(INTDIR)\tasktake.sbr"
	-@erase "$(INTDIR)\taskterraform.obj"
	-@erase "$(INTDIR)\taskterraform.sbr"
	-@erase "$(INTDIR)\taskturn.obj"
	-@erase "$(INTDIR)\taskturn.sbr"
	-@erase "$(INTDIR)\taskwait.obj"
	-@erase "$(INTDIR)\taskwait.sbr"
	-@erase "$(INTDIR)\terrain.obj"
	-@erase "$(INTDIR)\terrain.sbr"
	-@erase "$(INTDIR)\text.obj"
	-@erase "$(INTDIR)\text.sbr"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(INTDIR)\water.obj"
	-@erase "$(INTDIR)\water.sbr"
	-@erase "$(INTDIR)\window.obj"
	-@erase "$(INTDIR)\window.sbr"
	-@erase "$(INTDIR)\winmain.res"
	-@erase "$(OUTDIR)\projet1.bsc"
	-@erase "$(OUTDIR)\projet1.exe"
	-@erase "$(OUTDIR)\projet1.ilk"
	-@erase "$(OUTDIR)\projet1.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /FR"$(INTDIR)\\" /Fp"$(INTDIR)\projet1.pch" /YX /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\Debug/

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o NUL /win32 
RSC=rc.exe
RSC_PROJ=/l 0x100c /fo"$(INTDIR)\winmain.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\projet1.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\auto.sbr" \
	"$(INTDIR)\autobase.sbr" \
	"$(INTDIR)\autoconvert.sbr" \
	"$(INTDIR)\autoderrick.sbr" \
	"$(INTDIR)\autodestroyer.sbr" \
	"$(INTDIR)\autoegg.sbr" \
	"$(INTDIR)\autoenergy.sbr" \
	"$(INTDIR)\autofactory.sbr" \
	"$(INTDIR)\autoflag.sbr" \
	"$(INTDIR)\autohuston.sbr" \
	"$(INTDIR)\autoinfo.sbr" \
	"$(INTDIR)\autojostle.sbr" \
	"$(INTDIR)\autokid.sbr" \
	"$(INTDIR)\autolabo.sbr" \
	"$(INTDIR)\automush.sbr" \
	"$(INTDIR)\autonest.sbr" \
	"$(INTDIR)\autonuclear.sbr" \
	"$(INTDIR)\autopara.sbr" \
	"$(INTDIR)\autoportico.sbr" \
	"$(INTDIR)\autoradar.sbr" \
	"$(INTDIR)\autorepair.sbr" \
	"$(INTDIR)\autoresearch.sbr" \
	"$(INTDIR)\autoroot.sbr" \
	"$(INTDIR)\autosafe.sbr" \
	"$(INTDIR)\autostation.sbr" \
	"$(INTDIR)\autotower.sbr" \
	"$(INTDIR)\blitz.sbr" \
	"$(INTDIR)\brain.sbr" \
	"$(INTDIR)\button.sbr" \
	"$(INTDIR)\camera.sbr" \
	"$(INTDIR)\cbottoken.sbr" \
	"$(INTDIR)\check.sbr" \
	"$(INTDIR)\cloud.sbr" \
	"$(INTDIR)\cmdtoken.sbr" \
	"$(INTDIR)\color.sbr" \
	"$(INTDIR)\compass.sbr" \
	"$(INTDIR)\control.sbr" \
	"$(INTDIR)\d3dapp.sbr" \
	"$(INTDIR)\d3dengine.sbr" \
	"$(INTDIR)\d3denum.sbr" \
	"$(INTDIR)\d3dframe.sbr" \
	"$(INTDIR)\d3dmath.sbr" \
	"$(INTDIR)\d3dtextr.sbr" \
	"$(INTDIR)\d3dutil.sbr" \
	"$(INTDIR)\displayinfo.sbr" \
	"$(INTDIR)\displaytext.sbr" \
	"$(INTDIR)\edit.sbr" \
	"$(INTDIR)\editvalue.sbr" \
	"$(INTDIR)\event.sbr" \
	"$(INTDIR)\gauge.sbr" \
	"$(INTDIR)\group.sbr" \
	"$(INTDIR)\image.sbr" \
	"$(INTDIR)\iman.sbr" \
	"$(INTDIR)\interface.sbr" \
	"$(INTDIR)\joystick.sbr" \
	"$(INTDIR)\key.sbr" \
	"$(INTDIR)\label.sbr" \
	"$(INTDIR)\light.sbr" \
	"$(INTDIR)\list.sbr" \
	"$(INTDIR)\maindialog.sbr" \
	"$(INTDIR)\mainmap.sbr" \
	"$(INTDIR)\mainmovie.sbr" \
	"$(INTDIR)\mainshort.sbr" \
	"$(INTDIR)\map.sbr" \
	"$(INTDIR)\math3d.sbr" \
	"$(INTDIR)\metafile.sbr" \
	"$(INTDIR)\misc.sbr" \
	"$(INTDIR)\model.sbr" \
	"$(INTDIR)\modfile.sbr" \
	"$(INTDIR)\motion.sbr" \
	"$(INTDIR)\motionant.sbr" \
	"$(INTDIR)\motionbee.sbr" \
	"$(INTDIR)\motionhuman.sbr" \
	"$(INTDIR)\motionmother.sbr" \
	"$(INTDIR)\motionspider.sbr" \
	"$(INTDIR)\motiontoto.sbr" \
	"$(INTDIR)\motionvehicle.sbr" \
	"$(INTDIR)\motionworm.sbr" \
	"$(INTDIR)\object.sbr" \
	"$(INTDIR)\particule.sbr" \
	"$(INTDIR)\physics.sbr" \
	"$(INTDIR)\planet.sbr" \
	"$(INTDIR)\profile.sbr" \
	"$(INTDIR)\pyro.sbr" \
	"$(INTDIR)\restext.sbr" \
	"$(INTDIR)\robotmain.sbr" \
	"$(INTDIR)\script.sbr" \
	"$(INTDIR)\scroll.sbr" \
	"$(INTDIR)\shortcut.sbr" \
	"$(INTDIR)\slider.sbr" \
	"$(INTDIR)\sound.sbr" \
	"$(INTDIR)\studio.sbr" \
	"$(INTDIR)\target.sbr" \
	"$(INTDIR)\task.sbr" \
	"$(INTDIR)\taskadvance.sbr" \
	"$(INTDIR)\taskbuild.sbr" \
	"$(INTDIR)\taskfire.sbr" \
	"$(INTDIR)\taskfireant.sbr" \
	"$(INTDIR)\taskflag.sbr" \
	"$(INTDIR)\taskgoto.sbr" \
	"$(INTDIR)\taskgungoal.sbr" \
	"$(INTDIR)\taskinfo.sbr" \
	"$(INTDIR)\taskmanager.sbr" \
	"$(INTDIR)\taskmanip.sbr" \
	"$(INTDIR)\taskpen.sbr" \
	"$(INTDIR)\taskrecover.sbr" \
	"$(INTDIR)\taskreset.sbr" \
	"$(INTDIR)\tasksearch.sbr" \
	"$(INTDIR)\taskshield.sbr" \
	"$(INTDIR)\taskspiderexplo.sbr" \
	"$(INTDIR)\tasktake.sbr" \
	"$(INTDIR)\taskterraform.sbr" \
	"$(INTDIR)\taskturn.sbr" \
	"$(INTDIR)\taskwait.sbr" \
	"$(INTDIR)\terrain.sbr" \
	"$(INTDIR)\text.sbr" \
	"$(INTDIR)\water.sbr" \
	"$(INTDIR)\window.sbr"

"$(OUTDIR)\projet1.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib winmm.lib c:\dx7sdk\lib\ddraw.lib c:\dx7sdk\lib\dinput.lib\
 c:\dx7sdk\lib\dxguid.lib c:\dx7sdk\lib\d3dx.lib c:\dx7sdk\lib\dsound.lib\
 cbot\cbot.lib /nologo /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)\projet1.pdb" /debug /machine:I386 /out:"$(OUTDIR)\projet1.exe"\
 /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\auto.obj" \
	"$(INTDIR)\autobase.obj" \
	"$(INTDIR)\autoconvert.obj" \
	"$(INTDIR)\autoderrick.obj" \
	"$(INTDIR)\autodestroyer.obj" \
	"$(INTDIR)\autoegg.obj" \
	"$(INTDIR)\autoenergy.obj" \
	"$(INTDIR)\autofactory.obj" \
	"$(INTDIR)\autoflag.obj" \
	"$(INTDIR)\autohuston.obj" \
	"$(INTDIR)\autoinfo.obj" \
	"$(INTDIR)\autojostle.obj" \
	"$(INTDIR)\autokid.obj" \
	"$(INTDIR)\autolabo.obj" \
	"$(INTDIR)\automush.obj" \
	"$(INTDIR)\autonest.obj" \
	"$(INTDIR)\autonuclear.obj" \
	"$(INTDIR)\autopara.obj" \
	"$(INTDIR)\autoportico.obj" \
	"$(INTDIR)\autoradar.obj" \
	"$(INTDIR)\autorepair.obj" \
	"$(INTDIR)\autoresearch.obj" \
	"$(INTDIR)\autoroot.obj" \
	"$(INTDIR)\autosafe.obj" \
	"$(INTDIR)\autostation.obj" \
	"$(INTDIR)\autotower.obj" \
	"$(INTDIR)\blitz.obj" \
	"$(INTDIR)\brain.obj" \
	"$(INTDIR)\button.obj" \
	"$(INTDIR)\camera.obj" \
	"$(INTDIR)\cbottoken.obj" \
	"$(INTDIR)\check.obj" \
	"$(INTDIR)\cloud.obj" \
	"$(INTDIR)\cmdtoken.obj" \
	"$(INTDIR)\color.obj" \
	"$(INTDIR)\compass.obj" \
	"$(INTDIR)\control.obj" \
	"$(INTDIR)\d3dapp.obj" \
	"$(INTDIR)\d3dengine.obj" \
	"$(INTDIR)\d3denum.obj" \
	"$(INTDIR)\d3dframe.obj" \
	"$(INTDIR)\d3dmath.obj" \
	"$(INTDIR)\d3dtextr.obj" \
	"$(INTDIR)\d3dutil.obj" \
	"$(INTDIR)\displayinfo.obj" \
	"$(INTDIR)\displaytext.obj" \
	"$(INTDIR)\edit.obj" \
	"$(INTDIR)\editvalue.obj" \
	"$(INTDIR)\event.obj" \
	"$(INTDIR)\gauge.obj" \
	"$(INTDIR)\group.obj" \
	"$(INTDIR)\image.obj" \
	"$(INTDIR)\iman.obj" \
	"$(INTDIR)\interface.obj" \
	"$(INTDIR)\joystick.obj" \
	"$(INTDIR)\key.obj" \
	"$(INTDIR)\label.obj" \
	"$(INTDIR)\light.obj" \
	"$(INTDIR)\list.obj" \
	"$(INTDIR)\maindialog.obj" \
	"$(INTDIR)\mainmap.obj" \
	"$(INTDIR)\mainmovie.obj" \
	"$(INTDIR)\mainshort.obj" \
	"$(INTDIR)\map.obj" \
	"$(INTDIR)\math3d.obj" \
	"$(INTDIR)\metafile.obj" \
	"$(INTDIR)\misc.obj" \
	"$(INTDIR)\model.obj" \
	"$(INTDIR)\modfile.obj" \
	"$(INTDIR)\motion.obj" \
	"$(INTDIR)\motionant.obj" \
	"$(INTDIR)\motionbee.obj" \
	"$(INTDIR)\motionhuman.obj" \
	"$(INTDIR)\motionmother.obj" \
	"$(INTDIR)\motionspider.obj" \
	"$(INTDIR)\motiontoto.obj" \
	"$(INTDIR)\motionvehicle.obj" \
	"$(INTDIR)\motionworm.obj" \
	"$(INTDIR)\object.obj" \
	"$(INTDIR)\particule.obj" \
	"$(INTDIR)\physics.obj" \
	"$(INTDIR)\planet.obj" \
	"$(INTDIR)\profile.obj" \
	"$(INTDIR)\pyro.obj" \
	"$(INTDIR)\restext.obj" \
	"$(INTDIR)\robotmain.obj" \
	"$(INTDIR)\script.obj" \
	"$(INTDIR)\scroll.obj" \
	"$(INTDIR)\shortcut.obj" \
	"$(INTDIR)\slider.obj" \
	"$(INTDIR)\sound.obj" \
	"$(INTDIR)\studio.obj" \
	"$(INTDIR)\target.obj" \
	"$(INTDIR)\task.obj" \
	"$(INTDIR)\taskadvance.obj" \
	"$(INTDIR)\taskbuild.obj" \
	"$(INTDIR)\taskfire.obj" \
	"$(INTDIR)\taskfireant.obj" \
	"$(INTDIR)\taskflag.obj" \
	"$(INTDIR)\taskgoto.obj" \
	"$(INTDIR)\taskgungoal.obj" \
	"$(INTDIR)\taskinfo.obj" \
	"$(INTDIR)\taskmanager.obj" \
	"$(INTDIR)\taskmanip.obj" \
	"$(INTDIR)\taskpen.obj" \
	"$(INTDIR)\taskrecover.obj" \
	"$(INTDIR)\taskreset.obj" \
	"$(INTDIR)\tasksearch.obj" \
	"$(INTDIR)\taskshield.obj" \
	"$(INTDIR)\taskspiderexplo.obj" \
	"$(INTDIR)\tasktake.obj" \
	"$(INTDIR)\taskterraform.obj" \
	"$(INTDIR)\taskturn.obj" \
	"$(INTDIR)\taskwait.obj" \
	"$(INTDIR)\terrain.obj" \
	"$(INTDIR)\text.obj" \
	"$(INTDIR)\water.obj" \
	"$(INTDIR)\window.obj" \
	"$(INTDIR)\winmain.res"

"$(OUTDIR)\projet1.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "projet1 - Win32 Release" || "$(CFG)" ==\
 "projet1 - Win32 Debug"
SOURCE=.\auto.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_AUTO_=\
	".\auto.h"\
	".\blitz.h"\
	".\button.h"\
	".\camera.h"\
	".\cloud.h"\
	".\cmdtoken.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\gauge.h"\
	".\iman.h"\
	".\interface.h"\
	".\label.h"\
	".\light.h"\
	".\list.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\modfile.h"\
	".\object.h"\
	".\particule.h"\
	".\planet.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\water.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\auto.obj" : $(SOURCE) $(DEP_CPP_AUTO_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_AUTO_=\
	".\auto.h"\
	".\blitz.h"\
	".\button.h"\
	".\camera.h"\
	".\cloud.h"\
	".\cmdtoken.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\gauge.h"\
	".\iman.h"\
	".\interface.h"\
	".\label.h"\
	".\light.h"\
	".\list.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\modfile.h"\
	".\object.h"\
	".\particule.h"\
	".\planet.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\water.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\auto.obj"	"$(INTDIR)\auto.sbr" : $(SOURCE) $(DEP_CPP_AUTO_)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\autobase.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_AUTOB=\
	".\auto.h"\
	".\autobase.h"\
	".\blitz.h"\
	".\button.h"\
	".\camera.h"\
	".\cloud.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\iman.h"\
	".\interface.h"\
	".\language.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\planet.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autobase.obj" : $(SOURCE) $(DEP_CPP_AUTOB) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_AUTOB=\
	".\auto.h"\
	".\autobase.h"\
	".\blitz.h"\
	".\button.h"\
	".\camera.h"\
	".\cloud.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\iman.h"\
	".\interface.h"\
	".\language.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\planet.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autobase.obj"	"$(INTDIR)\autobase.sbr" : $(SOURCE) $(DEP_CPP_AUTOB)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\autoconvert.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_AUTOC=\
	".\auto.h"\
	".\autoconvert.h"\
	".\button.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\iman.h"\
	".\interface.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autoconvert.obj" : $(SOURCE) $(DEP_CPP_AUTOC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_AUTOC=\
	".\auto.h"\
	".\autoconvert.h"\
	".\button.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\iman.h"\
	".\interface.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autoconvert.obj"	"$(INTDIR)\autoconvert.sbr" : $(SOURCE)\
 $(DEP_CPP_AUTOC) "$(INTDIR)"


!ENDIF 

SOURCE=.\autoderrick.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_AUTOD=\
	".\auto.h"\
	".\autoderrick.h"\
	".\button.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\iman.h"\
	".\interface.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autoderrick.obj" : $(SOURCE) $(DEP_CPP_AUTOD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_AUTOD=\
	".\auto.h"\
	".\autoderrick.h"\
	".\button.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\iman.h"\
	".\interface.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autoderrick.obj"	"$(INTDIR)\autoderrick.sbr" : $(SOURCE)\
 $(DEP_CPP_AUTOD) "$(INTDIR)"


!ENDIF 

SOURCE=.\autodestroyer.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_AUTODE=\
	".\auto.h"\
	".\autodestroyer.h"\
	".\button.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\interface.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\pyro.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autodestroyer.obj" : $(SOURCE) $(DEP_CPP_AUTODE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_AUTODE=\
	".\auto.h"\
	".\autodestroyer.h"\
	".\button.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\interface.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\pyro.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autodestroyer.obj"	"$(INTDIR)\autodestroyer.sbr" : $(SOURCE)\
 $(DEP_CPP_AUTODE) "$(INTDIR)"


!ENDIF 

SOURCE=.\autoegg.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_AUTOE=\
	".\auto.h"\
	".\autoegg.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\pyro.h"\
	".\struct.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autoegg.obj" : $(SOURCE) $(DEP_CPP_AUTOE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_AUTOE=\
	".\auto.h"\
	".\autoegg.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\pyro.h"\
	".\struct.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autoegg.obj"	"$(INTDIR)\autoegg.sbr" : $(SOURCE) $(DEP_CPP_AUTOE)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\autoenergy.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_AUTOEN=\
	".\auto.h"\
	".\autoenergy.h"\
	".\button.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\gauge.h"\
	".\iman.h"\
	".\interface.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autoenergy.obj" : $(SOURCE) $(DEP_CPP_AUTOEN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_AUTOEN=\
	".\auto.h"\
	".\autoenergy.h"\
	".\button.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\gauge.h"\
	".\iman.h"\
	".\interface.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autoenergy.obj"	"$(INTDIR)\autoenergy.sbr" : $(SOURCE)\
 $(DEP_CPP_AUTOEN) "$(INTDIR)"


!ENDIF 

SOURCE=.\autofactory.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_AUTOF=\
	".\auto.h"\
	".\autofactory.h"\
	".\brain.h"\
	".\button.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\global.h"\
	".\iman.h"\
	".\interface.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\restext.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autofactory.obj" : $(SOURCE) $(DEP_CPP_AUTOF) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_AUTOF=\
	".\auto.h"\
	".\autofactory.h"\
	".\brain.h"\
	".\button.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\global.h"\
	".\iman.h"\
	".\interface.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\restext.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autofactory.obj"	"$(INTDIR)\autofactory.sbr" : $(SOURCE)\
 $(DEP_CPP_AUTOF) "$(INTDIR)"


!ENDIF 

SOURCE=.\autoflag.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_AUTOFL=\
	".\auto.h"\
	".\autoflag.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\struct.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autoflag.obj" : $(SOURCE) $(DEP_CPP_AUTOFL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_AUTOFL=\
	".\auto.h"\
	".\autoflag.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\struct.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autoflag.obj"	"$(INTDIR)\autoflag.sbr" : $(SOURCE) $(DEP_CPP_AUTOFL)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\autohuston.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_AUTOH=\
	".\auto.h"\
	".\autohuston.h"\
	".\button.h"\
	".\camera.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\interface.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autohuston.obj" : $(SOURCE) $(DEP_CPP_AUTOH) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_AUTOH=\
	".\auto.h"\
	".\autohuston.h"\
	".\button.h"\
	".\camera.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\interface.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autohuston.obj"	"$(INTDIR)\autohuston.sbr" : $(SOURCE)\
 $(DEP_CPP_AUTOH) "$(INTDIR)"


!ENDIF 

SOURCE=.\autoinfo.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_AUTOI=\
	".\auto.h"\
	".\autoinfo.h"\
	".\button.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\interface.h"\
	".\light.h"\
	".\list.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autoinfo.obj" : $(SOURCE) $(DEP_CPP_AUTOI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_AUTOI=\
	".\auto.h"\
	".\autoinfo.h"\
	".\button.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\interface.h"\
	".\light.h"\
	".\list.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autoinfo.obj"	"$(INTDIR)\autoinfo.sbr" : $(SOURCE) $(DEP_CPP_AUTOI)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\autojostle.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_AUTOJ=\
	".\auto.h"\
	".\autojostle.h"\
	".\button.h"\
	".\camera.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\interface.h"\
	".\light.h"\
	".\list.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autojostle.obj" : $(SOURCE) $(DEP_CPP_AUTOJ) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_AUTOJ=\
	".\auto.h"\
	".\autojostle.h"\
	".\button.h"\
	".\camera.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\interface.h"\
	".\light.h"\
	".\list.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autojostle.obj"	"$(INTDIR)\autojostle.sbr" : $(SOURCE)\
 $(DEP_CPP_AUTOJ) "$(INTDIR)"


!ENDIF 

SOURCE=.\autokid.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_AUTOK=\
	".\auto.h"\
	".\autokid.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autokid.obj" : $(SOURCE) $(DEP_CPP_AUTOK) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_AUTOK=\
	".\auto.h"\
	".\autokid.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autokid.obj"	"$(INTDIR)\autokid.sbr" : $(SOURCE) $(DEP_CPP_AUTOK)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\autolabo.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_AUTOL=\
	".\auto.h"\
	".\autolabo.h"\
	".\button.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\global.h"\
	".\iman.h"\
	".\interface.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autolabo.obj" : $(SOURCE) $(DEP_CPP_AUTOL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_AUTOL=\
	".\auto.h"\
	".\autolabo.h"\
	".\button.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\global.h"\
	".\iman.h"\
	".\interface.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autolabo.obj"	"$(INTDIR)\autolabo.sbr" : $(SOURCE) $(DEP_CPP_AUTOL)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\automush.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_AUTOM=\
	".\auto.h"\
	".\automush.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\automush.obj" : $(SOURCE) $(DEP_CPP_AUTOM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_AUTOM=\
	".\auto.h"\
	".\automush.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\automush.obj"	"$(INTDIR)\automush.sbr" : $(SOURCE) $(DEP_CPP_AUTOM)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\autonest.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_AUTON=\
	".\auto.h"\
	".\autonest.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\struct.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autonest.obj" : $(SOURCE) $(DEP_CPP_AUTON) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_AUTON=\
	".\auto.h"\
	".\autonest.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\struct.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autonest.obj"	"$(INTDIR)\autonest.sbr" : $(SOURCE) $(DEP_CPP_AUTON)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\autonuclear.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_AUTONU=\
	".\auto.h"\
	".\autonuclear.h"\
	".\button.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\global.h"\
	".\iman.h"\
	".\interface.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autonuclear.obj" : $(SOURCE) $(DEP_CPP_AUTONU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_AUTONU=\
	".\auto.h"\
	".\autonuclear.h"\
	".\button.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\global.h"\
	".\iman.h"\
	".\interface.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autonuclear.obj"	"$(INTDIR)\autonuclear.sbr" : $(SOURCE)\
 $(DEP_CPP_AUTONU) "$(INTDIR)"


!ENDIF 

SOURCE=.\autopara.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_AUTOP=\
	".\auto.h"\
	".\autopara.h"\
	".\button.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\global.h"\
	".\iman.h"\
	".\interface.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autopara.obj" : $(SOURCE) $(DEP_CPP_AUTOP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_AUTOP=\
	".\auto.h"\
	".\autopara.h"\
	".\button.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\global.h"\
	".\iman.h"\
	".\interface.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autopara.obj"	"$(INTDIR)\autopara.sbr" : $(SOURCE) $(DEP_CPP_AUTOP)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\autoportico.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_AUTOPO=\
	".\auto.h"\
	".\autoportico.h"\
	".\button.h"\
	".\camera.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\iman.h"\
	".\interface.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autoportico.obj" : $(SOURCE) $(DEP_CPP_AUTOPO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_AUTOPO=\
	".\auto.h"\
	".\autoportico.h"\
	".\button.h"\
	".\camera.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\iman.h"\
	".\interface.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autoportico.obj"	"$(INTDIR)\autoportico.sbr" : $(SOURCE)\
 $(DEP_CPP_AUTOPO) "$(INTDIR)"


!ENDIF 

SOURCE=.\autoradar.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_AUTOR=\
	".\auto.h"\
	".\autoradar.h"\
	".\button.h"\
	".\camera.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\gauge.h"\
	".\iman.h"\
	".\interface.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autoradar.obj" : $(SOURCE) $(DEP_CPP_AUTOR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_AUTOR=\
	".\auto.h"\
	".\autoradar.h"\
	".\button.h"\
	".\camera.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\gauge.h"\
	".\iman.h"\
	".\interface.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autoradar.obj"	"$(INTDIR)\autoradar.sbr" : $(SOURCE)\
 $(DEP_CPP_AUTOR) "$(INTDIR)"


!ENDIF 

SOURCE=.\autorepair.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_AUTORE=\
	".\auto.h"\
	".\autorepair.h"\
	".\button.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\interface.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autorepair.obj" : $(SOURCE) $(DEP_CPP_AUTORE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_AUTORE=\
	".\auto.h"\
	".\autorepair.h"\
	".\button.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\interface.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autorepair.obj"	"$(INTDIR)\autorepair.sbr" : $(SOURCE)\
 $(DEP_CPP_AUTORE) "$(INTDIR)"


!ENDIF 

SOURCE=.\autoresearch.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_AUTORES=\
	".\auto.h"\
	".\autoresearch.h"\
	".\button.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\gauge.h"\
	".\global.h"\
	".\iman.h"\
	".\interface.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autoresearch.obj" : $(SOURCE) $(DEP_CPP_AUTORES) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_AUTORES=\
	".\auto.h"\
	".\autoresearch.h"\
	".\button.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\gauge.h"\
	".\global.h"\
	".\iman.h"\
	".\interface.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autoresearch.obj"	"$(INTDIR)\autoresearch.sbr" : $(SOURCE)\
 $(DEP_CPP_AUTORES) "$(INTDIR)"


!ENDIF 

SOURCE=.\autoroot.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_AUTORO=\
	".\auto.h"\
	".\autoroot.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\struct.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autoroot.obj" : $(SOURCE) $(DEP_CPP_AUTORO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_AUTORO=\
	".\auto.h"\
	".\autoroot.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\struct.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autoroot.obj"	"$(INTDIR)\autoroot.sbr" : $(SOURCE) $(DEP_CPP_AUTORO)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\autosafe.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_AUTOS=\
	".\auto.h"\
	".\autosafe.h"\
	".\button.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\global.h"\
	".\iman.h"\
	".\interface.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autosafe.obj" : $(SOURCE) $(DEP_CPP_AUTOS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_AUTOS=\
	".\auto.h"\
	".\autosafe.h"\
	".\button.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\global.h"\
	".\iman.h"\
	".\interface.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autosafe.obj"	"$(INTDIR)\autosafe.sbr" : $(SOURCE) $(DEP_CPP_AUTOS)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\autostation.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_AUTOST=\
	".\auto.h"\
	".\autostation.h"\
	".\button.h"\
	".\camera.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\gauge.h"\
	".\iman.h"\
	".\interface.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autostation.obj" : $(SOURCE) $(DEP_CPP_AUTOST) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_AUTOST=\
	".\auto.h"\
	".\autostation.h"\
	".\button.h"\
	".\camera.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\gauge.h"\
	".\iman.h"\
	".\interface.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autostation.obj"	"$(INTDIR)\autostation.sbr" : $(SOURCE)\
 $(DEP_CPP_AUTOST) "$(INTDIR)"


!ENDIF 

SOURCE=.\autotower.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_AUTOT=\
	".\auto.h"\
	".\autotower.h"\
	".\button.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\gauge.h"\
	".\global.h"\
	".\iman.h"\
	".\interface.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autotower.obj" : $(SOURCE) $(DEP_CPP_AUTOT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_AUTOT=\
	".\auto.h"\
	".\autotower.h"\
	".\button.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\gauge.h"\
	".\global.h"\
	".\iman.h"\
	".\interface.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\autotower.obj"	"$(INTDIR)\autotower.sbr" : $(SOURCE)\
 $(DEP_CPP_AUTOT) "$(INTDIR)"


!ENDIF 

SOURCE=.\blitz.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_BLITZ=\
	".\auto.h"\
	".\autopara.h"\
	".\blitz.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\blitz.obj" : $(SOURCE) $(DEP_CPP_BLITZ) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_BLITZ=\
	".\auto.h"\
	".\autopara.h"\
	".\blitz.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\blitz.obj"	"$(INTDIR)\blitz.sbr" : $(SOURCE) $(DEP_CPP_BLITZ)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\brain.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_BRAIN=\
	".\brain.h"\
	".\button.h"\
	".\camera.h"\
	".\cbot\cbotdll.h"\
	".\cmdtoken.h"\
	".\color.h"\
	".\compass.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\edit.h"\
	".\event.h"\
	".\gauge.h"\
	".\global.h"\
	".\group.h"\
	".\iman.h"\
	".\interface.h"\
	".\label.h"\
	".\language.h"\
	".\list.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\motion.h"\
	".\motionspider.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\pyro.h"\
	".\restext.h"\
	".\robotmain.h"\
	".\script.h"\
	".\slider.h"\
	".\sound.h"\
	".\struct.h"\
	".\studio.h"\
	".\target.h"\
	".\task.h"\
	".\taskflag.h"\
	".\taskmanager.h"\
	".\taskmanip.h"\
	".\taskshield.h"\
	".\terrain.h"\
	".\text.h"\
	".\water.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\brain.obj" : $(SOURCE) $(DEP_CPP_BRAIN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_BRAIN=\
	".\brain.h"\
	".\button.h"\
	".\camera.h"\
	".\cbot\cbotdll.h"\
	".\cmdtoken.h"\
	".\color.h"\
	".\compass.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\edit.h"\
	".\event.h"\
	".\gauge.h"\
	".\global.h"\
	".\group.h"\
	".\iman.h"\
	".\interface.h"\
	".\label.h"\
	".\language.h"\
	".\list.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\motion.h"\
	".\motionspider.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\pyro.h"\
	".\restext.h"\
	".\robotmain.h"\
	".\script.h"\
	".\slider.h"\
	".\sound.h"\
	".\struct.h"\
	".\studio.h"\
	".\target.h"\
	".\task.h"\
	".\taskflag.h"\
	".\taskmanager.h"\
	".\taskmanip.h"\
	".\taskshield.h"\
	".\terrain.h"\
	".\text.h"\
	".\water.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\brain.obj"	"$(INTDIR)\brain.sbr" : $(SOURCE) $(DEP_CPP_BRAIN)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\button.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_BUTTO=\
	".\button.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\language.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\restext.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\button.obj" : $(SOURCE) $(DEP_CPP_BUTTO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_BUTTO=\
	".\button.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\language.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\restext.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\button.obj"	"$(INTDIR)\button.sbr" : $(SOURCE) $(DEP_CPP_BUTTO)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\camera.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_CAMER=\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\language.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\physics.h"\
	".\struct.h"\
	".\terrain.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\camera.obj" : $(SOURCE) $(DEP_CPP_CAMER) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_CAMER=\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\language.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\physics.h"\
	".\struct.h"\
	".\terrain.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\camera.obj"	"$(INTDIR)\camera.sbr" : $(SOURCE) $(DEP_CPP_CAMER)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\cbottoken.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_CBOTT=\
	".\cbottoken.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\global.h"\
	".\language.h"\
	".\object.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\cbottoken.obj" : $(SOURCE) $(DEP_CPP_CBOTT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_CBOTT=\
	".\cbottoken.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\global.h"\
	".\language.h"\
	".\object.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\cbottoken.obj"	"$(INTDIR)\cbottoken.sbr" : $(SOURCE)\
 $(DEP_CPP_CBOTT) "$(INTDIR)"


!ENDIF 

SOURCE=.\check.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_CHECK=\
	".\check.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\restext.h"\
	".\struct.h"\
	".\text.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\check.obj" : $(SOURCE) $(DEP_CPP_CHECK) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_CHECK=\
	".\check.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\restext.h"\
	".\struct.h"\
	".\text.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\check.obj"	"$(INTDIR)\check.sbr" : $(SOURCE) $(DEP_CPP_CHECK)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\cloud.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_CLOUD=\
	".\cloud.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\struct.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\cloud.obj" : $(SOURCE) $(DEP_CPP_CLOUD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_CLOUD=\
	".\cloud.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\struct.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\cloud.obj"	"$(INTDIR)\cloud.sbr" : $(SOURCE) $(DEP_CPP_CLOUD)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\cmdtoken.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_CMDTO=\
	".\camera.h"\
	".\cmdtoken.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\global.h"\
	".\language.h"\
	".\object.h"\
	".\pyro.h"\
	".\struct.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\cmdtoken.obj" : $(SOURCE) $(DEP_CPP_CMDTO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_CMDTO=\
	".\camera.h"\
	".\cmdtoken.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\global.h"\
	".\language.h"\
	".\object.h"\
	".\pyro.h"\
	".\struct.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\cmdtoken.obj"	"$(INTDIR)\cmdtoken.sbr" : $(SOURCE) $(DEP_CPP_CMDTO)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\color.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_COLOR=\
	".\color.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\language.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\restext.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\color.obj" : $(SOURCE) $(DEP_CPP_COLOR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_COLOR=\
	".\color.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\language.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\restext.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\color.obj"	"$(INTDIR)\color.sbr" : $(SOURCE) $(DEP_CPP_COLOR)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\compass.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_COMPA=\
	".\compass.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\compass.obj" : $(SOURCE) $(DEP_CPP_COMPA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_COMPA=\
	".\compass.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\compass.obj"	"$(INTDIR)\compass.sbr" : $(SOURCE) $(DEP_CPP_COMPA)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\control.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_CONTR=\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\language.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\particule.h"\
	".\restext.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\text.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\control.obj" : $(SOURCE) $(DEP_CPP_CONTR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_CONTR=\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\language.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\particule.h"\
	".\restext.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\text.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\control.obj"	"$(INTDIR)\control.sbr" : $(SOURCE) $(DEP_CPP_CONTR)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\d3dapp.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_D3DAP=\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dtextr.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\joystick.h"\
	".\language.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\profile.h"\
	".\restext.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	"c:\dx7sdk\include\dinput.h"\
	

"$(INTDIR)\d3dapp.obj" : $(SOURCE) $(DEP_CPP_D3DAP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_D3DAP=\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dtextr.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\joystick.h"\
	".\language.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\profile.h"\
	".\restext.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	"c:\dx7sdk\include\dinput.h"\
	

"$(INTDIR)\d3dapp.obj"	"$(INTDIR)\d3dapp.sbr" : $(SOURCE) $(DEP_CPP_D3DAP)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\d3dengine.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_D3DEN=\
	".\blitz.h"\
	".\cloud.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dtextr.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\interface.h"\
	".\language.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\planet.h"\
	".\profile.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\text.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\d3dengine.obj" : $(SOURCE) $(DEP_CPP_D3DEN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_D3DEN=\
	".\blitz.h"\
	".\cloud.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dtextr.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\interface.h"\
	".\language.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\planet.h"\
	".\profile.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\text.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\d3dengine.obj"	"$(INTDIR)\d3dengine.sbr" : $(SOURCE)\
 $(DEP_CPP_D3DEN) "$(INTDIR)"


!ENDIF 

SOURCE=.\d3denum.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_D3DENU=\
	".\d3denum.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	

"$(INTDIR)\d3denum.obj" : $(SOURCE) $(DEP_CPP_D3DENU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_D3DENU=\
	".\d3denum.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	

"$(INTDIR)\d3denum.obj"	"$(INTDIR)\d3denum.sbr" : $(SOURCE) $(DEP_CPP_D3DENU)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\d3dframe.cpp
DEP_CPP_D3DFR=\
	".\d3dframe.h"\
	".\d3dutil.h"\
	

!IF  "$(CFG)" == "projet1 - Win32 Release"


"$(INTDIR)\d3dframe.obj" : $(SOURCE) $(DEP_CPP_D3DFR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"


"$(INTDIR)\d3dframe.obj"	"$(INTDIR)\d3dframe.sbr" : $(SOURCE) $(DEP_CPP_D3DFR)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\d3dmath.cpp
DEP_CPP_D3DMA=\
	".\d3dmath.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

!IF  "$(CFG)" == "projet1 - Win32 Release"


"$(INTDIR)\d3dmath.obj" : $(SOURCE) $(DEP_CPP_D3DMA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"


"$(INTDIR)\d3dmath.obj"	"$(INTDIR)\d3dmath.sbr" : $(SOURCE) $(DEP_CPP_D3DMA)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\d3dtextr.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_D3DTE=\
	".\d3dtextr.h"\
	".\d3dutil.h"\
	".\language.h"\
	".\metafile.h"\
	".\misc.h"\
	

"$(INTDIR)\d3dtextr.obj" : $(SOURCE) $(DEP_CPP_D3DTE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_D3DTE=\
	".\d3dtextr.h"\
	".\d3dutil.h"\
	".\language.h"\
	".\metafile.h"\
	".\misc.h"\
	

"$(INTDIR)\d3dtextr.obj"	"$(INTDIR)\d3dtextr.sbr" : $(SOURCE) $(DEP_CPP_D3DTE)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\d3dutil.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_D3DUT=\
	".\d3dutil.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\d3dutil.obj" : $(SOURCE) $(DEP_CPP_D3DUT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_D3DUT=\
	".\d3dutil.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\d3dutil.obj"	"$(INTDIR)\d3dutil.sbr" : $(SOURCE) $(DEP_CPP_D3DUT)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\displayinfo.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_DISPL=\
	".\button.h"\
	".\camera.h"\
	".\cbottoken.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displayinfo.h"\
	".\edit.h"\
	".\event.h"\
	".\group.h"\
	".\iman.h"\
	".\interface.h"\
	".\language.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\motion.h"\
	".\motiontoto.h"\
	".\object.h"\
	".\particule.h"\
	".\restext.h"\
	".\robotmain.h"\
	".\slider.h"\
	".\struct.h"\
	".\text.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\displayinfo.obj" : $(SOURCE) $(DEP_CPP_DISPL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_DISPL=\
	".\button.h"\
	".\camera.h"\
	".\cbottoken.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displayinfo.h"\
	".\edit.h"\
	".\event.h"\
	".\group.h"\
	".\iman.h"\
	".\interface.h"\
	".\language.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\motion.h"\
	".\motiontoto.h"\
	".\object.h"\
	".\particule.h"\
	".\restext.h"\
	".\robotmain.h"\
	".\slider.h"\
	".\struct.h"\
	".\text.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\displayinfo.obj"	"$(INTDIR)\displayinfo.sbr" : $(SOURCE)\
 $(DEP_CPP_DISPL) "$(INTDIR)"


!ENDIF 

SOURCE=.\displaytext.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_DISPLA=\
	".\button.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\group.h"\
	".\iman.h"\
	".\interface.h"\
	".\label.h"\
	".\metafile.h"\
	".\misc.h"\
	".\motion.h"\
	".\motiontoto.h"\
	".\object.h"\
	".\restext.h"\
	".\sound.h"\
	".\struct.h"\
	".\text.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\displaytext.obj" : $(SOURCE) $(DEP_CPP_DISPLA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_DISPLA=\
	".\button.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\group.h"\
	".\iman.h"\
	".\interface.h"\
	".\label.h"\
	".\metafile.h"\
	".\misc.h"\
	".\motion.h"\
	".\motiontoto.h"\
	".\object.h"\
	".\restext.h"\
	".\sound.h"\
	".\struct.h"\
	".\text.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\displaytext.obj"	"$(INTDIR)\displaytext.sbr" : $(SOURCE)\
 $(DEP_CPP_DISPLA) "$(INTDIR)"


!ENDIF 

SOURCE=.\edit.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_EDIT_=\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\edit.h"\
	".\event.h"\
	".\iman.h"\
	".\language.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\restext.h"\
	".\scroll.h"\
	".\struct.h"\
	".\text.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\edit.obj" : $(SOURCE) $(DEP_CPP_EDIT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_EDIT_=\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\edit.h"\
	".\event.h"\
	".\iman.h"\
	".\language.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\restext.h"\
	".\scroll.h"\
	".\struct.h"\
	".\text.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\edit.obj"	"$(INTDIR)\edit.sbr" : $(SOURCE) $(DEP_CPP_EDIT_)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\editvalue.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_EDITV=\
	".\button.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\edit.h"\
	".\editvalue.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\editvalue.obj" : $(SOURCE) $(DEP_CPP_EDITV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_EDITV=\
	".\button.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\edit.h"\
	".\editvalue.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\editvalue.obj"	"$(INTDIR)\editvalue.sbr" : $(SOURCE)\
 $(DEP_CPP_EDITV) "$(INTDIR)"


!ENDIF 

SOURCE=.\event.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_EVENT=\
	".\event.h"\
	".\iman.h"\
	".\metafile.h"\
	".\misc.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\event.obj" : $(SOURCE) $(DEP_CPP_EVENT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_EVENT=\
	".\event.h"\
	".\iman.h"\
	".\metafile.h"\
	".\misc.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\event.obj"	"$(INTDIR)\event.sbr" : $(SOURCE) $(DEP_CPP_EVENT)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\gauge.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_GAUGE=\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\gauge.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\gauge.obj" : $(SOURCE) $(DEP_CPP_GAUGE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_GAUGE=\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\gauge.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\gauge.obj"	"$(INTDIR)\gauge.sbr" : $(SOURCE) $(DEP_CPP_GAUGE)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\group.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_GROUP=\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\group.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\restext.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\group.obj" : $(SOURCE) $(DEP_CPP_GROUP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_GROUP=\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\group.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\restext.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\group.obj"	"$(INTDIR)\group.sbr" : $(SOURCE) $(DEP_CPP_GROUP)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\image.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_IMAGE=\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\image.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\restext.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\image.obj" : $(SOURCE) $(DEP_CPP_IMAGE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_IMAGE=\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\image.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\restext.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\image.obj"	"$(INTDIR)\image.sbr" : $(SOURCE) $(DEP_CPP_IMAGE)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\iman.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_IMAN_=\
	".\iman.h"\
	".\metafile.h"\
	".\misc.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\iman.obj" : $(SOURCE) $(DEP_CPP_IMAN_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_IMAN_=\
	".\iman.h"\
	".\metafile.h"\
	".\misc.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\iman.obj"	"$(INTDIR)\iman.sbr" : $(SOURCE) $(DEP_CPP_IMAN_)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\interface.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_INTER=\
	".\button.h"\
	".\camera.h"\
	".\check.h"\
	".\color.h"\
	".\compass.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\edit.h"\
	".\editvalue.h"\
	".\event.h"\
	".\group.h"\
	".\image.h"\
	".\iman.h"\
	".\interface.h"\
	".\key.h"\
	".\label.h"\
	".\list.h"\
	".\map.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\scroll.h"\
	".\shortcut.h"\
	".\slider.h"\
	".\struct.h"\
	".\target.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\interface.obj" : $(SOURCE) $(DEP_CPP_INTER) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_INTER=\
	".\button.h"\
	".\camera.h"\
	".\check.h"\
	".\color.h"\
	".\compass.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\edit.h"\
	".\editvalue.h"\
	".\event.h"\
	".\group.h"\
	".\image.h"\
	".\iman.h"\
	".\interface.h"\
	".\key.h"\
	".\label.h"\
	".\list.h"\
	".\map.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\scroll.h"\
	".\shortcut.h"\
	".\slider.h"\
	".\struct.h"\
	".\target.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\interface.obj"	"$(INTDIR)\interface.sbr" : $(SOURCE)\
 $(DEP_CPP_INTER) "$(INTDIR)"


!ENDIF 

SOURCE=.\joystick.cpp
DEP_CPP_JOYST=\
	".\joystick.h"\
	"c:\dx7sdk\include\dinput.h"\
	

!IF  "$(CFG)" == "projet1 - Win32 Release"


"$(INTDIR)\joystick.obj" : $(SOURCE) $(DEP_CPP_JOYST) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"


"$(INTDIR)\joystick.obj"	"$(INTDIR)\joystick.sbr" : $(SOURCE) $(DEP_CPP_JOYST)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\key.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_KEY_C=\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\key.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\restext.h"\
	".\sound.h"\
	".\struct.h"\
	".\text.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\key.obj" : $(SOURCE) $(DEP_CPP_KEY_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_KEY_C=\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\key.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\restext.h"\
	".\sound.h"\
	".\struct.h"\
	".\text.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\key.obj"	"$(INTDIR)\key.sbr" : $(SOURCE) $(DEP_CPP_KEY_C)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\label.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_LABEL=\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\label.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\struct.h"\
	".\text.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\label.obj" : $(SOURCE) $(DEP_CPP_LABEL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_LABEL=\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\label.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\struct.h"\
	".\text.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\label.obj"	"$(INTDIR)\label.sbr" : $(SOURCE) $(DEP_CPP_LABEL)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\light.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_LIGHT=\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\light.obj" : $(SOURCE) $(DEP_CPP_LIGHT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_LIGHT=\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\light.obj"	"$(INTDIR)\light.sbr" : $(SOURCE) $(DEP_CPP_LIGHT)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\list.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_LIST_=\
	".\button.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\list.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\scroll.h"\
	".\struct.h"\
	".\text.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\list.obj" : $(SOURCE) $(DEP_CPP_LIST_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_LIST_=\
	".\button.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\list.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\scroll.h"\
	".\struct.h"\
	".\text.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\list.obj"	"$(INTDIR)\list.sbr" : $(SOURCE) $(DEP_CPP_LIST_)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\maindialog.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_MAIND=\
	".\button.h"\
	".\camera.h"\
	".\check.h"\
	".\cmdtoken.h"\
	".\color.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\edit.h"\
	".\editvalue.h"\
	".\event.h"\
	".\global.h"\
	".\group.h"\
	".\image.h"\
	".\iman.h"\
	".\interface.h"\
	".\key.h"\
	".\label.h"\
	".\language.h"\
	".\list.h"\
	".\maindialog.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\particule.h"\
	".\profile.h"\
	".\restext.h"\
	".\robotmain.h"\
	".\scroll.h"\
	".\slider.h"\
	".\sound.h"\
	".\struct.h"\
	".\text.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\maindialog.obj" : $(SOURCE) $(DEP_CPP_MAIND) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_MAIND=\
	".\button.h"\
	".\camera.h"\
	".\check.h"\
	".\cmdtoken.h"\
	".\color.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\edit.h"\
	".\editvalue.h"\
	".\event.h"\
	".\global.h"\
	".\group.h"\
	".\image.h"\
	".\iman.h"\
	".\interface.h"\
	".\key.h"\
	".\label.h"\
	".\language.h"\
	".\list.h"\
	".\maindialog.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\particule.h"\
	".\profile.h"\
	".\restext.h"\
	".\robotmain.h"\
	".\scroll.h"\
	".\slider.h"\
	".\sound.h"\
	".\struct.h"\
	".\text.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\maindialog.obj"	"$(INTDIR)\maindialog.sbr" : $(SOURCE)\
 $(DEP_CPP_MAIND) "$(INTDIR)"


!ENDIF 

SOURCE=.\mainmap.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_MAINM=\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\global.h"\
	".\group.h"\
	".\image.h"\
	".\iman.h"\
	".\interface.h"\
	".\mainmap.h"\
	".\map.h"\
	".\metafile.h"\
	".\misc.h"\
	".\scroll.h"\
	".\slider.h"\
	".\struct.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\mainmap.obj" : $(SOURCE) $(DEP_CPP_MAINM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_MAINM=\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\global.h"\
	".\group.h"\
	".\image.h"\
	".\iman.h"\
	".\interface.h"\
	".\mainmap.h"\
	".\map.h"\
	".\metafile.h"\
	".\misc.h"\
	".\scroll.h"\
	".\slider.h"\
	".\struct.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\mainmap.obj"	"$(INTDIR)\mainmap.sbr" : $(SOURCE) $(DEP_CPP_MAINM)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\mainmovie.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_MAINMO=\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\global.h"\
	".\iman.h"\
	".\interface.h"\
	".\mainmovie.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\motion.h"\
	".\motionhuman.h"\
	".\object.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\mainmovie.obj" : $(SOURCE) $(DEP_CPP_MAINMO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_MAINMO=\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\global.h"\
	".\iman.h"\
	".\interface.h"\
	".\mainmovie.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\motion.h"\
	".\motionhuman.h"\
	".\object.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\mainmovie.obj"	"$(INTDIR)\mainmovie.sbr" : $(SOURCE)\
 $(DEP_CPP_MAINMO) "$(INTDIR)"


!ENDIF 

SOURCE=.\mainshort.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_MAINS=\
	".\button.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\global.h"\
	".\iman.h"\
	".\interface.h"\
	".\mainshort.h"\
	".\map.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\robotmain.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\mainshort.obj" : $(SOURCE) $(DEP_CPP_MAINS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_MAINS=\
	".\button.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\global.h"\
	".\iman.h"\
	".\interface.h"\
	".\mainshort.h"\
	".\map.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\robotmain.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\mainshort.obj"	"$(INTDIR)\mainshort.sbr" : $(SOURCE)\
 $(DEP_CPP_MAINS) "$(INTDIR)"


!ENDIF 

SOURCE=.\map.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_MAP_C=\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\map.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\robotmain.h"\
	".\struct.h"\
	".\terrain.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\map.obj" : $(SOURCE) $(DEP_CPP_MAP_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_MAP_C=\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\map.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\robotmain.h"\
	".\struct.h"\
	".\terrain.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\map.obj"	"$(INTDIR)\map.sbr" : $(SOURCE) $(DEP_CPP_MAP_C)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\math3d.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_MATH3=\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\math3d.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\math3d.obj" : $(SOURCE) $(DEP_CPP_MATH3) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_MATH3=\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\math3d.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\math3d.obj"	"$(INTDIR)\math3d.sbr" : $(SOURCE) $(DEP_CPP_MATH3)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\metafile.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_METAF=\
	".\language.h"\
	".\metafile.h"\
	

"$(INTDIR)\metafile.obj" : $(SOURCE) $(DEP_CPP_METAF) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_METAF=\
	".\language.h"\
	".\metafile.h"\
	

"$(INTDIR)\metafile.obj"	"$(INTDIR)\metafile.sbr" : $(SOURCE) $(DEP_CPP_METAF)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\misc.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_MISC_=\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\language.h"\
	".\metafile.h"\
	".\misc.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\misc.obj" : $(SOURCE) $(DEP_CPP_MISC_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_MISC_=\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\language.h"\
	".\metafile.h"\
	".\misc.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\misc.obj"	"$(INTDIR)\misc.sbr" : $(SOURCE) $(DEP_CPP_MISC_)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\model.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_MODEL=\
	".\button.h"\
	".\cmdtoken.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\edit.h"\
	".\event.h"\
	".\iman.h"\
	".\interface.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\model.h"\
	".\modfile.h"\
	".\robotmain.h"\
	".\struct.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\model.obj" : $(SOURCE) $(DEP_CPP_MODEL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_MODEL=\
	".\button.h"\
	".\cmdtoken.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\edit.h"\
	".\event.h"\
	".\iman.h"\
	".\interface.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\model.h"\
	".\modfile.h"\
	".\robotmain.h"\
	".\struct.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\model.obj"	"$(INTDIR)\model.sbr" : $(SOURCE) $(DEP_CPP_MODEL)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\modfile.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_MODFI=\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\language.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\modfile.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\modfile.obj" : $(SOURCE) $(DEP_CPP_MODFI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_MODFI=\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\language.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\modfile.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\modfile.obj"	"$(INTDIR)\modfile.sbr" : $(SOURCE) $(DEP_CPP_MODFI)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\motion.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_MOTIO=\
	".\brain.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\motion.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\motion.obj" : $(SOURCE) $(DEP_CPP_MOTIO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_MOTIO=\
	".\brain.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\motion.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\motion.obj"	"$(INTDIR)\motion.sbr" : $(SOURCE) $(DEP_CPP_MOTIO)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\motionant.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_MOTION=\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\modfile.h"\
	".\motion.h"\
	".\motionant.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\motionant.obj" : $(SOURCE) $(DEP_CPP_MOTION) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_MOTION=\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\modfile.h"\
	".\motion.h"\
	".\motionant.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\motionant.obj"	"$(INTDIR)\motionant.sbr" : $(SOURCE)\
 $(DEP_CPP_MOTION) "$(INTDIR)"


!ENDIF 

SOURCE=.\motionbee.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_MOTIONB=\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\modfile.h"\
	".\motion.h"\
	".\motionbee.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\motionbee.obj" : $(SOURCE) $(DEP_CPP_MOTIONB) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_MOTIONB=\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\modfile.h"\
	".\motion.h"\
	".\motionbee.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\motionbee.obj"	"$(INTDIR)\motionbee.sbr" : $(SOURCE)\
 $(DEP_CPP_MOTIONB) "$(INTDIR)"


!ENDIF 

SOURCE=.\motionhuman.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_MOTIONH=\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\modfile.h"\
	".\motion.h"\
	".\motionhuman.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\motionhuman.obj" : $(SOURCE) $(DEP_CPP_MOTIONH) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_MOTIONH=\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\modfile.h"\
	".\motion.h"\
	".\motionhuman.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\motionhuman.obj"	"$(INTDIR)\motionhuman.sbr" : $(SOURCE)\
 $(DEP_CPP_MOTIONH) "$(INTDIR)"


!ENDIF 

SOURCE=.\motionmother.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_MOTIONM=\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\modfile.h"\
	".\motion.h"\
	".\motionmother.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\motionmother.obj" : $(SOURCE) $(DEP_CPP_MOTIONM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_MOTIONM=\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\modfile.h"\
	".\motion.h"\
	".\motionmother.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\motionmother.obj"	"$(INTDIR)\motionmother.sbr" : $(SOURCE)\
 $(DEP_CPP_MOTIONM) "$(INTDIR)"


!ENDIF 

SOURCE=.\motionspider.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_MOTIONS=\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\modfile.h"\
	".\motion.h"\
	".\motionspider.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\motionspider.obj" : $(SOURCE) $(DEP_CPP_MOTIONS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_MOTIONS=\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\modfile.h"\
	".\motion.h"\
	".\motionspider.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\motionspider.obj"	"$(INTDIR)\motionspider.sbr" : $(SOURCE)\
 $(DEP_CPP_MOTIONS) "$(INTDIR)"


!ENDIF 

SOURCE=.\motiontoto.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_MOTIONT=\
	".\brain.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\modfile.h"\
	".\motion.h"\
	".\motiontoto.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\motiontoto.obj" : $(SOURCE) $(DEP_CPP_MOTIONT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_MOTIONT=\
	".\brain.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\modfile.h"\
	".\motion.h"\
	".\motiontoto.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\motiontoto.obj"	"$(INTDIR)\motiontoto.sbr" : $(SOURCE)\
 $(DEP_CPP_MOTIONT) "$(INTDIR)"


!ENDIF 

SOURCE=.\motionvehicle.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_MOTIONV=\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\modfile.h"\
	".\motion.h"\
	".\motionvehicle.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\motionvehicle.obj" : $(SOURCE) $(DEP_CPP_MOTIONV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_MOTIONV=\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\modfile.h"\
	".\motion.h"\
	".\motionvehicle.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\motionvehicle.obj"	"$(INTDIR)\motionvehicle.sbr" : $(SOURCE)\
 $(DEP_CPP_MOTIONV) "$(INTDIR)"


!ENDIF 

SOURCE=.\motionworm.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_MOTIONW=\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\modfile.h"\
	".\motion.h"\
	".\motionworm.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\motionworm.obj" : $(SOURCE) $(DEP_CPP_MOTIONW) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_MOTIONW=\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\modfile.h"\
	".\motion.h"\
	".\motionworm.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\motionworm.obj"	"$(INTDIR)\motionworm.sbr" : $(SOURCE)\
 $(DEP_CPP_MOTIONW) "$(INTDIR)"


!ENDIF 

SOURCE=.\object.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_OBJEC=\
	".\auto.h"\
	".\autobase.h"\
	".\autoconvert.h"\
	".\autoderrick.h"\
	".\autodestroyer.h"\
	".\autoegg.h"\
	".\autoenergy.h"\
	".\autofactory.h"\
	".\autoflag.h"\
	".\autohuston.h"\
	".\autoinfo.h"\
	".\autojostle.h"\
	".\autokid.h"\
	".\autolabo.h"\
	".\automush.h"\
	".\autonest.h"\
	".\autonuclear.h"\
	".\autopara.h"\
	".\autoportico.h"\
	".\autoradar.h"\
	".\autorepair.h"\
	".\autoresearch.h"\
	".\autoroot.h"\
	".\autosafe.h"\
	".\autostation.h"\
	".\autotower.h"\
	".\blitz.h"\
	".\brain.h"\
	".\camera.h"\
	".\cbot\cbotdll.h"\
	".\cbottoken.h"\
	".\cmdtoken.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\global.h"\
	".\iman.h"\
	".\light.h"\
	".\mainmovie.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\modfile.h"\
	".\motion.h"\
	".\motionant.h"\
	".\motionbee.h"\
	".\motionhuman.h"\
	".\motionmother.h"\
	".\motionspider.h"\
	".\motiontoto.h"\
	".\motionvehicle.h"\
	".\motionworm.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\pyro.h"\
	".\restext.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\task.h"\
	".\terrain.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\object.obj" : $(SOURCE) $(DEP_CPP_OBJEC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_OBJEC=\
	".\auto.h"\
	".\autobase.h"\
	".\autoconvert.h"\
	".\autoderrick.h"\
	".\autodestroyer.h"\
	".\autoegg.h"\
	".\autoenergy.h"\
	".\autofactory.h"\
	".\autoflag.h"\
	".\autohuston.h"\
	".\autoinfo.h"\
	".\autojostle.h"\
	".\autokid.h"\
	".\autolabo.h"\
	".\automush.h"\
	".\autonest.h"\
	".\autonuclear.h"\
	".\autopara.h"\
	".\autoportico.h"\
	".\autoradar.h"\
	".\autorepair.h"\
	".\autoresearch.h"\
	".\autoroot.h"\
	".\autosafe.h"\
	".\autostation.h"\
	".\autotower.h"\
	".\blitz.h"\
	".\brain.h"\
	".\camera.h"\
	".\cbot\cbotdll.h"\
	".\cbottoken.h"\
	".\cmdtoken.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\global.h"\
	".\iman.h"\
	".\light.h"\
	".\mainmovie.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\modfile.h"\
	".\motion.h"\
	".\motionant.h"\
	".\motionbee.h"\
	".\motionhuman.h"\
	".\motionmother.h"\
	".\motionspider.h"\
	".\motiontoto.h"\
	".\motionvehicle.h"\
	".\motionworm.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\pyro.h"\
	".\restext.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\task.h"\
	".\terrain.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\object.obj"	"$(INTDIR)\object.sbr" : $(SOURCE) $(DEP_CPP_OBJEC)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\particule.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_PARTI=\
	".\auto.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dtextr.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\language.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\particule.obj" : $(SOURCE) $(DEP_CPP_PARTI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_PARTI=\
	".\auto.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dtextr.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\language.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\particule.obj"	"$(INTDIR)\particule.sbr" : $(SOURCE)\
 $(DEP_CPP_PARTI) "$(INTDIR)"


!ENDIF 

SOURCE=.\physics.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_PHYSI=\
	".\brain.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\global.h"\
	".\iman.h"\
	".\language.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\motion.h"\
	".\motionhuman.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\pyro.h"\
	".\sound.h"\
	".\struct.h"\
	".\task.h"\
	".\terrain.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\physics.obj" : $(SOURCE) $(DEP_CPP_PHYSI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_PHYSI=\
	".\brain.h"\
	".\camera.h"\
	".\cmdtoken.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\global.h"\
	".\iman.h"\
	".\language.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\motion.h"\
	".\motionhuman.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\pyro.h"\
	".\sound.h"\
	".\struct.h"\
	".\task.h"\
	".\terrain.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\physics.obj"	"$(INTDIR)\physics.sbr" : $(SOURCE) $(DEP_CPP_PHYSI)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\planet.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_PLANE=\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\planet.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\planet.obj" : $(SOURCE) $(DEP_CPP_PLANE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_PLANE=\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\planet.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\planet.obj"	"$(INTDIR)\planet.sbr" : $(SOURCE) $(DEP_CPP_PLANE)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\profile.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_PROFI=\
	".\language.h"\
	".\profile.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\profile.obj" : $(SOURCE) $(DEP_CPP_PROFI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_PROFI=\
	".\language.h"\
	".\profile.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\profile.obj"	"$(INTDIR)\profile.sbr" : $(SOURCE) $(DEP_CPP_PROFI)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\pyro.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_PYRO_=\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\iman.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\motion.h"\
	".\motionhuman.h"\
	".\object.h"\
	".\particule.h"\
	".\pyro.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\pyro.obj" : $(SOURCE) $(DEP_CPP_PYRO_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_PYRO_=\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\iman.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\motion.h"\
	".\motionhuman.h"\
	".\object.h"\
	".\particule.h"\
	".\pyro.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\pyro.obj"	"$(INTDIR)\pyro.sbr" : $(SOURCE) $(DEP_CPP_PYRO_)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\restext.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_RESTE=\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\language.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\restext.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\restext.obj" : $(SOURCE) $(DEP_CPP_RESTE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_RESTE=\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\language.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\restext.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\restext.obj"	"$(INTDIR)\restext.sbr" : $(SOURCE) $(DEP_CPP_RESTE)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\robotmain.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_ROBOT=\
	".\auto.h"\
	".\autobase.h"\
	".\blitz.h"\
	".\brain.h"\
	".\button.h"\
	".\camera.h"\
	".\cbot\cbotdll.h"\
	".\cbottoken.h"\
	".\classfile.cpp"\
	".\cloud.h"\
	".\cmdtoken.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displayinfo.h"\
	".\displaytext.h"\
	".\edit.h"\
	".\event.h"\
	".\global.h"\
	".\iman.h"\
	".\interface.h"\
	".\label.h"\
	".\language.h"\
	".\light.h"\
	".\maindialog.h"\
	".\mainmap.h"\
	".\mainmovie.h"\
	".\mainshort.h"\
	".\map.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\model.h"\
	".\modfile.h"\
	".\motion.h"\
	".\motionhuman.h"\
	".\motiontoto.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\planet.h"\
	".\profile.h"\
	".\pyro.h"\
	".\restext.h"\
	".\robotmain.h"\
	".\script.h"\
	".\shortcut.h"\
	".\slider.h"\
	".\sound.h"\
	".\struct.h"\
	".\task.h"\
	".\taskbuild.h"\
	".\taskmanip.h"\
	".\terrain.h"\
	".\text.h"\
	".\water.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\robotmain.obj" : $(SOURCE) $(DEP_CPP_ROBOT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_ROBOT=\
	".\auto.h"\
	".\autobase.h"\
	".\blitz.h"\
	".\brain.h"\
	".\button.h"\
	".\camera.h"\
	".\cbot\cbotdll.h"\
	".\cbottoken.h"\
	".\classfile.cpp"\
	".\cloud.h"\
	".\cmdtoken.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displayinfo.h"\
	".\displaytext.h"\
	".\edit.h"\
	".\event.h"\
	".\global.h"\
	".\iman.h"\
	".\interface.h"\
	".\label.h"\
	".\language.h"\
	".\light.h"\
	".\maindialog.h"\
	".\mainmap.h"\
	".\mainmovie.h"\
	".\mainshort.h"\
	".\map.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\model.h"\
	".\modfile.h"\
	".\motion.h"\
	".\motionhuman.h"\
	".\motiontoto.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\planet.h"\
	".\profile.h"\
	".\pyro.h"\
	".\restext.h"\
	".\robotmain.h"\
	".\script.h"\
	".\shortcut.h"\
	".\slider.h"\
	".\sound.h"\
	".\struct.h"\
	".\task.h"\
	".\taskbuild.h"\
	".\taskmanip.h"\
	".\terrain.h"\
	".\text.h"\
	".\water.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\robotmain.obj"	"$(INTDIR)\robotmain.sbr" : $(SOURCE)\
 $(DEP_CPP_ROBOT) "$(INTDIR)"


!ENDIF 

SOURCE=.\script.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_SCRIP=\
	".\cbot\cbotdll.h"\
	".\cbottoken.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\edit.h"\
	".\event.h"\
	".\global.h"\
	".\iman.h"\
	".\interface.h"\
	".\list.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\physics.h"\
	".\restext.h"\
	".\robotmain.h"\
	".\script.h"\
	".\struct.h"\
	".\task.h"\
	".\taskgoto.h"\
	".\taskmanager.h"\
	".\taskmanip.h"\
	".\taskshield.h"\
	".\terrain.h"\
	".\text.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\script.obj" : $(SOURCE) $(DEP_CPP_SCRIP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_SCRIP=\
	".\cbot\cbotdll.h"\
	".\cbottoken.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\edit.h"\
	".\event.h"\
	".\global.h"\
	".\iman.h"\
	".\interface.h"\
	".\list.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\physics.h"\
	".\restext.h"\
	".\robotmain.h"\
	".\script.h"\
	".\struct.h"\
	".\task.h"\
	".\taskgoto.h"\
	".\taskmanager.h"\
	".\taskmanip.h"\
	".\taskshield.h"\
	".\terrain.h"\
	".\text.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\script.obj"	"$(INTDIR)\script.sbr" : $(SOURCE) $(DEP_CPP_SCRIP)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\scroll.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_SCROL=\
	".\button.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\scroll.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\scroll.obj" : $(SOURCE) $(DEP_CPP_SCROL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_SCROL=\
	".\button.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\scroll.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\scroll.obj"	"$(INTDIR)\scroll.sbr" : $(SOURCE) $(DEP_CPP_SCROL)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\shortcut.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_SHORT=\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\shortcut.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\shortcut.obj" : $(SOURCE) $(DEP_CPP_SHORT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_SHORT=\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\shortcut.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\shortcut.obj"	"$(INTDIR)\shortcut.sbr" : $(SOURCE) $(DEP_CPP_SHORT)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\slider.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_SLIDE=\
	".\button.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\slider.h"\
	".\struct.h"\
	".\text.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\slider.obj" : $(SOURCE) $(DEP_CPP_SLIDE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_SLIDE=\
	".\button.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\slider.h"\
	".\struct.h"\
	".\text.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\slider.obj"	"$(INTDIR)\slider.sbr" : $(SOURCE) $(DEP_CPP_SLIDE)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\sound.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_SOUND=\
	".\iman.h"\
	".\language.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\sound.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\sound.obj" : $(SOURCE) $(DEP_CPP_SOUND) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_SOUND=\
	".\iman.h"\
	".\language.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\sound.h"\
	".\struct.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\sound.obj"	"$(INTDIR)\sound.sbr" : $(SOURCE) $(DEP_CPP_SOUND)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\studio.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_STUDI=\
	".\button.h"\
	".\camera.h"\
	".\cbottoken.h"\
	".\check.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\edit.h"\
	".\event.h"\
	".\group.h"\
	".\iman.h"\
	".\interface.h"\
	".\label.h"\
	".\language.h"\
	".\list.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\restext.h"\
	".\robotmain.h"\
	".\script.h"\
	".\slider.h"\
	".\sound.h"\
	".\struct.h"\
	".\studio.h"\
	".\text.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\studio.obj" : $(SOURCE) $(DEP_CPP_STUDI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_STUDI=\
	".\button.h"\
	".\camera.h"\
	".\cbottoken.h"\
	".\check.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\edit.h"\
	".\event.h"\
	".\group.h"\
	".\iman.h"\
	".\interface.h"\
	".\label.h"\
	".\language.h"\
	".\list.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\restext.h"\
	".\robotmain.h"\
	".\script.h"\
	".\slider.h"\
	".\sound.h"\
	".\struct.h"\
	".\studio.h"\
	".\text.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\studio.obj"	"$(INTDIR)\studio.sbr" : $(SOURCE) $(DEP_CPP_STUDI)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\target.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_TARGE=\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\restext.h"\
	".\robotmain.h"\
	".\struct.h"\
	".\target.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\target.obj" : $(SOURCE) $(DEP_CPP_TARGE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_TARGE=\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\restext.h"\
	".\robotmain.h"\
	".\struct.h"\
	".\target.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\target.obj"	"$(INTDIR)\target.sbr" : $(SOURCE) $(DEP_CPP_TARGE)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\task.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_TASK_=\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\iman.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\motion.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\task.h"\
	".\terrain.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\task.obj" : $(SOURCE) $(DEP_CPP_TASK_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_TASK_=\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\iman.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\motion.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\task.h"\
	".\terrain.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\task.obj"	"$(INTDIR)\task.sbr" : $(SOURCE) $(DEP_CPP_TASK_)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\taskadvance.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_TASKA=\
	".\brain.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\physics.h"\
	".\struct.h"\
	".\task.h"\
	".\taskadvance.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\taskadvance.obj" : $(SOURCE) $(DEP_CPP_TASKA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_TASKA=\
	".\brain.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\physics.h"\
	".\struct.h"\
	".\task.h"\
	".\taskadvance.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\taskadvance.obj"	"$(INTDIR)\taskadvance.sbr" : $(SOURCE)\
 $(DEP_CPP_TASKA) "$(INTDIR)"


!ENDIF 

SOURCE=.\taskbuild.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_TASKB=\
	".\auto.h"\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\iman.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\motion.h"\
	".\motionhuman.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\task.h"\
	".\taskbuild.h"\
	".\terrain.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\taskbuild.obj" : $(SOURCE) $(DEP_CPP_TASKB) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_TASKB=\
	".\auto.h"\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\iman.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\motion.h"\
	".\motionhuman.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\task.h"\
	".\taskbuild.h"\
	".\terrain.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\taskbuild.obj"	"$(INTDIR)\taskbuild.sbr" : $(SOURCE)\
 $(DEP_CPP_TASKB) "$(INTDIR)"


!ENDIF 

SOURCE=.\taskfire.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_TASKF=\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\sound.h"\
	".\struct.h"\
	".\task.h"\
	".\taskfire.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\taskfire.obj" : $(SOURCE) $(DEP_CPP_TASKF) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_TASKF=\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\sound.h"\
	".\struct.h"\
	".\task.h"\
	".\taskfire.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\taskfire.obj"	"$(INTDIR)\taskfire.sbr" : $(SOURCE) $(DEP_CPP_TASKF)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\taskfireant.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_TASKFI=\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\motion.h"\
	".\motionant.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\struct.h"\
	".\task.h"\
	".\taskfireant.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\taskfireant.obj" : $(SOURCE) $(DEP_CPP_TASKFI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_TASKFI=\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\motion.h"\
	".\motionant.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\struct.h"\
	".\task.h"\
	".\taskfireant.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\taskfireant.obj"	"$(INTDIR)\taskfireant.sbr" : $(SOURCE)\
 $(DEP_CPP_TASKFI) "$(INTDIR)"


!ENDIF 

SOURCE=.\taskflag.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_TASKFL=\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\motion.h"\
	".\motionhuman.h"\
	".\object.h"\
	".\physics.h"\
	".\pyro.h"\
	".\sound.h"\
	".\struct.h"\
	".\task.h"\
	".\taskflag.h"\
	".\terrain.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\taskflag.obj" : $(SOURCE) $(DEP_CPP_TASKFL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_TASKFL=\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\motion.h"\
	".\motionhuman.h"\
	".\object.h"\
	".\physics.h"\
	".\pyro.h"\
	".\sound.h"\
	".\struct.h"\
	".\task.h"\
	".\taskflag.h"\
	".\terrain.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\taskflag.obj"	"$(INTDIR)\taskflag.sbr" : $(SOURCE) $(DEP_CPP_TASKFL)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\taskgoto.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_TASKG=\
	".\brain.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\physics.h"\
	".\struct.h"\
	".\task.h"\
	".\taskgoto.h"\
	".\terrain.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\taskgoto.obj" : $(SOURCE) $(DEP_CPP_TASKG) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_TASKG=\
	".\brain.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\physics.h"\
	".\struct.h"\
	".\task.h"\
	".\taskgoto.h"\
	".\terrain.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\taskgoto.obj"	"$(INTDIR)\taskgoto.sbr" : $(SOURCE) $(DEP_CPP_TASKG)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\taskgungoal.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_TASKGU=\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\sound.h"\
	".\struct.h"\
	".\task.h"\
	".\taskgungoal.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\taskgungoal.obj" : $(SOURCE) $(DEP_CPP_TASKGU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_TASKGU=\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\sound.h"\
	".\struct.h"\
	".\task.h"\
	".\taskgungoal.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\taskgungoal.obj"	"$(INTDIR)\taskgungoal.sbr" : $(SOURCE)\
 $(DEP_CPP_TASKGU) "$(INTDIR)"


!ENDIF 

SOURCE=.\taskinfo.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_TASKI=\
	".\auto.h"\
	".\autoinfo.h"\
	".\brain.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\sound.h"\
	".\struct.h"\
	".\task.h"\
	".\taskinfo.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\taskinfo.obj" : $(SOURCE) $(DEP_CPP_TASKI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_TASKI=\
	".\auto.h"\
	".\autoinfo.h"\
	".\brain.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\sound.h"\
	".\struct.h"\
	".\task.h"\
	".\taskinfo.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\taskinfo.obj"	"$(INTDIR)\taskinfo.sbr" : $(SOURCE) $(DEP_CPP_TASKI)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\taskmanager.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_TASKM=\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\struct.h"\
	".\task.h"\
	".\taskadvance.h"\
	".\taskbuild.h"\
	".\taskfire.h"\
	".\taskfireant.h"\
	".\taskflag.h"\
	".\taskgoto.h"\
	".\taskgungoal.h"\
	".\taskinfo.h"\
	".\taskmanager.h"\
	".\taskmanip.h"\
	".\taskpen.h"\
	".\taskrecover.h"\
	".\taskreset.h"\
	".\tasksearch.h"\
	".\taskshield.h"\
	".\taskspiderexplo.h"\
	".\tasktake.h"\
	".\taskterraform.h"\
	".\taskturn.h"\
	".\taskwait.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\taskmanager.obj" : $(SOURCE) $(DEP_CPP_TASKM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_TASKM=\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\struct.h"\
	".\task.h"\
	".\taskadvance.h"\
	".\taskbuild.h"\
	".\taskfire.h"\
	".\taskfireant.h"\
	".\taskflag.h"\
	".\taskgoto.h"\
	".\taskgungoal.h"\
	".\taskinfo.h"\
	".\taskmanager.h"\
	".\taskmanip.h"\
	".\taskpen.h"\
	".\taskrecover.h"\
	".\taskreset.h"\
	".\tasksearch.h"\
	".\taskshield.h"\
	".\taskspiderexplo.h"\
	".\tasktake.h"\
	".\taskterraform.h"\
	".\taskturn.h"\
	".\taskwait.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\taskmanager.obj"	"$(INTDIR)\taskmanager.sbr" : $(SOURCE)\
 $(DEP_CPP_TASKM) "$(INTDIR)"


!ENDIF 

SOURCE=.\taskmanip.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_TASKMA=\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\physics.h"\
	".\pyro.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\task.h"\
	".\taskmanip.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\taskmanip.obj" : $(SOURCE) $(DEP_CPP_TASKMA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_TASKMA=\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\physics.h"\
	".\pyro.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\task.h"\
	".\taskmanip.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\taskmanip.obj"	"$(INTDIR)\taskmanip.sbr" : $(SOURCE)\
 $(DEP_CPP_TASKMA) "$(INTDIR)"


!ENDIF 

SOURCE=.\taskpen.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_TASKP=\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\motion.h"\
	".\motionant.h"\
	".\motionspider.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\sound.h"\
	".\struct.h"\
	".\task.h"\
	".\taskpen.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\taskpen.obj" : $(SOURCE) $(DEP_CPP_TASKP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_TASKP=\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\motion.h"\
	".\motionant.h"\
	".\motionspider.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\sound.h"\
	".\struct.h"\
	".\task.h"\
	".\taskpen.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\taskpen.obj"	"$(INTDIR)\taskpen.sbr" : $(SOURCE) $(DEP_CPP_TASKP)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\taskrecover.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_TASKR=\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\sound.h"\
	".\struct.h"\
	".\task.h"\
	".\taskrecover.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\taskrecover.obj" : $(SOURCE) $(DEP_CPP_TASKR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_TASKR=\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\sound.h"\
	".\struct.h"\
	".\task.h"\
	".\taskrecover.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\taskrecover.obj"	"$(INTDIR)\taskrecover.sbr" : $(SOURCE)\
 $(DEP_CPP_TASKR) "$(INTDIR)"


!ENDIF 

SOURCE=.\taskreset.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_TASKRE=\
	".\brain.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\task.h"\
	".\taskreset.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\taskreset.obj" : $(SOURCE) $(DEP_CPP_TASKRE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_TASKRE=\
	".\brain.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\task.h"\
	".\taskreset.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\taskreset.obj"	"$(INTDIR)\taskreset.sbr" : $(SOURCE)\
 $(DEP_CPP_TASKRE) "$(INTDIR)"


!ENDIF 

SOURCE=.\tasksearch.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_TASKS=\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\sound.h"\
	".\struct.h"\
	".\task.h"\
	".\tasksearch.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\tasksearch.obj" : $(SOURCE) $(DEP_CPP_TASKS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_TASKS=\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\displaytext.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\sound.h"\
	".\struct.h"\
	".\task.h"\
	".\tasksearch.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\tasksearch.obj"	"$(INTDIR)\tasksearch.sbr" : $(SOURCE)\
 $(DEP_CPP_TASKS) "$(INTDIR)"


!ENDIF 

SOURCE=.\taskshield.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_TASKSH=\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\sound.h"\
	".\struct.h"\
	".\task.h"\
	".\taskshield.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\taskshield.obj" : $(SOURCE) $(DEP_CPP_TASKSH) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_TASKSH=\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\light.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\sound.h"\
	".\struct.h"\
	".\task.h"\
	".\taskshield.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\taskshield.obj"	"$(INTDIR)\taskshield.sbr" : $(SOURCE)\
 $(DEP_CPP_TASKSH) "$(INTDIR)"


!ENDIF 

SOURCE=.\taskspiderexplo.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_TASKSP=\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\motion.h"\
	".\motionspider.h"\
	".\object.h"\
	".\physics.h"\
	".\pyro.h"\
	".\struct.h"\
	".\task.h"\
	".\taskspiderexplo.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\taskspiderexplo.obj" : $(SOURCE) $(DEP_CPP_TASKSP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_TASKSP=\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\motion.h"\
	".\motionspider.h"\
	".\object.h"\
	".\physics.h"\
	".\pyro.h"\
	".\struct.h"\
	".\task.h"\
	".\taskspiderexplo.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\taskspiderexplo.obj"	"$(INTDIR)\taskspiderexplo.sbr" : $(SOURCE)\
 $(DEP_CPP_TASKSP) "$(INTDIR)"


!ENDIF 

SOURCE=.\tasktake.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_TASKT=\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\motion.h"\
	".\motionhuman.h"\
	".\object.h"\
	".\physics.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\task.h"\
	".\tasktake.h"\
	".\terrain.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\tasktake.obj" : $(SOURCE) $(DEP_CPP_TASKT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_TASKT=\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\motion.h"\
	".\motionhuman.h"\
	".\object.h"\
	".\physics.h"\
	".\robotmain.h"\
	".\sound.h"\
	".\struct.h"\
	".\task.h"\
	".\tasktake.h"\
	".\terrain.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\tasktake.obj"	"$(INTDIR)\tasktake.sbr" : $(SOURCE) $(DEP_CPP_TASKT)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\taskterraform.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_TASKTE=\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\language.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\motion.h"\
	".\motionant.h"\
	".\motionspider.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\pyro.h"\
	".\sound.h"\
	".\struct.h"\
	".\task.h"\
	".\taskterraform.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\taskterraform.obj" : $(SOURCE) $(DEP_CPP_TASKTE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_TASKTE=\
	".\brain.h"\
	".\camera.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\language.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\motion.h"\
	".\motionant.h"\
	".\motionspider.h"\
	".\object.h"\
	".\particule.h"\
	".\physics.h"\
	".\pyro.h"\
	".\sound.h"\
	".\struct.h"\
	".\task.h"\
	".\taskterraform.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\taskterraform.obj"	"$(INTDIR)\taskterraform.sbr" : $(SOURCE)\
 $(DEP_CPP_TASKTE) "$(INTDIR)"


!ENDIF 

SOURCE=.\taskturn.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_TASKTU=\
	".\brain.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\physics.h"\
	".\struct.h"\
	".\task.h"\
	".\taskturn.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\taskturn.obj" : $(SOURCE) $(DEP_CPP_TASKTU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_TASKTU=\
	".\brain.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\physics.h"\
	".\struct.h"\
	".\task.h"\
	".\taskturn.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\taskturn.obj"	"$(INTDIR)\taskturn.sbr" : $(SOURCE) $(DEP_CPP_TASKTU)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\taskwait.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_TASKW=\
	".\brain.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\physics.h"\
	".\struct.h"\
	".\task.h"\
	".\taskwait.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\taskwait.obj" : $(SOURCE) $(DEP_CPP_TASKW) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_TASKW=\
	".\brain.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\physics.h"\
	".\struct.h"\
	".\task.h"\
	".\taskwait.h"\
	".\terrain.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\taskwait.obj"	"$(INTDIR)\taskwait.sbr" : $(SOURCE) $(DEP_CPP_TASKW)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\terrain.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_TERRA=\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\language.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\modfile.h"\
	".\struct.h"\
	".\terrain.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\terrain.obj" : $(SOURCE) $(DEP_CPP_TERRA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_TERRA=\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\language.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\modfile.h"\
	".\struct.h"\
	".\terrain.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\terrain.obj"	"$(INTDIR)\terrain.sbr" : $(SOURCE) $(DEP_CPP_TERRA)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\text.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_TEXT_=\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\language.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\struct.h"\
	".\text.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\text.obj" : $(SOURCE) $(DEP_CPP_TEXT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_TEXT_=\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\language.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\struct.h"\
	".\text.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\text.obj"	"$(INTDIR)\text.sbr" : $(SOURCE) $(DEP_CPP_TEXT_)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\water.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_WATER=\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\water.obj" : $(SOURCE) $(DEP_CPP_WATER) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_WATER=\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dmath.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\event.h"\
	".\iman.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\object.h"\
	".\particule.h"\
	".\sound.h"\
	".\struct.h"\
	".\terrain.h"\
	".\water.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\water.obj"	"$(INTDIR)\water.sbr" : $(SOURCE) $(DEP_CPP_WATER)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\window.cpp

!IF  "$(CFG)" == "projet1 - Win32 Release"

DEP_CPP_WINDO=\
	".\button.h"\
	".\check.h"\
	".\color.h"\
	".\compass.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\edit.h"\
	".\editvalue.h"\
	".\event.h"\
	".\gauge.h"\
	".\group.h"\
	".\image.h"\
	".\iman.h"\
	".\key.h"\
	".\label.h"\
	".\language.h"\
	".\list.h"\
	".\map.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\restext.h"\
	".\scroll.h"\
	".\shortcut.h"\
	".\slider.h"\
	".\struct.h"\
	".\target.h"\
	".\text.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\window.obj" : $(SOURCE) $(DEP_CPP_WINDO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

DEP_CPP_WINDO=\
	".\button.h"\
	".\check.h"\
	".\color.h"\
	".\compass.h"\
	".\control.h"\
	".\d3dapp.h"\
	".\d3dengine.h"\
	".\d3denum.h"\
	".\d3dframe.h"\
	".\d3dres.h"\
	".\d3dutil.h"\
	".\edit.h"\
	".\editvalue.h"\
	".\event.h"\
	".\gauge.h"\
	".\group.h"\
	".\image.h"\
	".\iman.h"\
	".\key.h"\
	".\label.h"\
	".\language.h"\
	".\list.h"\
	".\map.h"\
	".\math3d.h"\
	".\metafile.h"\
	".\misc.h"\
	".\restext.h"\
	".\scroll.h"\
	".\shortcut.h"\
	".\slider.h"\
	".\struct.h"\
	".\target.h"\
	".\text.h"\
	".\window.h"\
	"c:\dx7sdk\include\d3dvec.inl"\
	

"$(INTDIR)\window.obj"	"$(INTDIR)\window.sbr" : $(SOURCE) $(DEP_CPP_WINDO)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\winmain.rc
DEP_RSC_WINMA=\
	".\cur00001.cur"\
	".\cur00002.cur"\
	".\cur00003.cur"\
	".\cursor1.cur"\
	".\cursorha.cur"\
	".\cursorsc.cur"\
	".\directx.ico"\
	

"$(INTDIR)\winmain.res" : $(SOURCE) $(DEP_RSC_WINMA) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

