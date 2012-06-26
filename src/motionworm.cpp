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

// motionworm.cpp

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
#include "motionworm.h"



#define START_TIME      1000.0f     // beginning of the relative time
#define TIME_UPDOWN     2.0f        // time for up / down
#define DOWN_ALTITUDE       3.0f        // underground distance
#define WORM_PART       7       // number of parts of a worm



// Object's constructor.

CMotionWorm::CMotionWorm(CInstanceManager* iMan, CObject* object)
                        : CMotion(iMan, object)
{
    m_timeUp         = 18.0f;
    m_timeDown       = 18.0f;
    m_armMember      = START_TIME;
    m_armTimeAbs     = START_TIME;
    m_armTimeMarch   = START_TIME;
    m_armTimeAction  = START_TIME;
    m_armTimeIndex   = 0;
    m_armPartIndex   = 0;
    m_armMemberIndex = 0;
    m_armLinSpeed    = 0.0f;
    m_armCirSpeed    = 0.0f;
    m_armLastAction  = -1;
    m_specAction     = -1;
    m_lastParticule  = 0.0f;
    m_bArmStop = FALSE;
}

// Object's destructor.

CMotionWorm::~CMotionWorm()
{
}


// Removes an object.

void CMotionWorm::DeleteObject(BOOL bAll)
{
}


// Creates a vehicle traveling any lands on the ground.

BOOL CMotionWorm::Create(D3DVECTOR pos, float angle, ObjectType type,
                         float power)
{
    CModFile*   pModFile;
    int         rank, i;
    float       px;

    if ( m_engine->RetRestCreate() < 2+WORM_PART+1 )  return FALSE;

    pModFile = new CModFile(m_iMan);

    m_object->SetType(type);

    // Creates the main base.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, TYPEVEHICULE);  // this is a moving object
    m_object->SetObjectRank(0, rank);
    pModFile->ReadModel("objects\\worm0.mod");  // there is no purpose!
    pModFile->CreateEngineObject(rank);
    m_object->SetPosition(0, pos);
    m_object->SetAngleY(0, angle);

    // A vehicle must have a obligatory collision with a sphere of center (0, y, 0) (see GetCrashSphere).
    m_object->CreateCrashSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 4.0f, SOUND_BOUM, 0.20f);
    m_object->SetGlobalSphere(D3DVECTOR(0.0f, 0.0f, 0.0f), 5.0f);

    px = 1.0f+WORM_PART/2;

    // Creates the head.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, TYPEDESCENDANT);
    m_object->SetObjectRank(1, rank);
    m_object->SetObjectParent(1, 0);
    pModFile->ReadModel("objects\\worm1.mod");
    pModFile->CreateEngineObject(rank);
    m_object->SetPosition(1, D3DVECTOR(px, 0.0f, 0.0f));
    px -= 1.0f;

    // Creates the body.
    for ( i=0 ; i<WORM_PART ; i++ )
    {
        rank = m_engine->CreateObject();
        m_engine->SetObjectType(rank, TYPEDESCENDANT);
        m_object->SetObjectRank(2+i, rank);
        m_object->SetObjectParent(2+i, 0);
        pModFile->ReadModel("objects\\worm2.mod");
        pModFile->CreateEngineObject(rank);
        m_object->SetPosition(2+i, D3DVECTOR(px, 0.0f, 0.0f));
        px -= 1.0f;
    }

    // Creates the tail.
    rank = m_engine->CreateObject();
    m_engine->SetObjectType(rank, TYPEDESCENDANT);
    m_object->SetObjectRank(2+WORM_PART, rank);
    m_object->SetObjectParent(2+WORM_PART, 0);
    pModFile->ReadModel("objects\\worm3.mod");
    pModFile->CreateEngineObject(rank);
    m_object->SetPosition(2+WORM_PART, D3DVECTOR(px, 0.0f, 0.0f));

    m_object->CreateShadowCircle(0.0f, 1.0f, D3DSHADOWWORM);

    CreatePhysics();
    m_object->SetFloorHeight(0.0f);

    pos = m_object->RetPosition(0);
    m_object->SetPosition(0, pos);  // to display the shadows immediately

    m_engine->LoadAllTexture();

    delete pModFile;
    return TRUE;
}

