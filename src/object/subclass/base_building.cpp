/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "object/subclass/base_building.h"

#include "graphics/engine/engine.h"
#include "graphics/engine/oldmodelmanager.h"
#include "graphics/engine/terrain.h"

#include "math/geometry.h"

#include "object/object_create_params.h"

#include "object/auto/autobase.h"
#include "object/auto/autoconvert.h"
#include "object/auto/autoderrick.h"
#include "object/auto/autodestroyer.h"
#include "object/auto/autofactory.h"
#include "object/auto/autohouston.h"
#include "object/auto/autolabo.h"
#include "object/auto/autonest.h"
#include "object/auto/autonuclearplant.h"
#include "object/auto/autoportico.h"
#include "object/auto/autopowercaptor.h"
#include "object/auto/autopowerplant.h"
#include "object/auto/autopowerstation.h"
#include "object/auto/autoradar.h"
#include "object/auto/autorepair.h"
#include "object/auto/autoresearch.h"
#include "object/auto/autotower.h"
#include "object/auto/autovault.h"


CBaseBuilding::CBaseBuilding(int id, ObjectType type)
    : COldObject(id)
{
    SetType(type);
}

CBaseBuilding::~CBaseBuilding()
{}

std::unique_ptr<CBaseBuilding> CBaseBuilding::Create(
    const ObjectCreateParams& params,
    Gfx::COldModelManager* modelManager,
    Gfx::CEngine* engine)
{
    auto obj = std::make_unique<CBaseBuilding>(params.id, params.type);

    obj->SetTrainer(params.trainer || obj->GetPlusTrainer());
    obj->SetTeam(params.team);

    float height = params.height;

    int rank = engine->CreateObject();
    engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);  // it is a stationary object
    obj->SetObjectRank(0, rank);

    if ( params.type == OBJECT_PORTICO )
    {
        modelManager->AddModelReference("portico1", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        modelManager->AddModelReference("portico2", false, rank, params.team);
        obj->SetPartPosition(1, glm::vec3(0.0f, 67.0f, 0.0f));

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(2, rank);
        obj->SetObjectParent(2, 1);
        modelManager->AddModelReference("portico3", false, rank, params.team);
        obj->SetPartPosition(2, glm::vec3(0.0f, 0.0f, -33.0f));
        obj->SetPartRotationY(2, 45.0f*Math::PI/180.0f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(3, rank);
        obj->SetObjectParent(3, 2);
        modelManager->AddModelReference("portico4", false, rank, params.team);
        obj->SetPartPosition(3, glm::vec3(50.0f, 0.0f, 0.0f));
        obj->SetPartRotationY(3, -60.0f*Math::PI/180.0f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(4, rank);
        obj->SetObjectParent(4, 3);
        modelManager->AddModelReference("portico5", false, rank, params.team);
        obj->SetPartPosition(4, glm::vec3(35.0f, 0.0f, 0.0f));
        obj->SetPartRotationY(4, -55.0f*Math::PI/180.0f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(5, rank);
        obj->SetObjectParent(5, 1);
        modelManager->AddModelReference("portico3", false, rank, params.team);
        obj->SetPartPosition(5, glm::vec3(0.0f, 0.0f, 33.0f));
        obj->SetPartRotationY(5, -45.0f*Math::PI/180.0f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(6, rank);
        obj->SetObjectParent(6, 5);
        modelManager->AddModelReference("portico4", false, rank, params.team);
        obj->SetPartPosition(6, glm::vec3(50.0f, 0.0f, 0.0f));
        obj->SetPartRotationY(6, 60.0f*Math::PI/180.0f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(7, rank);
        obj->SetObjectParent(7, 6);
        modelManager->AddModelReference("portico5", false, rank, params.team);
        obj->SetPartPosition(7, glm::vec3(35.0f, 0.0f, 0.0f));
        obj->SetPartRotationY(7, 55.0f*Math::PI/180.0f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(8, rank);
        obj->SetObjectParent(8, 0);
        modelManager->AddModelReference("portico6", false, rank, params.team);
        obj->SetPartPosition(8, glm::vec3(-35.0f, 50.0f, -35.0f));
        obj->SetPartRotationY(8, -Math::PI/2.0f);
        obj->SetPartScale(8, 2.0f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(9, rank);
        obj->SetObjectParent(9, 8);
        modelManager->AddModelReference("portico7", false, rank, params.team);
        obj->SetPartPosition(9, glm::vec3(0.0f, 4.5f, 1.9f));

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(10, rank);
        obj->SetObjectParent(10, 0);
        modelManager->AddModelReference("portico6", false, rank, params.team);
        obj->SetPartPosition(10, glm::vec3(-35.0f, 50.0f, 35.0f));
        obj->SetPartRotationY(10, -Math::PI/2.0f);
        obj->SetPartScale(10, 2.0f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(11, rank);
        obj->SetObjectParent(11, 10);
        modelManager->AddModelReference("portico7", false, rank, params.team);
        obj->SetPartPosition(11, glm::vec3(0.0f, 4.5f, 1.9f));

        obj->AddCrashSphere(CrashSphere(glm::vec3(  0.0f, 28.0f,   0.0f), 45.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3( 27.0f, 10.0f, -42.0f), 15.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(  0.0f, 10.0f, -42.0f), 15.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(-27.0f, 10.0f, -42.0f), 15.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3( 27.0f, 10.0f,  42.0f), 15.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(  0.0f, 10.0f,  42.0f), 15.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(-27.0f, 10.0f,  42.0f), 15.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(-32.0f, 45.0f, -32.0f), 10.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(-32.0f, 45.0f,  32.0f), 10.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3( 32.0f, 45.0f, -32.0f), 10.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3( 32.0f, 45.0f,  32.0f), 10.0f, SOUND_BOUMm, 0.45f));
        obj->SetCameraCollisionSphere(Math::Sphere(glm::vec3(0.0f, 35.0f, 0.0f), 50.0f));

        obj->CreateShadowCircle(50.0f, 1.0f);
    }

    if ( params.type == OBJECT_BASE )
    {
        modelManager->AddModelReference("base1", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        for (int i=0 ; i<8 ; i++ )
        {
            rank = engine->CreateObject();
            engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
            obj->SetObjectRank(1+i, rank);
            obj->SetObjectParent(1+i, 0);
            modelManager->AddModelReference("base2", false, rank, params.team);
            glm::vec2 p = Math::RotatePoint(-Math::PI/4.0f*i, 27.8f);
            obj->SetPartPosition(1+i, glm::vec3(p.x, 30.0f, p.y));
            obj->SetPartRotationY(1+i, Math::PI/4.0f*i);
            obj->SetPartRotationZ(1+i, Math::PI/2.0f);

            rank = engine->CreateObject();
            engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
            obj->SetObjectRank(10+i, rank);
            obj->SetObjectParent(10+i, 1+i);
            modelManager->AddModelReference("base4", false, rank, params.team);
            obj->SetPartPosition(10+i, glm::vec3(23.5f, 0.0f, 7.0f));

            rank = engine->CreateObject();
            engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
            obj->SetObjectRank(18+i, rank);
            obj->SetObjectParent(18+i, 1+i);
            modelManager->AddModelReference("base4", true, rank, params.team);
            obj->SetPartPosition(18+i, glm::vec3(23.5f, 0.0f, -7.0f));
        }

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(9, rank);
        obj->SetObjectParent(9, 0);
        modelManager->AddModelReference("base3", false, rank, params.team); // central pillar

        obj->AddCrashSphere(CrashSphere(glm::vec3(  0.0f, 33.0f,   0.0f),  2.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(  0.0f, 39.0f,   0.0f),  2.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(  0.0f, 45.0f,   0.0f),  2.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(  0.0f, 51.0f,   0.0f),  2.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(  0.0f, 57.0f,   0.0f),  2.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(  0.0f, 63.0f,   0.0f),  2.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(  0.0f, 69.0f,   0.0f),  2.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(  0.0f, 82.0f,   0.0f),  8.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3( 18.0f, 94.0f,   0.0f), 10.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(-18.0f, 94.0f,   0.0f), 10.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(  0.0f, 94.0f,  18.0f), 10.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(  0.0f, 94.0f, -18.0f), 10.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3( 13.0f, 94.0f,  13.0f), 10.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(-13.0f, 94.0f,  13.0f), 10.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3( 13.0f, 94.0f, -13.0f), 10.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(-13.0f, 94.0f, -13.0f), 10.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(  0.0f,104.0f,   0.0f), 14.0f, SOUND_BOUMm, 0.45f));

        obj->SetCameraCollisionSphere(Math::Sphere(glm::vec3(0.0f, 45.0f, 0.0f), 10.0f));

        obj->CreateShadowCircle(60.0f, 1.0f);

        engine->GetTerrain()->AddBuildingLevel(params.pos, 28.6f, 73.4f, 30.0f, 0.4f);
    }

    if ( params.type == OBJECT_DERRICK )
    {
        modelManager->AddModelReference("derrick1", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        modelManager->AddModelReference("derrick2", false, rank, params.team);

        obj->AddCrashSphere(CrashSphere(glm::vec3(0.0f,  0.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(0.0f, 10.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(0.0f, 17.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(0.0f, 26.0f, 0.0f), 3.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(7.0f, 17.0f, 0.0f), 3.0f, SOUND_BOUMm, 0.45f));
        obj->SetCameraCollisionSphere(Math::Sphere(glm::vec3(0.0f, 10.0f, 0.0f), 10.0f));

        obj->CreateShadowCircle(10.0f, 0.4f);
    }

    if ( params.type == OBJECT_RESEARCH )
    {
        modelManager->AddModelReference("search1", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        modelManager->AddModelReference("search2", false, rank, params.team);
        obj->SetPartPosition(1, glm::vec3(0.0f, 13.0f, 0.0f));

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(2, rank);
        obj->SetObjectParent(2, 1);
        modelManager->AddModelReference("search3", false, rank, params.team);
        obj->SetPartPosition(2, glm::vec3(0.0f, 4.0f, 0.0f));
        obj->SetPartRotationZ(2, 35.0f*Math::PI/180.0f);

        obj->AddCrashSphere(CrashSphere(glm::vec3(0.0f,  0.0f, 0.0f), 9.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(0.0f,  6.0f, 0.0f), 9.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(0.0f, 14.0f, 0.0f), 7.0f, SOUND_BOUMm, 0.45f));
        obj->SetCameraCollisionSphere(Math::Sphere(glm::vec3(0.0f, 8.0f, 0.0f), 12.0f));

        obj->SetPowerPosition(glm::vec3(7.5f, 3.0f, 0.0f));

        obj->CreateShadowCircle(12.0f, 1.0f);
    }

    if ( params.type == OBJECT_RADAR )
    {
        modelManager->AddModelReference("radar1", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        modelManager->AddModelReference("radar2", false, rank, params.team);
        obj->SetPartPosition(1, glm::vec3(0.0f, 5.0f, 0.0f));

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(2, rank);
        obj->SetObjectParent(2, 0);
        modelManager->AddModelReference("radar3", false, rank, params.team);
        obj->SetPartPosition(2, glm::vec3(0.0f, 11.0f, 0.0f));
        obj->SetPartRotationY(2, -Math::PI/2.0f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(3, rank);
        obj->SetObjectParent(3, 2);
        modelManager->AddModelReference("radar4", false, rank, params.team);
        obj->SetPartPosition(3, glm::vec3(0.0f, 4.5f, 1.9f));

        obj->AddCrashSphere(CrashSphere(glm::vec3(0.0f,  3.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(0.0f, 11.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f));
        obj->SetCameraCollisionSphere(Math::Sphere(glm::vec3(0.0f, 7.0f, 0.0f), 7.0f));

        obj->CreateShadowCircle(8.0f, 1.0f);
    }

    if ( params.type == OBJECT_ENERGY )
    {
        modelManager->AddModelCopy("energy", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        obj->AddCrashSphere(CrashSphere(glm::vec3(-2.0f, 13.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(-7.0f,  3.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3( 0.0f,  1.0f, 0.0f), 1.5f, SOUND_BOUMm, 0.45f));
        obj->SetCameraCollisionSphere(Math::Sphere(glm::vec3(-7.0f, 5.0f, 0.0f), 5.0f));

        obj->SetPowerPosition(glm::vec3(0.0f, 3.0f, 0.0f));
        obj->SetEnergyLevel(params.power);  // initializes the energy level

        obj->CreateShadowCircle(6.0f, 0.5f);
    }

    if ( params.type == OBJECT_LABO )
    {
        modelManager->AddModelReference("labo1", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        modelManager->AddModelReference("labo2", false, rank, params.team);
        obj->SetPartPosition(1, glm::vec3(-9.0f, 3.0f, 0.0f));
        obj->SetPartRotationZ(1, Math::PI/2.0f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(2, rank);
        obj->SetObjectParent(2, 1);
        modelManager->AddModelReference("labo3", false, rank, params.team);
        obj->SetPartPosition(2, glm::vec3(9.0f, -1.0f, 0.0f));

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(3, rank);
        obj->SetObjectParent(3, 2);
        modelManager->AddModelReference("labo4", false, rank, params.team);
        obj->SetPartPosition(3, glm::vec3(0.0f, 0.0f, 0.0f));
        obj->SetPartRotationZ(3, 80.0f*Math::PI/180.0f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(4, rank);
        obj->SetObjectParent(4, 2);
        modelManager->AddModelReference("labo4", false, rank, params.team);
        obj->SetPartPosition(4, glm::vec3(0.0f, 0.0f, 0.0f));
        obj->SetPartRotationZ(4, 80.0f*Math::PI/180.0f);
        obj->SetPartRotationY(4, Math::PI*2.0f/3.0f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(5, rank);
        obj->SetObjectParent(5, 2);
        modelManager->AddModelReference("labo4", false, rank, params.team);
        obj->SetPartPosition(5, glm::vec3(0.0f, 0.0f, 0.0f));
        obj->SetPartRotationZ(5, 80.0f*Math::PI/180.0f);
        obj->SetPartRotationY(5, -Math::PI*2.0f/3.0f);

        obj->AddCrashSphere(CrashSphere(glm::vec3(  0.0f,  1.0f,  0.0f), 1.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(  0.0f, 11.0f,  0.0f), 4.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(-10.0f, 10.0f,  0.0f), 4.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(-12.0f,  3.0f,  3.0f), 4.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(-12.0f,  3.0f, -3.0f), 4.0f, SOUND_BOUMm, 0.45f));
        obj->SetCameraCollisionSphere(Math::Sphere(glm::vec3(-10.0f, 5.0f, 0.0f), 7.0f));

        obj->SetPowerPosition(glm::vec3(0.0f, 3.0f, 0.0f));

        obj->CreateShadowCircle(7.0f, 0.5f);
    }

    if ( params.type == OBJECT_FACTORY )
    {
        modelManager->AddModelReference("factory1", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        for (int i=0 ; i<9 ; i++ )
        {
            rank = engine->CreateObject();
            engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
            obj->SetObjectRank(1+i, rank);
            obj->SetObjectParent(1+i, 0);
            modelManager->AddModelReference("factory2", false, rank, params.team);
            obj->SetPartPosition(1+i, glm::vec3(10.0f, 2.0f*i, 10.0f));
            obj->SetPartRotationZ(1+i, Math::PI/2.0f);
            obj->SetPartScaleZ(1+i, 0.30f);

            rank = engine->CreateObject();
            engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
            obj->SetObjectRank(10+i, rank);
            obj->SetObjectParent(10+i, 0);
            modelManager->AddModelReference("factory2", false, rank, params.team);
            obj->SetPartPosition(10+i, glm::vec3(10.0f, 2.0f*i, -10.0f));
            obj->SetPartRotationZ(10+i, -Math::PI/2.0f);
            obj->SetPartRotationY(10+i, Math::PI);
            obj->SetPartScaleZ(10+i, 0.30f);
        }

        for (int i=0 ; i<2 ; i++ )
        {
            float s = static_cast<float>(i*2-1);
            obj->AddCrashSphere(CrashSphere(glm::vec3(-10.0f,  2.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f));
            obj->AddCrashSphere(CrashSphere(glm::vec3( -3.0f,  2.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f));
            obj->AddCrashSphere(CrashSphere(glm::vec3(  3.0f,  2.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f));
            obj->AddCrashSphere(CrashSphere(glm::vec3( 10.0f,  2.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f));
            obj->AddCrashSphere(CrashSphere(glm::vec3(-10.0f,  9.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f));
            obj->AddCrashSphere(CrashSphere(glm::vec3( -3.0f,  9.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f));
            obj->AddCrashSphere(CrashSphere(glm::vec3(  3.0f,  9.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f));
            obj->AddCrashSphere(CrashSphere(glm::vec3( 10.0f,  9.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f));
            obj->AddCrashSphere(CrashSphere(glm::vec3(-10.0f, 16.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f));
            obj->AddCrashSphere(CrashSphere(glm::vec3( -3.0f, 16.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f));
            obj->AddCrashSphere(CrashSphere(glm::vec3(  3.0f, 16.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f));
            obj->AddCrashSphere(CrashSphere(glm::vec3( 10.0f, 16.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f));
            obj->AddCrashSphere(CrashSphere(glm::vec3(-10.0f, 16.0f,  4.0f*s), 4.0f, SOUND_BOUMm, 0.45f));
            obj->AddCrashSphere(CrashSphere(glm::vec3( -3.0f, 16.0f,  4.0f*s), 4.0f, SOUND_BOUMm, 0.45f));
            obj->AddCrashSphere(CrashSphere(glm::vec3(  3.0f, 16.0f,  4.0f*s), 4.0f, SOUND_BOUMm, 0.45f));
            obj->AddCrashSphere(CrashSphere(glm::vec3( 10.0f, 16.0f,  4.0f*s), 4.0f, SOUND_BOUMm, 0.45f));
            obj->AddCrashSphere(CrashSphere(glm::vec3(-10.0f,  2.0f,  4.0f*s), 4.0f, SOUND_BOUMm, 0.45f));
            obj->AddCrashSphere(CrashSphere(glm::vec3(-10.0f,  9.0f,  4.0f*s), 4.0f, SOUND_BOUMm, 0.45f));
        }
        obj->AddCrashSphere(CrashSphere(glm::vec3(-10.0f, 21.0f, -4.0f), 3.0f, SOUND_BOUMm, 0.45f));
        obj->SetCameraCollisionSphere(Math::Sphere(glm::vec3(0.0f, 10.0f, 0.0f), 18.0f));

        obj->CreateShadowCircle(24.0f, 0.3f);
    }

    if ( params.type == OBJECT_REPAIR )
    {
        modelManager->AddModelReference("repair1", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        modelManager->AddModelReference("repair2", false, rank, params.team);
        obj->SetPartPosition(1, glm::vec3(-11.0f, 13.5f, 0.0f));
        obj->SetPartRotationZ(1, Math::PI/2.0f);

        engine->GetTerrain()->AddBuildingLevel(params.pos, 7.0f, 9.0f, 1.0f, 0.5f);

        obj->AddCrashSphere(CrashSphere(glm::vec3(-11.0f,  0.0f,  4.0f), 5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(-11.0f,  0.0f,  0.0f), 5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(-11.0f,  0.0f, -4.0f), 5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(-11.0f, 10.0f,  0.0f), 5.0f, SOUND_BOUMm, 0.45f));
        obj->SetCameraCollisionSphere(Math::Sphere(glm::vec3(-11.0f, 13.0f, 0.0f), 15.0f));
    }

    if ( params.type == OBJECT_DESTROYER )
    {
        modelManager->AddModelReference("destroy1", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        modelManager->AddModelReference("destroy2", false, rank, params.team);
        obj->SetPartPosition(1, glm::vec3(0.0f, 0.0f, 0.0f));

        engine->GetTerrain()->AddBuildingLevel(params.pos, 7.0f, 9.0f, 1.0f, 0.5f);

        obj->AddCrashSphere(CrashSphere(glm::vec3(-3.5f, 0.0f, -13.5f), 4.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3( 3.5f, 0.0f, -13.5f), 4.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(-3.5f, 0.0f,  13.5f), 4.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3( 3.5f, 0.0f,  13.5f), 4.0f, SOUND_BOUMm, 0.45f));

        obj->CreateShadowCircle(19.0f, 1.0f);
    }

    if ( params.type == OBJECT_STATION )
    {
        modelManager->AddModelCopy("station", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        engine->GetTerrain()->AddBuildingLevel(params.pos, 7.0f, 9.0f, 1.0f, 0.5f);

        obj->AddCrashSphere(CrashSphere(glm::vec3(-15.0f, 2.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(-15.0f, 6.0f, 0.0f), 4.0f, SOUND_BOUMm, 0.45f));
        obj->SetCameraCollisionSphere(Math::Sphere(glm::vec3(-15.0f, 5.0f, 0.0f), 6.0f));

        obj->SetEnergyLevel(params.power);
    }

    if ( params.type == OBJECT_CONVERT )
    {
        modelManager->AddModelReference("convert1", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        modelManager->AddModelReference("convert2", false, rank, params.team);
        obj->SetPartPosition(1, glm::vec3(0.0f, 14.0f, 0.0f));

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(2, rank);
        obj->SetObjectParent(2, 0);
        modelManager->AddModelReference("convert3", false, rank, params.team);
        obj->SetPartPosition(2, glm::vec3(0.0f, 11.5f, 0.0f));
        obj->SetPartRotationX(2, -Math::PI*0.35f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(3, rank);
        obj->SetObjectParent(3, 0);
        modelManager->AddModelReference("convert3", false, rank, params.team);
        obj->SetPartPosition(3, glm::vec3(0.0f, 11.5f, 0.0f));
        obj->SetPartRotationY(3, Math::PI);
        obj->SetPartRotationX(3, -Math::PI*0.35f);

        engine->GetTerrain()->AddBuildingLevel(params.pos, 7.0f, 9.0f, 1.0f, 0.5f);

        obj->AddCrashSphere(CrashSphere(glm::vec3(-10.0f,  2.0f,  4.0f), 5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(-10.0f,  2.0f, -4.0f), 5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(-10.0f,  9.0f,  0.0f), 6.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(  0.0f, 14.0f,  0.0f), 1.5f, SOUND_BOUMm, 0.45f));
        obj->SetCameraCollisionSphere(Math::Sphere(glm::vec3(-3.0f, 8.0f, 0.0f), 14.0f));
    }

    if ( params.type == OBJECT_TOWER )
    {
        modelManager->AddModelReference("tower", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        modelManager->AddModelReference("roller2c", false, rank, params.team);
        obj->SetPartPosition(1, glm::vec3(0.0f, 20.0f, 0.0f));
        obj->SetPartRotationZ(1, Math::PI/2.0f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(2, rank);
        obj->SetObjectParent(2, 1);
        modelManager->AddModelReference("roller3c", false, rank, params.team);
        obj->SetPartPosition(2, glm::vec3(4.5f, 0.0f, 0.0f));
        obj->SetPartRotationZ(2, 0.0f);

        obj->AddCrashSphere(CrashSphere(glm::vec3(0.0f,  0.0f, 0.0f), 6.5f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(0.0f,  8.0f, 0.0f), 4.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(0.0f, 15.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(0.0f, 24.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f));
        obj->SetCameraCollisionSphere(Math::Sphere(glm::vec3(0.0f, 5.0f, 0.0f), 7.0f));

        obj->SetPowerPosition(glm::vec3(5.0f, 3.0f, 0.0f));

        obj->CreateShadowCircle(6.0f, 1.0f);
    }

    if ( params.type == OBJECT_NUCLEAR )
    {
        modelManager->AddModelReference("nuclear1", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        modelManager->AddModelReference("nuclear2", false, rank, params.team);
        obj->SetPartPosition(1, glm::vec3(20.0f, 10.0f, 0.0f));
        obj->SetPartRotationZ(1, 135.0f*Math::PI/180.0f);

        obj->AddCrashSphere(CrashSphere(glm::vec3( 0.0f,  0.0f, 0.0f), 19.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3( 0.0f, 24.0f, 0.0f), 15.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(22.0f,  1.0f, 0.0f),  1.5f, SOUND_BOUMm, 0.45f));
        obj->SetCameraCollisionSphere(Math::Sphere(glm::vec3(0.0f, 17.0f, 0.0f), 26.0f));

        obj->SetPowerPosition(glm::vec3(22.0f, 3.0f, 0.0f));

        obj->CreateShadowCircle(21.0f, 1.0f);
    }

    if ( params.type == OBJECT_PARA )
    {
        modelManager->AddModelReference("para", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        engine->GetTerrain()->AddBuildingLevel(params.pos, 16.0f, 18.0f, 1.0f, 0.5f);

        obj->AddCrashSphere(CrashSphere(glm::vec3( 13.0f,  3.0f,  13.0f),  3.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3( 11.0f, 15.0f,  11.0f),  2.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(-13.0f,  3.0f,  13.0f),  3.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(-11.0f, 15.0f, -11.0f),  2.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3( 13.0f,  3.0f, -13.0f),  3.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3( 11.0f, 15.0f, -11.0f),  2.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(-13.0f,  3.0f, -13.0f),  3.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(-11.0f, 15.0f, -11.0f),  2.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(  0.0f, 26.0f,   0.0f),  9.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(  0.0f, 54.0f,   0.0f), 14.0f, SOUND_BOUMm, 0.45f));
        obj->SetCameraCollisionSphere(Math::Sphere(glm::vec3(0.0f, 10.0f, 0.0f), 20.0f));

        obj->CreateShadowCircle(21.0f, 1.0f);
    }

    if ( params.type == OBJECT_SAFE )
    {
        modelManager->AddModelReference("safe1", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        modelManager->AddModelReference("safe2", false, rank, params.team);
        obj->SetPartScale(1, 1.05f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(2, rank);
        obj->SetObjectParent(2, 0);
        modelManager->AddModelReference("safe3", false, rank, params.team);
        obj->SetPartScale(2, 1.05f);

        engine->GetTerrain()->AddBuildingLevel(params.pos, 18.0f, 20.0f, 1.0f, 0.5f);

        obj->AddCrashSphere(CrashSphere(glm::vec3(0.0f, 1.0f, 0.0f), 13.0f, SOUND_BOUMm, 0.45f));
        obj->SetCameraCollisionSphere(Math::Sphere(glm::vec3(0.0f, 1.0f, 0.0f), 13.0f));

        obj->CreateShadowCircle(23.0f, 1.0f);
    }

    if ( params.type == OBJECT_HUSTON )
    {
        modelManager->AddModelReference("huston1", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        modelManager->AddModelReference("huston2", false, rank, params.team);
        obj->SetPartPosition(1, glm::vec3(0.0f, 39.0f, 30.0f));
        obj->SetPartRotationY(1, -Math::PI/2.0f);
        obj->SetPartScale(1, 3.0f);

        rank = engine->CreateObject();
        engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(2, rank);
        obj->SetObjectParent(2, 1);
        modelManager->AddModelReference("huston3", false, rank, params.team);
        obj->SetPartPosition(2, glm::vec3(0.0f, 4.5f, 1.9f));

        obj->AddCrashSphere(CrashSphere(glm::vec3( 15.0f,  6.0f, -53.0f), 16.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(-15.0f,  6.0f, -53.0f), 16.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3( 15.0f,  6.0f, -26.0f), 16.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(-15.0f,  6.0f, -26.0f), 16.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3( 15.0f,  6.0f,   0.0f), 16.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(-15.0f,  6.0f,   0.0f), 16.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3( 15.0f,  6.0f,  26.0f), 16.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(-15.0f,  6.0f,  26.0f), 16.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3( 15.0f,  6.0f,  53.0f), 16.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(-15.0f,  6.0f,  53.0f), 16.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(  0.0f, 27.0f,  30.0f), 12.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(  0.0f, 45.0f,  30.0f), 14.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3( 26.0f,  4.0f, -61.0f),  5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(-26.0f,  4.0f, -61.0f),  5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3( 26.0f,  4.0f,  61.0f),  5.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(-26.0f,  4.0f,  61.0f),  5.0f, SOUND_BOUMm, 0.45f));
    }

    if ( params.type == OBJECT_TARGET1 )
    {
        modelManager->AddModelReference("target1", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetScale(1.5f);
        obj->SetFloorHeight(0.0f);

        obj->AddCrashSphere(CrashSphere(glm::vec3(  0.0f, 50.0f+14.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3( -7.0f, 50.0f+12.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(  7.0f, 50.0f+12.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(-12.0f, 50.0f+ 7.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3( 12.0f, 50.0f+ 7.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(-14.0f, 50.0f+ 0.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3( 14.0f, 50.0f+ 0.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(-12.0f, 50.0f- 7.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3( 12.0f, 50.0f- 7.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3( -7.0f, 50.0f-12.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(  7.0f, 50.0f-12.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(  0.0f, 50.0f-14.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f));

        obj->AddCrashSphere(CrashSphere(glm::vec3(0.0f, 30.0f, 0.0f), 2.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(0.0f, 24.0f, 0.0f), 3.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(0.0f, 16.0f, 0.0f), 4.0f, SOUND_BOUMm, 0.45f));
        obj->AddCrashSphere(CrashSphere(glm::vec3(0.0f,  4.0f, 0.0f), 8.0f, SOUND_BOUMm, 0.45f));

        obj->CreateShadowCircle(15.0f, 1.0f);
    }

    if ( params.type == OBJECT_TARGET2 )
    {
        modelManager->AddModelReference("target2", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        height += 50.0f*1.5f;
    }

    if ( params.type == OBJECT_NEST )
    {
        modelManager->AddModelReference("nest", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        engine->GetTerrain()->AddBuildingLevel(params.pos, 3.0f, 5.0f, 1.0f, 0.5f);

        obj->CreateShadowCircle(4.0f, 1.0f);
    }

    if ( params.type == OBJECT_START )
    {
        modelManager->AddModelReference("start", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        engine->GetTerrain()->AddBuildingLevel(params.pos, 7.0f, 9.0f, 1.0f, 0.5f);
    }

    if ( params.type == OBJECT_END )
    {
        modelManager->AddModelReference("end", false, rank, params.team);
        obj->SetPosition(params.pos);
        obj->SetRotationY(params.angle);
        obj->SetFloorHeight(0.0f);

        engine->GetTerrain()->AddBuildingLevel(params.pos, 7.0f, 9.0f, 1.0f, 0.5f);
    }

    glm::vec3 pos = obj->GetPosition();
    pos.y += height;
    obj->SetPosition(pos);  // to display the shadows immediately

    std::unique_ptr<CAuto> objAuto;
    if ( params.type == OBJECT_BASE )
    {
        objAuto = std::make_unique<CAutoBase>(obj.get());
    }
    if ( params.type == OBJECT_PORTICO )
    {
        objAuto = std::make_unique<CAutoPortico>(obj.get());
    }
    if ( params.type == OBJECT_DERRICK )
    {
        objAuto = std::make_unique<CAutoDerrick>(obj.get());
    }
    if ( params.type == OBJECT_FACTORY )
    {
        objAuto = std::make_unique<CAutoFactory>(obj.get());
    }
    if ( params.type == OBJECT_REPAIR )
    {
        objAuto = std::make_unique<CAutoRepair>(obj.get());
    }
    if ( params.type == OBJECT_DESTROYER )
    {
        objAuto = std::make_unique<CAutoDestroyer>(obj.get());
    }
    if ( params.type == OBJECT_STATION )
    {
        objAuto = std::make_unique<CAutoPowerStation>(obj.get());
    }
    if ( params.type == OBJECT_CONVERT )
    {
        objAuto = std::make_unique<CAutoConvert>(obj.get());
    }
    if ( params.type == OBJECT_TOWER )
    {
        objAuto = std::make_unique<CAutoTower>(obj.get());
    }
    if ( params.type == OBJECT_RESEARCH )
    {
        objAuto = std::make_unique<CAutoResearch>(obj.get());
    }
    if ( params.type == OBJECT_RADAR )
    {
        objAuto = std::make_unique<CAutoRadar>(obj.get());
    }
    if ( params.type == OBJECT_ENERGY )
    {
        objAuto = std::make_unique<CAutoPowerPlant>(obj.get());
    }
    if ( params.type == OBJECT_LABO )
    {
        objAuto = std::make_unique<CAutoLabo>(obj.get());
    }
    if ( params.type == OBJECT_NUCLEAR )
    {
        objAuto = std::make_unique<CAutoNuclearPlant>(obj.get());
    }
    if ( params.type == OBJECT_PARA )
    {
        objAuto = std::make_unique<CAutoPowerCaptor>(obj.get());
    }
    if ( params.type == OBJECT_SAFE )
    {
        objAuto = std::make_unique<CAutoVault>(obj.get());
    }
    if ( params.type == OBJECT_HUSTON )
    {
        objAuto = std::make_unique<CAutoHouston>(obj.get());
    }
    if ( params.type == OBJECT_NEST )
    {
        objAuto = std::make_unique<CAutoNest>(obj.get());
    }

    if (objAuto != nullptr)
    {
        objAuto->Init();
        obj->SetAuto(std::move(objAuto));
    }

    engine->LoadAllTextures();

    obj->UpdateMapping();

    return obj;
}
