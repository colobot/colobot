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
 * \file level/parser/parser.h
 * \brief Parser for level files
 */

#pragma once

#include "level/level_category.h"
#include "level/robotmain.h"

#include "level/parser/parserexceptions.h"
#include "level/parser/parserline.h"
#include "level/parser/parserparam.h"

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>
#include <memory>

class CLevelParser
{
public:
    //! Create an empty level file
    CLevelParser();
    //! Load level from file
    CLevelParser(const std::filesystem::path& filename);
    //! Load given level
    //@{
    CLevelParser(LevelCategory category, int chapter, int rank);
    CLevelParser(std::string_view category, int chapter, int rank);
    //@}

    //! Build category path
    //@{
    static std::filesystem::path BuildCategoryPath(LevelCategory category);
    static std::filesystem::path BuildCategoryPath(std::string_view category);
    //@}
    //! Build level filename
    //@{
    static std::filesystem::path BuildScenePath(LevelCategory category, int chapter, int rank, bool sceneFile = true);
    static std::filesystem::path BuildScenePath(std::string_view category, int chapter, int rank, bool sceneFile = true);
    //@}

    //! Check if level file exists
    bool Exists() const;
    //! Load file
    void Load();
    //! Save file
    void Save();

    //! Configure level paths for the given level
    void SetLevelPaths(LevelCategory category, int chapter = 0, int rank = 0);
    //! Inject %something% paths
    std::filesystem::path InjectLevelPaths(const std::filesystem::path& path, const std::filesystem::path& defaultDir = "");

    //! Get filename
    const std::filesystem::path& GetFilename() const;

    //! Get all lines from file
    inline const std::vector<CLevelParserLineUPtr>& GetLines()
    {
        return m_lines;
    }

    //! Insert new line to file
    void AddLine(CLevelParserLineUPtr line);

    //! Find first line with given command
    CLevelParserLine* Get(const std::string& command);

    //! Find first line with given command, null if doesn't exist
    CLevelParserLine* GetIfDefined(const std::string &command);

    //! Count lines with given command
    int CountLines(const std::string& command);

private:
    std::filesystem::path m_filename;
    std::vector<CLevelParserLineUPtr> m_lines;

    std::filesystem::path m_pathCat;
    std::filesystem::path m_pathChap;
    std::filesystem::path m_pathLvl;
};

inline std::filesystem::path InjectLevelPathsForCurrentLevel(const std::filesystem::path& path, const std::filesystem::path& defaultDir = "")
{
    CRobotMain* main = CRobotMain::GetInstancePointer();
    auto levelParser = std::make_unique<CLevelParser>();
    levelParser->SetLevelPaths(main->GetLevelCategory(), main->GetLevelChap(), main->GetLevelRank());
    return levelParser->InjectLevelPaths(path, defaultDir);
}
