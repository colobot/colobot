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


#include "common/misc.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>


// Returns a non-accented letter.

char GetNoAccent(char letter)
{
    /*
    if ( letter < 0 )
    {
        if ( letter == '�' ||
             letter == '�' ||
             letter == '�' ||
             letter == '�' ||
             letter == '�' )  return 'a';

        if ( letter == '�' ||
             letter == '�' ||
             letter == '�' ||
             letter == '�' )  return 'e';

        if ( letter == '�' ||
             letter == '�' ||
             letter == '�' ||
             letter == '�' )  return 'i';

        if ( letter == '�' ||
             letter == '�' ||
             letter == '�' ||
             letter == '�' ||
             letter == '�' )  return 'o';

        if ( letter == '�' ||
             letter == '�' ||
             letter == '�' ||
             letter == '�' )  return 'u';

        if ( letter == '�' )  return 'c';

        if ( letter == '�' )  return 'n';

        if ( letter == '�' ||
             letter == '�' ||
             letter == '�' ||
             letter == '�' ||
             letter == '�' )  return 'A';

        if ( letter == '�' ||
             letter == '�' ||
             letter == '�' ||
             letter == '�' )  return 'E';

        if ( letter == '�' ||
             letter == '�' ||
             letter == '�' ||
             letter == '�' )  return 'I';

        if ( letter == '�' ||
             letter == '�' ||
             letter == '�' ||
             letter == '�' ||
             letter == '�' )  return 'O';

        if ( letter == '�' ||
             letter == '�' ||
             letter == '�' ||
             letter == '�' )  return 'U';

        if ( letter == '�' )  return 'C';

        if ( letter == '�' )  return 'N';
    }*/

    return letter;
}

// Returns an uppercase letter.

char GetToUpper(char letter)
{
    /*if ( letter < 0 )
    {
        if ( letter == '�' )  return '�';
        if ( letter == '�' )  return '�';
        if ( letter == '�' )  return '�';
        if ( letter == '�' )  return '�';
        if ( letter == '�' )  return '�';

        if ( letter == '�' )  return '�';
        if ( letter == '�' )  return '�';
        if ( letter == '�' )  return '�';
        if ( letter == '�' )  return '�';

        if ( letter == '�' )  return '�';
        if ( letter == '�' )  return '�';
        if ( letter == '�' )  return '�';
        if ( letter == '�' )  return '�';

        if ( letter == '�' )  return '�';
        if ( letter == '�' )  return '�';
        if ( letter == '�' )  return '�';
        if ( letter == '�' )  return '�';
        if ( letter == '�' )  return '�';

        if ( letter == '�' )  return '�';
        if ( letter == '�' )  return '�';
        if ( letter == '�' )  return '�';
        if ( letter == '�' )  return '�';

        if ( letter == '�' )  return '�';

        if ( letter == '�' )  return '�';
    }*/

    return toupper(letter);
}

// Returns a lowercase letter.

char GetToLower(char letter)
{
    /*if ( letter < 0 )
    {
        if ( letter == '�' )  return '�';
        if ( letter == '�' )  return '�';
        if ( letter == '�' )  return '�';
        if ( letter == '�' )  return '�';
        if ( letter == '�' )  return '�';

        if ( letter == '�' )  return '�';
        if ( letter == '�' )  return '�';
        if ( letter == '�' )  return '�';
        if ( letter == '�' )  return '�';

        if ( letter == '�' )  return '�';
        if ( letter == '�' )  return '�';
        if ( letter == '�' )  return '�';
        if ( letter == '�' )  return '�';

        if ( letter == '�' )  return '�';
        if ( letter == '�' )  return '�';
        if ( letter == '�' )  return '�';
        if ( letter == '�' )  return '�';
        if ( letter == '�' )  return '�';

        if ( letter == '�' )  return '�';
        if ( letter == '�' )  return '�';
        if ( letter == '�' )  return '�';
        if ( letter == '�' )  return '�';

        if ( letter == '�' )  return '�';

        if ( letter == '�' )  return '�';
    }*/

    return tolower(letter);
}


// Converting time to string.

void TimeToAscii(time_t time, char *buffer)
{
    struct tm   when;
    int         year;

    when = *localtime(&time);
    year = when.tm_year+1900;
    if ( year < 2000 )  year -= 1900;
    else                year -= 2000;
/* TODO
#if _FRENCH
    sprintf(buffer, "%.2d.%.2d.%.2d %.2d:%.2d",
                    when.tm_mday, when.tm_mon+1, year,
                    when.tm_hour, when.tm_min);
#endif
#if _GERMAN | _WG
    sprintf(buffer, "%.2d.%.2d.%.2d %.2d:%.2d",
                    when.tm_mday, when.tm_mon+1, year,
                    when.tm_hour, when.tm_min);
#endif
#if _ENGLISH*/
    char        format[10];
    int         hour;

    hour = when.tm_hour;  // 0..23
    if ( hour < 12 )  // morning?
    {
        strcpy(format, "am");
    }
    else    // afternoon?
    {
        strcpy(format, "pm");
        hour -= 12;  // 0..11
    }
    if ( hour == 0 )  hour = 12;

    sprintf(buffer, "%.2d.%.2d.%.2d %.2d:%.2d %s",
                    when.tm_mon+1, when.tm_mday, year,
                    hour, when.tm_min, format);
/*#endif
#if _POLISH
    sprintf(buffer, "%.2d.%.2d.%.2d %.2d:%.2d",
                    when.tm_mday, when.tm_mon+1, year,
                    when.tm_hour, when.tm_min);
#endif*/
}

// Copy a list of numbered files into the temporary folder.

bool CopyFileListToTemp(char* filename, int* list, int total)
{
    /*char    name[100];
    char    ext[10];
    char    file[100];
    char    save[100];
    char*   p;
    int     i;

    strcpy(name, filename);
    p = strchr(name, '.');
    if ( p == 0 )
    {
        strcpy(ext, ".tga");
    }
    else
    {
        strcpy(ext, p);
        *p = 0;
    }

    for ( i=0 ; i<total ; i++ )
    {
        sprintf(file, "%s%.3d%s", name, list[i], ext);  // nameNNN.ext
        CopyFileToTemp(file);
    }

    strcpy(save, g_userDir);
    strcpy(g_userDir, "temp");
    UserDir(file, filename, "textures");
    strcpy(filename, file);
    strcpy(g_userDir, save);
*/
    return false;
}


// Adds an extension to file, if doesn't already one.

void AddExt(char* filename, const char* ext)
{
    if ( strchr(filename, '.') != 0 )  return;  // already an extension?
    strcat(filename, ext);
}

