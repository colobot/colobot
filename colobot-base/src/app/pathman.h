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

#pragma once

#include <filesystem>
#include <string>
#include <vector>

class CSystemUtils;

/**
 *  \class CPathManager
 *  \brief Class for managing data/lang/save paths
 */
class CPathManager
{
public:
    CPathManager(CSystemUtils* systemUtils);
    ~CPathManager();

    void SetDataPath(const std::filesystem::path& dataPath);
    void SetLangPath(const std::filesystem::path& langPath);
    void SetSavePath(const std::filesystem::path& savePath);

    const std::filesystem::path& GetDataPath() const;
    const std::filesystem::path& GetLangPath() const;
    const std::filesystem::path& GetSavePath() const;

    //! Checks if paths are configured correctly
    std::string VerifyPaths() const;
    //! Loads configured paths
    void InitPaths();

    //! Adds a path to a mod
    void AddMod(const std::filesystem::path& path);
    //! Find paths to mods in mod search directories
    std::vector<std::filesystem::path> FindMods() const;
    //! Adds a mod search directory
    void AddModSearchDir(const std::filesystem::path& modSearchDirPath);

private:
    std::vector<std::filesystem::path> FindModsInDir(const std::filesystem::path& dir) const;

private:
    //! Data path
    std::filesystem::path m_dataPath;
    //! Lang path
    std::filesystem::path m_langPath;
    //! Save path
    std::filesystem::path m_savePath;
    //! Mod search paths
    std::vector<std::filesystem::path> m_modSearchDirs;
    //! Additional mod paths
    std::vector<std::filesystem::path> m_mods;
};
