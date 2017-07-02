/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2016, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "object/subclass/buzzingcars/bc_bot.h"

#include "common/make_unique.h"

#include "graphics/engine/oldmodelmanager.h"
#include "graphics/engine/terrain.h"

#include "math/geometry.h"

#include "object/object_create_params.h"

#include "object/motion/buzzingcars/motionbot.h"

#include "physics/physics.h"


CBCBot::CBCBot(int id, ObjectType type)
    : COldObject(id)
{
    SetType(type);
}

CBCBot::~CBCBot()
{}

std::unique_ptr<CBCBot> CBCBot::Create(
    const ObjectCreateParams& params,
    Gfx::COldModelManager* modelManager,
    Gfx::CEngine* engine)
{
    auto obj = MakeUnique<CBCBot>(params.id, params.type);

    obj->SetOption(params.option);
    obj->SetTeam(params.team);

    if ( params.type == OBJECT_CARROT  ||
         params.type == OBJECT_STARTER ||
         params.type == OBJECT_WALKER  ||
         params.type == OBJECT_CRAZY   ||
         params.type == OBJECT_GUIDE   ||
         params.type == OBJECT_EVIL1   ||
         params.type == OBJECT_EVIL3   ||
         params.type == OBJECT_EVIL4   ||
         params.type == OBJECT_EVIL5   )
    {
        auto physics = MakeUnique<CPhysics>(obj.get());
        std::unique_ptr<CMotion> motion = MakeUnique<CMotionBot>(obj.get());

        motion->SetPhysics(physics.get());
        physics->SetMotion(motion.get());

        motion->Create(params.pos, params.angle, params.type, params.power, modelManager);

        obj->SetProgrammable();
        obj->SetMovable(std::move(motion), std::move(physics));
    }
    else
    {
        std::unique_ptr<CMotion> motion = MakeUnique<CMotionBot>(obj.get());
        motion->Create(params.pos, params.angle, params.type, params.power, modelManager);
        obj->SetMovable(std::move(motion), nullptr);
    }

    return obj;
}
