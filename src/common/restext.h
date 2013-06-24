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
 * \file common/restext.h
 * \brief Translation and string resource utilities
 */

#pragma once


#include "common/global.h"

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
    RT_VERSION_ID           = 1,
    RT_DISINFO_TITLE        = 2,
    RT_WINDOW_MAXIMIZED     = 3,
    RT_WINDOW_MINIMIZED     = 4,
    RT_WINDOW_STANDARD      = 5,
    RT_WINDOW_CLOSE         = 6,

    RT_STUDIO_TITLE         = 10,
    RT_SCRIPT_NEW           = 20,
    RT_NAME_DEFAULT         = 21,
    RT_IO_NEW               = 22,
    RT_KEY_OR               = 23,

    RT_TITLE_BASE           = 40,
    RT_TITLE_INIT           = 41,
    RT_TITLE_TRAINER        = 42,
    RT_TITLE_DEFI           = 43,
    RT_TITLE_MISSION        = 44,
    RT_TITLE_FREE           = 45,
    RT_TITLE_PROTO          = 46,
    RT_TITLE_SETUP          = 47,
    RT_TITLE_NAME           = 48,
    RT_TITLE_PERSO          = 49,
    RT_TITLE_WRITE          = 50,
    RT_TITLE_READ           = 51,
    RT_TITLE_USER           = 52,
    RT_TITLE_TEEN           = 53,

    RT_PLAY_CHAPt           = 60,
    RT_PLAY_CHAPd           = 61,
    RT_PLAY_CHAPm           = 62,
    RT_PLAY_CHAPf           = 63,
    RT_PLAY_CHAPp           = 64,
    RT_PLAY_LISTt           = 65,
    RT_PLAY_LISTd           = 66,
    RT_PLAY_LISTm           = 67,
    RT_PLAY_LISTf           = 68,
    RT_PLAY_LISTp           = 69,
    RT_PLAY_RESUME          = 70,
    RT_PLAY_CHAPu           = 71,
    RT_PLAY_LISTu           = 72,
    RT_PLAY_CHAPte          = 73,
    RT_PLAY_LISTk           = 74,

    RT_SETUP_DEVICE         = 80,
    RT_SETUP_MODE           = 81,
    RT_SETUP_KEY1           = 82,
    RT_SETUP_KEY2           = 83,

    RT_PERSO_FACE           = 90,
    RT_PERSO_GLASSES        = 91,
    RT_PERSO_HAIR           = 92,
    RT_PERSO_COMBI          = 93,
    RT_PERSO_BAND           = 94,

    RT_DIALOG_TITLE         = 100,
    RT_DIALOG_ABORT         = 101,
    RT_DIALOG_QUIT          = 102,
    RT_DIALOG_YES           = 103,
    RT_DIALOG_NO            = 104,
    RT_DIALOG_DELOBJ        = 105,
    RT_DIALOG_DELGAME       = 106,
    RT_DIALOG_YESDEL        = 107,
    RT_DIALOG_NODEL         = 108,
    RT_DIALOG_LOADING       = 109,
    RT_DIALOG_YESQUIT       = 110,
    RT_DIALOG_NOQUIT        = 111,

    RT_STUDIO_LISTTT        = 120,
    RT_STUDIO_COMPOK        = 121,
    RT_STUDIO_PROGSTOP      = 122,

    RT_SATCOM_LIST          = 140,
    RT_SATCOM_BOT           = 141,
    RT_SATCOM_BUILDING      = 142,
    RT_SATCOM_FRET          = 143,
    RT_SATCOM_ALIEN         = 144,
    RT_SATCOM_NULL          = 145,
    RT_SATCOM_ERROR1        = 146,
    RT_SATCOM_ERROR2        = 147,

    RT_IO_OPEN              = 150,
    RT_IO_SAVE              = 151,
    RT_IO_LIST              = 152,
    RT_IO_NAME              = 153,
    RT_IO_DIR               = 154,
    RT_IO_PRIVATE           = 155,
    RT_IO_PUBLIC            = 156,

    RT_GENERIC_DEV1         = 170,
    RT_GENERIC_DEV2         = 171,
    RT_GENERIC_EDIT1        = 172,
    RT_GENERIC_EDIT2        = 173,

    RT_INTERFACE_REC        = 180,

    RT_MESSAGE_WIN          = 200,
    RT_MESSAGE_LOST         = 201,


    RT_MAX //! < number of values
};


// TODO: move to CRobotMain

void     InitializeRestext();

void     SetGlobalGamerName(std::string name);
bool     SearchKey(const char *cmd, InputSlot& slot);
bool     GetResource(ResType type, int num, char* text);

