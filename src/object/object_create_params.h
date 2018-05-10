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

#include "math/vector.h"

#include "object/object_type.h"

struct ObjectCreateParams
{
    Math::Vector pos;
    float angle;
    ObjectType type;
    float power;
    float height;
    bool trainer;
    bool toy;
    int option;
    int team;
    int id;

    ObjectCreateParams()
    {
        pos = Math::Vector(0.0f, 0.0f, 0.0f);
        angle = 0.0f;
        type = OBJECT_NULL;
        power = -1.0f;
        height = 0.0f;
        trainer = false;
        toy = false;
        option = 0;
        team = 0;
        id = -1;
    }
};
