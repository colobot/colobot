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

// motionant.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "d3dengine.h"
#include "math3d.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "light.h"
#include "particule.h"
#include "terrain.h"
#include "object.h"
#include "physics.h"
#include "brain.h"
#include "camera.h"
#include "modfile.h"
#include "sound.h"
#include "motion.h"
#include "motionant.h"



#define ADJUST_ANGLE        FALSE       // TRUE -> adjusts the angles of the members
#define START_TIME      1000.0f     // beginning of the relative time



// Object's constructor.

CMotionAnt::CMotionAnt(CInstanceManager* iMan, CObject* object)
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
    m_bArmStop = FALSE;
    m_lastParticule = 0.0f;
}

// Object's destructor.

CMotionAnt::~CMotionAnt()
{
}


// Removes an object.

void CMotionAnt::DeleteObject(BOOL bAll)
{
}


// Creates a vehicle poses some rolling on the floor.

BOOL CMotionAnt::Create(D3DVECTOR pos, float angle, ObjectType type,
                        float power)
{
    CModFile*   pModFile;
    int         rank;

    if ( m_engine->RetRestCreate() < 3+18 )  return FALSE;

    pModFile = new CModFile(m_iMan);

    m_object->SetType(type);

    // Creates the main base.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, TYPEVEHICULE);  // this is a moving object
    m_object->SetObjectRank(0, rank);

    pModFile->ReadModel("objects\\ant1.mod");
    pModFile->CreateEngineObject(rank);

    m_object->SetPosition(0, pos);
    m_object->SetAngleY(0, angle);

    // A vehicle must have necessarily a collision
    //with a sphere of center (0, y, 0) (see GetCrashSphere).
    m_object->CreateCrashSphere(D3DVECTOR(0.0f, -2.0f, 0.0f), 4.0f, SOUND_BOUM, 0.20f);
    m_object->SetGlobalSphere(D3DVECTOR(-0.5f, 1.0f, 0.0f), 4.0f);

    // Creates the head.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, TYPEDESCENDANT);
    m_object->SetObjectRank(1, rank);
    m_object->SetObjectParent(1, 0);
    pModFile->ReadModel("objects\\ant2.mod");
    pModFile->CreateEngineObject(rank);
    m_object->SetPosition(1, D3DVECTOR(2.0f, 0.0f, 0.0f));

    // Creates the tail.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, TYPEDESCENDANT);
    m_object->SetObjectRank(2, rank);
    m_object->SetObjectParent(2, 0);
    pModFile->ReadModel("objects\\ant3.mod");
    pModFile->CreateEngineObject(rank);
    m_object->SetPosition(2, D3DVECTOR(-1.0f, 0.0f, 0.0f));

    // Creates a right-back thigh.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, TYPEDESCENDANT);
    m_object->SetObjectRank(3, rank);
    m_object->SetObjectParent(3, 0);
    pModFile->ReadModel("objects\\ant4.mod");
    pModFile->CreateEngineObject(rank);
    m_object->SetPosition(3, D3DVECTOR(-0.4f, -0.1f, -0.3f));

    // Creates a right-back leg.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, TYPEDESCENDANT);
    m_object->SetObjectRank(4, rank);
    m_object->SetObjectParent(4, 3);
    pModFile->ReadModel("objects\\ant5.mod");
    pModFile->CreateEngineObject(rank);
    m_object->SetPosition(4, D3DVECTOR(0.0f, 0.0f, -1.0f));

    // Creates a right-back foot.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, TYPEDESCENDANT);
    m_object->SetObjectRank(5, rank);
    m_object->SetObjectParent(5, 4);
    pModFile->ReadModel("objects\\ant6.mod");
    pModFile->CreateEngineObject(rank);
    m_object->SetPosition(5, D3DVECTOR(0.0f, 0.0f, -2.0f));

    // Creates two middle-right thighs.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, TYPEDESCENDANT);
    m_object->SetObjectRank(6, rank);
    m_object->SetObjectParent(6, 0);
    pModFile->ReadModel("objects\\ant4.mod");
    pModFile->CreateEngineObject(rank);
    m_object->SetPosition(6, D3DVECTOR(0.1f, -0.1f, -0.4f));

    // Creates two middle-right legs.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, TYPEDESCENDANT);
    m_object->SetObjectRank(7, rank);
    m_object->SetObjectParent(7, 6);
    pModFile->ReadModel("objects\\ant5.mod");
    pModFile->CreateEngineObject(rank);
    m_object->SetPosition(7, D3DVECTOR(0.0f, 0.0f, -1.0f));

    // Creates two middle-right foots.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, TYPEDESCENDANT);
    m_object->SetObjectRank(8, rank);
    m_object->SetObjectParent(8, 7);
    pModFile->ReadModel("objects\\ant6.mod");
    pModFile->CreateEngineObject(rank);
    m_object->SetPosition(8, D3DVECTOR(0.0f, 0.0f, -2.0f));

    // Creates the right front thigh.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, TYPEDESCENDANT);
    m_object->SetObjectRank(9, rank);
    m_object->SetObjectParent(9, 0);
    pModFile->ReadModel("objects\\ant4.mod");
    pModFile->CreateEngineObject(rank);
    m_object->SetPosition(9, D3DVECTOR(1.4f, -0.1f, -0.6f));

    // Creates the right front leg.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, TYPEDESCENDANT);
    m_object->SetObjectRank(10, rank);
    m_object->SetObjectParent(10, 9);
    pModFile->ReadModel("objects\\ant5.mod");
    pModFile->CreateEngineObject(rank);
    m_object->SetPosition(10, D3DVECTOR(0.0f, 0.0f, -1.0f));

    // Creates the right front foot.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, TYPEDESCENDANT);
    m_object->SetObjectRank(11, rank);
    m_object->SetObjectParent(11, 10);
    pModFile->ReadModel("objects\\ant6.mod");
    pModFile->CreateEngineObject(rank);
    m_object->SetPosition(11, D3DVECTOR(0.0f, 0.0f, -2.0f));

    // Creates a left-back thigh.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, TYPEDESCENDANT);
    m_object->SetObjectRank(12, rank);
    m_object->SetObjectParent(12, 0);
    pModFile->ReadModel("objects\\ant4.mod");
    pModFile->Mirror();
    pModFile->CreateEngineObject(rank);
    m_object->SetPosition(12, D3DVECTOR(-0.4f, -0.1f, 0.3f));

    // Creates a left-back leg.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, TYPEDESCENDANT);
    m_object->SetObjectRank(13, rank);
    m_object->SetObjectParent(13, 12);
    pModFile->ReadModel("objects\\ant5.mod");
    pModFile->Mirror();
    pModFile->CreateEngineObject(rank);
    m_object->SetPosition(13, D3DVECTOR(0.0f, 0.0f, 1.0f));

    // Creates a left-back foot.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, TYPEDESCENDANT);
    m_object->SetObjectRank(14, rank);
    m_object->SetObjectParent(14, 13);
    pModFile->ReadModel("objects\\ant6.mod");
    pModFile->Mirror();
    pModFile->CreateEngineObject(rank);
    m_object->SetPosition(14, D3DVECTOR(0.0f, 0.0f, 2.0f));

    // Creates two middle-left thighs.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, TYPEDESCENDANT);
    m_object->SetObjectRank(15, rank);
    m_object->SetObjectParent(15, 0);
    pModFile->ReadModel("objects\\ant4.mod");
    pModFile->Mirror();
    pModFile->CreateEngineObject(rank);
    m_object->SetPosition(15, D3DVECTOR(0.1f, -0.1f, 0.4f));

    // Creates two middle-left legs.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, TYPEDESCENDANT);
    m_object->SetObjectRank(16, rank);
    m_object->SetObjectParent(16, 15);
    pModFile->ReadModel("objects\\ant5.mod");
    pModFile->Mirror();
    pModFile->CreateEngineObject(rank);
    m_object->SetPosition(16, D3DVECTOR(0.0f, 0.0f, 1.0f));

    // Creates two middle-left foot.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, TYPEDESCENDANT);
    m_object->SetObjectRank(17, rank);
    m_object->SetObjectParent(17, 16);
    pModFile->ReadModel("objects\\ant6.mod");
    pModFile->Mirror();
    pModFile->CreateEngineObject(rank);
    m_object->SetPosition(17, D3DVECTOR(0.0f, 0.0f, 2.0f));

    // Creates the left front thigh.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, TYPEDESCENDANT);
    m_object->SetObjectRank(18, rank);
    m_object->SetObjectParent(18, 0);
    pModFile->ReadModel("objects\\ant4.mod");
    pModFile->Mirror();
    pModFile->CreateEngineObject(rank);
    m_object->SetPosition(18, D3DVECTOR(1.4f, -0.1f, 0.6f));

    // Creates the left front leg.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, TYPEDESCENDANT);
    m_object->SetObjectRank(19, rank);
    m_object->SetObjectParent(19, 18);
    pModFile->ReadModel("objects\\ant5.mod");
    pModFile->Mirror();
    pModFile->CreateEngineObject(rank);
    m_object->SetPosition(19, D3DVECTOR(0.0f, 0.0f, 1.0f));

    // Creates the left front foot.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, TYPEDESCENDANT);
    m_object->SetObjectRank(20, rank);
    m_object->SetObjectParent(20, 19);
    pModFile->ReadModel("objects\\ant6.mod");
    pModFile->Mirror();
    pModFile->CreateEngineObject(rank);
    m_object->SetPosition(20, D3DVECTOR(0.0f, 0.0f, 2.0f));

    m_object->CreateShadowCircle(4.0f, 0.5f);

    CreatePhysics();
    m_object->SetFloorHeight(0.0f);

    pos = m_object->RetPosition(0);
    m_object->SetPosition(0, pos);  // to display the shadows immediately

    m_engine->LoadAllTexture();

    delete pModFile;
    return TRUE;
}

