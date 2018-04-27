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

#include "object/interface/destroyable_object.h"

/**
 * \class CShieldedObject
 * \brief Interface for objects that can be destroyed, but only after the shield goes down to 0
 */
class CShieldedObject : public CDestroyableObject
{
public:
    explicit CShieldedObject(ObjectInterfaceTypes& types)
        : CDestroyableObject(types)
    {
        types[static_cast<int>(ObjectInterfaceType::Shielded)] = true;
    }
    virtual ~CShieldedObject()
    {}

    //! Set shield level
    virtual void SetShield(float level) = 0;
    //! Get shield level
    virtual float GetShield() = 0;

    //! Set damage multiplier for the object (bigger = more damage, weaker shield)
    virtual void SetMagnifyDamage(float factor) = 0;
    //! Return damage multiplier for the object (bigger = more damage, weaker shield)
    virtual float GetMagnifyDamage() = 0;

    //! Returns true if this object can be repaired in RepairStation
    virtual bool IsRepairable() = 0;
};
