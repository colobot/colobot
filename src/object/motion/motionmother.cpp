// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.


#include "object/motion/motionmother.h"

#include "app/app.h"

#include "graphics/engine/modelmanager.h"

#include "physics/physics.h"


#include <stdio.h>


#define ADJUST_ANGLE        false       // true -> adjusts the angles of the members
const float START_TIME = 1000.0f;       // beginning of the relative time



// Object's constructor.

CMotionMother::CMotionMother(CObject* object) : CMotion(object)
{
    m_armMember      = START_TIME;
    m_armTimeAbs     = START_TIME;
    m_armTimeMarch   = START_TIME;
    m_armTimeAction  = START_TIME;
    m_armTimeIndex   = 0;
    m_armPartIndex   = 0;
    m_armMemberIndex = 0;
    m_armLastAction  = -1;
    m_specAction     = -1;
    m_bArmStop = false;
}

// Object's destructor.

CMotionMother::~CMotionMother()
{
}


// Removes an object.

void CMotionMother::DeleteObject(bool bAll)
{
}


// Creates a vehicle traveling any lands on the ground.

bool CMotionMother::Create(Math::Vector pos, float angle, ObjectType type,
                          float power)
{
    Gfx::CModelManager* modelManager = Gfx::CModelManager::GetInstancePointer();
    int rank;

    m_object->SetType(type);

    // Creates main base.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_VEHICLE);  // this is a moving object
    m_object->SetObjectRank(0, rank);
    modelManager->AddModelReference("mother1.mod", false, rank);
    m_object->SetPosition(0, pos);
    m_object->SetAngleY(0, angle);

    // A vehicle must have a obligatory collision
    //with a sphere of center (0, y, 0) (see GetCrashSphere).
    m_object->CreateCrashSphere(Math::Vector(0.0f, 0.0f, 0.0f), 20.0f, SOUND_BOUM, 0.20f);
    m_object->SetGlobalSphere(Math::Vector(-2.0f, 10.0f, 0.0f), 25.0f);

    // Creates the head.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(1, rank);
    m_object->SetObjectParent(1, 0);
    modelManager->AddModelReference("mother2.mod", false, rank);
    m_object->SetPosition(1, Math::Vector(16.0f, 3.0f, 0.0f));

    // Creates a right-back leg.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(2, rank);
    m_object->SetObjectParent(2, 0);
    modelManager->AddModelReference("mother3.mod", false, rank);
    m_object->SetPosition(2, Math::Vector(-5.0f, -1.0f, -12.0f));

    // Creates a right-back foot.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(3, rank);
    m_object->SetObjectParent(3, 2);
    modelManager->AddModelReference("mother4.mod", false, rank);
    m_object->SetPosition(3, Math::Vector(0.0f, 0.0f, -8.5f));

    // Creates a middle-right leg.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(4, rank);
    m_object->SetObjectParent(4, 0);
    modelManager->AddModelReference("mother3.mod", false, rank);
    m_object->SetPosition(4, Math::Vector(3.5f, -1.0f, -12.0f));

    // Creates a middle-right foot.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(5, rank);
    m_object->SetObjectParent(5, 4);
    modelManager->AddModelReference("mother4.mod", false, rank);
    m_object->SetPosition(5, Math::Vector(0.0f, 0.0f, -8.5f));

    // Creates a right-front leg.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(6, rank);
    m_object->SetObjectParent(6, 0);
    modelManager->AddModelReference("mother3.mod", false, rank);
    m_object->SetPosition(6, Math::Vector(10.0f, -1.0f, -10.0f));

    // Creates a right-front foot.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(7, rank);
    m_object->SetObjectParent(7, 6);
    modelManager->AddModelReference("mother4.mod", false, rank);
    m_object->SetPosition(7, Math::Vector(0.0f, 0.0f, -8.5f));

    // Creates a left-back leg.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(8, rank);
    m_object->SetObjectParent(8, 0);
    modelManager->AddModelReference("mother3.mod", false, rank);
    m_object->SetPosition(8, Math::Vector(-5.0f, -1.0f, 12.0f));
    m_object->SetAngleY(8, Math::PI);

    // Creates a left-back foot.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(9, rank);
    m_object->SetObjectParent(9, 8);
    modelManager->AddModelReference("mother4.mod", false, rank);
    m_object->SetPosition(9, Math::Vector(0.0f, 0.0f, -8.5f));

    // Creates a middle-left leg.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(10, rank);
    m_object->SetObjectParent(10, 0);
    modelManager->AddModelReference("mother3.mod", false, rank);
    m_object->SetPosition(10, Math::Vector(3.5f, -1.0f, 12.0f));
    m_object->SetAngleY(10, Math::PI);

    // Creates a middle-left foot.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(11, rank);
    m_object->SetObjectParent(11, 10);
    modelManager->AddModelReference("mother4.mod", false, rank);
    m_object->SetPosition(11, Math::Vector(0.0f, 0.0f, -8.5f));

    // Creates a left-front leg.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(12, rank);
    m_object->SetObjectParent(12, 0);
    modelManager->AddModelReference("mother3.mod", false, rank);
    m_object->SetPosition(12, Math::Vector(10.0f, -1.0f, 10.0f));
    m_object->SetAngleY(12, Math::PI);

    // Creates a left-front foot.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(13, rank);
    m_object->SetObjectParent(13, 12);
    modelManager->AddModelReference("mother4.mod", false, rank);
    m_object->SetPosition(13, Math::Vector(0.0f, 0.0f, -8.5f));

    // Creates the right antenna.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(14, rank);
    m_object->SetObjectParent(14, 1);
    modelManager->AddModelReference("mother5.mod", false, rank);
    m_object->SetPosition(14, Math::Vector(6.0f, 1.0f, -2.5f));

    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(15, rank);
    m_object->SetObjectParent(15, 14);
    modelManager->AddModelReference("mother6.mod", false, rank);
    m_object->SetPosition(15, Math::Vector(8.0f, 0.0f, 0.0f));

    // Creates the left antenna.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(16, rank);
    m_object->SetObjectParent(16, 1);
    modelManager->AddModelReference("mother5.mod", false, rank);
    m_object->SetPosition(16, Math::Vector(6.0f, 1.0f, 2.5f));

    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(17, rank);
    m_object->SetObjectParent(17, 16);
    modelManager->AddModelReference("mother6.mod", false, rank);
    m_object->SetPosition(17, Math::Vector(8.0f, 0.0f, 0.0f));

    // Creates the right claw.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(18, rank);
    m_object->SetObjectParent(18, 1);
    modelManager->AddModelReference("mother7.mod", false, rank);
    m_object->SetPosition(18, Math::Vector(-4.0f, -3.5f, -8.0f));
    m_object->SetZoomX(18, 1.2f);

    // Creates the left claw.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(19, rank);
    m_object->SetObjectParent(19, 1);
    modelManager->AddModelReference("mother7.mod", true, rank);
    m_object->SetPosition(19, Math::Vector(-4.0f, -3.5f, 8.0f));
    m_object->SetZoomX(19, 1.2f);

    m_object->CreateShadowCircle(18.0f, 0.8f);

    CreatePhysics();
    m_object->SetFloorHeight(0.0f);

    pos = m_object->GetPosition(0);
    m_object->SetPosition(0, pos);  // to display the shadows immediately

    m_engine->LoadAllTextures();

    return true;
}

