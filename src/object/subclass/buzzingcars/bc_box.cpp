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

#include "object/subclass/buzzingcars/bc_box.h"

#include "common/make_unique.h"

#include "graphics/engine/oldmodelmanager.h"
#include "graphics/engine/terrain.h"

#include "math/geometry.h"

#include "object/object_create_params.h"

#include "object/motion/motion.h"

#include "physics/physics.h"

CBCBox::CBCBox(int id, ObjectType type)
    : COldObject(id)
{
    SetType(type);
}

CBCBox::~CBCBox()
{}

std::unique_ptr<CBCBox> CBCBox::Create(
    const ObjectCreateParams& params,
    Gfx::COldModelManager* modelManager,
    Gfx::CEngine* engine)
{
    auto obj = MakeUnique<CBCBox>(params.id, params.type);

    obj->SetTeam(params.team);

    bool bPlumb = params.bPlumb;

    int rank;
    char text[100]; // TODO (krzys_h): C strings...

    if ( params.type == OBJECT_BOX1 )
    {
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        sprintf(text, "buzzingcars/box1%d.mod", rand()%6);
        modelManager->AddModelReference(text, false, rank, params.team);
        obj->SetPartPosition(0, params.pos);
        obj->SetPartRotationY(0, params.angle);
        obj->GetCharacter()->height = 4.0f;

#if 0
        obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 0.0f, 0.0f), 4.0f, SOUND_BOUMm, 0.45f));
#else
        obj->AddCrashLine(CrashLine(Math::Point(-4.0f, -4.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 4.0f, -4.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 4.0f,  4.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-4.0f,  4.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-4.0f, -4.0f), SOUND_BOUMm, 0.45f));
#endif
        obj->CreateShadowCircle(5.0f, 1.0f);
    }

    if ( params.type == OBJECT_BOX2 )
    {
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        sprintf(text, "buzzingcars/box1%d.mod", rand()%6);
        modelManager->AddModelReference(text, false, rank, params.team);
        obj->SetPartPosition(0, params.pos);
        obj->SetPartRotationY(0, params.angle);
        obj->GetCharacter()->height = 4.0f;

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        sprintf(text, "buzzingcars/box1%d.mod", rand()%6);
        modelManager->AddModelReference(text, false, rank, params.team);
        obj->SetPartPosition(1, Math::Vector((Math::Rand()-0.5f)*4.0f, 8.0f, (Math::Rand()-0.5f)*4.0f));
        obj->SetPartRotationY(1, Math::Rand()*Math::PI);

#if 0
        obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 0.0f, 0.0f), 4.0f, SOUND_BOUMm, 0.45f));
#else
        obj->AddCrashLine(CrashLine(Math::Point(-4.0f, -4.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 4.0f, -4.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 4.0f,  4.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-4.0f,  4.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-4.0f, -4.0f), SOUND_BOUMm, 0.45f));
#endif
        obj->CreateShadowCircle(5.0f, 1.0f);
        obj->GetCharacter()->mass = 4000.0f;
    }

    if ( params.type == OBJECT_BOX3 )
    {
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        sprintf(text, "buzzingcars/box1%d.mod", rand()%6);
        modelManager->AddModelReference(text, false, rank, params.team);
        obj->SetPartPosition(0, params.pos);
        obj->SetPartRotationY(0, params.angle);
        obj->GetCharacter()->height = 4.0f;

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        sprintf(text, "buzzingcars/box1%d.mod", rand()%6);
        modelManager->AddModelReference(text, false, rank, params.team);
        obj->SetPartPosition(1, Math::Vector((Math::Rand()-0.5f)*3.0f, 8.0f, (Math::Rand()-0.5f)*3.0f));
        obj->SetPartRotationY(1, Math::Rand()*Math::PI);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(2, rank);
        obj->SetObjectParent(2, 1);
        sprintf(text, "buzzingcars/box1%d.mod", rand()%6);
        modelManager->AddModelReference(text, false, rank, params.team);
        obj->SetPartPosition(2, Math::Vector((Math::Rand()-0.5f)*2.0f, 8.0f, (Math::Rand()-0.5f)*2.0f));
        obj->SetPartRotationY(2, Math::Rand()*Math::PI);

#if 0
        obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 0.0f, 0.0f), 4.0f, SOUND_BOUMm, 0.45f));
#else
        obj->AddCrashLine(CrashLine(Math::Point(-4.0f, -4.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 4.0f, -4.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 4.0f,  4.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-4.0f,  4.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-4.0f, -4.0f), SOUND_BOUMm, 0.45f));
