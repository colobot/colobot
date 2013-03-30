// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
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


#include "common/iman.h"

#include <cassert>


template<> CInstanceManager* CSingleton<CInstanceManager>::m_instance = nullptr;


CInstanceManager::CInstanceManager()
{
    for (int i = 0; i < CLASS_MAX; i++)
    {
        m_table[i].maxCount  = 0;
        m_table[i].usedCount = 0;
        m_table[i].instances = nullptr;
    }
}

CInstanceManager::~CInstanceManager()
{
    Flush();
}

void CInstanceManager::Flush()
{
    for (int i = 0; i < CLASS_MAX; i++)
    {
        if (m_table[i].instances != nullptr)
            delete[] m_table[i].instances;

        m_table[i].instances = nullptr;
    }
}

void CInstanceManager::Flush(ManagedClassType classType)
{
    if (classType < 0 || classType >= CLASS_MAX) return;
    if (m_table[classType].instances == nullptr) return;

    delete[] m_table[classType].instances;
    m_table[classType].instances = nullptr;
}

bool CInstanceManager::AddInstance(ManagedClassType classType, void* instance, int max)
{
    if (classType < 0 || classType >= CLASS_MAX) return false;

    if (m_table[classType].instances == nullptr)
    {
        m_table[classType].instances = new void*[max];
        m_table[classType].maxCount  = max;
        m_table[classType].usedCount = 0;
    }

    if (m_table[classType].usedCount >= m_table[classType].maxCount) return false;

    int i = m_table[classType].usedCount++;
    m_table[classType].instances[i] = instance;
    return true;
}

bool CInstanceManager::DeleteInstance(ManagedClassType classType, void* instance)
{
    if (classType < 0 || classType >= CLASS_MAX) return false;

    for (int i = 0; i < m_table[classType].usedCount; i++)
    {
        if (m_table[classType].instances[i] == instance)
            m_table[classType].instances[i] = nullptr;
    }

    Compress(classType);
    return true;
}

void* CInstanceManager::SearchInstance(ManagedClassType classType, int rank)
{
    if (classType < 0 || classType >= CLASS_MAX) return nullptr;
    if (m_table[classType].instances == nullptr) return nullptr;
    if (rank >= m_table[classType].usedCount) return nullptr;

    return m_table[classType].instances[rank];
}

void CInstanceManager::Compress(ManagedClassType classType)
{
    if (classType < 0 || classType >= CLASS_MAX) return;

    int j = 0;
    for (int i = 0; i < m_table[classType].usedCount; i++)
    {
        if (m_table[classType].instances[i] != nullptr)
            m_table[classType].instances[j++] = m_table[classType].instances[i];
    }
    m_table[classType].usedCount = j;
}
