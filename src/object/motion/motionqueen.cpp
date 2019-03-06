/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
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


#include "object/motion/motionqueen.h"

#include "app/app.h"

#include "graphics/engine/oldmodelmanager.h"

#include "object/old_object.h"

#include "physics/physics.h"


#include <stdio.h>


const float START_TIME = 1000.0f;       // beginning of the relative time



// Object's constructor.

CMotionQueen::CMotionQueen(COldObject* object)
    : CMotion(object),
      m_armAngles()
{
    m_armMember      = START_TIME;
    m_armTimeAbs     = START_TIME;
    m_armTimeMarch   = START_TIME;
}

// Object's destructor.

CMotionQueen::~CMotionQueen()
{
}


// Removes an object.

void CMotionQueen::DeleteObject(bool bAll)
{
}


// Creates a vehicle traveling any lands on the ground.

void CMotionQueen::Create(Math::Vector pos, float angle, ObjectType type,
                          float power, Gfx::COldModelManager* modelManager)
{
    int rank;

    m_object->SetType(type);

    // Creates main base.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_VEHICLE);  // this is a moving object
    m_object->SetObjectRank(0, rank);
    modelManager->AddModelReference("mother1.mod", false, rank);
    m_object->SetPosition(pos);
    m_object->SetRotationY(angle);

    // A vehicle must have a obligatory collision
    //with a sphere of center (0, y, 0) (see GetCrashSphere).
    m_object->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 0.0f, 0.0f), 20.0f, SOUND_BOUM, 0.20f));
    m_object->SetCameraCollisionSphere(Math::Sphere(Math::Vector(-2.0f, 10.0f, 0.0f), 25.0f));

    // Creates the head.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(1, rank);
    m_object->SetObjectParent(1, 0);
    modelManager->AddModelReference("mother2.mod", false, rank);
    m_object->SetPartPosition(1, Math::Vector(16.0f, 3.0f, 0.0f));

    // Creates a right-back leg.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(2, rank);
    m_object->SetObjectParent(2, 0);
    modelManager->AddModelReference("mother3.mod", false, rank);
    m_object->SetPartPosition(2, Math::Vector(-5.0f, -1.0f, -12.0f));

    // Creates a right-back foot.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(3, rank);
    m_object->SetObjectParent(3, 2);
    modelManager->AddModelReference("mother4.mod", false, rank);
    m_object->SetPartPosition(3, Math::Vector(0.0f, 0.0f, -8.5f));

    // Creates a middle-right leg.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(4, rank);
    m_object->SetObjectParent(4, 0);
    modelManager->AddModelReference("mother3.mod", false, rank);
    m_object->SetPartPosition(4, Math::Vector(3.5f, -1.0f, -12.0f));

    // Creates a middle-right foot.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(5, rank);
    m_object->SetObjectParent(5, 4);
    modelManager->AddModelReference("mother4.mod", false, rank);
    m_object->SetPartPosition(5, Math::Vector(0.0f, 0.0f, -8.5f));

    // Creates a right-front leg.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(6, rank);
    m_object->SetObjectParent(6, 0);
    modelManager->AddModelReference("mother3.mod", false, rank);
    m_object->SetPartPosition(6, Math::Vector(10.0f, -1.0f, -10.0f));

    // Creates a right-front foot.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(7, rank);
    m_object->SetObjectParent(7, 6);
    modelManager->AddModelReference("mother4.mod", false, rank);
    m_object->SetPartPosition(7, Math::Vector(0.0f, 0.0f, -8.5f));

    // Creates a left-back leg.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(8, rank);
    m_object->SetObjectParent(8, 0);
    modelManager->AddModelReference("mother3.mod", false, rank);
    m_object->SetPartPosition(8, Math::Vector(-5.0f, -1.0f, 12.0f));
    m_object->SetPartRotationY(8, Math::PI);

    // Creates a left-back foot.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(9, rank);
    m_object->SetObjectParent(9, 8);
    modelManager->AddModelReference("mother4.mod", false, rank);
    m_object->SetPartPosition(9, Math::Vector(0.0f, 0.0f, -8.5f));

    // Creates a middle-left leg.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(10, rank);
    m_object->SetObjectParent(10, 0);
    modelManager->AddModelReference("mother3.mod", false, rank);
    m_object->SetPartPosition(10, Math::Vector(3.5f, -1.0f, 12.0f));
    m_object->SetPartRotationY(10, Math::PI);

    // Creates a middle-left foot.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(11, rank);
    m_object->SetObjectParent(11, 10);
    modelManager->AddModelReference("mother4.mod", false, rank);
    m_object->SetPartPosition(11, Math::Vector(0.0f, 0.0f, -8.5f));

    // Creates a left-front leg.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(12, rank);
    m_object->SetObjectParent(12, 0);
    modelManager->AddModelReference("mother3.mod", false, rank);
    m_object->SetPartPosition(12, Math::Vector(10.0f, -1.0f, 10.0f));
    m_object->SetPartRotationY(12, Math::PI);

    // Creates a left-front foot.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(13, rank);
    m_object->SetObjectParent(13, 12);
    modelManager->AddModelReference("mother4.mod", false, rank);
    m_object->SetPartPosition(13, Math::Vector(0.0f, 0.0f, -8.5f));

    // Creates the right antenna.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(14, rank);
    m_object->SetObjectParent(14, 1);
    modelManager->AddModelReference("mother5.mod", false, rank);
    m_object->SetPartPosition(14, Math::Vector(6.0f, 1.0f, -2.5f));

    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(15, rank);
    m_object->SetObjectParent(15, 14);
    modelManager->AddModelReference("mother6.mod", false, rank);
    m_object->SetPartPosition(15, Math::Vector(8.0f, 0.0f, 0.0f));

    // Creates the left antenna.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(16, rank);
    m_object->SetObjectParent(16, 1);
    modelManager->AddModelReference("mother5.mod", false, rank);
    m_object->SetPartPosition(16, Math::Vector(6.0f, 1.0f, 2.5f));

    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(17, rank);
    m_object->SetObjectParent(17, 16);
    modelManager->AddModelReference("mother6.mod", false, rank);
    m_object->SetPartPosition(17, Math::Vector(8.0f, 0.0f, 0.0f));

    // Creates the right claw.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(18, rank);
    m_object->SetObjectParent(18, 1);
    modelManager->AddModelReference("mother7.mod", false, rank);
    m_object->SetPartPosition(18, Math::Vector(-4.0f, -3.5f, -8.0f));
    m_object->SetPartScaleX(18, 1.2f);

    // Creates the left claw.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(19, rank);
    m_object->SetObjectParent(19, 1);
    modelManager->AddModelReference("mother7.mod", true, rank);
    m_object->SetPartPosition(19, Math::Vector(-4.0f, -3.5f, 8.0f));
    m_object->SetPartScaleX(19, 1.2f);

    m_object->CreateShadowCircle(18.0f, 0.8f);

    CreatePhysics();
    m_object->SetFloorHeight(0.0f);

    pos = m_object->GetPosition();
    m_object->SetPosition(pos);  // to display the shadows immediately

    m_engine->LoadAllTextures();
}

