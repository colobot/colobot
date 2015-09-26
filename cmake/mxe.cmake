# When cross-compiling with MXE, we need to straighten some things

# Checking is a bit primitive, but this should detect MXE toolchain file
if((${CMAKE_CROSSCOMPILING}) AND (DEFINED MSYS))
    message(STATUS "Detected MXE build")
    set(MXE 1)
    set(PLATFORM_WINDOWS 1)
    set(PLATFORM_LINUX 0)
    set(PLATFORM_OTHER 0)
    # Because some tests will not compile
    set(TESTS OFF)
    # All must be static, CBOT and GLEW too
    set(CBOT_STATIC ON)
    set(GLEW_STATIC ON)

    if (${OPENAL_SOUND})
        set(OPENAL_MXE_LIBS
            ${CMAKE_FIND_ROOT_PATH}/lib/libFLAC.a
            ${CMAKE_FIND_ROOT_PATH}/lib/libvorbis.a
            ${CMAKE_FIND_ROOT_PATH}/lib/libvorbisenc.a
            ${CMAKE_FIND_ROOT_PATH}/lib/libvorbisfile.a
            ${CMAKE_FIND_ROOT_PATH}/lib/libogg.a
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
      ${CMAKE_FIND_ROOT_PATH}/lib/libimm32.a
      ${CMAKE_FIND_ROOT_PATH}/lib/libole32.a
      ${CMAKE_FIND_ROOT_PATH}/lib/liboleaut32.a
      ${CMAKE_FIND_ROOT_PATH}/lib/libharfbuzz.a
      ${CMAKE_FIND_ROOT_PATH}/lib/libwebp.a
      ${CMAKE_FIND_ROOT_PATH}/lib/libversion.a
      ${CMAKE_FIND_ROOT_PATH}/lib/libglib-2.0.a
      ${CMAKE_FIND_ROOT_PATH}/lib/libwsock32.a
      ${CMAKE_FIND_ROOT_PATH}/lib/libws2_32.a
      ${CMAKE_FIND_ROOT_PATH}/lib/libintl.a
      ${OPENAL_MXE_LIBS}
    )
else()
    set(MXE 0)
endif()
