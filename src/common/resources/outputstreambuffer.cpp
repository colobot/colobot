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

#include "common/resources/outputstreambuffer.h"

#include "common/make_unique.h"

#include "common/resources/resourcemanager.h"

#include <stdexcept>
#include <sstream>

COutputStreamBuffer::COutputStreamBuffer(std::size_t bufferSize)
    : m_file(nullptr)
{
    if (bufferSize <= 0)
    {
        throw std::runtime_error("File buffer must be larger then 0 bytes");
    }

    m_buffer = MakeUniqueArray<char>(bufferSize);
    setp(m_buffer.get(), m_buffer.get() + bufferSize);
}


COutputStreamBuffer::~COutputStreamBuffer()
{
    close();
}


void COutputStreamBuffer::open(const std::string &filename, std::ios_base::openmode mode)
{
    if (PHYSFS_isInit())
    {
        if ( mode == std::ios_base::out ) m_file = PHYSFS_openWrite(CResourceManager::CleanPath(filename).c_str());
        else if ( mode == std::ios_base::app ) m_file = PHYSFS_openAppend(CResourceManager::CleanPath(filename).c_str());
    }
}


void COutputStreamBuffer::close()
{
    sync();
    if (is_open())
        PHYSFS_close(m_file);
}


bool COutputStreamBuffer::is_open()
{
    return m_file != nullptr;
}


std::streambuf::int_type COutputStreamBuffer::overflow(std::streambuf::int_type ch)
{
    /* This function should be called when pptr() == epptr(). We use it also in sync()
       so we also have to write data if buffer is not full. */

    if (pbase() == pptr()) // no data to write, sync() called with empty buffer
        return 0;

    // save buffer
    PHYSFS_sint64 bytes_written = PHYSFS_write(m_file, pbase(), 1, pptr() - pbase());
    if (bytes_written <= 0)
        return traits_type::eof();

    pbump(-bytes_written);
    // write final char
    if (ch != traits_type::eof())
    {
        bytes_written = PHYSFS_write(m_file, &ch, 1, 1);
        if (bytes_written <= 0)
            return traits_type::eof();
    }

    return ch;
}


int COutputStreamBuffer::sync()
{
    return overflow(traits_type::eof());
}
