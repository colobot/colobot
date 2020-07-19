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

#pragma once

#include "ui/maindialog.h"

#include "ui/screen/screen_setup.h"

#include <unordered_map>
#include <boost/optional.hpp>

class CPathManager;

struct Mod
{
    std::string name{};
    std::string path{};
    bool enabled = false;
    //TODO: add metadata for UI
};

/**
 * \class CApplication
 * \brief Main application
 *
 * This class handles the list of currently loaded mods.
 * The order matters since the order in which files are loaded matters.
 *
 */
class CModManager
{
public:
    CModManager(CApplication* app, CPathManager* pathManager);

    //! Finds all the mods along with their metadata
    void FindMods();

    //! Removes a mod from the list of loaded mods
    void EnableMod(const std::string& modName);

    //! Adds a mod to the list of loaded mods
    void DisableMod(const std::string& modName);

    //! Reloads application resources so the enabled mods are applied
    void ReloadResources();

    //! Saves the current configuration of mods to the config file
    void SaveMods();

    //! Updates the paths in Path Manager according to the current mod configuration
    void UpdatePaths();

    boost::optional<Mod> GetMod(const std::string& modName);
    const std::vector<Mod>& GetMods() const;

private:
    Mod* FindMod(const std::string& modName);

private:
    CApplication* m_app;
    CPathManager* m_pathManager;

    //TODO: better data structure?
    std::vector<Mod> m_mods;
};
