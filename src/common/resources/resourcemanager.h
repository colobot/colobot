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

#pragma once

#include "common/resources/sdl_file_wrapper.h"
#include "common/resources/sdl_memory_wrapper.h"
#include "common/resources/sndfile_wrapper.h"

#include <memory>
#include <string>
#include <vector>

class CResourceManager
{
public:
    CResourceManager(const char *argv0);
    ~CResourceManager();

    static std::string CleanPath(const std::string &path);

    static bool AddLocation(const std::string &location, bool prepend = true);
    static bool RemoveLocation(const std::string &location);

    static bool SetSaveLocation(const std::string &location);
    static std::string GetSaveLocation();

    static std::unique_ptr<CSDLFileWrapper> GetSDLFileHandler(const std::string &filename);
    static std::unique_ptr<CSDLMemoryWrapper> GetSDLMemoryHandler(const std::string &filename);
    static std::unique_ptr<CSNDFileWrapper> GetSNDFileHandler(const std::string &filename);

    //! Check if file exists
    static bool Exists(const std::string &filename);
    //! Check if file exists and is a directory
    static bool DirectoryExists(const std::string& directory);

    //! Create directory in write directory
    static bool CreateDirectory(const std::string& directory);
    //! Remove directory in write directory, recursively
    static bool RemoveDirectory(const std::string& directory);

    //! List files contained in directory
    static std::vector<std::string> ListFiles(const std::string &directory);
    //! List directories contained in directory
    static std::vector<std::string> ListDirectories(const std::string &directory);


    //! Returns file size in bytes
    static long long GetFileSize(const std::string &filename);
    //! Returns last modification date as timestamp
    static long long GetLastModificationTime(const std::string &filename);

    //! Remove file
    static bool Remove(const std::string& filename);
};
