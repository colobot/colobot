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


#include "object/motion/motionbee.h"

#include "app/app.h"

#include "graphics/engine/oldmodelmanager.h"

#include "object/old_object.h"

#include "physics/physics.h"


#include <stdio.h>


const float START_TIME = 1000.0f;       // beginning of the relative time



// Object's constructor.

CMotionBee::CMotionBee(COldObject* object)
    : CMotion(object),
      m_armAngles()
{
    m_armMember      = START_TIME;
    m_armTimeAbs     = START_TIME;
    m_armTimeMarch   = START_TIME;
}

// Object's destructor.

CMotionBee::~CMotionBee()
{
}


// Removes an object.

void CMotionBee::DeleteObject(bool bAll)
{
}


// Creates a vehicle traveling any lands on the ground.

void CMotionBee::Create(Math::Vector pos, float angle, ObjectType type,
                        float power, Gfx::COldModelManager* modelManager)
{
    int rank;

    m_object->SetType(type);

    // Creates main base.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_VEHICLE);  // this is a moving object
    m_object->SetObjectRank(0, rank);
    modelManager->AddModelReference("bee1.mod", false, rank);
    m_object->SetPosition(pos);
    m_object->SetRotationY(angle);

    // A vehicle must have an obligatory collision
    // with a sphere of center (0, y, 0) (see GetCrashSphere).
    m_object->AddCrashSphere(CrashSphere(Math::Vector(0.0f, 0.0f, 0.0f), 4.0f, SOUND_BOUM, 0.20f));
    m_object->SetCameraCollisionSphere(Math::Sphere(Math::Vector(-1.0f, 1.0f, 0.0f), 5.0f));

    // Creates the head.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(1, rank);
    m_object->SetObjectParent(1, 0);
    modelManager->AddModelReference("bee2.mod", false, rank);
    m_object->SetPartPosition(1, Math::Vector(1.6f, 0.3f, 0.0f));

    // Creates the tail.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(2, rank);
    m_object->SetObjectParent(2, 0);
    modelManager->AddModelReference("bee3.mod", false, rank);
    m_object->SetPartPosition(2, Math::Vector(-0.8f, 0.0f, 0.0f));

    // Creates a right-back thigh.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(3, rank);
    m_object->SetObjectParent(3, 0);
    modelManager->AddModelReference("ant4.mod", false, rank);
    m_object->SetPartPosition(3, Math::Vector(-0.3f, -0.1f, -0.2f));

    // Creates a right-back leg.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(4, rank);
    m_object->SetObjectParent(4, 3);
    modelManager->AddModelReference("ant5.mod", false, rank);
    m_object->SetPartPosition(4, Math::Vector(0.0f, 0.0f, -1.0f));

    // Creates a right-back foot.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(5, rank);
    m_object->SetObjectParent(5, 4);
    modelManager->AddModelReference("ant6.mod", false, rank);
    m_object->SetPartPosition(5, Math::Vector(0.0f, 0.0f, -2.0f));

    // Creates two middle-right thighs.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(6, rank);
    m_object->SetObjectParent(6, 0);
    modelManager->AddModelReference("ant4.mod", false, rank);
    m_object->SetPartPosition(6, Math::Vector(0.3f, -0.1f, -0.4f));

    // Creates two middle-right legs.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(7, rank);
    m_object->SetObjectParent(7, 6);
    modelManager->AddModelReference("ant5.mod", false, rank);
    m_object->SetPartPosition(7, Math::Vector(0.0f, 0.0f, -1.0f));

    // Creates two middle-right feet.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(8, rank);
    m_object->SetObjectParent(8, 7);
    modelManager->AddModelReference("ant6.mod", false, rank);
    m_object->SetPartPosition(8, Math::Vector(0.0f, 0.0f, -2.0f));

    // Creates the right front thigh.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(9, rank);
    m_object->SetObjectParent(9, 0);
    modelManager->AddModelReference("ant4.mod", false, rank);
    m_object->SetPartPosition(9, Math::Vector(1.0f, -0.1f, -0.7f));

    // Creates the right front leg.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(10, rank);
    m_object->SetObjectParent(10, 9);
    modelManager->AddModelReference("ant5.mod", false, rank);
    m_object->SetPartPosition(10, Math::Vector(0.0f, 0.0f, -1.0f));

    // Creates the right front foot.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(11, rank);
    m_object->SetObjectParent(11, 10);
    modelManager->AddModelReference("ant6.mod", false, rank);
    m_object->SetPartPosition(11, Math::Vector(0.0f, 0.0f, -2.0f));

    // Creates a left-back thigh.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(12, rank);
    m_object->SetObjectParent(12, 0);
    modelManager->AddModelReference("ant4.mod", false, rank);
    m_object->SetPartPosition(12, Math::Vector(-0.3f, -0.1f, 0.2f));
    m_object->SetPartRotationY(12, Math::PI);

    // Creates a left-back leg.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(13, rank);
    m_object->SetObjectParent(13, 12);
    modelManager->AddModelReference("ant5.mod", false, rank);
    m_object->SetPartPosition(13, Math::Vector(0.0f, 0.0f, -1.0f));

    // Creates a left-back foot.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(14, rank);
    m_object->SetObjectParent(14, 13);
    modelManager->AddModelReference("ant6.mod", false, rank);
    m_object->SetPartPosition(14, Math::Vector(0.0f, 0.0f, -2.0f));

    // Creates two middle-left thigh.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(15, rank);
    m_object->SetObjectParent(15, 0);
    modelManager->AddModelReference("ant4.mod", false, rank);
    m_object->SetPartPosition(15, Math::Vector(0.3f, -0.1f, 0.4f));
    m_object->SetPartRotationY(15, Math::PI);

    // Creates two middle-left legs.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(16, rank);
    m_object->SetObjectParent(16, 15);
    modelManager->AddModelReference("ant5.mod", false, rank);
    m_object->SetPartPosition(16, Math::Vector(0.0f, 0.0f, -1.0f));

    // Creates two middle-left feet.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(17, rank);
    m_object->SetObjectParent(17, 16);
    modelManager->AddModelReference("ant6.mod", false, rank);
    m_object->SetPartPosition(17, Math::Vector(0.0f, 0.0f, -2.0f));

    // Creates front-left thigh.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(18, rank);
    m_object->SetObjectParent(18, 0);
    modelManager->AddModelReference("ant4.mod", false, rank);
    m_object->SetPartPosition(18, Math::Vector(1.0f, -0.1f, 0.7f));
    m_object->SetPartRotationY(18, Math::PI);

    // Creates front-left leg.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(19, rank);
    m_object->SetObjectParent(19, 18);
    modelManager->AddModelReference("ant5.mod", false, rank);
    m_object->SetPartPosition(19, Math::Vector(0.0f, 0.0f, -1.0f));

    // Creates front-left foot.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(20, rank);
    m_object->SetObjectParent(20, 19);
    modelManager->AddModelReference("ant6.mod", false, rank);
    m_object->SetPartPosition(20, Math::Vector(0.0f, 0.0f, -2.0f));

    // Creates the right wing.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(21, rank);
    m_object->SetObjectParent(21, 0);
    modelManager->AddModelReference("bee7.mod", false, rank);
    m_object->SetPartPosition(21, Math::Vector(0.8f, 0.4f, -0.5f));

    // Creates the left wing.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(22, rank);
    m_object->SetObjectParent(22, 0);
    modelManager->AddModelReference("bee7.mod", true, rank);
    m_object->SetPartPosition(22, Math::Vector(0.8f, 0.4f, 0.5f));

    m_object->CreateShadowCircle(6.0f, 0.5f);

    CreatePhysics();
    m_object->SetFloorHeight(0.0f);

    pos = m_object->GetPosition();
    m_object->SetPosition(pos);  // to display the shadows immediately

    m_engine->LoadAllTextures();
}

