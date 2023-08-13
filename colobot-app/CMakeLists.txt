# Hide console on Windows (useful for release builds)
option(COLOBOT_HIDE_CONSOLE "Hide console" OFF)

add_executable(colobot)

target_sources(colobot PRIVATE
    src/main.cpp
)

if(PLATFORM_WINDOWS)
    target_sources(colobot PRIVATE ${PROJECT_BINARY_DIR}/desktop/colobot.rc)
endif()

target_link_libraries(colobot colobotbase SDL2::Main)

if(COLOBOT_HIDE_CONSOLE)
    set_target_properties(colobot PROPERTIES WIN32_EXECUTABLE TRUE)
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