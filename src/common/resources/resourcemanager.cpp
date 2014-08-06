// * This file is part of the COLOBOT source code
// * Copyright (C) 2014 Polish Portal of Colobot (PPC)
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.


#include "common/resources/resourcemanager.h"

#include "common/config.h"
#include "common/logger.h"

#include <physfs.h>


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
    
    PHYSFS_File *file = PHYSFS_openRead(filename.c_str());
    if (!file)
    {
        SDL_FreeRW(handler);
        return nullptr;
    }

    handler->seek = SDLSeek;
    handler->read = SDLRead;
    handler->write = SDLWrite;
    handler->close = SDLClose;
    handler->type = 0xc010b04f;
    handler->hidden.unknown.data1 = file;
    
    return handler;
}


CSNDFile* CResourceManager::GetSNDFileHandler(const std::string &filename)
{   
    return new CSNDFile(filename);
}


bool CResourceManager::Exists(const std::string &filename)
{
    return PHYSFS_exists(filename.c_str());
}

std::vector<std::string> CResourceManager::ListFiles(const std::string &directory)
{
    std::vector<std::string> result;
    
    char **files = PHYSFS_enumerateFiles(directory.c_str());
    
    for (char **i = files; *i != nullptr; i++) {
        result.push_back(*i);
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
    if (context->type != 0xc010b04f)
    {
        SDL_SetError("Wrong kind of RWops");
        return false;
    }
    
    return true;
}
