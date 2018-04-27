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
 * \file common/error.h
 * \brief Definition of the Error enum
 */

/**
 * \enum Error
 * \brief Type of error or info message
 */
enum Error
{
    ERR_OK                = 0,        //!<  ok
    ERR_UNKNOWN           = 1,        //!<  any error
    ERR_CONTINUE          = 2,        //!<  continues
    ERR_STOP              = 3,        //!<  stops
    ERR_CMD               = 4,        //!<  unknown command
    ERR_OBJ_BUSY          = 5,        //!<  object is busy
    ERR_MANIP_FLY         = 101,      //!<  impossible in flight
    ERR_MANIP_BUSY        = 102,      //!<  taking: hands already occupied
    ERR_MANIP_NIL         = 103,      //!<  taking: nothing has to take
    ERR_MANIP_MOTOR       = 105,      //!<  busy: impossible to move
    ERR_MANIP_OCC         = 106,      //!<  busy: location already occupied
    ERR_MANIP_RADIO       = 108,      //!<  impossible because radioactive
    ERR_MANIP_WATER       = 109,      //!<  not possible under water
    ERR_MANIP_EMPTY       = 110,      //!<  nothing to deposit
    ERR_BUILD_FLY         = 120,      //!<  not possible in flight
    ERR_BUILD_WATER       = 121,      //!<  not possible under water
    ERR_BUILD_METALAWAY   = 123,      //!<  lack of metal (too far)
    ERR_BUILD_METALNEAR   = 124,      //!<  lack of metal (too close)
    ERR_BUILD_METALINEX   = 125,      //!<  lack of metal
    ERR_BUILD_FLAT        = 126,      //!<  not enough flat ground
    ERR_BUILD_FLATLIT     = 127,      //!<  not enough flat ground space
    ERR_BUILD_BUSY        = 128,      //!<  location occupied
    ERR_BUILD_BASE        = 129,      //!<  too close to the rocket
    ERR_BUILD_NARROW      = 130,      //!<  buildings too close
    ERR_BUILD_MOTOR       = 131,      //!<  built: not possible in movement
    ERR_BUILD_DISABLED    = 132,      //!<  built: can not produce this object in this mission
    ERR_BUILD_RESEARCH    = 133,      //!<  built: can not produce not researched object
    ERR_SEARCH_FLY        = 140,      //!<  not possible in flight
    ERR_SEARCH_MOTOR      = 142,      //!<  impossible in movement
    ERR_TERRA_ENERGY      = 151,      //!<  not enough energy
    ERR_FIRE_ENERGY       = 161,      //!<  not enough energy
    ERR_RECOVER_ENERGY    = 171,      //!<  not enough energy
    ERR_RECOVER_NULL      = 172,      //!<  lack of ruin
    ERR_CONVERT_EMPTY     = 180,      //!<  no stone was transformed
    ERR_SHIELD_ENERGY     = 191,      //!<  not enough energy
    ERR_MOVE_IMPOSSIBLE   = 200,      //!<  move impossible
    ERR_GOTO_IMPOSSIBLE   = 210,      //!<  goto impossible
    ERR_GOTO_ITER         = 211,      //!<  goto too complicated
    ERR_GOTO_BUSY         = 212,      //!<  goto destination occupied
    ERR_DERRICK_NULL      = 300,      //!<  no ore underground
    ERR_STATION_NULL      = 301,      //!<  no energy underground
    ERR_TOWER_POWER       = 310,      //!<  no battery
    ERR_TOWER_ENERGY      = 311,      //!<  more energy
    ERR_RESEARCH_POWER    = 320,      //!<  no battery
    ERR_RESEARCH_ENERGY   = 321,      //!<  more energy
    ERR_RESEARCH_TYPE     = 322,      //!<  the wrong type of battery
    ERR_RESEARCH_ALREADY  = 323,      //!<  research already done
    ERR_ENERGY_NULL       = 330,      //!<  no energy underground
    ERR_ENERGY_LOW        = 331,      //!<  not enough energy
    ERR_ENERGY_EMPTY      = 332,      //!<  lack of metal
    ERR_ENERGY_BAD        = 333,      //!<  transforms only the metal
    ERR_BASE_DLOCK        = 340,      //!<  doors locked
    ERR_BASE_DHUMAN       = 341,      //!<  you must be on spaceship
    ERR_LABO_NULL         = 350,      //!<  nothing to analyze
    ERR_LABO_BAD          = 351,      //!<  analyzes only organic ball
    ERR_LABO_ALREADY      = 352,      //!<  analysis already made
    ERR_NUCLEAR_EMPTY     = 362,      //!<  lack of uranium
    ERR_NUCLEAR_BAD       = 363,      //!<  transforms only uranium
    ERR_FACTORY_NULL      = 370,      //!<  no metal
    ERR_FACTORY_NEAR      = 371,      //!<  vehicle too close
    ERR_INFO_NULL         = 390,      //!<  no information terminal
    ERR_VEH_VIRUS         = 400,      //!<  vehicle infected by a virus
    ERR_BAT_VIRUS         = 401,      //!<  building infected by a virus
    ERR_DESTROY_NOTFOUND  = 410,      //!<  not found anything to destroy
    ERR_WRONG_OBJ         = 420,      //!<  inappropriate vehicle
    ERR_VEH_POWER         = 500,      //!<  no battery
    ERR_VEH_ENERGY        = 501,      //!<  more energy
    ERR_FLAG_FLY          = 510,      //!<  impossible in flight
    ERR_FLAG_WATER        = 511,      //!<  impossible during swimming
    ERR_FLAG_MOTOR        = 512,      //!<  impossible in movement
    ERR_FLAG_BUSY         = 513,      //!<  taking: already creating flag
    ERR_FLAG_CREATE       = 514,      //!<  too many flags
    ERR_FLAG_PROXY        = 515,      //!<  too close
    ERR_FLAG_DELETE       = 516,      //!<  nothing to remove
    ERR_MISSION_NOTERM    = 600,      //!<  Mission not completed
    ERR_DELETEMOBILE      = 700,      //!<  vehicle destroyed
    ERR_DELETEBUILDING    = 701,      //!<  building destroyed
    ERR_ENEMY_OBJECT      = 703,      //!<  can't control enemy object
    ERR_OBLIGATORYTOKEN   = 800,      //!<  compulsory instruction missing
    ERR_PROHIBITEDTOKEN   = 801,      //!<  instruction prohibited
    ERR_AIM_IMPOSSIBLE    = 900,      //!<  cannot aim at specified angle(s)
    ERR_WRONG_BOT         = 910,      //!<  inappropriate bot
    ERR_NO_QUICK_SLOT     = 920,      //!<  quicksave slot not found

