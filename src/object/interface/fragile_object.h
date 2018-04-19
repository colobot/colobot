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
 * \class CFragileObject
 * \brief Interface for objects that can be destroyed immediately after hit
 */
class CFragileObject : public CDestroyableObject
{
public:
    explicit CFragileObject(ObjectInterfaceTypes& types)
        : CDestroyableObject(types)
    {
        types[static_cast<int>(ObjectInterfaceType::Fragile)] = true;
    }
    virtual ~CFragileObject()
    {}
};
