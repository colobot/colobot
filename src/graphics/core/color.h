// * This file is part of the COLOBOT source code
// * Copyright (C) 2012, Polish Portal of Colobot (PPC)
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.

/**
 * \file graphics/core/color.h
 * \brief Color structs and related functions
 */

#pragma once


#include <sstream>


// Graphics module namespace
namespace Gfx {

/**
 * \struct Color
 * \brief RGBA color
 */
struct Color
{
    //! Red, green, blue and alpha components
    float r, g, b, a;

    //! Constructor; default values are (0,0,0,0) = black
    explicit Color(float aR = 0.0f, float aG = 0.0f, float aB = 0.0f, float aA = 0.0f)
     : r(aR), g(aG), b(aB), a(aA) {}

    inline Color Inverse() const
    {
        return Color(1.0f - r, 1.0f - g, 1.0f - b, 1.0f - a);
    }

    //! Returns the struct cast to \c float* array; use with care!
    inline float* Array()
    {
        return reinterpret_cast<float*>(this);
    }

    //! Returns the struct cast to <tt>const float*</tt> array; use with care!
    inline const float* Array() const
    {
        return reinterpret_cast<const float*>(this);
    }

    //! Returns a string (r, g, b, a)
    inline std::string ToString() const
    {
        std::stringstream s;
        s.precision(3);
        s << "(" << r << ", " << g << ", " << b << ", " << a << ")";
        return s.str();
    }

    inline bool operator==(const Color &other) const
    {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }

    inline bool operator!=(const Color &other) const
    {
        return ! this->operator==(other);
    }

    inline Color operator*(float scale) const
    {
        Color c = *this;
        c.r *= scale;
        c.g *= scale;
        c.b *= scale;
        c.a *= scale;
        return c;
    }
};

/**
 * \struct IntColor
 * \brief Color with integer values
 *
 * May be used for precise pixel manipulations.
 */
struct IntColor
{
    //! Red, green, blue and alpha components
    unsigned char r, g, b, a;

    //! Constructor; default values are (0,0,0,0) = black
    explicit IntColor(unsigned char aR = 0, unsigned char aG = 0, unsigned char aB = 0, unsigned char aA = 0)
        : r(aR), g(aG), b(aB), a(aA) {}
};

inline Color IntColorToColor(IntColor color)
{
    return Color(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
}

inline IntColor ColorToIntColor(Color color)
{
    return IntColor(static_cast<unsigned char>(color.r * 255.0f),
                    static_cast<unsigned char>(color.g * 255.0f),
                    static_cast<unsigned char>(color.b * 255.0f),
                    static_cast<unsigned char>(color.a * 255.0f));
}

inline Color IntensityToColor(float intensity)
{
    if (intensity <= 0.0f) return Color(0.0f, 0.0f, 0.0f, 0.0f);
    if (intensity >= 1.0f) return Color(1.0f, 1.0f, 1.0f, 1.0f);

    return Color(intensity, intensity, intensity, intensity);
}

/**
 * \struct ColorHSV
 * \brief HSV color
 */
struct ColorHSV
{
    float h, s, v;

    ColorHSV(float aH = 0.0f, float aS = 0.0f, float aV = 0.0f)
     : h(aH), s(aS), v(aV) {}

     //! Returns a string "(h, s, v)"
     inline std::string ToString() const
     {
        std::stringstream s;
        s.precision(3);
        s << "(" << h << ", " << s << ", " << v << ")";
        return s.str();
     }
};

//! Converts a RGB color to HSV color
ColorHSV RGB2HSV(Color color);

//! Converts a HSV color to RGB color
Color HSV2RGB(ColorHSV color);


} // namespace Gfx
