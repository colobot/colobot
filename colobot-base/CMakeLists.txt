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

if(WINGETOPT)
    target_link_libraries(Colobot-Base PUBLIC wingetopt)
endif()

# Additional libraries per platform
if(COLOBOT_MSVC_STATIC_RUNTIME)
    find_library(BZ2_LIBRARY NAMES bz2)
    find_library(FREETYPE_LIBRARY NAMES freetype)
    find_library(ICONV_LIBRARY NAMES iconv)
    find_library(CHARSET_LIBRARY NAMES charset)
    find_library(BROTLICOMMON_LIBRARY NAMES brotlicommon-static)
    find_library(BROTLIENC_LIBRARY NAMES brotlienc-static)
    find_library(BROTLIDEC_LIBRARY NAMES brotlidec-static)

    target_link_libraries(Colobot-Base PUBLIC
        ${BZ2_LIBRARY}
        ${FREETYPE_LIBRARY}
        ${ICONV_LIBRARY}
        ${CHARSET_LIBRARY}
        ${BROTLICOMMON_LIBRARY}
        ${BROTLIENC_LIBRARY}
        ${BROTLIDEC_LIBRARY}

        # why
        winmm.lib
        dxguid.lib
        imm32.lib
        ole32.lib
        oleaut32.lib
        version.lib
        wsock32.lib
        ws2_32.lib
        setupapi.lib
        Dbghelp
    )
endif()

if(PLATFORM_LINUX)
    # for clock_gettime
    target_link_libraries(Colobot-Base PUBLIC -lrt)
endif()

# Linter-specific
if(COLOBOT_LINT_BUILD)
    add_fake_header_sources("src" Colobot-Base)
endif()
