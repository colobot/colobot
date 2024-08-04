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

constexpr bool UTF8IsContinuationByte(char8_t ch)
{
    return (ch & 0b1100'0000) == 0b1000'0000;
}

constexpr int UTF8SequenceLength(char8_t ch)
{
    if (ch < 0b1000'0000)           // 1-byte sequence
        return 1;
    else if (ch < 0b1100'0000)      // Continuation byte
        return 0;
    else if (ch < 0b1110'0000)      // 2-byte sequence
        return 2;
    else if (ch < 0b1111'0000)      // 3-byte sequence
        return 3;
    else if (ch < 0b1111'1000)      // 4-byte sequence
        return 4;
    else                            // Invalid sequence
        return 0;
}

template<unsigned Count, unsigned Offset>
constexpr char8_t Extract(char32_t ch)
{
    return static_cast<char8_t>((ch >> Offset) & ((1 << Count) - 1));
}

constexpr StrUtils::CodePoint UTF8Encode(char32_t ch)
{
    std::array<char, 4> result = { 0, 0, 0, 0 };
    int len = 0;

    if (ch < 0x80)
    {
        result[0] = static_cast<char>(ch);
        len = 1;
    }
    else if (ch < 0x800)
    {
        result[0] = static_cast<char>(0b1100'0000 | Extract<5, 6>(ch));
        result[1] = static_cast<char>(0b1000'0000 | Extract<6, 0>(ch));
        len = 2;
    }
    else if (ch < 0x01'0000)
    {
        result[0] = static_cast<char>(0b1110'0000 | Extract<4, 12>(ch));
        result[1] = static_cast<char>(0b1000'0000 | Extract<6, 6>(ch));
        result[2] = static_cast<char>(0b1000'0000 | Extract<6, 0>(ch));
        len = 3;
    }
    else if (ch < 0x11'0000)
    {
        result[0] = static_cast<char>(0b1111'0000 | Extract<3, 18>(ch));
        result[1] = static_cast<char>(0b1000'0000 | Extract<6, 12>(ch));
        result[2] = static_cast<char>(0b1000'0000 | Extract<6, 6>(ch));
        result[3] = static_cast<char>(0b1000'0000 | Extract<6, 0>(ch));
        len = 4;
    }

    return std::string_view(result.data(), len);
}

constexpr char32_t UTF8Decode(StrUtils::CodePoint code)
{
    char32_t result = 0;

    if (code.Size() == 1)
        result = static_cast<char32_t>(code[0]);
    else if (code.Size() == 2)
    {
        result |= static_cast<char32_t>(code[0] & 0b0001'1111) << 6;
        result |= static_cast<char32_t>(code[1] & 0b0011'1111);
    }
    else if (code.Size() == 3)
    {
        result |= static_cast<char32_t>(code[0] & 0b0000'1111) << 12;
        result |= static_cast<char32_t>(code[1] & 0b0011'1111) << 6;
        result |= static_cast<char32_t>(code[2] & 0b0011'1111);
    }
    else if (code.Size() == 4)
    {
        result |= static_cast<char32_t>(code[0] & 0b0000'1111) << 18;
        result |= static_cast<char32_t>(code[1] & 0b0011'1111) << 12;
        result |= static_cast<char32_t>(code[2] & 0b0011'1111) << 6;
        result |= static_cast<char32_t>(code[3] & 0b0011'1111);
    }

    return result;
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

    int len = UTF8SequenceLength(static_cast<char8_t>(string.front()));

    // UTF-8 sequence is not long enough
    if (string.size() < len) return 0;

    // Check continuation bytes
    for (int i = 1; i < len; i++)
        if (!UTF8IsContinuationByte(static_cast<char8_t>(string[i])))
            return 0;

    return len;
}

int StrUtils::UTF8StringLength(std::string_view string)
{
    int length = 0;

    while (!string.empty())
    {
        auto count = UTF8SequenceLength(static_cast<char8_t>(string.front()));

        if (count == 0)
            throw std::invalid_argument("Invalid character");
        
        for (int i = 1; i < count; i++)
            if (!UTF8IsContinuationByte(static_cast<char8_t>(string[i])))
                throw std::invalid_argument("Invalid character");

        length++;

        string.remove_prefix(count);
    }

    return length;
}

bool StrUtils::IsUTF8ContinuationByte(char c)
{
    return UTF8IsContinuationByte(static_cast<char8_t>(c));
}

CodePoint StrUtils::ReadUTF8(std::string_view text)
{
    if (text.empty()) return {};

    int len = UTF8CharLength(text);

    if (len == 0) return {};

    return text.substr(0, len);
}

CodePoint StrUtils::ToUTF8(char32_t code)
{
    return UTF8Encode(code);
}

char32_t StrUtils::ToUTF32(CodePoint code)
{
    return UTF8Decode(code);
}

std::string StrUtils::ToUTF8(std::u32string_view text)
{
    std::string result;

    for (auto c : text)
    {
        CodePoint code = UTF8Encode(c);

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

        result.push_back(UTF8Decode(code));

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
