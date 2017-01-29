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

#include "object/subclass/buzzingcars/bc_barrier.h"

#include "common/make_unique.h"

#include "graphics/engine/oldmodelmanager.h"
#include "graphics/engine/terrain.h"

#include "math/geometry.h"

#include "object/object_create_params.h"

#define CRASH_LINES_ARE_NOT_YET_IMPLEMENTED 1

CBCBarrier::CBCBarrier(int id, ObjectType type)
    : COldObject(id)
{
    SetType(type);
}

CBCBarrier::~CBCBarrier()
{}

std::unique_ptr<CBCBarrier> CBCBarrier::Create(
    const ObjectCreateParams& params,
    Gfx::COldModelManager* modelManager,
    Gfx::CEngine* engine)
{
    auto obj = MakeUnique<CBCBarrier>(params.id, params.type);

    obj->SetTeam(params.team);

    float height = params.height;
    bool bPlumb = params.bPlumb;

    if ( params.type == OBJECT_BARRIER0 )  // barričre //// jaune-noire 2.5m
    {
        int rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/barrier0.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);

        obj->AddCrashSphere(CrashSphere(Math::Vector( 3.5f, 3.0f, 0.0f), 0.7f, SOUND_CHOCo, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 0.0f, 3.0f, 0.0f), 0.7f, SOUND_CHOCo, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-3.5f, 3.0f, 0.0f), 0.7f, SOUND_CHOCo, 0.45f));
#if !CRASH_LINES_ARE_NOT_YET_IMPLEMENTED
        obj->SetCrashLineHeight(2.0f);
#endif
        obj->CreateShadowCircle(8.0f, 0.2f);
    }

    if ( params.type == OBJECT_BARRIER1 )  // barričre //// jaune-noire 5m
    {
        int rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/barrier1.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);

#if !CRASH_LINES_ARE_NOT_YET_IMPLEMENTED
        obj->CreateCrashLine(Math::Point(-10.0f, -1.0f), SOUND_CHOCo, 0.45f);
        obj->CreateCrashLine(Math::Point( 10.0f, -1.0f), SOUND_CHOCo, 0.45f);
        obj->CreateCrashLine(Math::Point( 10.0f,  1.0f), SOUND_CHOCo, 0.45f);
        obj->CreateCrashLine(Math::Point(-10.0f,  1.0f), SOUND_CHOCo, 0.45f);
        obj->CreateCrashLine(Math::Point(-10.0f, -1.0f), SOUND_CHOCo, 0.45f);
        obj->SetCrashLineHeight(2.0f);
#else
        // TODO: :(
#endif
// TODO (krzys_h):         obj->CreateShadowCircle(8.0f, 1.0f, D3DSHADOWBARRIER1);
    }

    if ( params.type == OBJECT_BARRIER2 )  // barričre >>>> rouge-grise 10m
    {
        int rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/barrier2.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);

#if !CRASH_LINES_ARE_NOT_YET_IMPLEMENTED
        obj->CreateCrashLine(Math::Point(-20.0f, -3.0f), SOUND_BOUMm, 0.45f);
        obj->CreateCrashLine(Math::Point( 20.0f, -3.0f), SOUND_BOUMm, 0.45f);
        obj->CreateCrashLine(Math::Point( 20.0f,  3.0f), SOUND_BOUMm, 0.45f);
        obj->CreateCrashLine(Math::Point(-20.0f,  3.0f), SOUND_BOUMm, 0.45f);
        obj->CreateCrashLine(Math::Point(-20.0f, -3.0f), SOUND_BOUMm, 0.45f);
        obj->SetCrashLineHeight(3.0f);
#else
        // TODO: :(
#endif
    }

    if ( params.type == OBJECT_BARRIER3 )  // barričre carrée
    {
        int rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/barrier3.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);

