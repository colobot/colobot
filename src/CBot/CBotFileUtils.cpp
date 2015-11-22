/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2015, Daniel Roux, EPSITEC SA & TerranovaTeam
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

// Modules inlcude
#include "CBotFileUtils.h"

// Local include

// Global include



// file management

// necessary because it is not possible to do the fopen in the main program
// fwrite and fread in a dll or using the FILE * returned.

////////////////////////////////////////////////////////////////////////////////
FILE* fOpen(const char* name, const char* mode)
{
    return fopen(name, mode);
}

////////////////////////////////////////////////////////////////////////////////
int fClose(FILE* filehandle)
{
    return fclose(filehandle);
}

////////////////////////////////////////////////////////////////////////////////
std::size_t fWrite(const void *buffer,
                   std::size_t elemsize,
                   std::size_t length,
                   FILE* filehandle)
{
    return fwrite(buffer, elemsize, length, filehandle);
}

////////////////////////////////////////////////////////////////////////////////
std::size_t fRead(void *buffer,
                  std::size_t elemsize,
                  std::size_t length,
                  FILE* filehandle)
{
    return fread(buffer, elemsize, length, filehandle);
}
