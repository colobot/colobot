add_library(Colobot-Common STATIC)

target_sources(Colobot-Common PRIVATE
    src/common/stringutils.cpp
    src/common/stringutils.h
)

target_include_directories(Colobot-Common PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)
