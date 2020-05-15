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
* \file math/half.h
* \brief Implementation of half-precision floating point values.
*/

#pragma once

#include <cstdint>

// Math module namespace
namespace Math
{

//! Converts float to half-float binary representation
uint16_t FloatToHalf(float value);

//! Converts half-float binary representation to float
float HaltToFloat(uint16_t value);

//@colobot-lint-exclude ClassNamingRule
/**
* \struct half
* \brief half-precision floating point type
*
* Represents a half-precision floating point value.
* Contains the required methods for converting to and from ints and floats.
*
* This type is for storage only.
* Conversion is expensive and should be avoided if possible.
*/
struct half
{
    //! 16-bit binary representation of half-float
    uint16_t bits;

    //! Default constructor
    half()
        : bits(0)
    {

    }

    //! Copy constructor
    half(const half& other)
        : bits(other.bits)
    {

    }

    //! Copy operator
    half& operator=(const half& other)
    {
        bits = other.bits;

        return *this;
    }

    //! Conversion constructor from int
    explicit half(int value)
        : bits(FloatToHalf(static_cast<float>(value)))
    {

    }

    //! Conversion constructor from float
    explicit half(float value)
        : bits(FloatToHalf(value))
    {

    }

    //! Conversion operator to int
    explicit operator int() const
    {
        return static_cast<int>(HaltToFloat(bits));
    }

    //! Conversion operator to float
    explicit operator float() const
    {
        return HaltToFloat(bits);
    }
};
//@end-colobot-lint-exclude

} // namespace Math
