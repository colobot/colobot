/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2015, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "CBot/CBotStringArray.h"

////////////////////////////////////////////////////////////////////////////////
CBotStringArray::CBotStringArray()
{
}

////////////////////////////////////////////////////////////////////////////////
CBotStringArray::~CBotStringArray()
{
    m_data.clear(); // destroys data !
}

////////////////////////////////////////////////////////////////////////////////
int CBotStringArray::GetSize()
{
    return m_data.size();
}

////////////////////////////////////////////////////////////////////////////////
void CBotStringArray::Add(const CBotString& str)
{
    m_data.push_back(str);
}

////////////////////////////////////////////////////////////////////////////////
void CBotStringArray::SetSize(int nNewSize)
{
    m_data.resize(nNewSize);
}

////////////////////////////////////////////////////////////////////////////////
CBotString& CBotStringArray::operator[](int nIndex)
{
    return ElementAt(nIndex);
}

////////////////////////////////////////////////////////////////////////////////
CBotString& CBotStringArray::ElementAt(int nIndex)
{
    return m_data[nIndex];
}
