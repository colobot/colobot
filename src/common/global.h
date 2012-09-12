// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
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
 * \file common/global.h
 * \brief Some common, global definitions
 */

#pragma once

#include "error_ids.h"

/**
 * \enum Language
 * \brief Application language
 */
enum Language
{
    LANG_ENGLISH = 0,
    LANG_FRENCH = 1,
    LANG_GERMAN = 2,
    LANG_POLISH = 3
};

/**
 * \enum BuildType
 * \brief Construction actions (buildings, etc.) available to user
 *
 * TODO: refactor
 */
enum BuildType
{
    BUILD_FACTORY       = (1<<0),       //! < factory
    BUILD_DERRICK       = (1<<1),       //! < derrick
    BUILD_CONVERT       = (1<<2),       //! < converter
    BUILD_RADAR         = (1<<3),       //! < radar
    BUILD_ENERGY        = (1<<4),       //! < factory of cells
    BUILD_NUCLEAR       = (1<<5),       //! < nuclear power plant
    BUILD_STATION       = (1<<6),       //! < base station
    BUILD_REPAIR        = (1<<7),       //! < repair center
    BUILD_TOWER         = (1<<8),       //! < defense tower
    BUILD_RESEARCH      = (1<<9),       //! < research center
    BUILD_LABO          = (1<<10),      //! < laboratory
    BUILD_PARA          = (1<<11),      //! < lightning protection
    BUILD_INFO          = (1<<12),      //! < information terminal
    BUILD_GFLAT         = (1<<16),      //! < flat floor
    BUILD_FLAG          = (1<<17)       //! < puts / removes colored flag
};

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

/**
 * \enum KeyRank
 * \brief Slots for key assignment of user controls
 */
// TODO: remove (use the new InputSlot enum from app/app.h)
enum KeyRank
{
    KEYRANK_LEFT    = 0,
    KEYRANK_RIGHT   = 1,
    KEYRANK_UP      = 2,
    KEYRANK_DOWN    = 3,
    KEYRANK_GUP     = 4,
    KEYRANK_GDOWN   = 5,
    KEYRANK_CAMERA  = 6,
    KEYRANK_DESEL   = 7,
    KEYRANK_ACTION  = 8,
    KEYRANK_NEAR    = 9,
    KEYRANK_AWAY    = 10,
    KEYRANK_NEXT    = 11,
    KEYRANK_HUMAN   = 12,
    KEYRANK_QUIT    = 13,
    KEYRANK_HELP    = 14,
    KEYRANK_PROG    = 15,
    KEYRANK_VISIT   = 16,
    KEYRANK_SPEED10 = 17,
    KEYRANK_SPEED15 = 18,
    KEYRANK_SPEED20 = 19,
    KEYRANK_SPEED30 = 20,
    KEYRANK_AIMUP   = 21,
    KEYRANK_AIMDOWN = 22,
    KEYRANK_CBOT    = 23,

    KEYRANK_MAX
};

// TODO: move to CRobotMain
extern long     g_id;                   // unique identifier
extern long     g_build;                // constructible buildings
extern long     g_researchDone;         // research done
extern long     g_researchEnable;       // research available
extern float    g_unit;                 // conversion factor
