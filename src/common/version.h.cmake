#pragma once

#include <string_view>

namespace Version
{
    static inline constexpr bool OFFICIAL_BUILD = @OFFICIAL_COLOBOT_BUILD@;
    static inline constexpr bool DEVELOPMENT_BUILD = @DEVELOPMENT_COLOBOT_BUILD@;
    static inline constexpr int BUILD_NUMBER = @BUILD_NUMBER@;

    static inline constexpr std::string_view FULL_NAME = "Colobot: Gold Edition @COLOBOT_VERSION_FULL@";
    static inline constexpr std::string_view VERSION = "@CMAKE_PROJECT_VERSION@";
    static inline constexpr std::string_view VERSION_DISPLAY = "@COLOBOT_VERSION_DISPLAY@";
};
