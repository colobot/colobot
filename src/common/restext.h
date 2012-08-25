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

// restext.h

#pragma once

#include "common/restext_ids.h"



class CD3DEngine;

// Possible types of the text resources.

enum ResType
{
    RES_TEXT        = 0,    // RT_*
    RES_EVENT       = 1,    // EVENT_*  (EventMsg)
    RES_OBJECT      = 2,    // OBJECT_* (ObjectType)
    RES_ERR         = 3,    // ERR_*    (Error)
    RES_KEY         = 4,    // VK_*     (keys)
    RES_CBOT        = 5,    // TX_*     (cbot.dll)
};


static CD3DEngine*  g_engine = 0;
static char         g_gamerName[100];

extern void     SetEngine(CD3DEngine *engine);
extern void     SetGlobalGamerName(char *name);
extern bool     SearchKey(char *cmd, KeyRank &key);
extern void     PutKeyName(char* dst, char* src);
extern bool     GetResource(ResType type, int num, char* text);
extern bool     GetResourceBase(ResType type, int num, char* text);


