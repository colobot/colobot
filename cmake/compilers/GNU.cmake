if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.7)
    message(FATAL_ERROR "${PROJECT_NAME} requires GCC 4.7 or greater.")
endif()

message(STATUS "Detected GCC version 4.7+")

set(NORMAL_CXX_FLAGS "-Wall -Wold-style-cast -pedantic-errors -Wmissing-declarations -lstdc++fs -fPIC")
set(NORMAL_CXX_FLAGS "${NORMAL_CXX_FLAGS} -Wno-error=deprecated-declarations") # updated version of physfs is not available on some platforms so we keep using deprecated functions, see #958

if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0)
set(NORMAL_CXX_FLAGS "${NORMAL_CXX_FLAGS} -Wsuggest-override")
endif()

set(RELEASE_CXX_FLAGS "-O2")
set(DEBUG_CXX_FLAGS "-g -O0")
set(TEST_CXX_FLAGS "-pthread")

add_definitions(-DHAVE_DEMANGLE)
