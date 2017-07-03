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

#include "object/subclass/buzzingcars/bc_building.h"

#include "common/make_unique.h"

#include "graphics/engine/oldmodelmanager.h"
#include "graphics/engine/terrain.h"

#include "math/geometry.h"

#include "object/object_create_params.h"

#include "object/auto/buzzingcars/building/autoalien.h"
#include "object/auto/buzzingcars/building/autobarrel.h"
#include "object/auto/buzzingcars/building/autocomputer.h"
#include "object/auto/buzzingcars/building/autodock.h"
#include "object/auto/buzzingcars/building/autodoor.h"
#include "object/auto/buzzingcars/building/autogenerator.h"
#include "object/auto/buzzingcars/building/autohome.h"
#include "object/auto/buzzingcars/building/autoinca.h"
#include "object/auto/buzzingcars/building/autoremote.h"
#include "object/auto/buzzingcars/building/autostand.h"


CBCBuilding::CBCBuilding(int id, ObjectType type)
    : CBaseBuilding(id, type)
{
}

CBCBuilding::~CBCBuilding()
{}

std::unique_ptr<CBCBuilding> CBCBuilding::Create(
    const ObjectCreateParams& params,
    Gfx::COldModelManager* modelManager,
    Gfx::CEngine* engine)
{
    auto obj = MakeUnique<CBCBuilding>(params.id, params.type);

    obj->SetTeam(params.team);

    float height = params.height;

    int rank = engine->CreateObject();
    engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);  // it is a stationary object
    obj->SetObjectRank(0, rank);

    if ( params.type == OBJECT_FACTORY1 )  // 2 tuyaux verticaux ?
    {
        modelManager->AddModelReference("buzzingcars/factory1.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

#if 0
        obj->AddCrashSphere(CrashSphere(Math::Vector(  0.0f,  0.0f,   0.0f), 40.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 30.0f,  5.0f,  10.0f),  9.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 30.0f,  5.0f,  30.0f),  9.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 35.0f,  5.0f, -10.0f),  5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 35.0f,  5.0f, -20.0f),  5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 35.0f,  5.0f, -30.0f),  5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 25.0f,  5.0f, -30.0f),  5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 15.0f,  5.0f, -35.0f),  5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-20.0f,  5.0f, -35.0f),  5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-30.0f,  5.0f, -35.0f),  5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 15.0f,  5.0f,  35.0f),  5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-20.0f,  5.0f,  35.0f),  5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-30.0f,  5.0f,  35.0f),  5.0f, SOUND_BOUMm, 0.45f));
#else
        obj->AddCrashLine(CrashLine(Math::Point(-35.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-35.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-35.0f, -40.0f), SOUND_BOUMm, 0.45f));
#endif

        obj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 10.0f, 0.0f), 40.0f));
    }

    if ( params.type == OBJECT_FACTORY2 )  // usine avec 2 cheminées ?
    {
        modelManager->AddModelReference("buzzingcars/factory2.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

#if 0
        obj->AddCrashSphere(CrashSphere(Math::Vector(  0.0f,  0.0f,   0.0f), 40.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 26.0f,  3.0f,  26.0f), 14.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 26.0f,  3.0f, -26.0f), 14.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-26.0f,  3.0f,  26.0f), 14.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-26.0f,  3.0f, -26.0f), 14.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 36.0f,  3.0f,  36.0f),  4.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 36.0f,  3.0f, -36.0f),  4.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-36.0f,  3.0f,  36.0f),  4.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-36.0f,  3.0f, -36.0f),  4.0f, SOUND_BOUMm, 0.45f));
#else
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f, -40.0f), SOUND_BOUMm, 0.45f));
#endif

        obj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 10.0f, 0.0f), 40.0f));
    }

    if ( params.type == OBJECT_FACTORY3 )  // usine avec toît /|/| ?
    {
        modelManager->AddModelReference("buzzingcars/factory3.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

#if 0
        obj->AddCrashSphere(CrashSphere(Math::Vector(  0.0f,  0.0f,   0.0f), 40.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 26.0f,  3.0f,  26.0f), 14.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 26.0f,  3.0f, -26.0f), 14.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-26.0f,  3.0f,  26.0f), 14.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-26.0f,  3.0f, -26.0f), 14.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 36.0f,  3.0f,  36.0f),  4.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 36.0f,  3.0f, -36.0f),  4.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-36.0f,  3.0f,  36.0f),  4.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-36.0f,  3.0f, -36.0f),  4.0f, SOUND_BOUMm, 0.45f));
#else
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f, -40.0f), SOUND_BOUMm, 0.45f));
#endif

        obj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 10.0f, 0.0f), 40.0f));
    }

    if ( params.type == OBJECT_FACTORY4 )  // ailettes refroidisseur ?
    {
        modelManager->AddModelReference("buzzingcars/factory4.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

#if 0
        obj->AddCrashSphere(CrashSphere(Math::Vector(  0.0f,  0.0f,   0.0f), 40.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 15.0f,  3.0f,  37.0f),  3.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 37.0f,  3.0f,  15.0f),  3.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 15.0f,  3.0f, -37.0f),  3.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 37.0f,  3.0f, -15.0f),  3.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-15.0f,  3.0f,  37.0f),  3.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-37.0f,  3.0f,  15.0f),  3.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-15.0f,  3.0f, -37.0f),  3.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-37.0f,  3.0f, -15.0f),  3.0f, SOUND_BOUMm, 0.45f));
#else
        obj->AddCrashLine(CrashLine(Math::Point(-17.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 17.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f, -17.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f,  17.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 17.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-17.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f,  17.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f, -17.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-17.0f, -40.0f), SOUND_BOUMm, 0.45f));
#endif

        obj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 10.0f, 0.0f), 40.0f));
    }

    if ( params.type == OBJECT_FACTORY5 )  // ?
    {
        modelManager->AddModelReference("buzzingcars/factory5.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

#if 0
        obj->AddCrashSphere(CrashSphere(Math::Vector(  0.0f,  0.0f,   0.0f), 40.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 35.0f,  3.0f,  15.0f),  5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 35.0f,  3.0f,  25.0f),  5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 15.0f,  3.0f,  35.0f),  5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 25.0f,  3.0f,  35.0f),  5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 35.0f,  3.0f, -15.0f),  5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 35.0f,  3.0f, -25.0f),  5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 15.0f,  3.0f, -35.0f),  5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 25.0f,  3.0f, -35.0f),  5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-35.0f,  3.0f,  15.0f),  5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-35.0f,  3.0f,  25.0f),  5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-15.0f,  3.0f,  35.0f),  5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-25.0f,  3.0f,  35.0f),  5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-35.0f,  3.0f, -15.0f),  5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-35.0f,  3.0f, -25.0f),  5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-15.0f,  3.0f, -35.0f),  5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-25.0f,  3.0f, -35.0f),  5.0f, SOUND_BOUMm, 0.45f));
