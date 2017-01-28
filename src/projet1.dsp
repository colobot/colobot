# Microsoft Developer Studio Project File - Name="projet1" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=projet1 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "projet1.mak".
!MESSAGE 
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

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "projet1 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /Zi /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x100c /d "NDEBUG"
# ADD RSC /l 0x100c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib c:\dx7sdk\lib\ddraw.lib c:\dx7sdk\lib\dinput.lib c:\dx7sdk\lib\dxguid.lib c:\dx7sdk\lib\d3dx.lib c:\dx7sdk\lib\dsound.lib cbot\cbot.lib /nologo /subsystem:windows /machine:I386 /out:"Release/BlupiMania-2.exe"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x100c /d "_DEBUG"
# ADD RSC /l 0x100c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib c:\dx7sdk\lib\ddraw.lib c:\dx7sdk\lib\dinput.lib c:\dx7sdk\lib\dxguid.lib c:\dx7sdk\lib\d3dx.lib c:\dx7sdk\lib\dsound.lib cbot\cbot.lib /nologo /subsystem:windows /debug /machine:I386 /out:"Debug/BlupiMania-2.exe" /pdbtype:sept
# SUBTRACT LINK32 /map

!ENDIF 

# Begin Target

# Name "projet1 - Win32 Release"
# Name "projet1 - Win32 Debug"
# Begin Source File

SOURCE=.\array.cpp
# End Source File
# Begin Source File

SOURCE=.\auto.cpp
# End Source File
# Begin Source File

SOURCE=.\autocatapult.cpp
# End Source File
# Begin Source File

SOURCE=.\autodock.cpp
# End Source File
# Begin Source File

SOURCE=.\autodoor.cpp
# End Source File
# Begin Source File

SOURCE=.\autofiole.cpp
# End Source File
# Begin Source File

SOURCE=.\autoflash.cpp
# End Source File
# Begin Source File

SOURCE=.\autogoal.cpp
# End Source File
# Begin Source File

SOURCE=.\autojostle.cpp
# End Source File
# Begin Source File

SOURCE=.\autolift.cpp
# End Source File
# Begin Source File

SOURCE=.\automax1x.cpp
# End Source File
# Begin Source File

SOURCE=.\automeca.cpp
# End Source File
# Begin Source File

SOURCE=.\autoscrap.cpp
# End Source File
# Begin Source File

SOURCE=.\blitz.cpp
# End Source File
# Begin Source File

SOURCE=.\button.cpp
# End Source File
# Begin Source File

SOURCE=.\camera.cpp
# End Source File
# Begin Source File

SOURCE=.\cbottoken.cpp
# End Source File
# Begin Source File

SOURCE=.\check.cpp
# End Source File
# Begin Source File

SOURCE=.\cloud.cpp
# End Source File
# Begin Source File

SOURCE=.\cmdtoken.cpp
# End Source File
# Begin Source File

SOURCE=.\color.cpp
# End Source File
# Begin Source File

SOURCE=.\compass.cpp
# End Source File
# Begin Source File

SOURCE=.\control.cpp
# End Source File
# Begin Source File

SOURCE=.\cryptfile.cpp
# End Source File
# Begin Source File

SOURCE=.\cur00001.cur
# End Source File
# Begin Source File

SOURCE=.\cur00002.cur
# End Source File
# Begin Source File

SOURCE=.\cur00003.cur
# End Source File
# Begin Source File

SOURCE=.\cur00004.cur
# End Source File
# Begin Source File

SOURCE=.\cursor1.cur
# End Source File
# Begin Source File

SOURCE=.\cursorha.cur
# End Source File
# Begin Source File

SOURCE=.\cursorsc.cur
# End Source File
# Begin Source File

SOURCE=.\cursorse.cur
# End Source File
# Begin Source File

SOURCE=.\d3dapp.cpp
# End Source File
# Begin Source File

SOURCE=.\d3dengine.cpp
# End Source File
# Begin Source File

SOURCE=.\d3denum.cpp
# End Source File
# Begin Source File

SOURCE=.\d3dframe.cpp
# End Source File
# Begin Source File

SOURCE=.\d3dmath.cpp
# End Source File
# Begin Source File

SOURCE=.\d3dtextr.cpp
# End Source File
# Begin Source File

SOURCE=.\d3dutil.cpp
# End Source File
# Begin Source File

SOURCE=.\directx.ico
# End Source File
# Begin Source File

SOURCE=.\displaytext.cpp
# End Source File
# Begin Source File

SOURCE=.\edit.cpp
# End Source File
# Begin Source File

SOURCE=.\editvalue.cpp
# End Source File
# Begin Source File

SOURCE=.\event.cpp
# End Source File
# Begin Source File

SOURCE=.\filebuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\gamerfile.cpp
# End Source File
# Begin Source File

SOURCE=.\gauge.cpp
# End Source File
# Begin Source File

SOURCE=.\group.cpp
# End Source File
# Begin Source File

SOURCE=.\image.cpp
# End Source File
# Begin Source File

SOURCE=.\iman.cpp
# End Source File
# Begin Source File

SOURCE=.\interface.cpp
# End Source File
# Begin Source File

