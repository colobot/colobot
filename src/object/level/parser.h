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
 * \file object/level/parser.h
 * \brief Parser for level files
 */

#pragma once

#include "object/level/parserline.h"
#include "object/level/parserparam.h"
#include "object/level/parserexceptions.h"

#include <string>
#include <vector>

class CLevelParser
{
public:
    //! Create an empty level file
    CLevelParser();
    //! Load level from file
    CLevelParser(std::string filename);
    //! Load given level
    CLevelParser(std::string category, int chapter, int rank);
    
    ~CLevelParser();
    
    //! Build level filename
    static std::string BuildSceneName(std::string category, int chapter, int rank, bool sceneFile = true);
    
    //! Check if level file exists
    bool Exists();
    //! Load file
    void Load();
    //! Save file
    void Save(std::string filename);
    
    //! Get filename
    const std::string& GetFilename();
    
    //! Get all lines from file
    std::vector<CLevelParserLine*> GetLines();
    //! Insert new line to file
    void AddLine(CLevelParserLine* line);
    
    //! Find first line with given command
    CLevelParserLine* Get(std::string command);
    
private:
    
    std::string m_filename;
    std::vector<CLevelParserLine*> m_lines;
};