// Creates the physics of the object.

void CMotionQueen::CreatePhysics()
{
    Character*  character;
    int         i;

    int member[] =
    {
    //  x1,y1,z1,       x2,y2,z2,       x3,y3,z3,       // in the air:
        30,30,10,       35,-15,10,      35,-35,10,      // t0: legs 1..3
        -80,-45,-35,        -115,-40,-35,       -90,10,-55,     // t0: feet 1..3
        0,0,0,          0,0,0,          0,0,0,          // t0: unused
                                                        // on the ground:
        15,-5,10,       10,-30,10,      5,-50,10,       // t1: legs 1..3
        -90,-15,-15,        -110,-55,-35,       -75,-75,-30,        // t1: feet 1..3
        0,0,0,          0,0,0,          0,0,0,          // t1: unused
                                                        // on the ground back:
        0,40,10,        5,5,10,         0,-15,10,       // t2: legs 1..3
        -45,0,-55,      -65,10,-50,     -125,-85,-45,       // t2: feet 1..3
        0,0,0,          0,0,0,          0,0,0,          // t2: unused
    };

    character = m_object->GetCharacter();
    character->wheelFront = 10.0f;
    character->wheelBack  = 10.0f;
    character->wheelLeft  = 20.0f;
    character->wheelRight = 20.0f;
    character->height     =  3.0f;

    m_physics->SetLinMotionX(MO_ADVSPEED,   8.0f);
    m_physics->SetLinMotionX(MO_RECSPEED,   8.0f);
    m_physics->SetLinMotionX(MO_ADVACCEL,  10.0f);
    m_physics->SetLinMotionX(MO_RECACCEL,  10.0f);
    m_physics->SetLinMotionX(MO_STOACCEL,  40.0f);
    m_physics->SetLinMotionX(MO_TERSLIDE,   5.0f);
    m_physics->SetLinMotionZ(MO_TERSLIDE,   5.0f);
    m_physics->SetLinMotionX(MO_TERFORCE,  30.0f);
    m_physics->SetLinMotionZ(MO_TERFORCE,  20.0f);
    m_physics->SetLinMotionZ(MO_MOTACCEL,  40.0f);

    m_physics->SetCirMotionY(MO_ADVSPEED,   0.1f*Math::PI);
    m_physics->SetCirMotionY(MO_RECSPEED,   0.1f*Math::PI);
    m_physics->SetCirMotionY(MO_ADVACCEL,  10.0f);
    m_physics->SetCirMotionY(MO_RECACCEL,  10.0f);
    m_physics->SetCirMotionY(MO_STOACCEL,  20.0f);

    for ( i=0 ; i<3*3*3*3 ; i++ )
    {
        m_armAngles[i] = member[i];
    }
}