#else
        obj->AddCrashLine(CrashLine(Math::Point(-30.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 30.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f, -30.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f,  30.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 30.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-30.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f,  30.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f, -30.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-30.0f, -40.0f), SOUND_BOUMm, 0.45f));
#endif

        obj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 10.0f, 0.0f), 40.0f));
    }

    if ( params.type == OBJECT_FACTORY6 )  // 2 tuyaux haut ?
    {
        modelManager->AddModelReference("buzzingcars/factory6.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);
    }

    if ( params.type == OBJECT_FACTORY7 )  // pont de chargement haut ?
    {
        modelManager->AddModelReference("buzzingcars/factory7.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);
    }

    if ( params.type == OBJECT_FACTORY8 )  // départ tuyau 6 ?
    {
        modelManager->AddModelReference("buzzingcars/factory8.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

#if 0
        obj->AddCrashSphere(CrashSphere(Math::Vector(  0.0f,  0.0f,   0.0f), 40.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 26.0f,  3.0f,  26.0f), 14.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 26.0f,  3.0f, -26.0f), 14.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-26.0f,  3.0f,  26.0f), 14.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-26.0f,  3.0f, -26.0f), 14.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 36.0f,  3.0f,  36.0f),  4.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 36.0f,  3.0f, -36.0f),  4.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-36.0f,  3.0f,  36.0f),  4.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-36.0f,  3.0f, -36.0f),  4.0f, SOUND_BOUMm, 0.45f));
#else
        obj->AddCrashLine(CrashLine(Math::Point(-30.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 30.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f, -30.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f,  30.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 30.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-30.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f,  30.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f, -30.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-30.0f, -40.0f), SOUND_BOUMm, 0.45f));
