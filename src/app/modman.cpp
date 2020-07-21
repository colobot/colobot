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

#include "app/modman.h"

#include "common/config.h"

#include "app/app.h"
#include "app/pathman.h"

#include "common/config_file.h"
#include "common/logger.h"

#include "common/resources/resourcemanager.h"

#include <algorithm>
#include <map>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

using namespace boost::filesystem;

CModManager::CModManager(CApplication* app, CPathManager* pathManager)
    : m_app{app},
      m_pathManager{pathManager}
{
}

void CModManager::FindMods()
{
    m_mods.clear();

    // Load names from the config file
    std::vector<std::string> savedModNames;
    GetConfigFile().GetArrayProperty("Mods", "Names", savedModNames);
    std::vector<bool> savedEnabled;
    GetConfigFile().GetArrayProperty("Mods", "Enabled", savedEnabled);

    // Transform the data into Mod structures
    m_mods.reserve(savedModNames.size());
    for (size_t i = 0; i < savedModNames.size(); ++i)
    {
        Mod mod{};
        mod.name = savedModNames[i];
        if (i < savedEnabled.size())
        {
            mod.enabled = savedEnabled[i];
        }
        mod.path = ""; // Find the path later
        m_mods.push_back(mod);
    }

    // Search the folders for mods
    const auto rawPaths = m_pathManager->FindMods();
    std::map<std::string, std::string> modPaths;
    for (const auto& path : rawPaths)
    {
        auto modName = boost::filesystem::path(path).stem().string();
        modPaths.insert(std::make_pair(modName, path));
    }

    // Find paths for already saved mods
    auto it = m_mods.begin();
    while (it != m_mods.end())
    {
        auto& mod = *it;
        const auto pathsIt = modPaths.find(mod.name);
        if (pathsIt != modPaths.end())
        {
            mod.path = (*pathsIt).second;
            modPaths.erase(pathsIt);
            ++it;
        }
        else
        {
            GetLogger()->Warn("Could not find mod %s, removing it from the list\n", mod.name.c_str());
            it = m_mods.erase(it);
        }
    }

    // Add the remaining found mods to the end of the list
    for (const auto& newMod : modPaths)
    {
        Mod mod{};
        mod.name = newMod.first;
        mod.path = newMod.second;
        m_mods.push_back(mod);
    }
}

void CModManager::EnableMod(size_t i)
{
    m_mods[i].enabled = true;
}

void CModManager::DisableMod(size_t i)
{
    m_mods[i].enabled = false;
}

size_t CModManager::MoveUp(size_t i)
{
    if (i != 0)
    {
        std::swap(m_mods[i - 1], m_mods[i]);
        return i - 1;
    }
    else
    {
        return i;
    }
}

size_t CModManager::MoveDown(size_t i)
{
    if (i != m_mods.size() - 1)
    {
        std::swap(m_mods[i], m_mods[i + 1]);
        return i + 1;
    }
    else
    {
        return i;
    }
}

void CModManager::UpdatePaths()
{
    m_pathManager->RemoveAllMods();
    for (const auto& mod : m_mods)
    {
        if (mod.enabled)
        {
            m_pathManager->AddMod(mod.path);
        }
    }
}

void CModManager::ReloadResources()
{
    m_app->ReloadResources();
}

void CModManager::SaveMods()
{
    std::vector<std::string> savedNames;
    savedNames.reserve(m_mods.size());
    std::transform(m_mods.begin(), m_mods.end(), std::back_inserter(savedNames), [](const Mod& mod) { return mod.name; });
    GetConfigFile().SetArrayProperty("Mods", "Names", savedNames);

    std::vector<bool> savedEnabled;
    savedEnabled.reserve(m_mods.size());
    std::transform(m_mods.begin(), m_mods.end(), std::back_inserter(savedEnabled), [](const Mod& mod) { return mod.enabled; });
    GetConfigFile().SetArrayProperty("Mods", "Enabled", savedEnabled);

    GetConfigFile().Save();
}

size_t CModManager::CountMods() const
{
    return m_mods.size();
}

const Mod& CModManager::GetMod(size_t i) const
{
    return m_mods[i];
}

const std::vector<Mod>& CModManager::GetMods() const
{
    return m_mods;
}
