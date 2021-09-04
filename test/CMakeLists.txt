# Compile flags as defined in global CMakeLists
set(CMAKE_CXX_FLAGS "${COLOBOT_CXX_FLAGS} ${COLOBOT_GTEST_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE ${COLOBOT_CXX_FLAGS_RELEASE})
set(CMAKE_CXX_FLAGS_DEBUG ${COLOBOT_CXX_FLAGS_DEBUG})

# Unit tests
add_subdirectory(unit)

# CBot tests
add_subdirectory(cbot)
