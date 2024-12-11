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

#include "object/object_interface_type.h"

#include <limits>

class CObject;

/**
 * \enum DamageType
 * \brief Type of damage, for use in CDamageableObject::DamageObject
 */
enum class DamageType
{
    Fire          = 1, //!< fire damage (AlienSpider or Shooter), burns on destruction
    AlienAnt      = 2, //!< organical damage (AlienAnt), explodes on destruction
    Phazer        = 3, //!< damage from PhazerShooter, explodes on destruction
    Tower         = 4, //!< damage from DefenseTower, explodes on destruction
    FallingObject = 5, //!< damaged by an falling object (OrgaMatter dropped by an AlienWasp), explodes on destruction
    Explosive     = 6, //!< destroyed by an explosive, explodes on destruction
    Collision     = 7, //!< damage after one object hits another, explodes on destruction
    Lightning     = 8, //!< struck by lightning, explodes on destruction
    Fall          = 9, //!< fall damage, explodes on destruction
    OrgaShooter   = 10, //!< damage from OrgaShooter, explodes on destruction
};

/**
 * \class CDamageableObject
 * \brief Interface for objects that generate particles when hit
 */
class CDamageableObject
{
public:
    explicit CDamageableObject(ObjectInterfaceTypes& types)
    {
        types[static_cast<int>(ObjectInterfaceType::Damageable)] = true;
    }
    virtual ~CDamageableObject()
    {}

    //! Damage the object, with the given force. Returns true if the object has been fully destroyed (assuming the object is destroyable, of course). If force == infinity, destroy immediately (this is the default value)
    /** NOTE: You should never assume that after this function exits, the object is destroyed, unless it returns true. Even if you specify force = infinity, if may still sometimes decide not to destroy the object. */
    virtual bool DamageObject(DamageType type, float force = std::numeric_limits<float>::infinity(), CObject* killer = nullptr) = 0;


    //! Set the status that means the object is currently taking damage
    virtual void        SetDamaging(bool damaging) = 0;
    //! Is object currently taking damage?
    virtual bool        IsDamaging() = 0;

};
