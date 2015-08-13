/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2015, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "object/interface/damageable_object.h"

enum class DestructionType
{
    NoEffect       = 0, //!< no effect, just disappear (only for MissionController constants, do not actually pass this to DestroyObject)
    Explosion      = 1, //!< explosion
    ExplosionWater = 2, //!< explosion underwater
    Burn           = 3, //!< burning
    Drowned        = 4, //!< drowned (only for Me)
};

/**
 * \class CDestroyableObject
 * \brief Interface for objects that can be destroyed
 *
 * NOTE: None of the objects should inherit this class directly. Instead, inherit one of the subclasses (CShieldedObject or CFragileObject)
 */
class CDestroyableObject : public CDamageableObject
{
public:
    explicit CDestroyableObject(ObjectInterfaceTypes& types)
        : CDamageableObject(types)
    {
        types[static_cast<int>(ObjectInterfaceType::Destroyable)] = true;
    }
    virtual ~CDestroyableObject()
    {}

    //! Destroy the object immediately. Use this only if you are 100% sure this is what you want, because object with magnifyDamage=0 should be able to bypass all damage. It's recommended to use CDamageableObject::DamageObject() instead.
    virtual void DestroyObject(DestructionType type) = 0;

    //! Returns the distance modifier for lightning, used to modify hit probability. Value in range [0..1], where 0 is never and 1 is normal probability
    virtual float GetLightningHitProbability() = 0;
};
