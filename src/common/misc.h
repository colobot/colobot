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

// misc.h

#pragma once


#include <time.h>


#include "common/metafile.h"
#include "common/event.h"
#include "common/error_ids.h"


extern CMetaFile    g_metafile;



// Existing classes.

enum ClassType
{
    CLASS_EVENT         = 1,
    CLASS_INTERFACE     = 2,
    CLASS_MAIN          = 3,
    CLASS_ENGINE        = 4,
    CLASS_TERRAIN       = 5,
    CLASS_OBJECT        = 6,
    CLASS_PHYSICS       = 7,
    CLASS_BRAIN         = 8,
    CLASS_CAMERA        = 9,
    CLASS_LIGHT         = 10,
    CLASS_PARTICULE     = 11,
    CLASS_AUTO          = 12,
    CLASS_DISPLAYTEXT   = 13,
    CLASS_PYRO          = 14,
    CLASS_SCRIPT        = 15,
    CLASS_TEXT          = 16,
    CLASS_STUDIO        = 17,
    CLASS_WATER         = 18,
    CLASS_CLOUD         = 19,
    CLASS_MOTION        = 20,
    CLASS_SOUND         = 21,
    CLASS_PLANET        = 22,
    CLASS_TASKMANAGER   = 23,
    CLASS_DIALOG        = 24,
    CLASS_MAP           = 25,
    CLASS_SHORT         = 26,
    CLASS_BLITZ         = 27,
};

const int CLASS_MAX = 30;




// Keyboard state.

const int KS_PAGEUP         = (1<<4);
const int KS_PAGEDOWN       = (1<<5);
const int KS_SHIFT          = (1<<6);
const int KS_CONTROL        = (1<<7);
const int KS_MLEFT          = (1<<8);
const int KS_MRIGHT         = (1<<9);
const int KS_NUMUP          = (1<<10);
const int KS_NUMDOWN        = (1<<11);
const int KS_NUMLEFT        = (1<<12);
const int KS_NUMRIGHT       = (1<<13);
const int KS_NUMPLUS        = (1<<14);
const int KS_NUMMINUS       = (1<<15);


// Procedures.

extern EventType GetUniqueEventType();

extern char GetNoAccent(char letter);
extern char GetToUpper(char letter);
extern char GetToLower(char letter);

extern void TimeToAscii(time_t time, char *buffer);

extern bool CopyFileToTemp(char* filename);
extern bool CopyFileListToTemp(char* filename, int* list, int total);
extern void AddExt(char* filename, char* ext);
extern void UserDir(bool bUser, char* dir);
extern void UserDir(char* buffer, char* dir, const char* def);

extern char GetLanguageLetter();


