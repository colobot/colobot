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

#include "common/resources/inputstreambuffer.h"

#include "common/resources/resourcemanager.h"

#include <stdexcept>
#include <sstream>

CInputStreamBuffer::CInputStreamBuffer(size_t buffer_size) : m_buffer_size(buffer_size)
{
    if (buffer_size <= 0)
    {
        throw std::runtime_error("File buffer must be larger then 0 bytes");
    }

    m_buffer = new char[buffer_size];
    m_file = nullptr;
}


CInputStreamBuffer::~CInputStreamBuffer()
{
    close();
    delete m_buffer;
}


void CInputStreamBuffer::open(const std::string &filename)
{
    if (PHYSFS_isInit())
        m_file = PHYSFS_openRead(CResourceManager::CleanPath(filename).c_str());
}


void CInputStreamBuffer::close()
{
    if (is_open())
        PHYSFS_close(m_file);
}


bool CInputStreamBuffer::is_open()
{
    return m_file;
}


size_t CInputStreamBuffer::size()
{
    return PHYSFS_fileLength(m_file);
}


std::streambuf::int_type CInputStreamBuffer::underflow()
{
    if (gptr() < egptr())
        return traits_type::to_int_type(*gptr());

    if (PHYSFS_eof(m_file))
        return traits_type::eof();

    PHYSFS_sint64 read_count = PHYSFS_read(m_file, m_buffer, sizeof(char), m_buffer_size);
    if (read_count <= 0)
        return traits_type::eof();

    setg(m_buffer, m_buffer, m_buffer + read_count);

    return traits_type::to_int_type(*gptr());
}


std::streampos CInputStreamBuffer::seekpos(std::streampos sp, std::ios_base::openmode which)
{
    return seekoff(off_type(sp), std::ios_base::beg, which);
}


std::streampos CInputStreamBuffer::seekoff(std::streamoff off, std::ios_base::seekdir way, std::ios_base::openmode which)
{
    /* A bit of explanation:
       We are reading file by m_buffer_size parts so our 3 internal pointers will be
       * eback (not used here) - start of block
       * gptr - position of read cursor in block
       * egtpr - end of block
       off argument is relative to way */

    std::streamoff new_position;

    switch (way)
    {
        case std::ios_base::beg:
            new_position = off;
            break;

        case std::ios_base::cur:
            // tell will give cursor at begining of block so we have to add where in block we currently are
            new_position = off + static_cast<off_type>(PHYSFS_tell(m_file)) - static_cast<off_type> (egptr() - gptr());
            break;

        case std::ios_base::end:
            new_position = off + static_cast<off_type>(PHYSFS_fileLength(m_file));
            break;

        default:
            break;
    }

    if (PHYSFS_seek(m_file, new_position))
    {
        setg(m_buffer, m_buffer, m_buffer); // reset buffer

        return pos_type(new_position);
    }

    return pos_type(off_type(-1));
}