#if !CRASH_LINES_ARE_NOT_YET_IMPLEMENTED
        obj->CreateCrashLine(Math::Point(-10.0f, -10.0f), SOUND_BOUMm, 0.45f);
        obj->CreateCrashLine(Math::Point( 10.0f, -10.0f), SOUND_BOUMm, 0.45f);
        obj->CreateCrashLine(Math::Point( 10.0f,  10.0f), SOUND_BOUMm, 0.45f);
        obj->CreateCrashLine(Math::Point(-10.0f,  10.0f), SOUND_BOUMm, 0.45f);
        obj->CreateCrashLine(Math::Point(-10.0f, -10.0f), SOUND_BOUMm, 0.45f);
        obj->SetCrashLineHeight(3.0f);
        obj->CreateShadowCircle(16.0f, 1.0f);
#else
        // TODO: :(
#endif
    }

    if ( params.type == OBJECT_BARRIER4 )  // barričre en bois cassée 5m
    {
        int rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/barrier4.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);

#if !CRASH_LINES_ARE_NOT_YET_IMPLEMENTED
        obj->CreateCrashLine(Math::Point(-10.0f, -1.0f), SOUND_CHOCo, 0.45f);
        obj->CreateCrashLine(Math::Point( 10.0f, -1.0f), SOUND_CHOCo, 0.45f);
        obj->CreateCrashLine(Math::Point( 10.0f,  1.0f), SOUND_CHOCo, 0.45f);
        obj->CreateCrashLine(Math::Point(-10.0f,  1.0f), SOUND_CHOCo, 0.45f);
        obj->CreateCrashLine(Math::Point(-10.0f, -1.0f), SOUND_CHOCo, 0.45f);
        obj->SetCrashLineHeight(2.0f);
#else
        // TODO: :(
#endif
// TODO (krzys_h):         obj->CreateShadowCircle(8.0f, 1.0f, D3DSHADOWBARRIER1);
    }

    if ( params.type == OBJECT_BARRIER5 )  // barričre en bois droite 5m
    {
        int rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/barrier5.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);

#if !CRASH_LINES_ARE_NOT_YET_IMPLEMENTED
        obj->CreateCrashLine(Math::Point(-10.0f, -1.0f), SOUND_CHOCo, 0.45f);
        obj->CreateCrashLine(Math::Point( 10.0f, -1.0f), SOUND_CHOCo, 0.45f);
        obj->CreateCrashLine(Math::Point( 10.0f,  1.0f), SOUND_CHOCo, 0.45f);
        obj->CreateCrashLine(Math::Point(-10.0f,  1.0f), SOUND_CHOCo, 0.45f);
        obj->CreateCrashLine(Math::Point(-10.0f, -1.0f), SOUND_CHOCo, 0.45f);
        obj->SetCrashLineHeight(2.0f);
#else
        // TODO: :(
#endif
// TODO (krzys_h):         obj->CreateShadowCircle(8.0f, 1.0f, D3DSHADOWBARRIER1);
    }

    if ( params.type == OBJECT_BARRIER6 )  // barričre métal 5m
    {
        int rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/barrier6.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);

#if !CRASH_LINES_ARE_NOT_YET_IMPLEMENTED
        obj->CreateCrashLine(Math::Point(-10.0f, -1.0f), SOUND_CHOCm, 0.45f);
        obj->CreateCrashLine(Math::Point( 10.0f, -1.0f), SOUND_CHOCm, 0.45f);
        obj->CreateCrashLine(Math::Point( 10.0f,  1.0f), SOUND_CHOCm, 0.45f);
        obj->CreateCrashLine(Math::Point(-10.0f,  1.0f), SOUND_CHOCm, 0.45f);
        obj->CreateCrashLine(Math::Point(-10.0f, -1.0f), SOUND_CHOCm, 0.45f);
        obj->SetCrashLineHeight(2.0f);
#else
        // TODO: :(
#endif
// TODO (krzys_h):         obj->CreateShadowCircle(8.0f, 1.0f, D3DSHADOWBARRIER1);
    }

    if ( params.type == OBJECT_BARRIER7 )  // barričre métal 10m
    {
        int rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/barrier7.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);

#if !CRASH_LINES_ARE_NOT_YET_IMPLEMENTED
        obj->CreateCrashLine(Math::Point(-20.0f, -1.0f), SOUND_CHOCm, 0.45f);
        obj->CreateCrashLine(Math::Point( 20.0f, -1.0f), SOUND_CHOCm, 0.45f);
        obj->CreateCrashLine(Math::Point( 20.0f,  1.0f), SOUND_CHOCm, 0.45f);
        obj->CreateCrashLine(Math::Point(-20.0f,  1.0f), SOUND_CHOCm, 0.45f);
        obj->CreateCrashLine(Math::Point(-20.0f, -1.0f), SOUND_CHOCm, 0.45f);
        obj->SetCrashLineHeight(2.0f);
