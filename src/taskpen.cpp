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

// taskpen.cpp

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
#include "sound.h"
#include "motion.h"
#include "motionant.h"
#include "motionspider.h"
#include "task.h"
#include "taskpen.h"



// Object's constructor.

CTaskPen::CTaskPen(CInstanceManager* iMan, CObject* object)
                               : CTask(iMan, object)
{
}

// Object's destructor.

CTaskPen::~CTaskPen()
{
}


// Management of an event.

BOOL CTaskPen::EventProcess(const Event &event)
{
    D3DVECTOR   pos, speed;
    FPOINT      dim;
    int         i;

    if ( m_engine->RetPause() )  return TRUE;
    if ( event.event != EVENT_FRAME )  return TRUE;
    if ( m_bError )  return FALSE;

    if ( m_delay == 0.0f )
    {
        m_progress = 1.0f;
    }
    else
    {
        m_progress += event.rTime*(1.0f/m_delay);  // others advance
        if ( m_progress > 1.0f )  m_progress = 1.0f;
    }

    m_time += event.rTime;

    if ( m_phase == TPP_UP )  // back the pencil
    {
        i = AngleToRank(m_object->RetAngleY(1));
        pos = m_object->RetPosition(10+i);
        pos.y = -3.2f*(1.0f-m_progress);
        m_object->SetPosition(10+i, pos);
    }

    if ( m_phase == TPP_TURN )  // turns the carousel?
    {
        if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
        {
            m_lastParticule = m_time;

            pos = m_supportPos;
            pos.x += (Rand()-0.5f)*5.0f;
            pos.z += (Rand()-0.5f)*5.0f;
            speed.x = (Rand()-0.5f)*3.0f;
            speed.z = (Rand()-0.5f)*3.0f;
            speed.y = Rand()*2.0f;
            dim.x = Rand()*1.5f+2.0f;
            dim.y = dim.x;
            m_particule->CreateParticule(pos, speed, dim, PARTISMOKE3, 4.0f);
        }

        m_object->SetAngleY(1, m_oldAngle+(m_newAngle-m_oldAngle)*m_progress);
    }

    if ( m_phase == TPP_DOWN )  // down the pencil?
    {
        if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
        {
            m_lastParticule = m_time;

            pos = m_supportPos;
            pos.x += (Rand()-0.5f)*5.0f;
            pos.z += (Rand()-0.5f)*5.0f;
            speed.x = (Rand()-0.5f)*3.0f;
            speed.z = (Rand()-0.5f)*3.0f;
            speed.y = Rand()*5.0f;
            dim.x = Rand()*1.0f+1.0f;
            dim.y = dim.x;
            m_particule->CreateParticule(pos, speed, dim, PARTIVAPOR, 4.0f);
        }

        i = AngleToRank(m_object->RetAngleY(1));
        pos = m_object->RetPosition(10+i);
        if ( m_timeDown == 0.0f )
        {
            pos.y = 0.0f;
        }
        else
        {
            pos.y = -3.2f*Bounce(Min(m_progress*1.8f, 1.0f));
        }
        m_object->SetPosition(10+i, pos);
    }

    return TRUE;
}


// Assigns the goal has achieved.

Error CTaskPen::Start(BOOL bDown, int color)
{
    D3DVECTOR   pos;
    D3DMATRIX*  mat;
    ObjectType  type;
    int         i;

    m_bError = TRUE;  // operation impossible

    type = m_object->RetType();
    if ( type != OBJECT_MOBILEdr )  return ERR_FIRE_VEH;

    m_bError = FALSE;  // ok

    m_oldAngle = m_object->RetAngleY(1);
    m_newAngle = ColorToAngle(color);

    i = AngleToRank(m_oldAngle);
    pos = m_object->RetPosition(10+i);

    if ( pos.y == 0.0f )  // pencil at the top?
    {
        m_timeUp = 0.0f;
    }
    else    // pencil on the bottom?
    {
        m_timeUp = 1.0f;  // must rise up
    }

    if ( bDown )  // must go down ?
    {
        m_timeDown = 0.7f;
    }
    else
    {
        m_timeDown = 0.0f;
    }

    mat = m_object->RetWorldMatrix(0);
    pos = D3DVECTOR(-3.0f, 7.0f, 0.0f);
    pos = Transform(*mat, pos);  // position of carousel
    m_supportPos = pos;

    m_phase    = TPP_UP;
    m_progress = 0.0f;
    m_delay    = m_timeUp;
    m_time     = 0.0f;

    if ( m_timeUp > 0.0f )
    {
        SoundManip(m_timeUp, 1.0f, 0.5f);
    }

    m_lastParticule = 0.0f;

//? m_camera->StartCentering(m_object, PI*0.60f, 99.9f, 5.0f, 0.5f);

    return ERR_OK;
}