// Creates the physics of the object.

void CMotionMother::CreatePhysics()
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

    m_physics->SetType(TYPE_ROLLING);

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

bool CMotionMother::EventProcess(const Event &event)
{
    CMotion::EventProcess(event);

    if ( event.type == EVENT_FRAME )
    {
        return EventFrame(event);
    }

    if ( event.type == EVENT_KEY_DOWN )
    {
#if ADJUST_ANGLE
        int     i;

        if ( event.param == 'A' )  m_armTimeIndex++;
        if ( m_armTimeIndex >= 3 )  m_armTimeIndex = 0;

        if ( event.param == 'Q' )  m_armPartIndex++;
        if ( m_armPartIndex >= 3 )  m_armPartIndex = 0;

        if ( event.param == 'W' )  m_armMemberIndex++;
        if ( m_armMemberIndex >= 3 )  m_armMemberIndex = 0;

        i  = m_armMemberIndex*3;
        i += m_armPartIndex*3*3;
        i += m_armTimeIndex*3*3*3;
//?     i += 3*3*3*3;

        if ( event.param == 'E' )  m_armAngles[i+0] += 5;
        if ( event.param == 'D' )  m_armAngles[i+0] -= 5;
        if ( event.param == 'R' )  m_armAngles[i+1] += 5;
        if ( event.param == 'F' )  m_armAngles[i+1] -= 5;
        if ( event.param == 'T' )  m_armAngles[i+2] += 5;
        if ( event.param == 'G' )  m_armAngles[i+2] -= 5;

        if ( event.param == 'Y' )  m_bArmStop = !m_bArmStop;
#endif
    }

    return true;
}