// Management of an event.

bool CMotionQueen::EventProcess(const Event &event)
{
    CMotion::EventProcess(event);

    if ( event.type == EVENT_FRAME )
    {
        return EventFrame(event);
    }

    return true;
}

// Management of an event.

bool CMotionQueen::EventFrame(const Event &event)
{
    Math::Vector    dir;
    float       s, a, prog;
    int         i, st, nd;
    bool        bStop;

    if ( m_engine->GetPause() )  return true;
    if ( !m_engine->IsVisiblePoint(m_object->GetPosition()) )  return true;

    s =     m_physics->GetLinMotionX(MO_MOTSPEED)*1.5f;
    a = fabs(m_physics->GetCirMotionY(MO_MOTSPEED)*26.0f);

    if ( s == 0.0f && a != 0.0f )  a *= 1.5f;

    m_armTimeAbs += event.rTime;
    m_armTimeMarch += (s)*event.rTime*0.05f;
    m_armMember += (s+a)*event.rTime*0.05f;

    bStop = ( a == 0.0f && s == 0.0f );  // stop?

    if ( bStop )
    {
        prog = Math::Mod(m_armTimeAbs, 2.0f)/10.0f;
        a = Math::Mod(m_armMember, 1.0f);
        a = (prog-a)*event.rTime*1.0f;  // stop position just pleasantly
        m_armMember += a;
    }

    for ( i=0 ; i<6 ; i++ )  // the six legs
    {
        if ( i < 3 )  prog = Math::Mod(m_armMember+(2.0f-(i%3))*0.33f+0.0f, 1.0f);
        else          prog = Math::Mod(m_armMember+(2.0f-(i%3))*0.33f+0.3f, 1.0f);
        if ( prog < 0.33f )  // t0..t1 ?
        {
            prog = prog/0.33f;  // 0..1
            st = 0;  // index start
            nd = 1;  // index end
        }
        else if ( prog < 0.67f )  // t1..t2 ?
        {
            prog = (prog-0.33f)/0.33f;  // 0..1
            st = 1;  // index start
            nd = 2;  // index end
        }
        else    // t2..t0 ?
        {
            prog = (prog-0.67f)/0.33f;  // 0..1
            st = 2;  // index start
            nd = 0;  // index end
        }
        st = st*27+(i%3)*3;
        nd = nd*27+(i%3)*3;
        if ( i < 3 )  // right leg (1..3) ?
        {
            m_object->SetPartRotationX(2+2*i+0, Math::PropAngle(m_armAngles[st+ 0], m_armAngles[nd+ 0], prog));
            m_object->SetPartRotationY(2+2*i+0, Math::PropAngle(m_armAngles[st+ 1], m_armAngles[nd+ 1], prog));
            m_object->SetPartRotationZ(2+2*i+0, Math::PropAngle(m_armAngles[st+ 2], m_armAngles[nd+ 2], prog));
            m_object->SetPartRotationX(2+2*i+1, Math::PropAngle(m_armAngles[st+ 9], m_armAngles[nd+ 9], prog));
            m_object->SetPartRotationY(2+2*i+1, Math::PropAngle(m_armAngles[st+10], m_armAngles[nd+10], prog));
            m_object->SetPartRotationZ(2+2*i+1, Math::PropAngle(m_armAngles[st+11], m_armAngles[nd+11], prog));
        }
        else    // left leg (4..6) ?
        {
            m_object->SetPartRotationX(2+2*i+0, Math::PropAngle(    m_armAngles[st+ 0],     m_armAngles[nd+ 0], prog));
            m_object->SetPartRotationY(2+2*i+0, Math::PropAngle(180-m_armAngles[st+ 1], 180-m_armAngles[nd+ 1], prog));
            m_object->SetPartRotationZ(2+2*i+0, Math::PropAngle(   -m_armAngles[st+ 2],    -m_armAngles[nd+ 2], prog));
            m_object->SetPartRotationX(2+2*i+1, Math::PropAngle(    m_armAngles[st+ 9],     m_armAngles[nd+ 9], prog));
            m_object->SetPartRotationY(2+2*i+1, Math::PropAngle(   -m_armAngles[st+10],    -m_armAngles[nd+10], prog));
            m_object->SetPartRotationZ(2+2*i+1, Math::PropAngle(   -m_armAngles[st+11],    -m_armAngles[nd+11], prog));
        }
    }

    assert(m_object->Implements(ObjectInterfaceType::Destroyable));
    if ( !bStop && !dynamic_cast<CDestroyableObject*>(m_object)->IsDying() )
    {
        a = Math::Mod(m_armTimeMarch, 1.0f);
        if ( a < 0.5f )  a = -1.0f+4.0f*a;  // -1..1
        else             a =  3.0f-4.0f*a;  // 1..-1
        dir.x = sinf(a)*0.03f;

        s = Math::Mod(m_armTimeMarch/2.0f, 1.0f);
        if ( s < 0.5f )  s = -1.0f+4.0f*s;  // -1..1
        else             s =  3.0f-4.0f*s;  // 1..-1
        dir.z = sinf(s)*0.05f;

        dir.y = 0.0f;
        m_object->SetTilt(dir);

        a = Math::Mod(m_armMember-0.1f, 1.0f);
        if ( a < 0.33f )
        {
            dir.y = -(1.0f-(a/0.33f))*0.3f;
        }
        else if ( a < 0.67f )
        {
            dir.y = 0.0f;
        }
        else
        {
            dir.y = -(a-0.67f)/0.33f*0.3f;
        }
        dir.x = 0.0f;
        dir.z = 0.0f;
        m_object->SetLinVibration(dir);
    }

    m_object->SetPartRotationZ(1, sinf(m_armTimeAbs*0.5f)*0.20f);  // head
    m_object->SetPartRotationX(1, sinf(m_armTimeAbs*0.6f)*0.10f);  // head
    m_object->SetPartRotationY(1, sinf(m_armTimeAbs*0.7f)*0.20f);  // head

    m_object->SetPartRotationZ(14,  0.50f);
    m_object->SetPartRotationZ(16,  0.50f);
    m_object->SetPartRotationY(14,  0.80f+sinf(m_armTimeAbs*1.1f)*0.53f);  // right antenna
    m_object->SetPartRotationY(15,  0.70f-sinf(m_armTimeAbs*1.7f)*0.43f);
    m_object->SetPartRotationY(16, -0.80f+sinf(m_armTimeAbs*0.9f)*0.53f);  // left antenna
    m_object->SetPartRotationY(17, -0.70f-sinf(m_armTimeAbs*1.3f)*0.43f);

    m_object->SetPartRotationY(18, sinf(m_armTimeAbs*1.1f)*0.20f);  // right claw
    m_object->SetPartRotationZ(18, -0.20f);
    m_object->SetPartRotationY(19, sinf(m_armTimeAbs*0.9f)*0.20f);  // left claw
    m_object->SetPartRotationZ(19, -0.20f);

    return true;
}
