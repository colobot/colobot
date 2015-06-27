/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2014, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsiteс.ch; http://colobot.info; http://github.com/colobot
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

#include "object/object_factory.h"

#include "graphics/engine/engine.h"
#include "graphics/engine/modelmanager.h"
#include "graphics/engine/terrain.h"
#include "graphics/engine/lightning.h"

#include "object/brain.h"
#include "object/object_create_params.h"
#include "object/robotmain.h"
#include "object/auto/autobase.h"
#include "object/auto/autoconvert.h"
#include "object/auto/autoderrick.h"
#include "object/auto/autodestroyer.h"
#include "object/auto/autoegg.h"
#include "object/auto/autoenergy.h"
#include "object/auto/autofactory.h"
#include "object/auto/autoflag.h"
#include "object/auto/autohuston.h"
#include "object/auto/autojostle.h"
#include "object/auto/autokid.h"
#include "object/auto/autolabo.h"
#include "object/auto/automush.h"
#include "object/auto/autonest.h"
#include "object/auto/autonuclear.h"
#include "object/auto/autopara.h"
#include "object/auto/autoportico.h"
#include "object/auto/autoradar.h"
#include "object/auto/autorepair.h"
#include "object/auto/autoresearch.h"
#include "object/auto/autoroot.h"
#include "object/auto/autosafe.h"
#include "object/auto/autostation.h"
#include "object/auto/autotower.h"
#include "object/motion/motionant.h"
#include "object/motion/motionbee.h"
#include "object/motion/motiondummy.h"
#include "object/motion/motionhuman.h"
#include "object/motion/motionmother.h"
#include "object/motion/motionspider.h"
#include "object/motion/motiontoto.h"
#include "object/motion/motionvehicle.h"
#include "object/motion/motionworm.h"
#include "object/subclass/exchange_post.h"

#include "math/geometry.h"

#include "physics/physics.h"

CObjectFactory::CObjectFactory(Gfx::CEngine* engine,
                               Gfx::CTerrain* terrain,
                               Gfx::CModelManager* modelManager,
                               Gfx::CParticle* particle,
                               CRobotMain* main)
   : m_engine(engine)
   , m_terrain(terrain)
   , m_modelManager(modelManager)
   , m_particle(particle)
   , m_main(main)
{}

CObjectUPtr CObjectFactory::CreateObject(const ObjectCreateParams& params)
{
    switch (params.type)
    {
        case OBJECT_NULL:
            return nullptr;

        case OBJECT_INFO:
            return CExchangePost::Create(params, m_modelManager, m_engine);

        case OBJECT_PORTICO:
        case OBJECT_BASE:
        case OBJECT_DERRICK:
        case OBJECT_FACTORY:
        case OBJECT_STATION:
        case OBJECT_CONVERT:
        case OBJECT_REPAIR:
        case OBJECT_DESTROYER:
        case OBJECT_TOWER:
        case OBJECT_NEST:
        case OBJECT_RESEARCH:
        case OBJECT_RADAR:
        case OBJECT_ENERGY:
        case OBJECT_LABO:
        case OBJECT_NUCLEAR:
        case OBJECT_PARA:
        case OBJECT_SAFE:
        case OBJECT_HUSTON:
        case OBJECT_TARGET1:
        case OBJECT_TARGET2:
        case OBJECT_START:
        case OBJECT_END:
            return CreateBuilding(params);

        case OBJECT_FRET:
        case OBJECT_STONE:
        case OBJECT_URANIUM:
        case OBJECT_METAL:
        case OBJECT_POWER:
        case OBJECT_ATOMIC:
        case OBJECT_BULLET:
        case OBJECT_BBOX:
        case OBJECT_KEYa:
        case OBJECT_KEYb:
        case OBJECT_KEYc:
        case OBJECT_KEYd:
        case OBJECT_TNT:
        case OBJECT_SCRAP1:
        case OBJECT_SCRAP2:
        case OBJECT_SCRAP3:
        case OBJECT_SCRAP4:
        case OBJECT_SCRAP5:
        case OBJECT_BOMB:
        case OBJECT_WAYPOINT:
        case OBJECT_SHOW:
        case OBJECT_WINFIRE:
        case OBJECT_BAG:
        case OBJECT_MARKPOWER:
        case OBJECT_MARKSTONE:
        case OBJECT_MARKURANIUM:
        case OBJECT_MARKKEYa:
        case OBJECT_MARKKEYb:
        case OBJECT_MARKKEYc:
        case OBJECT_MARKKEYd:
        case OBJECT_EGG:
            return CreateResource(params);

        case OBJECT_FLAGb:
        case OBJECT_FLAGr:
        case OBJECT_FLAGg:
        case OBJECT_FLAGy:
        case OBJECT_FLAGv:
            return CreateFlag(params);

        case OBJECT_BARRIER0:
        case OBJECT_BARRIER1:
        case OBJECT_BARRIER2:
        case OBJECT_BARRIER3:
            return CreateBarrier(params);

        case OBJECT_PLANT0:
        case OBJECT_PLANT1:
        case OBJECT_PLANT2:
        case OBJECT_PLANT3:
        case OBJECT_PLANT4:
        case OBJECT_PLANT5:
        case OBJECT_PLANT6:
        case OBJECT_PLANT7:
        case OBJECT_PLANT8:
        case OBJECT_PLANT9:
        case OBJECT_PLANT10:
        case OBJECT_PLANT11:
        case OBJECT_PLANT12:
        case OBJECT_PLANT13:
        case OBJECT_PLANT14:
        case OBJECT_PLANT15:
        case OBJECT_PLANT16:
        case OBJECT_PLANT17:
        case OBJECT_PLANT18:
        case OBJECT_PLANT19:
        case OBJECT_TREE0:
        case OBJECT_TREE1:
        case OBJECT_TREE2:
        case OBJECT_TREE3:
        case OBJECT_TREE4:
        case OBJECT_TREE5:
            return CreatePlant(params);

        case OBJECT_MUSHROOM1:
        case OBJECT_MUSHROOM2:
            return CreateMushroom(params);

        case OBJECT_TEEN0:
        case OBJECT_TEEN1:
        case OBJECT_TEEN2:
        case OBJECT_TEEN3:
        case OBJECT_TEEN4:
        case OBJECT_TEEN5:
        case OBJECT_TEEN6:
        case OBJECT_TEEN7:
        case OBJECT_TEEN8:
        case OBJECT_TEEN9:
        case OBJECT_TEEN10:
        case OBJECT_TEEN11:
        case OBJECT_TEEN12:
        case OBJECT_TEEN13:
        case OBJECT_TEEN14:
        case OBJECT_TEEN15:
        case OBJECT_TEEN16:
        case OBJECT_TEEN17:
        case OBJECT_TEEN18:
        case OBJECT_TEEN19:
        case OBJECT_TEEN20:
        case OBJECT_TEEN21:
        case OBJECT_TEEN22:
        case OBJECT_TEEN23:
        case OBJECT_TEEN24:
        case OBJECT_TEEN25:
        case OBJECT_TEEN26:
        case OBJECT_TEEN27:
        case OBJECT_TEEN28:
        case OBJECT_TEEN29:
        case OBJECT_TEEN30:
        case OBJECT_TEEN31:
        case OBJECT_TEEN32:
        case OBJECT_TEEN33:
        case OBJECT_TEEN34:
        case OBJECT_TEEN35:
        case OBJECT_TEEN36:
        case OBJECT_TEEN37:
        case OBJECT_TEEN38:
        case OBJECT_TEEN39:
        case OBJECT_TEEN40:
        case OBJECT_TEEN41:
        case OBJECT_TEEN42:
        case OBJECT_TEEN43:
        case OBJECT_TEEN44:
            return CreateTeen(params);

        case OBJECT_QUARTZ0:
        case OBJECT_QUARTZ1:
        case OBJECT_QUARTZ2:
        case OBJECT_QUARTZ3:
            return CreateQuartz(params);

        case OBJECT_ROOT0:
        case OBJECT_ROOT1:
        case OBJECT_ROOT2:
        case OBJECT_ROOT3:
        case OBJECT_ROOT4:
        case OBJECT_ROOT5:
            return CreateRoot(params);

        case OBJECT_HOME1:
            return CreateHome(params);

        case OBJECT_RUINmobilew1:
        case OBJECT_RUINmobilew2:
        case OBJECT_RUINmobilet1:
        case OBJECT_RUINmobilet2:
        case OBJECT_RUINmobiler1:
        case OBJECT_RUINmobiler2:
        case OBJECT_RUINfactory:
        case OBJECT_RUINdoor:
        case OBJECT_RUINsupport:
        case OBJECT_RUINradar:
        case OBJECT_RUINconvert:
        case OBJECT_RUINbase:
        case OBJECT_RUINhead:
            return CreateRuin(params);

        case OBJECT_APOLLO1:
        case OBJECT_APOLLO3:
        case OBJECT_APOLLO4:
        case OBJECT_APOLLO5:
            return CreateApollo(params);

        case OBJECT_MOTHER:
        case OBJECT_ANT:
        case OBJECT_SPIDER:
        case OBJECT_BEE:
        case OBJECT_WORM:
            return CreateInsect(params);  // no eggs

        case OBJECT_HUMAN:
        case OBJECT_TECH:
        case OBJECT_TOTO:
        case OBJECT_MOBILEfa:
        case OBJECT_MOBILEta:
        case OBJECT_MOBILEwa:
        case OBJECT_MOBILEia:
        case OBJECT_MOBILEfc:
        case OBJECT_MOBILEtc:
        case OBJECT_MOBILEwc:
        case OBJECT_MOBILEic:
        case OBJECT_MOBILEfi:
        case OBJECT_MOBILEti:
        case OBJECT_MOBILEwi:
        case OBJECT_MOBILEii:
        case OBJECT_MOBILEfs:
        case OBJECT_MOBILEts:
        case OBJECT_MOBILEws:
        case OBJECT_MOBILEis:
        case OBJECT_MOBILErt:
        case OBJECT_MOBILErc:
        case OBJECT_MOBILErr:
        case OBJECT_MOBILErs:
        case OBJECT_MOBILEsa:
        case OBJECT_MOBILEtg:
        case OBJECT_MOBILEft:
        case OBJECT_MOBILEtt:
        case OBJECT_MOBILEwt:
        case OBJECT_MOBILEit:
        case OBJECT_MOBILEdr:
        case OBJECT_APOLLO2:
            return CreateVehicle(params);

        default:
            break;
    }

    return nullptr;
}

// Creates a building laying on the ground.