    INFO_FIRST            = 10000,    //! <  first information
    INFO_BUILD            = 10001,    //! <  construction builded
    INFO_CONVERT          = 10002,    //! <  metal available
    INFO_RESEARCH         = 10003,    //! <  search ended
    INFO_FACTORY          = 10004,    //! <  vehicle manufactured
    INFO_LABO             = 10005,    //! <  analysis ended
    INFO_ENERGY           = 10006,    //! <  battery available
    INFO_NUCLEAR          = 10007,    //! <  nuclear battery available
    INFO_FINDING          = 10008,    //! <  useful object found
    INFO_MARKPOWER        = 10020,    //! <  location for station found
    INFO_MARKURANIUM      = 10021,    //! <  location for derrick found
    INFO_MARKSTONE        = 10022,    //! <  location for derrick found
    INFO_MARKKEYa         = 10023,    //! <  location for derrick found
    INFO_MARKKEYb         = 10024,    //! <  location for derrick found
    INFO_MARKKEYc         = 10025,    //! <  location for derrick found
    INFO_MARKKEYd         = 10026,    //! <  location for derrick found
    INFO_RESEARCHTANK     = 10030,    //! <  research ended
    INFO_RESEARCHFLY      = 10031,    //! <  research ended
    INFO_RESEARCHTHUMP    = 10032,    //! <  research ended
    INFO_RESEARCHCANON    = 10033,    //! <  research ended
    INFO_RESEARCHTOWER    = 10034,    //! <  research ended
    INFO_RESEARCHPHAZER   = 10035,    //! <  research ended
    INFO_RESEARCHSHIELD   = 10036,    //! <  research ended
    INFO_RESEARCHATOMIC   = 10037,    //! <  research ended
    INFO_WIN              = 10040,    //! <  win
    INFO_LOST             = 10041,    //! <  lost
    INFO_LOSTq            = 10042,    //! <  lost immediately
    INFO_WRITEOK          = 10043,    //! <  record done
    INFO_DELETEMOTHER     = 10100,    //! <  insect killed
    INFO_DELETEANT        = 10101,    //! <  insect killed
    INFO_DELETEBEE        = 10102,    //! <  insect killed
    INFO_DELETEWORM       = 10103,    //! <  insect killed
    INFO_DELETESPIDER     = 10104,    //! <  insect killed
    INFO_BEGINSATCOM      = 10105,    //! <  use your SatCom
    INFO_TEAM_FINISH      = 10110,
    INFO_TEAM_DEAD        = 10111,
    INFO_TEAM_SCORE       = 10112,

    ERR_MAX //! < number of values
};
