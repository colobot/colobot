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