CObjectUPtr CObjectFactory::CreateBuilding(const ObjectCreateParams& params)
{
    Math::Vector pos = params.pos;
    float angle = params.angle;
    float height = params.height;
    ObjectType type = params.type;
    float power = params.power;

    CObjectUPtr obj(new CObject(params.id));

    obj->SetType(type);

    int rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);  // it is a stationary object
    obj->SetObjectRank(0, rank);

    if ( type == OBJECT_PORTICO )
    {
        m_modelManager->AddModelReference("portico1.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetFloorHeight(0.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        m_modelManager->AddModelReference("portico2.mod", false, rank);
        obj->SetPosition(1, Math::Vector(0.0f, 67.0f, 0.0f));

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(2, rank);
        obj->SetObjectParent(2, 1);
        m_modelManager->AddModelReference("portico3.mod", false, rank);
        obj->SetPosition(2, Math::Vector(0.0f, 0.0f, -33.0f));
        obj->SetAngleY(2, 45.0f*Math::PI/180.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(3, rank);
        obj->SetObjectParent(3, 2);
        m_modelManager->AddModelReference("portico4.mod", false, rank);
        obj->SetPosition(3, Math::Vector(50.0f, 0.0f, 0.0f));
        obj->SetAngleY(3, -60.0f*Math::PI/180.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(4, rank);
        obj->SetObjectParent(4, 3);
        m_modelManager->AddModelReference("portico5.mod", false, rank);
        obj->SetPosition(4, Math::Vector(35.0f, 0.0f, 0.0f));
        obj->SetAngleY(4, -55.0f*Math::PI/180.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(5, rank);
        obj->SetObjectParent(5, 1);
        m_modelManager->AddModelReference("portico3.mod", false, rank);
        obj->SetPosition(5, Math::Vector(0.0f, 0.0f, 33.0f));
        obj->SetAngleY(5, -45.0f*Math::PI/180.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(6, rank);
        obj->SetObjectParent(6, 5);
        m_modelManager->AddModelReference("portico4.mod", false, rank);
        obj->SetPosition(6, Math::Vector(50.0f, 0.0f, 0.0f));
        obj->SetAngleY(6, 60.0f*Math::PI/180.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(7, rank);
        obj->SetObjectParent(7, 6);
        m_modelManager->AddModelReference("portico5.mod", false, rank);
        obj->SetPosition(7, Math::Vector(35.0f, 0.0f, 0.0f));
        obj->SetAngleY(7, 55.0f*Math::PI/180.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(8, rank);
        obj->SetObjectParent(8, 0);
        m_modelManager->AddModelReference("portico6.mod", false, rank);
        obj->SetPosition(8, Math::Vector(-35.0f, 50.0f, -35.0f));
        obj->SetAngleY(8, -Math::PI/2.0f);
        obj->SetZoom(8, 2.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(9, rank);
        obj->SetObjectParent(9, 8);
        m_modelManager->AddModelReference("portico7.mod", false, rank);
        obj->SetPosition(9, Math::Vector(0.0f, 4.5f, 1.9f));

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(10, rank);
        obj->SetObjectParent(10, 0);
        m_modelManager->AddModelReference("portico6.mod", false, rank);
        obj->SetPosition(10, Math::Vector(-35.0f, 50.0f, 35.0f));
        obj->SetAngleY(10, -Math::PI/2.0f);
        obj->SetZoom(10, 2.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(11, rank);
        obj->SetObjectParent(11, 10);
        m_modelManager->AddModelReference("portico7.mod", false, rank);
        obj->SetPosition(11, Math::Vector(0.0f, 4.5f, 1.9f));

        obj->CreateCrashSphere(Math::Vector(  0.0f, 28.0f,   0.0f), 45.5f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 27.0f, 10.0f, -42.0f), 15.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  0.0f, 10.0f, -42.0f), 15.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-27.0f, 10.0f, -42.0f), 15.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 27.0f, 10.0f,  42.0f), 15.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  0.0f, 10.0f,  42.0f), 15.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-27.0f, 10.0f,  42.0f), 15.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-32.0f, 45.0f, -32.0f), 10.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-32.0f, 45.0f,  32.0f), 10.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 32.0f, 45.0f, -32.0f), 10.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 32.0f, 45.0f,  32.0f), 10.0f, SOUND_BOUMm, 0.45f);
        obj->SetGlobalSphere(Math::Vector(0.0f, 35.0f, 0.0f), 50.0f);

        obj->CreateShadowCircle(50.0f, 1.0f);
    }

    if ( type == OBJECT_BASE )
    {
        m_modelManager->AddModelReference("base1.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetFloorHeight(0.0f);

        for (int i=0 ; i<8 ; i++ )
        {
            rank = m_engine->CreateObject();
            m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
            obj->SetObjectRank(1+i, rank);
            obj->SetObjectParent(1+i, 0);
            m_modelManager->AddModelReference("base2.mod", false, rank);
            Math::Point p = Math::RotatePoint(-Math::PI/4.0f*i, 27.8f);
            obj->SetPosition(1+i, Math::Vector(p.x, 30.0f, p.y));
            obj->SetAngleY(1+i, Math::PI/4.0f*i);
            obj->SetAngleZ(1+i, Math::PI/2.0f);

            rank = m_engine->CreateObject();
            m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
            obj->SetObjectRank(10+i, rank);
            obj->SetObjectParent(10+i, 1+i);
            m_modelManager->AddModelReference("base4.mod", false, rank);
            obj->SetPosition(10+i, Math::Vector(23.5f, 0.0f, 7.0f));

            rank = m_engine->CreateObject();
            m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
            obj->SetObjectRank(18+i, rank);
            obj->SetObjectParent(18+i, 1+i);
            m_modelManager->AddModelReference("base4.mod", true, rank);
            obj->SetPosition(18+i, Math::Vector(23.5f, 0.0f, -7.0f));
        }

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(9, rank);
        obj->SetObjectParent(9, 0);
        m_modelManager->AddModelReference("base3.mod", false, rank); // central pillar

        obj->CreateCrashSphere(Math::Vector(  0.0f, 33.0f,   0.0f),  2.5f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  0.0f, 39.0f,   0.0f),  2.5f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  0.0f, 45.0f,   0.0f),  2.5f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  0.0f, 51.0f,   0.0f),  2.5f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  0.0f, 57.0f,   0.0f),  2.5f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  0.0f, 63.0f,   0.0f),  2.5f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  0.0f, 69.0f,   0.0f),  2.5f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  0.0f, 82.0f,   0.0f),  8.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 18.0f, 94.0f,   0.0f), 10.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-18.0f, 94.0f,   0.0f), 10.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  0.0f, 94.0f,  18.0f), 10.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  0.0f, 94.0f, -18.0f), 10.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 13.0f, 94.0f,  13.0f), 10.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-13.0f, 94.0f,  13.0f), 10.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 13.0f, 94.0f, -13.0f), 10.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-13.0f, 94.0f, -13.0f), 10.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  0.0f,104.0f,   0.0f), 14.0f, SOUND_BOUMm, 0.45f);

        obj->SetGlobalSphere(Math::Vector(0.0f, 45.0f, 0.0f), 10.0f);

        obj->CreateShadowCircle(60.0f, 1.0f);
        obj->SetShowLimitRadius(200.0f);

        m_terrain->AddBuildingLevel(pos, 28.6f, 73.4f, 30.0f, 0.4f);
    }

    if ( type == OBJECT_DERRICK )
    {
        m_modelManager->AddModelReference("derrick1.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetFloorHeight(0.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        m_modelManager->AddModelReference("derrick2.mod", false, rank);

        obj->CreateCrashSphere(Math::Vector(0.0f,  0.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(0.0f, 10.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(0.0f, 17.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(0.0f, 26.0f, 0.0f), 3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(7.0f, 17.0f, 0.0f), 3.0f, SOUND_BOUMm, 0.45f);
        obj->SetGlobalSphere(Math::Vector(0.0f, 10.0f, 0.0f), 10.0f);

        obj->CreateShadowCircle(10.0f, 0.4f);
    }

    if ( type == OBJECT_RESEARCH )
    {
        m_modelManager->AddModelReference("search1.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetFloorHeight(0.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        m_modelManager->AddModelReference("search2.mod", false, rank);
        obj->SetPosition(1, Math::Vector(0.0f, 13.0f, 0.0f));

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(2, rank);
        obj->SetObjectParent(2, 1);
        m_modelManager->AddModelReference("search3.mod", false, rank);
        obj->SetPosition(2, Math::Vector(0.0f, 4.0f, 0.0f));
        obj->SetAngleZ(2, 35.0f*Math::PI/180.0f);

        obj->CreateCrashSphere(Math::Vector(0.0f,  0.0f, 0.0f), 9.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(0.0f,  6.0f, 0.0f), 9.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(0.0f, 14.0f, 0.0f), 7.0f, SOUND_BOUMm, 0.45f);
        obj->SetGlobalSphere(Math::Vector(0.0f, 8.0f, 0.0f), 12.0f);

        obj->GetCharacter()->posPower = Math::Vector(7.5f, 3.0f, 0.0f);

        obj->CreateShadowCircle(12.0f, 1.0f);
    }

    if ( type == OBJECT_RADAR )
    {
        m_modelManager->AddModelReference("radar1.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetFloorHeight(0.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        m_modelManager->AddModelReference("radar2.mod", false, rank);
        obj->SetPosition(1, Math::Vector(0.0f, 5.0f, 0.0f));

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(2, rank);
        obj->SetObjectParent(2, 0);
        m_modelManager->AddModelReference("radar3.mod", false, rank);
        obj->SetPosition(2, Math::Vector(0.0f, 11.0f, 0.0f));
        obj->SetAngleY(2, -Math::PI/2.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(3, rank);
        obj->SetObjectParent(3, 2);
        m_modelManager->AddModelReference("radar4.mod", false, rank);
        obj->SetPosition(3, Math::Vector(0.0f, 4.5f, 1.9f));

        obj->CreateCrashSphere(Math::Vector(0.0f,  3.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(0.0f, 11.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f);
        obj->SetGlobalSphere(Math::Vector(0.0f, 7.0f, 0.0f), 7.0f);

        obj->CreateShadowCircle(8.0f, 1.0f);
    }

    if ( type == OBJECT_ENERGY )
    {
        m_modelManager->AddModelCopy("energy.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetFloorHeight(0.0f);

        obj->CreateCrashSphere(Math::Vector(-2.0f, 13.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-7.0f,  3.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 0.0f,  1.0f, 0.0f), 1.5f, SOUND_BOUMm, 0.45f);
        obj->SetGlobalSphere(Math::Vector(-7.0f, 5.0f, 0.0f), 5.0f);

        obj->GetCharacter()->posPower = Math::Vector(0.0f, 3.0f, 0.0f);
        obj->SetEnergy(power);  // initializes the energy level

        obj->CreateShadowCircle(6.0f, 0.5f);
    }

    if ( type == OBJECT_LABO )
    {
        m_modelManager->AddModelReference("labo1.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetFloorHeight(0.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        m_modelManager->AddModelReference("labo2.mod", false, rank);
        obj->SetPosition(1, Math::Vector(-9.0f, 3.0f, 0.0f));
        obj->SetAngleZ(1, Math::PI/2.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(2, rank);
        obj->SetObjectParent(2, 1);
        m_modelManager->AddModelReference("labo3.mod", false, rank);
        obj->SetPosition(2, Math::Vector(9.0f, -1.0f, 0.0f));

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(3, rank);
        obj->SetObjectParent(3, 2);
        m_modelManager->AddModelReference("labo4.mod", false, rank);
        obj->SetPosition(3, Math::Vector(0.0f, 0.0f, 0.0f));
        obj->SetAngleZ(3, 80.0f*Math::PI/180.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(4, rank);
        obj->SetObjectParent(4, 2);
        m_modelManager->AddModelReference("labo4.mod", false, rank);
        obj->SetPosition(4, Math::Vector(0.0f, 0.0f, 0.0f));
        obj->SetAngleZ(4, 80.0f*Math::PI/180.0f);
        obj->SetAngleY(4, Math::PI*2.0f/3.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(5, rank);
        obj->SetObjectParent(5, 2);
        m_modelManager->AddModelReference("labo4.mod", false, rank);
        obj->SetPosition(5, Math::Vector(0.0f, 0.0f, 0.0f));
        obj->SetAngleZ(5, 80.0f*Math::PI/180.0f);
        obj->SetAngleY(5, -Math::PI*2.0f/3.0f);

        obj->CreateCrashSphere(Math::Vector(  0.0f,  1.0f,  0.0f), 1.5f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  0.0f, 11.0f,  0.0f), 4.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-10.0f, 10.0f,  0.0f), 4.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-12.0f,  3.0f,  3.0f), 4.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-12.0f,  3.0f, -3.0f), 4.0f, SOUND_BOUMm, 0.45f);
        obj->SetGlobalSphere(Math::Vector(-10.0f, 5.0f, 0.0f), 7.0f);

        obj->GetCharacter()->posPower = Math::Vector(0.0f, 3.0f, 0.0f);

        obj->CreateShadowCircle(7.0f, 0.5f);
    }

    if ( type == OBJECT_FACTORY )
    {
        m_modelManager->AddModelReference("factory1.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetFloorHeight(0.0f);

        for (int i=0 ; i<9 ; i++ )
        {
            rank = m_engine->CreateObject();
            m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
            obj->SetObjectRank(1+i, rank);
            obj->SetObjectParent(1+i, 0);
            m_modelManager->AddModelReference("factory2.mod", false, rank);
            obj->SetPosition(1+i, Math::Vector(10.0f, 2.0f*i, 10.0f));
            obj->SetAngleZ(1+i, Math::PI/2.0f);
            obj->SetZoomZ(1+i, 0.30f);

            rank = m_engine->CreateObject();
            m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
            obj->SetObjectRank(10+i, rank);
            obj->SetObjectParent(10+i, 0);
            m_modelManager->AddModelReference("factory2.mod", false, rank);
            obj->SetPosition(10+i, Math::Vector(10.0f, 2.0f*i, -10.0f));
            obj->SetAngleZ(10+i, -Math::PI/2.0f);
            obj->SetAngleY(10+i, Math::PI);
            obj->SetZoomZ(10+i, 0.30f);
        }

        for (int i=0 ; i<2 ; i++ )
        {
            float s = static_cast<float>(i*2-1);
            obj->CreateCrashSphere(Math::Vector(-10.0f,  2.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
            obj->CreateCrashSphere(Math::Vector( -3.0f,  2.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
            obj->CreateCrashSphere(Math::Vector(  3.0f,  2.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
            obj->CreateCrashSphere(Math::Vector( 10.0f,  2.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
            obj->CreateCrashSphere(Math::Vector(-10.0f,  9.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
            obj->CreateCrashSphere(Math::Vector( -3.0f,  9.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
            obj->CreateCrashSphere(Math::Vector(  3.0f,  9.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
            obj->CreateCrashSphere(Math::Vector( 10.0f,  9.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
            obj->CreateCrashSphere(Math::Vector(-10.0f, 16.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
            obj->CreateCrashSphere(Math::Vector( -3.0f, 16.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
            obj->CreateCrashSphere(Math::Vector(  3.0f, 16.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
            obj->CreateCrashSphere(Math::Vector( 10.0f, 16.0f, 11.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
            obj->CreateCrashSphere(Math::Vector(-10.0f, 16.0f,  4.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
            obj->CreateCrashSphere(Math::Vector( -3.0f, 16.0f,  4.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
            obj->CreateCrashSphere(Math::Vector(  3.0f, 16.0f,  4.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
            obj->CreateCrashSphere(Math::Vector( 10.0f, 16.0f,  4.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
            obj->CreateCrashSphere(Math::Vector(-10.0f,  2.0f,  4.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
            obj->CreateCrashSphere(Math::Vector(-10.0f,  9.0f,  4.0f*s), 4.0f, SOUND_BOUMm, 0.45f);
        }
        obj->CreateCrashSphere(Math::Vector(-10.0f, 21.0f, -4.0f), 3.0f, SOUND_BOUMm, 0.45f);
        obj->SetGlobalSphere(Math::Vector(0.0f, 10.0f, 0.0f), 18.0f);

        obj->CreateShadowCircle(24.0f, 0.3f);
    }

    if ( type == OBJECT_REPAIR )
    {
        m_modelManager->AddModelReference("repair1.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetFloorHeight(0.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        m_modelManager->AddModelReference("repair2.mod", false, rank);
        obj->SetPosition(1, Math::Vector(-11.0f, 13.5f, 0.0f));
        obj->SetAngleZ(1, Math::PI/2.0f);

        m_terrain->AddBuildingLevel(pos, 7.0f, 9.0f, 1.0f, 0.5f);

        obj->CreateCrashSphere(Math::Vector(-11.0f,  0.0f,  4.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-11.0f,  0.0f,  0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-11.0f,  0.0f, -4.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-11.0f, 10.0f,  0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->SetGlobalSphere(Math::Vector(-11.0f, 13.0f, 0.0f), 15.0f);
    }

    if ( type == OBJECT_DESTROYER )
    {
        m_modelManager->AddModelReference("destroy1.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetFloorHeight(0.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        m_modelManager->AddModelReference("destroy2.mod", false, rank);
        obj->SetPosition(1, Math::Vector(0.0f, 0.0f, 0.0f));

        m_terrain->AddBuildingLevel(pos, 7.0f, 9.0f, 1.0f, 0.5f);

        obj->CreateCrashSphere(Math::Vector(-3.5f, 0.0f, -13.5f), 4.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 3.5f, 0.0f, -13.5f), 4.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-3.5f, 0.0f,  13.5f), 4.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 3.5f, 0.0f,  13.5f), 4.0f, SOUND_BOUMm, 0.45f);

        obj->CreateShadowCircle(19.0f, 1.0f);
    }

    if ( type == OBJECT_STATION )
    {
        m_modelManager->AddModelCopy("station.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetFloorHeight(0.0f);

        m_terrain->AddBuildingLevel(pos, 7.0f, 9.0f, 1.0f, 0.5f);

        obj->CreateCrashSphere(Math::Vector(-15.0f, 2.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-15.0f, 6.0f, 0.0f), 4.0f, SOUND_BOUMm, 0.45f);
        obj->SetGlobalSphere(Math::Vector(-15.0f, 5.0f, 0.0f), 6.0f);

        obj->SetEnergy(power);
    }

    if ( type == OBJECT_CONVERT )
    {
        m_modelManager->AddModelReference("convert1.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetFloorHeight(0.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        m_modelManager->AddModelReference("convert2.mod", false, rank);
        obj->SetPosition(1, Math::Vector(0.0f, 14.0f, 0.0f));

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(2, rank);
        obj->SetObjectParent(2, 0);
        m_modelManager->AddModelReference("convert3.mod", false, rank);
        obj->SetPosition(2, Math::Vector(0.0f, 11.5f, 0.0f));
        obj->SetAngleX(2, -Math::PI*0.35f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(3, rank);
        obj->SetObjectParent(3, 0);
        m_modelManager->AddModelReference("convert3.mod", false, rank);
        obj->SetPosition(3, Math::Vector(0.0f, 11.5f, 0.0f));
        obj->SetAngleY(3, Math::PI);
        obj->SetAngleX(3, -Math::PI*0.35f);

        m_terrain->AddBuildingLevel(pos, 7.0f, 9.0f, 1.0f, 0.5f);

        obj->CreateCrashSphere(Math::Vector(-10.0f,  2.0f,  4.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-10.0f,  2.0f, -4.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-10.0f,  9.0f,  0.0f), 6.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  0.0f, 14.0f,  0.0f), 1.5f, SOUND_BOUMm, 0.45f);
        obj->SetGlobalSphere(Math::Vector(-3.0f, 8.0f, 0.0f), 14.0f);
    }

    if ( type == OBJECT_TOWER )
    {
        m_modelManager->AddModelReference("tower.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetFloorHeight(0.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        m_modelManager->AddModelReference("roller2c.mod", false, rank);
        obj->SetPosition(1, Math::Vector(0.0f, 20.0f, 0.0f));
        obj->SetAngleZ(1, Math::PI/2.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(2, rank);
        obj->SetObjectParent(2, 1);
        m_modelManager->AddModelReference("roller3c.mod", false, rank);
        obj->SetPosition(2, Math::Vector(4.5f, 0.0f, 0.0f));
        obj->SetAngleZ(2, 0.0f);

        obj->CreateCrashSphere(Math::Vector(0.0f,  0.0f, 0.0f), 6.5f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(0.0f,  8.0f, 0.0f), 4.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(0.0f, 15.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(0.0f, 24.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->SetGlobalSphere(Math::Vector(0.0f, 5.0f, 0.0f), 7.0f);

        obj->GetCharacter()->posPower = Math::Vector(5.0f, 3.0f, 0.0f);

        obj->CreateShadowCircle(6.0f, 1.0f);
        obj->SetShowLimitRadius(Gfx::LTNG_PROTECTION_RADIUS);
    }

    if ( type == OBJECT_NUCLEAR )
    {
        m_modelManager->AddModelReference("nuclear1.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetFloorHeight(0.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        m_modelManager->AddModelReference("nuclear2.mod", false, rank);
        obj->SetPosition(1, Math::Vector(20.0f, 10.0f, 0.0f));
        obj->SetAngleZ(1, 135.0f*Math::PI/180.0f);

        obj->CreateCrashSphere(Math::Vector( 0.0f,  0.0f, 0.0f), 19.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 0.0f, 24.0f, 0.0f), 15.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(22.0f,  1.0f, 0.0f),  1.5f, SOUND_BOUMm, 0.45f);
        obj->SetGlobalSphere(Math::Vector(0.0f, 17.0f, 0.0f), 26.0f);

        obj->GetCharacter()->posPower = Math::Vector(22.0f, 3.0f, 0.0f);

        obj->CreateShadowCircle(21.0f, 1.0f);
    }

    if ( type == OBJECT_PARA )
    {
        m_modelManager->AddModelReference("para.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetFloorHeight(0.0f);

        m_terrain->AddBuildingLevel(pos, 16.0f, 18.0f, 1.0f, 0.5f);

        obj->CreateCrashSphere(Math::Vector( 13.0f,  3.0f,  13.0f),  3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 11.0f, 15.0f,  11.0f),  2.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-13.0f,  3.0f,  13.0f),  3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-11.0f, 15.0f, -11.0f),  2.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 13.0f,  3.0f, -13.0f),  3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 11.0f, 15.0f, -11.0f),  2.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-13.0f,  3.0f, -13.0f),  3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-11.0f, 15.0f, -11.0f),  2.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  0.0f, 26.0f,   0.0f),  9.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  0.0f, 54.0f,   0.0f), 14.0f, SOUND_BOUMm, 0.45f);
        obj->SetGlobalSphere(Math::Vector(0.0f, 10.0f, 0.0f), 20.0f);

        obj->CreateShadowCircle(21.0f, 1.0f);
        obj->SetShowLimitRadius(Gfx::LTNG_PROTECTION_RADIUS);
    }

    if ( type == OBJECT_SAFE )
    {
        m_modelManager->AddModelReference("safe1.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetFloorHeight(0.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        m_modelManager->AddModelReference("safe2.mod", false, rank);
        obj->SetZoom(1, 1.05f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(2, rank);
        obj->SetObjectParent(2, 0);
        m_modelManager->AddModelReference("safe3.mod", false, rank);
        obj->SetZoom(2, 1.05f);

        m_terrain->AddBuildingLevel(pos, 18.0f, 20.0f, 1.0f, 0.5f);

        obj->CreateCrashSphere(Math::Vector(0.0f, 1.0f, 0.0f), 13.0f, SOUND_BOUMm, 0.45f);
        obj->SetGlobalSphere(Math::Vector(0.0f, 1.0f, 0.0f), 13.0f);

        obj->CreateShadowCircle(23.0f, 1.0f);
    }

    if ( type == OBJECT_HUSTON )
    {
        m_modelManager->AddModelReference("huston1.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetFloorHeight(0.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        m_modelManager->AddModelReference("huston2.mod", false, rank);
        obj->SetPosition(1, Math::Vector(0.0f, 39.0f, 30.0f));
        obj->SetAngleY(1, -Math::PI/2.0f);
        obj->SetZoom(1, 3.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(2, rank);
        obj->SetObjectParent(2, 1);
        m_modelManager->AddModelReference("huston3.mod", false, rank);
        obj->SetPosition(2, Math::Vector(0.0f, 4.5f, 1.9f));

        obj->CreateCrashSphere(Math::Vector( 15.0f,  6.0f, -53.0f), 16.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-15.0f,  6.0f, -53.0f), 16.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 15.0f,  6.0f, -26.0f), 16.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-15.0f,  6.0f, -26.0f), 16.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 15.0f,  6.0f,   0.0f), 16.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-15.0f,  6.0f,   0.0f), 16.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 15.0f,  6.0f,  26.0f), 16.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-15.0f,  6.0f,  26.0f), 16.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 15.0f,  6.0f,  53.0f), 16.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-15.0f,  6.0f,  53.0f), 16.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  0.0f, 27.0f,  30.0f), 12.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  0.0f, 45.0f,  30.0f), 14.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 26.0f,  4.0f, -61.0f),  5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-26.0f,  4.0f, -61.0f),  5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 26.0f,  4.0f,  61.0f),  5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-26.0f,  4.0f,  61.0f),  5.0f, SOUND_BOUMm, 0.45f);
    }

    if ( type == OBJECT_TARGET1 )
    {
        m_modelManager->AddModelReference("target1.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, 1.5f);
        obj->SetFloorHeight(0.0f);

        obj->CreateCrashSphere(Math::Vector(  0.0f, 50.0f+14.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( -7.0f, 50.0f+12.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  7.0f, 50.0f+12.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-12.0f, 50.0f+ 7.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 12.0f, 50.0f+ 7.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-14.0f, 50.0f+ 0.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 14.0f, 50.0f+ 0.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-12.0f, 50.0f- 7.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 12.0f, 50.0f- 7.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( -7.0f, 50.0f-12.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  7.0f, 50.0f-12.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  0.0f, 50.0f-14.0f, 0.0f),  3.0f, SOUND_BOUMm, 0.45f);

        obj->CreateCrashSphere(Math::Vector(0.0f, 30.0f, 0.0f), 2.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(0.0f, 24.0f, 0.0f), 3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(0.0f, 16.0f, 0.0f), 4.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(0.0f,  4.0f, 0.0f), 8.0f, SOUND_BOUMm, 0.45f);

        obj->CreateShadowCircle(15.0f, 1.0f);
    }

    if ( type == OBJECT_TARGET2 )
    {
        m_modelManager->AddModelReference("target2.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetFloorHeight(0.0f);

        height += 50.0f*1.5f;
    }

    if ( type == OBJECT_NEST )
    {
        m_modelManager->AddModelReference("nest.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetFloorHeight(0.0f);

        m_terrain->AddBuildingLevel(pos, 3.0f, 5.0f, 1.0f, 0.5f);

        obj->CreateShadowCircle(4.0f, 1.0f);
    }

    if ( type == OBJECT_START )
    {
        m_modelManager->AddModelReference("start.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetFloorHeight(0.0f);

        m_terrain->AddBuildingLevel(pos, 7.0f, 9.0f, 1.0f, 0.5f);
    }

    if ( type == OBJECT_END )
    {
        m_modelManager->AddModelReference("end.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetFloorHeight(0.0f);

        m_terrain->AddBuildingLevel(pos, 7.0f, 9.0f, 1.0f, 0.5f);
    }

#if 0
    if ( power > 0.0f )  // creates a battery?
    {
        CObject*    pPower;

        pPower = new CObject();
        pPower->obj->SetType(power<=1.0f?OBJECT_POWER:OBJECT_ATOMIC);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        pPower->obj->SetObjectRank(0, rank);

        if ( power <= 1.0f )  m_modelManager->AddModelReference("power.mod", false, rank);
        else                  m_modelManager->AddModelReference("atomic.mod", false, rank);

        pPower->obj->SetPosition(0, GetCharacter()->posPower);
        pPower->obj->CreateCrashSphere(Math::Vector(0.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        pPower->obj->SetGlobalSphere(Math::Vector(0.0f, 1.0f, 0.0f), 1.5f);

        pPower->SetTruck(obj.get());
        SetPower(pPower);

        if ( power <= 1.0f )  pPower->obj->SetEnergy(power);
        else                  pPower->obj->SetEnergy(power/100.0f);
    }
#endif

    pos = obj->GetPosition(0);
    pos.y += height;
    obj->SetPosition(0, pos);  // to display the shadows immediately

    AddObjectAuto(obj.get());
    m_engine->LoadAllTextures();

    return obj;
}

// Creates a small resource set on the ground.

CObjectUPtr CObjectFactory::CreateResource(const ObjectCreateParams& params)
{
    Math::Vector pos = params.pos;
    float angle = params.angle;
    ObjectType type = params.type;
    float power = params.power;

    CObjectUPtr obj(new CObject(params.id));

    obj->SetType(type);

    int rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);  // it is a stationary object
    obj->SetObjectRank(0, rank);
    obj->SetEnergy(power);

    std::string name;
    if ( type == OBJECT_STONE       )  name = "stone.mod";
    if ( type == OBJECT_URANIUM     )  name = "uranium.mod";
    if ( type == OBJECT_METAL       )  name = "metal.mod";
    if ( type == OBJECT_POWER       )  name = "power.mod";
    if ( type == OBJECT_ATOMIC      )  name = "atomic.mod";
    if ( type == OBJECT_BULLET      )  name = "bullet.mod";
    if ( type == OBJECT_BBOX        )  name = "bbox.mod";
    if ( type == OBJECT_KEYa        )  name = "keya.mod";
    if ( type == OBJECT_KEYb        )  name = "keyb.mod";
    if ( type == OBJECT_KEYc        )  name = "keyc.mod";
    if ( type == OBJECT_KEYd        )  name = "keyd.mod";
    if ( type == OBJECT_TNT         )  name = "tnt.mod";
    if ( type == OBJECT_SCRAP1      )  name = "scrap1.mod";
    if ( type == OBJECT_SCRAP2      )  name = "scrap2.mod";
    if ( type == OBJECT_SCRAP3      )  name = "scrap3.mod";
    if ( type == OBJECT_SCRAP4      )  name = "scrap4.mod";
    if ( type == OBJECT_SCRAP5      )  name = "scrap5.mod";
    if ( type == OBJECT_BOMB        )  name = "bomb.mod";
    if ( type == OBJECT_WAYPOINT    )  name = "waypoint.mod";
    if ( type == OBJECT_SHOW        )  name = "show.mod";
    if ( type == OBJECT_WINFIRE     )  name = "winfire.mod";
    if ( type == OBJECT_BAG         )  name = "bag.mod";
    if ( type == OBJECT_MARKSTONE   )  name = "cross1.mod";
    if ( type == OBJECT_MARKURANIUM )  name = "cross3.mod";
    if ( type == OBJECT_MARKPOWER   )  name = "cross2.mod";
    if ( type == OBJECT_MARKKEYa    )  name = "crossa.mod";
    if ( type == OBJECT_MARKKEYb    )  name = "crossb.mod";
    if ( type == OBJECT_MARKKEYc    )  name = "crossc.mod";
    if ( type == OBJECT_MARKKEYd    )  name = "crossd.mod";
    if ( type == OBJECT_EGG         )  name = "egg.mod";

    if (type == OBJECT_POWER || type == OBJECT_ATOMIC)
    {
        m_modelManager->AddModelCopy(name, false, rank);
    }
    else
    {
        m_modelManager->AddModelReference(name, false, rank);
    }

    obj->SetPosition(0, pos);
    obj->SetAngleY(0, angle);

    if ( type == OBJECT_SHOW )  // remains in the air?
    {
        return obj;
    }

    float radius = 1.5f;
    float height = 0.0f;

    if ( type == OBJECT_MARKSTONE   ||
         type == OBJECT_MARKURANIUM ||
         type == OBJECT_MARKKEYa    ||
         type == OBJECT_MARKKEYb    ||
         type == OBJECT_MARKKEYc    ||
         type == OBJECT_MARKKEYd    ||
         type == OBJECT_MARKPOWER   ||
         type == OBJECT_WAYPOINT    )
    {
    }
    else if ( type == OBJECT_EGG )
    {
        obj->CreateCrashSphere(Math::Vector(-1.0f, 2.8f, 0.0f), 3.0f, SOUND_BOUMm, 0.45f);
        obj->SetGlobalSphere(Math::Vector(0.0f, 5.0f, 0.0f), 10.0f);
        radius = 3.0f;
    }
    else if ( type == OBJECT_BOMB )
    {
        obj->CreateCrashSphere(Math::Vector(0.0f, 0.0f, 0.0f), 3.0f, SOUND_BOUMm, 0.45f);
        obj->SetGlobalSphere(Math::Vector(0.0f, 0.0f, 0.0f), 3.0f);
        radius = 3.0f;
    }
    else if ( type == OBJECT_BAG )
    {
        obj->CreateCrashSphere(Math::Vector(0.0f, 0.0f, 0.0f), 4.0f, SOUND_BOUMm, 0.45f);
        obj->SetGlobalSphere(Math::Vector(0.0f, 0.0f, 0.0f), 4.0f);
        obj->SetZoom(0, 1.5f);
        radius =  5.0f;
        height = -1.4f;
    }
    else
    {
        obj->CreateCrashSphere(Math::Vector(0.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        obj->SetGlobalSphere(Math::Vector(0.0f, 1.0f, 0.0f), 1.5f);
    }

    obj->CreateShadowCircle(radius, 1.0f);

    obj->SetFloorHeight(0.0f);
    AddObjectAuto(obj.get());
    m_engine->LoadAllTextures();
    obj->FloorAdjust();

    pos = obj->GetPosition(0);
    pos.y += height;
    obj->SetPosition(0, pos);  // to display the shadows immediately

    return obj;
}

// Creates a flag placed on the ground.

CObjectUPtr CObjectFactory::CreateFlag(const ObjectCreateParams& params)
{
    Math::Vector pos = params.pos;
    float angle = params.angle;
    ObjectType type = params.type;

    CObjectUPtr obj(new CObject(params.id));

    obj->SetType(type);

    std::string name;

    name = "";
    if ( type == OBJECT_FLAGb )  name = "flag1b.mod";
    if ( type == OBJECT_FLAGr )  name = "flag1r.mod";
    if ( type == OBJECT_FLAGg )  name = "flag1g.mod";
    if ( type == OBJECT_FLAGy )  name = "flag1y.mod";
    if ( type == OBJECT_FLAGv )  name = "flag1v.mod";

    int rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);  // it is a stationary object
    obj->SetObjectRank(0, rank);
    m_modelManager->AddModelReference(name, false, rank);
    obj->SetPosition(0, pos);
    obj->SetAngleY(0, angle);

    name = "";
    if ( type == OBJECT_FLAGb )  name = "flag2b.mod";
    if ( type == OBJECT_FLAGr )  name = "flag2r.mod";
    if ( type == OBJECT_FLAGg )  name = "flag2g.mod";
    if ( type == OBJECT_FLAGy )  name = "flag2y.mod";
    if ( type == OBJECT_FLAGv )  name = "flag2v.mod";

    for (int i=0 ; i<4 ; i++ )
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1+i, rank);
        obj->SetObjectParent(1+i, i);
        m_modelManager->AddModelReference(name, false, rank);
        if ( i == 0 )  obj->SetPosition(1+i, Math::Vector(0.15f, 5.0f, 0.0f));
        else           obj->SetPosition(1+i, Math::Vector(0.79f, 0.0f, 0.0f));
    }

    obj->SetJostlingSphere(Math::Vector(0.0f, 4.0f, 0.0f), 1.0f);
    obj->CreateShadowCircle(2.0f, 0.3f);

    obj->SetFloorHeight(0.0f);
    AddObjectAuto(obj.get());
    m_engine->LoadAllTextures();
    obj->FloorAdjust();

    pos = obj->GetPosition(0);
    obj->SetPosition(0, pos);  // to display the shadows immediately

    return obj;
}

// Creates a barrier placed on the ground.

CObjectUPtr CObjectFactory::CreateBarrier(const ObjectCreateParams& params)
{
    Math::Vector pos = params.pos;
    float angle = params.angle;
    float height = params.height;
    ObjectType type = params.type;

    CObjectUPtr obj(new CObject(params.id));

    obj->SetType(type);

    if ( type == OBJECT_BARRIER0 )
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("barrier0.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);

        obj->CreateCrashSphere(Math::Vector( 3.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 0.0f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-3.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);

        obj->CreateShadowCircle(6.0f, 0.5f, Gfx::ENG_SHADOW_WORM);
    }

    if ( type == OBJECT_BARRIER1 )
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("barrier1.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);

        obj->CreateCrashSphere(Math::Vector( 8.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 3.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 0.0f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-3.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-8.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);

        obj->CreateShadowCircle(12.0f, 0.5f, Gfx::ENG_SHADOW_WORM);
    }

    if ( type == OBJECT_BARRIER2 )  // cardboard?
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("barrier2.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);

        obj->CreateCrashSphere(Math::Vector( 8.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 3.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 0.0f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-3.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-8.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);

        obj->CreateShadowCircle(12.0f, 0.8f, Gfx::ENG_SHADOW_WORM);
    }

    if ( type == OBJECT_BARRIER3 )  // match + straw?
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("barrier3.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);

        obj->CreateCrashSphere(Math::Vector( 8.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 3.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 0.0f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-3.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-8.5f, 3.0f, 0.0f), 0.7f, SOUND_BOUMm, 0.45f);

        obj->CreateShadowCircle(10.0f, 0.5f, Gfx::ENG_SHADOW_WORM);
    }

    pos = obj->GetPosition(0);
    obj->SetPosition(0, pos);  // to display the shadows immediately

    obj->SetFloorHeight(0.0f);
    AddObjectAuto(obj.get());
    obj->FloorAdjust();

    pos = obj->GetPosition(0);
    pos.y += height;
    obj->SetPosition(0, pos);

    return obj;
}

// Creates a plant placed on the ground.

CObjectUPtr CObjectFactory::CreatePlant(const ObjectCreateParams& params)
{
    Math::Vector pos = params.pos;
    float angle = params.angle;
    float height = params.height;
    ObjectType type = params.type;

    CObjectUPtr obj(new CObject(params.id));

    obj->SetType(type);

    if ( type == OBJECT_PLANT0 ||
         type == OBJECT_PLANT1 ||
         type == OBJECT_PLANT2 ||
         type == OBJECT_PLANT3 ||
         type == OBJECT_PLANT4 )  // standard?
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        if ( type == OBJECT_PLANT0 )  m_modelManager->AddModelReference("plant0.mod", false, rank);
        if ( type == OBJECT_PLANT1 )  m_modelManager->AddModelReference("plant1.mod", false, rank);
        if ( type == OBJECT_PLANT2 )  m_modelManager->AddModelReference("plant2.mod", false, rank);
        if ( type == OBJECT_PLANT3 )  m_modelManager->AddModelReference("plant3.mod", false, rank);
        if ( type == OBJECT_PLANT4 )  m_modelManager->AddModelReference("plant4.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);

        height -= 2.0f;

        obj->CreateCrashSphere(Math::Vector(0.0f, 0.0f, 0.0f), 4.0f, SOUND_BOUM, 0.10f);
        obj->SetGlobalSphere(Math::Vector(0.0f, 3.0f, 0.0f), 6.0f);
        obj->SetJostlingSphere(Math::Vector(0.0f, 0.0f, 0.0f), 8.0f);

        obj->CreateShadowCircle(8.0f, 0.5f);
    }

    if ( type == OBJECT_PLANT5 ||
         type == OBJECT_PLANT6 ||
         type == OBJECT_PLANT7 )  // clover?
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        if ( type == OBJECT_PLANT5 )  m_modelManager->AddModelReference("plant5.mod", false, rank);
        if ( type == OBJECT_PLANT6 )  m_modelManager->AddModelReference("plant6.mod", false, rank);
        if ( type == OBJECT_PLANT7 )  m_modelManager->AddModelReference("plant7.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);

//?     obj->CreateCrashSphere(Math::Vector(0.0f, 0.0f, 0.0f), 3.0f, SOUND_BOUM, 0.10f);
        obj->SetJostlingSphere(Math::Vector(0.0f, 0.0f, 0.0f), 4.0f);

        obj->CreateShadowCircle(5.0f, 0.3f);
    }

    if ( type == OBJECT_PLANT8 ||
         type == OBJECT_PLANT9 )  // squash?
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        if ( type == OBJECT_PLANT8 )  m_modelManager->AddModelReference("plant8.mod", false, rank);
        if ( type == OBJECT_PLANT9 )  m_modelManager->AddModelReference("plant9.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);

        obj->CreateCrashSphere(Math::Vector(0.0f,  2.0f, 0.0f), 4.0f, SOUND_BOUM, 0.10f);
        obj->CreateCrashSphere(Math::Vector(0.0f, 10.0f, 0.0f), 4.0f, SOUND_BOUM, 0.10f);

        obj->CreateShadowCircle(10.0f, 0.5f);
    }

    if ( type == OBJECT_PLANT10 ||
         type == OBJECT_PLANT11 ||
         type == OBJECT_PLANT12 ||
         type == OBJECT_PLANT13 ||
         type == OBJECT_PLANT14 )  // succulent?
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        if ( type == OBJECT_PLANT10 )  m_modelManager->AddModelReference("plant10.mod", false, rank);
        if ( type == OBJECT_PLANT11 )  m_modelManager->AddModelReference("plant11.mod", false, rank);
        if ( type == OBJECT_PLANT12 )  m_modelManager->AddModelReference("plant12.mod", false, rank);
        if ( type == OBJECT_PLANT13 )  m_modelManager->AddModelReference("plant13.mod", false, rank);
        if ( type == OBJECT_PLANT14 )  m_modelManager->AddModelReference("plant14.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);

        obj->CreateCrashSphere(Math::Vector(0.0f, 12.0f, 0.0f), 5.0f, SOUND_BOUM, 0.10f);
        obj->SetGlobalSphere(Math::Vector(0.0f, 6.0f, 0.0f), 6.0f);
        obj->SetJostlingSphere(Math::Vector(0.0f, 4.0f, 0.0f), 8.0f);

        obj->CreateShadowCircle(8.0f, 0.3f);
    }

    if ( type == OBJECT_PLANT15 ||
         type == OBJECT_PLANT16 ||
         type == OBJECT_PLANT17 ||
         type == OBJECT_PLANT18 ||
         type == OBJECT_PLANT19 )  // fern?
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        if ( type == OBJECT_PLANT15 )  m_modelManager->AddModelReference("plant15.mod", false, rank);
        if ( type == OBJECT_PLANT16 )  m_modelManager->AddModelReference("plant16.mod", false, rank);
        if ( type == OBJECT_PLANT17 )  m_modelManager->AddModelReference("plant17.mod", false, rank);
        if ( type == OBJECT_PLANT18 )  m_modelManager->AddModelReference("plant18.mod", false, rank);
        if ( type == OBJECT_PLANT19 )  m_modelManager->AddModelReference("plant19.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);

        if ( type != OBJECT_PLANT19 )
        {
            obj->CreateCrashSphere(Math::Vector(0.0f, 0.0f, 0.0f), 4.0f, SOUND_BOUM, 0.10f);
            obj->SetGlobalSphere(Math::Vector(0.0f, 3.0f, 0.0f), 6.0f);
        }
        obj->SetJostlingSphere(Math::Vector(0.0f, 0.0f, 0.0f), 8.0f);

        obj->CreateShadowCircle(8.0f, 0.5f);
    }

    if ( type == OBJECT_TREE0 )
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("tree0.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);

        obj->CreateCrashSphere(Math::Vector( 0.0f,  3.0f, 2.0f), 3.0f, SOUND_BOUMs, 0.20f);
        obj->CreateCrashSphere(Math::Vector(-1.0f, 10.0f, 1.0f), 2.0f, SOUND_BOUMs, 0.20f);
        obj->CreateCrashSphere(Math::Vector( 0.0f, 17.0f, 0.0f), 2.0f, SOUND_BOUMs, 0.20f);
        obj->CreateCrashSphere(Math::Vector( 1.0f, 27.0f, 0.0f), 2.0f, SOUND_BOUMs, 0.20f);

        obj->CreateShadowCircle(8.0f, 0.5f);
    }

    if ( type == OBJECT_TREE1 )
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("tree1.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);

        obj->CreateCrashSphere(Math::Vector( 0.0f,  3.0f, 2.0f), 3.0f, SOUND_BOUMs, 0.20f);
        obj->CreateCrashSphere(Math::Vector(-2.0f, 11.0f, 1.0f), 2.0f, SOUND_BOUMs, 0.20f);
        obj->CreateCrashSphere(Math::Vector(-2.0f, 19.0f, 2.0f), 2.0f, SOUND_BOUMs, 0.20f);
        obj->CreateCrashSphere(Math::Vector( 2.0f, 26.0f, 0.0f), 2.0f, SOUND_BOUMs, 0.20f);
        obj->CreateCrashSphere(Math::Vector( 2.0f, 34.0f,-2.0f), 2.0f, SOUND_BOUMs, 0.20f);

        obj->CreateShadowCircle(8.0f, 0.5f);
    }

    if ( type == OBJECT_TREE2 )
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("tree2.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);

        obj->CreateCrashSphere(Math::Vector( 0.0f,  3.0f, 1.0f), 3.0f, SOUND_BOUMs, 0.20f);
        obj->CreateCrashSphere(Math::Vector(-2.0f, 10.0f, 1.0f), 2.0f, SOUND_BOUMs, 0.20f);
        obj->CreateCrashSphere(Math::Vector(-2.0f, 19.0f, 2.0f), 2.0f, SOUND_BOUMs, 0.20f);
        obj->CreateCrashSphere(Math::Vector( 2.0f, 25.0f, 0.0f), 2.0f, SOUND_BOUMs, 0.20f);
        obj->CreateCrashSphere(Math::Vector( 3.0f, 32.0f,-2.0f), 2.0f, SOUND_BOUMs, 0.20f);

        obj->CreateShadowCircle(8.0f, 0.5f);
    }

    if ( type == OBJECT_TREE3 )
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("tree3.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);

        obj->CreateCrashSphere(Math::Vector(-2.0f,  3.0f, 2.0f), 3.0f, SOUND_BOUMs, 0.20f);
        obj->CreateCrashSphere(Math::Vector(-3.0f,  9.0f, 1.0f), 2.0f, SOUND_BOUMs, 0.20f);
        obj->CreateCrashSphere(Math::Vector( 0.0f, 18.0f, 0.0f), 2.0f, SOUND_BOUMs, 0.20f);
        obj->CreateCrashSphere(Math::Vector( 0.0f, 27.0f, 7.0f), 2.0f, SOUND_BOUMs, 0.20f);

        obj->CreateShadowCircle(8.0f, 0.5f);
    }

    if ( type == OBJECT_TREE4 )
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("tree4.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);

        obj->CreateCrashSphere(Math::Vector(0.0f, 10.0f, 0.0f), 10.0f, SOUND_BOUMs, 0.20f);
        obj->CreateCrashSphere(Math::Vector(0.0f, 21.0f, 0.0f),  8.0f, SOUND_BOUMs, 0.20f);
        obj->CreateCrashSphere(Math::Vector(0.0f, 32.0f, 0.0f),  7.0f, SOUND_BOUMs, 0.20f);

        obj->CreateShadowCircle(8.0f, 0.5f);
    }

    if ( type == OBJECT_TREE5 )  // giant tree (for the world "teen")
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("tree5.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);

        obj->CreateCrashSphere(Math::Vector(  0.0f, 5.0f,-10.0f), 25.0f, SOUND_BOUMs, 0.20f);
        obj->CreateCrashSphere(Math::Vector(-65.0f, 5.0f, 65.0f), 20.0f, SOUND_BOUMs, 0.20f);
        obj->CreateCrashSphere(Math::Vector( 38.0f, 5.0f, 21.0f), 18.0f, SOUND_BOUMs, 0.20f);

        obj->CreateShadowCircle(50.0f, 0.5f);
    }

    pos = obj->GetPosition(0);
    obj->SetPosition(0, pos);  // to display the shadows immediately

    obj->SetFloorHeight(0.0f);
    AddObjectAuto(obj.get());

    pos = obj->GetPosition(0);
    pos.y += height;
    obj->SetPosition(0, pos);

    return obj;
}

// Creates a mushroom placed on the ground.

CObjectUPtr CObjectFactory::CreateMushroom(const ObjectCreateParams& params)
{
    Math::Vector pos = params.pos;
    float angle = params.angle;
    float height = params.height;
    ObjectType type = params.type;

    CObjectUPtr obj(new CObject(params.id));

    obj->SetType(type);

    if ( type == OBJECT_MUSHROOM1 )
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("mush1.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);

        obj->CreateCrashSphere(Math::Vector(0.0f, 4.0f, 0.0f), 3.0f, SOUND_BOUM, 0.10f);
        obj->SetGlobalSphere(Math::Vector(0.0f, 3.0f, 0.0f), 5.5f);
        obj->SetJostlingSphere(Math::Vector(0.0f, 3.0f, 0.0f), 5.5f);

        obj->CreateShadowCircle(6.0f, 0.5f);
    }

    if ( type == OBJECT_MUSHROOM2 )
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("mush2.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);

        obj->CreateCrashSphere(Math::Vector(0.0f, 5.0f, 0.0f), 3.0f, SOUND_BOUM, 0.10f);
        obj->SetGlobalSphere(Math::Vector(0.0f, 4.0f, 0.0f), 5.5f);
        obj->SetJostlingSphere(Math::Vector(0.0f, 4.0f, 0.0f), 5.5f);

        obj->CreateShadowCircle(5.0f, 0.5f);
    }

    pos = obj->GetPosition(0);
    obj->SetPosition(0, pos);  // to display the shadows immediately

    obj->SetFloorHeight(0.0f);
    AddObjectAuto(obj.get());

    pos = obj->GetPosition(0);
    pos.y += height;
    obj->SetPosition(0, pos);

    return obj;
}

// Creates a toy placed on the ground.

CObjectUPtr CObjectFactory::CreateTeen(const ObjectCreateParams& params)
{
    Math::Vector pos = params.pos;
    float angle = params.angle;
    float zoom = params.zoom;
    float height = params.height;
    ObjectType type = params.type;
    int option = params.option;

    CObjectUPtr obj(new CObject(params.id));

    obj->SetType(type);
    obj->SetOption(option);

    float fShadow = Math::Norm(1.0f-height/10.0f);
    bool floorAdjust = true;

    if ( type == OBJECT_TEEN0 )  // orange pencil lg=10
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen0.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        obj->CreateCrashSphere(Math::Vector( 5.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 2.5f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 0.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-2.5f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-5.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);

        obj->CreateShadowCircle(5.0f, 0.8f*fShadow, Gfx::ENG_SHADOW_WORM);
    }

    if ( type == OBJECT_TEEN1 )  // blue pencil lg=14
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen1.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        obj->CreateCrashSphere(Math::Vector( 6.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 4.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 2.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 0.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-2.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-4.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-6.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);

        obj->CreateShadowCircle(6.0f, 0.8f*fShadow, Gfx::ENG_SHADOW_WORM);
    }

    if ( type == OBJECT_TEEN2 )  // red pencil lg=16
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen2.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        obj->CreateCrashSphere(Math::Vector( 7.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 4.7f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 2.3f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 0.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-2.3f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-4.7f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-7.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);

        obj->CreateShadowCircle(6.0f, 0.8f*fShadow, Gfx::ENG_SHADOW_WORM);
    }

    if ( type == OBJECT_TEEN3 )  // jar with pencils
    {
        int rank = m_engine->CreateObject();
//?     m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_METAL);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen3.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        obj->CreateCrashSphere(Math::Vector( 0.0f, 4.0f, 0.0f), 4.0f, SOUND_BOUMm, 0.45f);
        obj->SetGlobalSphere(Math::Vector(0.0f, 4.0f, 0.0f), 4.0f);
        obj->CreateShadowCircle(6.0f, 0.5f*fShadow);
    }

    if ( type == OBJECT_TEEN4 )  // scissors
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen4.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        obj->CreateCrashSphere(Math::Vector(-9.0f, 1.0f, 0.0f), 1.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-6.0f, 1.0f, 0.0f), 1.1f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-3.0f, 1.0f, 0.0f), 1.2f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 0.0f, 1.0f, 0.0f), 1.3f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 5.1f, 1.0f,-1.3f), 2.6f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 8.0f, 1.0f, 2.2f), 2.3f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 9.4f, 1.0f,-2.0f), 2.0f, SOUND_BOUMm, 0.45f);

        obj->CreateShadowCircle(10.0f, 0.5f*fShadow, Gfx::ENG_SHADOW_WORM);
    }

    if ( type == OBJECT_TEEN5 )  // CD
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen5.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);
        obj->SetFloorHeight(0.0f);
        floorAdjust = false;

        m_terrain->AddBuildingLevel(pos, 5.9f, 6.1f, 0.2f, 0.5f);
        obj->CreateShadowCircle(8.0f, 0.2f*fShadow);
    }

    if ( type == OBJECT_TEEN6 )  // book 1
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen6.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        obj->CreateCrashSphere(Math::Vector(-5.0f, 3.0f, 7.5f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 4.5f, 3.0f, 7.5f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-5.0f, 3.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 4.5f, 3.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-5.0f, 3.0f,-7.5f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 4.5f, 3.0f,-7.5f), 5.0f, SOUND_BOUMm, 0.45f);

        obj->CreateShadowCircle(20.0f, 0.2f*fShadow);
    }

    if ( type == OBJECT_TEEN7 )  // book 2
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen7.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        obj->CreateCrashSphere(Math::Vector(-5.0f, 3.0f, 7.5f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 4.5f, 3.0f, 7.5f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-5.0f, 3.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 4.5f, 3.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-5.0f, 3.0f,-7.5f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 4.5f, 3.0f,-7.5f), 5.0f, SOUND_BOUMm, 0.45f);

        obj->CreateShadowCircle(20.0f, 0.2f*fShadow);
    }

    if ( type == OBJECT_TEEN8 )  // a stack of books 1
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen8.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        obj->CreateCrashSphere(Math::Vector(-5.0f, 3.0f, 7.5f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 4.5f, 3.0f, 7.5f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-5.0f, 3.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 4.5f, 3.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-5.0f, 3.0f,-7.5f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 4.5f, 3.0f,-7.5f), 5.0f, SOUND_BOUMm, 0.45f);

        obj->SetGlobalSphere(Math::Vector(0.0f, 10.0f, 0.0f), 12.0f);
        obj->CreateShadowCircle(20.0f, 0.2f*fShadow);
    }

    if ( type == OBJECT_TEEN9 )  // a stack of books 2
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen9.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        obj->CreateCrashSphere(Math::Vector(-5.0f, 3.0f, 7.5f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 4.5f, 3.0f, 7.5f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-5.0f, 3.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 4.5f, 3.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-5.0f, 3.0f,-7.5f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 4.5f, 3.0f,-7.5f), 5.0f, SOUND_BOUMm, 0.45f);

        obj->SetGlobalSphere(Math::Vector(0.0f, 10.0f, 0.0f), 12.0f);
        obj->CreateShadowCircle(20.0f, 0.2f*fShadow);
    }

    if ( type == OBJECT_TEEN10 )  // bookcase
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen10.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        obj->CreateCrashSphere(Math::Vector(-26.0f, 3.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-15.0f, 3.0f,-4.0f), 6.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-15.0f, 3.0f, 5.0f), 6.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( -4.0f, 3.0f,-4.0f), 6.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( -4.0f, 3.0f, 5.0f), 6.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  6.0f, 3.0f,-4.0f), 6.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  6.0f, 3.0f, 4.0f), 6.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 14.0f, 3.0f,-3.0f), 6.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 14.0f, 3.0f, 2.0f), 6.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 24.0f, 3.0f, 5.0f), 6.0f, SOUND_BOUMm, 0.45f);

        obj->SetGlobalSphere(Math::Vector(0.0f, 6.0f, 0.0f), 20.0f);
        obj->CreateShadowCircle(40.0f, 0.2f*fShadow);
    }

    if ( type == OBJECT_TEEN11 )  // lamp
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen11.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetFloorHeight(0.0f);
        obj->SetZoom(0, zoom);

        Math::Matrix* mat = obj->GetWorldMatrix(0);
        pos = Math::Transform(*mat, Math::Vector(-56.0f, 22.0f, 0.0f));
        m_particle->CreateParticle(pos, Math::Vector(0.0f, 0.0f, 0.0f), Math::Point(20.0f, 20.0f), Gfx::PARTISELY, 1.0f, 0.0f, 0.0f);

        pos = Math::Transform(*mat, Math::Vector(-65.0f, 40.0f, 0.0f));
        Gfx::Color color;
        color.r = 4.0f;
        color.g = 2.0f;
        color.b = 0.0f;  // yellow-orange
        color.a = 0.0f;
        m_main->CreateSpot(pos, color);
    }

    if ( type == OBJECT_TEEN12 )  // coke
    {
        int rank = m_engine->CreateObject();
//?     m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_METAL);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen12.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        obj->CreateCrashSphere(Math::Vector( 0.0f, 4.0f, 0.0f), 4.0f, SOUND_BOUMm, 0.45f);
        obj->SetGlobalSphere(Math::Vector(0.0f, 9.0f, 0.0f), 5.0f);
        obj->CreateShadowCircle(4.5f, 1.0f*fShadow);
    }

    if ( type == OBJECT_TEEN13 )  // cardboard farm
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen13.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        obj->CreateCrashSphere(Math::Vector(-10.0f, 4.0f,-7.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  0.0f, 4.0f,-7.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 10.0f, 4.0f,-7.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-10.0f, 4.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  0.0f, 4.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 10.0f, 4.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-10.0f, 4.0f, 7.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  0.0f, 4.0f, 7.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 10.0f, 4.0f, 7.0f), 5.0f, SOUND_BOUMm, 0.45f);

        obj->SetGlobalSphere(Math::Vector(0.0f, 5.0f, 0.0f), 15.0f);
        obj->CreateShadowCircle(20.0f, 1.0f*fShadow);
    }

    if ( type == OBJECT_TEEN14 )  // open box
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen14.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        obj->CreateCrashSphere(Math::Vector(-10.0f, 4.0f,-7.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  0.0f, 4.0f,-7.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 10.0f, 4.0f,-7.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-10.0f, 4.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  0.0f, 4.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 10.0f, 4.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-10.0f, 4.0f, 7.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  0.0f, 4.0f, 7.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 10.0f, 4.0f, 7.0f), 5.0f, SOUND_BOUMm, 0.45f);

        obj->SetGlobalSphere(Math::Vector(0.0f, 5.0f, 0.0f), 15.0f);
        obj->CreateShadowCircle(20.0f, 1.0f*fShadow);
    }

    if ( type == OBJECT_TEEN15 )  // stack of cartons
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen15.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        obj->CreateCrashSphere(Math::Vector(-10.0f, 4.0f,-7.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  0.0f, 4.0f,-7.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 10.0f, 4.0f,-7.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-10.0f, 4.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  0.0f, 4.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 10.0f, 4.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-10.0f, 4.0f, 7.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  0.0f, 4.0f, 7.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 10.0f, 4.0f, 7.0f), 5.0f, SOUND_BOUMm, 0.45f);

        obj->SetGlobalSphere(Math::Vector(0.0f, 5.0f, 0.0f), 15.0f);
        obj->CreateShadowCircle(20.0f, 1.0f*fShadow);
    }

    if ( type == OBJECT_TEEN16 )  // watering can
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen16.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        obj->CreateCrashSphere(Math::Vector(-8.0f, 4.0f, 0.0f), 12.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 8.0f, 4.0f, 0.0f), 12.0f, SOUND_BOUMm, 0.45f);

        obj->SetGlobalSphere(Math::Vector(0.0f, 13.0f, 0.0f), 20.0f);
        obj->CreateShadowCircle(18.0f, 1.0f*fShadow);
    }

    if ( type == OBJECT_TEEN17 )  // wheel |
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen17.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        obj->CreateCrashSphere(Math::Vector( 0.0f, 31.0f, 0.0f), 31.0f, SOUND_BOUMm, 0.45f);
        obj->SetGlobalSphere(Math::Vector(0.0f, 31.0f, 0.0f), 31.0f);
        obj->CreateShadowCircle(24.0f, 0.5f*fShadow);
    }

    if ( type == OBJECT_TEEN18 )  // wheel /
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen18.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        obj->CreateCrashSphere(Math::Vector( 0.0f, 31.0f, 0.0f), 31.0f, SOUND_BOUMm, 0.45f);
        obj->SetGlobalSphere(Math::Vector(0.0f, 31.0f, 0.0f), 31.0f);
        obj->CreateShadowCircle(24.0f, 0.5f*fShadow);
    }

    if ( type == OBJECT_TEEN19 )  // wheel =
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen19.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        obj->CreateCrashSphere(Math::Vector( 0.0f, 10.0f, 0.0f), 32.0f, SOUND_BOUMm, 0.45f);
        obj->SetGlobalSphere(Math::Vector(0.0f, 10.0f, 0.0f), 32.0f);
        obj->CreateShadowCircle(33.0f, 1.0f*fShadow);
    }

    if ( type == OBJECT_TEEN20 )  // wall with shelf
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen20.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        obj->CreateCrashSphere(Math::Vector(-175.0f, 0.0f,  -5.0f), 4.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-175.0f, 0.0f, -35.0f), 4.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( -55.0f, 0.0f,  -5.0f), 4.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( -55.0f, 0.0f, -35.0f), 4.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( -37.0f, 0.0f,  -5.0f), 4.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( -37.0f, 0.0f, -35.0f), 4.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  83.0f, 0.0f,  -5.0f), 4.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  83.0f, 0.0f, -35.0f), 4.0f, SOUND_BOUMm, 0.45f);
    }

    if ( type == OBJECT_TEEN21 )  // wall with window
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen21.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);
    }

    if ( type == OBJECT_TEEN22 )  // wall with door and shelf
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen22.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        obj->CreateCrashSphere(Math::Vector(-135.0f, 0.0f,  -5.0f), 4.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-135.0f, 0.0f, -35.0f), 4.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( -15.0f, 0.0f,  -5.0f), 4.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( -15.0f, 0.0f, -35.0f), 4.0f, SOUND_BOUMm, 0.45f);
    }

    if ( type == OBJECT_TEEN23 )  // skateboard on wheels
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen23.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        if ( option == 1 )  // passage under the prohibited skateboard?
        {
            obj->CreateCrashSphere(Math::Vector(-10.0f, 2.0f, 0.0f), 11.0f, SOUND_BOUMm, 0.45f);
            obj->CreateCrashSphere(Math::Vector( 10.0f, 2.0f, 0.0f), 11.0f, SOUND_BOUMm, 0.45f);
        }

        obj->CreateCrashSphere(Math::Vector(-23.0f, 2.0f, 7.0f), 3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-23.0f, 2.0f, 0.0f), 3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-23.0f, 2.0f,-7.0f), 3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 23.0f, 2.0f, 7.0f), 3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 23.0f, 2.0f, 0.0f), 3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 23.0f, 2.0f,-7.0f), 3.0f, SOUND_BOUMm, 0.45f);

        obj->CreateShadowCircle(35.0f, 0.8f*fShadow, Gfx::ENG_SHADOW_WORM);
    }

    if ( type == OBJECT_TEEN24 )  // skate /
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen24.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        obj->CreateCrashSphere(Math::Vector(-12.0f, 0.0f, -3.0f), 3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-12.0f, 0.0f,  3.0f), 3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateShadowCircle(20.0f, 0.2f*fShadow);
    }

    if ( type == OBJECT_TEEN25 )  // skate /
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen25.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        obj->CreateCrashSphere(Math::Vector(-12.0f, 0.0f, -3.0f), 3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-12.0f, 0.0f,  3.0f), 3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateShadowCircle(20.0f, 0.2f*fShadow);
    }

    if ( type == OBJECT_TEEN26 )  // ceiling lamp
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen26.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);
        obj->SetFloorHeight(0.0f);

        Math::Matrix* mat = obj->GetWorldMatrix(0);
        pos = Math::Transform(*mat, Math::Vector(0.0f, 50.0f, 0.0f));
        m_particle->CreateParticle(pos, Math::Vector(0.0f, 0.0f, 0.0f), Math::Point(100.0f, 100.0f), Gfx::PARTISELY, 1.0f, 0.0f, 0.0f);

        pos = Math::Transform(*mat, Math::Vector(0.0f, 50.0f, 0.0f));
        Gfx::Color color;
        color.r = 4.0f;
        color.g = 2.0f;
        color.b = 0.0f;  // yellow-orange
        color.a = 0.0f;
        m_main->CreateSpot(pos, color);
    }

    if ( type == OBJECT_TEEN27 )  // large plant?
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen27.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        obj->CreateCrashSphere(Math::Vector(0.0f, 0.0f, 0.0f), 4.0f, SOUND_BOUM, 0.10f);
        obj->CreateShadowCircle(40.0f, 0.5f);
    }

    if ( type == OBJECT_TEEN28 )  // bottle?
    {
        int rank = m_engine->CreateObject();
//?     m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_METAL);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen28.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        obj->CreateCrashSphere(Math::Vector(0.0f, 2.0f, 0.0f), 5.0f, SOUND_BOUM, 0.10f);
        obj->CreateShadowCircle(7.0f, 0.6f*fShadow);
    }

    if ( type == OBJECT_TEEN29 )  // bridge?
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen29.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);
        floorAdjust = false;
    }

    if ( type == OBJECT_TEEN30 )  // jump?
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen30.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        obj->CreateCrashSphere(Math::Vector(0.0f, 4.0f, 0.0f), 15.0f, SOUND_BOUM, 0.10f);
        obj->SetGlobalSphere(Math::Vector(0.0f, 15.0f, 0.0f), 17.0f);
        obj->CreateShadowCircle(20.0f, 1.0f*fShadow);
    }

    if ( type == OBJECT_TEEN31 )  // basket?
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen31.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        obj->CreateCrashSphere(Math::Vector(-10.0f, 2.0f, 0.0f), 5.0f, SOUND_BOUM, 0.10f);
        obj->CreateCrashSphere(Math::Vector(  0.0f, 2.0f, 0.0f), 6.0f, SOUND_BOUM, 0.10f);
        obj->CreateCrashSphere(Math::Vector(  9.0f, 4.0f, 1.0f), 6.0f, SOUND_BOUM, 0.10f);

        obj->SetGlobalSphere(Math::Vector(0.0f, 0.0f, 0.0f), 10.0f);
        obj->CreateShadowCircle(16.0f, 0.6f*fShadow);
    }

    if ( type == OBJECT_TEEN32 )  // chair?
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen32.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        obj->CreateCrashSphere(Math::Vector( 17.5f, 1.0f,  17.5f), 3.5f, SOUND_BOUM, 0.10f);
        obj->CreateCrashSphere(Math::Vector( 17.5f, 1.0f, -17.5f), 3.5f, SOUND_BOUM, 0.10f);
        obj->CreateCrashSphere(Math::Vector(-17.5f, 1.0f,  17.5f), 3.5f, SOUND_BOUM, 0.10f);
        obj->CreateCrashSphere(Math::Vector(-17.5f, 1.0f, -17.5f), 3.5f, SOUND_BOUM, 0.10f);
        obj->SetGlobalSphere(Math::Vector(0.0f, 0.0f, 0.0f), 26.0f);
        obj->CreateShadowCircle(35.0f, 0.3f*fShadow);
    }

    if ( type == OBJECT_TEEN33 )  // panel?
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen33.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        obj->CreateCrashSphere(Math::Vector(0.0f, 2.0f, 0.0f), 4.0f, SOUND_BOUM, 0.10f);
        obj->CreateShadowCircle(10.0f, 0.3f*fShadow);
    }

    if ( type == OBJECT_TEEN34 )  // stone?
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen34.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        obj->CreateCrashSphere(Math::Vector(0.0f, 2.0f, 0.0f), 4.0f, SOUND_BOUM, 0.10f);
        obj->CreateShadowCircle(3.0f, 1.0f*fShadow);
    }

    if ( type == OBJECT_TEEN35 )  // pipe?
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen35.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        obj->CreateCrashSphere(Math::Vector(-40.0f, 5.0f, 0.0f), 10.0f, SOUND_BOUM, 0.10f);
        obj->CreateCrashSphere(Math::Vector(-20.0f, 5.0f, 0.0f), 10.0f, SOUND_BOUM, 0.10f);
        obj->CreateCrashSphere(Math::Vector(  0.0f, 5.0f, 0.0f), 10.0f, SOUND_BOUM, 0.10f);
        obj->CreateCrashSphere(Math::Vector( 20.0f, 5.0f, 0.0f), 10.0f, SOUND_BOUM, 0.10f);
        obj->CreateCrashSphere(Math::Vector( 40.0f, 5.0f, 0.0f), 10.0f, SOUND_BOUM, 0.10f);
        obj->CreateShadowCircle(40.0f, 0.8f*fShadow, Gfx::ENG_SHADOW_WORM);
    }

    if ( type == OBJECT_TEEN36 )  // trunk?
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen36.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);
        floorAdjust = false;
    }

    if ( type == OBJECT_TEEN37 )  // boat?
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen37.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);
        floorAdjust = false;
    }

    if ( type == OBJECT_TEEN38 )  // fan?
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen38a.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        m_modelManager->AddModelReference("teen38b.mod", false, rank);  // engine
        obj->SetPosition(1, Math::Vector(0.0f, 30.0f, 0.0f));

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(2, rank);
        obj->SetObjectParent(2, 1);
        m_modelManager->AddModelReference("teen38c.mod", false, rank);  // propeller
        obj->SetPosition(2, Math::Vector(0.0f, 0.0f, 0.0f));

        obj->CreateCrashSphere(Math::Vector(0.0f, 2.0f, 0.0f), 10.0f, SOUND_BOUM, 0.10f);
        obj->SetGlobalSphere(Math::Vector(0.0f, 2.0f, 0.0f), 10.0f);
        obj->CreateShadowCircle(15.0f, 0.5f*fShadow);
    }

    if ( type == OBJECT_TEEN39 )  // potted plant?
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen39.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        obj->CreateCrashSphere(Math::Vector(0.0f, 2.0f, 0.0f), 8.5f, SOUND_BOUM, 0.10f);
        obj->SetGlobalSphere(Math::Vector(0.0f, 2.0f, 0.0f), 8.5f);
        obj->CreateShadowCircle(10.0f, 1.0f*fShadow);
    }

    if ( type == OBJECT_TEEN40 )  // balloon?
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen40.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        obj->CreateCrashSphere(Math::Vector(0.0f, 5.0f, 0.0f), 11.0f, SOUND_BOUM, 0.10f);
        obj->SetGlobalSphere(Math::Vector(0.0f, 14.0f, 0.0f), 15.0f);
        obj->CreateShadowCircle(15.0f, 0.7f*fShadow);
    }

    if ( type == OBJECT_TEEN41 )  // fence?
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen41.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);
    }

    if ( type == OBJECT_TEEN42 )  // clover?
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen42.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        obj->CreateCrashSphere(Math::Vector(0.0f, 2.0f, 0.0f), 2.0f, SOUND_BOUM, 0.10f);
        obj->CreateShadowCircle(15.0f, 0.4f*fShadow);
    }

    if ( type == OBJECT_TEEN43 )  // clover?
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen43.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        obj->CreateCrashSphere(Math::Vector(0.0f, 2.0f, 0.0f), 2.0f, SOUND_BOUM, 0.10f);
        obj->CreateShadowCircle(15.0f, 0.4f*fShadow);
    }

    if ( type == OBJECT_TEEN44 )  // car?
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("teen44.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, zoom);

        obj->CreateCrashSphere(Math::Vector(0.0f, 10.0f, 0.0f), 55.0f, SOUND_BOUM, 0.10f);
        obj->SetGlobalSphere(Math::Vector(0.0f, 10.0f, 0.0f), 55.0f);
        obj->CreateShadowCircle(55.0f, 1.0f*fShadow);
    }

    pos = obj->GetPosition(0);
    obj->SetPosition(0, pos);  // to display the shadows immediately

    if ( floorAdjust )
    {
        obj->SetFloorHeight(0.0f);
        obj->FloorAdjust();
    }

    AddObjectAuto(obj.get());

    pos = obj->GetPosition(0);
    pos.y += height;
    obj->SetPosition(0, pos);

    return obj;
}

