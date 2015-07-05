/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2015, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsiteс.ch; http://colobot.info; http://github.com/colobot
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

#include <string>

/**
 * \public
 * \enum    SoundType
 * \brief   Enum representing sound file
**/
enum SoundType
{
  SOUND_NONE = -1,
  SOUND_CLICK = 0,
  SOUND_BOUM  = 1,
  SOUND_EXPLO  = 2,
  SOUND_FLYh  = 3, /*!< human */
  SOUND_FLY  = 4,
  SOUND_STEPs  = 5, /*!< smooth */
  SOUND_MOTORw  = 6, /*!< wheel */
  SOUND_MOTORt  = 7, /*!< tank */
  SOUND_MOTORr  = 8, /*!< roller */
  SOUND_ERROR  = 9,
  SOUND_CONVERT  = 10,
  SOUND_ENERGY  = 11,
  SOUND_PLOUF  = 12,
  SOUND_BLUP  = 13,
  SOUND_WARNING  = 14,
  SOUND_DERRICK  = 15,
  SOUND_LABO  = 16,
  SOUND_STATION  = 17,
  SOUND_REPAIR  = 18,
  SOUND_RESEARCH  = 19,
  SOUND_INSECTs  = 20, /*!< spider */
  SOUND_BURN  = 21,
  SOUND_TZOING  = 22,
  SOUND_GGG  = 23,
  SOUND_MANIP  = 24,
  SOUND_FIRE  = 25, /*!< shooting with fireball */
  SOUND_HUMAN1  = 26, /*!< breathing */
  SOUND_STEPw  = 27, /*!< water */
  SOUND_SWIM  = 28,
  SOUND_RADAR  = 29,
  SOUND_BUILD  = 30,
  SOUND_ALARM  = 31, /*!< energy alarm */
  SOUND_SLIDE  = 32,
  SOUND_EXPLOi  = 33, /*!< insect */
  SOUND_INSECTa  = 34, /*!< ant */
  SOUND_INSECTb  = 35, /*!< bee */
  SOUND_INSECTw  = 36, /*!< worm */
  SOUND_INSECTm  = 37, /*!< mother */
  SOUND_TREMBLE  = 38,
  SOUND_PSHHH  = 39,
  SOUND_NUCLEAR  = 40,
  SOUND_INFO  = 41,
  SOUND_OPEN  = 42,
  SOUND_CLOSE  = 43,
  SOUND_FACTORY  = 44,
  SOUND_EGG  = 45,
  SOUND_MOTORs  = 46, /*!< submarine */
  SOUND_MOTORi  = 47, /*!< insect (legs) */
  SOUND_SHIELD  = 48,
  SOUND_FIREi  = 49, /*!< shooting with orgaball (insect) */
  SOUND_GUNDEL  = 50,
  SOUND_PSHHH2  = 51, /*!< shield */
  SOUND_MESSAGE  = 52,
  SOUND_BOUMm  = 53, /*!< metal */
  SOUND_BOUMv  = 54, /*!< plant */
  SOUND_BOUMs  = 55, /*!< smooth */
  SOUND_EXPLOl  = 56, /*!< little */
  SOUND_EXPLOlp  = 57, /*!< little power */
  SOUND_EXPLOp  = 58, /*!< power */
  SOUND_STEPh  = 59, /*!< hard */
  SOUND_STEPm  = 60, /*!< metal */
  SOUND_POWERON  = 61,
  SOUND_POWEROFF  = 62,
  SOUND_AIE  = 63,
  SOUND_WAYPOINT  = 64,
  SOUND_RECOVER  = 65,
  SOUND_DEADi  = 66,
  SOUND_JOSTLE  = 67,
  SOUND_GFLAT  = 68,
  SOUND_DEADg  = 69, /*!< shooting death */
  SOUND_DEADw  = 70, /*!< drowning */
  SOUND_FLYf  = 71, /*!< reactor fail */
  SOUND_ALARMt  = 72, /*!< temperature alarm */
  SOUND_FINDING  = 73, /*!< finds a cache object */
  SOUND_THUMP  = 74,
  SOUND_TOUCH  = 75,
  SOUND_BLITZ  = 76,
  SOUND_MUSHROOM  = 77,
  SOUND_FIREp  = 78, /*!< shooting with phazer */
  SOUND_EXPLOg1  = 79, /*!< impact gun 1 */
  SOUND_EXPLOg2  = 80, /*!< impact gun 2 */
//  SOUND_MOTORd  = 81, /*!< engine friction */
  SOUND_MAX /** number of items in enum */
};

SoundType ParseSoundType(const std::string& str);
