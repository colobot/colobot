if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 12.0)
    message(FATAL_ERROR "${PROJECT_NAME} requires AppleClang 12.0 or greater.")
endif()

message(STATUS "Detected AppleClang version 12.0+")

# Use override for char8_t on AppleClang before 14.0
if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 14.0)
    if(NOT DEFINED COLOBOT_CHAR8_T_OVERRIDE)
        set(COLOBOT_CHAR8_T_OVERRIDE ON CACHE BOOL "Override char8_t usage for compatibility reasons" FORCE)
    endif()
endif()

if (${PLATFORM_FREEBSD})
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=bfd")
endif()

add_compile_options(
    -Wall
    -Wold-style-cast
    -pedantic-errors
    -Wmissing-prototypes
    -Wno-error=deprecated-declarations  # updated version of physfs is not available on some platforms so we keep using deprecated functions, see #958
    -Wno-error=nullability-extension
    -pthread
)

add_definitions(-DHAVE_DEMANGLE)
