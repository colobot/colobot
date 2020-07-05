/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "level/parser/parserline.h"

#include "common/logger.h"
#include "common/make_unique.h"

#include "level/parser/parser.h"

CLevelParserLine::CLevelParserLine(std::string command)
    : m_level(nullptr),
      m_levelFilename(""),
      m_lineNumber(0),
      m_command(command)
{}

CLevelParserLine::CLevelParserLine(int lineNumber, std::string command)
    : m_level(nullptr),
      m_levelFilename(""),
      m_lineNumber(lineNumber),
      m_command(command)
{}

int CLevelParserLine::GetLineNumber()
{
    return m_lineNumber;
}

CLevelParser* CLevelParserLine::GetLevel()
{
    return m_level;
}

void CLevelParserLine::SetLevel(CLevelParser* level)
{
    m_level = level;

    // Only on the first call - this makes sure the level name doesn't change if the file is loaded using #Include
    if (m_levelFilename.empty())
    {
        m_levelFilename = m_level->GetFilename();
    }
}

const std::string& CLevelParserLine::GetLevelFilename()
{
    return m_levelFilename;
}

std::string CLevelParserLine::GetCommand()
{
    return m_command;
}

void CLevelParserLine::SetCommand(std::string command)
{
    m_command = command;
}

CLevelParserParam* CLevelParserLine::GetParam(std::string name)
{
    auto it = m_params.find(name);
    if (it != m_params.end())
    {
        return it->second.get();
    }

    auto paramUPtr = MakeUnique<CLevelParserParam>(name, true);
    paramUPtr->SetLine(this);
    CLevelParserParam* paramPtr = paramUPtr.get();
    m_params.insert(std::make_pair(name, std::move(paramUPtr)));
    return paramPtr;
}

void CLevelParserLine::AddParam(std::string name, CLevelParserParamUPtr value)
{
    value->SetLine(this);
    m_params.insert(std::make_pair(name, std::move(value)));
}

std::ostream& operator<<(std::ostream& str, const CLevelParserLine& line)
{
    str << line.m_command;
    for (const auto& param : line.m_params)
    {
        str << " " << param.first << "=" << param.second->GetValue();
    }
    return str;
}
