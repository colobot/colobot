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

/**
 * \file common/singleton.h
 * \brief CSingleton base class for singletons
 */

#pragma once

#include <cassert>


template<typename T> class CSingleton
{
protected:
    static T* m_instance;

public:
    static T& GetInstance()
    {
        assert(m_instance != nullptr);
        return *m_instance;
    }

    static T* GetInstancePointer()
    {
        assert(m_instance != nullptr);
        return m_instance;
    }

    static bool IsCreated()
    {
        return m_instance != nullptr;
    }

    CSingleton()
    {
        assert(m_instance == nullptr);
        m_instance = static_cast<T *>(this);
    }

    virtual ~CSingleton()
    {
        m_instance = nullptr;
    }

    #ifdef TESTS
    static void ReplaceInstance(T* newInstance)
    {
        assert(newInstance != nullptr);
        delete m_instance;
        m_instance = newInstance;
    }
    #endif

private:
    CSingleton& operator=(const CSingleton<T> &);
    CSingleton(const CSingleton<T> &);
};

