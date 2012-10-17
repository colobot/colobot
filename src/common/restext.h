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
#include "common/restext_ids.h"


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


// TODO: move to CRobotMain

extern void     SetGlobalGamerName(char *name);
extern bool     SearchKey(const char *cmd, InputSlot& slot);
extern bool     GetResource(ResType type, int num, char* text);

extern const char * const strings_text[];
extern const char * const strings_event[];
extern const char * const strings_object[];
extern const char * const strings_err[];
extern const char * const strings_cbot[];

extern const int strings_text_len;
extern const int strings_event_len;
extern const int strings_object_len;
extern const int strings_err_len;
extern const int strings_cbot_len;
