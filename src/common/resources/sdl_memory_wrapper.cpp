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


#include "common/resources/sdl_memory_wrapper.h"

#include "common/logger.h"
#include "common/make_unique.h"

#include <physfs.h>


CSDLMemoryWrapper::CSDLMemoryWrapper(const std::string& filename)
    : m_rwops(nullptr)
{
    GetLogger()->Trace("Opening SDL memory wrapper for file '%s'\n", filename.c_str());

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

    PHYSFS_sint64 length = PHYSFS_fileLength(file);
    m_buffer = MakeUniqueArray<char>(length);
    if (PHYSFS_read(file, m_buffer.get(), 1, length) != length)
    {
        GetLogger()->Error("Unable to read data for \"%s\"\n", filename.c_str());
        PHYSFS_close(file);
        return;
    }
    PHYSFS_close(file);
    m_rwops = SDL_RWFromMem(m_buffer.get(), length);

    if (m_rwops == nullptr)
    {
        GetLogger()->Error("Unable to allocate SDL_RWops for \"%s\"\n", filename.c_str());
        return;
    }
}

CSDLMemoryWrapper::~CSDLMemoryWrapper()
{
    SDL_FreeRW(m_rwops);
    m_buffer.reset();
}

SDL_RWops* CSDLMemoryWrapper::GetHandler()
{
    return m_rwops;
}

bool CSDLMemoryWrapper::IsOpen() const
{
    return m_rwops != nullptr;
}

