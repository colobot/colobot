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


#include "app/app.h"
#include "app/pathman.h"

#include "common/config.h"
#include "common/logger.h"

#include "common/resources/resourcemanager.h"

#include "common/system/system.h"

#ifdef PLATFORM_WINDOWS
    #include "common/system/system_windows.h"
#endif

#include <boost/algorithm/string.hpp>

#include <filesystem>

CPathManager::CPathManager(CSystemUtils* systemUtils)
    : m_dataPath(systemUtils->GetDataPath())
    , m_langPath(systemUtils->GetLangPath())
    , m_savePath(systemUtils->GetSaveDir())
    , m_modSearchDirs{}
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
    std::filesystem::path dataPath = std::filesystem::u8path(m_dataPath);

    if (! (std::filesystem::exists(dataPath) && std::filesystem::is_directory(dataPath)) )
    {
        GetLogger()->Error("Data directory '%s' doesn't exist or is not a directory\n", m_dataPath.c_str());
        return std::string("Could not read from data directory:\n") +
            std::string("'") + m_dataPath + std::string("'\n") +
            std::string("Please check your installation, or supply a valid data directory by -datadir option.");
    }

    std::filesystem::path langPath = std::filesystem::u8path(m_langPath);

    if (! (std::filesystem::exists(langPath) && std::filesystem::is_directory(langPath)) )
    {
        GetLogger()->Warn("Language path '%s' is invalid, assuming translation files not installed\n", m_langPath.c_str());
    }

    std::filesystem::create_directories(std::filesystem::u8path(m_savePath));
    std::filesystem::create_directories(std::filesystem::u8path(m_savePath + "/mods"));

    return "";
}

void CPathManager::InitPaths()
{
    GetLogger()->Info("Data path: %s\n", m_dataPath.c_str());
    GetLogger()->Info("Save path: %s\n", m_savePath.c_str());

    m_modSearchDirs.push_back(m_dataPath + "/mods");
    m_modSearchDirs.push_back(m_savePath + "/mods");

    if (!m_modSearchDirs.empty())
    {
        GetLogger()->Info("Mod search dirs:\n");
        for(const std::string& modSearchDir : m_modSearchDirs)
            GetLogger()->Info("  * %s\n", modSearchDir.c_str());
    }

    CResourceManager::AddLocation(m_dataPath);

    CResourceManager::SetSaveLocation(m_savePath);
    CResourceManager::AddLocation(m_savePath);

    GetLogger()->Debug("Finished initalizing data paths\n");
    GetLogger()->Debug("PHYSFS search path is:\n");
    for (const std::string& path : CResourceManager::GetLocations())
        GetLogger()->Debug("  * %s\n", path.c_str());
}

void CPathManager::AddMod(const std::string &path)
{
    m_mods.push_back(path);
}

std::vector<std::string> CPathManager::FindMods() const
{
    std::vector<std::string> mods;
    GetLogger()->Info("Found mods:\n");
    for (const auto &searchPath : m_modSearchDirs)
    {
        for (const auto &modPath : FindModsInDir(searchPath))
        {
            GetLogger()->Info("  * %s\n", modPath.c_str());
            mods.push_back(modPath);
        }
    }
    GetLogger()->Info("Additional mod paths:\n");

    for (const auto& modPath : m_mods)
    {
        if (std::filesystem::exists(modPath))
        {
            GetLogger()->Info("  * %s\n", modPath.c_str());
            mods.push_back(modPath);
        }
        else
        {
            GetLogger()->Warn("Mod does not exist: %s\n", modPath.c_str());
        }
    }
    return mods;
}

void CPathManager::AddModSearchDir(const std::string &modSearchDirPath)
{
    m_modSearchDirs.push_back(modSearchDirPath);
}

std::vector<std::string> CPathManager::FindModsInDir(const std::string &dir) const
{
    std::vector<std::string> ret;
    try
    {
        std::filesystem::directory_iterator iterator(std::filesystem::u8path(dir));

        for(; iterator != std::filesystem::directory_iterator(); ++iterator)
        {
            ret.push_back(iterator->path().u8string());
        }
    }
    catch (std::exception &e)
    {
        GetLogger()->Warn("Unable to load mods from directory '%s': %s\n", dir.c_str(), e.what());
    }
    return ret;
}
