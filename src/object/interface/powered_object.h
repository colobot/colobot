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

#include "object/interface/power_container_object.h"

class CObject;

/**
 * \class CPoweredObject
 * \brief Interface for objects powered using power cells
 *
 * TODO: It currently includes objects that take other objects as input
 *       and convert them, for example PowerPlant.
 *       We should create a dedicated interface for such uses.
 */
class CPoweredObject
{
public:
    explicit CPoweredObject(ObjectInterfaceTypes& types)
    {
        types[static_cast<int>(ObjectInterfaceType::Powered)] = true;
    }
    virtual ~CPoweredObject()
    {}

    //! Returns the power cell
    virtual CObject* GetPower() = 0;
    //! Sets power cell
    virtual void SetPower(CObject* power) = 0;

    //! Returns the relative position of power cell
    virtual Math::Vector GetPowerPosition() = 0;
    //! Sets the relative position of power cell
    virtual void SetPowerPosition(const Math::Vector& powerPosition) = 0;
};

inline float GetObjectEnergy(CObject* object)
{
    float energy = 0.0f;

    if (object->Implements(ObjectInterfaceType::Powered))
    {
        CObject* power = dynamic_cast<CPoweredObject*>(object)->GetPower();
        if (power != nullptr && power->Implements(ObjectInterfaceType::PowerContainer))
        {
            energy = dynamic_cast<CPowerContainerObject*>(power)->GetEnergy();
        }
    }

    return energy;
}

inline float GetObjectEnergyLevel(CObject* object)
{
    float energy = 0.0f;

    if (object->Implements(ObjectInterfaceType::Powered))
    {
        CObject* power = dynamic_cast<CPoweredObject*>(object)->GetPower();
        if (power != nullptr && power->Implements(ObjectInterfaceType::PowerContainer))
        {
            energy = dynamic_cast<CPowerContainerObject*>(power)->GetEnergyLevel();
        }
    }

    return energy;
}

inline bool ObjectHasPowerCell(CObject* object)
{
    return object->Implements(ObjectInterfaceType::Powered) &&
           dynamic_cast<CPoweredObject*>(object)->GetPower() != nullptr;
}
