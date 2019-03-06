/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include <cstdio>
#include <string>

namespace CBot
{

class CBotVar;
class CBotTypResult;

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

/*!
 * \brief SaveVars
 * \param pf
 * \param pVar
 * \return
 */
bool SaveVars(FILE* pf, CBotVar* pVar);

/*!
 * \brief WriteWord
 * \param pf
 * \param w
 * \return
 */
bool WriteWord(FILE* pf, unsigned short w);

/*!
 * \brief ReadWord
 * \param pf
 * \param w
 * \return
 */
bool ReadWord(FILE* pf, unsigned short& w);

/*!
 * \brief ReadLong
 * \param pf
 * \param w
 * \return
 */
bool ReadLong(FILE* pf, long& w);

/*!
 * \brief WriteFloat
 * \param pf
 * \param w
 * \return
 */
bool WriteFloat(FILE* pf, float w);

/*!
 * \brief WriteLong
 * \param pf
 * \param w
 * \return
 */
bool WriteLong(FILE* pf, long w);

/*!
 * \brief ReadFloat
 * \param pf
 * \param w
 * \return
 */
bool ReadFloat(FILE* pf, float& w);

/*!
 * \brief ReadString
 * \param pf
 * \param s
 * \return
 */
bool ReadString(FILE* pf, std::string& s);

/*!
 * \brief WriteType
 * \param pf
 * \param type
 * \return
 */
bool WriteType(FILE* pf, const CBotTypResult &type);

/*!
 * \brief ReadType
 * \param pf
 * \param type
 * \return
 */
bool ReadType(FILE* pf, CBotTypResult &type);

} // namespace CBot