#else
        // TODO: :(
#endif
// TODO (krzys_h):         obj->CreateShadowCircle(16.0f, 1.0f, D3DSHADOWBARRIER2);
    }

    if ( params.type == OBJECT_BARRIER8 )  // barričre métal 14m
    {
        int rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/barrier8.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);

#if !CRASH_LINES_ARE_NOT_YET_IMPLEMENTED
        obj->CreateCrashLine(Math::Point(-28.0f, -1.0f), SOUND_CHOCm, 0.45f);
        obj->CreateCrashLine(Math::Point( 28.0f, -1.0f), SOUND_CHOCm, 0.45f);
        obj->CreateCrashLine(Math::Point( 28.0f,  1.0f), SOUND_CHOCm, 0.45f);
        obj->CreateCrashLine(Math::Point(-28.0f,  1.0f), SOUND_CHOCm, 0.45f);
        obj->CreateCrashLine(Math::Point(-28.0f, -1.0f), SOUND_CHOCm, 0.45f);
        obj->SetCrashLineHeight(2.0f);
#else
        // TODO: :(
#endif
// TODO (krzys_h):         obj->CreateShadowCircle(19.0f, 1.0f, D3DSHADOWBARRIER2);
    }

    if ( params.type == OBJECT_BARRIER9 )  // barričre rouge-blanche 10m
    {
        int rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/barrier9.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);

#if !CRASH_LINES_ARE_NOT_YET_IMPLEMENTED
        obj->CreateCrashLine(Math::Point(-20.0f, -1.0f), SOUND_CHOCo, 0.45f);
        obj->CreateCrashLine(Math::Point( 20.0f, -1.0f), SOUND_CHOCo, 0.45f);
        obj->CreateCrashLine(Math::Point( 20.0f,  1.0f), SOUND_CHOCo, 0.45f);
        obj->CreateCrashLine(Math::Point(-20.0f,  1.0f), SOUND_CHOCo, 0.45f);
        obj->CreateCrashLine(Math::Point(-20.0f, -1.0f), SOUND_CHOCo, 0.45f);
        obj->SetCrashLineHeight(3.0f);
#else
        // TODO: :(
#endif
// TODO (krzys_h):         obj->CreateShadowCircle(15.0f, 1.0f, D3DSHADOWBARRIER2);
    }

    if ( params.type == OBJECT_BARRIER10 )  // tuyau extrémité 10m
    {
        int rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/barrier10.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);

#if !CRASH_LINES_ARE_NOT_YET_IMPLEMENTED
        obj->CreateCrashLine(Math::Point(-20.0f, -1.0f), SOUND_CHOCm, 0.45f);
        obj->CreateCrashLine(Math::Point( 20.0f, -1.0f), SOUND_CHOCm, 0.45f);
        obj->CreateCrashLine(Math::Point( 20.0f,  1.0f), SOUND_CHOCm, 0.45f);
        obj->CreateCrashLine(Math::Point(-20.0f,  1.0f), SOUND_CHOCm, 0.45f);
        obj->CreateCrashLine(Math::Point(-20.0f, -1.0f), SOUND_CHOCm, 0.45f);
        obj->SetCrashLineHeight(3.0f);
#else
        // TODO: :(
#endif
// TODO (krzys_h):         obj->CreateShadowCircle(15.0f, 1.0f, D3DSHADOWBARRIER2);
    }

    if ( params.type == OBJECT_BARRIER11 )  // tuyau milieu 10m
    {
        int rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/barrier11.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);

        obj->AddCrashSphere(CrashSphere(Math::Vector(-20.0f, 3.0f, 0.0f), 3.5f, SOUND_CHOCm, 0.45f));
