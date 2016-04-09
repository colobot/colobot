/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2016, Daniel Roux, EPSITEC SA & TerranovaTeam
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


#include "common/misc.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sstream>
#include <iomanip>
#include <chrono>


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

std::string TimeFormat(float time)
{
    int minutes = floor(time/60);
    double time2 = fmod(time, 60);
    double seconds;
    double fraction = modf(time2, &seconds)*100;
    std::ostringstream sstream;
    sstream << std::setfill('0') << std::setw(2) << minutes << ":" << std::setfill('0') << std::setw(2) << floor(seconds) << "." << std::setfill('0') << std::setw(2) << floor(fraction);
    return sstream.str();
}


// Adds an extension to file, if doesn't already one.

void AddExt(char* filename, const char* ext)
{
    if ( strchr(filename, '.') != nullptr )  return;  // already an extension?
    strcat(filename, ext);
}
