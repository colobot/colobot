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


#include "object/motion/motionspider.h"

#include "app/app.h"

#include "graphics/engine/oldmodelmanager.h"
#include "graphics/engine/particle.h"

#include "object/old_object.h"

#include "object/subclass/base_alien.h"

#include "physics/physics.h"

#include <stdio.h>



const float START_TIME = 1000.0f;       // beginning of the relative time



// Object's constructor.

CMotionSpider::CMotionSpider(COldObject* object)
    : CMotion(object),
      m_armAngles()
{
    m_armMember      = START_TIME;
    m_armTimeAbs     = START_TIME;
    m_armTimeAction  = START_TIME;
    m_lastParticle = 0.0f;
}

// Object's destructor.

CMotionSpider::~CMotionSpider()
{
}


// Removes an object.

void CMotionSpider::DeleteObject(bool bAll)
{
}


// Creates a vehicle traveling any lands on the ground.

void CMotionSpider::Create(Math::Vector pos, float angle, ObjectType type,
                           float power, Gfx::COldModelManager* modelManager)
{
    int         rank, i, j, parent;
    char        name[50];

    float           table[] =
    {
    //    x       y       z
         0.6f,   0.0f,   0.0f,  // back leg
         0.0f,   0.0f,  -2.0f,
         0.0f,   0.0f,  -2.0f,
         0.0f,   0.0f,  -2.0f,

         0.8f,   0.0f,  -0.2f,  // middle-back leg
         0.0f,   0.0f,  -2.0f,
         0.0f,   0.0f,  -2.0f,
         0.0f,   0.0f,  -2.0f,

         1.0f,   0.0f,  -0.2f,  // middle-front leg
         0.0f,   0.0f,  -2.0f,
         0.0f,   0.0f,  -2.0f,
         0.0f,   0.0f,  -2.0f,

         1.2f,   0.0f,   0.0f,  // front leg
         0.0f,   0.0f,  -2.0f,
         0.0f,   0.0f,  -2.0f,
         0.0f,   0.0f,  -2.0f,
    };

    m_object->SetType(type);

    // Creates the main base.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_VEHICLE);  // this is a moving object
    m_object->SetObjectRank(0, rank);
    // This is an "empty" object, without triangles
    m_object->SetPosition(pos);
    m_object->SetRotationY(angle);

    // A vehicle must have a obligatory collision
    // with a sphere of center (0, y, 0) (see GetCrashSphere).
    m_object->AddCrashSphere(CrashSphere(Math::Vector(0.0f, -2.0f, 0.0f), 4.0f, SOUND_BOUM, 0.20f));
    m_object->SetCameraCollisionSphere(Math::Sphere(Math::Vector(-0.5f, 1.0f, 0.0f), 4.0f));

    // Creates the abdomen.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(1, rank);
    m_object->SetObjectParent(1, 0);
    modelManager->AddModelReference("spider1.mod", false, rank);
    m_object->SetPartPosition(1, Math::Vector(1.0f, 0.0f, 0.0f));

    // Creates the head.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(2, rank);
    m_object->SetObjectParent(2, 0);
    modelManager->AddModelReference("spider2.mod", false, rank);
    m_object->SetPartPosition(2, Math::Vector(1.0f, 0.0f, 0.0f));

    // Creates legs.
    for ( i=0 ; i<4 ; i++ )
    {
        for ( j=0 ; j<4 ; j++ )
        {
            sprintf(name, "spider%d.mod", j+3);  // 3..6

            // Creates the right leg.
            rank = m_engine->CreateObject();
            m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
            m_object->SetObjectRank(3+i*4+j, rank);
            if ( j == 0 )  parent = 0;
            else           parent = 3+i*4+j-1;
            m_object->SetObjectParent(3+i*4+j, parent);
            modelManager->AddModelReference(name, false, rank);
            pos.x = table[i*12+j*3+0];
            pos.y = table[i*12+j*3+1];
            pos.z = table[i*12+j*3+2];
            m_object->SetPartPosition(3+i*4+j, pos);

            // Creates the left leg.
            rank = m_engine->CreateObject();
            m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
            m_object->SetObjectRank(19+i*4+j, rank);
            if ( j == 0 )  parent = 0;
            else           parent = 19+i*4+j-1;
            m_object->SetObjectParent(19+i*4+j, parent);
            modelManager->AddModelReference(name, true, rank);
            pos.x =  table[i*12+j*3+0];
            pos.y =  table[i*12+j*3+1];
            pos.z = -table[i*12+j*3+2];
            m_object->SetPartPosition(19+i*4+j, pos);
        }
    }

    // Creates the right mandible.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(35, rank);
    m_object->SetObjectParent(35, 1);
    modelManager->AddModelReference("spider7.mod", false, rank);
    m_object->SetPartPosition(35, Math::Vector(0.0f, 0.0f, -0.3f));

    // Creates the left mandible.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(36, rank);
    m_object->SetObjectParent(36, 1);
    modelManager->AddModelReference("spider7.mod", true, rank);
    m_object->SetPartPosition(36, Math::Vector(0.0f, 0.0f, 0.3f));

    m_object->CreateShadowCircle(4.0f, 0.5f);

    CreatePhysics();
    m_object->SetFloorHeight(0.0f);

    pos = m_object->GetPosition();
    m_object->SetPosition(pos);  // to display the shadows immediately

    m_engine->LoadAllTextures();
}