#if !CRASH_LINES_ARE_NOT_YET_IMPLEMENTED
        obj->CreateCrashLine(Math::Point(-20.0f, -1.0f), SOUND_CHOCm, 0.45f);
        obj->CreateCrashLine(Math::Point( 20.0f, -1.0f), SOUND_CHOCm, 0.45f);
        obj->CreateCrashLine(Math::Point( 20.0f,  1.0f), SOUND_CHOCm, 0.45f);
        obj->CreateCrashLine(Math::Point(-20.0f,  1.0f), SOUND_CHOCm, 0.45f);
        obj->CreateCrashLine(Math::Point(-20.0f, -1.0f), SOUND_CHOCm, 0.45f);
        obj->SetCrashLineHeight(3.0f);
#else
        // TODO: :(
#endif
// TODO (krzys_h):         obj->CreateShadowCircle(15.0f, 1.0f, D3DSHADOWBARRIER2);
    }

    if ( params.type == OBJECT_BARRIER12 )  // tuyau milieu avec robinet 10m
    {
        int rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/barrier12.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);

        obj->AddCrashSphere(CrashSphere(Math::Vector(-20.0f, 3.0f, 0.0f), 3.5f, SOUND_CHOCm, 0.45f));
#if !CRASH_LINES_ARE_NOT_YET_IMPLEMENTED
        obj->CreateCrashLine(Math::Point(-20.0f, -1.0f), SOUND_CHOCm, 0.45f);
        obj->CreateCrashLine(Math::Point( 20.0f, -1.0f), SOUND_CHOCm, 0.45f);
        obj->CreateCrashLine(Math::Point( 20.0f,  1.0f), SOUND_CHOCm, 0.45f);
        obj->CreateCrashLine(Math::Point(-20.0f,  1.0f), SOUND_CHOCm, 0.45f);
        obj->CreateCrashLine(Math::Point(-20.0f, -1.0f), SOUND_CHOCm, 0.45f);
        obj->SetCrashLineHeight(3.0f);
#else
        // TODO: :(
#endif
// TODO (krzys_h):         obj->CreateShadowCircle(15.0f, 1.0f, D3DSHADOWBARRIER2);
    }

    if ( params.type == OBJECT_BARRIER13 )  // tuyau aérien 10m
    {
        int rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/barrier13.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);

        obj->AddCrashSphere(CrashSphere(Math::Vector(-20.0f, 3.0f, 0.0f), 3.5f, SOUND_CHOCm, 0.45f));
// TODO (krzys_h):         obj->CreateShadowCircle(15.0f, 1.0f, D3DSHADOWBARRIER2);
    }

    if ( params.type == OBJECT_BARRIER14 )  // mur 10m
    {
        int rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/barrier14.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);

#if !CRASH_LINES_ARE_NOT_YET_IMPLEMENTED
        obj->CreateCrashLine(Math::Point(-20.0f, -1.0f), SOUND_BOUMm, 0.45f);
        obj->CreateCrashLine(Math::Point( 20.0f, -1.0f), SOUND_BOUMm, 0.45f);
        obj->CreateCrashLine(Math::Point( 20.0f,  1.0f), SOUND_BOUMm, 0.45f);
        obj->CreateCrashLine(Math::Point(-20.0f,  1.0f), SOUND_BOUMm, 0.45f);
        obj->CreateCrashLine(Math::Point(-20.0f, -1.0f), SOUND_BOUMm, 0.45f);
#else
        // TODO: :(
#endif
        obj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 3.0f, 0.0f), 21.0f));
        bPlumb = true;
    }

    if ( params.type == OBJECT_BARRIER15 )  // mur haut 10m
    {
        int rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/barrier15.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);

#if !CRASH_LINES_ARE_NOT_YET_IMPLEMENTED
        obj->CreateCrashLine(Math::Point(-20.0f, -1.0f), SOUND_BOUMm, 0.45f);
        obj->CreateCrashLine(Math::Point( 20.0f, -1.0f), SOUND_BOUMm, 0.45f);
        obj->CreateCrashLine(Math::Point( 20.0f,  1.0f), SOUND_BOUMm, 0.45f);
        obj->CreateCrashLine(Math::Point(-20.0f,  1.0f), SOUND_BOUMm, 0.45f);
        obj->CreateCrashLine(Math::Point(-20.0f, -1.0f), SOUND_BOUMm, 0.45f);
