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
 * \file math/intpoint.h
 * \brief IntPoint struct
 */

#pragma once

#include <cmath>

// Math module namespace
namespace Math {

/**
 * \struct IntPoint
 * \brief 2D Point with integer coords
 *
 * Analog of WinAPI's POINT struct.
 */
struct IntPoint
{
    //! X coord
    int x;
    //! Y coord
    int y;

    IntPoint(int aX = 0, int aY = 0) : x(aX), y(aY) {}

    inline bool operator==(const IntPoint& p) const
    {
        return x == p.x && y == p.y;
    }

    inline bool operator!=(const IntPoint& p) const
    {
        return !operator==(p);
    }

    inline float Length() const
    {
        return sqrtf(x*x + y*y);
    }
};


} // namespace Math

