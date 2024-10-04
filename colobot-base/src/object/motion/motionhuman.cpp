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


#include "object/motion/motionhuman.h"

#include "app/app.h"

#include "graphics/engine/engine.h"
#include "graphics/engine/oldmodelmanager.h"
#include "graphics/engine/terrain.h"
#include "graphics/engine/water.h"

#include "level/robotmain.h"

#include "math/geometry.h"

#include "object/object_manager.h"
#include "object/old_object.h"

#include "physics/physics.h"

#include "sound/sound.h"


#include <stdio.h>



const int ADJUST_ACTION = (3*3*3*3*MH_SPEC+3*3*3*MHS_SATCOM);

const float START_TIME = 1000.0f;       // beginning of the relative time



// Object's constructor.

CMotionHuman::CMotionHuman(COldObject* object)
    : CMotion(object),
      m_armAngles()
{
    m_partiReactor   = -1;
    m_armMember      = START_TIME;
    m_armTimeAbs     = START_TIME;
    m_armTimeAction  = START_TIME;
    m_armTimeSwim    = START_TIME;
    m_armTimeIndex   = 0;
    m_armPartIndex   = 0;
    m_armMemberIndex = 0;
    m_armLastAction  = -1;
    m_bArmStop = false;
    m_lastSoundMarch = 0.0f;
    m_lastSoundHhh = 0.0f;
    m_time = 0.0f;
    m_tired = 0.0f;
    m_bDisplayPerso = false;
    m_glassesRank = -1;
}

// Object's constructor.

CMotionHuman::~CMotionHuman()
{
}


// Removes an object.

void CMotionHuman::DeleteObject(bool bAll)
{
    if ( m_partiReactor != -1 )
    {
        m_particle->DeleteParticle(m_partiReactor);
        m_partiReactor = -1;
    }
}


// Starts an action.

Error CMotionHuman::SetAction(int action, float time)
{
    CMotion::SetAction(action, time);
    m_time = 0.0f;
    return ERR_OK;
}


// Creates cosmonaut on the ground.

void CMotionHuman::Create(glm::vec3 pos, float angle, ObjectType type,
                          float power, Gfx::COldModelManager* modelManager)
{
    std::array<char, 1000> filename;
    int         rank, option, face, glasses;

    m_object->SetType(type);
    option = m_object->GetOption();

    if ( m_main->GetGamerOnlyHead() )
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_VEHICLE);  // this is a moving object
        m_object->SetObjectRank(0, rank);
        face = m_main->GetGamerFace();
        snprintf(filename.data(), filename.size(), "human2h%d", face+1);
        modelManager->AddModelReference(filename.data(), false, rank);

        glasses = m_main->GetGamerGlasses();
        if ( glasses != 0 )
        {
            rank = m_engine->CreateObject();
            m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
            m_object->SetObjectRank(1, rank);
            m_object->SetObjectParent(1, 0);
            snprintf(filename.data(), filename.size(), "human2g%d", glasses);
            modelManager->AddModelReference(filename.data(), false, rank);
        }

        CreatePhysics(type);
        m_object->SetFloorHeight(0.0f);

        m_engine->LoadAllTextures();

        return;
    }

    // Creates the main base.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_VEHICLE);  // this is a moving object
    m_object->SetObjectRank(0, rank);

    if (option == 0)  // head in helmet?
        modelManager->AddModelReference("human1c", false, rank);
    else if (option == 1)  // head without helmet?
        modelManager->AddModelReference("human1h", false, rank);
    else if (option == 2)  // without a backpack?
        modelManager->AddModelReference("human1v", false, rank);

    m_object->SetPosition(pos);
    m_object->SetRotationY(angle);

    // A vehicle must have an obligatory collision with a sphere of center (0, y, 0) (see GetCrashSphere).
    m_object->AddCrashSphere(CrashSphere(glm::vec3(0.0f, 0.0f, 0.0f), 2.0f, SOUND_AIE, 0.20f));
    m_object->SetCameraCollisionSphere(Math::Sphere(glm::vec3(0.0f, 1.0f, 0.0f), 4.0f));

    // Creates the head.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(1, rank);
    m_object->SetObjectParent(1, 0);

    if ( type == OBJECT_HUMAN )
    {
        if (option == 0)  // head in helmet?
        {
            face = m_main->GetGamerFace();
            snprintf(filename.data(), filename.size(), "human2c%d", face+1);
            modelManager->AddModelReference(filename.data(), false, rank);
        }
        else if (option == 1 ||  // head without helmet?
                 option == 2)    // without a backpack?
        {
            face = m_main->GetGamerFace();
            snprintf(filename.data(), filename.size(), "human2h%d", face+1);
            modelManager->AddModelReference(filename.data(), false, rank);
        }
    }
    else if (type == OBJECT_TECH)
    {
        modelManager->AddModelReference("human2t", false, rank);
    }

    m_object->SetPartPosition(1, glm::vec3(0.0f, 2.7f, 0.0f));
    if (option == 1 ||  // head without helmet?
        option == 2)    // without a backpack?
    {
        m_object->SetPartScale(1, glm::vec3(1.0f, 1.05f, 1.0f));
    }

    // Creates the glasses.
    glasses = m_main->GetGamerGlasses();
    if ( glasses != 0 && type == OBJECT_HUMAN )
    {
        m_glassesRank = m_engine->CreateObject();
        m_engine->SetObjectType(m_glassesRank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(15, m_glassesRank);
        m_object->SetObjectParent(15, 1);
        snprintf(filename.data(), filename.size(), "human2g%d", glasses);
        modelManager->AddModelReference(filename.data(), false, m_glassesRank);
    }

    // Creates the right arm.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(2, rank);
    m_object->SetObjectParent(2, 0);
    modelManager->AddModelReference("human3", false, rank);
    m_object->SetPartPosition(2, glm::vec3(0.0f, 2.3f, -1.2f));
    m_object->SetPartRotation(2, glm::vec3(90.0f*Math::PI/180.0f, 90.0f*Math::PI/180.0f, -50.0f*Math::PI/180.0f));

    // Creates the right forearm.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(3, rank);
    m_object->SetObjectParent(3, 2);
    modelManager->AddModelReference("human4r", false, rank);
    m_object->SetPartPosition(3, glm::vec3(1.3f, 0.0f, 0.0f));
    m_object->SetPartRotation(3, glm::vec3(0.0f*Math::PI/180.0f, -20.0f*Math::PI/180.0f, 0.0f*Math::PI/180.0f));

    // Creates right hand.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(4, rank);
    m_object->SetObjectParent(4, 3);
    modelManager->AddModelReference("human5", false, rank);
    m_object->SetPartPosition(4, glm::vec3(1.2f, 0.0f, 0.0f));

    // Creates the right thigh.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(5, rank);
    m_object->SetObjectParent(5, 0);
    modelManager->AddModelReference("human6", false, rank);
    m_object->SetPartPosition(5, glm::vec3(0.0f, 0.0f, -0.7f));
    m_object->SetPartRotation(5, glm::vec3(10.0f*Math::PI/180.0f, 0.0f*Math::PI/180.0f, 5.0f*Math::PI/180.0f));

    // Creates the right leg.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(6, rank);
    m_object->SetObjectParent(6, 5);
    modelManager->AddModelReference("human7", false, rank);
    m_object->SetPartPosition(6, glm::vec3(0.0f, -1.5f, 0.0f));
    m_object->SetPartRotation(6, glm::vec3(0.0f*Math::PI/180.0f, 0.0f*Math::PI/180.0f, -10.0f*Math::PI/180.0f));

    // Creates the right foot.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(7, rank);
    m_object->SetObjectParent(7, 6);
    modelManager->AddModelReference("human8", false, rank);
    m_object->SetPartPosition(7, glm::vec3(0.0f, -1.5f, 0.0f));
    m_object->SetPartRotation(7, glm::vec3(-10.0f*Math::PI/180.0f, 5.0f*Math::PI/180.0f, 5.0f*Math::PI/180.0f));

    // Creates the left arm.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(8, rank);
    m_object->SetObjectParent(8, 0);
    modelManager->AddModelReference("human3", true, rank);
    m_object->SetPartPosition(8, glm::vec3(0.0f, 2.3f, 1.2f));
    m_object->SetPartRotation(8, glm::vec3(-90.0f*Math::PI/180.0f, -90.0f*Math::PI/180.0f, -50.0f*Math::PI/180.0f));

    // Creates the left forearm.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(9, rank);
    m_object->SetObjectParent(9, 8);
    modelManager->AddModelReference("human4l", true, rank);
    m_object->SetPartPosition(9, glm::vec3(1.3f, 0.0f, 0.0f));
    m_object->SetPartRotation(9, glm::vec3(0.0f*Math::PI/180.0f, 20.0f*Math::PI/180.0f, 0.0f*Math::PI/180.0f));

    // Creates left hand.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(10, rank);
    m_object->SetObjectParent(10, 9);
    modelManager->AddModelReference("human5", true, rank);
    m_object->SetPartPosition(10, glm::vec3(1.2f, 0.0f, 0.0f));

    // Creates the left thigh.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(11, rank);
    m_object->SetObjectParent(11, 0);
    modelManager->AddModelReference("human6", true, rank);
    m_object->SetPartPosition(11, glm::vec3(0.0f, 0.0f, 0.7f));
    m_object->SetPartRotation(11, glm::vec3(-10.0f*Math::PI/180.0f, 0.0f*Math::PI/180.0f, 5.0f*Math::PI/180.0f));

    // Creates the left leg.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(12, rank);
    m_object->SetObjectParent(12, 11);
    modelManager->AddModelReference("human7", true, rank);
    m_object->SetPartPosition(12, glm::vec3(0.0f, -1.5f, 0.0f));
    m_object->SetPartRotation(12, glm::vec3(0.0f*Math::PI/180.0f, 0.0f*Math::PI/180.0f, -10.0f*Math::PI/180.0f));

    // Creates the left foot.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(13, rank);
    m_object->SetObjectParent(13, 12);
    modelManager->AddModelReference("human8", true, rank);
    m_object->SetPartPosition(13, glm::vec3(0.0f, -1.5f, 0.0f));
    m_object->SetPartRotation(13, glm::vec3(10.0f*Math::PI/180.0f, -5.0f*Math::PI/180.0f, 5.0f*Math::PI/180.0f));

    // Creates the neutron gun.
    if ( option != 2 && !m_main->GetPlusExplorer())  // with backpack?
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
        m_object->SetObjectRank(14, rank);
        m_object->SetObjectParent(14, 0);
        modelManager->AddModelReference("human9", false, rank);
        m_object->SetPartPosition(14, glm::vec3(-1.5f, 0.3f, -1.35f));
        m_object->SetPartRotationZ(14, Math::PI);
    }

    m_object->CreateShadowCircle(2.0f, 0.8f);

    CreatePhysics(type);
    m_object->SetFloorHeight(0.0f);

    pos = m_object->GetPosition();
    m_object->SetPosition(pos);  // to display the shadows immediately

    m_engine->LoadAllTextures();
}

