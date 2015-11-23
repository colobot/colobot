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

// Modules inlcude
#include "CBot/CBotStringArray.h"

#include "CBot/CBotUtils.h"

// Local include

// Global include

// Forward declaration
#include <cstring>

////////////////////////////////////////////////////////////////////////////////
CBotStringArray::CBotStringArray()
{
    m_pData = nullptr;
    m_nSize = m_nMaxSize = 0;
}

////////////////////////////////////////////////////////////////////////////////
CBotStringArray::~CBotStringArray()
{
    SetSize(0);                    // destroys data !
}

////////////////////////////////////////////////////////////////////////////////
int CBotStringArray::GetSize()
{
    return m_nSize;
}

////////////////////////////////////////////////////////////////////////////////
void CBotStringArray::Add(const CBotString& str)
{
    SetSize(m_nSize+1);

    m_pData[m_nSize-1] = str;
}

////////////////////////////////////////////////////////////////////////////////
void CBotStringArray::SetSize(int nNewSize)
{
    if (nNewSize == 0)
    {
        // shrink to nothing

        DestructElements(m_pData, m_nSize);
        delete[] reinterpret_cast<unsigned char *>(m_pData);
        m_pData = nullptr;
        m_nSize = m_nMaxSize = 0;
    }
    else if (m_pData == nullptr)
    {
        // create one with exact size
        m_pData = reinterpret_cast<CBotString*> (new unsigned char[nNewSize * sizeof(CBotString)]);

        ConstructElements(m_pData, nNewSize);

        m_nSize = m_nMaxSize = nNewSize;
    }
    else if (nNewSize <= m_nMaxSize)
    {
        // it fits
        if (nNewSize > m_nSize)
        {
            // initialize the new elements

            ConstructElements(&m_pData[m_nSize], nNewSize-m_nSize);

        }

        else if (m_nSize > nNewSize)  // destroy the old elements
            DestructElements(&m_pData[nNewSize], m_nSize-nNewSize);

        m_nSize = nNewSize;
    }
    else
    {
        // otherwise, grow array
        int nGrowBy;
        {
            // heuristically determine growth when nGrowBy == 0
            //  (this avoids heap fragmentation in many situations)
            nGrowBy = std::min(1024, std::max(4, m_nSize / 8));
        }
        int nNewMax;
        if (nNewSize < m_nMaxSize + nGrowBy)
            nNewMax = m_nMaxSize + nGrowBy;  // granularity
        else
            nNewMax = nNewSize;  // no slush

        CBotString* pNewData = reinterpret_cast<CBotString*> (new unsigned char[nNewMax * sizeof(CBotString)]);

        // copy new data from old
        memcpy(pNewData, m_pData, m_nSize * sizeof(CBotString));

        // construct remaining elements
        ConstructElements(&pNewData[m_nSize], nNewSize-m_nSize);


        // Get rid of old stuff (note: no destructors called)
        delete[] reinterpret_cast<unsigned char *>(m_pData);
        m_pData = pNewData;
        m_nSize = nNewSize;
        m_nMaxSize = nNewMax;
    }
}

////////////////////////////////////////////////////////////////////////////////
CBotString& CBotStringArray::operator[](int nIndex)
{
    return ElementAt(nIndex);
}

////////////////////////////////////////////////////////////////////////////////
CBotString& CBotStringArray::ElementAt(int nIndex)
{
    return m_pData[nIndex];
}
