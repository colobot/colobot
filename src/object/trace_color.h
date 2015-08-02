/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2015, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsiteс.ch; http://colobot.info; http://github.com/colobot
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
#pragma once

#include <string>

enum class TraceColor : int
{
    Default = -1,

    White = 0,
    Black = 1,
    Gray = 2,
    LightGray = 3,
    Red = 4,
    Pink = 5,
    Purple = 6,
    Orange = 7,
    Yellow = 8,
    Beige = 9,
    Brown = 10,
    Skin = 11,
    Green = 12,
    LightGreen = 13,
    Blue = 14,
    LightBlue = 15,
    BlackArrow = 16,
    RedArrow = 17,
    Max,
};

std::string TraceColorName(TraceColor c);
