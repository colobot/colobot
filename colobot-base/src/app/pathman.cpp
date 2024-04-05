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
#include "common/stringutils.h"

#include "common/resources/resourcemanager.h"

#include "common/system/system.h"

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

void CPathManager::SetDataPath(const std::filesystem::path& dataPath)
{
    m_dataPath = dataPath;
}

void CPathManager::SetLangPath(const std::filesystem::path& langPath)
{
    m_langPath = langPath;
}

void CPathManager::SetSavePath(const std::filesystem::path& savePath)
{
    m_savePath = savePath;
}

const std::filesystem::path& CPathManager::GetDataPath() const
{
    return m_dataPath;
}

const std::filesystem::path& CPathManager::GetLangPath() const
{
    return m_langPath;
}

const std::filesystem::path& CPathManager::GetSavePath() const
{
    return m_savePath;
}

std::string CPathManager::VerifyPaths() const
{
    if (! (std::filesystem::exists(m_dataPath) && std::filesystem::is_directory(m_dataPath)) )
    {
        GetLogger()->Error("Data directory '%%' doesn't exist or is not a directory", m_dataPath);
        return std::string("Could not read from data directory:\n") +
            std::string("'") + StrUtils::ToString(m_dataPath) + std::string("'\n") +
            std::string("Please check your installation, or supply a valid data directory by -datadir option.");
    }

    if (! (std::filesystem::exists(m_langPath) && std::filesystem::is_directory(m_langPath)) )
    {
        GetLogger()->Warn("Language path '%%' is invalid, assuming translation files not installed", m_langPath);
    }

    std::filesystem::create_directories(m_savePath);
    std::filesystem::create_directories(m_savePath / "mods");

    return "";
}

void CPathManager::InitPaths()
{
    GetLogger()->Info("Data path: %%", m_dataPath);
    GetLogger()->Info("Save path: %%", m_savePath);

    m_modSearchDirs.push_back(m_dataPath / "mods");
    m_modSearchDirs.push_back(m_savePath / "mods");

    if (!m_modSearchDirs.empty())
    {
        GetLogger()->Info("Mod search dirs:");
        for(const auto& modSearchDir : m_modSearchDirs)
            GetLogger()->Info("  * %%", modSearchDir);
    }

    CResourceManager::AddLocation(m_dataPath);
    CResourceManager::SetSaveLocation(m_savePath);
    CResourceManager::AddLocation(m_savePath);

    GetLogger()->Debug("Finished initializing data paths");
    GetLogger()->Debug("PHYSFS search path is:");
    for (const auto& path : CResourceManager::GetLocations())
        GetLogger()->Debug("  * %%", path);
}

void CPathManager::AddMod(const std::filesystem::path& path)
{
    m_mods.push_back(path);
}

std::vector<std::filesystem::path> CPathManager::FindMods() const
{
    std::vector<std::filesystem::path> mods;
    GetLogger()->Info("Found mods:");
    for (const auto &searchPath : m_modSearchDirs)
    {
        for (const auto &modPath : FindModsInDir(searchPath))
        {
            GetLogger()->Info("  * %%", modPath);
            mods.push_back(modPath);
        }
    }
    GetLogger()->Info("Additional mod paths:");

    for (const auto& modPath : m_mods)
    {
        if (std::filesystem::exists(modPath))
        {
            GetLogger()->Info("  * %%", modPath);
            mods.push_back(modPath);
        }
        else
        {
            GetLogger()->Warn("Mod does not exist: %%", modPath);
        }
    }
    return mods;
}

void CPathManager::AddModSearchDir(const std::filesystem::path& modSearchDirPath)
{
    m_modSearchDirs.push_back(modSearchDirPath);
}

std::vector<std::filesystem::path> CPathManager::FindModsInDir(const std::filesystem::path& dir) const
{
    std::vector<std::filesystem::path> ret;
    try
    {
        std::filesystem::directory_iterator iterator(dir);

        for(; iterator != std::filesystem::directory_iterator(); ++iterator)
        {
            ret.push_back(iterator->path());
        }
    }
    catch (std::exception &e)
    {
        GetLogger()->Warn("Unable to load mods from directory '%%': %%", dir, e.what());
    }
    return ret;
}
