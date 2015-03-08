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


#include "common/pathman.h"


#include "app/app.h"
#include "app/system.h"

#include "common/config.h"
#include "common/logger.h"
#include "common/resources/resourcemanager.h"

#include "object/robotmain.h"

#include "object/level/parser.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

template<> CPathManager* CSingleton<CPathManager>::m_instance = nullptr;

CPathManager::CPathManager()
{
    #ifdef PORTABLE
        m_dataPath = "./data";
        m_langPath = "./lang";
        m_savePath = "./saves";
    #else
        m_dataPath = GetSystemUtils()->GetDataPath();
        m_langPath = GetSystemUtils()->GetLangPath();
        #ifdef DEV_BUILD
            m_savePath = "./saves";
        #else
            m_savePath = GetSystemUtils()->GetSaveDir();
        #endif
    #endif
}

CPathManager::~CPathManager()
{
}

void CPathManager::SetDataPath(std::string dataPath)
{
    m_dataPath = dataPath;
}

void CPathManager::SetLangPath(std::string langPath)
{
    m_langPath = langPath;
}

void CPathManager::SetSavePath(std::string savePath)
{
    m_savePath = savePath;
}

void CPathManager::AddMod(std::string modPath)
{
    CLogger::GetInstancePointer()->Info("Loading mod: '%s'\n", modPath.c_str());
    CResourceManager::AddLocation(modPath, true);
}

const std::string& CPathManager::GetDataPath()
{
    return m_dataPath;
}

const std::string& CPathManager::GetLangPath()
{
    return m_langPath;
}

const std::string& CPathManager::GetSavePath()
{
    return m_savePath;
}

std::string CPathManager::VerifyPaths()
{
    boost::filesystem::path dataPath(m_dataPath);
    if (! (boost::filesystem::exists(dataPath) && boost::filesystem::is_directory(dataPath)) )
    {
        CLogger::GetInstancePointer()->Error("Data directory '%s' doesn't exist or is not a directory\n", m_dataPath.c_str());
        return std::string("Could not read from data directory:\n") +
        std::string("'") + m_dataPath + std::string("'\n") +
        std::string("Please check your installation, or supply a valid data directory by -datadir option.");
    }

    boost::filesystem::path langPath(m_langPath);
    if (! (boost::filesystem::exists(langPath) && boost::filesystem::is_directory(langPath)) )
    {
        CLogger::GetInstancePointer()->Warn("Language path '%s' is invalid, assuming translation files not installed\n", m_langPath.c_str());
    }

    boost::filesystem::create_directories(m_savePath);
    boost::filesystem::create_directories(m_savePath+"/mods");

    return "";
}

void CPathManager::InitPaths()
{
    LoadModsFromDir(m_dataPath+"/mods");
    LoadModsFromDir(m_savePath+"/mods");

    CLogger::GetInstancePointer()->Info("Data path: %s\n", m_dataPath.c_str());
    CLogger::GetInstancePointer()->Info("Save path: %s\n", m_savePath.c_str());
    CResourceManager::AddLocation(m_dataPath, false);
    CResourceManager::SetSaveLocation(m_savePath);
    CResourceManager::AddLocation(m_savePath, true);
}

void CPathManager::LoadModsFromDir(const std::string &dir)
{
    try {
        boost::filesystem::directory_iterator iterator(dir);
        for(; iterator != boost::filesystem::directory_iterator(); ++iterator)
        {
            AddMod(iterator->path().string());
        }
    }
    catch(std::exception &e)
    {
        CLogger::GetInstancePointer()->Warn("Unable to load mods from directory '%s': %s\n", dir.c_str(), e.what());
    }
}

std::string CPathManager::InjectLevelDir(std::string path, const std::string& defaultDir)
{
    std::string newPath = path;
    std::string lvlDir = CLevelParser::BuildSceneName(CRobotMain::GetInstancePointer()->GetSceneName(), CRobotMain::GetInstancePointer()->GetSceneRank()/100, CRobotMain::GetInstancePointer()->GetSceneRank()%100, false);
    boost::replace_all(newPath, "%lvl%", lvlDir);
    std::string chapDir = CLevelParser::BuildSceneName(CRobotMain::GetInstancePointer()->GetSceneName(), CRobotMain::GetInstancePointer()->GetSceneRank()/100, 0, false);
    boost::replace_all(newPath, "%chap%", chapDir);
    if(newPath == path && !path.empty())
    {
        newPath = defaultDir + (!defaultDir.empty() ? "/" : "") + newPath;
    }

    //TODO: %cat%
    
    std::string langPath = newPath;
    std::string langStr(1, CApplication::GetInstancePointer()->GetLanguageChar());
    boost::replace_all(langPath, "%lng%", langStr);
    if(CResourceManager::Exists(langPath))
        return langPath;
    
    // Fallback to English if file doesn't exist
    boost::replace_all(newPath, "%lng%", "E");
    if(CResourceManager::Exists(newPath))
        return newPath;
    
    return langPath; // Return current language file if none of the files exist
}