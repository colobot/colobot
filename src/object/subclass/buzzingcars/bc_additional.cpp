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

#include "object/subclass/buzzingcars/bc_additional.h"

#include "common/make_unique.h"

#include "graphics/engine/oldmodelmanager.h"
#include "graphics/engine/terrain.h"

#include "math/geometry.h"

#include "object/object_create_params.h"

#include "object/auto/buzzingcars/additional/autoblitzer.h"
#include "object/auto/buzzingcars/additional/autocompass.h"
#include "object/auto/buzzingcars/additional/autofire.h"
#include "object/auto/buzzingcars/additional/autohelico.h"
#include "object/auto/buzzingcars/additional/autohook.h"

CBCAdditional::CBCAdditional(int id, ObjectType type)
    : COldObject(id)
{
    SetType(type);
}

CBCAdditional::~CBCAdditional()
{}

std::unique_ptr<CBCAdditional> CBCAdditional::Create(
    const ObjectCreateParams& params,
    Gfx::COldModelManager* modelManager,
    Gfx::CEngine* engine)
{
    auto obj = MakeUnique<CBCAdditional>(params.id, params.type);

    obj->SetTeam(params.team);

    bool bPlumb = params.bPlumb;

    int rank = engine->CreateObject();
    engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);  // c'est un objet fixe
    obj->SetObjectRank(0, rank);
    obj->GetCharacter()->mass = 1000.0f;

    if ( params.type == OBJECT_FIRE )
    {
        modelManager->AddModelReference("buzzingcars/fire1.mod", false, rank, params.team);
        obj->SetPartPosition(0, params.pos);
        obj->SetPartRotationY(0, params.angle);
        obj->SetPartScale(0, params.zoom);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        modelManager->AddModelReference("buzzingcars/fire2.mod", false, rank, params.team);
        obj->SetPartPosition(1, Math::Vector(0.0f, 4.5f, 0.0f));

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(2, rank);
        obj->SetObjectParent(2, 1);
        modelManager->AddModelReference("buzzingcars/fire3.mod", false, rank, params.team);
        obj->SetPartPosition(2, Math::Vector(0.0f, 2.0f, 0.0f));
        obj->SetPartRotationZ(2, 20.0f*Math::PI/180.0f);
    }

    if ( params.type == OBJECT_HELICO )
    {
        modelManager->AddModelReference("buzzingcars/helico1.mod", false, rank, params.team);
        obj->SetPartPosition(0, params.pos);
        obj->SetPartRotationY(0, params.angle);
        obj->SetPartScale(0, params.zoom);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        modelManager->AddModelReference("buzzingcars/helico2.mod", false, rank, params.team);
        obj->SetPartPosition(1, Math::Vector(0.0f, 5.0f, 0.0f));
    }

    if ( params.type == OBJECT_COMPASS )
    {
        modelManager->AddModelReference("buzzingcars/compass1.mod", false, rank, params.team);
        obj->SetPartPosition(0, params.pos);
        obj->SetPartRotationY(0, params.angle);
        obj->SetPartScale(0, params.zoom);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        modelManager->AddModelReference("buzzingcars/compass2.mod", false, rank, params.team);
        obj->SetPartPosition(1, Math::Vector(0.0f, 5.0f, 0.0f));
    }

    if ( params.type == OBJECT_BLITZER )
    {
        modelManager->AddModelReference("buzzingcars/blitzer.mod", false, rank, params.team);
        obj->SetPartPosition(0, params.pos);
        obj->SetPartRotationY(0, params.angle);
        obj->SetPartScale(0, params.zoom);
    }

    if ( params.type == OBJECT_HOOK )
    {
        modelManager->AddModelReference("buzzingcars/hook1.mod", false, rank, params.team);
        obj->SetPartPosition(0, params.pos);
        obj->SetPartRotationY(0, params.angle);
        obj->SetPartScale(0, params.zoom);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        modelManager->AddModelReference("buzzingcars/hook2.mod", false, rank, params.team);
        obj->SetPartPosition(1, Math::Vector(0.0f, 4.6f, 0.0f));
        obj->SetPartRotationY(1, Math::PI);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(2, rank);
        obj->SetObjectParent(2, 1);
        modelManager->AddModelReference("buzzingcars/hook3.mod", false, rank, params.team);
        obj->SetPartPosition(2, Math::Vector(-8.0f, 0.9f, 0.0f));

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(3, rank);
        obj->SetObjectParent(3, 2);
        modelManager->AddModelReference("buzzingcars/hook4.mod", false, rank, params.team);
        obj->SetPartPosition(3, Math::Vector(0.0f, -2.1f, 0.0f));
    }

    if ( params.type == OBJECT_AQUA )
    {
        modelManager->AddModelReference("buzzingcars/aqua1.mod", false, rank, params.team);
        obj->SetPartPosition(0, params.pos);
        obj->SetPartRotationY(0, params.angle);
        obj->SetPartScale(0, params.zoom);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        modelManager->AddModelReference("buzzingcars/aqua2.mod", false, rank, params.team);
        obj->SetPartPosition(1, Math::Vector(0.0f, 0.0f, 0.0f));

        obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 1.0f, 0.0f), 2.0f, SOUND_BOUMm, 0.45f));
        obj->CreateShadowCircle(2.0f, 0.8f);
    }

    obj->SetFloorHeight(0.0f);

    std::unique_ptr<CAuto> objAuto;
    if ( params.type == OBJECT_FIRE )
    {
        objAuto = MakeUnique<CAutoFire>(obj.get());
    }
    if ( params.type == OBJECT_HELICO )
    {
        objAuto = MakeUnique<CAutoHelico>(obj.get());
    }
    if ( params.type == OBJECT_COMPASS )
    {
        objAuto = MakeUnique<CAutoCompass>(obj.get());
    }
    if ( params.type == OBJECT_BLITZER )
    {
        objAuto = MakeUnique<CAutoBlitzer>(obj.get());
    }
    if ( params.type == OBJECT_HOOK )
    {
        objAuto = MakeUnique<CAutoHook>(obj.get());
    }

    if (objAuto != nullptr)
    {
        objAuto->Init();
        obj->SetAuto(std::move(objAuto));
    }

    engine->LoadAllTextures();
    if ( !bPlumb )  obj->FloorAdjust();

    Math::Vector pos = obj->GetPosition();
    pos.y += params.height;
    obj->SetPosition(pos);  // to display the shadows immediately

    return obj;
}
