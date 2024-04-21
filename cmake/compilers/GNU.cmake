if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.7)
    message(FATAL_ERROR "${PROJECT_NAME} requires GCC 4.7 or greater.")
endif()

message(STATUS "Detected GCC version 4.7+")

add_compile_options(
    -Wall
    -Wold-style-cast
    -pedantic-errors
    -Wmissing-declarations
    -lstdc++fs
    -fPIC
    -pthread
)

add_definitions(-DHAVE_DEMANGLE)
