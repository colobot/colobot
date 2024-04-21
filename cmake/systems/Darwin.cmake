message(STATUS "Build for Mac OSX system")

set(PLATFORM_WINDOWS 0)
set(PLATFORM_LINUX   0)
set(PLATFORM_GNU     0)
set(PLATFORM_MACOSX  1)
set(PLATFORM_OTHER   0)
set(PLATFORM_FREEBSD 0)

# Fix compilation errors in MacOS SDK files
set(CMAKE_CXX_FLAGS "${NORMAL_CXX_FLAGS} -Wno-nullability-extension -Wno-nullability-completeness -Wno-expansion-to-defined -Wno-four-char-constants -Wno-gnu-zero-variadic-macro-arguments -Wno-variadic-macros -Wno-zero-length-array -Wno-missing-declarations -Wno-invalid-utf8")
# To avoid CMake warning
set(CMAKE_MACOSX_RPATH 1)

set(PLATFORM_INSTALL_BIN_DIR "../MacOS")
set(PLATFORM_INSTALL_LIB_DIR "../MacOS")
set(PLATFORM_INSTALL_DATA_DIR ".")
set(PLATFORM_INSTALL_I18N_DIR "i18n")
set(PLATFORM_INSTALL_DOC_DIR "doc")
