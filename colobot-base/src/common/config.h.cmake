#pragma once

// Macros set by CMake
#cmakedefine PLATFORM_LINUX @PLATFORM_LINUX@
#cmakedefine PLATFORM_GNU @PLATFORM_GNU@
#cmakedefine PLATFORM_MACOSX @PLATFORM_MACOSX@

#ifdef PLATFORM_MACOSX
// Assume we have the Mac OS X function CFLocaleCopyCurrent in the CoreFoundation framework
#define HAVE_CFLOCALECOPYCURRENT 1
#endif

#ifndef GLEW_STATIC
#cmakedefine GLEW_STATIC
#endif

#cmakedefine OPENAL_SOUND

#define COLOBOT_DEFAULT_DATADIR "@COLOBOT_DATA_DIR@"
#define COLOBOT_I18N_DIR "@COLOBOT_I18N_DIR@"
