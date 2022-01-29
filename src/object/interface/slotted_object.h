/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2020, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "math/vector.h"

#include "object/object.h"
#include "object/object_interface_type.h"

#include "object/interface/power_container_object.h"

#include <assert.h>

class CObject;

/**
 * \class CSlottedObject
 * \brief Interface for objects that hold other objects
 */
class CSlottedObject
{
public:
    ///! Object-independent identifiers for certain special slots
    enum class Pseudoslot
    {
        POWER,
        CARRYING
    };

    explicit CSlottedObject(ObjectInterfaceTypes& types)
    {
        types[static_cast<int>(ObjectInterfaceType::Slotted)] = true;
    }
    virtual ~CSlottedObject()
    {}

    //! Given one of the PSEUDOSLOT enums, returns real slot number, or -1 if specified pseudoslot is not present in this object.
    virtual int MapPseudoSlot(Pseudoslot pseudoslot) = 0;

    //! Get number of slots. Valid slot numbers are 0 up to GetNumSlots()-1. Using invalid slot numbers in the other functions will crash the game.
    virtual int GetNumSlots() = 0;
    //! Get relative position of a slot.
    virtual Math::Vector GetSlotPosition(int slotNum) = 0;
    //! Get relative angle (in radians) where robot should be positioned when inserting into a slot.
    virtual float GetSlotAngle(int slotNum) = 0;
    //! Get the maximum angular distance from the ideal angle (in radians) where robot should be positioned when inserting into a slot.
    virtual float GetSlotAcceptanceAngle(int slotNum) = 0;
    //! Get object contained in a slot.
    virtual CObject *GetSlotContainedObject(int slotNum) = 0;
    //! Set object contained in a slot.
    virtual void SetSlotContainedObject(int slotNum, CObject *object) = 0;

    void SetSlotContainedObjectReq(Pseudoslot pseudoslot, CObject *object)
    {
        int slotNum = MapPseudoSlot(pseudoslot);
        assert(slotNum >= 0);
        SetSlotContainedObject(slotNum, object);
    }

    CObject *GetSlotContainedObjectOpt(Pseudoslot pseudoslot)
    {
        int slotNum = MapPseudoSlot(pseudoslot);
        return slotNum < 0 ? nullptr : GetSlotContainedObject(slotNum);
    }

    CObject *GetSlotContainedObjectReq(Pseudoslot pseudoslot)
    {
        int slotNum = MapPseudoSlot(pseudoslot);
        assert(slotNum >= 0);
        return GetSlotContainedObject(slotNum);
    }
};

inline bool HasPowerCellSlot(CObject *object)
{
    if (object->Implements(ObjectInterfaceType::Slotted))
    {
        return dynamic_cast<CSlottedObject&>(*object).MapPseudoSlot(CSlottedObject::Pseudoslot::POWER) >= 0;
    }
    return false;
}

inline CObject *GetObjectInPowerCellSlot(CObject *object)
{
    if (object->Implements(ObjectInterfaceType::Slotted))
    {
        return dynamic_cast<CSlottedObject&>(*object).GetSlotContainedObjectOpt(CSlottedObject::Pseudoslot::POWER);
    }
    return nullptr;
}

inline CPowerContainerObject *GetObjectPowerCell(CObject *object)
{
    if (CObject *powerSlotObj = GetObjectInPowerCellSlot(object))
    {
        if (powerSlotObj->Implements(ObjectInterfaceType::PowerContainer))
        {
            return dynamic_cast<CPowerContainerObject*>(powerSlotObj);
        }
    }
    return nullptr;
}

inline float GetObjectEnergy(CObject* object)
{
    if (CPowerContainerObject* power = GetObjectPowerCell(object))
        return power->GetEnergy();
    return 0.0f;
}

inline float GetObjectEnergyLevel(CObject* object)
{
    if (CPowerContainerObject* power = GetObjectPowerCell(object))
        return power->GetEnergyLevel();
    return 0.0f;
}

inline bool ObjectHasPowerCell(CObject* object)
{
    // XXX: not GetObjectPowerCell? We count e.g. titanium cubes as power cells in this function?
    return GetObjectInPowerCellSlot(object) != nullptr;
}

inline bool IsObjectCarryingCargo(CObject* object)
{
    if (object->Implements(ObjectInterfaceType::Slotted))
    {
        return dynamic_cast<CSlottedObject&>(*object).GetSlotContainedObjectOpt(CSlottedObject::Pseudoslot::CARRYING) != nullptr;
    }
    return false;
}
