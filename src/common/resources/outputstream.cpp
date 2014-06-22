// * This file is part of the COLOBOT source code
// * Copyright (C) 2014 Polish Portal of Colobot (PPC)
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.

#include "common/resources/outputstream.h"
#include "common/resources/resourcestreambuffer.h"


COutputStream::COutputStream() : std::ostream(new CResourceStreamBuffer())
{
}


COutputStream::~COutputStream()
{
    delete rdbuf();
}


void COutputStream::open(const std::string& filename)
{
    static_cast<CResourceStreamBuffer *>(rdbuf())->open(filename);
}


void COutputStream::close()
{
    static_cast<CResourceStreamBuffer *>(rdbuf())->close();
}


bool COutputStream::is_open()
{
    return static_cast<CResourceStreamBuffer *>(rdbuf())->is_open();
}


size_t COutputStream::size()
{
    return static_cast<CResourceStreamBuffer *>(rdbuf())->size();
}
