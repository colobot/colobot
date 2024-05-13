message(STATUS "Build for Mac OSX system")

set(PLATFORM_LINUX   0)
set(PLATFORM_GNU     0)
set(PLATFORM_MACOSX  1)
set(PLATFORM_FREEBSD 0)

add_compile_options(
    -Wall
    -Wold-style-cast
    -Wmissing-prototypes
    -pedantic-errors
    -Wno-error=deprecated-declarations

    # Fix compilation errors in MacOS SDK files
    -Wno-nullability-extension
    -Wno-nullability-completeness
    -Wno-expansion-to-defined
    -Wno-four-char-constants
    -Wno-gnu-zero-variadic-macro-arguments
    -Wno-variadic-macros
    -Wno-zero-length-array
    -Wno-missing-declarations
    -Wno-invalid-utf8
    -pthread
)

# To avoid CMake warning
set(CMAKE_MACOSX_RPATH 1)

set(PLATFORM_INSTALL_BIN_DIR "../MacOS")
set(PLATFORM_INSTALL_LIB_DIR "../MacOS")
set(PLATFORM_INSTALL_DATA_DIR ".")
set(PLATFORM_INSTALL_I18N_DIR "i18n")
set(PLATFORM_INSTALL_DOC_DIR "doc")

set(PLATFORM_SYSTEM_SOURCES
    system/system.cpp
    system/system.h

    system/system_macosx.cpp
    system/system_macosx.h

    CACHE INTERNAL "System sources"
)
