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


namespace Gfx {

/**
  \struct Color
  \brief RGBA color */
struct Color
{
    //! Red, green, blue and alpha components
    float r, g, b, a;

    //! Constructor; default values are (0,0,0,0) = black
    explicit Color(float aR = 0.0f, float aG = 0.0f, float aB = 0.0f, float aA = 0.0f)
     : r(aR), g(aG), b(aB), a(aA) {}

    inline Gfx::Color Inverse() const
    {
        return Gfx::Color(1.0f - r, 1.0f - g, 1.0f - b, 1.0f - a);
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

    inline bool operator==(const Gfx::Color &other) const
    {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }

    inline bool operator!=(const Gfx::Color &other) const
    {
        return ! this->operator==(other);
    }
};

/**
  \struct ColorHSV
  \brief HSV color */
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
Gfx::ColorHSV RGB2HSV(Gfx::Color color);

//! Converts a HSV color to RGB color
Gfx::Color HSV2RGB(Gfx::ColorHSV color);

}; // namespace Gfx