// Creates a crystal placed on the ground.

CObjectUPtr CObjectFactory::CreateQuartz(const ObjectCreateParams& params)
{
    Math::Vector pos = params.pos;
    float angle = params.angle;
    float height = params.height;
    ObjectType type = params.type;

    CObjectUPtr obj(new CObject(params.id));

    obj->SetType(type);

    if ( type == OBJECT_QUARTZ0 )
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_QUARTZ);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("quartz0.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);

        obj->CreateCrashSphere(Math::Vector(0.0f, 2.0f, 0.0f), 3.5f, SOUND_BOUMm, 0.45f);
        obj->SetGlobalSphere(Math::Vector(0.0f, 2.0f, 0.0f), 3.5f);

        obj->CreateShadowCircle(4.0f, 0.5f);
    }
    if ( type == OBJECT_QUARTZ1 )
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_QUARTZ);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("quartz1.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);

        obj->CreateCrashSphere(Math::Vector(0.0f, 4.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->SetGlobalSphere(Math::Vector(0.0f, 4.0f, 0.0f), 5.0f);

        obj->CreateShadowCircle(5.0f, 0.5f);
    }
    if ( type == OBJECT_QUARTZ2 )
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_QUARTZ);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("quartz2.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);

        obj->CreateCrashSphere(Math::Vector(0.0f, 6.0f, 0.0f), 6.0f, SOUND_BOUMm, 0.45f);
        obj->SetGlobalSphere(Math::Vector(0.0f, 6.0f, 0.0f), 6.0f);

        obj->CreateShadowCircle(6.0f, 0.5f);
    }
    if ( type == OBJECT_QUARTZ3 )
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_QUARTZ);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("quartz3.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);

        obj->CreateCrashSphere(Math::Vector(0.0f, 10.0f, 0.0f), 10.0f, SOUND_BOUMm, 0.45f);
        obj->SetGlobalSphere(Math::Vector(0.0f, 10.0f, 0.0f), 10.0f);

        obj->CreateShadowCircle(10.0f, 0.5f);
    }

    pos = obj->GetPosition(0);
    obj->SetPosition(0, pos);  // to display the shadows immediately

    obj->SetFloorHeight(0.0f);
    AddObjectAuto(obj.get());

    pos = obj->GetPosition(0);
    pos.y += height;
    obj->SetPosition(0, pos);

    float radius = 0.0f;
    if ( type == OBJECT_QUARTZ0 )
    {
        pos.y += 4.0f;
        radius = 2.0f;
    }
    if ( type == OBJECT_QUARTZ1 )
    {
        pos.y += 6.0f;
        radius = 4.0f;
    }
    if ( type == OBJECT_QUARTZ2 )
    {
        pos.y += 10.0f;
        radius = 5.0f;
    }
    if ( type == OBJECT_QUARTZ3 )
    {
        pos.y += 16.0f;
        radius = 8.0f;
    }
    m_particle->CreateParticle(pos, pos, Math::Point(2.0f, 2.0f), Gfx::PARTIQUARTZ, 0.7f+Math::Rand()*0.7f, radius, 0.0f);
    m_particle->CreateParticle(pos, pos, Math::Point(2.0f, 2.0f), Gfx::PARTIQUARTZ, 0.7f+Math::Rand()*0.7f, radius, 0.0f);

    return obj;
}