// Creates the physics of the object.

void CMotionSpider::CreatePhysics()
{
    Character*  character;
    int         i;

    int member_march[] =
    {
    //  x1,y1,z1,   x2,y2,z2,   x3,y3,z3,   x4,y4,z4,   // in the air:
        60,25,0,    60,0,0,     60,-25,0,   60,-50,0,   // t0: thighs 1..4
        -35,40,0,   -35,0,0,    -35,0,0,    -35,-40,0,  // t0: legs 1..4
        -65,0,-30,  -65,0,0,    -65,0,0,    -65,0,30,   // t0: feet 1..4
        25,0,0,     25,0,0,     25,0,0,     25,0,0,     // t0: fingers 1..4
                                                        // on the ground:
        30,15,0,    30,-10,0,   30,-35,0,   30,-60,0,   // t1: thighs 1..4
        -10,40,0,   -45,0,0,    -45,0,0,    -45,-40,0,  // t1: legs 1..4
        -90,0,0,    -20,0,0,    -20,0,0,    -20,0,0,    // t1: feet 1..4
        -5,0,0,     -5,0,0,     -5,0,0,     -5,0,0,     // t1: fingers 1..4
                                                        // on the ground back:
        35,35,0,    40,10,0,    40,-15,0,   40,-40,0,   // t2: thighs 1..4
        -35,40,0,   -35,0,0,    -35,0,0,    -25,-40,0,  // t2: legs 1..4
        -50,-25,-30,    -65,0,0,    -65,0,0,    -90,0,30,   // t2: feet 1..4
        -5,0,0,     -5,0,0,     -5,0,0,     -5,0,0,     // t2: fingers 1..4
    };

    int member_stop[] =
    {
    //  x1,y1,z1,   x2,y2,z2,   x3,y3,z3,   x4,y4,z4,   // in the air:
        35,35,0,    30,0,0,     30,-25,0,   30,-50,0,   // t0: thighs 1..4
        -35,40,0,   -45,0,0,    -45,0,0,    -45,-40,0,  // t0: legs 1..4
        -50,-25,-30,    -20,0,0,    -20,0,0,    -20,0,30,   // t0: feet 1..4
        -5,0,0,     -5,0,0,     -5,0,0,     -5,0,0,     // t0: fingers 1..4
                                                        //  on the ground:
        35,35,0,    30,0,0,     30,-25,0,   30,-50,0,   // t1: thighs 1..4
        -30,40,0,   -40,0,0,    -40,0,0,    -40,-40,0,  // t1: legs 1..4
        -55,-25,-30,    -25,0,0,    -25,0,0,    -25,0,0,    // t1: feet 1..4
        -5,0,0,     -5,0,0,     -5,0,0,     -5,0,0,     // t1: fingers 1..4
                                                        // on the ground back:
        35,35,0,    30,0,0,     30,-25,0,   30,-50,0,   // t2: thighs 1..4
        -30,40,0,   -40,0,0,    -40,0,0,    -40,-40,0,  // t2: legs 1..4
        -50,-25,-30,    -20,0,0,    -20,0,0,    -20,0,30,   // t2: feet 1..4
        -10,0,0,    -10,0,0,    -10,0,0,    -10,0,0,    // t2: fingers 1..4
    };

    int member_spec[] =
    {
    //  x1,y1,z1,   x2,y2,z2,   x3,y3,z3,   x4,y4,z4,   // burning:
        30,25,0,    30,0,0,     30,-25,0,   30,-50,0,   // s0: thighs 1..4
        -45,0,0,    -45,0,0,    -45,0,0,    -45,0,0,    // s0: legs 1..4
        -20,0,0,    -20,0,0,    -20,0,0,    -20,0,0,    // s0: feet 1..4
        -5,0,0,     -5,0,0,     -5,0,0,     -5,0,0,     // s0: fingers 1..4
                                                        // destroyed:
        30,25,0,    30,0,0,     30,-25,0,   30,-50,0,   // s1: thighs 1..4
        -45,0,0,    -45,0,0,    -45,0,0,    -45,0,0,    // s1: legs 1..4
        -20,0,0,    -20,0,0,    -20,0,0,    -20,0,0,    // s1: feet 1..4
        -5,0,0,     -5,0,0,     -5,0,0,     -5,0,0,     // s1: fingers 1..4
                                                        // explodes:
        40,25,0,    40,0,0,     40,-25,0,   40,-50,0,   // s2: thighs 1..4
        -55,0,0,    -55,0,0,    -55,0,0,    -55,0,0,    // s2: legs 1..4
        -30,0,0,    -30,0,0,    -30,0,0,    -30,0,0,    // s2: feet 1..4
        -5,0,0,     -5,0,0,     -5,0,0,     -5,0,0,     // s2: fingers 1..4
                                                        // back1 :
        35,35,0,    30,0,0,     30,-25,0,   30,-50,0,   // s3: thighs 1..4
        -30,40,0,   -40,0,0,    -40,0,0,    -40,-40,0,  // s3: legs 1..4
        -55,-25,-30,    -25,0,0,    -25,0,0,    -25,0,0,    // s3: feet 1..4
        -5,0,0,     -5,0,0,     -5,0,0,     -5,0,0,     // s3: fingers 1..4
                                                        // back2 :
        15,35,0,    15,0,0,     15,-25,0,   15,-50,0,   // s4: thighs 1..4
        -60,40,0,   -60,0,0,    -60,0,0,    -60,-40,0,  // s4: legs 1..4
        -65,-25,-30,    -65,0,0,    -65,0,0,    -65,0,0,    // s4: feet 1..4
        -15,0,0,    -15,0,0,    -15,0,0,    -15,0,0,    // s4: fingers 1..4
                                                        // back3 :
        35,35,0,    30,0,0,     30,-25,0,   30,-50,0,   // s5: thighs 1..4
        -30,40,0,   -40,0,0,    -40,0,0,    -40,-40,0,  // s5: legs 1..4
        -55,-25,-30,    -25,0,0,    -25,0,0,    -25,0,0,    // s5: feet 1..4
        -5,0,0,     -5,0,0,     -5,0,0,     -5,0,0,     // s5: fingers 1..4
    };

    character = m_object->GetCharacter();
    character->wheelFront = 4.0f;
    character->wheelBack  = 4.0f;
    character->wheelLeft  = 6.0f;
    character->wheelRight = 6.0f;
    character->height     = 0.6f;

    m_physics->SetLinMotionX(MO_ADVSPEED,  12.0f);
    m_physics->SetLinMotionX(MO_RECSPEED,  12.0f);
    m_physics->SetLinMotionX(MO_ADVACCEL,  15.0f);
    m_physics->SetLinMotionX(MO_RECACCEL,  15.0f);
    m_physics->SetLinMotionX(MO_STOACCEL,  40.0f);
    m_physics->SetLinMotionX(MO_TERSLIDE,   5.0f);
    m_physics->SetLinMotionZ(MO_TERSLIDE,   5.0f);
    m_physics->SetLinMotionX(MO_TERFORCE,   5.0f);
    m_physics->SetLinMotionZ(MO_TERFORCE,   5.0f);
    m_physics->SetLinMotionZ(MO_MOTACCEL,  10.0f);

    m_physics->SetCirMotionY(MO_ADVSPEED,   1.0f*Math::PI);
    m_physics->SetCirMotionY(MO_RECSPEED,   1.0f*Math::PI);
    m_physics->SetCirMotionY(MO_ADVACCEL,  20.0f);
    m_physics->SetCirMotionY(MO_RECACCEL,  20.0f);
    m_physics->SetCirMotionY(MO_STOACCEL,  40.0f);

    for ( i=0 ; i<3*4*4*3 ; i++ )
    {
        m_armAngles[3*4*4*3*MS_MARCH+i] = member_march[i];
    }
    for ( i=0 ; i<3*4*4*3 ; i++ )
    {
        m_armAngles[3*4*4*3*MS_STOP+i] = member_stop[i];
    }
    for ( i=0 ; i<3*4*4*6 ; i++ )
    {
        m_armAngles[3*4*4*3*MS_SPEC+i] = member_spec[i];
    }
}


