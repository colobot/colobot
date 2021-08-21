/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2020, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "object/subclass/base_robot.h"

#include "common/make_unique.h"

#include "graphics/engine/oldmodelmanager.h"

#include "object/object_create_params.h"

#include "object/motion/motionhuman.h"
#include "object/motion/motionlevelcontroller.h"
#include "object/motion/motiontoto.h"
#include "object/motion/motionvehicle.h"

#include "physics/physics.h"


CBaseRobot::CBaseRobot(int id, ObjectType type)
    : CBaseVehicle(id, type)
{}

CBaseRobot::~CBaseRobot()
{}

std::unique_ptr<CBaseRobot> CBaseRobot::Create(
    const ObjectCreateParams& params,
    Gfx::COldModelManager* modelManager,
    Gfx::CEngine* engine)
{
    auto obj = MakeUnique<CBaseRobot>(params.id, params.type);

    obj->SetOption(params.option);
    obj->SetTeam(params.team);

    if ( params.type == OBJECT_TOTO )
    {
        auto motion = MakeUnique<CMotionToto>(obj.get());
        motion->Create(params.pos, params.angle, params.type, 1.0f, modelManager);
        obj->SetMovable(std::move(motion), nullptr);
        return obj;
    }

    if ( params.type == OBJECT_HUMAN ||
         params.type == OBJECT_TECH  )
    {
        obj->SetTrainer(false);
    }
    else
    {
        obj->SetTrainer(params.trainer || obj->GetPlusTrainer());
    }

    obj->SetToy(params.toy);

    auto physics = MakeUnique<CPhysics>(obj.get());

    std::unique_ptr<CMotion> motion;
    if ( params.type == OBJECT_HUMAN ||
         params.type == OBJECT_TECH  )
    {
        motion = MakeUnique<CMotionHuman>(obj.get());
    }
    else if ( params.type == OBJECT_CONTROLLER )
    {
        motion = MakeUnique<CMotionLevelController>(obj.get());
    }
    else
    {
        motion = MakeUnique<CMotionVehicle>(obj.get());
    }

    motion->SetPhysics(physics.get());
    physics->SetMotion(motion.get());

    motion->Create(params.pos, params.angle, params.type, params.power, modelManager);

    obj->SetProgrammable();
    obj->SetMovable(std::move(motion), std::move(physics));

    return obj;
}