// Creates a root placed on the ground.
CObjectUPtr CObjectFactory::CreateRoot(const ObjectCreateParams& params)
{
    Math::Vector pos = params.pos;
    float angle = params.angle;
    float height = params.height;
    ObjectType type = params.type;

    CObjectUPtr obj(new CObject(params.id));

    obj->SetType(type);

    if ( type == OBJECT_ROOT0 )
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("root0.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, 2.0f);

        obj->CreateCrashSphere(Math::Vector(-5.0f,  1.0f,  0.0f), 2.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector( 4.0f,  1.0f,  2.0f), 2.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector( 4.0f,  1.0f, -3.0f), 2.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector( 2.0f,  5.0f, -1.0f), 1.5f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector(-4.0f,  5.0f, -1.0f), 1.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector(-2.0f,  8.0f, -0.5f), 1.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector( 0.0f, 10.0f, -0.5f), 1.0f, SOUND_BOUMv, 0.15f);
//?     obj->SetGlobalSphere(Math::Vector(0.0f, 6.0f, 0.0f), 11.0f);

        obj->CreateShadowCircle(16.0f, 0.5f);
    }
    if ( type == OBJECT_ROOT1 )
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("root1.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, 2.0f);

        obj->CreateCrashSphere(Math::Vector(-4.0f,  1.0f,  1.0f), 2.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector( 0.0f,  1.0f,  2.0f), 1.5f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector( 3.0f,  1.0f, -2.0f), 2.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector(-2.0f,  5.0f,  1.0f), 1.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector( 2.0f,  5.0f,  0.0f), 1.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector( 0.0f,  8.0f,  1.0f), 1.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector( 0.0f, 12.0f,  1.0f), 1.0f, SOUND_BOUMv, 0.15f);
//?     obj->SetGlobalSphere(Math::Vector(0.0f, 6.0f, 0.0f), 12.0f);

        obj->CreateShadowCircle(16.0f, 0.5f);
    }
    if ( type == OBJECT_ROOT2 )
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("root2.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, 2.0f);

        obj->CreateCrashSphere(Math::Vector(-3.0f,  1.0f,  0.5f), 2.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector( 3.0f,  1.0f, -1.0f), 2.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector(-1.0f,  4.5f,  0.0f), 1.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector( 3.0f,  7.0f,  1.0f), 1.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector( 0.0f,  7.0f, -1.0f), 1.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector( 4.0f, 11.0f,  1.0f), 1.0f, SOUND_BOUMv, 0.15f);
//?     obj->SetGlobalSphere(Math::Vector(0.0f, 6.0f, 0.0f), 10.0f);

        obj->CreateShadowCircle(16.0f, 0.5f);
    }
    if ( type == OBJECT_ROOT3 )
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("root3.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, 2.0f);

        obj->CreateCrashSphere(Math::Vector(-4.0f,  1.0f,  1.0f), 3.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector( 4.0f,  1.0f, -3.0f), 3.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector( 6.0f,  1.0f,  4.0f), 3.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector(-2.5f,  7.0f,  2.0f), 2.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector( 4.0f,  7.0f,  2.0f), 2.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector( 3.0f,  6.0f, -1.0f), 1.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector( 0.0f, 12.0f,  0.0f), 2.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector( 1.0f, 16.0f,  0.0f), 1.0f, SOUND_BOUMv, 0.15f);
//?     obj->SetGlobalSphere(Math::Vector(0.0f, 10.0f, 0.0f), 14.0f);

        obj->CreateShadowCircle(22.0f, 0.5f);
    }
    if ( type == OBJECT_ROOT4 )
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("root4.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, 2.0f);

        obj->CreateCrashSphere(Math::Vector( -7.0f,  2.0f,  3.0f), 4.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector(  5.0f,  2.0f, -6.0f), 4.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector(  6.0f,  2.0f,  6.0f), 3.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector(-11.0f,  1.0f, -2.0f), 2.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector(  1.0f,  1.0f, -7.0f), 2.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector( -4.0f, 10.0f,  3.0f), 2.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector(  1.0f, 11.0f,  7.0f), 2.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector(  3.0f, 11.0f, -3.0f), 2.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector( -3.0f, 17.0f,  1.0f), 2.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector( -3.0f, 23.0f, -1.0f), 2.0f, SOUND_BOUMv, 0.15f);
//?     obj->SetGlobalSphere(Math::Vector(0.0f, 12.0f, 0.0f), 20.0f);

        obj->CreateShadowCircle(30.0f, 0.5f);
    }
    if ( type == OBJECT_ROOT5 )  // gravity root ?
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("root4.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, 2.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        m_modelManager->AddModelReference("root5.mod", false, rank);
        obj->SetPosition(1, Math::Vector(-5.0f, 28.0f, -4.0f));
        obj->SetAngleX(1, -30.0f*Math::PI/180.0f);
        obj->SetAngleZ(1,  20.0f*Math::PI/180.0f);

        obj->CreateCrashSphere(Math::Vector( -7.0f,  2.0f,  3.0f), 4.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector(  5.0f,  2.0f, -6.0f), 4.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector(  6.0f,  2.0f,  6.0f), 3.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector(-11.0f,  1.0f, -2.0f), 2.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector(  1.0f,  1.0f, -7.0f), 2.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector( -4.0f, 10.0f,  3.0f), 2.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector(  1.0f, 11.0f,  7.0f), 2.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector(  3.0f, 11.0f, -3.0f), 2.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector( -3.0f, 17.0f,  1.0f), 2.0f, SOUND_BOUMv, 0.15f);
        obj->CreateCrashSphere(Math::Vector( -3.0f, 23.0f, -1.0f), 2.0f, SOUND_BOUMv, 0.15f);
//?     obj->SetGlobalSphere(Math::Vector(0.0f, 12.0f, 0.0f), 20.0f);

        obj->CreateShadowCircle(30.0f, 0.5f);
    }

    pos = obj->GetPosition(0);
    obj->SetPosition(0, pos);  // to display the shadows immediately

    obj->SetFloorHeight(0.0f);
    AddObjectAuto(obj.get());

    pos = obj->GetPosition(0);
    pos.y += height;
    obj->SetPosition(0, pos);

    return obj;
}

