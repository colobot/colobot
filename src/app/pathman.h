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

/**
 *  \file app/pathman.h
 *  \brief Class for managing data/lang/save paths
 */

#pragma once

#include "common/singleton.h"

#include <string>

class CSystemUtils;

/**
 *  \class CPathManager
 *  \brief Class for managing data/lang/save paths
 */
class CPathManager : public CSingleton<CPathManager>
{
public:
    CPathManager(CSystemUtils* systemUtils);
    ~CPathManager();

    void SetDataPath(std::string dataPath);
    void SetLangPath(std::string langPath);
    void SetSavePath(std::string savePath);
    void AddMod(std::string modPath);

    const std::string& GetDataPath();
    const std::string& GetLangPath();
    const std::string& GetSavePath();

    //! Checks if paths are configured correctly
    std::string VerifyPaths();
    //! Loads configured paths
    void InitPaths();

private:
    //! Loads all mods from given directory
    void LoadModsFromDir(const std::string &dir);

private:
    CSystemUtils* m_systemUtils;
    //! Data path
    std::string m_dataPath;
    //! Lang path
    std::string m_langPath;
    //! Save path
    std::string m_savePath;
};
