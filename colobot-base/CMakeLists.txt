# Todo: split this monstrosity into smaller libraries (graphics, math, ui, etc.)
add_library(Colobot-Base STATIC)

set_target_properties(Colobot-Base PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

add_subdirectory(src)

target_include_directories(Colobot-Base PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}/src
)

# Main libraries
target_link_libraries(Colobot-Base PUBLIC
    CBot
    Colobot-Common
    localename
    SDL2::Core
    SDL2::Image
    SDL2::TTF
    OpenGL::GL
    PNG::PNG
    GLEW::GLEW
    glm::glm
    PhysFS::PhysFS
    SndFile::sndfile
    nlohmann_json::nlohmann_json
    Intl::Intl
)

# Optional libraries
if(OPENAL_SOUND)
    target_link_libraries(Colobot-Base PUBLIC OpenAL::OpenAL)
endif()

# Additional libraries per platform
if(PLATFORM_ADDITIONAL_LIBRARIES)
    target_link_libraries(Colobot-Base PUBLIC
        ${PLATFORM_ADDITIONAL_LIBRARIES}
    )
endif()

# Linter-specific
if(COLOBOT_LINT_BUILD)
    add_fake_header_sources("src" Colobot-Base)
endif()