// Creates the physics of the object.

void CMotionAnt::CreatePhysics()
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

    int member_stop[] =
    {
    //  x1,y1,z1,   x2,y2,z2,   x3,y3,z3,   // in the air:
        0,30,0,     0,20,0,     0,15,0,     // t0: thighs 1..3
        -15,-35,0,  -20,-60,0,  -15,-75,0,  // t0: legs 1..3
        -35,35,0,   -25,40,0,   -40,65,0,   // t0: feet 1..3
                                            // on the ground:
        0,30,0,     0,20,0,     0,15,0,     // t1: thighs 1..3
        -15,-35,0,  -20,-60,0,  -15,-75,0,  // t1: legs 1..3
        -35,35,0,   -25,40,0,   -40,65,0,   // t1: feet 1..3
                                            // on the ground back:
        0,30,0,     0,20,0,     0,15,0,     // t2: thighs 1..3
        -15,-35,0,  -20,-60,0,  -15,-75,0,  // t2: legs 1..3
        -35,35,0,   -25,40,0,   -40,65,0,   // t2: feet 1..3
    };

    int member_spec[] =
    {
    //  x1,y1,z1,   x2,y2,z2,   x3,y3,z3,   // prepares the fire:
        0,20,0,     0,10,0,     0,50,0,     // s0: thighs 1..3
        -50,-30,0,  -20,-15,0,  35,-65,0,   // s0: legs 1..3
        -5,-40,0,   20,-70,0,   -10,-40,0,  // s0: feet 1..3
                                            // shot:
        0,20,0,     0,10,0,     0,50,0,     // s1: thighs 1..3
        -50,-30,0,  -20,-15,0,  35,-65,0,   // s1: legs 1..3
        -5,-40,0,   20,-70,0,   -10,-40,0,  // s1: feet 1..3
                                            // ends the fire:
        0,30,0,     0,20,0,     0,15,0,     // s2: thighs 1..3
        -15,-50,0,  -20,-60,0,  -10,-75,0,  // s2: legs 1..3
        -40,50,0,   -25,15,0,   -50,35,0,   // s2: feet 1..3
                                            // burning:
        0,30,0,     0,20,0,     0,15,0,     // s3: thighs 1..3
        -15,-35,0,  -20,-60,0,  -15,-75,0,  // s3: legs 1..3
        -35,35,0,   -25,40,0,   -40,65,0,   // s3: feet 1..3
                                            // destroyed:
        0,30,0,     0,20,0,     0,15,0,     // s4: thighs 1..3
        -15,-35,0,  -20,-60,0,  -15,-75,0,  // s4: legs 1..3
        -35,35,0,   -25,40,0,   -40,65,0,   // s4: feet 1..3
                                            // back1 :
        0,30,0,     0,20,0,     0,15,0,     // s5: thighs 1..3
        -15,-35,0,  -20,-60,0,  -15,-75,0,  // s5: legs 1..3
        -35,35,0,   -25,40,0,   -40,65,0,   // s5: feet 1..3
                                            // back2 :
        0,45,0,     0,45,0,     0,50,0,     // s6: thighs 1..3
        -35,-70,0,  -20,-85,-25,    -25,-100,0, // s6: legs 1..3
        -110,75,-15,    -130,80,-25,    -125,40,0,  // s6: feet 1..3
                                            // back3 :
        0,30,0,     0,20,0,     0,15,0,     // s7: thighs 1..3
        -15,-35,0,  -20,-60,0,  -15,-75,0,  // s7: legs 1..3
        -35,35,0,   -25,40,0,   -40,65,0,   // s7: feet 1..3
    };

    m_physics->SetType(TYPE_ROLLING);

    character = m_object->RetCharacter();
    character->wheelFront = 3.0f;
    character->wheelBack  = 3.0f;
    character->wheelLeft  = 5.0f;
    character->wheelRight = 5.0f;
    character->height     = 1.2f;

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

    m_physics->SetCirMotionY(MO_ADVSPEED,   1.0f*PI);
    m_physics->SetCirMotionY(MO_RECSPEED,   1.0f*PI);
    m_physics->SetCirMotionY(MO_ADVACCEL,  20.0f);
    m_physics->SetCirMotionY(MO_RECACCEL,  20.0f);
    m_physics->SetCirMotionY(MO_STOACCEL,  40.0f);

    for ( i=0 ; i<3*3*3*3 ; i++ )
    {
        m_armAngles[3*3*3*3*MA_MARCH+i] = member_march[i];
    }
    for ( i=0 ; i<3*3*3*3 ; i++ )
    {
        m_armAngles[3*3*3*3*MA_STOP+i] = member_stop[i];
    }
    for ( i=0 ; i<3*3*3*8 ; i++ )
    {
        m_armAngles[3*3*3*3*MA_SPEC+i] = member_spec[i];
    }
}


