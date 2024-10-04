if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 12.0)
    message(FATAL_ERROR "${PROJECT_NAME} requires AppleClang 12.0 or greater.")
endif()

message(STATUS "Detected AppleClang version 12.0+")

if (${PLATFORM_FREEBSD})
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=bfd")
endif()

add_definitions(-DHAVE_DEMANGLE)