#endif

        obj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 10.0f, 0.0f), 40.0f));
    }

    if ( params.type == OBJECT_FACTORY9 )  // cilo rond ?
    {
        modelManager->AddModelReference("buzzingcars/factory9.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        obj->AddCrashSphere(CrashSphere(Math::Vector( 19.0f, 3.0f,   5.0f), 1.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 19.0f, 3.0f,  -5.0f), 1.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-19.0f, 3.0f,   5.0f), 1.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-19.0f, 3.0f,  -5.0f), 1.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(  5.0f, 3.0f,  19.0f), 1.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( -5.0f, 3.0f,  19.0f), 1.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(  5.0f, 3.0f, -19.0f), 1.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( -5.0f, 3.0f, -19.0f), 1.5f, SOUND_BOUMm, 0.45f));

        obj->CreateShadowCircle(20.0f, 0.5f);
    }

    if ( params.type == OBJECT_FACTORY10 )  // tanker A2 ?
    {
        modelManager->AddModelReference("buzzingcars/factory10.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 3.0f, 0.0f), 40.0f, SOUND_BOUMm, 0.45f));
        obj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 10.0f, 0.0f), 40.0f));
    }

    if ( params.type == OBJECT_FACTORY11 )  // tanker C5 ?
    {
        modelManager->AddModelReference("buzzingcars/factory11.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 3.0f, 0.0f), 40.0f, SOUND_BOUMm, 0.45f));
        obj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 10.0f, 0.0f), 40.0f));
    }

    if ( params.type == OBJECT_FACTORY12 )  // tanker F4 ?
    {
        modelManager->AddModelReference("buzzingcars/factory12.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 3.0f, 0.0f), 40.0f, SOUND_BOUMm, 0.45f));
        obj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 10.0f, 0.0f), 40.0f));
    }

    if ( params.type == OBJECT_FACTORY13 )  // cilo carré ?
    {
        modelManager->AddModelReference("buzzingcars/factory13.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        obj->AddCrashSphere(CrashSphere(Math::Vector( 9.0f, 3.0f,   29.5f), 1.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 9.0f, 3.0f,  -29.5f), 1.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-9.0f, 3.0f,   29.5f), 1.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-9.0f, 3.0f,  -29.5f), 1.5f, SOUND_BOUMm, 0.45f));

        obj->CreateShadowCircle(20.0f, 0.5f);
    }

    if ( params.type == OBJECT_FACTORY14 )  // taMath::PIs roulant ?
    {
        modelManager->AddModelReference("buzzingcars/factory14.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

#if 1
        obj->AddCrashLine(CrashLine(Math::Point(-18.0f, -4.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(  5.0f, -4.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(  5.0f,  4.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-18.0f,  4.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-18.0f, -4.0f), SOUND_BOUMm, 0.45f));
#endif

//?        obj->CreateShadowCircle(18.0f, 1.0f, D3DSHADOWBARRIER2);
    }

    if ( params.type == OBJECT_GRAVEL )  // tas de gravier ?
    {
        modelManager->AddModelReference("buzzingcars/gravel.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetScale(params.zoom);
        obj->SetFloorHeight(0.0f);

//?        obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 3.0f, 0.0f), 15.0f, SOUND_BOUMv, 0.15f));
        engine->GetTerrain()->AddBuildingLevel(params.pos, 6.0f*params.zoom, 14.0f*params.zoom, 10.0f*params.zoom, 0.5f);
    }

    if ( params.type == OBJECT_TUB )  // bac de gravier ?
    {
        modelManager->AddModelReference("buzzingcars/tub.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetScale(params.zoom);
        obj->SetFloorHeight(0.0f);

#if 1
        obj->AddCrashLine(CrashLine(Math::Point(-20.0f, -20.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 20.0f, -20.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 20.0f,  20.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-20.0f,  20.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-20.0f, -20.0f), SOUND_BOUMm, 0.45f));
#endif
    }

    if ( params.type == OBJECT_ALIEN1 )  // usine ?
    {
        modelManager->AddModelReference("buzzingcars/alien1.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

#if 1
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f, -40.0f), SOUND_BOUMm, 0.45f));
#endif

        obj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 10.0f, 0.0f), 40.0f));
    }

    if ( params.type == OBJECT_ALIEN2 )  // tour ?
    {
        modelManager->AddModelReference("buzzingcars/alien2.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        modelManager->AddModelReference("buzzingcars/alien21.mod", false, rank, params.team);
        obj->SetPartPosition(1, Math::Vector(0.0f, 40.0f, 0.0f));

#if 1
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f, -40.0f), SOUND_BOUMm, 0.45f));
#endif

        obj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 10.0f, 0.0f), 40.0f));
    }

    if ( params.type == OBJECT_ALIEN3 )  // tour haute ?
    {
        modelManager->AddModelReference("buzzingcars/alien3.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        modelManager->AddModelReference("buzzingcars/alien21.mod", false, rank, params.team);
        obj->SetPartPosition(1, Math::Vector(0.0f, 50.0f, 0.0f));

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(2, rank);
        obj->SetObjectParent(2, 0);
        modelManager->AddModelReference("buzzingcars/alien21.mod", false, rank, params.team);
        obj->SetPartPosition(2, Math::Vector(0.0f, 80.0f, 0.0f));

#if 1
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f, -40.0f), SOUND_BOUMm, 0.45f));
#endif

        obj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 10.0f, 0.0f), 40.0f));
    }

    if ( params.type == OBJECT_ALIEN4 )  // habitation ?
    {
        modelManager->AddModelReference("buzzingcars/alien4.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

#if 1
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f, -40.0f), SOUND_BOUMm, 0.45f));
#endif

        obj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 10.0f, 0.0f), 40.0f));
    }

    if ( params.type == OBJECT_ALIEN5 )  // marteau ?
    {
        modelManager->AddModelReference("buzzingcars/alien5.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        modelManager->AddModelReference("buzzingcars/alien51.mod", false, rank, params.team);
        obj->SetPartPosition(1, Math::Vector(0.0f, 25.0f, 17.0f));


#if 1
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f, -40.0f), SOUND_BOUMm, 0.45f));
#endif

        obj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 10.0f, 0.0f), 40.0f));
    }

    if ( params.type == OBJECT_ALIEN6 )  // écraseur ?
    {
        modelManager->AddModelReference("buzzingcars/alien6.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        modelManager->AddModelReference("buzzingcars/alien61.mod", false, rank, params.team);
        obj->SetPartPosition(1, Math::Vector(-26.0f, 13.0f, 0.0f));

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(2, rank);
        obj->SetObjectParent(2, 0);
        modelManager->AddModelReference("buzzingcars/alien62.mod", false, rank, params.team);
        obj->SetPartPosition(2, Math::Vector(26.0f, 13.0f, 0.0f));

#if 1
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f, -40.0f), SOUND_BOUMm, 0.45f));
#endif

        obj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 10.0f, 0.0f), 40.0f));
    }

    if ( params.type == OBJECT_ALIEN7 )  // électrocuteur ?
    {
        modelManager->AddModelReference("buzzingcars/alien7.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

#if 1
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f, -40.0f), SOUND_BOUMm, 0.45f));
#endif

        obj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 10.0f, 0.0f), 40.0f));
    }

    if ( params.type == OBJECT_ALIEN8 )  // générateur ?
    {
        modelManager->AddModelReference("buzzingcars/alien8.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        rank = engine->CreateObject();  // bras
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        modelManager->AddModelReference("buzzingcars/alien81.mod", false, rank, params.team);
        obj->SetPartPosition(1, Math::Vector(25.0f, 13.0f, 15.0f));
        obj->SetPartRotationY(1, 50.0f*Math::PI/180.0f);

        rank = engine->CreateObject();  // Math::PInce gauche
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(2, rank);
        obj->SetObjectParent(2, 1);
        modelManager->AddModelReference("buzzingcars/alien82.mod", false, rank, params.team);
        obj->SetPartPosition(2, Math::Vector(-3.0f, 0.0f, -47.0f));
        obj->SetPartRotationY(2, 45.0f*Math::PI/180.0f);

        rank = engine->CreateObject();  // Math::PInce droite
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(3, rank);
        obj->SetObjectParent(3, 1);
        modelManager->AddModelReference("buzzingcars/alien83.mod", false, rank, params.team);
        obj->SetPartPosition(3, Math::Vector(3.0f, 0.0f, -47.0f));
        obj->SetPartRotationY(3, -45.0f*Math::PI/180.0f);

        rank = engine->CreateObject();  // bouton rouge
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(4, rank);
        obj->SetObjectParent(4, 0);
        modelManager->AddModelReference("buzzingcars/alien84.mod", false, rank, params.team);
        obj->SetPartPosition(4, Math::Vector(-14.0f, 5.0f, 40.0f));

        // bouton rouge :
        obj->AddCrashSphere(CrashSphere(Math::Vector(-14.0f, 3.0f, 42.0f), 3.0f, SOUND_BOUMm, 0.44f));

#if 1
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f, -40.0f), SOUND_BOUMm, 0.45f));
#endif

        obj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 10.0f, 0.0f), 40.0f));
    }

    if ( params.type == OBJECT_ALIEN9 )  // torture ?
    {
        modelManager->AddModelReference("buzzingcars/alien9.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        modelManager->AddModelReference("buzzingcars/alien91.mod", false, rank, params.team);
        obj->SetPartPosition(1, Math::Vector(0.0f, 34.0f, 0.0f));

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(2, rank);
        obj->SetObjectParent(2, 1);
        modelManager->AddModelReference("buzzingcars/alien92.mod", false, rank, params.team);
        obj->SetPartPosition(2, Math::Vector(0.0f, -1.0f, 0.0f));

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(3, rank);
        obj->SetObjectParent(3, 2);
        modelManager->AddModelReference("buzzingcars/alien93.mod", false, rank, params.team);
        obj->SetPartPosition(3, Math::Vector(0.0f, 1.0f, 0.0f));

#if 1
        obj->AddCrashLine(CrashLine(Math::Point(-15.0f, -15.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 15.0f, -15.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 15.0f,  15.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-15.0f,  15.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-15.0f, -15.0f), SOUND_BOUMm, 0.45f));
#endif
    }

    if ( params.type == OBJECT_INCA1 )  // temple ?
    {
        modelManager->AddModelReference("buzzingcars/inca1.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

#if 1
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f, -40.0f), SOUND_BOUMm, 0.45f));
#endif
    }

    if ( params.type == OBJECT_INCA2 )  // temple ?
    {
        modelManager->AddModelReference("buzzingcars/inca2.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

#if 1
        obj->AddCrashLine(CrashLine(Math::Point(-10.0f, -10.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 10.0f, -10.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 10.0f,  10.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-10.0f,  10.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-10.0f, -10.0f), SOUND_BOUMm, 0.45f));
#endif
    }

    if ( params.type == OBJECT_INCA3 )  // temple ?
    {
        modelManager->AddModelReference("buzzingcars/inca3.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

#if 1
        obj->AddCrashLine(CrashLine(Math::Point(-50.0f, -30.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 50.0f, -30.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 50.0f,  30.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-50.0f,  30.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-50.0f, -30.0f), SOUND_BOUMm, 0.45f));
#endif
    }

    if ( params.type == OBJECT_INCA4 )  // temple ?
    {
        modelManager->AddModelReference("buzzingcars/inca4.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

#if 1
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f, -10.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f, -10.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f,  10.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f,  10.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f, -10.0f), SOUND_BOUMm, 0.45f));
#endif
    }

    if ( params.type == OBJECT_INCA5 )  // temple ?
    {
        modelManager->AddModelReference("buzzingcars/inca5.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

#if 1
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f, -40.0f), SOUND_BOUMm, 0.45f));
#endif
    }

    if ( params.type == OBJECT_INCA6 )  // statue ?
    {
        modelManager->AddModelReference("buzzingcars/inca6.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

#if 1
        obj->AddCrashLine(CrashLine(Math::Point(-10.0f, -10.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 10.0f, -10.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 10.0f,  10.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-10.0f,  10.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-10.0f, -10.0f), SOUND_BOUMm, 0.45f));
#endif
    }

    if ( params.type == OBJECT_INCA7 )  // temple électrocuteur ?
    {
        modelManager->AddModelReference("buzzingcars/inca71.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        rank = engine->CreateObject();  // bouton rouge
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        modelManager->AddModelReference("buzzingcars/inca72.mod", false, rank, params.team);
        obj->SetPartPosition(1, Math::Vector(-10.0f, 5.0f, 0.0f));

        rank = engine->CreateObject();  // Math::PIlier
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(2, rank);
        obj->SetObjectParent(2, 0);
        modelManager->AddModelReference("buzzingcars/inca73.mod", false, rank, params.team);
        obj->SetPartPosition(2, Math::Vector(0.0f, 10.0f, 0.0f));

        rank = engine->CreateObject();  // porte gauche
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(3, rank);
        obj->SetObjectParent(3, 2);
        modelManager->AddModelReference("buzzingcars/inca74.mod", false, rank, params.team);
        obj->SetPartPosition(3, Math::Vector(0.0f, 25.0f, 0.0f));

        rank = engine->CreateObject();  // porte droite
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(4, rank);
        obj->SetObjectParent(4, 2);
        modelManager->AddModelReference("buzzingcars/inca75.mod", false, rank, params.team);
        obj->SetPartPosition(4, Math::Vector(0.0f, 25.0f, 0.0f));

        // bouton rouge :
        obj->AddCrashSphere(CrashSphere(Math::Vector(-12.0f, 3.0f,  0.0f), 3.0f, SOUND_BOUMm, 0.44f));

#if 1
        obj->AddCrashLine(CrashLine(Math::Point(-10.0f, -10.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 10.0f, -10.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 10.0f,  10.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-10.0f,  10.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-10.0f, -10.0f), SOUND_BOUMm, 0.45f));
#endif
    }

    if ( params.type == OBJECT_BUILDING1 )  // bâtiment ?
    {
        modelManager->AddModelReference("buzzingcars/buildin1.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

#if 1
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f, -40.0f), SOUND_BOUMm, 0.45f));
#endif
    }

    if ( params.type == OBJECT_BUILDING2 )  // bâtiment ?
    {
        modelManager->AddModelReference("buzzingcars/buildin2.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

#if 1
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f, -40.0f), SOUND_BOUMm, 0.45f));
#endif
    }

    if ( params.type == OBJECT_BUILDING3 )  // bâtiment ?
    {
        modelManager->AddModelReference("buzzingcars/buildin3.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

#if 1
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f, -40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 40.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f,  40.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-40.0f, -40.0f), SOUND_BOUMm, 0.45f));
#endif
    }

    if ( params.type == OBJECT_CARCASS1 )  // carcasse ?
    {
        modelManager->AddModelReference("buzzingcars/carcass1.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);
        obj->SetScale(1.4f);

#if 1
        obj->AddCrashLine(CrashLine(Math::Point(-6.0f, -3.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 6.0f, -3.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 6.0f,  3.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-6.0f,  3.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-6.0f, -3.0f), SOUND_BOUMm, 0.45f));
#endif

        // TODO (krzys_h): obj->CreateShadowCircle(4.6f, 1.0f, D3DSHADOWCAR07);
        obj->FloorAdjust();
    }

    if ( params.type == OBJECT_CARCASS2 )  // carcasse ?
    {
        modelManager->AddModelReference("buzzingcars/carcass2.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);
        obj->SetScale(1.4f);

#if 1
        obj->AddCrashLine(CrashLine(Math::Point(-6.0f, -3.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 6.0f, -3.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 6.0f,  3.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-6.0f,  3.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-6.0f, -3.0f), SOUND_BOUMm, 0.45f));
#endif

        obj->CreateShadowCircle(7.0f, 0.2f);
        obj->FloorAdjust();
    }

    if ( params.type == OBJECT_ORGA1 )  // organique ?
    {
        modelManager->AddModelReference("buzzingcars/orga1.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);
        obj->SetScale(params.zoom);

        obj->AddCrashSphere(CrashSphere(Math::Vector(-25.0f, 3.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 24.0f, 3.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f));
    }

    if ( params.type == OBJECT_ORGA2 )  // organique ?
    {
        modelManager->AddModelReference("buzzingcars/orga2.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);
        obj->SetScale(params.zoom);

        obj->AddCrashSphere(CrashSphere(Math::Vector(-25.0f, 3.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 28.0f, 3.0f, 0.0f), 4.0f, SOUND_BOUMm, 0.45f));
    }

    if ( params.type == OBJECT_ORGA3 )  // organique ?
    {
        modelManager->AddModelReference("buzzingcars/orga3.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);
        obj->SetScale(params.zoom);

        obj->AddCrashSphere(CrashSphere(Math::Vector(-25.0f, 3.0f, 0.0f), 4.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 24.0f, 3.0f, 0.0f), 4.0f, SOUND_BOUMm, 0.45f));
    }

    /*if ( params.type == OBJECT_TOWER )
    {
        modelManager->AddModelReference("buzzingcars/tower1.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        modelManager->AddModelReference("buzzingcars/tower2.mod", false, rank, params.team);
        obj->SetPartPosition(1, Math::Vector(0.0f, 40.0f, 0.0f));

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(2, rank);
        obj->SetObjectParent(2, 1);
        modelManager->AddModelReference("buzzingcars/tower3.mod", false, rank, params.team);
        obj->SetPartPosition(2, Math::Vector(0.0f, 9.0f, 0.0f));
        SetRotationZ(2, 0.0f);

        obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 4.0f, 0.0f), 11.0f, SOUND_BOUMm, 0.45f));
        obj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 7.0f, 0.0f), 11.0f));

        obj->CreateShadowCircle(9.0f, 1.0f);
    }

    if ( params.type == OBJECT_NUCLEAR )
    {
        modelManager->AddModelReference("buzzingcars/nuclear1.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        obj->AddCrashSphere(CrashSphere(Math::Vector( 0.0f,  0.0f, 0.0f), 55.0f, SOUND_BOUMm, 0.45f));
        obj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 10.0f, 0.0f), 60.0f));
    }

    if ( params.type == OBJECT_PARA )
    {
        modelManager->AddModelReference("buzzingcars/para.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        engine->GetTerrain()->AddBuildingLevel(params.pos, 24.5f, 27.4f, 2.2f, 0.5f);

        obj->AddCrashSphere(CrashSphere(Math::Vector( 19.5f, 4.5f,  19.5f), 4.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-19.5f, 4.5f,  19.5f), 4.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 19.5f, 4.5f, -19.5f), 4.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-19.5f, 4.5f, -19.5f), 4.5f, SOUND_BOUMm, 0.45f));
        obj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 15.0f, 0.0f), 30.0f));
    }*/

    if ( params.type == OBJECT_COMPUTER )
    {
        modelManager->AddModelReference("buzzingcars/compu1.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        rank = engine->CreateObject();  // bouton rouge
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        modelManager->AddModelReference("buzzingcars/compu2.mod", false, rank, params.team);
        obj->SetPartPosition(1, Math::Vector(-6.0f, 5.0f, 0.0f));
        obj->SetPartScale(1, Math::Vector(0.8f, 0.7f, 0.7f));

        rank = engine->CreateObject();  // bouton rouge
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(2, rank);
        obj->SetObjectParent(2, 0);
        modelManager->AddModelReference("buzzingcars/compu2.mod", false, rank, params.team);
        obj->SetPartPosition(2, Math::Vector(3.0f, 5.0f, -5.2f));
        obj->SetPartScale(2, Math::Vector(0.8f, 0.7f, 0.7f));
        obj->SetPartRotationY(2, -120.0f*Math::PI/180.0f);

        rank = engine->CreateObject();  // bouton rouge
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(3, rank);
        obj->SetObjectParent(3, 0);
        modelManager->AddModelReference("buzzingcars/compu2.mod", false, rank, params.team);
        obj->SetPartPosition(3, Math::Vector(3.0f, 5.0f, 5.2f));
        obj->SetPartScale(3, Math::Vector(0.8f, 0.7f, 0.7f));
        obj->SetPartRotationY(3, 120.0f*Math::PI/180.0f);

        // boutons rouges :
        obj->AddCrashSphere(CrashSphere(Math::Vector(-9.0f, 3.0f,  0.0f), 2.0f, SOUND_BOUMm, 0.44f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 4.4f, 3.0f,  7.8f), 2.0f, SOUND_BOUMm, 0.43f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 4.4f, 3.0f, -7.8f), 2.0f, SOUND_BOUMm, 0.42f));

        obj->AddCrashSphere(CrashSphere(Math::Vector( 0.0f,  2.0f, 0.0f), 9.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 0.0f, 10.0f, 0.0f), 9.0f, SOUND_BOUMm, 0.45f));
        obj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 10.0f, 0.0f), 10.0f));
        obj->CreateShadowCircle(12.0f, 1.0f);
    }

    /*if ( params.type == OBJECT_REPAIR )
    {
        modelManager->AddModelReference("buzzingcars/repair1.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        modelManager->AddModelReference("buzzingcars/repair2.mod", false, rank, params.team);
        obj->SetPartPosition(1, Math::Vector(-11.0f, 13.5f, 0.0f));
        SetRotationZ(1, Math::PI*0.5f);

        obj->AddCrashSphere(CrashSphere(Math::Vector(-11.0f, 3.0f, 0.0f), 8.0f, SOUND_BOUMm, 0.45f));
        obj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(-11.0f, 3.0f, 0.0f), 8.0f));
        engine->GetTerrain()->AddBuildingLevel(params.pos, 7.0f, 9.0f, 1.0f, 0.5f);
    }*/

    if ( params.type == OBJECT_SWEET )
    {
        modelManager->AddModelReference("buzzingcars/sweet.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        obj->AddCrashSphere(CrashSphere(Math::Vector(-4.0f, 3.0f, 0.0f), 7.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 4.0f, 3.0f, 0.0f), 7.0f, SOUND_BOUMm, 0.45f));
        obj->SetCameraCollisionSphere(Math::Sphere(Math::Vector(0.0f, 3.0f, 0.0f), 10.0f));
        obj->CreateShadowCircle(12.0f, 1.0f);
    }

    if ( params.type == OBJECT_DOOR1 )
    {
        modelManager->AddModelReference("buzzingcars/door10.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        modelManager->AddModelReference("buzzingcars/door11.mod", false, rank, params.team);

        // Sphčres déplacées par MoveCrashSphere.
        obj->AddCrashSphere(CrashSphere(Math::Vector(  0.0f, 35.0f,  24.0f),  6.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(  0.0f, 35.0f,  12.0f),  6.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(  0.0f, 35.0f,   0.0f),  6.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(  0.0f, 35.0f, -12.0f),  6.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(  0.0f, 35.0f, -24.0f),  6.0f, SOUND_BOUMm, 0.45f));

#if 0
        obj->AddCrashSphere(CrashSphere(Math::Vector(  5.0f,  3.0f,  42.0f), 12.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( -5.0f,  3.0f,  42.0f), 12.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 14.0f,  3.0f,  33.0f),  3.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-14.0f,  3.0f,  33.0f),  3.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 14.0f,  3.0f,  51.0f),  3.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-14.0f,  3.0f,  51.0f),  3.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(  0.0f, 36.0f,  38.0f),  9.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(  5.0f,  3.0f, -42.0f), 12.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( -5.0f,  3.0f, -42.0f), 12.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 14.0f,  3.0f, -33.0f),  3.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-14.0f,  3.0f, -33.0f),  3.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 14.0f,  3.0f, -51.0f),  3.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-14.0f,  3.0f, -51.0f),  3.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(  0.0f, 36.0f, -38.0f),  9.0f, SOUND_BOUMm, 0.45f));
