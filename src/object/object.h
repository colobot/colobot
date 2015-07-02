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
 * \file object/object.h
 * \brief CObject - base class for all game objects
 */

#pragma once

#include "object/object_interface_type.h"
#include "object/old_object_interface.h"

/**
 * \class CObject
 * \brief Base class for all 3D in-game objects
 *
 * CObject serves as a base class for all in-game objects, including:
 *  - buildings,
 *  - robots,
 *  - astronaut,
 *  - plants,
 *  - aliens.
 *
 * As every object has its specific behavior, there are or will be
 * separate subclasses for each of the specific objects. For the time being,
 * old object interface is still present, but its functions will be moved to
 * appropriate subclasses with time. The new CObject interface implemented
 * here will feature only functions common to all objects.
 */
class CObject : public COldObjectInterface
{
protected:
    //! Constructor only accessible to subclasses
    CObject(int id, ObjectType type)
        : m_id(id)
        , m_type(type)
    {
        m_implementedInterfaces.fill(false);
    }

public:
    CObject(const CObject&) = delete;
    CObject& operator=(const CObject&) = delete;

    virtual ~CObject()
    {}

    //! Returns object type
    inline ObjectType  GetType() const
    {
        return m_type;
    }
    //! Returns object's unique id
    inline int GetID() const
    {
        return m_id;
    }

    //! Writes object properties to line in level file
    virtual void Write(CLevelParserLine* line) = 0;
    //! Reads object properties from line in level file
    virtual void Read(CLevelParserLine* line) = 0;

    //! Check if object implements the given type of interface
    inline bool Implements(ObjectInterfaceType type) const
    {
        return m_implementedInterfaces[static_cast<int>(type)];
    }

protected:
    const int m_id; //!< unique identifier
    ObjectType m_type; //!< object type
    std::string m_name; //!< object class name
    ObjectInterfaceTypes m_implementedInterfaces; //! interfaces that the object implements
};