// Creates the physical object.

void CMotionHuman::CreatePhysics(ObjectType type)
{
    Character*  character;
    int         i;

    int member_march[] =
    {
    //  x1,y1,z1,   x2,y2,z2,   x3,y3,z3,   // in the air:
        90,90,-50,  10,0,55,    0,0,0,      // t0: arms/thighs/-
        0,-20,0,    -5,0,-110,  0,0,0,      // t0: forearm/legs/-
        0,0,0,      -5,0,40,    0,0,0,      // t0: hands/feet/-
                                            // on the ground:
        125,115,-45,    10,0,50,    0,0,0,      // t1: arms/thighs/-
        0,-20,0,    -5,0,-15,   0,0,0,      // t1: forearm/legs/-
        0,0,0,      -5,0,0,     0,0,0,      // t1: hands/feet/-
                                            // on the ground back:
        25,55,-40,  10,0,-15,   0,0,0,      // t2: arms/thighs/-
        30,-50,40,  -5,0,-55,   0,0,0,      // t2: forearm/legs/-
        0,0,0,      -5,0,25,    0,0,0,      // t2: hands/feet/-
    };

    int member_march_take[] =
    {
    //  x1,y1,z1,   x2,y2,z2,   x3,y3,z3,   // in the air:
        15,50,-50,  10,0,55,    0,0,0,      // t0: arms/thighs/-
        45,-70,10,  -5,0,-110,  0,0,0,      // t0: forearm/legs/-
        -10,25,0,   -5,0,40,    0,0,0,      // t0: hands/feet/-
                                            // on the ground:
        15,50,-55,  10,0,50,    0,0,0,      // t1: arms/thighs/-
        45,-70,10,  -5,0,-15,   0,0,0,      // t1: forearm/legs/-
        -10,25,0,   -5,0,0,     0,0,0,      // t1: hands/feet/-
                                            // on the ground back:
        15,50,-45,  10,0,-15,   0,0,0,      // t2: arms/thighs/-
        45,-70,10,  -5,0,-55,   0,0,0,      // t2: forearm/legs/-
        -10,25,0,   -5,0,45,    0,0,0,      // t2: hands/feet/-
    };

    int member_turn[] =
    {
    //  x1,y1,z1,   x2,y2,z2,   x3,y3,z3,   // in the air:
        90,90,-50,  10,0,30,    0,0,0,      // t0: arms/thighs/-
        0,-20,0,    -5,0,-60,   0,0,0,      // t0: forearm/legs/-
        0,0,0,      -5,0,30,    0,0,0,      // t0: hands/feet/-
                                            // on the ground:
        90,110,-45, 10,0,0,     0,0,0,      // t1: arms/thighs/-
        0,-20,0,    -5,5,0,     0,0,0,      // t1: forearm/legs/-
        0,0,0,      -5,10,0,    0,0,0,      // t1: hands/feet/-
                                            // on the ground back:
        90,70,-45,  10,0,0,     0,0,0,      // t2: arms/thighs/-
        0,-20,10,   -5,-5,0,    0,0,0,      // t2: forearm/legs/-
        0,0,0,      -5,-10,0,   0,0,0,      // t2: hands/feet/-
    };

    int member_stop[] =
    {
    //  x1,y1,z1,   x2,y2,z2,   x3,y3,z3,
        90,90,-50,  10,0,5,     0,0,0,      // arms/thighs/-
        0,-20,0,    0,0,-10,    0,0,0,      // forearm/legs/-
        0,0,0,      -10,5,5,    0,0,0,      // hands/feet/-
                                            //
        90,90,-55,  10,0,5,     0,0,0,      // arms/thighs/-
        0,-15,0,    0,0,-10,    0,0,0,      // forearm/legs/-
        0,0,0,      -10,5,5,    0,0,0,      // hands/feet/-
                                            //
        90,90,-60,  10,0,5,     0,0,0,      // arms/thighs/-
        0,-10,0,    0,0,-10,    0,0,0,      // forearm/legs/-
        0,0,0,      -10,5,5,    0,0,0,      // hands/feet/-
    };

    int member_fly[] =
    {
    //  x1,y1,z1,   x2,y2,z2,   x3,y3,z3,
        -5,90,-60,  20,5,-25,   0,0,0,      // arms/thighs/-
        85,-40,-25, 10,0,-30,   0,0,0,      // forearm/legs/-
        40,10,25,   0,15,0,     0,0,0,      // hands/feet/-
                                            //
        -15,90,-40, 20,5,-35,   0,0,0,      // arms/thighs/-
        85,-40,-25, 10,0,-40,   0,0,0,      // forearm/legs/-
        45,5,20,    0,15,0,     0,0,0,      // hands/feet/-
                                            //
        -25,90,-50, 20,5,-20,   0,0,0,      // arms/thighs/-
        85,-40,-25, 10,0,-10,   0,0,0,      // forearm/legs/-
        30,15,25,   0,15,0,     0,0,0,      // hands/feet/-
    };

    int member_swim[] =
    {
    //  x1,y1,z1,   x2,y2,z2,   x3,y3,z3,
        130,-70,200,    10,20,55,   0,0,0,      // arms/thighs/-
        115,-125,0, -5,0,-110,  0,0,0,      // forearm/legs/-
        0,0,0,      -5,10,-5,   0,0,0,      // hands/feet/-
                                            //
        130,-95,115,55,5,5,     0,0,0,      // arms/thighs/-
        75,-50,25,  -5,0,-15,   0,0,0,      // forearm/legs/-
        0,0,0,      -5,5,-30,   0,0,0,      // hands/feet/-
                                            //
        130,-100,220,5,0,0,     0,0,0,      // arms/thighs/-
        150,5,0,    -5,0,-15,   0,0,0,      // forearm/legs/-
        0,0,0,      -5,30,-20,  0,0,0,      // hands/feet/-
    };

    int member_spec[] =
    {
    //  x1,y1,z1,   x2,y2,z2,   x3,y3,z3,   // shooting:
        65,5,-20,   10,0,40,    0,0,0,      // s0: arms/thighs/-
        -50,-30,50, 0,0,-70,    0,0,0,      // s0: forearm/legs/-
        0,50,0,     -10,0,35,   0,0,0,      // s0: hands/feet/-
                                            // takes weapon:
        160,135,-20,10,0,5,     0,0,0,      // s1: arms/thighs/-
        10,-60,40,  0,0,-10,    0,0,0,      // s1: forearm/legs/-
        0,-5,-25,   -10,5,5,    0,0,0,      // s1: hands/feet/-
                                            // carries earth:
        25,40,-40,  10,0,60,    0,0,0,      // s2: arms/thighs/-
        0,-45,0,    0,0,-120,   0,0,0,      // s2: forearm/legs/-
        0,15,5,     -10,0,70,   0,0,0,      // s2: hands/feet/-
                                            // carries in front:
        25,20,5,    10,0,55,    0,0,0,      // s3: arms/thighs/-
        -15,-30,10, 0,0,-110,   0,0,0,      // s3: forearm/legs/-
        0,0,0,      -10,0,65,   0,0,0,      // s3: hands/feet/-
                                            // carries vertically:
        -30,15,-5,  10,0,15,    0,0,0,      // s4: arms/thighs/-
        0,-15,15,   0,0,-30,    0,0,0,      // s4: forearm/legs/-
        35,0,-15,   -10,0,25,   0,0,0,      // s4: hands/feet/-
                                            // rises:
        15,50,-50,  10,0,5,     0,0,0,      // s5: arms/thighs/-
        45,-70,10,  0,0,-10,    0,0,0,      // s5: forearm/legs/-
        -10,25,0,   -10,5,5,    0,0,0,      // s5: hands/feet/-
                                            // wins:
        90,90,-30,  20,0,5,     0,0,0,      // s6: arms/thighs/-
        0,-90,0,    -10,0,-10,  0,0,0,      // s6: forearm/legs/-
        0,25,0,     -10,5,5,    0,0,0,      // s6: hands/feet/-
                                            // lose:
        -70,45,35,  10,0,40,    0,0,0,      // s7: arms/thighs/-
        15,-95,-5,  0,0,-70,    0,0,0,      // s7: forearm/legs/-
        0,0,0,      -10,0,35,   0,0,0,      // s7: hands/feet/-
                                            // shooting death (falls):
        90,90,-50,  10,0,5,     0,0,0,      // s8: arms/thighs/-
        0,-20,0,    0,0,-10,    0,0,0,      // s8: forearm/legs/-
        0,0,0,      -10,5,5,    0,0,0,      // s8: hands/feet/-
                                            // shooting death (knees):
        110,105,-5, 10,0,25,    0,0,0,      // s9: arms/thighs/-
        0,-40,20,   0,0,-120,   0,0,0,      // s9: forearm/legs/-
        0,0,0,      -10,5,5,    0,0,0,      // s9: hands/feet/-
                                            // shooting death (knees):
        110,120,-25,    10,0,25,    0,0,0,      // s10: arms/thighs/-
        0,-40,20,   0,0,-120,   0,0,0,      // s10: forearm/legs/-
        0,0,0,      -10,5,5,    0,0,0,      // s10: hands/feet/-
                                            // shooting death (face down):
        110,100,-25,    25,0,10,    0,0,0,      // s11: arms/thighs/-
        0,-40,20,   0,0,-25,    0,0,0,      // s11: forearm/legs/-
        0,0,0,      -10,5,5,    0,0,0,      // s11: hands/feet/-
                                            // shooting death (face down):
        110,100,-25,    25,0,10,    0,0,0,      // s12: arms/thighs/-
        0,-40,20,   0,0,-25,    0,0,0,      // s12: forearm/legs/-
        0,0,0,      -10,5,5,    0,0,0,      // s12: hands/feet/-
                                            // drowned:
        110,100,-25,    25,0,10,    0,0,0,      // s13: arms/thighs/-
        0,-40,20,   0,0,-25,    0,0,0,      // s13: forearm/legs/-
        0,0,0,      -10,5,5,    0,0,0,      // s13: hands/feet/-
                                            // puts / removes flag:
        85,45,-50,  10,0,60,    0,0,0,      // s14: arms/thighs/-
        -60,15,65,  0,0,-105,   0,0,0,      // s14: forearm/legs/-
        0,10,0,     -10,0,60,   0,0,0,      // s14: hands/feet/-
                                            // reads SatCom:
        70,30,-20,  10,0,5,     0,0,0,      // s15: arms/thighs/-
        115,-65,60, 0,0,-10,    0,0,0,      // s15: forearm/legs/-
        0,20,0,     -10,5,5,    0,0,0,      // s15: hands/feet/-
    };

    character = m_object->GetCharacter();
    character->wheelFront = 4.0f;
    character->wheelBack  = 4.0f;
    character->wheelLeft  = 4.0f;
    character->wheelRight = 4.0f;
    character->height     = 3.5f;

    if ( type == OBJECT_HUMAN )
    {
        m_physics->SetLinMotionX(MO_ADVSPEED, 50.0f);
        m_physics->SetLinMotionX(MO_RECSPEED, 35.0f);
        m_physics->SetLinMotionX(MO_ADVACCEL, 20.0f);
        m_physics->SetLinMotionX(MO_RECACCEL, 20.0f);
        m_physics->SetLinMotionX(MO_STOACCEL, 20.0f);
        m_physics->SetLinMotionX(MO_TERSLIDE,  5.0f);
        m_physics->SetLinMotionZ(MO_TERSLIDE,  5.0f);
        m_physics->SetLinMotionX(MO_TERFORCE, 70.0f);
        m_physics->SetLinMotionZ(MO_TERFORCE, 40.0f);
        m_physics->SetLinMotionZ(MO_MOTACCEL, 40.0f);
        m_physics->SetLinMotionY(MO_ADVSPEED, 60.0f);
        m_physics->SetLinMotionY(MO_RECSPEED, 60.0f);
        m_physics->SetLinMotionY(MO_ADVACCEL, 20.0f);
        m_physics->SetLinMotionY(MO_RECACCEL, 50.0f);
        m_physics->SetLinMotionY(MO_STOACCEL, 50.0f);

        m_physics->SetCirMotionY(MO_ADVSPEED,  0.8f*Math::PI);
        m_physics->SetCirMotionY(MO_RECSPEED,  0.8f*Math::PI);
        m_physics->SetCirMotionY(MO_ADVACCEL,  6.0f);
        m_physics->SetCirMotionY(MO_RECACCEL,  6.0f);
        m_physics->SetCirMotionY(MO_STOACCEL,  4.0f);
    }
    else
    {
        m_physics->SetLinMotionX(MO_ADVSPEED, 40.0f);
        m_physics->SetLinMotionX(MO_RECSPEED, 15.0f);
        m_physics->SetLinMotionX(MO_ADVACCEL,  8.0f);
        m_physics->SetLinMotionX(MO_RECACCEL,  8.0f);
        m_physics->SetLinMotionX(MO_STOACCEL,  8.0f);
        m_physics->SetLinMotionX(MO_TERSLIDE,  5.0f);
        m_physics->SetLinMotionZ(MO_TERSLIDE,  5.0f);
        m_physics->SetLinMotionX(MO_TERFORCE, 50.0f);
        m_physics->SetLinMotionZ(MO_TERFORCE, 50.0f);
        m_physics->SetLinMotionZ(MO_MOTACCEL, 40.0f);
        m_physics->SetLinMotionY(MO_ADVSPEED, 60.0f);
        m_physics->SetLinMotionY(MO_RECSPEED, 60.0f);
        m_physics->SetLinMotionY(MO_ADVACCEL, 20.0f);
        m_physics->SetLinMotionY(MO_RECACCEL, 50.0f);
        m_physics->SetLinMotionY(MO_STOACCEL, 50.0f);

        m_physics->SetCirMotionY(MO_ADVSPEED,  0.6f*Math::PI);
        m_physics->SetCirMotionY(MO_RECSPEED,  0.6f*Math::PI);
        m_physics->SetCirMotionY(MO_ADVACCEL,  4.0f);
        m_physics->SetCirMotionY(MO_RECACCEL,  4.0f);
        m_physics->SetCirMotionY(MO_STOACCEL,  3.0f);
    }

    for ( i=0 ; i<3*3*3*3 ; i++ )
    {
        m_armAngles[3*3*3*3*MH_MARCH+i] = member_march[i];
    }
    for ( i=0 ; i<3*3*3*3 ; i++ )
    {
        m_armAngles[3*3*3*3*MH_MARCHTAKE+i] = member_march_take[i];
    }
    for ( i=0 ; i<3*3*3*3 ; i++ )
    {
        m_armAngles[3*3*3*3*MH_TURN+i] = member_turn[i];
    }
    for ( i=0 ; i<3*3*3*3 ; i++ )
    {
        m_armAngles[3*3*3*3*MH_STOP+i] = member_stop[i];
    }
    for ( i=0 ; i<3*3*3*3 ; i++ )
    {
        m_armAngles[3*3*3*3*MH_FLY+i] = member_fly[i];
    }
    for ( i=0 ; i<3*3*3*3 ; i++ )
    {
        m_armAngles[3*3*3*3*MH_SWIM+i] = member_swim[i];
    }
    for ( i=0 ; i<3*3*3*16 ; i++ )
    {
        m_armAngles[3*3*3*3*MH_SPEC+i] = member_spec[i];
    }
}


