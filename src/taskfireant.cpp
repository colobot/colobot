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

// taskfireant.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "math3d.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "particule.h"
#include "terrain.h"
#include "object.h"
#include "physics.h"
#include "brain.h"
#include "camera.h"
#include "motion.h"
#include "motionant.h"
#include "task.h"
#include "taskfireant.h"




// Object's constructor.

CTaskFireAnt::CTaskFireAnt(CInstanceManager* iMan, CObject* object)
                           : CTask(iMan, object)
{
    m_phase = TFA_NULL;
}

// Object's destructor.

CTaskFireAnt::~CTaskFireAnt()
{
}


// Management of an event.

BOOL CTaskFireAnt::EventProcess(const Event &event)
{
    D3DVECTOR   dir, vib;
    float       a, g, cirSpeed;

    if ( m_engine->RetPause() )  return TRUE;
    if ( event.event != EVENT_FRAME )  return TRUE;
    if ( m_bError )  return FALSE;

    if ( m_object->RetFixed() )  // insect on its back?
    {
        m_bError = TRUE;
        return FALSE;
    }

    m_time += event.rTime;
    m_progress += event.rTime*m_speed;

    if ( m_phase == TFA_TURN )  // preliminary rotation?
    {
        a = m_object->RetAngleY(0);
        g = m_angle;
        cirSpeed = Direction(a, g)*2.0f;
        if ( cirSpeed >  2.0f )  cirSpeed =  2.0f;
        if ( cirSpeed < -2.0f )  cirSpeed = -2.0f;

        m_physics->SetMotorSpeedZ(cirSpeed);  // turns left/right
    }

    return TRUE;
}


// Assigns the goal was achieved.

Error CTaskFireAnt::Start(D3DVECTOR impact)
{
    D3DVECTOR   pos;
    ObjectType  type;

    m_impact = impact;

    m_bError = TRUE;  // operation impossible
    if ( !m_physics->RetLand() )  return ERR_FIRE_VEH;

    type = m_object->RetType();
    if ( type != OBJECT_ANT )  return ERR_FIRE_VEH;

    // Insect on its back?
    if ( m_object->RetFixed() )  return ERR_FIRE_VEH;

    m_physics->SetMotorSpeed(D3DVECTOR(0.0f, 0.0f, 0.0f));

    pos = m_object->RetPosition(0);
    m_angle = RotateAngle(m_impact.x-pos.x, pos.z-m_impact.z);  // CW !

    m_phase = TFA_TURN;
    m_speed = 1.0f/1.0f;
    m_progress = 0.0f;
    m_time = 0.0f;
    m_lastParticule = 0.0f;
    m_bError = FALSE;  // ok
    m_bFire = FALSE;  // once!

    return ERR_OK;
}

// Indicates whether the action is finished.

Error CTaskFireAnt::IsEnded()
{
    D3DMATRIX*  mat;
    D3DVECTOR   pos, speed;
    FPOINT      dim;
    float       angle, dist;
    int         i, channel;

    if ( m_engine->RetPause() )  return ERR_CONTINUE;
    if ( m_bError )  return ERR_STOP;
    if ( m_object->RetFixed() )  return ERR_STOP;  // insect on its back?

    if ( m_phase == TFA_TURN )  // rotation ?
    {
        angle = m_object->RetAngleY(0);
        angle = NormAngle(angle);  // 0..2*PI
        if ( !TestAngle(angle, m_angle-PI*0.05f, m_angle+PI*0.05f) )  return ERR_CONTINUE;

        m_physics->SetMotorSpeedZ(0.0f);  // rotation ended

        m_phase = TFA_PREPARE;
//?     m_speed = 1.0f/1.5f;
        m_speed = 1.0f/0.4f;
        m_progress = 0.0f;
//?     m_motion->SetAction(MAS_PREPARE, 1.5f);
        m_motion->SetAction(MAS_PREPARE, 0.4f);
    }

    if ( m_phase == TFA_PREPARE )  // preparation?
    {
        if ( m_progress < 1.0f )  return ERR_CONTINUE;

        m_phase = TFA_FIRE;
//?     m_speed = 1.0f/2.0f;
        m_speed = 1.0f/0.5f;
        m_progress = 0.0f;
//?     m_motion->SetAction(MAS_FIRE, 2.0f);
        m_motion->SetAction(MAS_FIRE, 0.5f);
    }

    if ( m_phase == TFA_FIRE )  // shooting?
    {
        if ( m_progress > 0.75f && !m_bFire )
        {
            m_bFire = TRUE;  // once

            for ( i=0 ; i<20 ; i++ )
            {
                pos = D3DVECTOR(-2.5f, -0.7f, 0.0f);
                mat = m_object->RetWorldMatrix(2);
                pos = Transform(*mat, pos);
                dist = Length(pos, m_impact);
                speed = m_impact-pos;
                speed.x += (Rand()-0.5f)*dist*1.2f;
                speed.y += (Rand()-0.5f)*dist*0.4f+50.0f;
                speed.z += (Rand()-0.5f)*dist*1.2f;
                dim.x = 1.0f;
                dim.y = dim.x;
                channel = m_particule->CreateParticule(pos, speed, dim, PARTIGUN2, 2.0f, 100.0f, 0.0f);
                m_particule->SetObjectFather(channel, m_object);
            }
        }

        if ( m_progress < 1.0f )  return ERR_CONTINUE;

        m_phase = TFA_TERMINATE;
//?     m_speed = 1.0f/0.9f;
        m_speed = 1.0f/0.4f;
        m_progress = 0.0f;
//?     m_motion->SetAction(MAS_TERMINATE, 0.9f);
        m_motion->SetAction(MAS_TERMINATE, 0.4f);
    }

    if ( m_phase == TFA_TERMINATE )  // ends?
    {
        if ( m_progress < 1.0f )  return ERR_CONTINUE;

        m_phase = TFA_NULL;
        m_speed = 1.0f/1.0f;
        m_progress = 0.0f;
    }

    Abort();
    return ERR_STOP;
}


// Suddenly ends the current action.

BOOL CTaskFireAnt::Abort()
{
    m_motion->SetAction(-1);
    return TRUE;
}

