/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2016, Daniel Roux, EPSITEC SA & TerranovaTeam
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
 * \file level/parser/parser.h
 * \brief Parser for level files
 */

#pragma once

#include "common/make_unique.h"

#include "level/level_category.h"

#include "level/parser/parserexceptions.h"
#include "level/parser/parserline.h"
#include "level/parser/parserparam.h"

#include <string>
#include <vector>
#include <memory>

class CLevelReadOnlyParser
{
public:
    virtual const std::vector<CLevelParserLineUPtr>& GetLines() = 0;
    virtual int CountLines(const std::string& command) = 0;
};

class CLevelParser : public CLevelReadOnlyParser
{
public:
    //! Create an empty level file
    CLevelParser();
    //! Load level from file
    CLevelParser(std::string filename);
    //! Load given level
    //@{
    CLevelParser(LevelCategory category, int chapter, int rank);
    CLevelParser(std::string category, int chapter, int rank);
    //@}

    //! Build category path
    //@{
    static std::string BuildCategoryPath(LevelCategory category);
    static std::string BuildCategoryPath(std::string category);
    //@}
    //! Build level filename
    //@{
    static std::string BuildScenePath(LevelCategory category, int chapter, int rank, bool sceneFile = true);
    static std::string BuildScenePath(std::string category, int chapter, int rank, bool sceneFile = true);
    //@}

    //! Check if level file exists
    bool Exists();
    //! Load file
    void Load();
    //! Load from raw text, '\n' splits text into lines
    void LoadFromText();
    //! Save file
    void Save();

    //! Configure level paths for the given level
    void SetLevelPaths(LevelCategory category, int chapter = 0, int rank = 0);
    //! Inject %something% paths
    std::string InjectLevelPaths(const std::string& path, const std::string& defaultDir = "");

    //! Get filename
    const std::string& GetFilename();

    //! Get all lines from file
    virtual const std::vector<CLevelParserLineUPtr>& GetLines();

    //! Insert new line to file
    void AddLine(CLevelParserLineUPtr line);

    //! Find first line with given command
    CLevelParserLine* Get(const std::string& command);

    //! Find first line with given command, null if doesn't exist
    CLevelParserLine* GetIfDefined(const std::string &command);

    //! Count lines with given command
    virtual int CountLines(const std::string& command);

private:
    void LoadFromStream(std::istream& stream);

    std::string m_filename;
    std::vector<CLevelParserLineUPtr> m_lines;

    std::string m_pathCat;
    std::string m_pathChap;
    std::string m_pathLvl;
};

inline std::string InjectLevelPathsForCurrentLevel(const std::string& path, LevelCategory category, int chap, int rank, const std::string& defaultDir = "")
{
    auto levelParser = MakeUnique<CLevelParser>();
    levelParser->SetLevelPaths(category, chap, rank);
    return levelParser->InjectLevelPaths(path, defaultDir);
}
