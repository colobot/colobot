if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.1)
    message(FATAL_ERROR "${PROJECT_NAME} requires Clang 3.1 or greater.")
endif()

message(STATUS "Detected Clang version 3.1+")

if (${PLATFORM_FREEBSD})
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=bfd")
endif()

add_compile_options(
    -Wall
    -Wold-style-cast
    -pedantic-errors
    -Wmissing-prototypes
    -Wno-error=deprecated-declarations      # updated version of physfs is not available on some platforms so we keep using deprecated functions, see #958
    -pthread
)

add_definitions(-DHAVE_DEMANGLE)
