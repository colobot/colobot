add_executable(CBot-CompileGraph
    src/compile_graph.cpp
)

target_link_directories(CBot-CompileGraph PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(CBot-CompileGraph PRIVATE CBot)

if(COLOBOT_LINT_BUILD)
    add_fake_header_sources("test/cbot" CBot-CompileGraph)
endif()