// Creates the physics of the object.

void CMotionWorm::CreatePhysics()
{
    Character*  character;

    m_physics->SetType(TYPE_ROLLING);

    character = m_object->RetCharacter();
    character->wheelFront = 10.0f;
    character->wheelBack  = 10.0f;
    character->wheelLeft  =  2.0f;
    character->wheelRight =  2.0f;
    character->height     = -0.2f;

    m_physics->SetLinMotionX(MO_ADVSPEED,   3.0f);
    m_physics->SetLinMotionX(MO_RECSPEED,   3.0f);
    m_physics->SetLinMotionX(MO_ADVACCEL,  10.0f);
    m_physics->SetLinMotionX(MO_RECACCEL,  10.0f);
    m_physics->SetLinMotionX(MO_STOACCEL,  40.0f);
    m_physics->SetLinMotionX(MO_TERSLIDE,   5.0f);
    m_physics->SetLinMotionZ(MO_TERSLIDE,   5.0f);
    m_physics->SetLinMotionX(MO_TERFORCE,   5.0f);
    m_physics->SetLinMotionZ(MO_TERFORCE,   5.0f);
    m_physics->SetLinMotionZ(MO_MOTACCEL,  40.0f);

    m_physics->SetCirMotionY(MO_ADVSPEED,   0.2f*PI);
    m_physics->SetCirMotionY(MO_RECSPEED,   0.2f*PI);
    m_physics->SetCirMotionY(MO_ADVACCEL,  10.0f);
    m_physics->SetCirMotionY(MO_RECACCEL,  10.0f);
    m_physics->SetCirMotionY(MO_STOACCEL,  20.0f);
}



// Specifies a special parameter.

BOOL CMotionWorm::SetParam(int rank, float value)
{
    if ( rank == 0 )
    {
        m_timeDown = value;
        return TRUE;
    }

    if ( rank == 1 )
    {
        m_timeUp = value;
        return TRUE;
    }

    return FALSE;
}

float CMotionWorm::RetParam(int rank)
{
    if ( rank == 0 )  return m_timeDown;
    if ( rank == 1 )  return m_timeUp;
    return 0.0f;
}



// Management of an event.

BOOL CMotionWorm::EventProcess(const Event &event)
{
    CMotion::EventProcess(event);

    if ( event.event == EVENT_FRAME )
    {
        return EventFrame(event);
    }

    if ( event.event == EVENT_KEYDOWN )
    {
    }

    return TRUE;
}

// Management of an event.

