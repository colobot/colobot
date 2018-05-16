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


#include "app/pathman.h"

#include "common/config.h"

#include "app/app.h"


#include "common/logger.h"

#include "common/resources/resourcemanager.h"

#include "common/system/system.h"
#ifdef PLATFORM_WINDOWS
    #include "common/system/system_windows.h"
#endif

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

CPathManager::CPathManager(CSystemUtils* systemUtils)
    : m_systemUtils(systemUtils)
{
    #ifdef PORTABLE
        m_dataPath = "./data";
        m_langPath = "./lang";
        m_savePath = "./saves";
    #else
        m_dataPath = m_systemUtils->GetDataPath();
        m_langPath = m_systemUtils->GetLangPath();
        #ifdef DEV_BUILD
            m_savePath = "./saves";
        #else
            m_savePath = m_systemUtils->GetSaveDir();
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
    GetLogger()->Info("Loading mod: '%s'\n", modPath.c_str());
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
    #if PLATFORM_WINDOWS
    boost::filesystem::path dataPath(CSystemUtilsWindows::UTF8_Decode(m_dataPath));
    #else
    boost::filesystem::path dataPath(m_dataPath);
    #endif
    if (! (boost::filesystem::exists(dataPath) && boost::filesystem::is_directory(dataPath)) )
    {
        GetLogger()->Error("Data directory '%s' doesn't exist or is not a directory\n", m_dataPath.c_str());
        return std::string("Could not read from data directory:\n") +
        std::string("'") + m_dataPath + std::string("'\n") +
        std::string("Please check your installation, or supply a valid data directory by -datadir option.");
    }

    #if PLATFORM_WINDOWS
    boost::filesystem::path langPath(CSystemUtilsWindows::UTF8_Decode(m_langPath));
    #else
    boost::filesystem::path langPath(m_langPath);
    #endif
    if (! (boost::filesystem::exists(langPath) && boost::filesystem::is_directory(langPath)) )
    {
        GetLogger()->Warn("Language path '%s' is invalid, assuming translation files not installed\n", m_langPath.c_str());
    }

    #if PLATFORM_WINDOWS
    boost::filesystem::create_directories(CSystemUtilsWindows::UTF8_Decode(m_savePath));
    boost::filesystem::create_directories(CSystemUtilsWindows::UTF8_Decode(m_savePath+"/mods"));
    #else
    boost::filesystem::create_directories(m_savePath);
    boost::filesystem::create_directories(m_savePath+"/mods");
    #endif

    return "";
}

void CPathManager::InitPaths()
{
    LoadModsFromDir(m_dataPath+"/mods");
    LoadModsFromDir(m_savePath+"/mods");

    GetLogger()->Info("Data path: %s\n", m_dataPath.c_str());
    GetLogger()->Info("Save path: %s\n", m_savePath.c_str());
    CResourceManager::AddLocation(m_dataPath, false);
    CResourceManager::SetSaveLocation(m_savePath);
    CResourceManager::AddLocation(m_savePath, true);
}

void CPathManager::LoadModsFromDir(const std::string &dir)
{
    GetLogger()->Trace("Looking for mods in '%s' ...\n", dir.c_str());
    try
    {
        #if PLATFORM_WINDOWS
        boost::filesystem::directory_iterator iterator(CSystemUtilsWindows::UTF8_Decode(dir));
        #else
        boost::filesystem::directory_iterator iterator(dir);
        #endif
        for(; iterator != boost::filesystem::directory_iterator(); ++iterator)
        {
            #if PLATFORM_WINDOWS
            AddMod(CSystemUtilsWindows::UTF8_Encode(iterator->path().wstring()));
            #else
            AddMod(iterator->path().string());
            #endif
        }
    }
    catch (std::exception &e)
    {
        GetLogger()->Warn("Unable to load mods from directory '%s': %s\n", dir.c_str(), e.what());
    }
}
