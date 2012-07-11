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

// ioutils.h

#pragma once


#include <iostream>


namespace IOUtils {

//! Writes a binary number to output stream
/**
  \c T is a numeric type (int, unsigned int, etc.)
  \c N is number of bytes
  Write order is little-endian */
template<int N, typename T>
void WriteBinary(T value, std::ostream &ostr)
{
    for (int i = 0; i < N; ++i)
    {
        unsigned char byte = (value >> (i*8)) & 0xFF;
        ostr.write((char*)&byte, 1);
    }
}

//! Reads a binary number from input stream
/**
  \c T is a numeric type (int, unsigned int, etc.)
  \c N is number of bytes
  Read order is little-endian */
template<int N, typename T>
T ReadBinary(std::istream &istr)
{
    T value = 0;
    for (int i = 0; i < N; ++i)
    {
        unsigned char byte = 0;
        istr.read((char*)&byte, 1);
        value |= byte << (i*8);
    }
    return value;
}

//! Writes a binary 32-bit float to output stream
/**
  Write order is little-endian
  NOTE: code is probably not portable as there are platforms with other float representations. */
void WriteBinaryFloat(float value, std::ostream &ostr)
{
    unsigned int iValue = *( (unsigned int*)( (void*)(&value) ) );
    IOUtils::WriteBinary<4, unsigned int>(iValue, ostr);
}

//! Reads a binary 32-bit float from input stream
/**
  Read order is little-endian
  NOTE: code is probably not portable as there are platforms with other float representations. */
float ReadBinaryFloat(std::istream &istr)
{
    unsigned int iValue = IOUtils::ReadBinary<4, unsigned int>(istr);
    float result = *( (float*)( (void*)(&iValue) ) );
    return result;
}

}; // namespace IOUtils
