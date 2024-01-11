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


#include "common/resources/sdl_memory_wrapper.h"

#include "common/logger.h"
#include "common/stringutils.h"

#include <physfs.h>


CSDLMemoryWrapper::CSDLMemoryWrapper(const std::filesystem::path& filename)
    : m_rwops(nullptr)
{
    GetLogger()->Trace("Opening SDL memory wrapper for file '%%'\n", filename);

    if (!PHYSFS_isInit())
    {
        GetLogger()->Error("PHYSFS not initialized!");
        return;
    }

    PHYSFS_File *file = PHYSFS_openRead(StrUtils::ToString(filename.lexically_normal()).c_str());
    if (file == nullptr)
    {
        GetLogger()->Error("Error opening file with PHYSFS: \"%%\"", filename);
        return;
    }

    PHYSFS_sint64 length = PHYSFS_fileLength(file);
    m_buffer = std::make_unique<char[]>(length);
    if (PHYSFS_readBytes(file, m_buffer.get(), length) != length)
    {
        GetLogger()->Error("Unable to read data for \"%%\"", filename);
        PHYSFS_close(file);
        return;
    }
    PHYSFS_close(file);
    m_rwops = SDL_RWFromMem(m_buffer.get(), length);

    if (m_rwops == nullptr)
    {
        GetLogger()->Error("Unable to allocate SDL_RWops for \"%%\"\n", filename);
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

