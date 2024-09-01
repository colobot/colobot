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

#include "level/scene_conditions.h"

#include "level/parser/parserline.h"

#include "math/geometry.h"

#include "object/object.h"
#include "object/object_manager.h"

#include "object/interface/power_container_object.h"
#include "object/interface/slotted_object.h"
#include "object/interface/transportable_object.h"

#include "common/stringutils.h"

#include <limits>


void CObjectCondition::Read(CLevelParserLine* line)
{
    this->pos      = line->GetParam("pos")->AsPoint(glm::vec3(0.0f, 0.0f, 0.0f))*g_unit;
    this->dist     = line->GetParam("dist")->AsFloat(std::numeric_limits<float>::infinity())*g_unit;
    this->type     = line->GetParam("type")->AsObjectType(OBJECT_NULL);
    this->powermin = line->GetParam("powermin")->AsFloat(-1);
    this->powermax = line->GetParam("powermax")->AsFloat(100);
    this->tool     = line->GetParam("tool")->AsToolType(ToolType::Other);
    this->drive    = line->GetParam("drive")->AsDriveType(DriveType::Other);
    this->countTransported = line->GetParam("countTransported")->AsBool(true);
    this->team     = line->GetParam("team")->AsInt(0);
}

bool CObjectCondition::CheckForObject(CObject* obj)
{
    if (!this->countTransported)
    {
        if (IsObjectBeingTransported(obj)) return false;
    }

    ObjectType type = obj->GetType();

    ToolType tool = GetToolFromObject(type);
    DriveType drive = GetDriveFromObject(type);
    if (this->tool != ToolType::Other &&
        tool != this->tool)
        return false;

    if (this->drive != DriveType::Other &&
        drive != this->drive)
        return false;

    if (this->tool == ToolType::Other &&
        this->drive == DriveType::Other &&
        type != this->type &&
        this->type != OBJECT_NULL)
        return false;

    if ((this->team > 0 && obj->GetTeam() != this->team) ||
        (this->team < 0 && (obj->GetTeam() == -(this->team) || obj->GetTeam() == 0)))
        return false;

    float energyLevel = -1;
    CPowerContainerObject* power = nullptr;
    if (obj->Implements(ObjectInterfaceType::PowerContainer))
    {
        power = dynamic_cast<CPowerContainerObject*>(obj);
    }
    else
    {
        power = GetObjectPowerCell(obj);
    }

    if (power != nullptr)
    {
        energyLevel = power->GetEnergy();
        if (power->GetCapacity() > 1.0f) energyLevel *= 10; // TODO: Who designed it like that ?!?!
    }
    if (energyLevel < this->powermin || energyLevel > this->powermax) return false;

    glm::vec3 oPos{};
    if (IsObjectBeingTransported(obj))
        oPos = dynamic_cast<CTransportableObject&>(*obj).GetTransporter()->GetPosition();
    else
        oPos = obj->GetPosition();
    oPos.y = 0.0f;

    glm::vec3 bPos = this->pos;
    bPos.y = 0.0f;

    if (Math::DistanceProjected(oPos, bPos) <= this->dist)
        return true;

    return false;
}

int CObjectCondition::CountObjects()
{
    int nb = 0;
    for (CObject* obj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        if (!obj->GetActive()) continue;
        if (!CheckForObject(obj)) continue;
        nb ++;
    }
    return nb;
}

void CSceneCondition::Read(CLevelParserLine* line)
{
    CObjectCondition::Read(line);

    // Scene conditions STILL use a different default value
    // See issue #759
    this->dist     = line->GetParam("dist")->AsFloat(8.0f)*g_unit;

    this->min      = line->GetParam("min")->AsInt(1);
    this->max      = line->GetParam("max")->AsInt(9999);
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
    this->music    = line->GetParam("filename")->AsPath("music");
    this->repeat   = line->GetParam("repeat")->AsBool(true);
}
