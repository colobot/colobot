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

#include "modman.h"

//TODO: clean up includes
#include "common/config.h"

#include "app/app.h"
#include "app/pathman.h"

#include "common/restext.h"
#include "common/logger.h"
#include "common/settings.h"
#include "common/stringutils.h"

#include "common/resources/resourcemanager.h"

#include "common/system/system.h"

#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

using namespace boost::filesystem;

CModManager::CModManager(CApplication* app, CPathManager* pathManager)
    : m_app{app},
      m_pathManager{pathManager}
{
}

void CModManager::ReinitMods()
{
    m_mods.clear();
    const auto foundMods = m_pathManager->FindMods();
    for (const auto& modPath : foundMods)
    {
        Mod mod;
        mod.name = boost::filesystem::path(modPath).stem().string();
        mod.path = modPath;
        mod.enabled = m_pathManager->ModLoaded(mod.path); //TODO: load from some config file
        m_mods.push_back(mod);
    }
}

void CModManager::EnableMod(const std::string& modName)
{
    Mod* mod = FindMod(modName);
    if (!mod)
    {
        GetLogger()->Error("Could not enable mod: %s not found\n", modName.c_str());
        return;
    }
    mod->enabled = true;
}

void CModManager::DisableMod(const std::string& modName)
{
    Mod* mod = FindMod(modName);
    if (!mod)
    {
        GetLogger()->Error("Could not disable mod: %s not found\n", modName.c_str());
        return;
    }
    mod->enabled = false;
}

void CModManager::ReloadMods()
{
    for (const auto& mod : m_mods)
    {
        bool loaded = m_pathManager->ModLoaded(mod.path);
        if (mod.enabled && !loaded)
        {
            m_pathManager->AddMod(mod.path);
        }
        else if (!mod.enabled && loaded)
        {
            m_pathManager->RemoveMod(mod.path);
        }
    }
    m_app->ReloadResources();
}

boost::optional<Mod> CModManager::GetMod(const std::string& modName)
{
    Mod* mod = FindMod(modName);
    return mod != nullptr ? *mod : boost::optional<Mod>();
}

const std::vector<Mod>& CModManager::GetMods() const
{
    return m_mods;
}

Mod* CModManager::FindMod(const std::string& modName)
{
    auto it = std::find_if(m_mods.begin(), m_mods.end(), [&](Mod& mod) { return mod.name == modName; });
    return it != m_mods.end() ? &(*it) : nullptr;
}
