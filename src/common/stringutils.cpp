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
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cwchar>
#include <cwctype>
#include <stdexcept>
#include <vector>

unsigned int StrUtils::HexStringToInt(const std::string& str)
{
    std::stringstream ss;
    ss << std::hex << str;
    unsigned int x;
    ss >> x;
    return x;
}

namespace
{

std::string VFormat(const char *fmt, va_list ap)
{
    std::array<char, 1024> stackbuf;
    std::vector<char> dynamicbuf;
    char *buf = stackbuf.data();
    size_t size = stackbuf.size();

    while (true)
    {
        int needed = vsnprintf (buf, size, fmt, ap);

        if (needed <= static_cast<int>(size) && needed >= 0)
        {
            return std::string(buf, static_cast<std::size_t>(needed));
        }

        size = (needed > 0) ? (needed+1) : (size*2);
        dynamicbuf.resize(size);
        buf = dynamicbuf.data();
    }
}

using UTF8Buffer = std::array<char, 8>;

} // anonymous namespace

std::string StrUtils::Format(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    std::string buf = VFormat(fmt, ap);
    va_end(ap);
    return buf;
}

std::string StrUtils::Replace(const std::string &str, const std::string &oldStr, const std::string &newStr)
{
    std::string result = str;
    std::size_t pos = 0;
    while ((pos = result.find(oldStr, pos)) != std::string::npos)
    {
        result.replace(pos, oldStr.length(), newStr);
        pos += newStr.length();
    }
    return result;
}

std::vector<std::string> StrUtils::Split(const std::string& text, std::string_view separators)
{
    std::string_view stream = text;

    std::vector<std::string> result;
    std::string part;

    while (!stream.empty())
    {
        char c = stream.front();
        stream.remove_prefix(1);

        if (separators.find(c) != std::string::npos)
        {
            if (!part.empty())
            {
                result.push_back(part);
                part.clear();
            }
            continue;
        }

        part += c;
    }

    if (!part.empty()) result.push_back(part);

    return result;
}

void StrUtils::TrimLeft(std::string& s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

void StrUtils::TrimRight(std::string& s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

void StrUtils::Trim(std::string& str)
{
    TrimLeft(str);
    TrimRight(str);
}

std::string StrUtils::UnicodeCharToUtf8(unsigned int ch)
{
    UTF8Buffer buffer;

    std::mbstate_t state = {};

    int count = wcrtomb(buffer.data(), static_cast<wchar_t>(ch), &state);

    if (count == 0) count = 1;
    else if (count == -1) throw std::invalid_argument("Invalid character");

    return std::string(buffer.data(), count);
}

std::string StrUtils::UnicodeStringToUtf8(const std::wstring &str)
{
    std::string result;
    result.reserve(str.size());

    UTF8Buffer buffer;

    for (const auto& ch : str)
    {
        std::mbstate_t state = {};

        size_t len = std::wcrtomb(buffer.data(), ch, &state);

        if (len == 0) len = 1;
        else if (len == -1) throw std::invalid_argument("Invalid character");

        result.append(buffer.data(), len);
    }

    return result;
}

unsigned int StrUtils::Utf8CharToUnicode(const std::string &ch)
{
    if (ch.empty())
        return 0;

    std::mbstate_t state = {};

    wchar_t c = 0;

    int len = mbrtowc(&c, ch.data(), ch.size(), &state);

    if (len == 0) return L'\0';
    else if (len == -1) throw std::invalid_argument("Invalid character");
    else if (len == -2) throw std::invalid_argument("Invalid character");

    return c;
}

std::wstring StrUtils::Utf8StringToUnicode(const std::string &str)
{
    std::wstring result;
    result.reserve(str.size());

    for (size_t i = 0; i < str.size();)
    {
        std::mbstate_t state = {};
        wchar_t ch;

        int len = std::mbrtowc(&ch, str.data() + i, str.size() - i, &state);

        if (len == 0) len = 1;
        else if (len == -1) throw std::invalid_argument("Invalid character");
        else if (len == -2) throw std::invalid_argument("Invalid character");

        i += len;

        result += ch;
    }

    return result;
}

int StrUtils::Utf8CharSizeAt(const std::string &str, unsigned int pos)
{
    if (pos >= str.size())
        throw std::out_of_range("Index is greater than size");

    std::mbstate_t state = {};

    int len = std::mbrlen(str.data() + pos, str.size() - pos, &state);

    if (len == 0) len = 1;
    else if (len == -1) throw std::invalid_argument("Invalid character");
    else if (len == -2) throw std::invalid_argument("Invalid character");

    return len;
}

std::size_t StrUtils::Utf8StringLength(const std::string &str)
{
    std::size_t result = 0;

    for (size_t i = 0; i < str.size();)
    {
        std::mbstate_t state = {};

        size_t count = std::mbrlen(str.data() + i, str.size() - i, &state);

        if (count == 0) count = 1;
        else if (count == -1) throw std::invalid_argument("Invalid character");
        else if (count == -2) throw std::invalid_argument("Invalid character");

        i += count;

        ++result;
    }

    return result;
}

bool StrUtils::isUtf8ContinuationByte(char c)
{
    return (c & 0b1100'0000) == 0b1000'0000;
}

std::string StrUtils::ToLower(const std::string& text)
{
    std::string result;
    result.reserve(text.size());

    UTF8Buffer buffer;

    for (size_t i = 0; i < text.size();)
    {
        std::mbstate_t state = {};
        wchar_t ch;

        int len = std::mbrtowc(&ch, text.data() + i, text.size() - i, &state);

        if (len == 0) len = 1;
        else if (len == -1) throw std::invalid_argument("Invalid character");
        else if (len == -2) throw std::invalid_argument("Invalid character");

        i += len;

        ch = std::towlower(ch);

        state = {};
        size_t count = std::wcrtomb(buffer.data(), ch, &state);

        if (count == 0) count = 1;
        else if (count == -1) throw std::invalid_argument("Invalid character");

        result.append(buffer.data(), count);
    }

    return result;
}

std::string StrUtils::ToUpper(const std::string& text)
{
    std::string result;
    result.reserve(text.size());

    UTF8Buffer buffer;

    for (size_t i = 0; i < text.size();)
    {
        std::mbstate_t state = {};
        wchar_t ch;

        size_t len = std::mbrtowc(&ch, text.data() + i, text.size() - i, &state);

        if (len == 0) len = 1;
        else if (len == -1) throw std::invalid_argument("Invalid character");
        else if (len == -2) throw std::invalid_argument("Invalid character");

        i += len;

        ch = std::towupper(ch);

        state = {};
        size_t count = std::wcrtomb(buffer.data(), ch, &state);

        if (count == 0) count = 1;
        else if (count == -1) throw std::invalid_argument("Invalid character");

        result.append(buffer.data(), count);
    }

    return result;
}