// Creates a small home.
CObjectUPtr CObjectFactory::CreateHome(const ObjectCreateParams& params)
{
    Math::Vector pos = params.pos;
    float angle = params.angle;
    float height = params.height;
    ObjectType type = params.type;

    CObjectUPtr obj(new CObject(params.id));

    obj->SetType(type);

    if ( type == OBJECT_HOME1 )
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("home1.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, 1.3f);

        obj->CreateCrashSphere(Math::Vector(0.0f, 5.0f, 0.0f), 10.0f, SOUND_BOUMs, 0.25f);
//?     obj->SetGlobalSphere(Math::Vector(0.0f, 6.0f, 0.0f), 11.0f);
        obj->CreateShadowCircle(16.0f, 0.5f);
    }

    pos = obj->GetPosition(0);
    obj->SetPosition(0, pos);  // to display the shadows immediately

    obj->SetFloorHeight(0.0f);
    AddObjectAuto(obj.get());

    pos = obj->GetPosition(0);
    pos.y += height;
    obj->SetPosition(0, pos);

    return obj;
}

// Creates ruin placed on the ground.
CObjectUPtr CObjectFactory::CreateRuin(const ObjectCreateParams& params)
{
    Math::Vector pos = params.pos;
    float angle = params.angle;
    float height = params.height;
    ObjectType type = params.type;

    CObjectUPtr obj(new CObject(params.id));

    obj->SetType(type);

    int rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);  // it is a stationary object
    obj->SetObjectRank(0, rank);

    std::string name;
    if ( type == OBJECT_RUINmobilew1 )  name = "ruin1.mod";
    if ( type == OBJECT_RUINmobilew2 )  name = "ruin1.mod";
    if ( type == OBJECT_RUINmobilet1 )  name = "ruin2.mod";
    if ( type == OBJECT_RUINmobilet2 )  name = "ruin2.mod";
    if ( type == OBJECT_RUINmobiler1 )  name = "ruin3.mod";
    if ( type == OBJECT_RUINmobiler2 )  name = "ruin3.mod";
    if ( type == OBJECT_RUINfactory  )  name = "ruin4.mod";
    if ( type == OBJECT_RUINdoor     )  name = "ruin5.mod";
    if ( type == OBJECT_RUINsupport  )  name = "ruin6.mod";
    if ( type == OBJECT_RUINradar    )  name = "ruin7.mod";
    if ( type == OBJECT_RUINconvert  )  name = "ruin8.mod";
    if ( type == OBJECT_RUINbase     )  name = "ruin9.mod";
    if ( type == OBJECT_RUINhead     )  name = "ruin10.mod";

    m_modelManager->AddModelReference(name, false, rank);

    obj->SetPosition(0, pos);
    obj->SetAngleY(0, angle);

    if ( type == OBJECT_RUINmobilew1 )  // vehicle had wheels?
    {
        // Creates the right-back wheel.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(6, rank);
        obj->SetObjectParent(6, 0);

        m_modelManager->AddModelReference("ruin1w.mod", false, rank);

        obj->SetPosition(6, Math::Vector(-3.0f, 1.8f, -4.0f));
        obj->SetAngleX(6, -Math::PI/2.0f);

        // Creates the left-back wheel.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(7, rank);
        obj->SetObjectParent(7, 0);

        m_modelManager->AddModelReference("ruin1w.mod", false, rank);

        obj->SetPosition(7, Math::Vector(-3.0f, 1.0f, 3.0f));
        obj->SetAngleY(7, Math::PI-0.3f);
        obj->SetAngleX(7, -0.3f);

        // Creates the right-front wheel.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(8, rank);
        obj->SetObjectParent(8, 0);

        m_modelManager->AddModelReference("ruin1w.mod", false, rank);

        obj->SetPosition(8, Math::Vector(2.0f, 1.6f, -3.0f));
        obj->SetAngleY(8, 0.3f);

        // Creates the left-front wheel.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(9, rank);
        obj->SetObjectParent(9, 0);

        m_modelManager->AddModelReference("ruin1w.mod", false, rank);

        obj->SetPosition(9, Math::Vector(2.0f, 1.0f, 3.0f));
        obj->SetAngleY(9, Math::PI-0.2f);
        obj->SetAngleX(9, 0.2f);

        obj->CreateCrashSphere(Math::Vector(0.0f, 2.8f, 0.0f), 3.0f, SOUND_BOUMm, 0.45f);
//?     obj->SetGlobalSphere(Math::Vector(0.0f, 5.0f, 0.0f), 10.0f);

        obj->CreateShadowCircle(4.0f, 1.0f);
    }

    if ( type == OBJECT_RUINmobilew2 )  // vehicle has wheels?
    {
        // Creates the left-back wheel.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(7, rank);
        obj->SetObjectParent(7, 0);

        m_modelManager->AddModelReference("ruin1w.mod", false, rank);

        obj->SetPosition(7, Math::Vector(-3.0f, 1.0f, 3.0f));
        obj->SetAngleY(7, Math::PI+0.3f);
        obj->SetAngleX(7, 0.4f);

        // Creates the left-front wheel.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(9, rank);
        obj->SetObjectParent(9, 0);

        m_modelManager->AddModelReference("ruin1w.mod", false, rank);

        obj->SetPosition(9, Math::Vector(2.0f, 1.0f, 3.0f));
        obj->SetAngleY(9, Math::PI+0.3f);
        obj->SetAngleX(9, -0.3f);

        obj->CreateCrashSphere(Math::Vector(0.0f, 2.8f, 0.0f), 3.0f, SOUND_BOUMm, 0.45f);
//?     obj->SetGlobalSphere(Math::Vector(0.0f, 5.0f, 0.0f), 10.0f);

        obj->CreateShadowCircle(4.0f, 1.0f);
    }

    if ( type == OBJECT_RUINmobilet1 )  // vehicle have caterpillars?
    {
        // Creates the cannon.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);

        m_modelManager->AddModelReference("ruin2c.mod", false, rank);

        obj->SetPosition(1, Math::Vector(3.0f, 5.0f, -2.5f));
        obj->SetAngleX(1, -Math::PI*0.85f);
        obj->SetAngleY(1, -0.4f);
        obj->SetAngleZ(1, -0.1f);

        obj->CreateCrashSphere(Math::Vector(1.0f, 2.8f, -1.0f), 5.0f, SOUND_BOUMm, 0.45f);
//?     obj->SetGlobalSphere(Math::Vector(1.0f, 5.0f, -1.0f), 10.0f);

        obj->CreateShadowCircle(5.0f, 1.0f);
    }

    if ( type == OBJECT_RUINmobilet2 )  // vehicle have caterpillars?
    {
        obj->CreateCrashSphere(Math::Vector(0.0f, 2.8f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
//?     obj->SetGlobalSphere(Math::Vector(0.0f, 5.0f, 0.0f), 10.0f);

        obj->CreateShadowCircle(5.0f, 1.0f);
    }

    if ( type == OBJECT_RUINmobiler1 )  // vehicle skating?
    {
        obj->CreateCrashSphere(Math::Vector(1.0f, 2.8f, -1.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->SetGlobalSphere(Math::Vector(1.0f, 5.0f, -1.0f), 10.0f);

        obj->CreateShadowCircle(5.0f, 1.0f);
    }

    if ( type == OBJECT_RUINmobiler2 )  // vehicle skating?
    {
        obj->CreateCrashSphere(Math::Vector(0.0f, 1.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->SetGlobalSphere(Math::Vector(0.0f, 5.0f, 0.0f), 10.0f);

        obj->CreateShadowCircle(6.0f, 1.0f);
    }

    if ( type == OBJECT_RUINfactory )  // factory ?
    {
        obj->CreateCrashSphere(Math::Vector(  9.0f,  1.0f, -11.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  0.0f,  2.0f, -11.0f), 4.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-10.0f,  4.0f, -10.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-12.0f, 11.0f,  -4.0f), 3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-10.0f,  4.0f,  -2.0f), 3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-11.0f,  8.0f,   3.0f), 3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-11.0f,  2.0f,   4.0f), 3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-11.0f,  2.0f,  10.0f), 3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( -4.0f,  0.0f,  10.0f), 3.0f, SOUND_BOUMm, 0.45f);
        obj->SetGlobalSphere(Math::Vector(0.0f, 0.0f, 0.0f), 18.0f);

        obj->CreateShadowCircle(20.0f, 0.7f);
    }

    if ( type == OBJECT_RUINdoor )  // converter holder?
    {
        obj->CreateCrashSphere(Math::Vector(0.0f, 0.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
//?     obj->SetGlobalSphere(Math::Vector(0.0f, 0.0f, 0.0f), 6.0f);

        obj->CreateShadowCircle(6.0f, 1.0f);
    }

    if ( type == OBJECT_RUINsupport )  // radar holder?
    {
        obj->CreateCrashSphere(Math::Vector(0.0f, 0.0f, 0.0f), 3.0f, SOUND_BOUMm, 0.45f);
//?     obj->SetGlobalSphere(Math::Vector(0.0f, 0.0f, 0.0f), 4.0f);

        obj->CreateShadowCircle(3.0f, 1.0f);
    }

    if ( type == OBJECT_RUINradar )  // radar base?
    {
        obj->CreateCrashSphere(Math::Vector(0.0f, 0.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
//?     obj->SetGlobalSphere(Math::Vector(0.0f, 0.0f, 0.0f), 6.0f);

        obj->CreateShadowCircle(6.0f, 1.0f);
    }

    if ( type == OBJECT_RUINconvert )  // converter?
    {
        m_terrain->AddBuildingLevel(pos, 7.0f, 9.0f, 1.0f, 0.5f);

        obj->CreateCrashSphere(Math::Vector(-10.0f,  0.0f,  4.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-10.0f,  0.0f, -4.0f), 5.0f, SOUND_BOUMm, 0.45f);
//?     obj->SetGlobalSphere(Math::Vector(-3.0f, 0.0f, 0.0f), 14.0f);
    }

    if ( type == OBJECT_RUINbase )  // base?
    {
        obj->CreateCrashSphere(Math::Vector(  0.0f, 15.0f,   0.0f),28.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 17.0f,  6.0f,  42.0f), 6.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 17.0f, 17.0f,  42.0f), 4.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-17.0f,  6.0f,  42.0f), 6.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-17.0f, 17.0f,  42.0f), 4.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-42.0f,  6.0f,  17.0f), 6.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-42.0f, 17.0f,  17.0f), 4.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-42.0f,  6.0f, -17.0f), 6.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-42.0f, 17.0f, -17.0f), 4.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-17.0f,  6.0f, -42.0f), 6.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-17.0f, 10.0f, -42.0f), 4.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 15.0f, 13.0f, -34.0f), 3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 31.0f, 15.0f, -13.0f), 3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 21.0f,  8.0f, -39.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 26.0f,  8.0f, -33.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->SetGlobalSphere(Math::Vector(0.0f, 0.0f, 0.0f), 48.0f);

        obj->CreateShadowCircle(40.0f, 1.0f);
    }

    if ( type == OBJECT_RUINhead )  // base cap?
    {
        obj->CreateCrashSphere(Math::Vector(  0.0f, 13.0f,   0.0f),20.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  0.0f, -8.0f,   0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  0.0f,-16.0f,   0.0f), 3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  0.0f,-22.0f,   0.0f), 3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-21.0f,  7.0f,   9.0f), 8.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( -9.0f,  7.0f,  21.0f), 8.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 21.0f,  7.0f,   9.0f), 8.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  9.0f,  7.0f,  21.0f), 8.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-21.0f,  7.0f,  -9.0f), 8.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( -9.0f,  7.0f, -21.0f), 8.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 21.0f,  7.0f,  -9.0f), 8.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  9.0f,  7.0f, -21.0f), 8.0f, SOUND_BOUMm, 0.45f);
        obj->SetGlobalSphere(Math::Vector(0.0f, 0.0f, 0.0f), 35.0f);

        obj->CreateShadowCircle(30.0f, 1.0f);
    }

    pos = obj->GetPosition(0);
    obj->SetPosition(0, pos);  //to display the shadows immediately

    obj->SetFloorHeight(0.0f);
    AddObjectAuto(obj.get());

    if ( type != OBJECT_RUINfactory &&
         type != OBJECT_RUINconvert &&
         type != OBJECT_RUINbase    )
    {
        obj->FloorAdjust();
    }

    pos = obj->GetPosition(0);
    pos.y += height;
    obj->SetPosition(0, pos);  //to display the shadows immediately

    if ( type == OBJECT_RUINmobilew1 )
    {
        pos = obj->GetPosition(0);
        pos.y -= 0.5f;
        obj->SetPosition(0, pos);

        angle = obj->GetAngleX(0)-0.1f;
        obj->SetAngleX(0, angle);
    }

    if ( type == OBJECT_RUINmobilew2 )
    {
        pos = obj->GetPosition(0);
        pos.y -= 1.5f;
        obj->SetPosition(0, pos);

        angle = obj->GetAngleX(0)-0.9f;
        obj->SetAngleX(0, angle);

        angle = obj->GetAngleZ(0)-0.1f;
        obj->SetAngleZ(0, angle);
    }

    if ( type == OBJECT_RUINmobilet1 )
    {
        pos = obj->GetPosition(0);
        pos.y -= 0.9f;
        obj->SetPosition(0, pos);

        angle = obj->GetAngleX(0)-0.3f;
        obj->SetAngleX(0, angle);
    }

    if ( type == OBJECT_RUINmobilet2 )
    {
        pos = obj->GetPosition(0);
        pos.y -= 1.5f;
        obj->SetPosition(0, pos);

        angle = obj->GetAngleX(0)-0.3f;
        obj->SetAngleX(0, angle);

        angle = obj->GetAngleZ(0)+0.8f;
        obj->SetAngleZ(0, angle);
    }

    if ( type == OBJECT_RUINmobiler1 )
    {
        pos = obj->GetPosition(0);
        pos.y += 4.0f;
        obj->SetPosition(0, pos);

        angle = obj->GetAngleX(0)-Math::PI*0.6f;
        obj->SetAngleX(0, angle);

        angle = obj->GetAngleZ(0)-0.2f;
        obj->SetAngleZ(0, angle);
    }

    if ( type == OBJECT_RUINmobiler2 )
    {
        pos = obj->GetPosition(0);
        pos.y += 2.0f;
        obj->SetPosition(0, pos);

        angle = obj->GetAngleX(0)-0.1f;
        obj->SetAngleX(0, angle);

        angle = obj->GetAngleZ(0)-0.3f;
        obj->SetAngleZ(0, angle);
    }

    if ( type == OBJECT_RUINdoor )
    {
        pos = obj->GetPosition(0);
        pos.y -= 0.5f;
        obj->SetPosition(0, pos);

        angle = obj->GetAngleZ(0)-0.1f;
        obj->SetAngleZ(0, angle);
    }

    if ( type == OBJECT_RUINsupport )
    {
        pos = obj->GetPosition(0);
        pos.y += 0.5f;
        obj->SetPosition(0, pos);

//?     angle = GetAngleY(0)+0.1f;
//?     obj->SetAngleY(0, angle);

        angle = obj->GetAngleX(0)+0.1f;
        obj->SetAngleX(0, angle);

        angle = obj->GetAngleZ(0)+0.1f;
        obj->SetAngleZ(0, angle);
    }

    if ( type == OBJECT_RUINradar )
    {
        pos = obj->GetPosition(0);
        pos.y -= 0.5f;
        obj->SetPosition(0, pos);

        angle = obj->GetAngleX(0)+0.15f;
        obj->SetAngleX(0, angle);

        angle = obj->GetAngleZ(0)+0.1f;
        obj->SetAngleZ(0, angle);
    }

    if ( type == OBJECT_RUINconvert )
    {
        pos = obj->GetPosition(0);
        pos.y -= 1.0f;
        obj->SetPosition(0, pos);
    }

    if ( type == OBJECT_RUINbase )
    {
        pos = obj->GetPosition(0);
        pos.y -= 1.0f;
        obj->SetPosition(0, pos);

        angle = obj->GetAngleX(0)+0.15f;
        obj->SetAngleX(0, angle);
    }

    if ( type == OBJECT_RUINhead )
    {
        pos = obj->GetPosition(0);
        pos.y += 8.0f;
        obj->SetPosition(0, pos);

        angle = obj->GetAngleX(0)+Math::PI*0.4f;
        obj->SetAngleX(0, angle);
    }

    return obj;
}

