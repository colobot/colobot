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
 * \file level/parser/parserline.h
 * \brief Class for one line from level file
 */

#pragma once

#include "level/parser/parserparam.h"

#include <filesystem>
#include <string>
#include <map>
#include <memory>

class CLevelParser;
class CLevelParserLine;
using CLevelParserLineUPtr = std::unique_ptr<CLevelParserLine>;

class CLevelParserLine
{
public:
    CLevelParserLine(int lineNumber, const std::string& command);
    CLevelParserLine(const std::string& command);

    //! Get line number
    int GetLineNumber() const;

    //! Get CLevelParser this line is part of
    CLevelParser* GetLevel() const;
    //! Set CLevelParser this line is part of
    void SetLevel(CLevelParser* level);

    const std::filesystem::path& GetLevelFilename() const;

    std::string GetCommand() const;
    void SetCommand(const std::string& command);

    CLevelParserParam* GetParam(const std::string& name);
    void AddParam(const std::string& name, CLevelParserParamUPtr value);

    friend std::ostream& operator<<(std::ostream& str, const CLevelParserLine& line);

private:
    CLevelParser* m_level = nullptr;
    std::filesystem::path m_levelFilename = {};
    int m_lineNumber = 0;
    std::string m_command = {};
    std::map<std::string, CLevelParserParamUPtr> m_params;
};
