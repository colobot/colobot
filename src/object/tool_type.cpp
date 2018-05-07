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

#include "object/tool_type.h"

ToolType GetToolFromObject(ObjectType type)
{
    switch (type)
    {
        case OBJECT_MOBILEwa:
        case OBJECT_MOBILEta:
        case OBJECT_MOBILEfa:
        case OBJECT_MOBILEia:
            return ToolType::Grabber;

        case OBJECT_MOBILEws:
        case OBJECT_MOBILEts:
        case OBJECT_MOBILEfs:
        case OBJECT_MOBILEis:
            return ToolType::Sniffer;

        case OBJECT_MOBILEwc:
        case OBJECT_MOBILEtc:
        case OBJECT_MOBILEfc:
        case OBJECT_MOBILEic:
            return ToolType::Shooter;

        case OBJECT_MOBILEwi:
        case OBJECT_MOBILEti:
        case OBJECT_MOBILEfi:
        case OBJECT_MOBILEii:
            return ToolType::OrganicShooter;

        default:
            return ToolType::Other;
    }
}
