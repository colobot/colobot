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

#include "object/subclass/buzzingcars/bc_car.h"

#include "common/make_unique.h"

#include "graphics/engine/oldmodelmanager.h"
#include "graphics/engine/terrain.h"

#include "math/geometry.h"

#include "object/object_create_params.h"

#include "object/motion/buzzingcars/motioncar.h"

#include "physics/physics.h"


CBCCar::CBCCar(int id, ObjectType type)
    : COldObject(id)
{
    SetType(type);
}

CBCCar::~CBCCar()
{}

std::unique_ptr<CBCCar> CBCCar::Create(
    const ObjectCreateParams& params,
    Gfx::COldModelManager* modelManager,
    Gfx::CEngine* engine)
{
    auto obj = MakeUnique<CBCCar>(params.id, params.type);

    obj->SetOption(params.option);
    obj->SetTeam(params.team);
    obj->SetTrainer(params.trainer);
    obj->SetModel(params.model);
    obj->SetSubModel(params.subModel);

    /* TODO (krzys_h):
    if ( m_subModel == 1 )
	{
		sprintf(actualTex, "car%.2d.tga", m_model);
		sprintf(futureTex, "car%.2db.tga", m_model);
		m_engine->SetReplaceTex(actualTex, futureTex);
	}
	if ( m_subModel == 2 )
	{
		sprintf(actualTex, "car%.2d.tga", m_model);
		sprintf(futureTex, "car%.2dc.tga", m_model);
		m_engine->SetReplaceTex(actualTex, futureTex);
	}
	if ( m_subModel == 3 )
	{
		sprintf(actualTex, "car%.2d.tga", m_model);
		sprintf(futureTex, "car%.2dd.tga", m_model);
		m_engine->SetReplaceTex(actualTex, futureTex);
	}
    */

    auto physics = MakeUnique<CPhysics>(obj.get());
    std::unique_ptr<CMotion> motion = MakeUnique<CMotionCar>(obj.get());

    motion->SetPhysics(physics.get());
    physics->SetMotion(motion.get());

    motion->Create(params.pos, params.angle, params.type, params.power, modelManager);

    obj->SetProgrammable();
    obj->SetMovable(std::move(motion), std::move(physics));

    obj->m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Controllable)] = true;
    
    obj->m_implementedInterfaces[static_cast<int>(ObjectInterfaceType::Damageable)] = true; // TODO (krzys_h): hardcoding, yey!

    return obj;
}
