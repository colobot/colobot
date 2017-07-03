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

#include "object/subclass/buzzingcars/bc_home.h"

#include "common/make_unique.h"

#include "graphics/engine/oldmodelmanager.h"
#include "graphics/engine/terrain.h"

#include "math/geometry.h"

#include "object/object_create_params.h"

CBCHome::CBCHome(int id, ObjectType type)
    : COldObject(id)
{
    SetType(type);
}

CBCHome::~CBCHome()
{}

std::unique_ptr<CBCHome> CBCHome::Create(
    const ObjectCreateParams& params,
    Gfx::COldModelManager* modelManager,
    Gfx::CEngine* engine)
{
    auto obj = MakeUnique<CBCHome>(params.id, params.type);

    obj->SetTeam(params.team);

    bool bPlumb = params.bPlumb;

    int rank;

    if ( params.type == OBJECT_HOME1 )
    {
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/home1.mod", false, rank, params.team);
        obj->SetPartPosition(0, params.pos);
        obj->SetPartRotationY(0, params.angle);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        modelManager->AddModelReference("buzzingcars/home102.mod", false, rank, params.team);
        obj->SetPartPosition(1, Math::Vector(8.5f, 14.0f, 8.5f));
        obj->SetPartRotationY(1, Math::PI*1.75f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(2, rank);
        obj->SetObjectParent(2, 0);
        modelManager->AddModelReference("buzzingcars/home102.mod", false, rank, params.team);
        obj->SetPartPosition(2, Math::Vector(-8.5f, 14.0f, 8.5f));
        obj->SetPartRotationY(2, Math::PI*1.25f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(3, rank);
        obj->SetObjectParent(3, 0);
        modelManager->AddModelReference("buzzingcars/home102.mod", false, rank, params.team);
        obj->SetPartPosition(3, Math::Vector(8.5f, 14.0f, -8.5f));
        obj->SetPartRotationY(3, Math::PI*0.25f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(4, rank);
        obj->SetObjectParent(4, 0);
        modelManager->AddModelReference("buzzingcars/home102.mod", false, rank, params.team);
        obj->SetPartPosition(4, Math::Vector(-8.5f, 14.0f, -8.5f));
        obj->SetPartRotationY(4, Math::PI*0.75f);

#if 0
        obj->AddCrashSphere(CrashSphere(Math::Vector( 0.0f, 2.0f,  0.0f), 10.0f, SOUND_BOUMm, 0.25f));
		obj->AddCrashSphere(CrashSphere(Math::Vector(-6.0f, 2.0f,  6.0f),  4.0f, SOUND_BOUMm, 0.45f));
		obj->AddCrashSphere(CrashSphere(Math::Vector(-6.0f, 2.0f,  0.0f),  4.0f, SOUND_BOUMm, 0.45f));
		obj->AddCrashSphere(CrashSphere(Math::Vector(-6.0f, 2.0f, -6.0f),  4.0f, SOUND_BOUMm, 0.45f));
		obj->AddCrashSphere(CrashSphere(Math::Vector( 0.0f, 2.0f,  6.0f),  4.0f, SOUND_BOUMm, 0.45f));
		obj->AddCrashSphere(CrashSphere(Math::Vector( 0.0f, 2.0f,  0.0f),  4.0f, SOUND_BOUMm, 0.45f));
		obj->AddCrashSphere(CrashSphere(Math::Vector( 0.0f, 2.0f, -6.0f),  4.0f, SOUND_BOUMm, 0.45f));
		obj->AddCrashSphere(CrashSphere(Math::Vector( 6.0f, 2.0f,  6.0f),  4.0f, SOUND_BOUMm, 0.45f));
		obj->AddCrashSphere(CrashSphere(Math::Vector( 6.0f, 2.0f,  0.0f),  4.0f, SOUND_BOUMm, 0.45f));
		obj->AddCrashSphere(CrashSphere(Math::Vector( 6.0f, 2.0f, -6.0f),  4.0f, SOUND_BOUMm, 0.45f));
#else
        obj->AddCrashLine(CrashLine(Math::Point(-10.0f, -10.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 10.0f, -10.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 10.0f,  10.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-10.0f,  10.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-10.0f, -10.0f), SOUND_BOUMm, 0.45f));
#endif
        obj->CreateShadowCircle(16.0f, 0.5f);
    }

    if ( params.type == OBJECT_HOME2 )
    {
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/home2.mod", false, rank, params.team);
        obj->SetPartPosition(0, params.pos);
        obj->SetPartRotationY(0, params.angle);

#if 0
        obj->AddCrashSphere(CrashSphere(Math::Vector( 0.0f, 2.0f,  0.0f), 10.0f, SOUND_BOUMm, 0.25f));
		obj->AddCrashSphere(CrashSphere(Math::Vector(-6.0f, 2.0f,  6.0f),  4.0f, SOUND_BOUMm, 0.45f));
		obj->AddCrashSphere(CrashSphere(Math::Vector(-6.0f, 2.0f,  0.0f),  4.0f, SOUND_BOUMm, 0.45f));
		obj->AddCrashSphere(CrashSphere(Math::Vector(-6.0f, 2.0f, -6.0f),  4.0f, SOUND_BOUMm, 0.45f));
		obj->AddCrashSphere(CrashSphere(Math::Vector( 0.0f, 2.0f,  6.0f),  4.0f, SOUND_BOUMm, 0.45f));
		obj->AddCrashSphere(CrashSphere(Math::Vector( 0.0f, 2.0f,  0.0f),  4.0f, SOUND_BOUMm, 0.45f));
		obj->AddCrashSphere(CrashSphere(Math::Vector( 0.0f, 2.0f, -6.0f),  4.0f, SOUND_BOUMm, 0.45f));
		obj->AddCrashSphere(CrashSphere(Math::Vector( 6.0f, 2.0f,  6.0f),  4.0f, SOUND_BOUMm, 0.45f));
		obj->AddCrashSphere(CrashSphere(Math::Vector( 6.0f, 2.0f,  0.0f),  4.0f, SOUND_BOUMm, 0.45f));
		obj->AddCrashSphere(CrashSphere(Math::Vector( 6.0f, 2.0f, -6.0f),  4.0f, SOUND_BOUMm, 0.45f));
#else
        obj->AddCrashLine(CrashLine(Math::Point(-10.0f, -10.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 10.0f, -10.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 10.0f,  10.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-10.0f,  10.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-10.0f, -10.0f), SOUND_BOUMm, 0.45f));
#endif
        obj->CreateShadowCircle(16.0f, 0.5f);
    }

    if ( params.type == OBJECT_HOME3 )
    {
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/home3.mod", false, rank, params.team);
        obj->SetPartPosition(0, params.pos);
        obj->SetPartRotationY(0, params.angle);

#if 0
        obj->AddCrashSphere(CrashSphere(Math::Vector( 0.0f, 2.0f,  0.0f), 10.0f, SOUND_BOUMm, 0.25f));
		obj->AddCrashSphere(CrashSphere(Math::Vector(-6.0f, 2.0f,  6.0f),  4.0f, SOUND_BOUMm, 0.45f));
		obj->AddCrashSphere(CrashSphere(Math::Vector(-6.0f, 2.0f,  0.0f),  4.0f, SOUND_BOUMm, 0.45f));
		obj->AddCrashSphere(CrashSphere(Math::Vector(-6.0f, 2.0f, -6.0f),  4.0f, SOUND_BOUMm, 0.45f));
		obj->AddCrashSphere(CrashSphere(Math::Vector( 0.0f, 2.0f,  6.0f),  4.0f, SOUND_BOUMm, 0.45f));
		obj->AddCrashSphere(CrashSphere(Math::Vector( 0.0f, 2.0f,  0.0f),  4.0f, SOUND_BOUMm, 0.45f));
		obj->AddCrashSphere(CrashSphere(Math::Vector( 0.0f, 2.0f, -6.0f),  4.0f, SOUND_BOUMm, 0.45f));
		obj->AddCrashSphere(CrashSphere(Math::Vector( 6.0f, 2.0f,  6.0f),  4.0f, SOUND_BOUMm, 0.45f));
		obj->AddCrashSphere(CrashSphere(Math::Vector( 6.0f, 2.0f,  0.0f),  4.0f, SOUND_BOUMm, 0.45f));
		obj->AddCrashSphere(CrashSphere(Math::Vector( 6.0f, 2.0f, -6.0f),  4.0f, SOUND_BOUMm, 0.45f));
#else
        obj->AddCrashLine(CrashLine(Math::Point(-9.5f, -9.5f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 9.5f, -9.5f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 9.5f,  9.5f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-9.5f,  9.5f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-9.5f, -9.5f), SOUND_BOUMm, 0.45f));
#endif
        obj->CreateShadowCircle(16.0f, 0.5f);
    }

    obj->SetFloorHeight(0.0f);

    engine->LoadAllTextures();
    if ( !bPlumb )  obj->FloorAdjust();

    Math::Vector pos = obj->GetPosition();
    pos.y += params.height;
    obj->SetPosition(pos);  // to display the shadows immediately

    return obj;
}
