add_library(Colobot-Common STATIC)

target_sources(Colobot-Common PRIVATE
    src/common/stringutils.cpp
    src/common/stringutils.h
)

if(WIN32)
    target_sources(Colobot-Common PRIVATE
        src/common/stringutils_windows.cpp
    )
else()
    target_sources(Colobot-Common PRIVATE
        src/common/stringutils_generic.cpp
    )
endif()

target_include_directories(Colobot-Common PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)
