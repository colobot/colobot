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

#include "common/resources/sndfile_wrapper.h"

#include "common/stringutils.h"

#include <cstring>

namespace
{

sf_count_t SNDLength(void *data)
{
    return PHYSFS_fileLength(static_cast<PHYSFS_File *>(data));
}

sf_count_t SNDRead(void *ptr, sf_count_t count, void *data)
{
    return PHYSFS_readBytes(static_cast<PHYSFS_File *>(data), ptr, count);
}

sf_count_t SNDSeek(sf_count_t offset, int whence, void *data)
{
    PHYSFS_File *file = static_cast<PHYSFS_File *>(data);
    switch(whence)
    {
        case SEEK_CUR:
            PHYSFS_seek(file, PHYSFS_tell(file) + offset);
            break;
        case SEEK_SET:
            PHYSFS_seek(file, offset);
            break;
        case SEEK_END:
            PHYSFS_seek(file, PHYSFS_fileLength(file) + offset);
            break;
    }

    return PHYSFS_tell(file);
}

sf_count_t SNDTell(void *data)
{
    return PHYSFS_tell(static_cast<PHYSFS_File *>(data));
}

sf_count_t SNDWrite(const void *ptr, sf_count_t count, void *data)
{
    return PHYSFS_writeBytes(static_cast<PHYSFS_File *>(data), ptr, count);
}

} // namespace

CSNDFileWrapper::CSNDFileWrapper(const std::filesystem::path& filename)
{
    m_snd_callbacks = { SNDLength, SNDSeek, SNDRead, SNDWrite, SNDTell };
    if (PHYSFS_isInit())
    {
        m_file = PHYSFS_openRead(StrUtils::ToString(filename).c_str());
    }
    else
    {
        m_last_error = "Resource system not started!";
    }
    if (m_file)
    {
        m_snd_file = sf_open_virtual(&m_snd_callbacks, SFM_READ, &m_file_info, m_file);
        if (!m_snd_file)
        {
            m_last_error = "Could not load file";
        }
    }
    else
    {
        PHYSFS_ErrorCode errorCode = PHYSFS_getLastErrorCode();
        m_last_error = std::string(PHYSFS_getErrorByCode(errorCode));
    }
}

CSNDFileWrapper::~CSNDFileWrapper()
{
    if (m_file)
    {
        PHYSFS_close(m_file);
        if (m_snd_file)
        {
            sf_close(m_snd_file);
        }
    }
}

bool CSNDFileWrapper::IsOpen() const
{
    return m_file && m_snd_file;
}

const SF_INFO& CSNDFileWrapper::GetFileInfo() const
{
    return m_file_info;
}

const std::string& CSNDFileWrapper::GetLastError() const
{
    return m_last_error;
}

sf_count_t CSNDFileWrapper::Read(short int *ptr, sf_count_t items)
{
    return sf_read_short(m_snd_file, ptr, items);
}