// Indicates whether the action is finished.

Error CTaskPen::IsEnded()
{
    if ( m_engine->RetPause() )  return ERR_CONTINUE;
    if ( m_bError )  return ERR_STOP;

    if ( m_progress < 1.0f )  return ERR_CONTINUE;
    m_progress = 0.0f;

    if ( m_phase == TPP_UP )
    {
        m_phase    = TPP_TURN;
        m_progress = 0.0f;
        m_delay    = Abs(m_oldAngle-m_newAngle)/PI;
        m_time     = 0.0f;
        m_lastParticule = 0.0f;
        if ( m_delay > 0.0f )
        {
            SoundManip(m_delay, 1.0f, 1.0f);
        }
        return ERR_CONTINUE;
    }

    if ( m_phase == TPP_TURN )
    {
        m_sound->Play(SOUND_PSHHH2, m_supportPos, 1.0f, 1.4f);
        m_phase    = TPP_DOWN;
        m_progress = 0.0f;
        m_delay    = m_timeDown;
        m_time     = 0.0f;
        m_lastParticule = 0.0f;
        return ERR_CONTINUE;
    }

    Abort();
    return ERR_STOP;
}

// Suddenly ends the current action.

BOOL CTaskPen::Abort()
{
//? m_camera->StopCentering(m_object, 0.5f);
    return TRUE;
}


// Plays the sound of the manipulator arm.

void CTaskPen::SoundManip(float time, float amplitude, float frequency)
{
    int     i;

    i = m_sound->Play(SOUND_MANIP, m_object->RetPosition(0), 0.0f, 0.3f*frequency, TRUE);
    m_sound->AddEnvelope(i, 0.5f*amplitude, 1.0f*frequency, 0.1f, SOPER_CONTINUE);
    m_sound->AddEnvelope(i, 0.5f*amplitude, 1.0f*frequency, time-0.1f, SOPER_CONTINUE);
    m_sound->AddEnvelope(i, 0.0f, 0.3f*frequency, 0.1f, SOPER_STOP);
}


// Converting a angle to number of pencil.

int CTaskPen::AngleToRank(float angle)
{
//? return (int)(angle/(-45.0f*PI/180.0f));
    angle = -angle;
    angle += (45.0f*PI/180.0f)/2.0f;
    return (int)(angle/(45.0f*PI/180.0f));
}

// Converting a color to the angle of carousel of pencils.

float CTaskPen::ColorToAngle(int color)
{
    return -45.0f*PI/180.0f*ColorToRank(color);
}

// Converting a color number to the pencil (0 .. 7).

int CTaskPen::ColorToRank(int color)
{
    if ( color ==  8 )  return 1;  // yellow
    if ( color ==  7 )  return 2;  // orange
    if ( color ==  5 )  return 2;  // pink
    if ( color ==  4 )  return 3;  // red
    if ( color ==  6 )  return 4;  // purple
    if ( color == 14 )  return 5;  // blue
    if ( color == 15 )  return 5;  // light blue
    if ( color == 12 )  return 6;  // green
    if ( color == 13 )  return 6;  // light green
    if ( color == 10 )  return 7;  // brown
    if ( color ==  9 )  return 7;  // beige
    return 0;  // black
}

