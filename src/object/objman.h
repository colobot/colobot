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

/**
 * \file common/objman.h
 * \brief Instance manager for objects
 */

#pragma once

#include "object/object.h"

#include "common/singleton.h"

const int MAX_OBJECTS = 500;

/**
 * \class ObjectManager
 * \brief Manager for objects
 */
class CObjectManager : public CSingleton<CObjectManager>
{
public:
    CObjectManager();
    virtual ~CObjectManager();

    //! Registers new object
    bool      AddInstance(CObject* instance);
    //! Deletes the registered object
    bool      DeleteInstance(CObject* instance);
    //! Seeks for an object
    CObject*  SearchInstance(int id);
    //! Creates an object
    CObject*  CreateObject(Math::Vector pos, float angle, float zoom, float height, ObjectType type, float power, bool trainer, bool toy, int option);

protected:
    CObject* m_table[MAX_OBJECTS];
    int usedCount;
};


