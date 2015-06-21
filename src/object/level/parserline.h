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

/**
 * \file object/level/parserline.h
 * \brief Class for one line from level file
 */

#pragma once

#include "object/level/parserparam.h"

#include <string>
#include <map>
#include <memory>

class CLevelParser;
class CLevelParserLine;
using CLevelParserLineUPtr = std::unique_ptr<CLevelParserLine>;

class CLevelParserLine
{
public:
    CLevelParserLine(int lineNumber, std::string command);
    CLevelParserLine(std::string command);

    //! Get line number
    int GetLineNumber();

    //! Get CLevelParser this line is part of
    CLevelParser* GetLevel();
    //! Set CLevelParser this line is part of
    void SetLevel(CLevelParser* level);

    std::string GetCommand();
    void SetCommand(std::string command);

    CLevelParserParam* GetParam(std::string name);
    void AddParam(std::string name, CLevelParserParamUPtr value);

    friend std::ostream& operator<<(std::ostream& str, const CLevelParserLine& line);

private:
    CLevelParser* m_level;
    int m_lineNumber;
    std::string m_command;
    std::map<std::string, CLevelParserParamUPtr> m_params;
};
