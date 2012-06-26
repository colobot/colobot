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

// profile.cpp

#define STRICT
#define D3D_OVERLOADS

#include <stdio.h>
#include <d3d.h>
#include <stdlib.h>

#include "language.h"
#include "struct.h"
#include "profile.h"



static char g_filename[100];



BOOL InitCurrentDirectory()
{
#if _SCHOOL
    _fullpath(g_filename, "ceebot.ini", 100);
#else
    _fullpath(g_filename, "colobot.ini", 100);
#endif
    return TRUE;
}


BOOL SetLocalProfileString(char* section, char* key, char* string)
{
    WritePrivateProfileString(section, key, string, g_filename);
    return TRUE;
}

BOOL GetLocalProfileString(char* section, char* key, char* buffer, int max)
{
    int     nb;

    nb = GetPrivateProfileString(section, key, "", buffer, max, g_filename);
    if ( nb == 0 )
    {
        buffer[0] = 0;
        return FALSE;
    }
    return TRUE;
}


BOOL SetLocalProfileInt(char* section, char* key, int value)
{
    char    s[20];

    sprintf(s, "%d", value);
    WritePrivateProfileString(section, key, s, g_filename);
    return TRUE;
}

BOOL GetLocalProfileInt(char* section, char* key, int &value)
{
    char    s[20];
    int     nb;

    nb = GetPrivateProfileString(section, key, "", s, 20, g_filename);
    if ( nb == 0 )
    {
        value = 0;
        return FALSE;
    }
    sscanf(s, "%d", &value);
    return TRUE;
}


BOOL SetLocalProfileFloat(char* section, char* key, float value)
{
    char    s[20];

    sprintf(s, "%.2f", value);
    WritePrivateProfileString(section, key, s, g_filename);
    return TRUE;
}

BOOL GetLocalProfileFloat(char* section, char* key, float &value)
{
    char    s[20];
    int     nb;

    nb = GetPrivateProfileString(section, key, "", s, 20, g_filename);
    if ( nb == 0 )
    {
        value = 0.0f;
        return FALSE;
    }
    sscanf(s, "%f", &value);
    return TRUE;
}


