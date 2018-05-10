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

#include "object/subclass/shielder.h"

#include "common/make_unique.h"

#include "graphics/engine/oldmodelmanager.h"

#include "level/parser/parserline.h"
#include "level/parser/parserparam.h"

#include "object/object_create_params.h"

#include "object/motion/motionvehicle.h"

#include "object/task/taskshield.h"

#include "physics/physics.h"


CShielder::CShielder(int id)
    : CBaseRobot(id, OBJECT_MOBILErs),
      m_shieldRadius(1.0f)
{}

CShielder::~CShielder()
{}

std::unique_ptr<CShielder> CShielder::Create(
    const ObjectCreateParams& params,
    Gfx::COldModelManager* modelManager,
    Gfx::CEngine* engine)
{
    assert(params.type == OBJECT_MOBILErs);
    auto obj = MakeUnique<CShielder>(params.id);

    obj->SetTeam(params.team);
    obj->SetTrainer(params.trainer);
    obj->SetToy(params.toy);

    auto physics = MakeUnique<CPhysics>(obj.get());
    auto motion = MakeUnique<CMotionVehicle>(obj.get());

    motion->SetPhysics(physics.get());
    physics->SetMotion(motion.get());

    motion->Create(params.pos, params.angle, params.type, params.power, modelManager);

    obj->SetProgrammable();
    obj->SetMovable(std::move(motion), std::move(physics));

    return obj;
}

void CShielder::SetShieldRadius(float shieldRadius)
{
    m_shieldRadius = shieldRadius;
}

float CShielder::GetShieldRadius()
{
    return m_shieldRadius;
}

float CShielder::GetActiveShieldRadius()
{
    if (IsBackgroundTask())
    {
        CTaskShield* taskShield = dynamic_cast<CTaskShield*>(GetBackgroundTask());
        if (taskShield != nullptr)
        {
            return taskShield->GetActiveRadius();
        }
    }
    return 0.0f;
}


// TODO: Is shield radius saved somewhere? I can't find it

void CShielder::Read(CLevelParserLine* line)
{
    COldObject::Read(line);

    if( line->GetParam("bShieldActive")->AsBool(false) )
    {
        StartTaskShield(TSM_START);
    }
}

void CShielder::Write(CLevelParserLine* line)
{
    COldObject::Write(line);

    line->AddParam("bShieldActive", MakeUnique<CLevelParserParam>(IsBackgroundTask()));
}
