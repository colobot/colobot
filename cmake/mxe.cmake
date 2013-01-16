# When cross-compiling with MXE, we need to straighten some things

# Checking is a bit primitive, but this should detect MXE toolchain file
if((${CMAKE_CROSSCOMPILING}) AND (DEFINED MSYS))
    message(STATUS "Detected MXE build")
    set(MXE 1)
    # Because some tests will not compile
    set(TESTS OFF)
    # All must be static, CBOT and GLEW too
    set(CBOT_STATIC ON)
    set(GLEW_STATIC ON)
    # Because find package scripts are lame
    set(SDLTTF_INCLUDE_DIR ${CMAKE_FIND_ROOT_PATH}/include/SDL)
    set(SDLIMAGE_INCLUDE_DIR ${CMAKE_FIND_ROOT_PATH}/include/SDL)

    if (${OPENAL_SOUND})
        set(OPENAL_MXE_LIBS
            ${CMAKE_FIND_ROOT_PATH}/lib/libFLAC.a
            ${CMAKE_FIND_ROOT_PATH}/lib/libvorbis.a
            ${CMAKE_FIND_ROOT_PATH}/lib/libvorbisenc.a
            ${CMAKE_FIND_ROOT_PATH}/lib/libvorbisfile.a
            ${CMAKE_FIND_ROOT_PATH}/lib/libogg.a
            ${CMAKE_FIND_ROOT_PATH}/lib/libwsock32.a
        )
    endif()

    set(MXE_CFLAGS "-DAL_LIBTYPE_STATIC")
    set(MXE_LIBS
      # For some reason, these have to be absolute paths
      ${CMAKE_FIND_ROOT_PATH}/lib/libintl.a
      ${CMAKE_FIND_ROOT_PATH}/lib/libiconv.a
      ${CMAKE_FIND_ROOT_PATH}/lib/libglew32s.a
      ${CMAKE_FIND_ROOT_PATH}/lib/libfreetype.a
      ${CMAKE_FIND_ROOT_PATH}/lib/libopengl32.a
      ${CMAKE_FIND_ROOT_PATH}/lib/libjpeg.a
      ${CMAKE_FIND_ROOT_PATH}/lib/libwinmm.a
      ${CMAKE_FIND_ROOT_PATH}/lib/libdxguid.a
      ${CMAKE_FIND_ROOT_PATH}/lib/libbz2.a
      ${OPENAL_MXE_LIBS}
    )
else()
    set(MXE 0)
endif()