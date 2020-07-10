# Note: LibSndFile exports targets itself with SndFileConfig.cmake,
#    However, the file is not for some reason provided in packages in some Linux distros, e.g. Ubuntu 18.04
#    Hence this Find*.cmake file.

FIND_PATH(SndFile_INCLUDE_DIR sndfile.h)
FIND_LIBRARY(SndFile_LIBRARY NAMES sndfile libsndfile sndfile-1)

find_package_handle_standard_args(
    SndFile
    DEFAULT_MSG
    SndFile_LIBRARY
    SndFile_INCLUDE_DIR
)

# Export targets
if(SndFile_FOUND AND NOT TARGET SndFile::sndfile)
    add_library(SndFile::sndfile UNKNOWN IMPORTED)
    set_target_properties(SndFile::sndfile PROPERTIES
                          IMPORTED_LOCATION "${SndFile_LIBRARY}"
                          INTERFACE_INCLUDE_DIRECTORIES "${SndFile_INCLUDE_DIR}")
    # If we want to statically link it, we also need its dependencies
    if(SNDFILE_STATIC)
        find_package(Ogg REQUIRED)
        find_package(FLAC REQUIRED)
        find_package(Vorbis REQUIRED)
        find_package(VorbisEnc REQUIRED)
        set_property(TARGET SndFile::sndfile APPEND PROPERTY
                     INTERFACE_LINK_LIBRARIES Ogg::ogg FLAC::FLAC Vorbis::vorbis Vorbis::VorbisEnc)
    endif()
endif()
