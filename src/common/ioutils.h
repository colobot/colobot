// * This file is part of the COLOBOT source code
// * Copyright (C) 2012, Polish Portal of Colobot (PPC)
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

/**
 * \file common/ioutils.h
 * \brief Functions for binary I/O
 */

#pragma once


#include <iostream>

#include <cstring>

namespace IOUtils {

//! Writes a binary number to output stream
/**
 * \c T is a numeric type (int, unsigned int, etc.)
 * \c N is number of bytes
 * Write order is little-endian
 */
template<int N, typename T>
void WriteBinary(T value, std::ostream &ostr)
{
    for (int i = 0; i < N; ++i)
    {
        unsigned char byte = (value >> (i*8)) & 0xFF;
        ostr.write(reinterpret_cast<char*>(&byte), 1);
    }
}

//! Reads a binary number from input stream
/**
 * \c T is a numeric type (int, unsigned int, etc.)
 * \c N is number of bytes
 * Read order is little-endian
 */
template<int N, typename T>
T ReadBinary(std::istream &istr)
{
    T value = 0;
    for (int i = 0; i < N; ++i)
    {
        unsigned char byte = 0;
        istr.read(reinterpret_cast<char*>(&byte), 1);
        value |= byte << (i*8);
    }
    return value;
}

//! Writes a binary 1-byte boolean
/**
 * false is 0; true is 1.
 */
void WriteBinaryBool(float value, std::ostream &ostr)
{
    unsigned char v = value ? 1 : 0;
    IOUtils::WriteBinary<1, unsigned char>(v, ostr);
}

//! Reads a binary 1-byte boolean
/**
 * 0 is false; other values are true.
 */
bool ReadBinaryBool(std::istream &istr)
{
    int v = IOUtils::ReadBinary<1, unsigned char>(istr);
    return v != 0;
}

//! Writes a binary 32-bit float to output stream
/**
 * Write order is little-endian
 * NOTE: code is probably not portable as there are platforms with other float representations.
 */
void WriteBinaryFloat(float value, std::ostream &ostr)
{
    union { float fValue; unsigned int iValue; } u;
    memset(&u, 0, sizeof(u));
    u.fValue = value;
    IOUtils::WriteBinary<4, unsigned int>(u.iValue, ostr);
}

//! Reads a binary 32-bit float from input stream
/**
 * Read order is little-endian
 * NOTE: code is probably not portable as there are platforms with other float representations.
 */
float ReadBinaryFloat(std::istream &istr)
{
    union { float fValue; unsigned int iValue; } u;
    memset(&u, 0, sizeof(u));
    u.iValue = IOUtils::ReadBinary<4, unsigned int>(istr);
    return u.fValue;
}

//! Writes a variable binary string to output stream
/**
 * The string is written by first writing string length
 * in \c N byte binary number and then the string bytes.
 */
template<int N>
void WriteBinaryString(const std::string &value, std::ostream &ostr)
{
    int length = value.size();
    WriteBinary<N, int>(length, ostr);

    for (int i = 0; i < length; ++i)
        ostr.put(value[i]);
}

//! Reads a variable binary string from output stream
/**
 * The string is read by first reading string length
 * in \c N byte binary number and then the string bytes.
 */
template<int N>
std::string ReadBinaryString(std::istream &istr)
{
    int length = ReadBinary<N, int>(istr);

    std::string str;
    char c = 0;
    for (int i = 0; i < length; ++i)
    {
        istr.read(&c, 1);
        str += c;
    }

    return str;
}

}; // namespace IOUtils

