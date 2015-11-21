# Sources
set(CBOT_CONSOLE_SOURCES
    main.cpp
)

# Includes
include_directories(
        ${COLOBOT_LOCAL_INCLUDES}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
)

# Libraries
set(LIBS
    CBot
    colobotbase # Needed for error strings (TODO: why are they on Colobot side? :/)
    ${COLOBOT_LIBS} # Needed for colobotbase
)
# Targets

add_executable(CBot_console ${CBOT_CONSOLE_SOURCES})
target_link_libraries(CBot_console ${LIBS})