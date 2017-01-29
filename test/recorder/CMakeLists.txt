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
    ${HIPPOMOCKS_INCLUDE_DIR}
    ${COLOBOT_SYSTEM_INCLUDES}
)

# Libraries
set(LIBS
    gtest
    colobotbase
    ${COLOBOT_LIBS}
)

# Sources
set(SOURCES
    app_mock.cpp
    controller_mock.cpp
    event_queue_mock.cpp
    input_mock.cpp
    main.cpp
    object_maker_mock.cpp
    recorder.cpp)

add_executable(colobot_recorder ${SOURCES})
target_link_libraries(colobot_recorder ${LIBS})

# Copy records from source directory to build directory
add_custom_command(TARGET colobot_recorder PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       "${CMAKE_CURRENT_SOURCE_DIR}/records/" "${CMAKE_CURRENT_BINARY_DIR}/../../records/")
