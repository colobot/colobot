#pragma once

#include <string_view>

namespace Version
{
    static inline constexpr bool OFFICIAL_BUILD = @CONFIG_OFFICIAL_BUILD@;
    static inline constexpr bool DEVELOPMENT_BUILD = @CONFIG_DEVELOPMENT_BUILD@;
    static inline constexpr bool PORTABLE_SAVES = @CONFIG_PORTABLE_SAVES@;
    static inline constexpr bool RELATIVE_PATHS = @CONFIG_RELATIVE_PATHS@;

    static inline constexpr int BUILD_NUMBER = @BUILD_NUMBER@;

    static inline constexpr std::string_view FULL_NAME = "Colobot: Gold Edition @COLOBOT_VERSION_FULL@";
    static inline constexpr std::string_view VERSION = "@CMAKE_PROJECT_VERSION@";
    static inline constexpr int MAJOR = @CMAKE_PROJECT_VERSION_MAJOR@;
    static inline constexpr int MINOR = @CMAKE_PROJECT_VERSION_MINOR@;
    static inline constexpr int PATCH = @CMAKE_PROJECT_VERSION_PATCH@;
    static inline constexpr int TWEAK = @CMAKE_PROJECT_VERSION_TWEAK@;
    static inline constexpr std::string_view VERSION_DISPLAY = "@COLOBOT_VERSION_DISPLAY@";
}
