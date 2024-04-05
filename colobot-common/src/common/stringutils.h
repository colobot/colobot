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

/**
 * \file common/stringutils.h
 * \brief Some useful string operations
 */

#pragma once

#include <cstddef>
#include <filesystem>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace StrUtils
{

class CodePoint;

//! Converts a value to string
/** If given, \a ok is set to true/false on success/failure.
    Warning: To avoid unnecessary problems, *always* give full template qualifier e.g. ToString\<int\> */
template<class T>
std::string ToString(const T& value, bool *ok = nullptr)
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

//! Converts the string into another type by casting code units individually
template<typename To, typename From>
To Cast(const From& text)
{
    if constexpr (std::is_same_v<From, To>)
    {
        return text;
    }
    else
    {
        using Char = typename To::value_type;

        To result;

        for (auto ch : text)
            result.push_back(static_cast<Char>(ch));

        return result;
    }
}

template<>
std::string ToString(const std::filesystem::path& path, bool *ok);

template<>
std::filesystem::path FromString(const std::string& path, bool *ok);

// Converts UTF-8 encoded string to std::filesystem::path
std::filesystem::path ToPath(std::string_view path);

//! Converts string of hex characters to int
unsigned int HexStringToInt(std::string_view str);

//! Replacement for sprintf()
std::string Format(const char *fmt, ...);

//! Returns a string with every occurence of \a oldStr in \a str replaced to \a newStr
std::string Replace(const std::string &str, const std::string &oldStr, const std::string &newStr);

//! Splits a string into parts using character separators
std::vector<std::string> Split(const std::string& text, std::string_view separators);

//! Remove whitespace from the beginning of the given string (in place)
void TrimLeft(std::string& str);

//! Remove whitespace from the end of the given string (in place)
void TrimRight(std::string& str);

//! Remove whitespace from both ends of the given string (in place)
void Trim(std::string& str);

//! Removes comments of form // comment
void RemoveComments(std::string& text);

//! Returns the length in characters of first UTF-8 code point in \a string
int UTF8CharLength(std::string_view string);

//! Returns the length in characters of UTF-8 string \a string
int UTF8StringLength(std::string_view string);

//! Returns true if char is continuation UTF-8 byte
bool IsUTF8ContinuationByte(char c);

//! Reads UTF-8 character
CodePoint ReadUTF8(std::string_view text);

//! Converts UTF-32 code point to UTF-8
CodePoint ToUTF8(char32_t code);

//! Converts UTF-8 code point to UTF-32
char32_t ToUTF32(CodePoint code);

//! Converts UTF-32 string to UTF-8
std::string ToUTF8(std::u32string_view text);

//! Converts UTF-8 string to UTF-32
std::u32string ToUTF32(std::string_view text);

//! Returns the character converted to lower case when possible
char32_t ToLower(char32_t ch);

//! Returns the character converted to upper case when possible
char32_t ToUpper(char32_t ch);

//! Returns the string with characters converted to lower case when possible
std::string ToLower(std::string_view text);

//! Returns the string with characters converted to upper case when possible
std::string ToUpper(std::string_view text);

} // namespace StrUtil

[[deprecated("Temporary conversion")]]
inline std::string TempToString(const std::filesystem::path& path)
{
    return StrUtils::ToString(path);
}

[[deprecated("Temporary conversion")]]
inline std::filesystem::path TempToPath(std::string_view path)
{
    return StrUtils::ToPath(path);
}
