add_executable(CBot_console console.cpp)
target_link_directories(CBot_console PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)
target_link_libraries(CBot_console PRIVATE
    CBot
    Colobot-Base # Needed for error strings (TODO: why are they on Colobot side? :/)
)

add_executable(CBot_compile_graph compile_graph.cpp)
target_link_directories(CBot_compile_graph PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)
target_link_libraries(CBot_compile_graph CBot)

if(COLOBOT_LINT_BUILD)
    add_fake_header_sources("test/cbot" CBot_console)
    add_fake_header_sources("test/cbot" CBot_compile_graph)
endif()
