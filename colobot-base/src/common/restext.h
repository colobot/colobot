/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
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
 * \file common/restext.h
 * \brief Translation and string resource utilities
 */

#pragma once


#include <string>


/**
 * \enum ResType
 * \brief Types of text resources
 */
enum ResType
{
    RES_TEXT        = 0,    //! < RT_*
    RES_EVENT       = 1,    //! < EVENT_*  (EventMsg)
    RES_OBJECT      = 2,    //! < OBJECT_* (ObjectType)
    RES_ERR         = 3,    //! < ERR_*    (Error)
    RES_KEY         = 4,    //! < KEY()    (keys)
    RES_CBOT        = 5,    //! < TX_*     (CBot)
};

/**
 * \enum ResTextType
 * \brief Text resources available for translation
 */
enum ResTextType
{
    RT_WINDOW_MAXIMIZED     = 3,
    RT_WINDOW_MINIMIZED     = 4,
    RT_WINDOW_STANDARD      = 5,
    RT_WINDOW_CLOSE         = 6,

    RT_STUDIO_TITLE         = 10,
    RT_SCRIPT_NEW           = 20,
    RT_NAME_DEFAULT         = 21,
    RT_IO_NEW               = 22,
    RT_KEY_OR               = 23,

    RT_TITLE_APPNAME        = 40,
    RT_TITLE_TRAINER        = 42,
    RT_TITLE_DEFI           = 43,
    RT_TITLE_MISSION        = 44,
    RT_TITLE_FREE           = 45,
    RT_TITLE_CODE_BATTLES   = 46,
    RT_TITLE_SETUP          = 47,
    RT_TITLE_NAME           = 48,
    RT_TITLE_PERSO          = 49,
    RT_TITLE_WRITE          = 50,
    RT_TITLE_READ           = 51,
    RT_TITLE_USER           = 52,
    RT_TITLE_PLUS           = 53,
    RT_TITLE_MODS           = 54,

    RT_PLAY_CHAP_CHAPTERS   = 60,
    RT_PLAY_CHAP_PLANETS    = 61,
    RT_PLAY_CHAP_USERLVL    = 62,
    RT_PLAY_LIST_LEVELS     = 63,
    RT_PLAY_LIST_EXERCISES  = 64,
    RT_PLAY_LIST_CHALLENGES = 65,
    RT_PLAY_LIST_MISSIONS   = 66,
    RT_PLAY_LIST_FREEGAME   = 67,
    RT_PLAY_RESUME          = 68,

    RT_SETUP_MODE           = 81,
    RT_SETUP_KEY1           = 82,
    RT_SETUP_KEY2           = 83,

    RT_PERSO_FACE           = 90,
    RT_PERSO_GLASSES        = 91,
    RT_PERSO_HAIR           = 92,
    RT_PERSO_COMBI          = 93,
    RT_PERSO_BAND           = 94,

    RT_DIALOG_ABORT         = 103,
    RT_DIALOG_CONTINUE      = 104,
    RT_DIALOG_DELOBJ        = 105,
    RT_DIALOG_DELGAME       = 106,
    RT_DIALOG_YES           = 107,
    RT_DIALOG_NO            = 108,
    RT_DIALOG_LOADING       = 109,
    RT_DIALOG_OK            = 110,
    RT_DIALOG_NOUSRLVL_TITLE = 111,
    RT_DIALOG_NOUSRLVL_TEXT = 112,
    RT_DIALOG_OPEN_PATH_FAILED_TITLE = 113,
    RT_DIALOG_OPEN_PATH_FAILED_TEXT = 114,
    RT_DIALOG_OPEN_WEBSITE_FAILED_TITLE = 115,
    RT_DIALOG_OPEN_WEBSITE_FAILED_TEXT = 116,
    RT_DIALOG_CHANGES_QUESTION = 117,
    RT_DIALOG_OPEN_UNSUPPORTED = 118,

    RT_STUDIO_LISTTT        = 120,
    RT_STUDIO_COMPOK        = 121,
    RT_STUDIO_PROGSTOP      = 122,
    RT_STUDIO_CLONED        = 123,

    RT_PROGRAM_READONLY     = 130,
    RT_PROGRAM_EXAMPLE      = 131,

    RT_IO_OPEN              = 150,
    RT_IO_SAVE              = 151,
    RT_IO_LIST              = 152,
    RT_IO_NAME              = 153,
    RT_IO_DIR               = 154,
    RT_IO_PRIVATE           = 155,
    RT_IO_PUBLIC            = 156,
    RT_IO_REPLACE           = 157,
    RT_IO_SELECT_DIR        = 158,

    RT_GENERIC_DEV1         = 170,
    RT_GENERIC_DEV2         = 171,
    RT_GENERIC_EDIT1        = 172,
    RT_GENERIC_EDIT2        = 173,

    RT_INTERFACE_REC        = 180,

    RT_LOADING_INIT           = 210,
    RT_LOADING_PROCESSING     = 211,
    RT_LOADING_LEVEL_SETTINGS = 212,
    RT_LOADING_MUSIC          = 213,
    RT_LOADING_TERRAIN        = 214,
    RT_LOADING_OBJECTS        = 215,
    RT_LOADING_OBJECTS_SAVED  = 216,
    RT_LOADING_CBOT_SAVE      = 218,
    RT_LOADING_FINISHED       = 219,
    RT_LOADING_TERRAIN_RELIEF = 220,
    RT_LOADING_TERRAIN_RES    = 221,
    RT_LOADING_TERRAIN_TEX    = 222,
    RT_LOADING_TERRAIN_GEN    = 223,

    RT_SCOREBOARD_RESULTS     = 230,
    RT_SCOREBOARD_RESULTS_TEXT= 231,
    RT_SCOREBOARD_RESULTS_TIME= 232,
    RT_SCOREBOARD_RESULTS_LINE= 233,

    RT_MOD_LIST               = 234,
    RT_MOD_DETAILS            = 235,
    RT_MOD_SUMMARY            = 236,
    RT_MOD_ENABLE             = 237,
    RT_MOD_DISABLE            = 238,
    RT_MOD_UNKNOWN_AUTHOR     = 239,
    RT_MOD_AUTHOR_FIELD_NAME  = 240,
    RT_MOD_VERSION_FIELD_NAME = 241,
    RT_MOD_WEBSITE_FIELD_NAME = 242,
    RT_MOD_CHANGES_FIELD_NAME = 243,
    RT_MOD_NO_SUMMARY         = 244,
    RT_MOD_NO_CHANGES         = 245,

    RT_MAX //! < number of values
};


// TODO: move to CRobotMain

void     InitializeRestext();

void     SetGlobalGamerName(std::string name);
bool     GetResource(ResType type, unsigned int num, std::string& text);