#else
        obj->AddCrashLine(CrashLine(Math::Point(-17.0f,  30.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 17.0f,  30.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 17.0f,  55.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-17.0f,  55.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-17.0f,  30.0f), SOUND_BOUMm, 0.45f));

        obj->AddCrashLine(CrashLine(Math::Point(-17.0f, -55.0f), SOUND_BOUMm, 0.45f, true));
        obj->AddCrashLine(CrashLine(Math::Point( 17.0f, -55.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 17.0f, -30.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-17.0f, -30.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-17.0f, -55.0f), SOUND_BOUMm, 0.45f));
#endif
    }

    if ( params.type == OBJECT_DOOR2 )
    {
        modelManager->AddModelReference("buzzingcars/door20.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        // machoire gauche
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        modelManager->AddModelReference("buzzingcars/door21.mod", false, rank, params.team);
        obj->SetPartPosition(1, Math::Vector(0.0f, 17.0f, 20.0f));

        // machoire droite
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(2, rank);
        obj->SetObjectParent(2, 0);
        modelManager->AddModelReference("buzzingcars/door21.mod", true, rank, params.team);
        obj->SetPartPosition(2, Math::Vector(0.0f, 17.0f, -20.0f));

        // grande roue
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(3, rank);
        obj->SetObjectParent(3, 0);
        modelManager->AddModelReference("buzzingcars/door23.mod", false, rank, params.team);
        obj->SetPartPosition(3, Math::Vector(21.0f, 9.0f, -38.0f));

        // Math::PIston
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(4, rank);
        obj->SetObjectParent(4, 0);
        modelManager->AddModelReference("buzzingcars/door24.mod", false, rank, params.team);
        obj->SetPartPosition(4, Math::Vector(26.0f, 28.0f, -38.0f));

        // bielle
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(5, rank);
        obj->SetObjectParent(5, 4);
        modelManager->AddModelReference("buzzingcars/door25.mod", false, rank, params.team);
        obj->SetPartPosition(5, Math::Vector(0.0f, 0.0f, 0.0f));

        // petite roue
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(6, rank);
        obj->SetObjectParent(6, 0);
        modelManager->AddModelReference("buzzingcars/door26.mod", false, rank, params.team);
        obj->SetPartPosition(6, Math::Vector(21.0f, 17.0f, -42.0f));

        // Sphčres déplacées par MoveCrashSphere.
        obj->AddCrashSphere(CrashSphere(Math::Vector(-15.0f, 5.0f, -23.0f),  4.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( -8.0f, 5.0f, -23.0f),  4.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(  0.0f, 5.0f, -23.0f),  4.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(  8.0f, 5.0f, -23.0f),  4.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 15.0f, 5.0f, -23.0f),  4.0f, SOUND_BOUMm, 0.45f));
        // Sphčres déplacées par MoveCrashSphere.
        obj->AddCrashSphere(CrashSphere(Math::Vector(-15.0f, 5.0f,  23.0f),  4.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( -8.0f, 5.0f,  23.0f),  4.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(  0.0f, 5.0f,  23.0f),  4.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(  8.0f, 5.0f,  23.0f),  4.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 15.0f, 5.0f,  23.0f),  4.0f, SOUND_BOUMm, 0.45f));

        // Sphčre du mécanisme.
        obj->AddCrashSphere(CrashSphere(Math::Vector( 20.0f, 4.0f, -38.0f),  7.0f, SOUND_BOUMm, 0.44f));

        obj->AddCrashSphere(CrashSphere(Math::Vector(-12.0f, 5.0f, -38.0f),  9.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(  0.0f, 5.0f, -38.0f),  9.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 12.0f, 5.0f, -38.0f),  9.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(  0.0f, 5.0f, -48.0f), 11.0f, SOUND_BOUMm, 0.45f));

        obj->AddCrashSphere(CrashSphere(Math::Vector(-12.0f, 5.0f,  38.0f),  9.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(  0.0f, 5.0f,  38.0f),  9.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 12.0f, 5.0f,  38.0f),  9.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(  0.0f, 5.0f,  48.0f), 11.0f, SOUND_BOUMm, 0.45f));
    }

    if ( params.type == OBJECT_DOOR3 )
    {
        modelManager->AddModelReference("buzzingcars/door30.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        // Math::PIston écraseur
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        modelManager->AddModelReference("buzzingcars/door31.mod", false, rank, params.team);
        obj->SetPartPosition(1, Math::Vector(0.0f, 20.0f, 0.0f));

        // grande roue
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(3, rank);
        obj->SetObjectParent(3, 0);
        modelManager->AddModelReference("buzzingcars/door23.mod", false, rank, params.team);
        obj->SetPartPosition(3, Math::Vector(21.0f, 9.0f, -43.0f));

        // Math::PIston
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(4, rank);
        obj->SetObjectParent(4, 0);
        modelManager->AddModelReference("buzzingcars/door24.mod", false, rank, params.team);
        obj->SetPartPosition(4, Math::Vector(26.0f, 28.0f, -43.0f));

        // bielle
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(5, rank);
        obj->SetObjectParent(5, 4);
        modelManager->AddModelReference("buzzingcars/door25.mod", false, rank, params.team);
        obj->SetPartPosition(5, Math::Vector(0.0f, 0.0f, 0.0f));

        // petite roue
        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(6, rank);
        obj->SetObjectParent(6, 0);
        modelManager->AddModelReference("buzzingcars/door26.mod", false, rank, params.team);
        obj->SetPartPosition(6, Math::Vector(21.0f, 17.0f, -47.0f));

        // Sphčres déplacées par MoveCrashSphere.
        obj->AddCrashSphere(CrashSphere(Math::Vector( 14.0f, 25.0f,   0.0f),  6.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-14.0f, 25.0f,   0.0f),  6.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(  0.0f, 25.0f,  14.0f),  6.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(  0.0f, 25.0f, -14.0f),  6.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 10.0f, 25.0f,  10.0f),  6.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 10.0f, 25.0f, -10.0f),  6.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-10.0f, 25.0f,  10.0f),  6.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-10.0f, 25.0f, -10.0f),  6.0f, SOUND_BOUMm, 0.45f));

        // Sphčre du mécanisme.
        obj->AddCrashSphere(CrashSphere(Math::Vector( 20.0f, 4.0f, -43.0f),  7.0f, SOUND_BOUMm, 0.44f));

