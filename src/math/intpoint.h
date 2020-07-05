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

/**
 * \file math/intpoint.h
 * \brief IntPoint struct
 */

#pragma once

#include <cmath>
#include <string>
#include <sstream>

// Math module namespace
namespace Math
{

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

    //! Sets the zero point: (0,0)
    inline void LoadZero()
    {
        x = y = 0.0f;
    }

    //! Returns the struct cast to \c int* array; use with care!
    inline int* Array()
    {
        return reinterpret_cast<int*>(this);
    }

    //! Returns the struct cast to <tt>const int*</tt> array; use with care!
    inline const int* Array() const
    {
        return reinterpret_cast<const int*>(this);
    }

    //! Returns the inverted point
    inline IntPoint operator-() const
    {
        return IntPoint(-x, -y);
    }

    //! Adds the given point
    inline const IntPoint& operator+=(const IntPoint &right)
    {
        x += right.x;
        y += right.y;
        return *this;
    }

    //! Adds two points
    inline friend const IntPoint operator+(const IntPoint &left, const IntPoint &right)
    {
        return IntPoint(left.x + right.x, left.y + right.y);
    }

    //! Subtracts the given point
    inline const IntPoint& operator-=(const IntPoint &right)
    {
        x -= right.x;
        y -= right.y;
        return *this;
    }

    //! Subtracts two points
    inline friend const IntPoint operator-(const IntPoint &left, const IntPoint &right)
    {
        return IntPoint(left.x - right.x, left.y - right.y);
    }

    //! Multiplies by given scalar
    inline const IntPoint& operator*=(const float &right)
    {
        x *= right;
        y *= right;
        return *this;
    }

    //! Multiplies point by scalar
    inline friend const IntPoint operator*(const float &left, const IntPoint &right)
    {
        return IntPoint(left * right.x, left * right.y);
    }

    //! Multiplies point by scalar
    inline friend const IntPoint operator*(const IntPoint &left, const int &right)
    {
        return IntPoint(left.x * right, left.y * right);
    }

    //! Divides by given scalar
    inline const IntPoint& operator/=(const float &right)
    {
        x /= right;
        y /= right;
        return *this;
    }

    //! Divides point by scalar
    inline friend const IntPoint operator/(const IntPoint &left, const int &right)
    {
        return IntPoint(left.x / right, left.y / right);
    }

    //! Returns a string "[x, y]"
    inline std::string ToString() const
    {
        std::stringstream s;
        s << "[" << x << ", " << y << "]";
        return s.str();
    }
};


} // namespace Math

