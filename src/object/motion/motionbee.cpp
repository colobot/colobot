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


#include "object/motion/motionbee.h"

#include "app/app.h"

#include "graphics/engine/modelmanager.h"

#include "physics/physics.h"


#include <stdio.h>


#define ADJUST_ANGLE        false       // true -> adjusts the angles of the members
const float START_TIME = 1000.0f;       // beginning of the relative time



// Object's constructor.

CMotionBee::CMotionBee(CInstanceManager* iMan, CObject* object)
                      : CMotion(iMan, object)
{
    m_armMember      = START_TIME;
    m_armTimeAbs     = START_TIME;
    m_armTimeMarch   = START_TIME;
    m_armTimeAction  = START_TIME;
    m_armTimeIndex   = 0;
    m_armPartIndex   = 0;
    m_armMemberIndex = 0;
    m_armLastAction  = -1;
    m_bArmStop = false;
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

bool CMotionBee::Create(Math::Vector pos, float angle, ObjectType type,
                        float power)
{
    Gfx::CModelManager* modelManager = Gfx::CModelManager::GetInstancePointer();
    int rank;

    m_object->SetType(type);

    // Creates main base.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_VEHICULE);  // this is a moving object
    m_object->SetObjectRank(0, rank);
    modelManager->AddModelReference("bee1.mod", false, rank);
    m_object->SetPosition(0, pos);
    m_object->SetAngleY(0, angle);

    // A vehicle must have an obligatory collision
    // with a sphere of center (0, y, 0) (see GetCrashSphere).
    m_object->CreateCrashSphere(Math::Vector(0.0f, 0.0f, 0.0f), 4.0f, SOUND_BOUM, 0.20f);
    m_object->SetGlobalSphere(Math::Vector(-1.0f, 1.0f, 0.0f), 5.0f);

    // Creates the head.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(1, rank);
    m_object->SetObjectParent(1, 0);
    modelManager->AddModelReference("bee2.mod", false, rank);
    m_object->SetPosition(1, Math::Vector(1.6f, 0.3f, 0.0f));

    // Creates the tail.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(2, rank);
    m_object->SetObjectParent(2, 0);
    modelManager->AddModelReference("bee3.mod", false, rank);
    m_object->SetPosition(2, Math::Vector(-0.8f, 0.0f, 0.0f));

    // Creates a right-back thigh.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(3, rank);
    m_object->SetObjectParent(3, 0);
    modelManager->AddModelReference("ant4.mod", false, rank);
    m_object->SetPosition(3, Math::Vector(-0.3f, -0.1f, -0.2f));

    // Creates a right-back leg.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(4, rank);
    m_object->SetObjectParent(4, 3);
    modelManager->AddModelReference("ant5.mod", false, rank);
    m_object->SetPosition(4, Math::Vector(0.0f, 0.0f, -1.0f));

    // Creates a right-back foot.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(5, rank);
    m_object->SetObjectParent(5, 4);
    modelManager->AddModelReference("ant6.mod", false, rank);
    m_object->SetPosition(5, Math::Vector(0.0f, 0.0f, -2.0f));

    // Creates two middle-right thighs.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(6, rank);
    m_object->SetObjectParent(6, 0);
    modelManager->AddModelReference("ant4.mod", false, rank);
    m_object->SetPosition(6, Math::Vector(0.3f, -0.1f, -0.4f));

    // Creates two middle-right legs.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(7, rank);
    m_object->SetObjectParent(7, 6);
    modelManager->AddModelReference("ant5.mod", false, rank);
    m_object->SetPosition(7, Math::Vector(0.0f, 0.0f, -1.0f));

    // Creates two middle-right feet.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(8, rank);
    m_object->SetObjectParent(8, 7);
    modelManager->AddModelReference("ant6.mod", false, rank);
    m_object->SetPosition(8, Math::Vector(0.0f, 0.0f, -2.0f));

    // Creates the right front thigh.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(9, rank);
    m_object->SetObjectParent(9, 0);
    modelManager->AddModelReference("ant4.mod", false, rank);
    m_object->SetPosition(9, Math::Vector(1.0f, -0.1f, -0.7f));

    // Creates the right front leg.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(10, rank);
    m_object->SetObjectParent(10, 9);
    modelManager->AddModelReference("ant5.mod", false, rank);
    m_object->SetPosition(10, Math::Vector(0.0f, 0.0f, -1.0f));

    // Creates the right front foot.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(11, rank);
    m_object->SetObjectParent(11, 10);
    modelManager->AddModelReference("ant6.mod", false, rank);
    m_object->SetPosition(11, Math::Vector(0.0f, 0.0f, -2.0f));

    // Creates a left-back thigh.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(12, rank);
    m_object->SetObjectParent(12, 0);
    modelManager->AddModelReference("ant4.mod", false, rank);
    m_object->SetPosition(12, Math::Vector(-0.3f, -0.1f, 0.2f));
    m_object->SetAngleY(12, Math::PI);

    // Creates a left-back leg.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(13, rank);
    m_object->SetObjectParent(13, 12);
    modelManager->AddModelReference("ant5.mod", false, rank);
    m_object->SetPosition(13, Math::Vector(0.0f, 0.0f, -1.0f));

    // Creates a left-back foot.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(14, rank);
    m_object->SetObjectParent(14, 13);
    modelManager->AddModelReference("ant6.mod", false, rank);
    m_object->SetPosition(14, Math::Vector(0.0f, 0.0f, -2.0f));

    // Creates two middle-left thigh.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(15, rank);
    m_object->SetObjectParent(15, 0);
    modelManager->AddModelReference("ant4.mod", false, rank);
    m_object->SetPosition(15, Math::Vector(0.3f, -0.1f, 0.4f));
    m_object->SetAngleY(15, Math::PI);

    // Creates two middle-left legs.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(16, rank);
    m_object->SetObjectParent(16, 15);
    modelManager->AddModelReference("ant5.mod", false, rank);
    m_object->SetPosition(16, Math::Vector(0.0f, 0.0f, -1.0f));

    // Creates two middle-left feet.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(17, rank);
    m_object->SetObjectParent(17, 16);
    modelManager->AddModelReference("ant6.mod", false, rank);
    m_object->SetPosition(17, Math::Vector(0.0f, 0.0f, -2.0f));

    // Creates front-left thigh.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(18, rank);
    m_object->SetObjectParent(18, 0);
    modelManager->AddModelReference("ant4.mod", false, rank);
    m_object->SetPosition(18, Math::Vector(1.0f, -0.1f, 0.7f));
    m_object->SetAngleY(18, Math::PI);

    // Creates front-left leg.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(19, rank);
    m_object->SetObjectParent(19, 18);
    modelManager->AddModelReference("ant5.mod", false, rank);
    m_object->SetPosition(19, Math::Vector(0.0f, 0.0f, -1.0f));

    // Creates front-left foot.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(20, rank);
    m_object->SetObjectParent(20, 19);
    modelManager->AddModelReference("ant6.mod", false, rank);
    m_object->SetPosition(20, Math::Vector(0.0f, 0.0f, -2.0f));

    // Creates the right wing.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(21, rank);
    m_object->SetObjectParent(21, 0);
    modelManager->AddModelReference("bee7.mod", false, rank);
    m_object->SetPosition(21, Math::Vector(0.8f, 0.4f, -0.5f));

    // Creates the left wing.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, Gfx::ENG_OBJTYPE_DESCENDANT);
    m_object->SetObjectRank(22, rank);
    m_object->SetObjectParent(22, 0);
    modelManager->AddModelReference("bee7.mod", true, rank);
    m_object->SetPosition(22, Math::Vector(0.8f, 0.4f, 0.5f));

    m_object->CreateShadowCircle(6.0f, 0.5f);

    CreatePhysics();
    m_object->SetFloorHeight(0.0f);

    pos = m_object->GetPosition(0);
    m_object->SetPosition(0, pos);  // to display the shadows immediately

    m_engine->LoadAllTextures();

    return true;
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

    m_physics->SetType(TYPE_FLYING);

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

bool CMotionBee::EventFrame(const Event &event)
{
    Math::Vector        dir;
    float           s, a, prog;
    int             action, i, st, nd;
    bool            bStop;

    if ( m_engine->GetPause() )  return true;
    if ( !m_engine->IsVisiblePoint(m_object->GetPosition(0)) )  return true;

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
    if ( m_object->GetFret() != 0 )  m_actionType = MBS_HOLD;  // carries the ball

    if ( m_object->GetRuin() )  // destroyed?
    {
        m_actionType = MBS_RUIN;
    }
    if ( m_object->GetBurn() )  // burning?
    {
        m_actionType = MBS_BURN;
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
            st = 3*3*3*3*action + st*3*3*3 + (i%3)*3;
            nd = 3*3*3*3*action + nd*3*3*3 + (i%3)*3;
        }

        if ( i < 3 )  // right leg (1..3) ?
        {
            m_object->SetAngleX(3+3*i+0, Math::PropAngle(m_armAngles[st+ 0], m_armAngles[nd+ 0], prog));
            m_object->SetAngleY(3+3*i+0, Math::PropAngle(m_armAngles[st+ 1], m_armAngles[nd+ 1], prog));
            m_object->SetAngleZ(3+3*i+0, Math::PropAngle(m_armAngles[st+ 2], m_armAngles[nd+ 2], prog));
            m_object->SetAngleX(3+3*i+1, Math::PropAngle(m_armAngles[st+ 9], m_armAngles[nd+ 9], prog));
            m_object->SetAngleY(3+3*i+1, Math::PropAngle(m_armAngles[st+10], m_armAngles[nd+10], prog));
            m_object->SetAngleZ(3+3*i+1, Math::PropAngle(m_armAngles[st+11], m_armAngles[nd+11], prog));
            m_object->SetAngleX(3+3*i+2, Math::PropAngle(m_armAngles[st+18], m_armAngles[nd+18], prog));
            m_object->SetAngleY(3+3*i+2, Math::PropAngle(m_armAngles[st+19], m_armAngles[nd+19], prog));
            m_object->SetAngleZ(3+3*i+2, Math::PropAngle(m_armAngles[st+20], m_armAngles[nd+20], prog));
        }
        else    // left leg(4..6) ?
        {
            m_object->SetAngleX(3+3*i+0, Math::PropAngle(   -m_armAngles[st+ 0],    -m_armAngles[nd+ 0], prog));
            m_object->SetAngleY(3+3*i+0, Math::PropAngle(180-m_armAngles[st+ 1], 180-m_armAngles[nd+ 1], prog));
            m_object->SetAngleZ(3+3*i+0, Math::PropAngle(   -m_armAngles[st+ 2],    -m_armAngles[nd+ 2], prog));
            m_object->SetAngleX(3+3*i+1, Math::PropAngle(    m_armAngles[st+ 9],     m_armAngles[nd+ 9], prog));
            m_object->SetAngleY(3+3*i+1, Math::PropAngle(   -m_armAngles[st+10],    -m_armAngles[nd+10], prog));
            m_object->SetAngleZ(3+3*i+1, Math::PropAngle(   -m_armAngles[st+11],    -m_armAngles[nd+11], prog));
            m_object->SetAngleX(3+3*i+2, Math::PropAngle(    m_armAngles[st+18],     m_armAngles[nd+18], prog));
            m_object->SetAngleY(3+3*i+2, Math::PropAngle(   -m_armAngles[st+19],    -m_armAngles[nd+19], prog));
            m_object->SetAngleZ(3+3*i+2, Math::PropAngle(   -m_armAngles[st+20],    -m_armAngles[nd+20], prog));
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

    if ( m_physics->GetLand() )  // on the ground?
    {
        if ( m_object->GetRuin() )
        {
        }
        else if ( bStop || m_object->GetBurn() )
        {
            m_object->SetAngleZ(2, sinf(m_armTimeAbs*1.7f)*0.15f+0.35f);  // tail
        }
        else
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
            m_object->SetInclinaison(dir);

            m_object->SetAngleZ(2, -sinf(a)*0.3f);  // tail

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

#if 0
    a = Math::Rand()*Math::PI/2.0f*prog;
    m_object->SetAngleX(21, a);  // right wing
    a = -Math::Rand()*Math::PI/4.0f*prog;
    m_object->SetAngleY(21, a);

    a = -Math::Rand()*Math::PI/2.0f*prog;
    m_object->SetAngleX(22, a);  // left wing
    a = Math::Rand()*Math::PI/4.0f*prog;
    m_object->SetAngleY(22, a);
#else
    m_object->SetAngleX(21, (sinf(m_armTimeAbs*30.0f)+1.0f)*(Math::PI/4.0f)*prog);
    m_object->SetAngleY(21, -Math::Rand()*Math::PI/6.0f*prog);

    m_object->SetAngleX(22, -(sinf(m_armTimeAbs*30.0f)+1.0f)*(Math::PI/4.0f)*prog);
    m_object->SetAngleY(22, Math::Rand()*Math::PI/6.0f*prog);
#endif

    m_object->SetAngleZ(1, sinf(m_armTimeAbs*1.4f)*0.20f);  // head
    m_object->SetAngleX(1, sinf(m_armTimeAbs*1.9f)*0.10f);  // head
    m_object->SetAngleY(1, sinf(m_armTimeAbs*2.1f)*0.50f);  // head

#if 0
    h = m_terrain->GetFloorHeight(GetPosition(0));
    radius = 4.0f+h/4.0f;
    color.r = 0.3f+h/80.0f;
    color.g = color.r;
    color.b = color.r;
    color.a = color.r;
    m_engine->SetObjectShadowRadius(m_objectPart[0].object, radius);
    m_engine->SetObjectShadowColor(m_objectPart[0].object, color);
#endif

    return true;
}


