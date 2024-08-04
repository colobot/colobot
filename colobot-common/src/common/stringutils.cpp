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

#include "common/codepoint.h"

#include <algorithm>
#include <array>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cwchar>
#include <cwctype>
#include <limits>
#include <locale>
#include <optional>
#include <stdexcept>
#include <vector>

namespace
{

std::optional<std::locale> GetLocale(const char* name)
try
{
    return std::locale(name);
}
catch(...)
{
    return std::nullopt;
}

std::locale GetConversionLocale()
{
    if (auto locale = GetLocale("en_US.UTF-8"))
    {
        return *locale;
    }
    else if (auto locale = GetLocale("C.UTF-8"))
    {
        return *locale;
    }
    else
    {
        return std::locale("");
    }
}

const std::locale convertion_locale = GetConversionLocale();

#ifndef COLOBOT_CHAR8_T_OVERRIDE
using UTF8Char = char8_t;
#else
using UTF8Char = char;
#endif

const auto& wchar = std::use_facet<std::ctype<wchar_t>>(convertion_locale);
const auto& utf32 = std::use_facet<std::codecvt<char32_t, UTF8Char, std::mbstate_t>>(convertion_locale);

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

using namespace StrUtils;

template<>
std::string StrUtils::ToString(const std::filesystem::path& path, bool *ok)
{
    if (ok != nullptr)
        *ok = true;

    return Cast<std::string>(path.generic_u8string());
}

template<>
std::filesystem::path StrUtils::FromString(const std::string& path, bool *ok)
{
    if (ok != nullptr)
        *ok = true;

    return ToPath(path);
}

std::filesystem::path StrUtils::ToPath(std::string_view path)
{
#ifndef COLOBOT_CHAR8_T_OVERRIDE
    auto data = reinterpret_cast<const char8_t*>(path.data());

    return std::filesystem::path(data, data + path.size());
#else
    return std::filesystem::u8path(path.begin(), path.end());
#endif
}

unsigned int StrUtils::HexStringToInt(std::string_view str)
{
    auto parse = [](char c) -> unsigned
    {
        if ('0' <= c && c <= '9') return static_cast<unsigned>(c - '0');
        if ('A' <= c && c <= 'F') return static_cast<unsigned>(c - 'A' + 10);
        if ('a' <= c && c <= 'f') return static_cast<unsigned>(c - 'a' + 10);
        
        throw std::invalid_argument(std::string("Invalid character: ") + c);
    };

    unsigned result = 0;

    for (char c : str)
    {
        result = (result << 4) | parse(c);
    }

    return result;
}

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

void StrUtils::RemoveComments(std::string& text)
{
    if (text.empty()) return;

    for (std::size_t i = 0; i < text.size() - 1; i++)
    {
        char c = text[i];

        // If a string literal of form "text" or 'text', skip
        if (c == '"' || c == '\'')
        {
            size_t j = i + 1;

            while (j < text.size())
            {
                if (text[j] == c) break;

                j++;
            }

            i = j;
        }
        // If a comment of form // comment, remove and end processing
        else if (text[i] == '/' && text[i + 1] == '/')
        {
            text.erase(std::next(text.begin(), i), text.end());
            break;
        }
    }
}

int StrUtils::UTF8CharLength(std::string_view string)
{
    if (string.empty()) return 0;

    std::mbstate_t state = {};

    auto data = reinterpret_cast<const UTF8Char*>(string.data());

    return utf32.length(state, data, data + string.size(), 1);
}

int StrUtils::UTF8StringLength(std::string_view string)
{
    int length = 0;

    while (!string.empty())
    {
        std::mbstate_t state = {};

        auto data = reinterpret_cast<const UTF8Char*>(string.data());

        auto count = utf32.length(state, data, data + string.size(), 1);

        if (count == 0)
            throw std::invalid_argument("Invalid character");

        length++;

        string.remove_prefix(count);
    }

    return length;
}

bool StrUtils::IsUTF8ContinuationByte(char c)
{
    return (c & 0b1100'0000) == 0b1000'0000;
}

CodePoint StrUtils::ReadUTF8(std::string_view text)
{
    if (text.empty()) return {};

    std::mbstate_t state = {};

    auto data = reinterpret_cast<const UTF8Char*>(text.data());

    int len = utf32.length(state, data, data + text.size(), 1);

    if (len == 0) return {};

    return text.substr(0, len);
}

CodePoint StrUtils::ToUTF8(char32_t code)
{
    std::mbstate_t state = {};
    std::array<UTF8Char, 4> buffer;

    const char32_t* read = nullptr;
    UTF8Char* written = nullptr;

    auto result = utf32.out(state, &code, &code + 1, read,
        buffer.data(), buffer.data() + buffer.size(), written);

    if (result != std::codecvt_base::ok)
        return {};

    auto data = reinterpret_cast<const char*>(buffer.data());

    return CodePoint(std::string_view(data, written - buffer.data()));
}

char32_t StrUtils::ToUTF32(CodePoint code)
{
    std::mbstate_t state = {};
    char32_t ch = {};

    const UTF8Char* read = nullptr;
    char32_t* written = nullptr;

    auto data = reinterpret_cast<const UTF8Char*>(code.Data());

    auto result = utf32.in(state,
        data, data + code.Size(), read,
        &ch, &ch + 1, written);

    if (result != std::codecvt_base::ok)
        throw std::invalid_argument("Invalid code point");
    
    return ch;
}

std::string StrUtils::ToUTF8(std::u32string_view text)
{
    std::string result;

    for (auto c : text)
    {
        CodePoint code = ToUTF8(c);

        if (code.Size() == 0)
            throw std::invalid_argument("Invalid character");

        result.append(code.Data(), code.Size());
    }

    return result;
}

std::u32string StrUtils::ToUTF32(std::string_view text)
{
    std::u32string result;

    while (!text.empty())
    {
        CodePoint code = ReadUTF8(text);

        if (code.Size() == 0)
            throw std::invalid_argument("Invalid character");

        result.push_back(ToUTF32(code));

        text.remove_prefix(code.Size());
    }

    return result;
}

char32_t StrUtils::ToLower(char32_t ch)
{
    if (ch > std::numeric_limits<char16_t>::max()) return ch;

    return static_cast<char32_t>(wchar.tolower(static_cast<wchar_t>(ch)));
}

char32_t StrUtils::ToUpper(char32_t ch)
{
    if (ch > std::numeric_limits<char16_t>::max()) return ch;

    return static_cast<char32_t>(wchar.toupper(static_cast<wchar_t>(ch)));
}

std::string StrUtils::ToLower(std::string_view text)
{
    std::string result;
    result.reserve(text.size());

    while (!text.empty())
    {
        CodePoint code = ReadUTF8(text);
        if (code.Size() == 0) throw std::invalid_argument("Invalid character");

        text.remove_prefix(code.Size());

        char32_t ch = ToUTF32(code);
        ch = ToLower(ch);

        code = ToUTF8(ch);
        if (code.Size() == 0) throw std::invalid_argument("Invalid character");

        result.append(code.Data(), code.Size());
    }

    return result;
}

std::string StrUtils::ToUpper(std::string_view text)
{
    std::string result;
    result.reserve(text.size());

    while (!text.empty())
    {
        CodePoint code = ReadUTF8(text);
        if (code.Size() == 0) throw std::invalid_argument("Invalid character");

        text.remove_prefix(code.Size());

        char32_t ch = ToUTF32(code);
        ch = ToUpper(ch);

        code = ToUTF8(ch);
        if (code.Size() == 0) throw std::invalid_argument("Invalid character");

        result.append(code.Data(), code.Size());
    }

    return result;
}
