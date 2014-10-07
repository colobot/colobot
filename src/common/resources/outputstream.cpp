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
#include "common/resources/outputstreambuffer.h"


COutputStream::COutputStream() : std::ostream(new COutputStreamBuffer())
{
}

COutputStream::COutputStream(const std::string& filename) : std::ostream(new COutputStreamBuffer())
{
    open(filename);
}


COutputStream::~COutputStream()
{
    delete rdbuf();
}


void COutputStream::open(const std::string& filename)
{
    static_cast<COutputStreamBuffer *>(rdbuf())->open(filename);
}


void COutputStream::close()
{
    static_cast<COutputStreamBuffer *>(rdbuf())->close();
}


bool COutputStream::is_open()
{
    return static_cast<COutputStreamBuffer *>(rdbuf())->is_open();
}
