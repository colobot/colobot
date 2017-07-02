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

#include "object/subclass/buzzingcars/bc_stone.h"

#include "common/make_unique.h"

#include "graphics/engine/oldmodelmanager.h"
#include "graphics/engine/terrain.h"

#include "math/geometry.h"

#include "object/object_create_params.h"

CBCStone::CBCStone(int id, ObjectType type)
    : COldObject(id)
{
    SetType(type);
}

CBCStone::~CBCStone()
{}

std::unique_ptr<CBCStone> CBCStone::Create(
    const ObjectCreateParams& params,
    Gfx::COldModelManager* modelManager,
    Gfx::CEngine* engine)
{
    auto obj = MakeUnique<CBCStone>(params.id, params.type);

    obj->SetTeam(params.team);

    bool bPlumb = params.bPlumb;

    int rank;

    if ( params.type == OBJECT_STONE1 )  // cube 1
    {
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/stone1.mod", false, rank, params.team);
        obj->SetPartScale(0, params.zoom);
        obj->SetPartPosition(0, params.pos);
        obj->SetPartRotationY(0, params.angle);

#if 1
        obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 3.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f));
#else
        obj->AddCrashLine(CrashLine(Math::Point(-6.0f, -6.0f), SOUND_BOUMm, 0.45f));
		obj->AddCrashLine(CrashLine(Math::Point( 6.0f, -6.0f), SOUND_BOUMm, 0.45f));
		obj->AddCrashLine(CrashLine(Math::Point( 6.0f,  6.0f), SOUND_BOUMm, 0.45f));
		obj->AddCrashLine(CrashLine(Math::Point(-6.0f,  6.0f), SOUND_BOUMm, 0.45f));
		obj->AddCrashLine(CrashLine(Math::Point(-6.0f, -6.0f), SOUND_BOUMm, 0.45f));
#endif
        obj->CreateShadowCircle(8.0f, 1.0f);
    }

    if ( params.type == OBJECT_STONE2 )  // cube 2
    {
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/stone2.mod", false, rank, params.team);
        obj->SetPartScale(0, params.zoom);
        obj->SetPartPosition(0, params.pos);
        obj->SetPartRotationY(0, params.angle);

#if 1
        obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 3.0f, 0.0f), 8.0f, SOUND_BOUMm, 0.45f));
#else
        obj->AddCrashLine(CrashLine(Math::Point(-8.0f, -8.0f), SOUND_BOUMm, 0.45f));
		obj->AddCrashLine(CrashLine(Math::Point( 8.0f, -8.0f), SOUND_BOUMm, 0.45f));
		obj->AddCrashLine(CrashLine(Math::Point( 8.0f,  8.0f), SOUND_BOUMm, 0.45f));
		obj->AddCrashLine(CrashLine(Math::Point(-8.0f,  8.0f), SOUND_BOUMm, 0.45f));
		obj->AddCrashLine(CrashLine(Math::Point(-8.0f, -8.0f), SOUND_BOUMm, 0.45f));
#endif
        obj->CreateShadowCircle(10.0f, 1.0f);
    }

    if ( params.type == OBJECT_STONE3 )  // cube 3
    {
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/stone3.mod", false, rank, params.team);
        obj->SetPartScale(0, params.zoom);
        obj->SetPartPosition(0, params.pos);
        obj->SetPartRotationY(0, params.angle);

#if 1
        obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 3.0f, 0.0f), 10.0f, SOUND_BOUMm, 0.45f));
#else
        obj->AddCrashLine(CrashLine(Math::Point(-9.5f, -9.5f), SOUND_BOUMm, 0.45f));
		obj->AddCrashLine(CrashLine(Math::Point( 9.5f, -9.5f), SOUND_BOUMm, 0.45f));
		obj->AddCrashLine(CrashLine(Math::Point( 9.5f,  9.5f), SOUND_BOUMm, 0.45f));
		obj->AddCrashLine(CrashLine(Math::Point(-9.5f,  9.5f), SOUND_BOUMm, 0.45f));
		obj->AddCrashLine(CrashLine(Math::Point(-9.5f, -9.5f), SOUND_BOUMm, 0.45f));