BOOL CMotionWorm::EventFrame(const Event &event)
{
    D3DMATRIX*  mat;
    D3DVECTOR   pos, p, angle, speed;
    FPOINT      center, pp, dim;
    float       height[WORM_PART+2];
    float       floor, a, s, px, curve, phase, h, zoom, radius;
    int         i, under;

    if ( m_engine->RetPause() )  return TRUE;

    s = m_physics->RetLinMotionX(MO_MOTSPEED)/m_physics->RetLinMotionX(MO_ADVSPEED);
    a = m_physics->RetCirMotionY(MO_MOTSPEED)/m_physics->RetCirMotionY(MO_ADVSPEED);

    if ( s == 0.0f && a != 0.0f )  s = a;

    m_armLinSpeed += (s-m_armLinSpeed)*event.rTime*3.0f;
    m_armCirSpeed += (a-m_armCirSpeed)*event.rTime*1.5f;

    m_armTimeAbs   += event.rTime;
    m_armTimeMarch += event.rTime*m_armLinSpeed;

    under = 0;  // no piece under the ground
    for ( i=0 ; i<WORM_PART+2 ; i++ )
    {
        phase = Mod(m_armTimeMarch-START_TIME-i*0.3f, TIME_UPDOWN+m_timeDown+TIME_UPDOWN+m_timeUp);
        if ( phase < TIME_UPDOWN )  // descends?
        {
            h = -(phase/TIME_UPDOWN)*DOWN_ALTITUDE;
        }
        else if ( phase < TIME_UPDOWN+m_timeDown )  // advance underground?
        {
            h = -DOWN_ALTITUDE;
            under ++;  // the most of a piece entirely under ground
        }
        else if ( phase < TIME_UPDOWN+m_timeDown+TIME_UPDOWN )  // up?
        {
            h = -(1.0f-(phase-TIME_UPDOWN-m_timeDown)/TIME_UPDOWN)*DOWN_ALTITUDE;
        }
        else  // advance on earth?
        {
            h = 0.0f;
        }
        if ( m_object->RetBurn() )  // is burning?
        {
            h = 0.0f;  // remains on earth
        }
        h += 0.3f;
        height[i] = h;
    }
    m_object->SetVisible(under!=WORM_PART+2);

    if ( !m_engine->IsVisiblePoint(m_object->RetPosition(0)) )  return TRUE;

    pos = m_object->RetPosition(0);
    floor = m_terrain->RetFloorLevel(pos, TRUE);

    mat = m_object->RetWorldMatrix(0);

    px = 1.0f+WORM_PART/2;
    for ( i=0 ; i<WORM_PART+2 ; i++ )
    {
        radius = 1.0f+(height[i]-0.3f)/DOWN_ALTITUDE;  // 0 = underground, 1 = surface
        radius = radius*1.3f-0.3f;
        if ( radius < 0.0f )  radius = 0.0f;
        radius *= 5.0f;
        m_engine->SetObjectShadowRadius(m_object->RetObjectRank(0), radius);

        pos.x = px+       sinf(m_armTimeMarch*4.0f+0.5f*i)*0.6f;
        pos.y = height[i]+sinf(m_armTimeMarch*4.0f+0.5f*i)*0.2f*m_armLinSpeed;
        pos.y +=          sinf(m_armTimeAbs  *1.3f+0.2f*i)*0.1f;
        pos.z =           sinf(m_armTimeAbs  *2.0f+0.7f*i)*0.2f;

        curve = ((float)i-(WORM_PART+2)/2)*m_armCirSpeed*0.1f;
        center.x = 0.0f;
        center.y = 0.0f;
        pp.x = pos.x;
        pp.y = pos.z;
        pp = RotatePoint(center, curve, pp);
        pos.x = pp.x;
        pos.z = pp.y;

        p = Transform(*mat, pos);
        pos.y += m_terrain->RetFloorLevel(p, TRUE)-floor;
        m_object->SetPosition(i+1, pos);

        zoom = Mod(m_armTimeAbs*0.5f+100.0f-i*0.1f, 2.0f);
        if ( zoom > 1.0f )  zoom = 2.0f-zoom;
        zoom *= 1.6f;
        if ( zoom < 1.0f )  zoom = 1.0f;
        m_object->SetZoomY(i+1, 0.2f+zoom*0.8f);
        m_object->SetZoomZ(i+1, zoom);

        if ( height[i] >= -1.0f && height[i] < -0.2f &&
             m_lastParticule+m_engine->ParticuleAdapt(0.2f) <= m_armTimeMarch )
        {
            m_lastParticule = m_armTimeMarch;

            pos = p;
            pos.y += -height[i];
            pos.x += (Rand()-0.5f)*4.0f;
            pos.z += (Rand()-0.5f)*4.0f;
            speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
            dim.x = Rand()*2.0f+1.5f;
            dim.y = dim.x;
            m_particule->CreateParticule(pos, speed, dim, PARTICRASH, 2.0f);
        }

        px -= 1.0f;
    }

    for ( i=0 ; i<WORM_PART+1 ; i++ )
    {
        pos  = m_object->RetPosition(i+2);
        pos -= m_object->RetPosition(i+1);

        angle.z = -RotateAngle(Length(pos.x, pos.z), pos.y);
        angle.y = PI-RotateAngle(pos.x, pos.z);
        angle.x = 0.0f;
        m_object->SetAngle(i+1, angle);

        if ( i == WORM_PART )
        {
            m_object->SetAngle(i+2, angle);
        }
    }

    return TRUE;
}