// Management of an event.

bool CMotionMother::EventFrame(const Event &event)
{
    Math::Vector    dir;
    float       s, a, prog;
    int         i, st, nd;
    bool        bStop;

    if ( m_engine->GetPause() )  return true;
    if ( !m_engine->IsVisiblePoint(m_object->GetPosition(0)) )  return true;

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
        if ( m_bArmStop )
        {
            prog = static_cast< float >(m_armTimeIndex/3.0f);
        }
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
            m_object->SetAngleX(2+2*i+0, Math::PropAngle(m_armAngles[st+ 0], m_armAngles[nd+ 0], prog));
            m_object->SetAngleY(2+2*i+0, Math::PropAngle(m_armAngles[st+ 1], m_armAngles[nd+ 1], prog));
            m_object->SetAngleZ(2+2*i+0, Math::PropAngle(m_armAngles[st+ 2], m_armAngles[nd+ 2], prog));
            m_object->SetAngleX(2+2*i+1, Math::PropAngle(m_armAngles[st+ 9], m_armAngles[nd+ 9], prog));
            m_object->SetAngleY(2+2*i+1, Math::PropAngle(m_armAngles[st+10], m_armAngles[nd+10], prog));
            m_object->SetAngleZ(2+2*i+1, Math::PropAngle(m_armAngles[st+11], m_armAngles[nd+11], prog));
        }
        else    // left leg (4..6) ?
        {
            m_object->SetAngleX(2+2*i+0, Math::PropAngle(    m_armAngles[st+ 0],     m_armAngles[nd+ 0], prog));
            m_object->SetAngleY(2+2*i+0, Math::PropAngle(180-m_armAngles[st+ 1], 180-m_armAngles[nd+ 1], prog));
            m_object->SetAngleZ(2+2*i+0, Math::PropAngle(   -m_armAngles[st+ 2],    -m_armAngles[nd+ 2], prog));
            m_object->SetAngleX(2+2*i+1, Math::PropAngle(    m_armAngles[st+ 9],     m_armAngles[nd+ 9], prog));
            m_object->SetAngleY(2+2*i+1, Math::PropAngle(   -m_armAngles[st+10],    -m_armAngles[nd+10], prog));
            m_object->SetAngleZ(2+2*i+1, Math::PropAngle(   -m_armAngles[st+11],    -m_armAngles[nd+11], prog));
        }
    }

#if ADJUST_ANGLE
    if ( m_object->GetSelect() )
    {
        char s[100];
        sprintf(s, "A:time=%d Q:part=%d W:member=%d", m_armTimeIndex, m_armPartIndex, m_armMemberIndex);
        m_engine->SetInfoText(4, s);
    }
#endif

    if ( !bStop && !m_object->GetRuin() )
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
        m_object->SetInclinaison(dir);

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

    m_object->SetAngleZ(1, sinf(m_armTimeAbs*0.5f)*0.20f);  // head
    m_object->SetAngleX(1, sinf(m_armTimeAbs*0.6f)*0.10f);  // head
    m_object->SetAngleY(1, sinf(m_armTimeAbs*0.7f)*0.20f);  // head

    m_object->SetAngleZ(14,  0.50f);
    m_object->SetAngleZ(16,  0.50f);
    m_object->SetAngleY(14,  0.80f+sinf(m_armTimeAbs*1.1f)*0.53f);  // right antenna
    m_object->SetAngleY(15,  0.70f-sinf(m_armTimeAbs*1.7f)*0.43f);
    m_object->SetAngleY(16, -0.80f+sinf(m_armTimeAbs*0.9f)*0.53f);  // left antenna
    m_object->SetAngleY(17, -0.70f-sinf(m_armTimeAbs*1.3f)*0.43f);

    m_object->SetAngleY(18, sinf(m_armTimeAbs*1.1f)*0.20f);  // right claw
    m_object->SetAngleZ(18, -0.20f);
    m_object->SetAngleY(19, sinf(m_armTimeAbs*0.9f)*0.20f);  // left claw
    m_object->SetAngleZ(19, -0.20f);

    return true;
}

