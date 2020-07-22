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

#include "app/moddata.h"

#include <unordered_map>
#include <vector>

class CApplication;
class CPathManager;

struct Mod
{
    std::string name{};
    std::string path{};
    bool enabled = false;
    ModData data{};
};

/**
 * \class CModManager
 * \brief This class handles the list of mods.
 *
 * The order matters since the order in which files are loaded matters,
 * because some files can be overwritten.
 *
 * The changes in the list do not immediately apply
 * and will be lost after the \ref ReloadMods call if they were not
 * saved beforehand with \ref SaveMods.
 *
 * The changes are also lost to mods which are no longer found in the search paths.
 */
class CModManager
{
public:
    CModManager(CApplication* app, CPathManager* pathManager);

    //! Loads mods without resource reloading; should be called only once after creation
    void Init();

    //! Finds all the mods along with their metadata
    void ReloadMods();

    //! Removes a mod from the list of loaded mods
    void EnableMod(size_t i);

    //! Adds a mod to the list of loaded mods
    void DisableMod(size_t i);

    //! Moves the selected mod up in the list so that it's loaded sooner than others, returns the new index
    size_t MoveUp(size_t i);

    //! Moves the selected mod down in the list so that it's loaded later than others, returns the new index
    size_t MoveDown(size_t i);

    //! Saves the current configuration of mods to the config file
    void SaveMods();

    //! Number of mods loaded
    size_t CountMods() const;

    //! Returns the reference to the mod in given position
    const Mod& GetMod(size_t i) const;

    //! Returns the list of mods
    const std::vector<Mod>& GetMods() const;

private:
    //! Updates the paths in Path Manager according to the current mod configuration
    void UpdatePaths();

    //! Reloads application resources so the enabled mods are applied
    void ReloadResources();

private:
    CApplication* m_app;
    CPathManager* m_pathManager;

    std::vector<Mod> m_mods;
};
