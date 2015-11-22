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

#pragma once

// Modules inlcude

// Local include

// Global include
 #include <cstdio>

///////////////////////////////////////////////////////////////////////////////
// routines for file management  (* FILE)

/*!
 * \brief fOpen
 * \param name
 * \param mode
 * \return
 */
FILE* fOpen(const char* name, const char* mode);

/*!
 * \brief fClose
 * \param filehandle
 * \return
 */
int fClose(FILE* filehandle);

/*!
 * \brief fWrite
 * \param buffer
 * \param elemsize
 * \param length
 * \param filehandle
 * \return
 */
std::size_t fWrite(const void *buffer,
                   std::size_t elemsize,
                   std::size_t length,
                   FILE* filehandle);

/*!
 * \brief fRead
 * \param buffer
 * \param elemsize
 * \param length
 * \param filehandle
 * \return
 */
std::size_t fRead(void *buffer,
                  std::size_t elemsize,
                  std::size_t length,
                  FILE* filehandle);
