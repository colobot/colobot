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


// TODO: to be removed
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

// TODO: rewrite/refactor or remove

extern char RetNoAccent(char letter);
extern char RetToUpper(char letter);
extern char RetToLower(char letter);

extern void TimeToAscii(time_t time, char *buffer);

extern bool CopyFileToTemp(char* filename);
extern bool CopyFileListToTemp(char* filename, int* list, int total);
extern void AddExt(char* filename, char* ext);
extern void UserDir(bool bUser, char* dir);
extern void UserDir(char* buffer, char* dir, char* def);
