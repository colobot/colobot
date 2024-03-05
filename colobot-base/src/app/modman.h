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

#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

class CApplication;
class CPathManager;

struct ModData
{
    std::string displayName{};
    std::string author{};
    std::string version{};
    std::string website{};
    std::string summary{};
    std::vector<std::string> changes{};
};

struct Mod
{
    std::string name{};
    std::filesystem::path path{};
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
 * The changes in the list do not immediately apply.
 */
class CModManager
{
public:
    CModManager(CApplication* app, CPathManager* pathManager);

    //! Finds all the mods along with their metadata
    void FindMods();

    //! Applies the current configuration and reloads the application
    void ReloadMods();

    //! Removes a mod from the list of loaded mods
    void EnableMod(size_t i);

    //! Adds a mod to the list of loaded mods
    void DisableMod(size_t i);

    //! Moves the selected mod up in the list so that it's loaded sooner than others, returns the new index
    size_t MoveUp(size_t i);

    //! Moves the selected mod down in the list so that it's loaded later than others, returns the new index
    size_t MoveDown(size_t i);

    //! Checks if the list of currently used mods differs from the current configuration or there were changes made by the user
    bool Changes();

    //! Saves the current configuration of mods to the config file
    void SaveMods();

    //! Number of mods loaded
    size_t CountMods() const;

    //! Returns the reference to the mod in given position
    const Mod& GetMod(size_t i) const;

    //! Returns the list of mods
    const std::vector<Mod>& GetMods() const;

private:
    // Allow access to MountAllMods() as CApplication doesn't want to reload itself during initialization
    friend CApplication;

    //! Reloads application resources so the enabled mods are applied
    void ReloadResources();

    //! Load mod data into mod
    void LoadModData(Mod& mod);

    //! Updates the paths in Path Manager according to the current mod configuration
    void MountAllMods();

    void MountMod(const Mod& mod, const std::string& mountPoint = "");
    void MountMod(const std::filesystem::path& path, const std::string& mountPoint = "");
    void UnmountMod(const Mod& mod);
    void UnmountMod(const std::filesystem::path& path);
    void UnmountAllMountedMods();

private:
    CApplication* m_app;
    CPathManager* m_pathManager;

    //! Paths to mods already in the virtual filesystem
    std::vector<std::filesystem::path> m_mountedModPaths;

    //! List of mods
    std::vector<Mod> m_mods;

    bool m_userChanges = false;
};