#if 0
        obj->AddCrashSphere(CrashSphere(Math::Vector(  0.0f, 5.0f, -48.0f), 20.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 15.0f, 5.0f, -35.0f),  5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-15.0f, 5.0f, -35.0f),  5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 15.0f, 5.0f, -61.0f),  5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-15.0f, 5.0f, -61.0f),  5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 25.0f, 3.0f, -32.5f),  3.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-25.0f, 3.0f, -32.5f),  3.0f, SOUND_BOUMm, 0.45f));

        obj->AddCrashSphere(CrashSphere(Math::Vector(  0.0f, 5.0f,  48.0f), 20.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 15.0f, 5.0f,  35.0f),  5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-15.0f, 5.0f,  35.0f),  5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 15.0f, 5.0f,  61.0f),  5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-15.0f, 5.0f,  61.0f),  5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 25.0f, 3.0f,  32.5f),  3.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-25.0f, 3.0f,  32.5f),  3.0f, SOUND_BOUMm, 0.45f));
#else
        obj->AddCrashSphere(CrashSphere(Math::Vector( 25.0f, 3.0f, -32.5f),  3.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-25.0f, 3.0f, -32.5f),  3.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 25.0f, 3.0f,  32.5f),  3.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-25.0f, 3.0f,  32.5f),  3.0f, SOUND_BOUMm, 0.45f));

        obj->AddCrashLine(CrashLine(Math::Point(-20.0f,  30.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 20.0f,  30.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 20.0f,  67.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-20.0f,  67.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-20.0f,  30.0f), SOUND_BOUMm, 0.45f));

        obj->AddCrashLine(CrashLine(Math::Point(-20.0f, -67.0f), SOUND_BOUMm, 0.45f, true));
        obj->AddCrashLine(CrashLine(Math::Point( 20.0f, -67.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 20.0f, -30.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-20.0f, -30.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-20.0f, -67.0f), SOUND_BOUMm, 0.45f));
