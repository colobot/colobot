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

#pragma once

#include "common/stringutils.h"

#include "level/parser/parser.h"

#include <stdexcept>
#include <string>

class UnusedCmdException : public std::runtime_error
{
public:
    explicit UnusedCmdException(CLevelParserLine* line)
        : std::runtime_error(MakeMessage(line))
    {}

private:
    static std::string MakeMessage(CLevelParserLine* line)
    {
        const std::string command = line->GetCommand();
        const std::string fileName = StrUtils::ToString(line->GetLevelFilename());
        const std::string lineNumber = std::to_string(line->GetLineNumber());

        return "Unknown command '" + command + "' (in " + fileName + ":" + lineNumber + ")";
    }
};

class UnusedParamException : public std::runtime_error
{
public:
    explicit UnusedParamException(CLevelParserLine* line, std::string key)
        : std::runtime_error(MakeMessage(line, key))
    {}

private:
    static std::string MakeMessage(CLevelParserLine* line, std::string key)
    {
        const std::string command = line->GetCommand();
        const std::string fileName = StrUtils::ToString(line->GetLevelFilename());
        const std::string lineNumber = std::to_string(line->GetLineNumber());

        return "Unknown parameter '" + key + "' in command '" + command + "' (in " + fileName + ":" + lineNumber + ")";
    }
};
