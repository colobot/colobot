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


#include "common/resources/resourcemanager.h"

#include "common/config.h"

#if PLATFORM_WINDOWS
    #include "common/system/system_windows.h"
#endif

#include "common/logger.h"
#include "common/make_unique.h"

#include <physfs.h>

#include <boost/regex.hpp>


CResourceManager::CResourceManager(const char *argv0)
{
    if (!PHYSFS_init(argv0))
    {
        GetLogger()->Error("Error while initializing physfs: %s\n", PHYSFS_getLastError());
        assert(false);
    }
    PHYSFS_permitSymbolicLinks(1);
}


CResourceManager::~CResourceManager()
{
    if (PHYSFS_isInit())
    {
        if (!PHYSFS_deinit())
        {
            GetLogger()->Error("Error while deinitializing physfs: %s\n", PHYSFS_getLastError());
        }
    }
}

std::string CResourceManager::CleanPath(const std::string& path)
{
    return boost::regex_replace(path, boost::regex("(.*)/\\.\\./"), "");
}


bool CResourceManager::AddLocation(const std::string &location, bool prepend, const std::string &mountPoint)
{
    if (!PHYSFS_mount(location.c_str(), mountPoint.c_str(), prepend ? 0 : 1))
    {
        GetLogger()->Error("Error while mounting \"%s\": %s\n", location.c_str(), PHYSFS_getLastError());
        return false;
    }

    return true;
}


bool CResourceManager::RemoveLocation(const std::string &location)
{
    if (!PHYSFS_removeFromSearchPath(location.c_str()))
    {
        GetLogger()->Error("Error while unmounting \"%s\": %s\n", location.c_str(), PHYSFS_getLastError());
        return false;
    }

    return true;
}

std::vector<std::string> CResourceManager::GetLocations()
{
    std::vector<std::string> ret;
    char **list = PHYSFS_getSearchPath();
    for (char **it = list; *it != nullptr; ++it)
        ret.push_back(*it);
    PHYSFS_freeList(list);
    return ret;
}

bool CResourceManager::LocationExists(const std::string& location)
{
    auto locations = GetLocations();
    auto it = std::find(locations.cbegin(), locations.cend(), location);
    return it != locations.cend();
}

bool CResourceManager::SetSaveLocation(const std::string &location)
{
    if (!PHYSFS_setWriteDir(location.c_str()))
    {
        GetLogger()->Error("Error while setting save location to \"%s\": %s\n", location.c_str(), PHYSFS_getLastError());
        return false;
    }

    return true;
}

std::string CResourceManager::GetSaveLocation()
{
    if (PHYSFS_isInit())
    {
        return PHYSFS_getWriteDir();
    }
    return "";
}

std::unique_ptr<CSDLFileWrapper> CResourceManager::GetSDLFileHandler(const std::string &filename)
{
    return MakeUnique<CSDLFileWrapper>(CleanPath(filename));
}

std::unique_ptr<CSDLMemoryWrapper> CResourceManager::GetSDLMemoryHandler(const std::string &filename)
{
    return MakeUnique<CSDLMemoryWrapper>(CleanPath(filename));
}

std::unique_ptr<CSNDFileWrapper> CResourceManager::GetSNDFileHandler(const std::string &filename)
{
    return MakeUnique<CSNDFileWrapper>(CleanPath(filename));
}


bool CResourceManager::Exists(const std::string &filename)
{
    if (PHYSFS_isInit())
    {
        return PHYSFS_exists(CleanPath(filename).c_str());
    }
    return false;
}

bool CResourceManager::DirectoryExists(const std::string& directory)
{
    if (PHYSFS_isInit())
    {
        return PHYSFS_exists(CleanPath(directory).c_str()) && PHYSFS_isDirectory(CleanPath(directory).c_str());
    }
    return false;
}

bool CResourceManager::CreateDirectory(const std::string& directory)
{
    if (PHYSFS_isInit())
    {
        return PHYSFS_mkdir(CleanPath(directory).c_str());
    }
    return false;
}

bool CResourceManager::RemoveDirectory(const std::string& directory)
{
    if (PHYSFS_isInit())
    {
        std::string path = CleanPath(directory);
        for (auto file : ListFiles(path))
        {
            if (PHYSFS_delete((path + "/" + file).c_str()) == 0)
                return false;
        }
        return PHYSFS_delete(path.c_str()) != 0;
    }
    return false;
}

std::vector<std::string> CResourceManager::ListFiles(const std::string &directory, bool excludeDirs)
{
    std::vector<std::string> result;

    if (PHYSFS_isInit())
    {
        char **files = PHYSFS_enumerateFiles(CleanPath(directory).c_str());

        for (char **i = files; *i != nullptr; i++)
        {
            if (excludeDirs)
            {
                std::string path = CleanPath(directory) + "/" + (*i);
                if (PHYSFS_isDirectory(path.c_str())) continue;
            }
            result.push_back(*i);
        }

        PHYSFS_freeList(files);
    }

    return result;
}

std::vector<std::string> CResourceManager::ListDirectories(const std::string &directory)
{
    std::vector<std::string> result;

    if (PHYSFS_isInit())
    {
        char **files = PHYSFS_enumerateFiles(CleanPath(directory).c_str());

        for (char **i = files; *i != nullptr; i++)
        {
            std::string path = CleanPath(directory) + "/" + (*i);
            if (PHYSFS_isDirectory(path.c_str()))
            {
                result.push_back(*i);
            }
        }

        PHYSFS_freeList(files);
    }

    return result;
}

long long CResourceManager::GetFileSize(const std::string& filename)
{
    if (PHYSFS_isInit())
    {
        PHYSFS_File* file = PHYSFS_openRead(CleanPath(filename).c_str());
        if(file == nullptr) return -1;
        long long size = PHYSFS_fileLength(file);
        PHYSFS_close(file);
        return size;
    }
    return -1;
}

long long CResourceManager::GetLastModificationTime(const std::string& filename)
{
    if (PHYSFS_isInit())
    {
        return PHYSFS_getLastModTime(CleanPath(filename).c_str());
    }
    return -1;
}

bool CResourceManager::Remove(const std::string& filename)
{
    if (PHYSFS_isInit())
    {
        return PHYSFS_delete(filename.c_str()) != 0;
    }
    return false;
}
