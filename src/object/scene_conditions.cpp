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

#include "object/scene_conditions.h"

#include "math/geometry.h"

#include "object/object.h"
#include "object/object_manager.h"

#include "object/interface/powered_object.h"
#include "object/interface/transportable_object.h"

#include "object/level/parserline.h"


void CSceneCondition::Read(CLevelParserLine* line)
{
    this->pos      = line->GetParam("pos")->AsPoint(Math::Vector(0.0f, 0.0f, 0.0f))*g_unit;
    this->dist     = line->GetParam("dist")->AsFloat(8.0f)*g_unit;
    this->type     = line->GetParam("type")->AsObjectType(OBJECT_NULL);
    this->powermin = line->GetParam("powermin")->AsFloat(-1);
    this->powermax = line->GetParam("powermax")->AsFloat(100);
    this->tool     = line->GetParam("tool")->AsToolType(ToolType::Other);
    this->drive    = line->GetParam("drive")->AsDriveType(DriveType::Other);
    this->countTransported = line->GetParam("countTransported")->AsBool(true);
    this->team     = line->GetParam("team")->AsInt(0);

    this->min      = line->GetParam("min")->AsInt(1);
    this->max      = line->GetParam("max")->AsInt(9999);
}

int CSceneCondition::CountObjects()
{
    Math::Vector bPos = this->pos;
    bPos.y = 0.0f;

    Math::Vector oPos;

    int nb = 0;
    for (CObject* obj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        // Do not use GetActive () because an invisible worm (underground)
        // should be regarded as existing here!
        if (obj->GetLock()) continue;
        if (obj->GetRuin()) continue;
        if (!obj->GetEnable()) continue;

        if (!this->countTransported)
        {
            if (IsObjectBeingTransported(obj)) continue;
        }

        // TODO: I really hate those fragments that hardcode subcategories into one in random places in code, we should refactor that at some point
        ObjectType type = obj->GetType();
        if (type == OBJECT_SCRAP2 ||
            type == OBJECT_SCRAP3 ||
            type == OBJECT_SCRAP4 ||
            type == OBJECT_SCRAP5)  // wastes?
        {
            type = OBJECT_SCRAP1;
        }

        ToolType tool = GetToolFromObject(type);
        DriveType drive = GetDriveFromObject(type);
        if (this->tool != ToolType::Other &&
            tool != this->tool)
            continue;

        if (this->drive != DriveType::Other &&
            drive != this->drive)
            continue;

        if (this->tool == ToolType::Other &&
            this->drive == DriveType::Other &&
            type != this->type &&
            this->type != OBJECT_NULL)
            continue;

        if ((this->team > 0 && obj->GetTeam() != this->team) ||
            (this->team < 0 && (obj->GetTeam() == -(this->team) || obj->GetTeam() == 0)))
            continue;

        float energyLevel = -1;
        CPowerContainerObject* power = nullptr;
        if (obj->Implements(ObjectInterfaceType::PowerContainer))
        {
            power = dynamic_cast<CPowerContainerObject*>(obj);
        }
        else if (obj->Implements(ObjectInterfaceType::Powered))
        {
            CObject* powerObj = dynamic_cast<CPoweredObject*>(obj)->GetPower();
            if(powerObj != nullptr && powerObj->Implements(ObjectInterfaceType::PowerContainer))
            {
                power = dynamic_cast<CPowerContainerObject*>(powerObj);
            }
        }

        if (power != nullptr)
        {
            energyLevel = power->GetEnergy();
            if (power->GetCapacity() > 1.0f) energyLevel *= 10; // TODO: Who designed it like that ?!?!
        }
        if (energyLevel < this->powermin || energyLevel > this->powermax) continue;

        if (IsObjectBeingTransported(obj))
            oPos = dynamic_cast<CTransportableObject*>(obj)->GetTransporter()->GetPosition();
        else
            oPos = obj->GetPosition();

        oPos.y = 0.0f;

        if (Math::DistanceProjected(oPos, bPos) <= this->dist)
            nb ++;
    }
    return nb;
}

bool CSceneCondition::Check()
{
    int nb = CountObjects();
    return nb >= this->min && nb <= this->max;
}


void CSceneEndCondition::Read(CLevelParserLine* line)
{
    CSceneCondition::Read(line);
    this->winTeam  = line->GetParam("winTeam")->AsInt(0);
    this->lost     = line->GetParam("lost")->AsInt(-1);
    this->immediat = line->GetParam("immediat")->AsBool(false);
}

bool CSceneEndCondition::CheckLost()
{
    int nb = CountObjects();
    return nb <= this->lost;
}

Error CSceneEndCondition::GetMissionResult()
{
    if (CheckLost())
    {
        if (this->type == OBJECT_HUMAN)
            return INFO_LOSTq;
        else
            return INFO_LOST;
    }

    if (!Check())
    {
        return ERR_MISSION_NOTERM;
    }

    return ERR_OK;
}


void CAudioChangeCondition::Read(CLevelParserLine* line)
{
    CSceneCondition::Read(line);
    this->music    = std::string("../")+line->GetParam("filename")->AsPath("music");
    this->repeat   = line->GetParam("repeat")->AsBool(true);
}
