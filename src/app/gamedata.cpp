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

#include "app/gamedata.h"


#include "app/app.h"

#include <boost/filesystem.hpp>

template<> CGameData* CSingleton<CGameData>::m_instance = nullptr;

CGameData::CGameData()
{
    m_dataDirSet = false;
    
    for (int i = 0; i < DIR_MAX; ++i)
        m_standardDataDirs[i] = nullptr;

    m_standardDataDirs[DIR_AI]       = "ai";
    m_standardDataDirs[DIR_FONT]     = "fonts";
    m_standardDataDirs[DIR_HELP]     = "help";
    m_standardDataDirs[DIR_ICON]     = "icons";
    m_standardDataDirs[DIR_LEVEL]    = "levels";
    m_standardDataDirs[DIR_MODEL]    = "models";
    m_standardDataDirs[DIR_MUSIC]    = "music";
    m_standardDataDirs[DIR_SOUND]    = "sounds";
    m_standardDataDirs[DIR_TEXTURE]  = "textures";
}

CGameData::~CGameData()
{
}

void CGameData::SetDataDir(std::string path)
{
    assert(!m_dataDirSet);
    m_dataDirSet = true;
    
    m_dataDirs.insert(m_dataDirs.begin(), path);
}

void CGameData::AddMod(std::string path)
{
    m_dataDirs.push_back(path);
}

void CGameData::Init()
{
    std::string out = "Using datadirs: ";
    bool first = true;
    for(std::vector<std::string>::reverse_iterator rit = m_dataDirs.rbegin(); rit != m_dataDirs.rend(); ++rit) {
        if(!first) out += ", ";
        first = false;
        out += *rit;
    }
    out += "\n";
    CLogger::GetInstancePointer()->Info(out.c_str());
}

std::string CGameData::GetFilePath(DataDir dir, const std::string& subpath)
{
    int index = static_cast<int>(dir);
    assert(index >= 0 && index < DIR_MAX);
    
    for(std::vector<std::string>::reverse_iterator rit = m_dataDirs.rbegin(); rit != m_dataDirs.rend(); ++rit) {
        std::stringstream str;
        
        if ( subpath.find("save") == std::string::npos ){ // if its NOT a path to a savefile screenshot
            str << *rit;
            str << "/";
            str << m_standardDataDirs[index];

            if (dir == DIR_HELP)
            {
                str << "/";
                str << CApplication::GetInstancePointer()->GetLanguageChar();
            }
            str << "/";
        }

        str << subpath;

        boost::filesystem::path path(str.str());
        if(boost::filesystem::exists(path))
        {
            return str.str();
        }
    }
    
    if(m_dataDirs.size() > 0) {
        std::stringstream str;
        if ( subpath.find("save") == std::string::npos ){ // if its NOT a path to a savefile screenshot
            str << m_dataDirs[0];
            str << "/";
            str << m_standardDataDirs[index];
            
            if (dir == DIR_HELP)
            {
                str << "/";
                str << CApplication::GetInstancePointer()->GetLanguageChar();
            }
            str << "/";
        }
        str << subpath;
        return str.str();
    }
    
    return subpath;
}

std::string CGameData::GetDataPath(const std::string &subpath)
{
    for(std::vector<std::string>::reverse_iterator rit = m_dataDirs.rbegin(); rit != m_dataDirs.rend(); ++rit) {
        std::string path = *rit + "/" + subpath;
        boost::filesystem::path boostPath(path);
        if(boost::filesystem::exists(boostPath))
        {
            return path;
        }
    }
    return m_dataDirs[0] + "/" + subpath;
}
