/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include <cstdint>
#include <iostream>
#include <string>

namespace CBot
{

class CBotVar;
class CBotTypResult;

/*!
 * \brief Save a linked list if variables
 * \param ostr Output stream
 * \param pVar First variable in the list
 * \return true on success
 */
bool SaveVars(std::ostream &ostr, CBotVar* pVar);

/*!
 * \brief WriteWord
 * \param ostr Output stream
 * \param w
 * \return true on success
 */
bool WriteWord(std::ostream &ostr, unsigned short w);

/*!
 * \brief ReadWord
 * \param istr Input stream
 * \param[out] w
 * \return true on success
 */
bool ReadWord(std::istream &istr, unsigned short &w);

/*!
 * \brief WriteByte
 * \param ostr Output stream
 * \param c
 * \return true on success
 */
bool WriteByte(std::ostream &ostr, char c);

/*!
 * \brief ReadByte
 * \param istr Input stream
 * \param[out] c
 * \return true on success
 */
bool ReadByte(std::istream &istr, char& c);

/*!
 * \brief WriteShort
 * \param ostr Output stream
 * \param s
 * \return true on success
 */
bool WriteShort(std::ostream &ostr, short s);

/*!
 * \brief ReadShort
 * \param istr Input stream
 * \param[out] s
 * \return true on success
 */
bool ReadShort(std::istream &istr, short &s);

/*!
 * \brief WriteUInt32
 * \param ostr Output stream
 * \param i
 * \return true on success
 */
bool WriteUInt32(std::ostream &ostr, uint32_t i);

/*!
 * \brief ReadUInt32
 * \param istr Input stream
 * \param[out] i
 * \return true on success
 */
bool ReadUInt32(std::istream &istr, uint32_t &i);

/*!
 * \brief WriteInt
 * \param ostr Output stream
 * \param i
 * \return true on success
 */
bool WriteInt(std::ostream &ostr, int i);

/*!
 * \brief ReadInt
 * \param istr Input stream
 * \param[out] i
 * \return true on success
 */
bool ReadInt(std::istream &istr, int &i);

/*!
 * \brief WriteLong
 * \param ostr Output stream
 * \param l
 * \param padTo minimum number of bytes to write
 * \return true on success
 */
bool WriteLong(std::ostream &ostr, long l, unsigned padTo = 0);

/*!
 * \brief ReadLong
 * \param istr Input stream
 * \param[out] l
 * \return true on success
 */
bool ReadLong(std::istream &istr, long &l);

/*!
 * \brief WriteFloat
 * \param ostr Output stream
 * \param f
 * \return true on success
 */
bool WriteFloat(std::ostream &ostr, float f);

/*!
 * \brief ReadFloat
 * \param istr Input stream
 * \param[out] f
 * \return true on success
 */
bool ReadFloat(std::istream &istr, float &f);

/*!
 * \brief WriteDouble
 * \param ostr Output stream
 * \param d
 * \return true on success
 */
bool WriteDouble(std::ostream &ostr, double d);

/*!
 * \brief ReadDouble
 * \param istr Input stream
 * \param[out] d
 * \return true on success
 */
bool ReadDouble(std::istream &istr, double &d);

/*!
 * \brief WriteString
 * \param ostr Output stream
 * \param s
 * \return true on success
 */
bool WriteString(std::ostream &ostr, const std::string &s);

/*!
 * \brief ReadString
 * \param istr Input stream
 * \param[out] s
 * \return true on success
 */
bool ReadString(std::istream &istr, std::string &s);

/*!
 * \brief WriteType
 * \param ostr Output stream
 * \param type
 * \return true on success
 */
bool WriteType(std::ostream &ostr, const CBotTypResult &type);

/*!
 * \brief ReadType
 * \param istr Input stream
 * \param[out] type
 * \return true on success
 */
bool ReadType(std::istream &istr, CBotTypResult &type);

/*!
 * \brief WriteStream
 * \param ostr Output stream
 * \param istr Input stream
 * \return true on success
 */
bool WriteStream(std::ostream &ostr, std::istream& istr);

/*!
 * \brief ReadStream
 * \param istr Input stream
 * \param ostr Output stream
 * \return true on success
 */
bool ReadStream(std::istream& istr, std::ostream &ostr);

} // namespace CBot
