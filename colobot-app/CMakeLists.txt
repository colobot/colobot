# Hide console on Windows (useful for release builds)
option(COLOBOT_HIDE_CONSOLE "Hide console" OFF)

add_executable(colobot)

target_sources(colobot PRIVATE
    src/main.cpp
)

if(MSVC)
    target_compile_options(colobot PRIVATE /utf-8)
endif()

if(PLATFORM_WINDOWS)
    target_sources(colobot PRIVATE ${PROJECT_BINARY_DIR}/desktop/colobot.rc)
endif()

target_link_libraries(colobot Colobot-Base SDL2::SDL2)

if(COLOBOT_HIDE_CONSOLE)
    set_target_properties(colobot PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# Copy to build directory for direct execution in MSVC
if(COLOBOT_DEVELOPMENT_MODE AND MSVC)
    add_custom_command(TARGET colobot POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:colobot> ${CMAKE_BINARY_DIR}
    )

    if(NOT CBOT_STATIC)
        add_custom_command(TARGET colobot POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:CBot> ${CMAKE_BINARY_DIR}
        )
    endif()
endif()

# Install
install(TARGETS colobot RUNTIME DESTINATION ${COLOBOT_INSTALL_BIN_DIR})

if(MSVC)
    install(FILES $<TARGET_PDB_FILE:colobot>
        CONFIGURATIONS Debug RelWithDebInfo
        DESTINATION ${COLOBOT_INSTALL_BIN_DIR} OPTIONAL)
endif()

if(NOT CBOT_STATIC)
    set_target_properties(colobot PROPERTIES
        INSTALL_RPATH ${COLOBOT_INSTALL_LIB_DIR})
endif()

# Linter-specific
if(COLOBOT_LINT_BUILD)
    add_fake_header_sources("src" colobot)
endif()
