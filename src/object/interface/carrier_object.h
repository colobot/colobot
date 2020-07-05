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

#include "object/object.h"
#include "object/object_interface_type.h"

/**
 * \class CCarrierObject
 * \brief Interface for carrier objects
 */
class CCarrierObject
{
public:
    explicit CCarrierObject(ObjectInterfaceTypes& types)
    {
        types[static_cast<int>(ObjectInterfaceType::Carrier)] = true;
    }
    virtual ~CCarrierObject()
    {}

    //! Returns carried object
    virtual CObject* GetCargo() = 0;
    //! Sets carried object
    virtual void SetCargo(CObject* cargo) = 0;

    //! Checks whether there is any cargo
    inline bool IsCarryingCargo()
    {
        return GetCargo() != nullptr;
    }
};

inline bool IsObjectCarryingCargo(CObject* obj)
{
    return obj->Implements(ObjectInterfaceType::Carrier) &&
           dynamic_cast<CCarrierObject*>(obj)->IsCarryingCargo();
}
