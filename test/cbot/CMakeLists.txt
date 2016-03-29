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

add_executable(CBot_console console.cpp)
target_link_libraries(CBot_console ${LIBS})

add_executable(CBot_compile_graph compile_graph.cpp)
target_link_libraries(CBot_compile_graph CBot)