/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#pragma once

#include "object/object_interface_type.h"

class CPhysics;
class CMotion;

/**
 * \class CMovableObject
 * \brief Interface for objects that can move (have an engine)
 *
 * TODO: Currently, it just returns pointers to CPhysics and CMotion.
 *       These classes should be probably merged with CObject,
 *       and maybe even split into some more interfaces.
 */
class CMovableObject
{
public:
    explicit CMovableObject(ObjectInterfaceTypes& types)
    {
        types[static_cast<int>(ObjectInterfaceType::Movable)] = true;
    }
    virtual ~CMovableObject()
    {}

    //! Returns CPhysics instance associated with this object. If the object implements Movable interface, and type != OBJECT_TOTO, this can be assumed to be != nullptr
    virtual CPhysics* GetPhysics() = 0;
    //! Returns CMotion instance associated with this object. If the object implements Movable interface, this can be assumed to be != nullptr
    virtual CMotion* GetMotion() = 0;
};