#endif
    }

    if ( params.type == OBJECT_DOOR4 )
    {
        modelManager->AddModelReference("buzzingcars/door40.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 2.0f,  34.0f), 6.0f, SOUND_CHOCm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 2.0f, -34.0f), 6.0f, SOUND_CHOCm, 0.45f));
    }

    if ( params.type == OBJECT_DOCK )
    {
        modelManager->AddModelReference("buzzingcars/dock1.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        modelManager->AddModelReference("buzzingcars/dock2.mod", false, rank, params.team);
        obj->SetPartPosition(1, Math::Vector(0.0f, 20.0f, 0.0f));

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(2, rank);
        obj->SetObjectParent(2, 1);
        modelManager->AddModelReference("buzzingcars/dock3.mod", false, rank, params.team);
        obj->SetPartPosition(2, Math::Vector(0.0f, 0.0f, 0.0f));

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(3, rank);
        obj->SetObjectParent(3, 2);
        modelManager->AddModelReference("buzzingcars/dock4.mod", false, rank, params.team);
        obj->SetPartPosition(3, Math::Vector(0.0f, (16.0f-14.0f)*3.0f/10.0f, 0.0f));

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(4, rank);
        obj->SetObjectParent(4, 2);
        modelManager->AddModelReference("buzzingcars/dock5.mod", false, rank, params.team);
        obj->SetPartPosition(4, Math::Vector(0.0f, (16.0f-14.0f)*6.0f/10.0f, 0.0f));

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(5, rank);
        obj->SetObjectParent(5, 2);
        modelManager->AddModelReference("buzzingcars/dock6.mod", false, rank, params.team);
        obj->SetPartPosition(5, Math::Vector(0.0f, 16.0f-14.0f, 0.0f));

        engine->GetTerrain()->AddBuildingLevel(params.pos, 7.0f, 9.0f, 1.0f, 0.5f);
        engine->GetTerrain()->AddSlowerZone(params.pos, 20.0f, 30.0f, 0.2f);

#if 0
        obj->AddCrashSphere(CrashSphere(Math::Vector(  0.0f, 6.0f, 27.0f), 15.0f, SOUND_BOUMm, 0.45f));

        obj->AddCrashSphere(CrashSphere(Math::Vector(-15.5f, 2.0f,-12.0f),  1.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-15.0f, 7.0f,-12.0f),  1.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-15.5f, 2.0f, 12.0f),  1.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-14.0f, 2.0f, 17.0f),  2.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-14.0f, 2.0f, 22.0f),  2.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-14.0f, 2.0f, 27.0f),  2.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-14.0f, 2.0f, 32.0f),  2.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-14.0f, 2.0f, 37.0f),  2.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-15.5f, 2.0f, 42.0f),  1.5f, SOUND_BOUMm, 0.45f));

        obj->AddCrashSphere(CrashSphere(Math::Vector( 15.5f, 2.0f,-12.0f),  1.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 15.0f, 7.0f,-12.0f),  1.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 15.5f, 2.0f, 12.0f),  1.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 14.0f, 2.0f, 17.0f),  2.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 14.0f, 2.0f, 22.0f),  2.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 14.0f, 2.0f, 27.0f),  2.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 14.0f, 2.0f, 32.0f),  2.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 14.0f, 2.0f, 37.0f),  2.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 15.5f, 2.0f, 42.0f),  1.5f, SOUND_BOUMm, 0.45f));

        obj->AddCrashSphere(CrashSphere(Math::Vector(-10.0f, 2.0f, 14.0f),  2.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( -5.0f, 2.0f, 14.0f),  2.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(  0.0f, 2.0f, 14.0f),  2.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 10.0f, 2.0f, 14.0f),  2.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 10.0f, 2.0f, 14.0f),  2.5f, SOUND_BOUMm, 0.45f));

        obj->AddCrashSphere(CrashSphere(Math::Vector(-10.0f, 2.0f, 40.0f),  2.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( -5.0f, 2.0f, 40.0f),  2.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(  0.0f, 2.0f, 40.0f),  2.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 10.0f, 2.0f, 40.0f),  2.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 10.0f, 2.0f, 40.0f),  2.5f, SOUND_BOUMm, 0.45f));
