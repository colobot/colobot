# CMake module to find GLEW
# Borrowed from http://code.google.com/p/nvidia-texture-tools/
# MIT license Copyright (c) 2007 NVIDIA Corporation

# Try to find GLEW library and include path.
# Once done this will define
#
# GLEW_FOUND
# GLEW_INCLUDE_PATH
# GLEW_LIBRARY
#

IF (WIN32)
    FIND_PATH( GLEW_INCLUDE_PATH GL/glew.h
    $ENV{PROGRAMFILES}/GLEW/include
    ${PROJECT_SOURCE_DIR}/src/nvgl/glew/include
    DOC "The directory where GL/glew.h resides")
    FIND_LIBRARY( GLEW_LIBRARY
    NAMES glew GLEW glew32 glew32s
    PATHS
    /mingw/bin # for MinGW's MSYS
    /mingw/lib
    ${PROJECT_SOURCE_DIR}/glew/bin # or in local directory
    ${PROJECT_SOURCE_DIR}/glew/lib
    DOC "The GLEW library")
ELSE (WIN32)
    FIND_PATH( GLEW_INCLUDE_PATH GL/glew.h
    /usr/include
    /usr/local/include
    /sw/include
    /opt/local/include
    DOC "The directory where GL/glew.h resides")
    FIND_LIBRARY( GLEW_LIBRARY
    NAMES GLEW glew
    PATHS
    /usr/lib64
    /usr/lib
    /usr/local/lib64
    /usr/local/lib
    /sw/lib
    /opt/local/lib
    DOC "The GLEW library")
ENDIF (WIN32)

IF (GLEW_INCLUDE_PATH)
    SET( GLEW_FOUND 1 CACHE STRING "Set to 1 if GLEW is found, 0 otherwise")
ELSE (GLEW_INCLUDE_PATH)
    SET( GLEW_FOUND 0 CACHE STRING "Set to 1 if GLEW is found, 0 otherwise")
ENDIF (GLEW_INCLUDE_PATH)

MARK_AS_ADVANCED( GLEW_FOUND )

add_library(GLEW::GLEW INTERFACE IMPORTED)
target_include_directories(GLEW::GLEW SYSTEM INTERFACE ${GLEW_INCLUDE_DIR})
target_link_libraries(GLEW::GLEW INTERFACE ${GLEW_LIBRARY})
