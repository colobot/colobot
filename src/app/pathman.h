/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2020, Daniel Roux, EPSITEC SA & TerranovaTeam
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

    void SetDataPath(const std::string &dataPath);
    void SetLangPath(const std::string &langPath);
    void SetSavePath(const std::string &savePath);
    void AddModSearchDir(const std::string &modSearchDirPath);
    void AddMod(const std::string &modPath);
    void RemoveMod(const std::string &modPath);
    bool ModLoaded(const std::string& modPath);
    std::vector<std::string> FindMods() const;

    const std::string& GetDataPath();
    const std::string& GetLangPath();
    const std::string& GetSavePath();

    //! Checks if paths are configured correctly
    std::string VerifyPaths();
    //! Loads configured paths
    void InitPaths();

private:
    //! Loads all mods from given directory
    std::vector<std::string> FindModsInDir(const std::string &dir) const;

private:
    //! Data path
    std::string m_dataPath;
    //! Lang path
    std::string m_langPath;
    //! Save path
    std::string m_savePath;
    //! Mod search paths
    std::vector<std::string> m_modSearchDirs;
};