#endif
        obj->CreateShadowCircle(5.0f, 1.0f);
        obj->GetCharacter()->mass = 6000.0f;
    }

    if ( params.type == OBJECT_BOX4 )
    {
        auto physics = MakeUnique<CPhysics>(obj.get());
        physics->SetType(TYPE_MASS);

        obj->GetCharacter()->mass = 2000.0f;

//?		physics->SetLinMotion(MO_STOACCEL, Math::Vector(20.0f, 50.0f, 20.0f));
        physics->SetLinMotion(MO_STOACCEL, Math::Vector( 5.0f, 50.0f,  5.0f));
//?		physics->SetLinMotionY(MO_MOTSPEED, -200.0f);
        physics->SetLinMotion(MO_TERSLIDE, Math::Vector( 1.0f,  1.0f,  1.0f));
        physics->SetLinMotion(MO_TERFORCE, Math::Vector(50.0f, 50.0f, 50.0f));
        physics->SetCirMotion(MO_MOTACCEL, Math::Vector( 1.0f,  1.0f,  1.0f));
        obj->SetMovable(nullptr, std::move(physics));

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        sprintf(text, "buzzingcars/box1%d.mod", rand()%6);
        modelManager->AddModelReference(text, false, rank, params.team);
        obj->SetPartPosition(0, params.pos);
        obj->SetPartRotationY(0, params.angle);
        obj->GetCharacter()->height = 4.0f;

#if 0
        obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 0.0f, 0.0f), 4.0f, SOUND_BOUMm, 0.45f));
#else
        obj->AddCrashLine(CrashLine(Math::Point(-4.0f, -4.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 4.0f, -4.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 4.0f,  4.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-4.0f,  4.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-4.0f, -4.0f), SOUND_BOUMm, 0.45f));
#endif
        obj->CreateShadowCircle(5.0f, 1.0f);
    }

    if ( params.type == OBJECT_BOX5 )
    {
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/box5.mod", false, rank, params.team);
        obj->SetPartPosition(0, params.pos);
        obj->SetPartRotationY(0, params.angle);

        obj->AddCrashLine(CrashLine(Math::Point(-4.0f, -4.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 4.0f, -4.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 4.0f,  4.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-4.0f,  4.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-4.0f, -4.0f), SOUND_BOUMm, 0.45f));
        obj->CreateShadowCircle(5.0f, 1.0f);
    }

    if ( params.type == OBJECT_BOX6 )
    {
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/box6.mod", false, rank, params.team);
        obj->SetPartPosition(0, params.pos);
        obj->SetPartRotationY(0, params.angle);

        obj->AddCrashLine(CrashLine(Math::Point(-4.0f, -4.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 4.0f, -4.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 4.0f,  4.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-4.0f,  4.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-4.0f, -4.0f), SOUND_BOUMm, 0.45f));
        obj->CreateShadowCircle(5.0f, 1.0f);
    }

    if ( params.type == OBJECT_TOYS1 )
    {
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/toys1.mod", false, rank, params.team);
        obj->SetPartPosition(0, params.pos);
        obj->SetPartRotationY(0, params.angle);

        obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 2.0f, 0.0f), 2.2f, SOUND_CHOCo, 0.45f));
        obj->CreateShadowCircle(3.0f, 1.0f);
    }

    if ( params.type == OBJECT_TOYS2 )
    {
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/toys2.mod", false, rank, params.team);
        obj->SetPartPosition(0, params.pos);
        obj->SetPartRotationY(0, params.angle);

        obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 2.0f, 0.0f), 2.2f, SOUND_CHOCo, 0.45f));
        obj->CreateShadowCircle(3.0f, 1.0f);
    }

    if ( params.type == OBJECT_TOYS3 )
    {
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/toys3.mod", false, rank, params.team);
        obj->SetPartPosition(0, params.pos);
        obj->SetPartRotationY(0, params.angle);

        obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 2.0f, 0.0f), 2.2f, SOUND_CHOCo, 0.45f));
        obj->CreateShadowCircle(3.0f, 1.0f);
    }

    if ( params.type == OBJECT_TOYS4 )
    {
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/toys4.mod", false, rank, params.team);
        obj->SetPartPosition(0, params.pos);
        obj->SetPartRotationY(0, params.angle);

        obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 2.0f, 0.0f), 2.2f, SOUND_CHOCo, 0.45f));
        obj->CreateShadowCircle(3.0f, 1.0f);
    }

    if ( params.type == OBJECT_TOYS5 )
    {
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/toys5.mod", false, rank, params.team);
        obj->SetPartPosition(0, params.pos);
        obj->SetPartRotationY(0, params.angle);

        obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 2.0f, 0.0f), 2.2f, SOUND_CHOCo, 0.45f));
        obj->CreateShadowCircle(3.0f, 1.0f);
    }

    obj->SetFloorHeight(0.0f);

    engine->LoadAllTextures();
    if ( !bPlumb )  obj->FloorAdjust();

    Math::Vector pos = obj->GetPosition();
    pos.y += params.height;
    obj->SetPosition(pos);  // to display the shadows immediately

    return obj;
}
