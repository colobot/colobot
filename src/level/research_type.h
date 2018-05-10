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

#pragma once


/**
 * \enum ResearchType
 * \brief Research actions available to user
 */
enum ResearchType
{
    RESEARCH_TANK       = (1<<0),       //! < caterpillars
    RESEARCH_FLY        = (1<<1),       //! < wings
    RESEARCH_CANON      = (1<<2),       //! < cannon
    RESEARCH_TOWER      = (1<<3),       //! < defense tower
    RESEARCH_ATOMIC     = (1<<4),       //! < nuclear
    RESEARCH_THUMP      = (1<<5),       //! < thumper
    RESEARCH_SHIELD     = (1<<6),       //! < shield
    RESEARCH_PHAZER     = (1<<7),       //! < phazer gun
    RESEARCH_iPAW       = (1<<8),       //! < legs of insects
    RESEARCH_iGUN       = (1<<9),       //! < cannon of insects
    RESEARCH_RECYCLER   = (1<<10),      //! < recycler
    RESEARCH_SUBM       = (1<<11),      //! < submarine
    RESEARCH_SNIFFER    = (1<<12)       //! < sniffer
};
