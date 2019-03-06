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

#pragma once

#include <cstddef>
#include <memory>
#include <streambuf>
#include <string>

#include <physfs.h>

class CInputStreamBuffer : public std::streambuf
{
public:
    CInputStreamBuffer(std::size_t bufferSize = 512);
    virtual ~CInputStreamBuffer();

    CInputStreamBuffer(const CInputStreamBuffer &) = delete;
    CInputStreamBuffer &operator= (const CInputStreamBuffer &) = delete;

    void open(const std::string &filename);
    void close();
    bool is_open();
    std::size_t size();

private:
    int_type underflow() override;

    std::streampos seekpos(std::streampos sp, std::ios_base::openmode which) override;
    std::streampos seekoff(std::streamoff off, std::ios_base::seekdir way, std::ios_base::openmode which) override;

    const std::size_t m_bufferSize;
    PHYSFS_File *m_file;
    std::unique_ptr<char[]> m_buffer;
};
