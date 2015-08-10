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

#pragma once

#include "object/object.h"
#include "object/object_interface_type.h"

/**
 * \class CTransportableObject
 * \brief Interface for transportable objects
 */
class CTransportableObject
{
public:
    explicit CTransportableObject(ObjectInterfaceTypes& types)
    {
        types[static_cast<int>(ObjectInterfaceType::Transportable)] = true;
    }
    virtual ~CTransportableObject()
    {}

    virtual void SetTransporter(CObject* transporter) = 0;
    virtual CObject* GetTransporter() = 0;
    virtual void SetTransporterPart(int part) = 0;

    inline bool IsBeingTransported()
    {
        return GetTransporter() != nullptr;
    }
};

inline bool IsObjectBeingTransported(CObject* obj)
{
    return obj->Implements(ObjectInterfaceType::Transportable) &&
           dynamic_cast<CTransportableObject*>(obj)->IsBeingTransported();
}