#else
        // TODO: :(
#endif
        obj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 3.0f, 0.0f), 21.0f));
        bPlumb = true;
    }

    if ( params.type == OBJECT_BARRIER16 )  // mur  14m
    {
        int rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/barrier16.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);

#if !CRASH_LINES_ARE_NOT_YET_IMPLEMENTED
        obj->CreateCrashLine(Math::Point(-28.0f, -1.0f), SOUND_BOUMm, 0.45f);
        obj->CreateCrashLine(Math::Point( 28.0f, -1.0f), SOUND_BOUMm, 0.45f);
        obj->CreateCrashLine(Math::Point( 28.0f,  1.0f), SOUND_BOUMm, 0.45f);
        obj->CreateCrashLine(Math::Point(-28.0f,  1.0f), SOUND_BOUMm, 0.45f);
        obj->CreateCrashLine(Math::Point(-28.0f, -1.0f), SOUND_BOUMm, 0.45f);
#else
        // TODO: :(
#endif
        obj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 3.0f, 0.0f), 30.0f));
        bPlumb = true;
    }

    if ( params.type == OBJECT_BARRIER17 )  // barričre panneaux >
    {
        int rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/barrier17.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);

#if !CRASH_LINES_ARE_NOT_YET_IMPLEMENTED
        obj->CreateCrashLine(Math::Point(-20.0f, -2.5f), SOUND_BOUMm, 0.45f);
        obj->CreateCrashLine(Math::Point( 20.0f, -2.5f), SOUND_BOUMm, 0.45f);
        obj->CreateCrashLine(Math::Point( 20.0f,  2.5f), SOUND_BOUMm, 0.45f);
        obj->CreateCrashLine(Math::Point(-20.0f,  2.5f), SOUND_BOUMm, 0.45f);
        obj->CreateCrashLine(Math::Point(-20.0f, -2.5f), SOUND_BOUMm, 0.45f);
        obj->SetCrashLineHeight(3.0f);
#else
        // TODO: :(
#endif
    }
    if ( params.type == OBJECT_BARRIER18 )  // barričre panneaux <
    {
        int rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/barrier17.mod", true, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);

#if !CRASH_LINES_ARE_NOT_YET_IMPLEMENTED
        obj->CreateCrashLine(Math::Point(-20.0f, -2.5f), SOUND_BOUMm, 0.45f);
        obj->CreateCrashLine(Math::Point( 20.0f, -2.5f), SOUND_BOUMm, 0.45f);
        obj->CreateCrashLine(Math::Point( 20.0f,  2.5f), SOUND_BOUMm, 0.45f);
        obj->CreateCrashLine(Math::Point(-20.0f,  2.5f), SOUND_BOUMm, 0.45f);
        obj->CreateCrashLine(Math::Point(-20.0f, -2.5f), SOUND_BOUMm, 0.45f);
        obj->SetCrashLineHeight(3.0f);
#else
        // TODO: :(
#endif
    }

    if ( params.type == OBJECT_BARRIER19 )  // poteau >
    {
        int rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        modelManager->AddModelReference("buzzingcars/barrier19.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);

        obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 3.0f, 0.0f), 1.2f, SOUND_CHOCo, 0.45f));
#if !CRASH_LINES_ARE_NOT_YET_IMPLEMENTED
        obj->SetCrashLineHeight(3.0f);
#endif
        obj->CreateShadowCircle(2.5f, 0.5f);
    }

    Math::Vector pos = obj->GetPosition();
    obj->SetPosition(pos);  // to display the shadows immediately

    obj->SetFloorHeight(0.0f);
    // TODO (krzys_h): CreateOtherObject(type);
    if ( !bPlumb )
    {
        obj->FloorAdjust();
        if ( params.type != OBJECT_BARRIER3  &&
             params.type != OBJECT_BARRIER14 )
        {
            obj->SetRotationX(0.0f);
        }
    }

    pos = obj->GetPosition();
    pos.y += height;
    obj->SetPosition(pos);

#if !CRASH_LINES_ARE_NOT_YET_IMPLEMENTED
    obj->CrashLineFusion();
#endif

    return obj;
}
