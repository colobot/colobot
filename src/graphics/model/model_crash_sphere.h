#pragma once

#include "math/vector.h"

namespace Gfx {

/**
 * \struct ModelCrashSphere
 * \brief Crash sphere data as saved in model file
 */
struct ModelCrashSphere
{
    Math::Vector position;
    float radius = 0.0f;
    std::string sound;
    float hardness = 0.0f;
};

} // namespace Gfx
