# Compile flags as defined in global CMakeLists
set(CMAKE_CXX_FLAGS "${COLOBOT_CXX_FLAGS} ${MXE_CFLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE ${COLOBOT_CXX_FLAGS_RELEASE})
set(CMAKE_CXX_FLAGS_DEBUG ${COLOBOT_CXX_FLAGS_DEBUG})

# CBot utils
add_subdirectory(cbot)

# Unit tests
add_subdirectory(unit)

# Test environments
add_subdirectory(envs)
