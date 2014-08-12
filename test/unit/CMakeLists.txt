# Platform-dependent tests
if(PLATFORM_WINDOWS)
    set(PLATFORM_TESTS app/system_windows_test.cpp)
elseif(PLATFORM_LINUX)
    set(PLATFORM_TESTS app/system_linux_test.cpp)
endif()

# Sources
set(UT_SOURCES
    main.cpp
    app/app_test.cpp
    common/profile_test.cpp
    graphics/engine/lightman_test.cpp
    math/func_test.cpp
    math/geometry_test.cpp
    math/matrix_test.cpp
    math/vector_test.cpp
    ${PLATFORM_TESTS}
)

# Includes
include_directories(
    common
    math
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${COLOBOT_LOCAL_INCLUDES}
)

include_directories(
    SYSTEM
    ${GTEST_INCLUDE_DIR}
    ${GMOCK_INCLUDE_DIR}
    ${COLOBOT_SYSTEM_INCLUDES}
)

# Libraries
set(LIBS
    gtest
    gmock
    colobotbase
    ${COLOBOT_LIBS}
)

# Test files

set(TEST_FILES
    common/colobot.ini
)

file(COPY ${TEST_FILES} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# Targets

add_executable(colobot_ut ${UT_SOURCES})
target_link_libraries(colobot_ut ${LIBS})

add_test(
    NAME colobot_ut
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/colobot_ut
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
