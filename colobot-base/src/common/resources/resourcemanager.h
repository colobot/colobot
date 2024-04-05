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

#include "common/resources/sdl_file_wrapper.h"
#include "common/resources/sdl_memory_wrapper.h"
#include "common/resources/sndfile_wrapper.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

class CResourceManager
{
public:
    CResourceManager(const char *argv0);
    ~CResourceManager();

    static std::string CleanPath(const std::filesystem::path& path);

    //! Add a location to the search path
    static bool AddLocation(const std::filesystem::path& location, bool prepend = true, const std::string &mountPoint = "");
    //! Remove a location from the search path
    static bool RemoveLocation(const std::filesystem::path& location);
    //! List all locations in the search path
    static std::vector<std::filesystem::path> GetLocations();
    //! Check if given location is in the search path
    static bool LocationExists(const std::filesystem::path& location);

    static bool SetSaveLocation(const std::filesystem::path& location);
    static std::filesystem::path GetSaveLocation();

    static std::unique_ptr<CSDLFileWrapper> GetSDLFileHandler(const std::filesystem::path& filename);
    static std::unique_ptr<CSDLMemoryWrapper> GetSDLMemoryHandler(const std::filesystem::path& filename);
    static std::unique_ptr<CSNDFileWrapper> GetSNDFileHandler(const std::filesystem::path& filename);

    //! Check if file exists
    static bool Exists(const std::filesystem::path& filename);
    //! Check if file exists and is a directory
    static bool DirectoryExists(const std::filesystem::path& directory);

    //! Create directory in write directory
    static bool CreateNewDirectory(const std::filesystem::path& directory);
    //! Remove directory in write directory, recursively
    static bool RemoveExistingDirectory(const std::filesystem::path& directory);

    //! List files contained in directory
    static std::vector<std::filesystem::path> ListFiles(const std::filesystem::path& directory, bool excludeDirs = false);
    //! List directories contained in directory
    static std::vector<std::filesystem::path> ListDirectories(const std::filesystem::path& directory);


    //! Returns file size in bytes
    static long long GetFileSize(const std::filesystem::path& filename);
    //! Returns last modification date as timestamp
    static long long GetLastModificationTime(const std::filesystem::path& filename);

    //! Remove file
    static bool Remove(const std::filesystem::path& filename);
};
