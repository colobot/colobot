/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2021, Daniel Roux, EPSITEC SA & TerranovaTeam
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

/**
 * \file core/stringutils.h
 * \brief Some useful string operations
 */

#pragma once

#include <cstddef>
#include <string>
#include <sstream>
#include <functional>

namespace StrUtils
{

//! Converts a value to string
/** If given, \a ok is set to true/false on success/failure.
    Warning: To avoid unnecessary problems, *always* give full template qualifier e.g. ToString\<int\> */
template<class T>
std::string ToString(T value, bool *ok = nullptr)
{
    std::ostringstream s;
    s << value;
    if (ok != nullptr)
        *ok = !s.fail();
    return s.str();
}

//! Converts a value to string
/** If given, \a ok is set to true/false on success/failure.
    Warning: To avoid unnecessary problems, *always* give full template qualifier e.g. FromString\<int\> */
template<class T>
T FromString(const std::string &str, bool *ok = nullptr)
{
    std::istringstream s;
    s.str(str);
    T value;
    s >> value;
    if (ok != nullptr)
        *ok = !s.fail();
    return value;
}

//! Converts string of hex characters to int
unsigned int HexStringToInt(const std::string& str);

//! Replacement for sprintf()
std::string Format(const char *fmt, ...);

//! Returns a string with every occurence of \a oldStr in \a str replaced to \a newStr
std::string Replace(const std::string &str, const std::string &oldStr, const std::string &newStr);

//! Remove whitespace from the beginning of the given string (in place)
void TrimLeft(std::string& str);

//! Remove whitespace from the end of the given string (in place)
void TrimRight(std::string& str);

//! Remove whitespace from both ends of the given string (in place)
void Trim(std::string& str);

//! Split string by the given predicate predicate (similar to boost::split)
// TODO c++20 predicate instead of std::function ?
void Split(std::vector<std::string>& out, const std::string& str, std::function<bool(char)> should_split);

//! Convert utf-8 string to lower case (in place)
void ToLower(std::string& str);

//! Convert utf-8 string to upper case (in place)
void ToUpper(std::string& str);

//! Converts a wide Unicode char to a single UTF-8 encoded char
std::string UnicodeCharToUtf8(unsigned int ch);

//! Converts a wide Unicode string to a UTF-8 encoded string
std::string UnicodeStringToUtf8(const std::wstring &str);

//! Converts a UTF-8 encoded single character to wide Unicode char
unsigned int Utf8CharToUnicode(const std::string &ch);

//! Converts a UTF-8 encoded string to wide Unicode string
std::wstring Utf8StringToUnicode(const std::string &str);

//! Returns the size in bytes of UTF-8 character at given \a pos in a UTF-8 \a str
int Utf8CharSizeAt(const std::string &str, unsigned int pos);

//! Returns the length in characters of UTF-8 string \a str
std::size_t Utf8StringLength(const std::string &str);

//! Returns true if char is continuation UTF-8 byte
bool isUtf8ContinuationByte(char c);

} // namespace StrUtil

