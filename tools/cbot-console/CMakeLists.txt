add_executable(CBot-Console
    src/console.cpp
)

target_link_directories(CBot-Console PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(CBot-Console PRIVATE
    CBot
    Colobot-Base # Needed for error strings (TODO: why are they on Colobot side? :/)
)
