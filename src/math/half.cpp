/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsitec.ch; http://colobot.info; http://github.com/colobot
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://gnu.org/licenses
 */

#include "math/half.h"

#include <cmath>
#include <iomanip>
#include <limits>

// Math module namespace
namespace Math
{

//! Converts float to half-float
uint16_t FloatToHalf(float value)
{
    uint16_t sign = (std::copysign(1.0f, value) > 0.0f ? 0x0000 : 0x8000);

    // Infinity
    if (std::isinf(value))
    {
        return sign | 0x7C00;
    }
    // NaN
    else if (std::isnan(value))
    {
        return sign | 0x7FFF;
    }

    int exponent;

    float significand = std::fabs(std::frexp(value, &exponent));

    // Exponent bias
    exponent += 15;

    // Crosses upper boundary, clamp to infinity
    if (exponent > 31)
    {
        return sign | 0x7C00;
    }
    // Crosses lower boundary, clamp to zero
    else if (exponent <= 0)
    {
        return sign | 0x0000;
    }
    // Zero
    else if (significand < 0.25f)
    {
        return sign | 0x0000;
    }

    // Normal value
    uint16_t mantissa = static_cast<uint16_t>(std::ldexp(2 * significand - 1, 10));

    uint16_t bits = sign | mantissa | ((exponent - 1) << 10);

    return bits;
}

//! Converts half-float to float
float HaltToFloat(uint16_t value)
{
    int exponent = (value >> 10) & 0x001F;
    int mantissa = (value >> 0) & 0x03FF;

    float result;

    // Zero
    if ((exponent == 0) && (mantissa == 0))
    {
        result = 0.0f;
    }
    // Subnormal
    else if ((exponent == 0) && (mantissa != 0))
    {
        result = std::ldexp(static_cast<float>(mantissa), -24);
    }
    // Infinity
    else if ((exponent == 31) && (mantissa == 0))
    {
        result = std::numeric_limits<float>::infinity();
    }
    // NaN
    else if ((exponent == 31) && (mantissa != 0))
    {
        result = std::nanf("");
    }
    // Normal number
    else
    {
        result = std::ldexp(static_cast<float>(mantissa | 0x0400), exponent - 25);
    }

    float sign = ((value & 0x8000) == 0 ? 1.0f : -1.0f);

    return std::copysignf(result, sign);
}

} // namespace Math
