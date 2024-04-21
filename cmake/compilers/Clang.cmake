if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.1)
    message(FATAL_ERROR "${PROJECT_NAME} requires Clang 3.1 or greater.")
endif()

message(STATUS "Detected Clang version 3.1+")

if (${PLATFORM_FREEBSD})
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=bfd")
endif()

set(NORMAL_CXX_FLAGS "-Wall -Wold-style-cast -pedantic-errors -Wmissing-prototypes")
set(NORMAL_CXX_FLAGS "${NORMAL_CXX_FLAGS} -Wno-error=deprecated-declarations") # updated version of physfs is not available on some platforms so we keep using deprecated functions, see #958
set(RELEASE_CXX_FLAGS "-O2")
set(DEBUG_CXX_FLAGS "-g -O0")
set(TEST_CXX_FLAGS "-pthread")

add_definitions(-DHAVE_DEMANGLE)
