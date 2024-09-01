# PHYSFS_FOUND
# PHYSFS_INCLUDE_PATH
# PHYSFS_LIBRARY
#

IF (WIN32)
    FIND_PATH( PHYSFS_INCLUDE_PATH physfs.h
                DOC "The directory where physfs.h resides")
    FIND_LIBRARY( PHYSFS_LIBRARY
                    NAMES physfs physfs-static
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

INCLUDE (FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS (PhysFS
    DEFAULT_MSG
    PHYSFS_INCLUDE_PATH
    PHYSFS_LIBRARY
    )

MARK_AS_ADVANCED( PHYSFS_FOUND PHYSFS_INCLUDE_PATH PHYSFS_LIBRARY )

# Targets
if(PHYSFS_FOUND AND NOT TARGET PhysFS::PhysFS)
    add_library(PhysFS::PhysFS UNKNOWN IMPORTED)
    set_target_properties(PhysFS::PhysFS PROPERTIES
                          IMPORTED_LOCATION "${PHYSFS_LIBRARY}"
                          INTERFACE_INCLUDE_DIRECTORIES "${PHYSFS_INCLUDE_PATH}")
endif()
