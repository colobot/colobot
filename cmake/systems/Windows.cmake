message(STATUS "Build for Windows system")

set(PLATFORM_GNU     0)
set(PLATFORM_LINUX   0)
set(PLATFORM_MACOSX  0)
set(PLATFORM_FREEBSD 0)

set(PLATFORM_INSTALL_BIN_DIR "")
set(PLATFORM_INSTALL_LIB_DIR "")
set(PLATFORM_INSTALL_DATA_DIR "data")
set(PLATFORM_INSTALL_I18N_DIR "lang")
set(PLATFORM_INSTALL_DOC_DIR "doc")

add_subdirectory(${PROJECT_SOURCE_DIR}/lib/wingetopt)

list(APPEND WINDOWS_ADDITIONAL_LIBRARIES
    wingetopt
)

# Additional libraries for static runtime
if(COLOBOT_MSVC_STATIC_RUNTIME)
    find_library(BZ2_LIBRARY NAMES bz2)
    find_library(FREETYPE_LIBRARY NAMES freetype)
    find_library(ICONV_LIBRARY NAMES iconv)
    find_library(CHARSET_LIBRARY NAMES charset)
    find_library(BROTLICOMMON_LIBRARY NAMES brotlicommon-static)
    find_library(BROTLIENC_LIBRARY NAMES brotlienc-static)
    find_library(BROTLIDEC_LIBRARY NAMES brotlidec-static)

    list(APPEND WINDOWS_ADDITIONAL_LIBRARIES
        ${BZ2_LIBRARY}
        ${FREETYPE_LIBRARY}
        ${ICONV_LIBRARY}
        ${CHARSET_LIBRARY}
        ${BROTLICOMMON_LIBRARY}
        ${BROTLIENC_LIBRARY}
        ${BROTLIDEC_LIBRARY}

        # why
        winmm.lib
        dxguid.lib
        imm32.lib
        ole32.lib
        oleaut32.lib
        version.lib
        wsock32.lib
        ws2_32.lib
        setupapi.lib
        Dbghelp
    )
endif()

set(PLATFORM_ADDITIONAL_LIBRARIES
    ${WINDOWS_ADDITIONAL_LIBRARIES}
    CACHE INTERNAL "Additional libraries"
)

set(PLATFORM_SYSTEM_SOURCES
    system/system.cpp
    system/system.h

    system/system_windows.cpp
    system/system_windows.h

    CACHE INTERNAL "System sources"
)
