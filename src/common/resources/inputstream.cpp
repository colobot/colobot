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

#include "common/resources/inputstream.h"
#include "common/resources/inputstreambuffer.h"


CInputStream::CInputStream() : std::istream(new CInputStreamBuffer())
{
}

CInputStream::CInputStream(const std::string& filename) : std::istream(new CInputStreamBuffer())
{
    open(filename);
}


CInputStream::~CInputStream()
{
    delete rdbuf();
}


void CInputStream::open(const std::string& filename)
{
    static_cast<CInputStreamBuffer *>(rdbuf())->open(filename);
}


void CInputStream::close()
{
    static_cast<CInputStreamBuffer *>(rdbuf())->close();
}


bool CInputStream::is_open()
{
    return static_cast<CInputStreamBuffer *>(rdbuf())->is_open();
}


size_t CInputStream::size()
{
    return static_cast<CInputStreamBuffer *>(rdbuf())->size();
}
