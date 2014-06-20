# PHYSFS_FOUND
# PHYSFS_INCLUDE_PATH
# PHYSFS_LIBRARY
#

IF (WIN32)
    FIND_PATH( PHYSFS_INCLUDE_PATH physfs.h
                DOC "The directory where physfs.h resides")
    FIND_LIBRARY( PHYSFS_LIBRARY
                    NAMES physfs
                    PATHS /mingw/lib
                    DOC "The PhysFS library")
ELSE (WIN32)
    FIND_PATH( PHYSFS_INCLUDE_PATH physfs.h
        /usr/include
        /usr/local/include
        /opt/local/include
        DOC "The directory where physfs.h resides")
    FIND_LIBRARY( PHYSFS_LIBRARY
        NAMES physfs
        PATHS
        /usr/lib64
        /usr/lib
        /usr/local/lib64
        /usr/local/lib
        /opt/local/lib
        DOC "The PhysFS library")
ENDIF (WIN32)

IF (PHYSFS_INCLUDE_PATH)
    SET( PHYSFS_FOUND 1 CACHE STRING "Set to 1 if PhysFS is found, 0 otherwise")
ELSE (GLEW_INCLUDE_PATH)
    SET( PHYSFS_FOUND 0 CACHE STRING "Set to 1 if PhysFS is found, 0 otherwise")
ENDIF (PHYSFS_INCLUDE_PATH)

MARK_AS_ADVANCED( PHYSFS_FOUND )
