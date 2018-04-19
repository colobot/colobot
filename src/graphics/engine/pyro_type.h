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
 * \file graphics/engine/pyro_type.h
 * \brief PyroType enum
 */

#pragma once

namespace Gfx
{

/**
 * \enum PyroType
 * \brief Type of pyro effect
 */
enum PyroType
{
    PT_NULL     = 0,
    PT_FRAGT    = 1,        //! < fragmentation of technical object
    PT_FRAGO    = 2,        //! < fragmentation of organic object
    PT_FRAGW    = 4,        //! < fragmentation of object under water
    PT_EXPLOT   = 5,        //! < explosion of technical object
    PT_EXPLOO   = 6,        //! < explosion of organic object
    PT_EXPLOW   = 8,        //! < explosion of object under water
    PT_SHOTT    = 9,        //! < hit technical object
    PT_SHOTH    = 10,       //! < hit human
    PT_SHOTM    = 11,       //! < hit queen
    PT_SHOTW    = 12,       //! < hit under water (TODO: check if unused)
    PT_EGG      = 13,       //! < break the egg
    PT_BURNT    = 14,       //! < burning of technical object
    PT_BURNO    = 15,       //! < burning of organic object
    PT_SPIDER   = 16,       //! < spider explosion
    PT_FALL     = 17,       //! < cargo falling
    PT_WPCHECK  = 18,       //! < indicator reaches
    PT_FLCREATE = 19,       //! < flag create
    PT_FLDELETE = 20,       //! < flag destroy
    PT_RESET    = 21,       //! < reset position of the object
    PT_WIN      = 22,       //! < fireworks
    PT_LOST     = 23,       //! < black smoke
    PT_DEADG    = 24,       //! < shooting death
    PT_DEADW    = 25,       //! < drowning death
    PT_FINDING  = 26,       //! < object discovered
};

} // namespace Gfx
