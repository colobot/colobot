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
 * \class CModManager
 * \brief This class handles the list of mods.
 *
 * The order matters since the order in which files are loaded matters,
 * because some files can be overwritten.
 *
 * The list can be kept in the config file with the \ref SaveMods function.
 *
 * The changes in the list do not immediately apply.
 * Separate calls to \ref UpdatePaths and \ref ReloadResources, probably in this order,
 * need to be done for the changes to apply.
 *
 */
class CModManager
{
public:
    CModManager(CApplication* app, CPathManager* pathManager);

    //! Finds all the mods along with their metadata
    void FindMods();

    //! Removes a mod from the list of loaded mods
    void EnableMod(size_t i);

    //! Adds a mod to the list of loaded mods
    void DisableMod(size_t i);

    //! Moves the selected mod up in the list so that it's loaded sooner than others, returns the new index
    size_t MoveUp(size_t i);

    //! Moves the selected mod down in the list so that it's loaded later than others, returns the new index
    size_t MoveDown(size_t i);

    //! Reloads application resources so the enabled mods are applied
    void ReloadResources();

    //! Saves the current configuration of mods to the config file
    void SaveMods();

    //! Updates the paths in Path Manager according to the current mod configuration
    void UpdatePaths();

    //! Number of mods loaded
    size_t CountMods() const;

    //! Returns the reference to the mod in given position
    const Mod& GetMod(size_t i) const;

    //! Returns the list of mods
    const std::vector<Mod>& GetMods() const;

private:
    CApplication* m_app;
    CPathManager* m_pathManager;

    //TODO: better data structure?
    std::vector<Mod> m_mods;
};