// Creates the physical object.

void CMotionBee::CreatePhysics()
{
    Character*  character;
    int         i;

    int member_march[] =
    {
    //  x1,y1,z1,   x2,y2,z2,   x3,y3,z3,   // in the air:
        0,45,0,     0,45,0,     0,50,0,     // t0: thighs 1..3
        30,-70,0,   20,-105,20, 25,-100,0,  // t0: legs 1..3
        -70,75,0,   -30,80,0,   -80,80,0,   // t0: feet 1..3
                                            // on the ground:
        0,30,0,     0,20,0,     0,15,0,     // t1: thighs 1..3
        -15,-50,0,  -20,-60,0,  -10,-75,0,  // t1: legs 1..3
        -40,50,0,   -25,15,0,   -50,35,0,   // t1: feet 1..3
                                            // on the ground back:
        0,35,0,     0,30,0,     0,20,0,     // t2: thighs 1..3
        -20,-15,0,  -30,-55,0,  -25,-70,15, // t2: legs 1..3
        -25,25,0,   -20,60,0,   -30,95,0,   // t2: feet 1..3
    };

    int member_spec[] =
    {
    //  x1,y1,z1,   x2,y2,z2,   x3,y3,z3,   // ball carrier:
        0,45,0,     0,45,0,     0,50,0,     // s0: thighs 1..3
        -35,-70,0,  -20,-85,-25,    -25,-100,0, // s0: legs 1..3
        -110,75,-15,    -130,80,-25,    -125,40,0,  // s0: feet 1..3
                                            // burning:
        0,45,0,     0,45,0,     0,50,0,     // s1: thighs 1..3
        -35,-70,0,  -20,-85,-25,    -25,-100,0, // s1: legs 1..3
        -110,75,-15,    -130,80,-25,    -125,40,0,  // s1: feet 1..3
                                            // destroyed:
        0,45,0,     0,45,0,     0,50,0,     // s2: thighs 1..3
        -35,-70,0,  -20,-85,-25,    -25,-100,0, // s2: legs 1..3
        -110,75,-15,    -130,80,-25,    -125,40,0,  // s2: feet 1..3
    };

    character = m_object->GetCharacter();
    character->wheelFront = 3.0f;
    character->wheelBack  = 3.0f;
    character->wheelLeft  = 5.0f;
    character->wheelRight = 5.0f;
    character->height     = 2.5f;

    m_physics->SetLinMotionX(MO_ADVSPEED,  50.0f);
    m_physics->SetLinMotionX(MO_RECSPEED,  50.0f);
    m_physics->SetLinMotionX(MO_ADVACCEL,  20.0f);
    m_physics->SetLinMotionX(MO_RECACCEL,  20.0f);
    m_physics->SetLinMotionX(MO_STOACCEL,  20.0f);
    m_physics->SetLinMotionX(MO_TERSLIDE,   5.0f);
    m_physics->SetLinMotionZ(MO_TERSLIDE,   5.0f);
    m_physics->SetLinMotionX(MO_TERFORCE,  10.0f);
    m_physics->SetLinMotionZ(MO_TERFORCE,  10.0f);
    m_physics->SetLinMotionZ(MO_MOTACCEL,  40.0f);
    m_physics->SetLinMotionY(MO_ADVSPEED,  60.0f);
    m_physics->SetLinMotionY(MO_RECSPEED,  60.0f);
    m_physics->SetLinMotionY(MO_ADVACCEL,  20.0f);
    m_physics->SetLinMotionY(MO_RECACCEL,  50.0f);
    m_physics->SetLinMotionY(MO_STOACCEL,  50.0f);

    m_physics->SetCirMotionY(MO_ADVSPEED,   1.0f*Math::PI);
    m_physics->SetCirMotionY(MO_RECSPEED,   1.0f*Math::PI);
    m_physics->SetCirMotionY(MO_ADVACCEL,  20.0f);
    m_physics->SetCirMotionY(MO_RECACCEL,  20.0f);
    m_physics->SetCirMotionY(MO_STOACCEL,  40.0f);

    for ( i=0 ; i<3*3*3*3 ; i++ )
    {
        m_armAngles[3*3*3*3*MB_MARCH+i] = member_march[i];
    }
    for ( i=0 ; i<3*3*3*3 ; i++ )
    {
        m_armAngles[3*3*3*3*MB_SPEC+i] = member_spec[i];
    }
}


