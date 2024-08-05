add_library(Colobot-Common STATIC)

set_target_properties(Colobot-Common PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

target_sources(Colobot-Common PRIVATE
    src/common/codepoint.h
    src/common/stringutils.cpp
    src/common/stringutils_case.cpp
    src/common/stringutils.h
)

target_include_directories(Colobot-Common PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

if(COLOBOT_CHAR8_T_OVERRIDE)
    target_compile_definitions(Colobot-Common PRIVATE
        COLOBOT_CHAR8_T_OVERRIDE=1
    )
endif()
