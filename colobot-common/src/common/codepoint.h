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
 * \file common/codepoint.h
 * \brief Implementation of CodePoint class for storing UTF-8 code points
 */

#pragma once

#include <algorithm>
#include <array>
#include <string_view>

namespace StrUtils
{


/**
 * \class CodePoint
 * \brief CodePoint class for storing UTF-8 code points
 */
class CodePoint
{
public:
    //! Creates an empty code point
    constexpr CodePoint() = default;

    //! Creates a code point from a string view, up to 4 characters
    constexpr CodePoint(std::string_view chars)
    {
        _size = static_cast<unsigned char>(std::min(4u, static_cast<unsigned>(chars.size())));

        for (unsigned i = 0; i < _size; i++)
            _chars[i] = chars[i];
    }

    //! Returns the start of code unit array
    constexpr const char* Data() const
    {
        return _chars.data();
    }

    //! Returns the number of code units
    constexpr unsigned Size() const
    {
        return _size;
    }

    //! Returns the code unit under given index
    constexpr const char& operator[](unsigned index) const
    {
        return _chars[index];
    }

private:
    std::array<char, 5> _chars = { '\0' };
    unsigned char _size = 0;
};

} // namespace StrUtil
