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

#include "object/interface/flying_object.h"

/**
 * \class CJetFlyingObject
 * \brief Interface for objects that can fly using a jet engine
 */
class CJetFlyingObject : public CFlyingObject
{
public:
    explicit CJetFlyingObject(ObjectInterfaceTypes& types)
        : CFlyingObject(types)
    {
        types[static_cast<int>(ObjectInterfaceType::JetFlying)] = true;
    }
    virtual ~CJetFlyingObject()
    {}

    // TODO: Refactor naming of these functions

    //! Sets jet engine heating speed (bigger = slower, 0 for infinite)
    virtual void SetRange(float range) = 0;
    //! Returns jet engine heating speed (bigger = slower, 0 for infinite)
    virtual float GetRange() = 0;

    //! Sets current jet engine heat level (this is actually how much is left before it overheats, so smaller = more hot)
    virtual void SetReactorRange(float reactorRange) = 0;
    //! Returns current jet engine heat level (this is actually how much is left before it overheats, so smaller = more hot)
    virtual float GetReactorRange() = 0;
};
