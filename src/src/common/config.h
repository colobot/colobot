#pragma once

// Macros set by CMake
#define PLATFORM_WINDOWS 1
/* #undef PLATFORM_LINUX */
/* #undef PLATFORM_GNU */
/* #undef PLATFORM_MACOSX */
/* #undef PLATFORM_OTHER */

#ifdef PLATFORM_MACOSX
// Assume we have the Mac OS X function CFLocaleCopyCurrent in the CoreFoundation framework
#define HAVE_CFLOCALECOPYCURRENT 1
#endif

/* #undef GLEW_STATIC */

#define OPENAL_SOUND

/* #undef PORTABLE */

#define COLOBOT_DEFAULT_DATADIR "C:/Program Files (x86)/colobot/data"
#define COLOBOT_I18N_DIR "C:/Program Files (x86)/colobot/lang"
