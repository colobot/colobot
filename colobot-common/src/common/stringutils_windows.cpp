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


#include "common/stringutils.h"

#include <algorithm>
#include <array>
#include <stdexcept>
#include <vector>

#include <Windows.h>
#include <stringapiset.h>

std::string StrUtils::UnicodeCharToUtf8(unsigned int ch)
{
    wchar_t character = ch;
    std::array<char, 8> buffer;

    int length = WideCharToMultiByte(CP_UTF8, 0,
        &character, 1, buffer.data(), buffer.size(), nullptr, nullptr);

    if (length == 0) throw std::invalid_argument("Invalid character");

    return std::string(buffer.data(), length);
}

std::string StrUtils::UnicodeStringToUtf8(const std::wstring &str)
{
    int length = WideCharToMultiByte(CP_UTF8, 0,
        str.c_str(), str.size(), nullptr, 0, nullptr, nullptr);

    if (length == 0) throw std::invalid_argument("Invalid character");

    std::string result(length, '\0');

    length = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), str.size(),
        result.data(), result.size() + 1, nullptr, nullptr);

    if (length == 0) throw std::invalid_argument("Invalid character");

    return result;
}

unsigned int StrUtils::Utf8CharToUnicode(const std::string &ch)
{
    if (ch.empty())
        return 0;

    wchar_t c = 0;

    int length = MultiByteToWideChar(CP_UTF8, 0,
        ch.c_str(), ch.size(), &c, 1);

    if (length == 0) throw std::invalid_argument("Invalid character");

    return c;
}

std::wstring StrUtils::Utf8StringToUnicode(const std::string &str)
{
    int length = MultiByteToWideChar(CP_UTF8, 0,
        str.c_str(), str.size(), nullptr, 0);

    if (length == 0) throw std::invalid_argument("Invalid character");

    std::wstring result(length, L'\0');

    length = MultiByteToWideChar(CP_UTF8, 0,
        str.c_str(), str.size(), result.data(), length);

    if (length == 0) throw std::invalid_argument("Invalid character");

    return result;
}

int StrUtils::Utf8CharSizeAt(const std::string &str, unsigned int pos)
{
    if (pos >= str.size())
        throw std::out_of_range("Index is greater than size");

    std::array<wchar_t, 4> buffer;
    
    int length = MultiByteToWideChar(CP_UTF8, 0,
        str.data() + pos, min(4, str.size() - pos),
        buffer.data(), buffer.size());

    if (length == 0) throw std::invalid_argument("Invalid character");

    length = WideCharToMultiByte(CP_UTF8, 0,
        buffer.data(), 1, nullptr, 0, 0, 0);

    if (length == 0) throw std::invalid_argument("Invalid character");

    return length;
}

std::size_t StrUtils::Utf8StringLength(const std::string &str)
{
    int length = MultiByteToWideChar(CP_UTF8, 0,
        str.c_str(), str.size(), nullptr, 0);

    if (length == 0) throw std::invalid_argument("Invalid character");

    return length;
}

bool StrUtils::isUtf8ContinuationByte(char c)
{
    return (c & 0b1100'0000) == 0b1000'0000;
}

std::string StrUtils::ToLower(const std::string& text)
{
    std::wstring unicode = Utf8StringToUnicode(text);

    CharLowerBuffW(unicode.data(), unicode.size());

    return UnicodeStringToUtf8(unicode);
}

std::string StrUtils::ToUpper(const std::string& text)
{
    std::wstring unicode = Utf8StringToUnicode(text);

    CharUpperBuffW(unicode.data(), unicode.size());

    return UnicodeStringToUtf8(unicode);
}
