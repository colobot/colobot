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


#include "common/resources/sdl_file_wrapper.h"

#include "common/logger.h"
#include "common/stringutils.h"
#include "common/resources/physfs_utils.h"

#include <physfs.h>

namespace
{

bool SDLClose(void *userdata)
{
    if (userdata == nullptr)
		return false;

	PHYSFS_close(static_cast<PHYSFS_file *>(userdata));
	
    return true;
}

Sint64 SDLSeek(void *userData, Sint64 offset, SDL_IOWhence whence)
{
	PHYSFS_File *file = static_cast<PHYSFS_File *>(userData);

	switch (whence)
	{
	default:
	case SDL_IO_SEEK_SET:
		{
			auto result = PHYSFS_seek(file, offset);
			return (result != 0) ? offset : -1;
		}

	case SDL_IO_SEEK_CUR:
		{
			int position = offset + PHYSFS_tell(file);
			auto result = PHYSFS_seek(file, position);
			return (result != 0) ? position : -1;
		}

	case SDL_IO_SEEK_END:
		{
			int position = PHYSFS_fileLength(file) - offset;
			auto result = PHYSFS_seek(file, position);
			return (result != 0) ? position : -1;
		}
	}

    return -1;
}

Sint64 SDLSize(void *userdata)
{
    return -1; // Not needed for now
}

size_t SDLRead(void *userdata, void *ptr, size_t size, SDL_IOStatus* status)
{
	PHYSFS_File *file = static_cast<PHYSFS_File *>(userdata);
	SDL_memset(ptr, 0, size);

	auto result = PHYSFS_readBytes(file, ptr, size);
	return (result >= 0) ? result : 0;
}

size_t SDLWrite(void *userdata, const void *ptr, size_t size, SDL_IOStatus* status)
{
    assert(!!"Writing to CSDLFileWrapper is currently not supported");
    return 0;
}

} // namespace

CSDLFileWrapper::CSDLFileWrapper(const std::filesystem::path& filename)
{
    if (!PHYSFS_isInit())
    {
        GetLogger()->Error("PHYSFS not initialized!");
        return;
    }

    auto path = StrUtils::ToString(filename.lexically_normal());

    PHYSFS_File *file = LoudOpenRead(path.c_str());
    if (file == nullptr)
    {
        GetLogger()->Error("Error opening file with PHYSFS: \"%%\"", filename);
        return;
    }

	SDL_IOStreamInterface interface;
	
    interface.seek = SDLSeek;
    interface.read = SDLRead;
    interface.write = SDLWrite;
    interface.size = SDLSize;
    interface.close = SDLClose;

    m_rwops = SDL_OpenIO(&interface, file);
    if (m_rwops == nullptr)
    {
        GetLogger()->Error("Unable to allocate SDL_IOStream for \"%%\"\n", filename);
        return;
    }
}

CSDLFileWrapper::~CSDLFileWrapper()
{
    SDL_CloseIO(m_rwops);
}

SDL_IOStream* CSDLFileWrapper::GetHandler()
{
    return m_rwops;
}

bool CSDLFileWrapper::IsOpen() const
{
    return m_rwops != nullptr;
}
