/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
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

    //! Set transporter object that transports this object
    virtual void SetTransporter(CObject* transporter) = 0;
    //! Return transported object
    virtual CObject* GetTransporter() const = 0;

    // TODO: This will have to be refactored while implementing new model format
    virtual void SetTransporterPart(int part) = 0;

    //! Return true if the object is currently transported
    inline bool IsBeingTransported() const
    {
        return GetTransporter() != nullptr;
    }
};

inline bool IsObjectBeingTransported(const CObject* obj)
{
    return obj->Implements(ObjectInterfaceType::Transportable) &&
           dynamic_cast<const CTransportableObject&>(*obj).IsBeingTransported();
}
