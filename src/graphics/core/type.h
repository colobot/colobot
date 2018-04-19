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
* \file graphics/core/type.h
* \brief Type support and conversion
*/

#pragma once

// Graphics module namespace
namespace Gfx
{

/**
* \enum class Type
* \brief Value types for vertex attributes
*/
enum class Type : unsigned char
{
    //! Unsigned byte (8-bit)
    UBYTE = 0,
    //! Signed byte (8-bit)
    BYTE,
    //! Unsigned short (16-bit)
    USHORT,
    //! Signed short (16-bit)
    SHORT,
    //! Unsigned int (32-bit)
    UINT,
    //! Signed int (32-bit)
    INT,
    //! Half precision floating-point (16-bit)
    HALF,
    //! Single precision floating-point (32-bit)
    FLOAT,
    //! Double precision floating-point (64-bit)
    DOUBLE,
};

//! Returns size in bytes of given type
int GetTypeSize(Type type);

// TODO: functions for conversion between types

} // namespace Gfx