// Creates a gadget apollo.

CObjectUPtr CObjectFactory::CreateApollo(const ObjectCreateParams& params)
{
    Math::Vector pos = params.pos;
    float angle = params.angle;
    ObjectType type = params.type;

    CObjectUPtr obj(new CObject(params.id));

    obj->SetType(type);

    if ( type == OBJECT_APOLLO1 )  // LEM ?
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);  // it is a stationary object
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("apollol1.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetZoom(0, 1.2f);
        obj->SetFloorHeight(0.0f);

        for (int i=0 ; i<4 ; i++ )  // creates feet
        {
            rank = m_engine->CreateObject();
            m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
            obj->SetObjectRank(i+1, rank);
            obj->SetObjectParent(i+1, 0);
            m_modelManager->AddModelReference("apollol2.mod", false, rank);
            obj->SetAngleY(i+1, Math::PI/2.0f*i);
        }

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(5, rank);
        obj->SetObjectParent(5, 0);
        m_modelManager->AddModelReference("apollol3.mod", false, rank);  // ladder

//?     m_terrain->AddBuildingLevel(pos, 10.0f, 13.0f, 12.0f, 0.0f);

        obj->CreateCrashSphere(Math::Vector(  0.0f, 4.0f,   0.0f), 9.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 11.0f, 5.0f,   0.0f), 3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-11.0f, 5.0f,   0.0f), 3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  0.0f, 5.0f, -11.0f), 3.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(  0.0f, 5.0f,  11.0f), 3.0f, SOUND_BOUMm, 0.45f);

        obj->SetGlobalSphere(Math::Vector(0.0f, 4.0f, 0.0f), 9.0f);

        obj->CreateShadowCircle(16.0f, 0.5f);
    }

    if ( type == OBJECT_APOLLO2 )  // jeep
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);  //it is a stationary object
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("apolloj1.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetFloorHeight(0.0f);

        // Wheels.
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        m_modelManager->AddModelReference("apolloj4.mod", false, rank);  // wheel
        obj->SetPosition(1, Math::Vector(-5.75f, 1.65f, -5.0f));

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(2, rank);
        obj->SetObjectParent(2, 0);
        m_modelManager->AddModelReference("apolloj4.mod", false, rank);  // wheel
        obj->SetPosition(2, Math::Vector(-5.75f, 1.65f, 5.0f));

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(3, rank);
        obj->SetObjectParent(3, 0);
        m_modelManager->AddModelReference("apolloj4.mod", false, rank);  // wheel
        obj->SetPosition(3, Math::Vector(5.75f, 1.65f, -5.0f));

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(4, rank);
        obj->SetObjectParent(4, 0);
        m_modelManager->AddModelReference("apolloj4.mod", false, rank);  // wheel
        obj->SetPosition(4, Math::Vector(5.75f, 1.65f, 5.0f));

        // Accessories:
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(5, rank);
        obj->SetObjectParent(5, 0);
        m_modelManager->AddModelReference("apolloj2.mod", false, rank);  // antenna
        obj->SetPosition(5, Math::Vector(5.5f, 8.8f, 2.0f));
        obj->SetAngleY(5, -120.0f*Math::PI/180.0f);
        obj->SetAngleZ(5,   45.0f*Math::PI/180.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(6, rank);
        obj->SetObjectParent(6, 0);
        m_modelManager->AddModelReference("apolloj3.mod", false, rank);  // camera
        obj->SetPosition(6, Math::Vector(5.5f, 2.8f, -2.0f));
        obj->SetAngleY(6, 30.0f*Math::PI/180.0f);

        obj->CreateCrashSphere(Math::Vector( 3.0f, 2.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector(-3.0f, 2.0f, 0.0f), 5.0f, SOUND_BOUMm, 0.45f);
        obj->CreateCrashSphere(Math::Vector( 7.0f, 9.0f, 2.0f), 2.0f, SOUND_BOUMm, 0.20f);

        obj->CreateShadowCircle(7.0f, 0.8f);

        obj->FloorAdjust();
    }

    if ( type == OBJECT_APOLLO3 )  // flag?
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);  // it is a stationary object
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("apollof.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetFloorHeight(0.0f);

        obj->SetJostlingSphere(Math::Vector(0.0f, 4.0f, 0.0f), 1.0f);
        obj->CreateShadowCircle(2.0f, 0.3f);
    }

    if ( type == OBJECT_APOLLO4 )  // module?
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);  // it is a stationary object
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("apollom.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetFloorHeight(0.0f);

        obj->CreateCrashSphere(Math::Vector(0.0f, 2.0f, 0.0f), 2.0f, SOUND_BOUMm, 0.45f);
        obj->CreateShadowCircle(5.0f, 0.8f);

        obj->FloorAdjust();
    }

    if ( type == OBJECT_APOLLO5 )  // antenna?
    {
        int rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_FIX);  // it is a stationary object
        obj->SetObjectRank(0, rank);
        m_modelManager->AddModelReference("apolloa.mod", false, rank);
        obj->SetPosition(0, pos);
        obj->SetAngleY(0, angle);
        obj->SetFloorHeight(0.0f);

        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        obj->SetObjectRank(1, rank);
        obj->SetObjectParent(1, 0);
        m_modelManager->AddModelReference("apolloj2.mod", false, rank);  // antenna
        obj->SetPosition(1, Math::Vector(0.0f, 5.0f, 0.0f));
        obj->SetAngleY(1, -120.0f*Math::PI/180.0f);
        obj->SetAngleZ(1,   45.0f*Math::PI/180.0f);

        obj->CreateCrashSphere(Math::Vector(0.0f, 4.0f, 0.0f), 3.0f, SOUND_BOUMm, 0.35f);
        obj->CreateShadowCircle(3.0f, 0.7f);
    }

    AddObjectAuto(obj.get());

    pos = obj->GetPosition(0);
    obj->SetPosition(0, pos);  // to display the shadows immediately

    return obj;
}