// Management of an event.

bool CMotionSpider::EventProcess(const Event &event)
{
    CMotion::EventProcess(event);

    if ( event.type == EVENT_FRAME )
    {
        return EventFrame(event);
    }

    return true;
}

// Management of an event.

bool CMotionSpider::EventFrame(const Event &event)
{
    Math::Vector    dir, pos, speed;
    Math::Point     dim;
    float       s, a, prog = 0.0f, time;
    float       tSt[12], tNd[12];
    int         i, ii, st, nd, action;
    bool        bStop;

    if ( m_engine->GetPause() )  return true;
    if ( !m_engine->IsVisiblePoint(m_object->GetPosition()) )  return true;

    s =     m_physics->GetLinMotionX(MO_MOTSPEED)*1.5f;
    a = fabs(m_physics->GetCirMotionY(MO_MOTSPEED)*2.0f);

    if ( s == 0.0f && a != 0.0f )  a *= 1.5f;

    m_armTimeAbs += event.rTime;
    m_armTimeAction += event.rTime;
    m_armMember += (s+a)*event.rTime*0.15f;

    bStop = ( a == 0.0f && s == 0.0f );  // stop?

    action = MS_MARCH;  // waslking
    if ( s == 0.0f && a == 0.0f )
    {
        action = MS_STOP;  // stop
    }

    if ( bStop )
    {
        prog = Math::Mod(m_armTimeAbs, 2.0f)/10.0f;
        a = Math::Mod(m_armMember, 1.0f);
        a = (prog-a)*event.rTime*2.0f;  // stop position just pleasantly
        m_armMember += a;
    }

    assert(m_object->Implements(ObjectInterfaceType::Destroyable));
    if (dynamic_cast<CDestroyableObject*>(m_object)->GetDying() == DeathType::Burning )  // burning?
    {
        if ( dynamic_cast<CBaseAlien*>(m_object)->GetFixed() )
        {
            m_actionType = MSS_BURN;
        }
        else
        {
            m_actionType = -1;
        }
    }
    else if ( dynamic_cast<CDestroyableObject*>(m_object)->IsDying() )  // destroyed?
    {
        m_actionType = MSS_RUIN;
    }

    for ( i=0 ; i<8 ; i++ )  // the 8 legs
    {
        if ( m_actionType != -1 )  // special action in progress?
        {
            st = 3*4*4*3*MS_SPEC + 3*4*4*m_actionType + (i%4)*3;
            nd = st;
            time = event.rTime*m_actionTime;
            m_armTimeAction = 0.0f;
        }
        else
        {
//?         if ( i < 4 )  prog = Math::Mod(m_armMember+(2.0f-(i%4))*0.25f+0.0f, 1.0f);
//?         else          prog = Math::Mod(m_armMember+(2.0f-(i%4))*0.25f+0.3f, 1.0f);
            if ( i < 4 )  prog = Math::Mod(m_armMember+(2.0f-(i%4))*0.25f+0.0f, 1.0f);
            else          prog = Math::Mod(m_armMember+(2.0f-(i%4))*0.25f+0.5f, 1.0f);
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
            st = 3*4*4*3*action + st*3*4*4 + (i%4)*3;
            nd = 3*4*4*3*action + nd*3*4*4 + (i%4)*3;

            // Less and less soft ...
//?         time = event.rTime*(2.0f+Math::Min(m_armTimeAction*20.0f, 40.0f));
            time = event.rTime*10.0f;
        }

        tSt[ 0] = m_armAngles[st+ 0];  // x
        tSt[ 1] = m_armAngles[st+ 1];  // y
        tSt[ 2] = m_armAngles[st+ 2];  // z
        tSt[ 3] = m_armAngles[st+12];  // x
        tSt[ 4] = m_armAngles[st+13];  // y
        tSt[ 5] = m_armAngles[st+14];  // z
        tSt[ 6] = m_armAngles[st+24];  // x
        tSt[ 7] = m_armAngles[st+25];  // y
        tSt[ 8] = m_armAngles[st+26];  // z
        tSt[ 9] = m_armAngles[st+36];  // x
        tSt[10] = m_armAngles[st+37];  // y
        tSt[11] = m_armAngles[st+38];  // z

        tNd[ 0] = m_armAngles[nd+ 0];  // x
        tNd[ 1] = m_armAngles[nd+ 1];  // y
        tNd[ 2] = m_armAngles[nd+ 2];  // z
        tNd[ 3] = m_armAngles[nd+12];  // x
        tNd[ 4] = m_armAngles[nd+13];  // y
        tNd[ 5] = m_armAngles[nd+14];  // z
        tNd[ 6] = m_armAngles[nd+24];  // x
        tNd[ 7] = m_armAngles[nd+25];  // y
        tNd[ 8] = m_armAngles[nd+26];  // z
        tNd[ 9] = m_armAngles[nd+36];  // z
        tNd[10] = m_armAngles[nd+37];  // z
        tNd[11] = m_armAngles[nd+38];  // z

        if ( m_actionType == MSS_BACK2 )   // on the back?
        {
            for ( ii=0 ; ii<12 ; ii++ )
            {
                tSt[ii] += Math::Rand()*20.0f;
                tNd[ii] = tSt[ii];
            }
//?         time = 100.0f;
            time = event.rTime*10.0f;
        }

        if ( i < 4 )  // right leg (1..4) ?
        {
            m_object->SetPartRotationX(3+4*i+0, Math::Smooth(m_object->GetPartRotationX(3+4*i+0), Math::PropAngle(tSt[ 0], tNd[ 0], prog), time));
            m_object->SetPartRotationY(3+4*i+0, Math::Smooth(m_object->GetPartRotationY(3+4*i+0), Math::PropAngle(tSt[ 1], tNd[ 1], prog), time));
            m_object->SetPartRotationZ(3+4*i+0, Math::Smooth(m_object->GetPartRotationZ(3+4*i+0), Math::PropAngle(tSt[ 2], tNd[ 2], prog), time));
            m_object->SetPartRotationX(3+4*i+1, Math::Smooth(m_object->GetPartRotationX(3+4*i+1), Math::PropAngle(tSt[ 3], tNd[ 3], prog), time));
            m_object->SetPartRotationY(3+4*i+1, Math::Smooth(m_object->GetPartRotationY(3+4*i+1), Math::PropAngle(tSt[ 4], tNd[ 4], prog), time));
            m_object->SetPartRotationZ(3+4*i+1, Math::Smooth(m_object->GetPartRotationZ(3+4*i+1), Math::PropAngle(tSt[ 5], tNd[ 5], prog), time));
            m_object->SetPartRotationX(3+4*i+2, Math::Smooth(m_object->GetPartRotationX(3+4*i+2), Math::PropAngle(tSt[ 6], tNd[ 6], prog), time));
            m_object->SetPartRotationY(3+4*i+2, Math::Smooth(m_object->GetPartRotationY(3+4*i+2), Math::PropAngle(tSt[ 7], tNd[ 7], prog), time));
            m_object->SetPartRotationZ(3+4*i+2, Math::Smooth(m_object->GetPartRotationZ(3+4*i+2), Math::PropAngle(tSt[ 8], tNd[ 8], prog), time));
            m_object->SetPartRotationX(3+4*i+3, Math::Smooth(m_object->GetPartRotationX(3+4*i+3), Math::PropAngle(tSt[ 9], tNd[ 9], prog), time));
            m_object->SetPartRotationY(3+4*i+3, Math::Smooth(m_object->GetPartRotationY(3+4*i+3), Math::PropAngle(tSt[10], tNd[10], prog), time));
            m_object->SetPartRotationZ(3+4*i+3, Math::Smooth(m_object->GetPartRotationZ(3+4*i+3), Math::PropAngle(tSt[11], tNd[11], prog), time));
        }
        else    // left leg (5..8) ?
        {
            m_object->SetPartRotationX(3+4*i+0, Math::Smooth(m_object->GetPartRotationX(3+4*i+0), Math::PropAngle(-tSt[ 0], -tNd[ 0], prog), time));
            m_object->SetPartRotationY(3+4*i+0, Math::Smooth(m_object->GetPartRotationY(3+4*i+0), Math::PropAngle(-tSt[ 1], -tNd[ 1], prog), time));
            m_object->SetPartRotationZ(3+4*i+0, Math::Smooth(m_object->GetPartRotationZ(3+4*i+0), Math::PropAngle( tSt[ 2],  tNd[ 2], prog), time));
            m_object->SetPartRotationX(3+4*i+1, Math::Smooth(m_object->GetPartRotationX(3+4*i+1), Math::PropAngle(-tSt[ 3], -tNd[ 3], prog), time));
            m_object->SetPartRotationY(3+4*i+1, Math::Smooth(m_object->GetPartRotationY(3+4*i+1), Math::PropAngle(-tSt[ 4], -tNd[ 4], prog), time));
            m_object->SetPartRotationZ(3+4*i+1, Math::Smooth(m_object->GetPartRotationZ(3+4*i+1), Math::PropAngle( tSt[ 5],  tNd[ 5], prog), time));
            m_object->SetPartRotationX(3+4*i+2, Math::Smooth(m_object->GetPartRotationX(3+4*i+2), Math::PropAngle(-tSt[ 6], -tNd[ 6], prog), time));
            m_object->SetPartRotationY(3+4*i+2, Math::Smooth(m_object->GetPartRotationY(3+4*i+2), Math::PropAngle(-tSt[ 7], -tNd[ 7], prog), time));
            m_object->SetPartRotationZ(3+4*i+2, Math::Smooth(m_object->GetPartRotationZ(3+4*i+2), Math::PropAngle( tSt[ 8],  tNd[ 8], prog), time));
            m_object->SetPartRotationX(3+4*i+3, Math::Smooth(m_object->GetPartRotationX(3+4*i+3), Math::PropAngle(-tSt[ 9], -tNd[ 9], prog), time));
            m_object->SetPartRotationY(3+4*i+3, Math::Smooth(m_object->GetPartRotationY(3+4*i+3), Math::PropAngle(-tSt[10], -tNd[10], prog), time));
            m_object->SetPartRotationZ(3+4*i+3, Math::Smooth(m_object->GetPartRotationZ(3+4*i+3), Math::PropAngle( tSt[11],  tNd[11], prog), time));
        }
    }

    if ( m_actionType == MSS_BURN )  // burning?
    {
        dir = Math::Vector(Math::PI, 0.0f, 0.0f);
        SetCirVibration(dir);
        dir = Math::Vector(0.0f, 0.0f, 0.0f);
        SetLinVibration(dir);
        SetTilt(dir);

        time = event.rTime*1.0f;
        m_object->SetPartRotationZ(1, Math::Smooth(m_object->GetPartRotationZ(1), 0.0f, time));  // head
    }
    else if ( m_actionType == MSS_RUIN )  // destroyed?
    {
        dir = Math::Vector(0.0f, 0.0f, 0.0f);
        SetLinVibration(dir);
        SetCirVibration(dir);
        SetTilt(dir);
    }
    else if ( m_actionType == MSS_EXPLO )  // exploded?
    {
        m_object->SetPartScaleY(1, 1.0f+m_progress);
        m_object->SetPartScaleZ(1, 1.0f+m_progress);
        m_object->SetPartScaleX(1, 1.0f+m_progress/2.0f);

        dir.x = (Math::Rand()-0.5f)*0.1f*m_progress;
        dir.y = (Math::Rand()-0.5f)*0.1f*m_progress;
        dir.z = (Math::Rand()-0.5f)*0.1f*m_progress;
        m_object->SetCirVibration(dir);
    }
    else if ( m_actionType == MSS_BACK1 )  // turns on the back?
    {
        if ( m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_armTimeAbs )
        {
            m_lastParticle = m_armTimeAbs;

            pos = m_object->GetPosition();
            speed.x = (Math::Rand()-0.5f)*10.0f;
            speed.z = (Math::Rand()-0.5f)*10.0f;
            speed.y = Math::Rand()*5.0f;
            dim.x = Math::Rand()*3.0f+2.0f;
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTICRASH, 2.0f);
        }

        if ( m_progress < 0.5f )
        {
            dir.x = 0.0f;
            dir.y = powf(m_progress/0.5f, 2.0f)*12.0f;
            dir.z = 0.0f;
            SetLinVibration(dir);
        }
        else
        {
            dir.x = 0.0f;
            dir.y = powf(2.0f-m_progress/0.5f, 2.0f)*12.0f;
            dir.z = 0.0f;
            SetLinVibration(dir);
        }
        dir.x = m_progress*Math::PI;
        dir.y = 0.0f;
        dir.z = 0.0f;
        SetCirVibration(dir);

        dir = Math::Vector(0.0f, 0.0f, 0.0f);
        SetTilt(dir);

        if ( m_progress >= 1.0f )
        {
            SetAction(MSS_BACK2, 55.0f+Math::Rand()*10.0f);
        }
    }
    else if ( m_actionType == MSS_BACK2 )  // moves on the back?
    {
        if ( m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_armTimeAbs )
        {
            m_lastParticle = m_armTimeAbs;

            if ( rand()%10 == 0 )
            {
                pos = m_object->GetPosition();
                pos.x += (Math::Rand()-0.5f)*8.0f;
                pos.z += (Math::Rand()-0.5f)*8.0f;
                pos.y -= 1.0f;
                speed.x = (Math::Rand()-0.5f)*2.0f;
                speed.z = (Math::Rand()-0.5f)*2.0f;
                speed.y = Math::Rand()*2.0f;
                dim.x = Math::Rand()*1.0f+1.0f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTICRASH, 2.0f);
            }
        }

        dir = Math::Vector(0.0f, 0.0f, 0.0f);
        SetLinVibration(dir);
        dir.x = sinf(m_armTimeAbs* 3.0f)*0.20f+
                sinf(m_armTimeAbs* 6.0f)*0.20f+
                sinf(m_armTimeAbs*10.0f)*0.20f+
                sinf(m_armTimeAbs*17.0f)*0.30f+Math::PI;
        dir.y = sinf(m_armTimeAbs* 4.0f)*0.02f+
                sinf(m_armTimeAbs* 5.0f)*0.02f+
                sinf(m_armTimeAbs*11.0f)*0.02f+
                sinf(m_armTimeAbs*18.0f)*0.03f;
        dir.z = sinf(m_armTimeAbs* 2.0f)*0.02f+
                sinf(m_armTimeAbs* 7.0f)*0.02f+
                sinf(m_armTimeAbs*13.0f)*0.02f+
                sinf(m_armTimeAbs*15.0f)*0.03f;
        SetCirVibration(dir);
        dir = Math::Vector(0.0f, 0.0f, 0.0f);
        SetTilt(dir);

        m_object->SetPartRotationY(1, sinf(m_armTimeAbs*5.0f)*0.05f);  // tail
        m_object->SetPartRotationY(2, cosf(m_armTimeAbs*5.0f)*0.20f);  // head
        m_object->SetPartRotationZ(1, 0.4f);  // tail
        m_object->SetPartRotationZ(2, 0.0f);  // head

        if ( m_progress >= 1.0f )
        {
            SetAction(MSS_BACK3, 0.4f);
        }
    }
    else if ( m_actionType == MSS_BACK3 )  // recovers on the legs?
    {
        if ( m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_armTimeAbs )
        {
            m_lastParticle = m_armTimeAbs;

            pos = m_object->GetPosition();
            speed.x = (Math::Rand()-0.5f)*10.0f;
            speed.z = (Math::Rand()-0.5f)*10.0f;
            speed.y = Math::Rand()*5.0f;
            dim.x = Math::Rand()*3.0f+2.0f;
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTICRASH, 2.0f);
        }

        if ( m_progress < 0.5f )
        {
            dir.x = 0.0f;
            dir.y = powf(m_progress/0.5f, 2.0f)*5.0f;
            dir.z = 0.0f;
            SetLinVibration(dir);
        }
        else
        {
            dir.x = 0.0f;
            dir.y = powf(2.0f-m_progress/0.5f, 2.0f)*5.0f;
            dir.z = 0.0f;
            SetLinVibration(dir);
        }
        dir.x = (1.0f-m_progress)*Math::PI;
        dir.y = 0.0f;
        dir.z = 0.0f;
        SetCirVibration(dir);

        dir = Math::Vector(0.0f, 0.0f, 0.0f);
        SetTilt(dir);

        if ( m_progress >= 1.0f )
        {
            SetAction(-1);
            dynamic_cast<CBaseAlien*>(m_object)->SetFixed(false);  // moving again
        }
    }
    else
    {
        if ( bStop )
        {
            dir = Math::Vector(0.0f, 0.0f, 0.0f);
            SetTilt(dir);
        }
        else
        {
            a = Math::Mod(m_armMember, 1.0f);
            if ( a < 0.5f )  a = -1.0f+4.0f*a;  // -1..1
            else             a =  3.0f-4.0f*a;  // 1..-1
            dir.x = sinf(a)*0.05f;

            s = Math::Mod(m_armMember/2.0f, 1.0f);
            if ( s < 0.5f )  s = -1.0f+4.0f*s;  // -1..1
            else             s =  3.0f-4.0f*s;  // 1..-1
            dir.z = sinf(s)*0.1f;

            dir.y = 0.0f;
            SetTilt(dir);
        }

        dir = Math::Vector(0.0f, 0.0f, 0.0f);
        SetLinVibration(dir);
        SetCirVibration(dir);

        m_object->SetPartRotationZ(1, sinf(m_armTimeAbs*1.7f)*0.02f);  // tail
        m_object->SetPartRotationX(1, sinf(m_armTimeAbs*1.3f)*0.05f);
        m_object->SetPartRotationY(1, sinf(m_armTimeAbs*2.4f)*0.10f);
        m_object->SetPartScale(1, 1.0f+sinf(m_armTimeAbs*3.3f)*0.05f);

        m_object->SetPartRotationZ(2, sinf(m_armTimeAbs*1.4f)*0.20f);  // head
        m_object->SetPartRotationX(2, sinf(m_armTimeAbs*1.9f)*0.10f);
        m_object->SetPartRotationY(2, sinf(m_armTimeAbs*2.1f)*0.10f);

        m_object->SetPartRotationY(35,  sinf(m_armTimeAbs*3.1f)*0.20f);  // mandible
        m_object->SetPartRotationY(36, -sinf(m_armTimeAbs*3.1f)*0.20f);  // mandible
    }

    return true;
}
