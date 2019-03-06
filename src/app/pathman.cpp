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
    : m_dataPath(systemUtils->GetDataPath())
    , m_langPath(systemUtils->GetLangPath())
    , m_savePath(systemUtils->GetSaveDir())
    , m_modAutoloadDir{ m_dataPath + "/mods", m_savePath + "/mods" }
    , m_mods{}
{
}

CPathManager::~CPathManager()
{
}

void CPathManager::SetDataPath(const std::string &dataPath)
{
    m_dataPath = dataPath;
}

void CPathManager::SetLangPath(const std::string &langPath)
{
    m_langPath = langPath;
}

void CPathManager::SetSavePath(const std::string &savePath)
{
    m_savePath = savePath;
}

void CPathManager::AddModAutoloadDir(const std::string &modAutoloadDirPath)
{
    m_modAutoloadDir.push_back(modAutoloadDirPath);
}

void CPathManager::AddMod(const std::string &modPath)
{
    m_mods.push_back(modPath);
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
    GetLogger()->Info("Data path: %s\n", m_dataPath.c_str());
    GetLogger()->Info("Save path: %s\n", m_savePath.c_str());
    if (!m_modAutoloadDir.empty())
    {
        GetLogger()->Info("Mod autoload dirs:\n");
        for(const std::string& modAutoloadDir : m_modAutoloadDir)
            GetLogger()->Info("  * %s\n", modAutoloadDir.c_str());
    }
    if (!m_mods.empty())
    {
        GetLogger()->Info("Mods:\n");
        for(const std::string& modPath : m_mods)
            GetLogger()->Info("  * %s\n", modPath.c_str());
    }

    CResourceManager::AddLocation(m_dataPath);

    for (const std::string& modAutoloadDir : m_modAutoloadDir)
    {
        GetLogger()->Trace("Searching for mods in '%s'...\n", modAutoloadDir.c_str());
        for (const std::string& modPath : FindModsInDir(modAutoloadDir))
        {
            GetLogger()->Info("Autoloading mod: '%s'\n", modPath.c_str());
            CResourceManager::AddLocation(modPath);
        }
    }

    for (const std::string& modPath : m_mods)
    {
        GetLogger()->Info("Loading mod: '%s'\n", modPath.c_str());
        CResourceManager::AddLocation(modPath);
    }

    CResourceManager::SetSaveLocation(m_savePath);
    CResourceManager::AddLocation(m_savePath);

    GetLogger()->Debug("Finished initalizing data paths\n");
    GetLogger()->Debug("PHYSFS search path is:\n");
    for (const std::string& path : CResourceManager::GetLocations())
        GetLogger()->Debug("  * %s\n", path.c_str());
}

std::vector<std::string> CPathManager::FindModsInDir(const std::string &dir)
{
    std::vector<std::string> ret;
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
            ret.push_back(CSystemUtilsWindows::UTF8_Encode(iterator->path().wstring()));
            #else
            ret.push_back(iterator->path().string());
            #endif
        }
    }
    catch (std::exception &e)
    {
        GetLogger()->Warn("Unable to load mods from directory '%s': %s\n", dir.c_str(), e.what());
    }
    return ret;
}
