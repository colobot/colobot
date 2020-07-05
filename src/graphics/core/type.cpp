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
* \file graphics/core/type.cpp
* \brief Type support and conversion
*/

#include "graphics/core/type.h"

#include <cassert>

// Graphics module namespace
namespace Gfx
{

//! Returns size in bytes of given type
int GetTypeSize(Type type)
{
    switch (type)
    {
    case Type::BYTE:
    case Type::UBYTE:
        return 1;
    case Type::SHORT:
    case Type::USHORT:
    case Type::HALF:
        return 2;
    case Type::INT:
    case Type::UINT:
    case Type::FLOAT:
        return 4;
    case Type::DOUBLE:
        return 8;
    default:
        return 0;
    }
}

} // namespace Gfx