#endif
        obj->CreateShadowCircle(12.0f, 1.0f);
    }

    if ( params.type == OBJECT_STONE4 )  // cube 4
    {
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/stone4.mod", false, rank, params.team);
        obj->SetPartScale(0, params.zoom);
        obj->SetPartPosition(0, params.pos);
        obj->SetPartRotationY(0, params.angle);

#if 1
        obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 3.0f, 0.0f), 15.0f, SOUND_BOUMm, 0.45f));
#else
        obj->AddCrashLine(CrashLine(Math::Point(-14.0f, -14.0f), SOUND_BOUMm, 0.45f));
		obj->AddCrashLine(CrashLine(Math::Point( 14.0f, -14.0f), SOUND_BOUMm, 0.45f));
		obj->AddCrashLine(CrashLine(Math::Point( 14.0f,  14.0f), SOUND_BOUMm, 0.45f));
		obj->AddCrashLine(CrashLine(Math::Point(-14.0f,  14.0f), SOUND_BOUMm, 0.45f));
		obj->AddCrashLine(CrashLine(Math::Point(-14.0f, -14.0f), SOUND_BOUMm, 0.45f));
#endif
        obj->CreateShadowCircle(18.0f, 1.0f);
    }

    if ( params.type == OBJECT_STONE5 )  // pilier h=30
    {
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/stone5.mod", false, rank, params.team);
        obj->SetPartScale(0, params.zoom);
        obj->SetPartPosition(0, params.pos);
        obj->SetPartRotationY(0, params.angle);

#if 1
        obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 3.0f, 0.0f), 8.0f, SOUND_BOUMm, 0.45f));
#else
        obj->AddCrashLine(CrashLine(Math::Point(-8.0f, -8.0f), SOUND_BOUMm, 0.45f));
		obj->AddCrashLine(CrashLine(Math::Point( 8.0f, -8.0f), SOUND_BOUMm, 0.45f));
		obj->AddCrashLine(CrashLine(Math::Point( 8.0f,  8.0f), SOUND_BOUMm, 0.45f));
		obj->AddCrashLine(CrashLine(Math::Point(-8.0f,  8.0f), SOUND_BOUMm, 0.45f));
		obj->AddCrashLine(CrashLine(Math::Point(-8.0f, -8.0f), SOUND_BOUMm, 0.45f));
#endif
        obj->CreateShadowCircle(10.0f, 1.0f);
    }

    if ( params.type == OBJECT_STONE6 )  // dalle haute pour piliers espacés d=60
    {
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/stone6.mod", false, rank, params.team);
        obj->SetPartScale(0, params.zoom);
        obj->SetPartPosition(0, params.pos);
        obj->SetPartRotationY(0, params.angle);
    }

    if ( params.type == OBJECT_CROSS1 )
    {
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/cross1.mod", false, rank, params.team);
        obj->SetPartScale(0, params.zoom);
        obj->SetPartPosition(0, params.pos);
        obj->SetPartRotationY(0, params.angle);

        obj->AddCrashSphere(CrashSphere(Math::Vector( 33.5f, 3.0f,  33.5f), 3.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 33.5f, 3.0f, -33.5f), 3.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-33.5f, 3.0f,  33.5f), 3.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-33.5f, 3.0f, -33.5f), 3.5f, SOUND_BOUMm, 0.45f));
    }

    obj->SetFloorHeight(0.0f);

    engine->LoadAllTextures();
    if ( !bPlumb )  obj->FloorAdjust();

    Math::Vector pos = obj->GetPosition();
    pos.y += params.height;
    obj->SetPosition(pos);  // to display the shadows immediately

    return obj;
}
