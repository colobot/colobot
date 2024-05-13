message(STATUS "Build for kFreeBSD system")

set(PLATFORM_GNU     1)
set(PLATFORM_MACOSX  0)
set(PLATFORM_FREEBSD 0)

set(PLATFORM_INSTALL_BIN_DIR "games")
set(PLATFORM_INSTALL_LIB_DIR "lib/colobot")
set(PLATFORM_INSTALL_DATA_DIR "share/games/colobot")
set(PLATFORM_INSTALL_I18N_DIR "share/locale")
set(PLATFORM_INSTALL_DOC_DIR "share/doc/colobot")

set(PLATFORM_SYSTEM_SOURCES
    system/system.cpp
    system/system.h

    system/system_other.cpp
    system/system_other.h

    CACHE INTERNAL "System sources"
)
