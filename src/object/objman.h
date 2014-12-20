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
 * \file object/objman.h
 * \brief Object manager
 */

#pragma once

#include "object/object.h"

#include "common/singleton.h"

#include <map>

/**
 * \class ObjectManager
 * \brief Manages CObject instances
 */
class CObjectManager : public CSingleton<CObjectManager>
{
public:
    CObjectManager();
    virtual ~CObjectManager();

    //! Registers new object
    bool      AddObject(CObject* instance);
    //! Unregisters the object
    bool      DeleteObject(CObject* instance);
    //! Finds object by id
    CObject*  GetObjectById(int id);
    //! Removes all objects
    void      Flush();
    
    
    //! Creates an object
    CObject*  CreateObject(Math::Vector pos, float angle, ObjectType type, float power = -1.f, float zoom = 1.f, float height = 0.f, bool trainer = false, bool toy = false, int option = 0);
    //! Destroys an object
    bool      DestroyObject(int id);

protected:
    std::map<int, CObject*> m_table;
};