// Management of an event.

bool CMotionBee::EventProcess(const Event &event)
{
    CMotion::EventProcess(event);

    if ( event.type == EVENT_FRAME )
    {
        return EventFrame(event);
    }

    return true;
}

// Management of an event.

bool CMotionBee::EventFrame(const Event &event)
{
    Math::Vector        dir;
    float           s, a, prog = 0.0f;
    int             action, i, st, nd;
    bool            bStop;

    if ( m_engine->GetPause() )  return true;
    if ( !m_engine->IsVisiblePoint(m_object->GetPosition()) )  return true;

    s =     m_physics->GetLinMotionX(MO_MOTSPEED)*0.30f;
    a = fabs(m_physics->GetCirMotionY(MO_MOTSPEED)*2.00f);

    if ( s == 0.0f && a != 0.0f )  a *= 1.5f;

    m_armTimeAbs += event.rTime;
    m_armTimeMarch += (s)*event.rTime*0.15f;
    m_armMember += (s+a)*event.rTime*0.15f;

    bStop = ( a == 0.0f && s == 0.0f );  // stopped?
    if ( !m_physics->GetLand() )  bStop = true;

    if ( bStop )
    {
        prog = Math::Mod(m_armTimeAbs, 2.0f)/10.0f;
        a = Math::Mod(m_armMember, 1.0f);
        a = (prog-a)*event.rTime*2.0f;  // stop position is pleasantly
        m_armMember += a;
    }

    action = MB_MARCH;  // flying

    m_actionType = -1;
    if (IsObjectCarryingCargo(m_object))  m_actionType = MBS_HOLD;  // carries the ball

    assert(m_object->Implements(ObjectInterfaceType::Destroyable));
    if ( dynamic_cast<CDestroyableObject*>(m_object)->GetDying() == DeathType::Burning )  // burning?
    {
        m_actionType = MBS_BURN;
    }
    else if ( dynamic_cast<CDestroyableObject*>(m_object)->IsDying() )  // destroyed?
    {
        m_actionType = MBS_RUIN;
    }

    for ( i=0 ; i<6 ; i++ )  // the six legs
    {
        if ( m_actionType != -1 )  // special action in progress?
        {
            st = 3*3*3*3*MB_SPEC + 3*3*3*m_actionType + (i%3)*3;
            nd = st;
        }
        else
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
            st = 3*3*3*3*action + st*3*3*3 + (i%3)*3;
            nd = 3*3*3*3*action + nd*3*3*3 + (i%3)*3;
        }

        if ( i < 3 )  // right leg (1..3) ?
        {
            m_object->SetPartRotationX(3+3*i+0, Math::PropAngle(m_armAngles[st+ 0], m_armAngles[nd+ 0], prog));
            m_object->SetPartRotationY(3+3*i+0, Math::PropAngle(m_armAngles[st+ 1], m_armAngles[nd+ 1], prog));
            m_object->SetPartRotationZ(3+3*i+0, Math::PropAngle(m_armAngles[st+ 2], m_armAngles[nd+ 2], prog));
            m_object->SetPartRotationX(3+3*i+1, Math::PropAngle(m_armAngles[st+ 9], m_armAngles[nd+ 9], prog));
            m_object->SetPartRotationY(3+3*i+1, Math::PropAngle(m_armAngles[st+10], m_armAngles[nd+10], prog));
            m_object->SetPartRotationZ(3+3*i+1, Math::PropAngle(m_armAngles[st+11], m_armAngles[nd+11], prog));
            m_object->SetPartRotationX(3+3*i+2, Math::PropAngle(m_armAngles[st+18], m_armAngles[nd+18], prog));
            m_object->SetPartRotationY(3+3*i+2, Math::PropAngle(m_armAngles[st+19], m_armAngles[nd+19], prog));
            m_object->SetPartRotationZ(3+3*i+2, Math::PropAngle(m_armAngles[st+20], m_armAngles[nd+20], prog));
        }
        else    // left leg(4..6) ?
        {
            m_object->SetPartRotationX(3+3*i+0, Math::PropAngle(   -m_armAngles[st+ 0],    -m_armAngles[nd+ 0], prog));
            m_object->SetPartRotationY(3+3*i+0, Math::PropAngle(180-m_armAngles[st+ 1], 180-m_armAngles[nd+ 1], prog));
            m_object->SetPartRotationZ(3+3*i+0, Math::PropAngle(   -m_armAngles[st+ 2],    -m_armAngles[nd+ 2], prog));
            m_object->SetPartRotationX(3+3*i+1, Math::PropAngle(    m_armAngles[st+ 9],     m_armAngles[nd+ 9], prog));
            m_object->SetPartRotationY(3+3*i+1, Math::PropAngle(   -m_armAngles[st+10],    -m_armAngles[nd+10], prog));
            m_object->SetPartRotationZ(3+3*i+1, Math::PropAngle(   -m_armAngles[st+11],    -m_armAngles[nd+11], prog));
            m_object->SetPartRotationX(3+3*i+2, Math::PropAngle(    m_armAngles[st+18],     m_armAngles[nd+18], prog));
            m_object->SetPartRotationY(3+3*i+2, Math::PropAngle(   -m_armAngles[st+19],    -m_armAngles[nd+19], prog));
            m_object->SetPartRotationZ(3+3*i+2, Math::PropAngle(   -m_armAngles[st+20],    -m_armAngles[nd+20], prog));
        }
    }

    if ( m_physics->GetLand() )  // on the ground?
    {
        assert(m_object->Implements(ObjectInterfaceType::Destroyable));
        if ( bStop || dynamic_cast<CDestroyableObject*>(m_object)->GetDying() == DeathType::Burning )
        {
            m_object->SetPartRotationZ(2, sinf(m_armTimeAbs*1.7f)*0.15f+0.35f);  // tail
        }
        if ( !dynamic_cast<CDestroyableObject*>(m_object)->IsDying() )
        {
            a = Math::Mod(m_armTimeMarch, 1.0f);
            if ( a < 0.5f )  a = -1.0f+4.0f*a;  // -1..1
            else             a =  3.0f-4.0f*a;  // 1..-1
            dir.x = sinf(a)*0.05f;

            s = Math::Mod(m_armTimeMarch/2.0f, 1.0f);
            if ( s < 0.5f )  s = -1.0f+4.0f*s;  // -1..1
            else             s =  3.0f-4.0f*s;  // 1..-1
            dir.z = sinf(s)*0.1f;

            dir.y = 0.0f;
            m_object->SetTilt(dir);

            m_object->SetPartRotationZ(2, -sinf(a)*0.3f);  // tail

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
    }

    if ( m_physics->GetLand() )
    {
        if ( bStop )  prog = 0.05f;
        else          prog = 0.15f;
    }
    else
    {
        prog = 1.00f;
    }

    m_object->SetPartRotationX(21, (sinf(m_armTimeAbs*30.0f)+1.0f)*(Math::PI/4.0f)*prog);
    m_object->SetPartRotationY(21, -Math::Rand()*Math::PI/6.0f*prog);

    m_object->SetPartRotationX(22, -(sinf(m_armTimeAbs*30.0f)+1.0f)*(Math::PI/4.0f)*prog);
    m_object->SetPartRotationY(22, Math::Rand()*Math::PI/6.0f*prog);

    m_object->SetPartRotationZ(1, sinf(m_armTimeAbs*1.4f)*0.20f);  // head
    m_object->SetPartRotationX(1, sinf(m_armTimeAbs*1.9f)*0.10f);  // head
    m_object->SetPartRotationY(1, sinf(m_armTimeAbs*2.1f)*0.50f);  // head

    return true;
}