#else
        obj->AddCrashSphere(CrashSphere(Math::Vector(-15.5f, 2.0f,-12.0f),  1.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector(-15.0f, 7.0f,-12.0f),  1.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 15.5f, 2.0f,-12.0f),  1.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 15.0f, 7.0f,-12.0f),  1.5f, SOUND_BOUMm, 0.45f));

        obj->AddCrashLine(CrashLine(Math::Point(-15.0f, 12.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 15.0f, 12.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 15.0f, 42.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-15.0f, 42.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-15.0f, 12.0f), SOUND_BOUMm, 0.45f));
#endif
    }

    if ( params.type == OBJECT_REMOTE )
    {
        modelManager->AddModelReference("buzzingcars/remote1.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        modelManager->AddModelReference("buzzingcars/remote2.mod", false, rank, params.team);
        obj->SetPartPosition(1, Math::Vector(3.6f, 4.4f, 0.0f));

        engine->GetTerrain()->AddBuildingLevel(params.pos, 7.0f, 9.0f, 1.0f, 0.5f);

        obj->AddCrashSphere(CrashSphere(Math::Vector( 6.0f, 3.0f,  4.0f), 1.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 6.0f, 3.0f,  0.0f), 1.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(Math::Vector( 6.0f, 3.0f, -4.0f), 1.0f, SOUND_BOUMm, 0.45f));
    }

    if ( params.type == OBJECT_STAND )
    {
        modelManager->AddModelReference("buzzingcars/stand.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        engine->GetTerrain()->AddBuildingLevel(params.pos, 7.0f, 9.0f, 1.0f, 0.5f);

#if 1
        obj->AddCrashLine(CrashLine(Math::Point(-30.0f, -60.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 22.0f, -60.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point( 22.0f,  60.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-30.0f,  60.0f), SOUND_BOUMm, 0.45f));
        obj->AddCrashLine(CrashLine(Math::Point(-30.0f, -60.0f), SOUND_BOUMm, 0.45f));
#endif
    }

    if ( params.type == OBJECT_GENERATOR )
    {
        modelManager->AddModelReference("buzzingcars/generat.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);
    }

    /*if ( params.type == OBJECT_START )
    {
        modelManager->AddModelReference("buzzingcars/start.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        engine->GetTerrain()->AddBuildingLevel(params.pos, 7.0f, 9.0f, 1.0f, 0.5f);
    }

    if ( params.type == OBJECT_END )
    {
        modelManager->AddModelReference("buzzingcars/end.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        engine->GetTerrain()->AddBuildingLevel(params.pos, 7.0f, 9.0f, 1.0f, 0.5f);
    }*/

    if ( params.type == OBJECT_SUPPORT )
    {
        modelManager->AddModelReference("buzzingcars/support.mod", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);
    }

    Math::Vector pos = obj->GetPosition();
    pos.y += height;
    obj->SetPosition(pos);  // to display the shadows immediately

    std::unique_ptr<CAuto> objAuto;
    /*if ( params.type == OBJECT_TOWER )
    {
        objAuto = MakeUnique<CAutoTower>(obj.get());
    }*/
    if ( params.type == OBJECT_DOOR1 ||
         params.type == OBJECT_DOOR2 ||
         params.type == OBJECT_DOOR3 ||
         params.type == OBJECT_DOOR4 )
    {
        objAuto = MakeUnique<CAutoDoor>(obj.get());
    }
    if ( params.type == OBJECT_ALIEN2 ||
         params.type == OBJECT_ALIEN3 ||
         params.type == OBJECT_ALIEN5 ||
         params.type == OBJECT_ALIEN6 ||
         params.type == OBJECT_ALIEN7 ||
         params.type == OBJECT_ALIEN8 ||
         params.type == OBJECT_ALIEN9 )
    {
        objAuto = MakeUnique<CAutoAlien>(obj.get());
    }
    if ( params.type == OBJECT_DOCK )
    {
        objAuto = MakeUnique<CAutoDock>(obj.get());
    }
    if ( params.type == OBJECT_REMOTE )
    {
        objAuto = MakeUnique<CAutoRemote>(obj.get());
    }
    if ( params.type == OBJECT_STAND )
    {
        objAuto = MakeUnique<CAutoStand>(obj.get());
    }
    if ( params.type == OBJECT_GENERATOR )
    {
        objAuto = MakeUnique<CAutoGenerator>(obj.get());
    }
    if ( params.type == OBJECT_COMPUTER )
    {
        objAuto = MakeUnique<CAutoComputer>(obj.get());
    }
    /*if ( params.type == OBJECT_REPAIR )
    {
        objAuto = MakeUnique<CAutoRepair>(obj.get());
    }*/
    if ( params.type == OBJECT_INCA7 )
    {
        objAuto = MakeUnique<CAutoInca>(obj.get());
    }
    if ( params.type == OBJECT_CARCASS1  ||
         params.type == OBJECT_CARCASS2  ||
         params.type == OBJECT_CARCASS3  ||
         params.type == OBJECT_CARCASS4  ||
         params.type == OBJECT_CARCASS5  ||
         params.type == OBJECT_CARCASS6  ||
         params.type == OBJECT_CARCASS7  ||
         params.type == OBJECT_CARCASS8  ||
         params.type == OBJECT_CARCASS9  ||
         params.type == OBJECT_CARCASS10 )
    {
        objAuto = MakeUnique<CAutoBarrel>(obj.get());
    }
    /*if ( params.type == OBJECT_BOMB )
    {
        objAuto = MakeUnique<CAutoBomb>(obj.get());
    }*/
    if ( params.type == OBJECT_HOME1 || //?
         params.type == OBJECT_HOME2 ||
         params.type == OBJECT_HOME3 ||
         params.type == OBJECT_HOME4 ||
         params.type == OBJECT_HOME5 )
    {
        objAuto = MakeUnique<CAutoHome>(obj.get());
    }
    /* TODO: this is the same as Colobot, right?
    if ( params.type == OBJECT_ROOT5 )
    {
        objAuto = MakeUnique<CAutoRoot>(obj.get());
    }
    */

    if (objAuto != nullptr)
    {
        objAuto->Init();
        obj->SetAuto(std::move(objAuto));
    }

    engine->LoadAllTextures();

    return obj;
}
