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

#include "common/resources/outputstream.h"

#include "common/resources/outputstreambuffer.h"


COutputStream::COutputStream()
    : COutputStreamBufferContainer(),
      std::ostream(&m_buffer)
{
}

COutputStream::COutputStream(const std::filesystem::path& path, std::ios_base::openmode mode)
    : COutputStreamBufferContainer(),
      std::ostream(&m_buffer)
{
    open(path, mode);
}

COutputStream::~COutputStream()
{
}

void COutputStream::open(const std::filesystem::path& path, std::ios_base::openmode mode)
{
    m_buffer.open(path, mode);
}

void COutputStream::close()
{
    m_buffer.close();
}

bool COutputStream::is_open()
{
    return m_buffer.is_open();
}