// Creates a vehicle traveling any pose on the floor.

CObjectUPtr CObjectFactory::CreateVehicle(const ObjectCreateParams& params)
{
    Math::Vector pos = params.pos;
    float angle = params.angle;
    ObjectType type = params.type;
    float power = params.power;
    bool trainer = params.trainer;
    bool toy = params.toy;
    int option = params.option;

    CObjectUPtr obj(new CObject(params.id));

    obj->SetType(type);
    obj->SetOption(option);

    if ( type == OBJECT_TOTO )
    {
        std::unique_ptr<CMotion> motion{new CMotionToto(obj.get())};
        motion->Create(pos, angle, type, 1.0f, m_modelManager);
        obj->SetMotion(std::move(motion));
        return obj;
    }

    if ( type == OBJECT_HUMAN ||
         type == OBJECT_TECH  )
    {
        obj->SetTrainer(false);
    }
    else
    {
        obj->SetTrainer(trainer);
    }

    obj->SetToy(toy);

    float showLimitRadius = 0.0f;
#if 0
    if ( type == OBJECT_MOBILEfc ||
         type == OBJECT_MOBILEtc ||
         type == OBJECT_MOBILEwc ||
         type == OBJECT_MOBILEic )  // fireball cannon?
    {
        showLimitRadius = 160.0f;
    }
    if ( type == OBJECT_MOBILEfi ||
         type == OBJECT_MOBILEti ||
         type == OBJECT_MOBILEwi ||
         type == OBJECT_MOBILEii )  // orgaball cannon?
    {
        showLimitRadius = 160.0f;
    }
    if ( type == OBJECT_MOBILErc )  // phazer cannon?
    {
        showLimitRadius = 160.0f;
    }
    if ( type == OBJECT_MOBILErs )  // robot shield?
    {
        showLimitRadius = 50.0f;
    }
#endif
    if ( type == OBJECT_MOBILErt )  // robot thumper?
    {
        showLimitRadius = 400.0f;
    }
    obj->SetShowLimitRadius(showLimitRadius);

    std::unique_ptr<CPhysics> physics{new CPhysics(obj.get())};
    std::unique_ptr<CBrain> brain{new CBrain(obj.get())};
    std::unique_ptr<CMotion> motion;

    if ( type == OBJECT_HUMAN ||
         type == OBJECT_TECH  )
    {
        motion.reset(new CMotionHuman(obj.get()));
    }
    else if ( type == OBJECT_CONTROLLER )
    {
        motion.reset(new CMotionDummy(obj.get())); //dummy object
    }
    else
    {
        motion.reset(new CMotionVehicle(obj.get()));
    }

    brain->SetMotion(motion.get());
    brain->SetPhysics(physics.get());
    motion->SetBrain(brain.get());
    motion->SetPhysics(physics.get());
    physics->SetBrain(brain.get());
    physics->SetMotion(motion.get());

    motion->Create(pos, angle, type, power, m_modelManager);

    obj->SetBrain(std::move(brain));
    obj->SetMotion(std::move(motion));
    obj->SetPhysics(std::move(physics));

    return obj;
}

