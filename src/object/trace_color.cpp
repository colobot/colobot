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
#include "object/trace_color.h"

std::string TraceColorName(TraceColor color)
{
    switch(color)
    {
        case TraceColor::White:      return "White";      break;
        case TraceColor::Black:      return "Black";      break;
        case TraceColor::Gray:       return "Gray";       break;
        case TraceColor::LightGray:  return "LightGray";  break;
        case TraceColor::Red:        return "Red";        break;
        case TraceColor::Pink:       return "Pink";       break;
        case TraceColor::Purple:     return "Purple";     break;
        case TraceColor::Orange:     return "Orange";     break;
        case TraceColor::Yellow:     return "Yellow";     break;
        case TraceColor::Beige:      return "Beige";      break;
        case TraceColor::Brown:      return "Brown";      break;
        case TraceColor::Skin:       return "Skin";       break;
        case TraceColor::Green:      return "Green";      break;
        case TraceColor::LightGreen: return "LightGreen"; break;
        case TraceColor::Blue:       return "Blue";       break;
        case TraceColor::LightBlue:  return "LightBlue";  break;
        case TraceColor::RedArrow:   return "RedArrow";   break;
        case TraceColor::BlackArrow: return "BlackArrow"; break;
        default:                     return "";           break;
    }
}
