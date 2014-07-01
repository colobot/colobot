// * This file is part of the COLOBOT source code
// * Copyright (C) 2014, Polish Portal of Colobot (PPC)
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.

/**
 * \file app/gamedata.h
 * \brief Game data
 */
 
#pragma once

#include "common/singleton.h"

#include <string>
#include <vector>

/**
 * \enum DataDir
 * \brief Directories in data directory
 */
enum DataDir
{
    DIR_AI,       //! < ai scripts
    DIR_FONT,     //! < fonts
    DIR_HELP,     //! < help files
    DIR_ICON,     //! < icons & images
    DIR_LEVEL,    //! < levels
    DIR_MODEL,    //! < models
    DIR_MUSIC,    //! < music
    DIR_SOUND,    //! < sounds
    DIR_TEXTURE,  //! < textures

    DIR_MAX       //! < number of dirs
};

class CGameData : public CSingleton<CGameData>
{
public:
    CGameData();
    ~CGameData();
    
    void Init();
    void SetDataDir(std::string path);
    void AddMod(std::string path);
    
    std::string GetFilePath(DataDir dir, const std::string &subpath);
    std::string GetDataPath(const std::string &subpath);
    
private:
    bool m_dataDirSet;
    std::vector<std::string> m_dataDirs;
    const char* m_standardDataDirs[DIR_MAX];
};

