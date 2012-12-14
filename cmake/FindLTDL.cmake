# Find the native LTDL includes and library
# Copied verbatim from
# http://code.metager.de/source/xref/hshannover/trust/tnc-fhh/shared/cmake_modules/FindLTDL.cmake
# The above version was under GPL-2, this one is under GPL-3, for consistency
# with the rest of the colobot project.
#
# This module defines
#  LTDL_INCLUDE_DIR, where to find ltdl.h, etc.
#  LTDL_LIBRARY, where to find the LTDL library.
#  LTDL_FOUND, If false, do not try to use LTDL.

FIND_PATH(LTDL_INCLUDE_DIR ltdl.h)

FIND_LIBRARY(LTDL_LIBRARY ltdl)

IF (LTDL_INCLUDE_DIR AND LTDL_LIBRARY)
   SET(LTDL_FOUND TRUE)
ELSE(LTDL_INCLUDE_DIR AND LTDL_LIBRARY)
   SET(LTDL_FOUND FALSE)
ENDIF (LTDL_INCLUDE_DIR AND LTDL_LIBRARY)

IF (LTDL_FOUND)
   IF (NOT LTDL_FIND_QUIETLY)
      MESSAGE(STATUS "Found LTDL: ${LTDL_LIBRARY}")
   ENDIF (NOT LTDL_FIND_QUIETLY)
ELSE (LTDL_FOUND)
   IF (LTDL_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find LTDL")
   ENDIF (LTDL_FIND_REQUIRED)
ENDIF (LTDL_FOUND)
