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


#include "common/resources/sdl_file_wrapper.h"

#include "common/logger.h"

#include <physfs.h>


CSDLFileWrapper::CSDLFileWrapper(const std::string& filename)
    : m_rwops(nullptr)
{
    if (!PHYSFS_isInit())
    {
        GetLogger()->Error("PHYSFS not initialized!\n");
        return;
    }

    PHYSFS_File *file = PHYSFS_openRead(filename.c_str());
    if (file == nullptr)
    {
        GetLogger()->Error("Error opening file with PHYSFS: \"%s\"\n", filename.c_str());
        return;
    }

    m_rwops = SDL_AllocRW();
    if (m_rwops == nullptr)
    {
        GetLogger()->Error("Unable to allocate SDL_RWops for \"%s\"\n", filename.c_str());
        return;
    }

    m_rwops->type = SDL_RWOPS_UNKNOWN;
    m_rwops->hidden.unknown.data1 = file;
    m_rwops->seek = SDLSeek;
    m_rwops->read = SDLRead;
    m_rwops->write = SDLWrite;
    m_rwops->size = SDLSize;
    // This is safe because SDL_FreeRW will be called in destructor
    m_rwops->close = SDLCloseWithoutFreeRW;
}

CSDLFileWrapper::~CSDLFileWrapper()
{
    SDLCloseWithFreeRW(m_rwops);
}

SDL_RWops* CSDLFileWrapper::GetHandler()
{
    return m_rwops;
}

bool CSDLFileWrapper::IsOpen() const
{
    return m_rwops != nullptr;
}

int CSDLFileWrapper::SDLClose(SDL_RWops *context, bool freeRW)
{
    if (context == nullptr)
        return 0;

    if (!CheckSDLContext(context))
        return 1;

    if (context->hidden.unknown.data1 != nullptr)
    {
        PHYSFS_close(static_cast<PHYSFS_File *>(context->hidden.unknown.data1));
        context->hidden.unknown.data1 = nullptr;
    }

    if (freeRW)
        SDL_FreeRW(context);

    return 0;
}

int CSDLFileWrapper::SDLCloseWithoutFreeRW(SDL_RWops *context)
{
    return SDLClose(context, false);
}

int CSDLFileWrapper::SDLCloseWithFreeRW(SDL_RWops *context)
{
    return SDLClose(context, true);
}

bool CSDLFileWrapper::CheckSDLContext(SDL_RWops *context)
{
    if (context->type != SDL_RWOPS_UNKNOWN)
    {
        SDL_SetError("Wrong kind of RWops");
        return false;
    }

    return true;
}

Sint64 CSDLFileWrapper::SDLSeek(SDL_RWops *context, Sint64 offset, int whence)
{
    if (CheckSDLContext(context))
    {
        PHYSFS_File *file = static_cast<PHYSFS_File *>(context->hidden.unknown.data1);

        switch (whence)
        {
            default:
            case RW_SEEK_SET:
            {
                auto result = PHYSFS_seek(file, offset);
                return (result != 0) ? offset : -1;
            }

            case RW_SEEK_CUR:
            {
                int position = offset + PHYSFS_tell(file);
                auto result = PHYSFS_seek(file, position);
                return (result != 0) ? position : -1;
            }

            case RW_SEEK_END:
            {
                int position = PHYSFS_fileLength(file) - offset;
                auto result = PHYSFS_seek(file, position);
                return (result != 0) ? position : -1;
            }
        }
    }

    return -1;
}

Sint64 CSDLFileWrapper::SDLSize(SDL_RWops *context)
{
    return -1; // Not needed for now
}

std::size_t CSDLFileWrapper::SDLRead(SDL_RWops *context, void *ptr, std::size_t size, std::size_t maxnum)
{
    if (CheckSDLContext(context))
    {
        PHYSFS_File *file = static_cast<PHYSFS_File *>(context->hidden.unknown.data1);
        SDL_memset(ptr, 0, size * maxnum);

        auto result = PHYSFS_read(file, ptr, size, maxnum);
        return (result >= 0) ? result : 0;
    }

    return 0;
}

std::size_t CSDLFileWrapper::SDLWrite(SDL_RWops *context, const void *ptr, std::size_t size, std::size_t num)
{
    assert(!!"Writing to CSDLFileWrapper is currently not supported");
    return 0;
}
