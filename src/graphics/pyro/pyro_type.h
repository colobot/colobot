/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2020, Daniel Roux, EPSITEC SA & TerranovaTeam
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
 * \file graphics/pyro/pyro_type.h
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

    // CFragExploOrShotPyro
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

    // Other subclasses
    PT_BURNT    = 14,       //! < burning of technical object
    PT_BURNO    = 15,       //! < burning of organic object
    PT_WIN      = 22,       //! < fireworks
    PT_LOST     = 23,       //! < black smoke

    PT_OTHER    = 100,      //! < No special type code; behaviour is decided by subclass
};

} // namespace Gfx