// Management of an event.

bool CMotionHuman::EventProcess(const Event &event)
{
    CMotion::EventProcess(event);

    if ( event.type == EVENT_FRAME )
    {
        return EventFrame(event);
    }

    return true;
}

// Management of an event.

bool CMotionHuman::EventFrame(const Event &event)
{
    glm::vec3    dir, actual, pos, speed, pf;
    glm::vec2    dim, p2;
    float       s, a, prog, rTime[2], lTime[2], time, rot, hr, hl;
    float       al, ar, af;
    float       tSt[9], tNd[9];
    float       aa, bb, deadFactor = 0.0f, level;
    int         i, ii, st, nd, action, legAction, armAction;
    bool        bOnBoard, bSwim;

    if ( m_engine->GetPause() )
    {
        if ( m_actionType == MHS_SATCOM )
        {
            m_progress += event.rTime*m_actionTime;
        }
        else
        {
            return true;
        }
    }

    bOnBoard = false;
    if ( m_object->GetSelect() &&
         m_camera->GetType() == Gfx::CAM_TYPE_ONBOARD )
    {
        bOnBoard = true;
    }

    if ( m_bDisplayPerso && m_main->GetGamerOnlyHead() )
    {
        m_time += event.rTime;
        m_object->SetLinVibration(glm::vec3(0.0f, -0.55f, 0.0f));
        m_object->SetCirVibration(glm::vec3(0.0f, m_main->GetPersoAngle(), 0.0f));
        return true;
    }
    if ( m_bDisplayPerso )
    {
        m_object->SetCirVibration(glm::vec3(0.0f, m_main->GetPersoAngle()+0.2f, 0.0f));
    }

    if ( m_glassesRank != -1 )
    {
        if ( m_camera->GetType() == Gfx::CAM_TYPE_ONBOARD )
        {
            m_engine->SetObjectDrawWorld(m_glassesRank, false);
        }
        else
        {
            m_engine->SetObjectDrawWorld(m_glassesRank, true);
        }
    }

    bSwim = m_physics->GetSwim();

    rot = m_physics->GetCirMotionY(MO_MOTSPEED);
    a = m_physics->GetLinMotionX(MO_REASPEED);
    s = m_physics->GetLinMotionX(MO_MOTSPEED)*0.2f;
    if ( fabs(a) > fabs(s) )  s = a;  // the highest value
    a = m_physics->GetLinMotionX(MO_TERSPEED);
    if ( a < 0.0f )  // rises?
    {
        a += m_physics->GetLinMotionX(MO_TERSLIDE);
        if ( a < 0.0f )  s -= a;
    }
    if ( a > 0.0f )  // falls?
    {
        a -= m_physics->GetLinMotionX(MO_TERSLIDE);
        if ( a > 0.0f )  s -= a;
    }
    s *= 2.0f;
    a = fabs(rot*12.0f);

    if ( !m_physics->GetLand() && !bSwim )  // in flight?
    {
        s = 0.0f;
    }

    if (IsObjectCarryingCargo(m_object))  // carries something?
    {
        s *= 1.3f;
    }

    m_time += event.rTime;
    m_armTimeAbs += event.rTime;
    m_armTimeAction += event.rTime;
    m_armMember += s*event.rTime*0.05f;

    // Fatigue management when short.
    if ( m_physics->GetLand() && s != 0.0f )  // on the ground?
    {
        m_tired += event.rTime*0.1f;
        if ( m_tired > 1.0f )
        {
            m_tired = 1.0f;
            if ( m_lastSoundHhh > 3.0f )  m_lastSoundHhh = 0.5f;
        }
    }
    else
    {
        m_tired -= event.rTime*0.2f;
        if ( m_tired < 0.0f )  m_tired = 0.0f;
    }

    if ( bSwim )  // swims?
    {
        s += fabs(m_physics->GetLinMotionY(MO_REASPEED)*2.0f);
        a *= 2.0f;
        m_armTimeSwim += Math::Min(Math::Max(s,a,3.0f),15.0f)*event.rTime*0.05f;
    }

    prog = 0.0f;

    if ( m_physics->GetLand() )  // on the ground?
    {
        if ( s == 0.0f && a == 0.0f )
        {
            action = MH_STOP;  // stop
            rTime[0] = rTime[1] = m_armTimeAbs*0.21f;
            lTime[0] = lTime[1] = m_armTimeAbs*0.25f;
            m_armMember = START_TIME;
        }
        else
        {
            if ( s == 0.0f )
            {
                action = MH_TURN;  // turn
                rTime[0] = rTime[1] = m_armTimeAbs;
                lTime[0] = lTime[1] = m_armTimeAbs+0.5f;
                if ( rot < 0.0f )
                {
                    rTime[1] = 1000000.0f-rTime[1];
                }
                else
                {
                    lTime[1] = 1000000.0f-lTime[1];
                }
                m_armMember = START_TIME;
            }
            else
            {
                action = MH_MARCH;  // walking
                if (IsObjectCarryingCargo(m_object))  action = MH_MARCHTAKE;  // take walking
                rTime[0] = rTime[1] = m_armMember;
                lTime[0] = lTime[1] = m_armMember+0.5f;
            }
        }
        if ( bSwim )
        {
            rTime[0] *= 0.6f;
            rTime[1] *= 0.6f;
            lTime[0] = rTime[0]+0.5f;
            lTime[1] = rTime[1]+0.5f;
        }
    }
    else
    {
        if ( bSwim )
        {
            action = MH_SWIM;  // swim
            rTime[0] = rTime[1] = m_armTimeSwim;
            lTime[0] = lTime[1] = m_armTimeSwim;
        }
        else
        {
            action = MH_FLY;  // fly
            rTime[0] = rTime[1] = m_armTimeAbs*0.30f;
            lTime[0] = lTime[1] = m_armTimeAbs*0.31f;
            m_armMember = START_TIME;
        }
    }

    if ( action != m_armLastAction )
    {
        m_armLastAction = action;
        m_armTimeAction = 0.0f;
    }

    armAction = action;
    legAction = action;

    if (IsObjectCarryingCargo(m_object))  // carries something?
    {
        armAction = MH_MARCHTAKE;  // take walking
    }

    if ( m_physics->GetLand() )  // on the ground?
    {
        a = m_object->GetRotationY();
        pos = m_object->GetPosition();
        m_terrain->AdjustToFloor(pos);

        pf.x = pos.x+cosf(a+Math::PI*1.5f)*0.7f;
        pf.y = pos.y;
        pf.z = pos.z-sinf(a+Math::PI*1.5f)*0.7f;
        m_terrain->AdjustToFloor(pf);
        al = atanf((pf.y-pos.y)/0.7f);  // angle for left leg

        pf = pos;
        pf.x = pos.x+cosf(a+Math::PI*0.5f)*0.7f;
        pf.y = pos.y;
        pf.z = pos.z-sinf(a+Math::PI*0.5f)*0.7f;
        m_terrain->AdjustToFloor(pf);
        ar = atanf((pf.y-pos.y)/0.7f);  // angle to right leg

        pf.x = pos.x+cosf(a+Math::PI)*0.3f;
        pf.y = pos.y;
        pf.z = pos.z-sinf(a+Math::PI)*0.3f;
        m_terrain->AdjustToFloor(pf);
        af = atanf((pf.y-pos.y)/0.3f);  // angle for feet
    }
    else
    {
        al = 0.0f;
        ar = 0.0f;
        af = 0.0f;
    }

    for ( i=0 ; i<4 ; i++ )  // 4 members
    {
        if ( m_bArmStop )  // focus?
        {
            st = ADJUST_ACTION + (i%2)*3;
            nd = st;
            time = 100.0f;
            m_armTimeAction = 0.0f;
        }
        else if ( m_actionType != -1 )  // special action in progress?
        {
            st = 3*3*3*3*MH_SPEC + 3*3*3*m_actionType + (i%2)*3;
            nd = st;
            time = event.rTime*m_actionTime;
            m_armTimeAction = 0.0f;
        }
        else
        {
            if ( i < 2 )  prog = Math::Mod(rTime[i%2], 1.0f);
            else          prog = Math::Mod(lTime[i%2], 1.0f);
            if ( prog < 0.25f )  // t0..t1 ?
            {
                prog = prog/0.25f;  // 0..1
                st = 0;  // index start
                nd = 1;  // index end
            }
            else if ( prog < 0.75f )  // t1..t2 ?
            {
                prog = (prog-0.25f)/0.50f;  // 0..1
                st = 1;  // index start
                nd = 2;  // index end
            }
            else    // t2..t0 ?
            {
                prog = (prog-0.75f)/0.25f;  // 0..1
                st = 2;  // index start
                nd = 0;  // index end
            }
            if ( i%2 == 0 )  // arm?
            {
                st = 3*3*3*3*armAction + st*3*3*3 + (i%2)*3;
                nd = 3*3*3*3*armAction + nd*3*3*3 + (i%2)*3;
            }
            else    // leg?
            {
                st = 3*3*3*3*legAction + st*3*3*3 + (i%2)*3;
                nd = 3*3*3*3*legAction + nd*3*3*3 + (i%2)*3;
            }

            // Less soft ...
            time = event.rTime*(5.0f+Math::Min(m_armTimeAction*50.0f, 100.0f));
            if ( bSwim )  time *= 0.25f;
        }

        tSt[0] = m_armAngles[st+ 0];  // x
        tSt[1] = m_armAngles[st+ 1];  // y
        tSt[2] = m_armAngles[st+ 2];  // z
        tSt[3] = m_armAngles[st+ 9];  // x
        tSt[4] = m_armAngles[st+10];  // y
        tSt[5] = m_armAngles[st+11];  // z
        tSt[6] = m_armAngles[st+18];  // x
        tSt[7] = m_armAngles[st+19];  // y
        tSt[8] = m_armAngles[st+20];  // z

        tNd[0] = m_armAngles[nd+ 0];  // x
        tNd[1] = m_armAngles[nd+ 1];  // y
        tNd[2] = m_armAngles[nd+ 2];  // z
        tNd[3] = m_armAngles[nd+ 9];  // x
        tNd[4] = m_armAngles[nd+10];  // y
        tNd[5] = m_armAngles[nd+11];  // z
        tNd[6] = m_armAngles[nd+18];  // x
        tNd[7] = m_armAngles[nd+19];  // y
        tNd[8] = m_armAngles[nd+20];  // z

        aa = 0.5f;
        if ( i%2 == 0 )  // arm?
        {
            if (! IsObjectCarryingCargo(m_object))
            {
                aa = 2.0f;  // moves a lot
            }
            else
            {
                aa = 0.0f;  // immobile
            }
        }

        if ( i < 2 )  // left?
        {
            bb = sinf(m_time*1.1f)*aa;  tSt[0] += bb;  tNd[0] += bb;
            bb = sinf(m_time*1.0f)*aa;  tSt[1] += bb;  tNd[1] += bb;
            bb = sinf(m_time*1.2f)*aa;  tSt[2] += bb;  tNd[2] += bb;
            bb = sinf(m_time*2.5f)*aa;  tSt[3] += bb;  tNd[3] += bb;
            bb = sinf(m_time*2.0f)*aa;  tSt[4] += bb;  tNd[4] += bb;
            bb = sinf(m_time*3.8f)*aa;  tSt[5] += bb;  tNd[5] += bb;
            bb = sinf(m_time*3.0f)*aa;  tSt[6] += bb;  tNd[6] += bb;
            bb = sinf(m_time*2.3f)*aa;  tSt[7] += bb;  tNd[7] += bb;
            bb = sinf(m_time*4.0f)*aa;  tSt[8] += bb;  tNd[8] += bb;
        }
        else    // right?
        {
            bb = sinf(m_time*0.9f)*aa;  tSt[0] += bb;  tNd[0] += bb;
            bb = sinf(m_time*1.2f)*aa;  tSt[1] += bb;  tNd[1] += bb;
            bb = sinf(m_time*1.4f)*aa;  tSt[2] += bb;  tNd[2] += bb;
            bb = sinf(m_time*2.9f)*aa;  tSt[3] += bb;  tNd[3] += bb;
            bb = sinf(m_time*1.4f)*aa;  tSt[4] += bb;  tNd[4] += bb;
            bb = sinf(m_time*3.1f)*aa;  tSt[5] += bb;  tNd[5] += bb;
            bb = sinf(m_time*3.7f)*aa;  tSt[6] += bb;  tNd[6] += bb;
            bb = sinf(m_time*2.0f)*aa;  tSt[7] += bb;  tNd[7] += bb;
            bb = sinf(m_time*3.1f)*aa;  tSt[8] += bb;  tNd[8] += bb;
        }

        if ( i%2 == 1           &&  // leg?
             m_actionType == -1 )   // no special action?
        {
            if ( i == 1 )  // right leg?
            {
                ii = 5;
                a = ar*0.25f;
            }
            else
            {
                ii = 11;
                a = al*0.25f;
            }
            if ( a < -0.2f )  a = -0.2f;
            if ( a >  0.2f )  a =  0.2f;

            pos = m_object->GetPartPosition(ii+0);
            pos.y = 0.0f+a;
            m_object->SetPartPosition(ii+0, pos);  // lengthens / shortcuts thigh

            pos = m_object->GetPartPosition(ii+1);
            pos.y = -1.5f+a;
            m_object->SetPartPosition(ii+1, pos);  // lengthens / shortcuts leg

            pos = m_object->GetPartPosition(ii+2);
            pos.y = -1.5f+a;
            m_object->SetPartPosition(ii+2, pos);  // lengthens / shortcuts foot

            if ( i == 1 )  // right leg?
            {
                aa = (ar*180.0f/Math::PI*0.5f);
            }
            else    // left leg?
            {
                aa = (al*180.0f/Math::PI*0.5f);
            }
            tSt[6] += aa;
            tNd[6] += aa;  // increases the angle X of the foot

            if ( i == 1 )  // right leg?
            {
                aa = (ar*180.0f/Math::PI);
            }
            else    // left leg?
            {
                aa = (al*180.0f/Math::PI);
            }
            if ( aa <  0.0f )  aa =  0.0f;
            if ( aa > 30.0f )  aa = 30.0f;

            tSt[2] += aa;
            tNd[2] += aa;    // increases the angle Z of the thigh
            tSt[5] -= aa*2;
            tNd[5] -= aa*2;  // increases the angle Z of the leg
            tSt[8] += aa;
            tNd[8] += aa;    // increases the angle Z of the foot

            aa = (af*180.0f/Math::PI)*0.7f;
            if ( aa < -30.0f )  aa = -30.0f;
            if ( aa >  30.0f )  aa =  30.0f;

            tSt[8] -= aa;
            tNd[8] -= aa;    // increases the angle Z of the foot
        }

        if ( m_actionType == MHS_DEADw )   // drowned?
        {
            if ( m_progress < 0.5f )
            {
                deadFactor = m_progress/0.5f;
            }
            else
            {
                deadFactor = 1.0f-(m_progress-0.5f)/0.5f;
            }
            if ( deadFactor < 0.0f )  deadFactor = 0.0f;
            if ( deadFactor > 1.0f )  deadFactor = 1.0f;

            for ( ii=0 ; ii<9 ; ii++ )
            {
                tSt[ii] += Math::Rand()*20.0f*deadFactor;
                tNd[ii] = tSt[ii];
            }
            time = 100.0f;
        }

        if ( i < 2 )  // right member (0..1) ?
        {
            m_object->SetPartRotationX(2+3*i+0, Math::Smooth(m_object->GetPartRotationX(2+3*i+0), Math::PropAngle(tSt[0], tNd[0], prog), time));
            m_object->SetPartRotationY(2+3*i+0, Math::Smooth(m_object->GetPartRotationY(2+3*i+0), Math::PropAngle(tSt[1], tNd[1], prog), time));
            m_object->SetPartRotationZ(2+3*i+0, Math::Smooth(m_object->GetPartRotationZ(2+3*i+0), Math::PropAngle(tSt[2], tNd[2], prog), time));
            m_object->SetPartRotationX(2+3*i+1, Math::Smooth(m_object->GetPartRotationX(2+3*i+1), Math::PropAngle(tSt[3], tNd[3], prog), time));
            m_object->SetPartRotationY(2+3*i+1, Math::Smooth(m_object->GetPartRotationY(2+3*i+1), Math::PropAngle(tSt[4], tNd[4], prog), time));
            m_object->SetPartRotationZ(2+3*i+1, Math::Smooth(m_object->GetPartRotationZ(2+3*i+1), Math::PropAngle(tSt[5], tNd[5], prog), time));
            m_object->SetPartRotationX(2+3*i+2, Math::Smooth(m_object->GetPartRotationX(2+3*i+2), Math::PropAngle(tSt[6], tNd[6], prog), time));
            m_object->SetPartRotationY(2+3*i+2, Math::Smooth(m_object->GetPartRotationY(2+3*i+2), Math::PropAngle(tSt[7], tNd[7], prog), time));
            m_object->SetPartRotationZ(2+3*i+2, Math::Smooth(m_object->GetPartRotationZ(2+3*i+2), Math::PropAngle(tSt[8], tNd[8], prog), time));
        }
        else    // left member (2..3) ?
        {
            m_object->SetPartRotationX(2+3*i+0, Math::Smooth(m_object->GetPartRotationX(2+3*i+0), Math::PropAngle(-tSt[0], -tNd[0], prog), time));
            m_object->SetPartRotationY(2+3*i+0, Math::Smooth(m_object->GetPartRotationY(2+3*i+0), Math::PropAngle(-tSt[1], -tNd[1], prog), time));
            m_object->SetPartRotationZ(2+3*i+0, Math::Smooth(m_object->GetPartRotationZ(2+3*i+0), Math::PropAngle( tSt[2],  tNd[2], prog), time));
            m_object->SetPartRotationX(2+3*i+1, Math::Smooth(m_object->GetPartRotationX(2+3*i+1), Math::PropAngle(-tSt[3], -tNd[3], prog), time));
            m_object->SetPartRotationY(2+3*i+1, Math::Smooth(m_object->GetPartRotationY(2+3*i+1), Math::PropAngle(-tSt[4], -tNd[4], prog), time));
            m_object->SetPartRotationZ(2+3*i+1, Math::Smooth(m_object->GetPartRotationZ(2+3*i+1), Math::PropAngle( tSt[5],  tNd[5], prog), time));
            m_object->SetPartRotationX(2+3*i+2, Math::Smooth(m_object->GetPartRotationX(2+3*i+2), Math::PropAngle(-tSt[6], -tNd[6], prog), time));
            m_object->SetPartRotationY(2+3*i+2, Math::Smooth(m_object->GetPartRotationY(2+3*i+2), Math::PropAngle(-tSt[7], -tNd[7], prog), time));
            m_object->SetPartRotationZ(2+3*i+2, Math::Smooth(m_object->GetPartRotationZ(2+3*i+2), Math::PropAngle( tSt[8],  tNd[8], prog), time));
        }
    }

    // calculates the height lowering as a function
    // of the position of the legs.
    hr = 1.5f*(1.0f-cosf(m_object->GetPartRotationZ(5))) +
         1.5f*(1.0f-cosf(m_object->GetPartRotationZ(5)+m_object->GetPartRotationZ(6)));
    a = 1.0f*sinf(m_object->GetPartRotationZ(5)+m_object->GetPartRotationZ(6)+m_object->GetPartRotationZ(7));
    if ( a < 0.0f )  hr += a;

    hl = 1.5f*(1.0f-cosf(m_object->GetPartRotationZ(11))) +
         1.5f*(1.0f-cosf(m_object->GetPartRotationZ(11)+m_object->GetPartRotationZ(12)));
    a = 1.0f*sinf(m_object->GetPartRotationZ(11)+m_object->GetPartRotationZ(12)+m_object->GetPartRotationZ(13));
    if ( a < 0.0f )  hl += a;

    hr = Math::Min(hr, hl);

    if ( m_actionType == MHS_FIRE )  // shooting?
    {
        time = event.rTime*m_actionTime;

        dir.x = (Math::Rand()-0.5f)/8.0f;
        dir.z = (Math::Rand()-0.5f)/8.0f;
        dir.y = -0.5f;  // slightly lower
        actual = m_object->GetLinVibration();
        dir.x = Math::Smooth(actual.x, dir.x, time);
//?     dir.y = Math::Smooth(actual.y, dir.y, time);
        dir.y = -hr;
        dir.z = Math::Smooth(actual.z, dir.z, time);
        m_object->SetLinVibration(dir);

        dir.x = 0.0f;
        dir.y = (Math::Rand()-0.5f)/3.0f;
        dir.z = -0.1f;  // slightly leaning forward
        actual = m_object->GetTilt();
        dir.x = Math::Smooth(actual.x, dir.x, time);
        dir.y = Math::Smooth(actual.y, dir.y, time);
        dir.z = Math::Smooth(actual.z, dir.z, time);
        m_object->SetTilt(dir);
    }
    else if ( m_actionType == MHS_TAKE      ||  // carrying?
              m_actionType == MHS_TAKEOTHER )   // flag?
    {
        time = event.rTime*m_actionTime*2.0f;

        dir.x = 0.0f;
        dir.z = 0.0f;
        dir.y = -1.5f;  // slightly lower
        actual = m_object->GetLinVibration();
        dir.x = Math::Smooth(actual.x, dir.x, time);
//?     dir.y = Math::Smooth(actual.y, dir.y, time);
        dir.y = -hr;
        dir.z = Math::Smooth(actual.z, dir.z, time);
        m_object->SetLinVibration(dir);

        dir.x = 0.0f;
        dir.y = 0.0f;
        dir.z = -0.2f;
        actual = m_object->GetTilt();
        dir.x = Math::Smooth(actual.x, dir.x, time);
        dir.y = Math::Smooth(actual.y, dir.y, time);
        dir.z = Math::Smooth(actual.z, dir.z, time);
        m_object->SetTilt(dir);
    }
    else if ( m_actionType == MHS_TAKEHIGH )   // carrying?
    {
        time = event.rTime*m_actionTime*2.0f;

        dir.x = 0.4f;  // slightly forward
        dir.z = 0.0f;
        dir.y = 0.0f;
        actual = m_object->GetLinVibration();
        dir.x = Math::Smooth(actual.x, dir.x, time);
//?     dir.y = Math::Smooth(actual.y, dir.y, time);
        dir.y = -hr;
        dir.z = Math::Smooth(actual.z, dir.z, time);
        m_object->SetLinVibration(dir);

        dir.x = 0.0f;
        dir.y = 0.0f;
        dir.z = -0.2f;
        actual = m_object->GetTilt();
        dir.x = Math::Smooth(actual.x, dir.x, time);
        dir.y = Math::Smooth(actual.y, dir.y, time);
        dir.z = Math::Smooth(actual.z, dir.z, time);
        m_object->SetTilt(dir);
    }
    else if ( m_actionType == MHS_FLAG )   // flag?
    {
        time = event.rTime*m_actionTime*2.0f;

        dir.x = 0.0f;
        dir.z = 0.0f;
        dir.y = -2.0f;  // slightly lower
        actual = m_object->GetLinVibration();
        dir.x = Math::Smooth(actual.x, dir.x, time);
//?     dir.y = Math::Smooth(actual.y, dir.y, time);
        dir.y = -hr;
        dir.z = Math::Smooth(actual.z, dir.z, time);
        m_object->SetLinVibration(dir);

        dir.x = 0.0f;
        dir.y = 0.0f;
        dir.z = -0.4f;
        actual = m_object->GetTilt();
        dir.x = Math::Smooth(actual.x, dir.x, time);
        dir.y = Math::Smooth(actual.y, dir.y, time);
        dir.z = Math::Smooth(actual.z, dir.z, time);
        m_object->SetTilt(dir);
    }
    else if ( m_actionType == MHS_DEADg )   // shooting death (falls)?
    {
        if ( m_physics->GetLand() )  // on the ground?
        {
            SetAction(MHS_DEADg1, 0.5f);  // knees
        }
    }
    else if ( m_actionType == MHS_DEADg1 )   // shooting death (knees)?
    {
        prog = m_progress;
        if ( prog >= 1.0f )
        {
            prog = 1.0f;

            for ( i=0 ; i<10 ; i++ )
            {
                pos = m_object->GetPosition();
                pos.x += (Math::Rand()-0.5f)*4.0f;
                pos.z += (Math::Rand()-0.5f)*4.0f;
                m_terrain->AdjustToFloor(pos);
                speed = glm::vec3(0.0f, 0.0f, 0.0f);
                dim.x = 1.2f+Math::Rand()*1.2f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTICRASH, 2.0f, 0.0f, 0.0f);
            }
            m_sound->Play(SOUND_BOUMv, m_object->GetPosition());

            SetAction(MHS_DEADg2, 1.0f);  // expects knees
        }

        time = 100.0f;

        dir.x = 0.0f;
        dir.z = 0.0f;
        dir.y = -1.5f*prog;
        actual = m_object->GetLinVibration();
        dir.x = Math::Smooth(actual.x, dir.x, time);
        dir.y = Math::Smooth(actual.y, dir.y, time);
        dir.z = Math::Smooth(actual.z, dir.z, time);
        m_object->SetLinVibration(dir);

        dir.x = 0.0f;
        dir.y = 0.0f;
        dir.z = -(20.0f*Math::PI/180.0f)*prog;
        actual = m_object->GetTilt();
        dir.x = Math::Smooth(actual.x, dir.x, time);
        dir.y = Math::Smooth(actual.y, dir.y, time);
        dir.z = Math::Smooth(actual.z, dir.z, time);
        m_object->SetTilt(dir);
    }
    else if ( m_actionType == MHS_DEADg2 )   // shooting death (knees)?
    {
        if ( m_progress >= 1.0f )
        {
            SetAction(MHS_DEADg3, 1.0f);  // face down
        }

        time = 100.0f;

        dir.x = 0.0f;
        dir.z = 0.0f;
        dir.y = -1.5f;
        actual = m_object->GetLinVibration();
        dir.x = Math::Smooth(actual.x, dir.x, time);
        dir.y = Math::Smooth(actual.y, dir.y, time);
        dir.z = Math::Smooth(actual.z, dir.z, time);
        m_object->SetLinVibration(dir);

        dir.x = 0.0f;
        dir.y = 0.0f;
        dir.z = -(20.0f*Math::PI/180.0f);
        actual = m_object->GetTilt();
        dir.x = Math::Smooth(actual.x, dir.x, time);
        dir.y = Math::Smooth(actual.y, dir.y, time);
        dir.z = Math::Smooth(actual.z, dir.z, time);
        m_object->SetTilt(dir);
    }
    else if ( m_actionType == MHS_DEADg3 )   // shooting death (face down)?
    {
        prog = m_progress;
        if ( prog >= 1.0f )
        {
            prog = 1.0f;

            for ( i=0 ; i<20 ; i++ )
            {
                pos = m_object->GetPosition();
                pos.x += (Math::Rand()-0.5f)*8.0f;
                pos.z += (Math::Rand()-0.5f)*8.0f;
                m_terrain->AdjustToFloor(pos);
                speed = glm::vec3(0.0f, 0.0f, 0.0f);
                dim.x = 2.0f+Math::Rand()*1.5f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTICRASH, 2.0f, 0.0f, 0.0f);
            }
            m_sound->Play(SOUND_BOUMv, m_object->GetPosition());

            SetAction(MHS_DEADg4, 3.0f);  // expects face down
        }

        time = 100.0f;
        prog = powf(prog, 3.0f);

        dir.y = -(1.5f+1.5f*prog);
        dir.x = 0.0f;
        dir.z = 0.0f;
        actual = m_object->GetLinVibration();
        dir.x = Math::Smooth(actual.x, dir.x, time);
        dir.y = Math::Smooth(actual.y, dir.y, time);
        dir.z = Math::Smooth(actual.z, dir.z, time);
        m_object->SetLinVibration(dir);

        dir.z = -((20.0f*Math::PI/180.0f)+(70.0f*Math::PI/180.0f)*prog);
        dir.x = 0.0f;
        dir.y = 0.0f;
        actual = m_object->GetTilt();
        dir.x = Math::Smooth(actual.x, dir.x, time);
        dir.y = Math::Smooth(actual.y, dir.y, time);
        dir.z = Math::Smooth(actual.z, dir.z, time);
        m_object->SetTilt(dir);
    }
    else if ( m_actionType == MHS_DEADg4 )   // shooting death (face down)?
    {
        if ( m_progress >= 1.0f )
        {
            CObjectManager::GetInstancePointer()->DeleteObject(m_object);
            return false;
        }

        time = 100.0f;

        dir.y = -(1.5f+1.5f);
        dir.x = 0.0f;
        dir.z = 0.0f;
        actual = m_object->GetLinVibration();
        dir.x = Math::Smooth(actual.x, dir.x, time);
        dir.y = Math::Smooth(actual.y, dir.y, time);
        dir.z = Math::Smooth(actual.z, dir.z, time);
        m_object->SetLinVibration(dir);

        dir.z = -((20.0f*Math::PI/180.0f)+(70.0f*Math::PI/180.0f));
        dir.x = 0.0f;
        dir.y = 0.0f;
        actual = m_object->GetTilt();
        dir.x = Math::Smooth(actual.x, dir.x, time);
        dir.y = Math::Smooth(actual.y, dir.y, time);
        dir.z = Math::Smooth(actual.z, dir.z, time);
        m_object->SetTilt(dir);
    }
    else if ( m_actionType == MHS_DEADw )   // drowned?
    {
        pos = m_object->GetPosition();
        level = m_water->GetLevel()-0.5f;
        if ( pos.y < level )
        {
            pos.y += 4.0f*event.rTime;  // back to the surface
            if ( pos.y > level )  pos.y = level;
            m_object->SetPosition(pos);
        }
        if ( pos.y > level )
        {
            pos.y -= 10.0f*event.rTime;  // down quickly
            if ( pos.y < level )  pos.y = level;
            m_object->SetPosition(pos);
        }

        prog = m_progress;
        if ( prog >= 1.0f )
        {
            prog = 1.0f;
            if ( pos.y >= level )
            {
                CObjectManager::GetInstancePointer()->DeleteObject(m_object);
                return false;
            }
        }

        prog *= 2.0f;
        if ( prog > 1.0f )  prog = 1.0f;

        time = 100.0f;

        dir.z = -(90.0f*Math::PI/180.0f)*prog;
        dir.x = Math::Rand()*0.3f*deadFactor;
        dir.y = Math::Rand()*0.3f*deadFactor;
        actual = m_object->GetTilt();
        dir.x = Math::Smooth(actual.x, dir.x, time);
        dir.y = Math::Smooth(actual.y, dir.y, time);
        dir.z = Math::Smooth(actual.z, dir.z, time);
        m_object->SetTilt(dir);

        m_object->SetCirVibration(glm::vec3(0.0f, 0.0f, 0.0f));
    }
    else if ( m_actionType == MHS_LOST )   // lost?
    {
        time = m_time;
        if ( time < 10.0f )  time *= time/10.0f;  // starts slowly

        dir.x = time*2.0f;
        dir.y = sinf(m_time*0.8f)*0.8f;
        dir.z = sinf(m_time*0.6f)*0.5f;
        m_object->SetTilt(dir);
        SetTilt(dir);

//?     dir.x = -(sinf(time*0.05f+Math::PI*1.5f)+1.0f)*100.0f;
        // original code: Math::Min(time/30.0f) (?) changed to time/30.0f
        dir.x = -(powf(time/30.0f, 4.0f))*1000.0f;  // from the distance
        dir.y = 0.0f;
        dir.z = 0.0f;
        m_object->SetLinVibration(dir);
        SetLinVibration(dir);

        glm::mat4 mat = m_object->GetWorldMatrix(0);
        pos = glm::vec3(0.5f, 3.7f, 0.0f);
        pos.x += (Math::Rand()-0.5f)*1.0f;
        pos.y += (Math::Rand()-0.5f)*1.0f;
        pos.z += (Math::Rand()-0.5f)*1.0f;
        pos = Math::Transform(mat, pos);
        speed.x = (Math::Rand()-0.5f)*0.5f;
        speed.y = (Math::Rand()-0.5f)*0.5f;
        speed.z = (Math::Rand()-0.5f)*0.5f;
        dim.x = 0.5f+Math::Rand()*0.5f;
        dim.y = dim.x;
        m_particle->CreateParticle(pos, speed, dim, Gfx::PARTILENS1, 5.0f, 0.0f, 0.0f);
    }
    else if ( m_actionType == MHS_SATCOM )  // look at the SatCom?
    {
        SetCirVibration(glm::vec3(0.0f, 0.0f, 0.0f));
        SetLinVibration(glm::vec3(0.0f, 0.0f, 0.0f));
        SetTilt(glm::vec3(0.0f, 0.0f, 0.0f));
    }
    else
    {
        if ( m_physics->GetLand() )  // on the ground?
        {
            time = event.rTime*8.0f;
            if ( bSwim )  time *= 0.25f;

            if ( action == MH_MARCH )   // walking?
            {
                dir.x = sinf(Math::Mod(rTime[0]+0.5f, 1.0f)*Math::PI*2.0f)*0.10f;
                dir.y = sinf(Math::Mod(rTime[0]+0.6f, 1.0f)*Math::PI*2.0f)*0.20f;
                s = m_physics->GetLinMotionX(MO_REASPEED)*0.03f;
            }
            else if ( action == MH_MARCHTAKE )   // takes walking?
            {
                dir.x = sinf(Math::Mod(rTime[0]+0.5f, 1.0f)*Math::PI*2.0f)*0.10f;
                dir.y = sinf(Math::Mod(rTime[0]+0.6f, 1.0f)*Math::PI*2.0f)*0.15f;
                s = m_physics->GetLinMotionX(MO_REASPEED)*0.02f;
            }
            else
            {
                dir.x = 0.0f;
                dir.y = 0.0f;
                s = m_physics->GetLinMotionX(MO_REASPEED)*0.03f;
            }

            if ( s < 0.0f )  s *= 0.5f;
            dir.z = -s*0.7f;

            actual = m_object->GetTilt();
            dir.x = Math::Smooth(actual.x, dir.x, time);
            dir.y = Math::Smooth(actual.y, dir.y, time);
            dir.z = Math::Smooth(actual.z, dir.z, time);
            if ( bOnBoard )  dir *= 0.3f;
            m_object->SetTilt(dir);
            SetTilt(dir);

            if ( action == MH_MARCH )   // walking?
            {
                p2.x = 0.0f;
                p2.y = sinf(Math::Mod(rTime[0]+0.5f, 1.0f)*Math::PI*2.0f)*0.5f;
                p2 = Math::RotatePoint(-m_object->GetRotationY(), p2);
                dir.x = p2.x;
                dir.z = p2.y;
                dir.y = sinf(Math::Mod(rTime[0]*2.0f, 1.0f)*Math::PI*2.0f)*0.3f;
            }
            else if ( action == MH_MARCHTAKE )   // takes walking?
            {
                p2.x = 0.0f;
                p2.y = sinf(Math::Mod(rTime[0]+0.5f, 1.0f)*Math::PI*2.0f)*0.25f;
                p2 = Math::RotatePoint(-m_object->GetRotationY(), p2);
                dir.x = p2.x;
                dir.z = p2.y;
                dir.y = sinf(Math::Mod(rTime[0]*2.0f, 1.0f)*Math::PI*2.0f)*0.05f-0.3f;
            }
            else
            {
                dir.x = 0.0f;
                dir.z = 0.0f;
                dir.y = 0.0f;
            }

            actual = m_object->GetLinVibration();
            dir.x = Math::Smooth(actual.x, dir.x, time);
            if ( action == MH_MARCHTAKE )  // takes walking?
            {
                dir.y = -hr;
            }
            else
            {
                s = Math::Min(m_armTimeAction, 1.0f);
                dir.y = Math::Smooth(actual.y, dir.y, time)*s;
                dir.y += -hr*(1.0f-s);
            }
            dir.z = Math::Smooth(actual.z, dir.z, time);
            if ( bOnBoard )  dir *= 0.3f;
            m_object->SetLinVibration(dir);

            dir.x = 0.0f;
            dir.z = 0.0f;
            dir.y = 0.0f;
            SetCirVibration(dir);
        }
    }

    // Management of the head.
    if ( m_actionType == MHS_TAKE ||  // takes?
         m_actionType == MHS_FLAG )   // takes?
    {
        m_object->SetPartRotationZ(1, Math::Smooth(m_object->GetPartRotationZ(1), sinf(m_armTimeAbs*1.0f)*0.2f-0.6f, event.rTime*5.0f));
        m_object->SetPartRotationX(1, sinf(m_armTimeAbs*1.1f)*0.1f);
        m_object->SetPartRotationY(1, Math::Smooth(m_object->GetPartRotationY(1), sinf(m_armTimeAbs*1.3f)*0.2f+rot*0.3f, event.rTime*5.0f));
    }
    else if ( m_actionType == MHS_TAKEOTHER ||  // takes?
              m_actionType == MHS_TAKEHIGH  )   // takes?
    {
        m_object->SetPartRotationZ(1, Math::Smooth(m_object->GetPartRotationZ(1), sinf(m_armTimeAbs*1.0f)*0.2f-0.3f, event.rTime*5.0f));
        m_object->SetPartRotationX(1, sinf(m_armTimeAbs*1.1f)*0.1f);
        m_object->SetPartRotationY(1, Math::Smooth(m_object->GetPartRotationY(1), sinf(m_armTimeAbs*1.3f)*0.2f+rot*0.3f, event.rTime*5.0f));
    }
    else if ( m_actionType == MHS_WIN )   // win
    {
        float   factor = 0.6f+(sinf(m_armTimeAbs*0.5f)*0.40f);
        m_object->SetPartRotationZ(1, sinf(m_armTimeAbs*5.0f)*0.20f*factor);
        m_object->SetPartRotationX(1, sinf(m_armTimeAbs*0.6f)*0.10f);
        m_object->SetPartRotationY(1, sinf(m_armTimeAbs*1.5f)*0.15f);
    }
    else if ( m_actionType == MHS_LOST )   // lost?
    {
        float   factor = 0.6f+(sinf(m_armTimeAbs*0.5f)*0.40f);
        m_object->SetPartRotationZ(1, sinf(m_armTimeAbs*0.6f)*0.10f);
        m_object->SetPartRotationX(1, sinf(m_armTimeAbs*0.7f)*0.10f);
        m_object->SetPartRotationY(1, sinf(m_armTimeAbs*3.0f)*0.30f*factor);
    }
    else if ( m_object->Implements(ObjectInterfaceType::Destroyable) && !dynamic_cast<CDestroyableObject*>(m_object)->IsDying() )  // dead?
    {
        m_object->SetPartRotationZ(1, Math::Smooth(m_object->GetPartRotationZ(1), sinf(m_armTimeAbs*1.0f)*0.2f, event.rTime*5.0f));
        m_object->SetPartRotationX(1, sinf(m_armTimeAbs*1.1f)*0.1f);
        m_object->SetPartRotationY(1, Math::Smooth(m_object->GetPartRotationY(1), sinf(m_armTimeAbs*1.3f)*0.2f+rot*0.3f, event.rTime*5.0f));
    }

    if ( bOnBoard )
    {
        m_object->SetPartRotationZ(1, 0.0f);
        m_object->SetPartRotationX(1, 0.0f);
        m_object->SetPartRotationY(1, 0.0f);
    }

    // Steps sound effects.
    if ( legAction == MH_MARCH     ||
         legAction == MH_MARCHTAKE )
    {
        SoundType   sound[2];
        float   synchro, volume[2], freq[2], hard;

        float speedX = m_physics->GetLinMotionX(MO_REASPEED);

        if (IsObjectCarryingCargo(m_object))
        {
            if ( speedX > 0.0f )  synchro = 0.15f;  // synchro forward
            else                 synchro = 0.35f;  // synchro backward
        }
        else
        {
            if ( speedX > 0.0f )  synchro = 0.21f;  // synchro forward
            else                 synchro = 0.29f;  // synchro backward
        }
        time = rTime[1]+synchro;

        if ( fabs(m_lastSoundMarch-time) > 0.4f &&
             Math::Mod(time, 0.5f) < 0.1f )
        {
            volume[0] = 0.5f;
            freq[0] = 1.0f;
            if (IsObjectCarryingCargo(m_object))
            {
//?             volume[0] *= 2.0f;
                freq[0] = 0.7f;
            }
            volume[1] = volume[0];
            freq[1] = freq[0];
            sound[0] = SOUND_NONE;
            sound[1] = SOUND_NONE;

            pos = m_object->GetPosition();

            level = m_water->GetLevel();
            if ( pos.y <= level+3.0f )  // underwater?
            {
                sound[0] = SOUND_STEPw;
            }
            else
            {
                hard = m_terrain->GetHardness(pos);

                if ( hard >= 0.875 )
                {
                    sound[0] = SOUND_STEPm;  // metal
                }
                else
                {
                    hard /= 0.875;
                    sound[0] = SOUND_STEPs;  // smooth
                    sound[1] = SOUND_STEPh;  // hard

                    volume[0] *= 1.0f-hard;
                    volume[1] *= hard;
                    if ( hard < 0.5f )
                    {
                        volume[0] *= 1.0f+hard*2.0f;
                        volume[1] *= 1.0f+hard*2.0f;
                    }
                    else
                    {
                        volume[0] *= 3.0f-hard*2.0f;
                        volume[1] *= 3.0f-hard*2.0f;
                    }
                    freq[0] *= 1.0f+hard;
                    freq[1] *= 0.5f+hard;
                }
            }

            if ( sound[0] != SOUND_NONE )
            {
                m_sound->Play(sound[0], pos, volume[0], freq[0]);
            }
            if ( sound[1] != SOUND_NONE )
            {
                m_sound->Play(sound[1], pos, volume[1], freq[1]);
            }
            m_lastSoundMarch = time;
        }
    }

    if ( legAction == MH_SWIM )
    {
        time = rTime[0]+0.5f;

        if ( fabs(m_lastSoundMarch-time) > 0.9f &&
             Math::Mod(time, 1.0f) < 0.1f )
        {
            m_sound->Play(SOUND_SWIM, m_object->GetPosition(), 0.5f);
            m_lastSoundMarch = time;
        }
    }

    m_lastSoundHhh -= event.rTime;
    if ( m_lastSoundHhh <= 0.0f &&
         m_object->GetSelect()  &&
         m_object->GetOption() == 0 )  // helmet?
    {
        m_sound->Play(SOUND_HUMAN1, m_object->GetPosition(), (0.5f+m_tired*0.2f));
        m_lastSoundHhh = (4.0f-m_tired*2.5f)+(4.0f-m_tired*2.5f)*Math::Rand();
    }

    return true;
}


// Management of the display mode when customizing the personal.

void CMotionHuman::StartDisplayPerso()
{
    m_bDisplayPerso = true;
}

void CMotionHuman::StopDisplayPerso()
{
    m_bDisplayPerso = false;
}
