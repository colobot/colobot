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

#include "object/interface/shielded_object.h"

/**
 * \class CShieldedAutoRegenObject
 * \brief Interface for shielded objects with auto regeneration
 */
class CShieldedAutoRegenObject : public CShieldedObject
{
public:
    explicit CShieldedAutoRegenObject(ObjectInterfaceTypes& types)
        : CShieldedObject(types)
    {
        types[static_cast<int>(ObjectInterfaceType::ShieldedAutoRegen)] = true;
    }
    virtual ~CShieldedAutoRegenObject()
    {}

    //! Return time (in seconds) required for full shield regeneration
    virtual float GetShieldFullRegenTime() = 0;
};