// Management of an event.

BOOL CMotionAnt::EventProcess(const Event &event)
{
    CMotion::EventProcess(event);

    if ( event.event == EVENT_FRAME )
    {
        return EventFrame(event);
    }

    if ( event.event == EVENT_KEYDOWN )
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

    return TRUE;
}

// Calculates a value (radians) proportional between a and b (degrees).

inline float Propf(float a, float b, float p)
{
    float   aa, bb;

    aa = a*PI/180.0f;
    bb = b*PI/180.0f;

    return aa+p*(bb-aa);
}

// Management of an event.

BOOL CMotionAnt::EventFrame(const Event &event)
{
    D3DVECTOR   dir, pos, speed;
    FPOINT      dim;
    float       s, a, prog, time;
    float       tSt[9], tNd[9];
    int         i, ii, st, nd, action;
    BOOL        bStop;

    if ( m_engine->RetPause() )  return TRUE;
    if ( !m_engine->IsVisiblePoint(m_object->RetPosition(0)) )  return TRUE;

    s =     m_physics->RetLinMotionX(MO_MOTSPEED)*1.5f;
    a = Abs(m_physics->RetCirMotionY(MO_MOTSPEED)*2.0f);

    if ( s == 0.0f && a != 0.0f )  a *= 1.5f;

    m_armTimeAbs += event.rTime;
    m_armTimeMarch += (s)*event.rTime*0.15f;
    m_armMember += (s+a)*event.rTime*0.15f;

    bStop = ( a == 0.0f && s == 0.0f );  // stopped?

    action = MA_MARCH;  // walking
    if ( s == 0.0f && a == 0.0f )
    {
        action = MA_STOP;  // stop
    }

    if ( bStop )
    {
        prog = Mod(m_armTimeAbs, 2.0f)/10.0f;
        a = Mod(m_armMember, 1.0f);
        a = (prog-a)*event.rTime*2.0f;  // stop position is pleasantly
        m_armMember += a;
    }

    if ( m_object->RetRuin() )  // destroyed?
    {
        m_actionType = MAS_RUIN;
    }
    if ( m_object->RetBurn() )  // burning?
    {
        if ( m_object->RetFixed() )
        {
            m_actionType = MAS_BURN;
        }
        else
        {
            m_actionType = -1;
        }
    }

    for ( i=0 ; i<6 ; i++ )  // the six legs
    {
        if ( m_actionType != -1 )  // special action in progress?
        {
            st = 3*3*3*3*MA_SPEC + 3*3*3*m_actionType + (i%3)*3;
            nd = st;
            time = event.rTime*m_actionTime;
            m_armTimeAction = 0.0f;
        }
        else
        {
            if ( i < 3 )  prog = Mod(m_armMember+(2.0f-(i%3))*0.33f+0.0f, 1.0f);
            else          prog = Mod(m_armMember+(2.0f-(i%3))*0.33f+0.3f, 1.0f);
            if ( m_bArmStop )
            {
                prog = (float)m_armTimeIndex/3.0f;
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

            // More and more soft ...
            time = event.rTime*(10.0f+Min(m_armTimeAction*100.0f, 200.0f));
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

        if ( m_actionType == MAS_BACK2 )   // on the back?
        {
            for ( ii=0 ; ii<9 ; ii++ )
            {
                tSt[ii] += Rand()*50.0f;
                tNd[ii] = tSt[ii];
            }
//?         time = 100.0f;
            time = event.rTime*10.0f;
        }

        if ( i < 3 )  // right leg (1..3) ?
        {
            m_object->SetAngleX(3+3*i+0, Smooth(m_object->RetAngleX(3+3*i+0), Propf(tSt[0], tNd[0], prog), time));
            m_object->SetAngleY(3+3*i+0, Smooth(m_object->RetAngleY(3+3*i+0), Propf(tSt[1], tNd[1], prog), time));
            m_object->SetAngleZ(3+3*i+0, Smooth(m_object->RetAngleZ(3+3*i+0), Propf(tSt[2], tNd[2], prog), time));
            m_object->SetAngleX(3+3*i+1, Smooth(m_object->RetAngleX(3+3*i+1), Propf(tSt[3], tNd[3], prog), time));
            m_object->SetAngleY(3+3*i+1, Smooth(m_object->RetAngleY(3+3*i+1), Propf(tSt[4], tNd[4], prog), time));
            m_object->SetAngleZ(3+3*i+1, Smooth(m_object->RetAngleZ(3+3*i+1), Propf(tSt[5], tNd[5], prog), time));
            m_object->SetAngleX(3+3*i+2, Smooth(m_object->RetAngleX(3+3*i+2), Propf(tSt[6], tNd[6], prog), time));
            m_object->SetAngleY(3+3*i+2, Smooth(m_object->RetAngleY(3+3*i+2), Propf(tSt[7], tNd[7], prog), time));
            m_object->SetAngleZ(3+3*i+2, Smooth(m_object->RetAngleZ(3+3*i+2), Propf(tSt[8], tNd[8], prog), time));
        }
        else    // left leg (4..6) ?
        {
            m_object->SetAngleX(3+3*i+0, Smooth(m_object->RetAngleX(3+3*i+0), Propf(-tSt[0], -tNd[0], prog), time));
            m_object->SetAngleY(3+3*i+0, Smooth(m_object->RetAngleY(3+3*i+0), Propf(-tSt[1], -tNd[1], prog), time));
            m_object->SetAngleZ(3+3*i+0, Smooth(m_object->RetAngleZ(3+3*i+0), Propf( tSt[2],  tNd[2], prog), time));
            m_object->SetAngleX(3+3*i+1, Smooth(m_object->RetAngleX(3+3*i+1), Propf(-tSt[3], -tNd[3], prog), time));
            m_object->SetAngleY(3+3*i+1, Smooth(m_object->RetAngleY(3+3*i+1), Propf(-tSt[4], -tNd[4], prog), time));
            m_object->SetAngleZ(3+3*i+1, Smooth(m_object->RetAngleZ(3+3*i+1), Propf( tSt[5],  tNd[5], prog), time));
            m_object->SetAngleX(3+3*i+2, Smooth(m_object->RetAngleX(3+3*i+2), Propf(-tSt[6], -tNd[6], prog), time));
            m_object->SetAngleY(3+3*i+2, Smooth(m_object->RetAngleY(3+3*i+2), Propf(-tSt[7], -tNd[7], prog), time));
            m_object->SetAngleZ(3+3*i+2, Smooth(m_object->RetAngleZ(3+3*i+2), Propf( tSt[8],  tNd[8], prog), time));
        }
    }

#if ADJUST_ANGLE
    if ( m_object->RetSelect() )
    {
        char s[100];
        sprintf(s, "A:time=%d Q:part=%d W:member=%d", m_armTimeIndex, m_armPartIndex, m_armMemberIndex);
        m_engine->SetInfoText(4, s);
    }
#endif

    if ( m_actionType == MAS_PREPARE )  // prepares the shooting?
    {
        prog = m_progress;

        dir.x = 0.0f;
        dir.y = 0.0f;
        dir.z = Prop(0, -50, prog);
        SetInclinaison(dir);
        m_object->SetAngleZ(1, Prop(0,  65, prog));  // head
        m_object->SetAngleZ(2, Prop(0, -95, prog));  // tail
    }
    else if ( m_actionType == MAS_FIRE )  // shooting?
    {
        if ( m_progress < 0.75f )  a = m_progress/0.75f;
        else                       a = (1.0f-m_progress)/0.25f;
        m_object->SetZoom(2, (a*0.5f)+1.0f);  // tail
        m_object->SetAngleX(2, (Rand()-0.5f)*0.3f*a);
        m_object->SetAngleY(2, (Rand()-0.5f)*0.3f*a);

        dir.x = (Rand()-0.5f)*0.02f*a;
        dir.y = (Rand()-0.5f)*0.05f*a;
        dir.z = (Rand()-0.5f)*0.03f*a;
        SetCirVibration(dir);
    }
    else if ( m_actionType == MAS_TERMINATE )  // ends the shooting?
    {
        prog = 1.0f-m_progress;

        dir.x = 0.0f;
        dir.y = 0.0f;
        dir.z = Prop(0, -50, prog);
        SetInclinaison(dir);
        m_object->SetAngleZ(1, Prop(0,  65, prog));  // head
        m_object->SetAngleZ(2, Prop(0, -95, prog));  // tail
    }
    else if ( m_actionType == MAS_BURN )  // burning?
    {
        dir = D3DVECTOR(PI, 0.0f, 0.0f);
        SetCirVibration(dir);
        dir = D3DVECTOR(0.0f, -1.5f, 0.0f);
        SetLinVibration(dir);
        dir = D3DVECTOR(0.0f, 0.0f, 0.0f);
        SetInclinaison(dir);

        time = event.rTime*1.0f;
        m_object->SetAngleZ(1, Smooth(m_object->RetAngleZ(1), 0.0f, time));  // head
        m_object->SetAngleZ(2, Smooth(m_object->RetAngleZ(2), 0.0f, time));  // tail
    }
    else if ( m_actionType == MAS_RUIN )  // destroyed?
    {
        dir = D3DVECTOR(0.0f, 0.0f, 0.0f);
        SetLinVibration(dir);
        SetCirVibration(dir);
        SetInclinaison(dir);
    }
    else if ( m_actionType == MAS_BACK1 )  // starts on the back?
    {
        if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_armTimeAbs )
        {
            m_lastParticule = m_armTimeAbs;

            pos = m_object->RetPosition(0);
            speed.x = (Rand()-0.5f)*10.0f;
            speed.z = (Rand()-0.5f)*10.0f;
            speed.y = Rand()*5.0f;
            dim.x = Rand()*3.0f+2.0f;
            dim.y = dim.x;
            m_particule->CreateParticule(pos, speed, dim, PARTICRASH, 2.0f);
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
        dir.x = m_progress*PI;
        dir.y = 0.0f;
        dir.z = 0.0f;
        SetCirVibration(dir);

        dir = D3DVECTOR(0.0f, 0.0f, 0.0f);
        SetInclinaison(dir);

        if ( m_progress >= 1.0f )
        {
            SetAction(MAS_BACK2, 55.0f+Rand()*10.0f);
        }
    }
    else if ( m_actionType == MAS_BACK2 )  // moves on the back?
    {
        if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_armTimeAbs )
        {
            m_lastParticule = m_armTimeAbs;

            if ( rand()%10 == 0 )
            {
                pos = m_object->RetPosition(0);
                pos.x += (Rand()-0.5f)*5.0f;
                pos.z += (Rand()-0.5f)*5.0f;
                pos.y -= 1.0f;
                speed.x = (Rand()-0.5f)*2.0f;
                speed.z = (Rand()-0.5f)*2.0f;
                speed.y = Rand()*2.0f;
                dim.x = Rand()*1.0f+1.0f;
                dim.y = dim.x;
                m_particule->CreateParticule(pos, speed, dim, PARTICRASH, 2.0f);
            }
        }

        dir = D3DVECTOR(0.0f, -1.0f, 0.0f);
        SetLinVibration(dir);
        dir.x = sinf(m_armTimeAbs* 4.0f)*0.10f+
                sinf(m_armTimeAbs* 7.0f)*0.20f+
                sinf(m_armTimeAbs*10.0f)*0.40f+
                sinf(m_armTimeAbs*21.0f)*0.50f+PI;
        dir.y = sinf(m_armTimeAbs* 3.0f)*0.01f+
                sinf(m_armTimeAbs* 6.0f)*0.02f+
                sinf(m_armTimeAbs*11.0f)*0.04f+
                sinf(m_armTimeAbs*20.0f)*0.02f;
        dir.z = sinf(m_armTimeAbs* 5.0f)*0.01f+
                sinf(m_armTimeAbs* 8.0f)*0.02f+
                sinf(m_armTimeAbs* 9.0f)*0.04f+
                sinf(m_armTimeAbs*23.0f)*0.03f;
        SetCirVibration(dir);
        dir = D3DVECTOR(0.0f, 0.0f, 0.0f);
        SetInclinaison(dir);

        m_object->SetAngleY(1, sinf(m_armTimeAbs*8.0f)*0.7f);  // head
        m_object->SetAngleY(2, cosf(m_armTimeAbs*8.0f)*0.7f);  // tail
        m_object->SetAngleZ(1, 0.0f);  // head
        m_object->SetAngleZ(2, 0.0f);  // tail

        if ( m_progress >= 1.0f )
        {
            SetAction(MAS_BACK3, 0.4f);
        }
    }
    else if ( m_actionType == MAS_BACK3 )  // goes back on the legs?
    {
        if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_armTimeAbs )
        {
            m_lastParticule = m_armTimeAbs;

            pos = m_object->RetPosition(0);
            speed.x = (Rand()-0.5f)*10.0f;
            speed.z = (Rand()-0.5f)*10.0f;
            speed.y = Rand()*5.0f;
            dim.x = Rand()*3.0f+2.0f;
            dim.y = dim.x;
            m_particule->CreateParticule(pos, speed, dim, PARTICRASH, 2.0f);
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
        dir.x = (1.0f-m_progress)*PI;
        dir.y = 0.0f;
        dir.z = 0.0f;
        SetCirVibration(dir);

        dir = D3DVECTOR(0.0f, 0.0f, 0.0f);
        SetInclinaison(dir);

        if ( m_progress >= 1.0f )
        {
            SetAction(-1);
            m_object->SetFixed(FALSE);  // moving again
        }
    }
    else
    {
        m_object->SetZoom(2, 1.0f);  // tail
        m_object->SetAngleX(2, 0.0f);
        m_object->SetAngleY(2, 0.0f);

        if ( bStop )
        {
            m_object->SetAngleZ(2, sinf(m_armTimeAbs*1.7f)*0.15f);  // tail

            dir = D3DVECTOR(0.0f, 0.0f, 0.0f);
            SetLinVibration(dir);
            SetInclinaison(dir);
        }
        else
        {
            a = Mod(m_armTimeMarch, 1.0f);
            if ( a < 0.5f )  a = -1.0f+4.0f*a;  // -1..1
            else             a =  3.0f-4.0f*a;  // 1..-1
            dir.x = sinf(a)*0.05f;

            s = Mod(m_armTimeMarch/2.0f, 1.0f);
            if ( s < 0.5f )  s = -1.0f+4.0f*s;  // -1..1
            else             s =  3.0f-4.0f*s;  // 1..-1
            dir.z = sinf(s)*0.1f;

            dir.y = 0.0f;
            SetInclinaison(dir);

            m_object->SetAngleZ(2, -sinf(a)*0.3f);  // tail

            a = Mod(m_armMember-0.1f, 1.0f);
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
            SetLinVibration(dir);
        }

        dir = D3DVECTOR(0.0f, 0.0f, 0.0f);
        SetCirVibration(dir);

        m_object->SetAngleZ(1, sinf(m_armTimeAbs*1.4f)*0.20f);  // head
        m_object->SetAngleX(1, sinf(m_armTimeAbs*1.9f)*0.10f);  // head
        m_object->SetAngleY(1, sinf(m_armTimeAbs*2.1f)*0.50f);  // head
    }

    return TRUE;
}