// Creates an insect lands on any ground.

CObjectUPtr CObjectFactory::CreateInsect(const ObjectCreateParams& params)
{
    Math::Vector pos = params.pos;
    float angle = params.angle;
    ObjectType type = params.type;

    CObjectUPtr obj(new CObject(params.id));

    obj->SetType(type);

    std::unique_ptr<CPhysics> physics{new CPhysics(obj.get())};
    std::unique_ptr<CBrain> brain{new CBrain(obj.get())};

    std::unique_ptr<CMotion> motion;
    if ( type == OBJECT_MOTHER )
    {
        motion.reset(new CMotionMother(obj.get()));
    }
    if ( type == OBJECT_ANT )
    {
        motion.reset(new CMotionAnt(obj.get()));
    }
    if ( type == OBJECT_SPIDER )
    {
        motion.reset(new CMotionSpider(obj.get()));
    }
    if ( type == OBJECT_BEE )
    {
        motion.reset(new CMotionBee(obj.get()));
    }
    if ( type == OBJECT_WORM )
    {
        motion.reset(new CMotionWorm(obj.get()));
    }
    assert(motion != nullptr);

    physics->SetBrain(brain.get());
    physics->SetMotion(motion.get());
    brain->SetMotion(motion.get());
    brain->SetPhysics(physics.get());
    motion->SetBrain(brain.get());
    motion->SetPhysics(physics.get());

    motion->Create(pos, angle, type, 0.0f, m_modelManager);

    obj->SetMotion(std::move(motion));
    obj->SetPhysics(std::move(physics));
    obj->SetBrain(std::move(brain));

    return obj;
}

// Creates all sub-objects for managing the object.

void CObjectFactory::AddObjectAuto(CObject* obj)
{
    std::unique_ptr<CAuto> objAuto;

    auto type = obj->GetType();

    if ( type == OBJECT_BASE )
    {
        objAuto.reset(new CAutoBase(obj));
    }
    if ( type == OBJECT_PORTICO )
    {
        objAuto.reset(new CAutoPortico(obj));
    }
    if ( type == OBJECT_DERRICK )
    {
        objAuto.reset(new CAutoDerrick(obj));
    }
    if ( type == OBJECT_FACTORY )
    {
        objAuto.reset(new CAutoFactory(obj));
    }
    if ( type == OBJECT_REPAIR )
    {
        objAuto.reset(new CAutoRepair(obj));
    }
    if ( type == OBJECT_DESTROYER )
    {
        objAuto.reset(new CAutoDestroyer(obj));
    }
    if ( type == OBJECT_STATION )
    {
        objAuto.reset(new CAutoStation(obj));
    }
    if ( type == OBJECT_CONVERT )
    {
        objAuto.reset(new CAutoConvert(obj));
    }
    if ( type == OBJECT_TOWER )
    {
        objAuto.reset(new CAutoTower(obj));
    }
    if ( type == OBJECT_RESEARCH )
    {
        objAuto.reset(new CAutoResearch(obj));
    }
    if ( type == OBJECT_RADAR )
    {
        objAuto.reset(new CAutoRadar(obj));
    }
    if ( type == OBJECT_ENERGY )
    {
        objAuto.reset(new CAutoEnergy(obj));
    }
    if ( type == OBJECT_LABO )
    {
        objAuto.reset(new CAutoLabo(obj));
    }
    if ( type == OBJECT_NUCLEAR )
    {
        objAuto.reset(new CAutoNuclear(obj));
    }
    if ( type == OBJECT_PARA )
    {
        objAuto.reset(new CAutoPara(obj));
    }
    if ( type == OBJECT_SAFE )
    {
        objAuto.reset(new CAutoSafe(obj));
    }
    if ( type == OBJECT_HUSTON )
    {
        objAuto.reset(new CAutoHuston(obj));
    }
    if ( type == OBJECT_EGG )
    {
        objAuto.reset(new CAutoEgg(obj));
    }
    if ( type == OBJECT_NEST )
    {
        objAuto.reset(new CAutoNest(obj));
    }
    if ( type == OBJECT_ROOT5 )
    {
        objAuto.reset(new CAutoRoot(obj));
    }
    if ( type == OBJECT_MUSHROOM2 )
    {
        objAuto.reset(new CAutoMush(obj));
    }
    if ( type == OBJECT_FLAGb ||
         type == OBJECT_FLAGr ||
         type == OBJECT_FLAGg ||
         type == OBJECT_FLAGy ||
         type == OBJECT_FLAGv )
    {
        objAuto.reset(new CAutoFlag(obj));
    }
    if ( type == OBJECT_TEEN36 ||  // trunk?
         type == OBJECT_TEEN37 ||  // boat?
         type == OBJECT_TEEN38 )   // fan?
    {
        objAuto.reset(new CAutoKid(obj));
    }

    if (objAuto != nullptr)
    {
        objAuto->Init();
        obj->SetAuto(std::move(objAuto));
    }
}
