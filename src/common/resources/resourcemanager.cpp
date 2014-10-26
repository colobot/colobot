/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2014, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsiteс.ch; http://colobot.info; http://github.com/colobot
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
#include "common/logger.h"

#include <physfs.h>

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

namespace fs = boost::filesystem;

namespace
{
    const Uint32 PHYSFS_RWOPS_TYPE = 0xc010b04f;
}


CResourceManager::CResourceManager(const char *argv0)
{
    if (!PHYSFS_init(argv0))
    {
        CLogger::GetInstancePointer()->Error("Error while initializing physfs\n");
    }
}


CResourceManager::~CResourceManager()
{
    if (PHYSFS_isInit())
    {
        if (!PHYSFS_deinit())
        {
            CLogger::GetInstancePointer()->Error("Error while deinitializing physfs\n");
        }
    }
}

std::string CResourceManager::CleanPath(const std::string& path)
{
    return boost::regex_replace(path, boost::regex("(.*)/\\.\\./"), "");
}


bool CResourceManager::AddLocation(const std::string &location, bool prepend)
{
    if (PHYSFS_isInit())
    {
        if (!PHYSFS_mount(location.c_str(), nullptr, prepend ? 0 : 1))
        {
            CLogger::GetInstancePointer()->Error("Error while mounting \"%s\"\n", location.c_str());
        }
    }

    return false;
}


bool CResourceManager::RemoveLocation(const std::string &location)
{
    if (PHYSFS_isInit())
    {
        if (!PHYSFS_removeFromSearchPath(location.c_str()))
        {
            CLogger::GetInstancePointer()->Error("Error while unmounting \"%s\"\n", location.c_str());
        }
    }

    return false;
}


bool CResourceManager::SetSaveLocation(const std::string &location)
{
    if (PHYSFS_isInit())
    {
        if (!PHYSFS_setWriteDir(location.c_str()))
        {
            CLogger::GetInstancePointer()->Error("Error while setting save location to \"%s\"\n", location.c_str());
        }
    }

    return false;
}

std::string CResourceManager::GetSaveLocation()
{
    if(PHYSFS_isInit()) {
        return PHYSFS_getWriteDir();
    }
    return "";
}


SDL_RWops* CResourceManager::GetSDLFileHandler(const std::string &filename)
{
    SDL_RWops *handler = SDL_AllocRW();
    if (!handler)
    {
        CLogger::GetInstancePointer()->Error("Unable to allocate SDL_RWops for \"%s\"\n", filename.c_str());
        return nullptr;
    }

    if (!PHYSFS_isInit())
    {
        SDL_FreeRW(handler);
        return nullptr;
    }

    PHYSFS_File *file = PHYSFS_openRead(CleanPath(filename).c_str());
    if (!file)
    {
        SDL_FreeRW(handler);
        return nullptr;
    }

    handler->seek = SDLSeek;
    handler->read = SDLRead;
    handler->write = SDLWrite;
    handler->close = SDLClose;
    handler->type = PHYSFS_RWOPS_TYPE;
    handler->hidden.unknown.data1 = file;

    return handler;
}


CSNDFile* CResourceManager::GetSNDFileHandler(const std::string &filename)
{
    return new CSNDFile(CleanPath(filename));
}


bool CResourceManager::Exists(const std::string &filename)
{
    return PHYSFS_exists(CleanPath(filename).c_str());
}

bool CResourceManager::DirectoryExists(const std::string& directory)
{
    return PHYSFS_exists(CleanPath(directory).c_str()) && PHYSFS_isDirectory(CleanPath(directory).c_str());
}

bool CResourceManager::CreateDirectory(const std::string& directory)
{
    return PHYSFS_mkdir(CleanPath(directory).c_str());
}

//TODO: Don't use boost filesystem here
bool CResourceManager::RemoveDirectory(const std::string& directory)
{
    bool success = true;
    std::string writeDir = PHYSFS_getWriteDir();
    try
    {
        fs::remove_all(writeDir + "/" + CleanPath(directory));
    }
    catch (std::exception & e)
    {
        success = false;
    }
    return success;
}

std::vector<std::string> CResourceManager::ListFiles(const std::string &directory)
{
    std::vector<std::string> result;

    char **files = PHYSFS_enumerateFiles(CleanPath(directory).c_str());

    for (char **i = files; *i != nullptr; i++)
    {
        result.push_back(*i);
    }

    PHYSFS_freeList(files);

    return result;
}

std::vector<std::string> CResourceManager::ListDirectories(const std::string &directory)
{
    std::vector<std::string> result;

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

    return result;
}


int CResourceManager::SDLClose(SDL_RWops *context)
{
    if (CheckSDLContext(context))
    {
        PHYSFS_close(static_cast<PHYSFS_File *>(context->hidden.unknown.data1));
        SDL_FreeRW(context);

        return 0;
    }

    return 1;
}


int CResourceManager::SDLRead(SDL_RWops *context, void *ptr, int size, int maxnum)
{
    if (CheckSDLContext(context))
    {
        PHYSFS_File *file = static_cast<PHYSFS_File *>(context->hidden.unknown.data1);
        SDL_memset(ptr, 0, size * maxnum);

        return PHYSFS_read(file, ptr, size, maxnum);
    }

    return 0;
}


int CResourceManager::SDLWrite(SDL_RWops *context, const void *ptr, int size, int num)
{
    return 0;
}


int CResourceManager::SDLSeek(SDL_RWops *context, int offset, int whence)
{
    if (CheckSDLContext(context))
    {
        PHYSFS_File *file = static_cast<PHYSFS_File *>(context->hidden.unknown.data1);
        int position, result;

        switch (whence)
        {
            default:
            case RW_SEEK_SET:
                result = PHYSFS_seek(file, offset);
                return result > 0 ? offset : -1;

            case RW_SEEK_CUR:
                position = offset + PHYSFS_tell(file);
                result = PHYSFS_seek(file, position);
                return result > 0 ? position : -1;

            case RW_SEEK_END:
                position = PHYSFS_fileLength(file) - offset;
                result = PHYSFS_seek(file, position);
                return result > 0 ? position : -1;
        }
    }

    return -1;
}


bool CResourceManager::CheckSDLContext(SDL_RWops *context)
{
    if (context->type != PHYSFS_RWOPS_TYPE)
    {
        SDL_SetError("Wrong kind of RWops");
        return false;
    }

    return true;
}
