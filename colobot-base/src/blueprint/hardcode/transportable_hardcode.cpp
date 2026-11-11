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

#include "blueprint/blueprint_hardcode.h"
#include "blueprint/part/transportable_blueprint.h"

#include "object/object_type.h"

namespace {

bool Enabled(ObjectType type)
{
    switch (type)
    {
        case OBJECT_STONE:
        case OBJECT_URANIUM:
        case OBJECT_BULLET:
        case OBJECT_METAL:
        case OBJECT_POWER:
        case OBJECT_ATOMIC:
        case OBJECT_BBOX:
        case OBJECT_KEYa:
        case OBJECT_KEYb:
        case OBJECT_KEYc:
        case OBJECT_KEYd:
        case OBJECT_TNT:
            return true;
        default:
            return false;
    }
}

bool IsRadioactive(ObjectType type)
{
    switch (type)
    {
        case OBJECT_URANIUM:
            return true;
        default:
            return false;
    }
}

bool IsDropZoneVisible(ObjectType type)
{
    switch (type)
    {
        case OBJECT_METAL:
            return true;
        default:
            return false;
    }
}

} // namespace

template<>
TransportableBlueprint GetBlueprintHardcode<TransportableBlueprint>(ObjectType type)
{
    TransportableBlueprint hardcode;
    hardcode.enabled = Enabled(type);
    hardcode.isRadioactive = IsRadioactive(type);
    hardcode.isDropZoneVisible = IsDropZoneVisible(type);
    return hardcode;
}