SOURCE=.\joystick.cpp
# End Source File
# Begin Source File

SOURCE=.\key.cpp
# End Source File
# Begin Source File

SOURCE=.\label.cpp
# End Source File
# Begin Source File

SOURCE=.\language.h
# End Source File
# Begin Source File

SOURCE=.\light.cpp
# End Source File
# Begin Source File

SOURCE=.\link.cpp
# End Source File
# Begin Source File

SOURCE=.\list.cpp
# End Source File
# Begin Source File

SOURCE=.\maindialog.cpp
# End Source File
# Begin Source File

SOURCE=.\mainedit.cpp
# End Source File
# Begin Source File

SOURCE=.\mainmap.cpp
# End Source File
# Begin Source File

SOURCE=.\mainundo.cpp
# End Source File
# Begin Source File

SOURCE=.\map.cpp
# End Source File
# Begin Source File

SOURCE=.\math3d.cpp
# End Source File
# Begin Source File

SOURCE=.\menu.cpp
# End Source File
# Begin Source File

SOURCE=.\metafile.cpp
# End Source File
# Begin Source File

SOURCE=.\misc.cpp
# End Source File
# Begin Source File

SOURCE=.\model.cpp
# End Source File
# Begin Source File

SOURCE=.\modfile.cpp
# End Source File
# Begin Source File

SOURCE=.\motion.cpp
# End Source File
# Begin Source File

SOURCE=.\motionbird.cpp
# End Source File
# Begin Source File

SOURCE=.\motionblupi.cpp
# End Source File
# Begin Source File

SOURCE=.\motionbot.cpp
# End Source File
# Begin Source File

SOURCE=.\motionfish.cpp
# End Source File
# Begin Source File

SOURCE=.\motiongun.cpp
# End Source File
# Begin Source File

SOURCE=.\motionjet.cpp
# End Source File
# Begin Source File

SOURCE=.\motionperfo.cpp
# End Source File
# Begin Source File

SOURCE=.\motionptero.cpp
# End Source File
# Begin Source File

SOURCE=.\motionsnake.cpp
# End Source File
# Begin Source File

SOURCE=.\motionsubm.cpp
# End Source File
# Begin Source File

SOURCE=.\motiontrax.cpp
# End Source File
# Begin Source File

SOURCE=.\object.cpp
# End Source File
# Begin Source File

SOURCE=.\particule.cpp
# End Source File
# Begin Source File

SOURCE=.\planet.cpp
# End Source File
# Begin Source File

SOURCE=.\profile.cpp
# End Source File
# Begin Source File

SOURCE=.\progress.cpp
# End Source File
# Begin Source File

SOURCE=.\pyro.cpp
# End Source File
# Begin Source File

SOURCE=.\restext.cpp
# End Source File
# Begin Source File

SOURCE=.\robotmain.cpp
# End Source File
# Begin Source File

SOURCE=.\script.cpp
# End Source File
# Begin Source File

SOURCE=.\scroll.cpp
# End Source File
# Begin Source File

SOURCE=.\shortcut.cpp
# End Source File
# Begin Source File

SOURCE=.\slider.cpp
# End Source File
# Begin Source File

SOURCE=.\sound.cpp
# End Source File
# Begin Source File

SOURCE=.\task.cpp
# End Source File
# Begin Source File

SOURCE=.\taskadvance.cpp
# End Source File
# Begin Source File

SOURCE=.\taskcatapult.cpp
# End Source File
# Begin Source File

SOURCE=.\taskdive.cpp
# End Source File
# Begin Source File

SOURCE=.\taskdock.cpp
# End Source File
# Begin Source File

SOURCE=.\taskdrink.cpp
# End Source File
# Begin Source File

SOURCE=.\taskfire.cpp
# End Source File
# Begin Source File

SOURCE=.\taskgoal.cpp
# End Source File
# Begin Source File

SOURCE=.\taskgoto.cpp
# End Source File
# Begin Source File

SOURCE=.\taskgun.cpp
# End Source File
# Begin Source File

SOURCE=.\tasklist.cpp
# End Source File
# Begin Source File

SOURCE=.\taskmanager.cpp
# End Source File
# Begin Source File

SOURCE=.\taskperfo.cpp
# End Source File
# Begin Source File

SOURCE=.\taskpush.cpp
# End Source File
# Begin Source File

SOURCE=.\taskroll.cpp
# End Source File
# Begin Source File

SOURCE=.\tasktrax.cpp
# End Source File
# Begin Source File

SOURCE=.\taskturn.cpp
# End Source File
# Begin Source File

SOURCE=.\taskwait.cpp
# End Source File
# Begin Source File

SOURCE=.\terrain.cpp
# End Source File
# Begin Source File

SOURCE=.\text.cpp
# End Source File
# Begin Source File

SOURCE=.\water.cpp
# End Source File
# Begin Source File

SOURCE=.\window.cpp
# End Source File
# Begin Source File

SOURCE=.\winmain.rc

!IF  "$(CFG)" == "projet1 - Win32 Release"

!ELSEIF  "$(CFG)" == "projet1 - Win32 Debug"

!ENDIF 

# End Source File
# End Target
# End Project
