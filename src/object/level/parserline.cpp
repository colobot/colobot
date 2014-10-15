/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2014, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "object/level/parserline.h"


#include "object/level/parser.h"
#include "common/logger.h"

CLevelParserLine::CLevelParserLine(std::string command)
{
    m_command = command;
    m_lineNumber = 0;
}

CLevelParserLine::CLevelParserLine(int lineNumber, std::string command)
{
    m_command = command;
    m_lineNumber = lineNumber;
}

CLevelParserLine::~CLevelParserLine()
{
    for(auto param : m_params)
    {
        delete param.second;
    }
}

std::string CLevelParserLine::GetLine()
{
    assert(false); //TODO
}

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
    if(m_params[name] == nullptr) {
        CLevelParserParam* param = new CLevelParserParam(name, true);
        param->SetLine(this);
        m_params[name] = param;
    }
    return m_params[name];
}

void CLevelParserLine::AddParam(std::string name, CLevelParserParam* value)
{
    value->SetLine(this);
    m_params[name] = value;
}