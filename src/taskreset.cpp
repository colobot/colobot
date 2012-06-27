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

// taskreset.cpp

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
#include "particule.h"
#include "terrain.h"
#include "object.h"
#include "physics.h"
#include "brain.h"
#include "sound.h"
#include "robotmain.h"
#include "task.h"
#include "taskreset.h"



#define RESET_DELAY_ZOOM    0.7f
#define RESET_DELAY_MOVE    0.7f




// Object's constructor.

CTaskReset::CTaskReset(CInstanceManager* iMan, CObject* object)
                     : CTask(iMan, object)
{
}

// Object's destructor.

CTaskReset::~CTaskReset()
{
}


// Management of an event.

BOOL CTaskReset::EventProcess(const Event &event)
{
    D3DVECTOR   pos, speed;
    FPOINT      dim;
    float       angle, duration;

    if ( m_engine->RetPause() )  return TRUE;
    if ( event.event != EVENT_FRAME )  return TRUE;
    if ( m_bError )  return FALSE;

    m_time += event.rTime;
    m_progress += event.rTime*m_speed;

    if ( m_phase == TRSP_ZOUT )
    {
        angle = m_iAngle;
        angle += powf(m_progress*5.0f, 2.0f);  // accelerates
        m_object->SetAngleY(0, angle);
        m_object->SetZoom(0, 1.0f-m_progress);

        if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
        {
            m_lastParticule = m_time;

            pos = m_begin;
            pos.x += (Rand()-0.5f)*5.0f;
            pos.z += (Rand()-0.5f)*5.0f;
            speed.x = 0.0f;
            speed.z = 0.0f;
            speed.y = 5.0f+Rand()*5.0f;
            dim.x = Rand()*2.0f+2.0f;
            dim.y = dim.x;
            m_particule->CreateParticule(pos, speed, dim, PARTIGLINTb, 2.0f);

            pos = m_begin;
            speed.x = (Rand()-0.5f)*20.0f;
            speed.z = (Rand()-0.5f)*20.0f;
            speed.y = Rand()*10.0f;
            speed *= 1.0f-m_progress*0.5f;
            pos += speed*1.5f;
            speed = -speed;
            dim.x = 0.6f;
            dim.y = dim.x;
            pos.y += dim.y;
            duration = Rand()*1.5f+1.5f;
            m_particule->CreateTrack(pos, speed, dim, PARTITRACK6,
                                     duration, 0.0f,
                                     duration*0.9f, 0.7f);
        }
    }

    if ( m_phase == TRSP_MOVE )
    {
        pos = m_begin+(m_goal-m_begin)*m_progress;
        m_object->SetPosition(0, pos);

        if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
        {
            m_lastParticule = m_time;

            pos.x += (Rand()-0.5f)*5.0f;
            pos.z += (Rand()-0.5f)*5.0f;
            speed.x = 0.0f;
            speed.z = 0.0f;
            speed.y = 2.0f+Rand()*2.0f;
            dim.x = Rand()*2.0f+2.0f;
            dim.y = dim.x;
            m_particule->CreateParticule(pos, speed, dim, PARTIGLINTb, 2.0f);
        }
    }

    if ( m_phase == TRSP_ZIN )
    {
        angle = m_angle.y;
        angle += -powf((1.0f-m_progress)*5.0f, 2.0f);  // slows
        m_object->SetAngleY(0, angle);
        m_object->SetZoom(0, m_progress);

        if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
        {
            m_lastParticule = m_time;

            pos = m_goal;
            pos.x += (Rand()-0.5f)*5.0f;
            pos.z += (Rand()-0.5f)*5.0f;
            speed.x = 0.0f;
            speed.z = 0.0f;
            speed.y = 5.0f+Rand()*5.0f;
            dim.x = Rand()*2.0f+2.0f;
            dim.y = dim.x;
            m_particule->CreateParticule(pos, speed, dim, PARTIGLINTb, 2.0f);

            pos = m_goal;
            speed.x = (Rand()-0.5f)*20.0f;
            speed.z = (Rand()-0.5f)*20.0f;
            speed.y = Rand()*10.0f;
            speed *= 0.5f+m_progress*0.5f;
            dim.x = 0.6f;
            dim.y = dim.x;
            pos.y += dim.y;
            duration = Rand()*1.5f+1.5f;
            m_particule->CreateTrack(pos, speed, dim, PARTITRACK6,
                                     duration, 0.0f,
                                     duration*0.9f, 0.7f);
        }
    }

    return TRUE;
}


// Assigns the goal was achieved.
// A positive angle makes a turn right.

Error CTaskReset::Start(D3DVECTOR goal, D3DVECTOR angle)
{
    CObject*    fret;
    int     i;

    fret = m_object->RetFret();
    if ( fret != 0 && fret->RetResetCap() == RESET_MOVE )
    {
        fret->SetTruck(0);
        m_object->SetFret(0);  // does nothing
    }

    if ( !m_main->RetNiceReset() )  // quick return?
    {
        m_object->SetPosition(0, goal);
        m_object->SetAngle(0, angle);
        m_brain->RunProgram(m_object->RetResetRun());

        m_bError = FALSE;
        return ERR_OK;
    }

    m_begin = m_object->RetPosition(0);
    m_goal = goal;
    m_angle = angle;

    if ( SearchVehicle() )  // starting location occupied?
    {
        m_bError = TRUE;
        return ERR_RESET_NEAR;
    }

    m_iAngle = m_object->RetAngleY(0);
    m_time = 0.0f;
    m_phase = TRSP_ZOUT;
    m_speed = 1.0f/RESET_DELAY_ZOOM;
    m_progress = 0.0f;
    m_lastParticule = 0.0f;

    m_object->SetResetBusy(TRUE);

    i = m_sound->Play(SOUND_GGG, m_begin, 1.0f, 2.0f, TRUE);
    m_sound->AddEnvelope(i, 0.0f, 0.5f, RESET_DELAY_ZOOM, SOPER_STOP);

    m_bError = FALSE;
    return ERR_OK;
}

// Indicates whether the action is finished.

Error CTaskReset::IsEnded()
{
    CObject*    power;
    float       dist;
    int         i;

    if ( !m_main->RetNiceReset() )  // quick return?
    {
        return ERR_STOP;
    }

    if ( m_engine->RetPause() )  return ERR_CONTINUE;
    if ( m_bError )  return ERR_STOP;
    if ( m_progress < 1.0f )  return ERR_CONTINUE;

    if ( m_phase == TRSP_ZOUT )
    {
        dist = Length(m_begin, m_goal);
        m_phase = TRSP_MOVE;
        m_speed = 1.0f/(dist*RESET_DELAY_MOVE/100.0f);
        m_progress = 0.0f;
        return ERR_CONTINUE;
    }

    if ( m_phase == TRSP_MOVE )
    {
        m_object->SetPosition(0, m_goal);
        m_object->SetAngle(0, m_angle);

        i = m_sound->Play(SOUND_GGG, m_goal, 1.0f, 0.5f, TRUE);
        m_sound->AddEnvelope(i, 0.0f, 2.0f, RESET_DELAY_ZOOM, SOPER_STOP);

        m_phase = TRSP_ZIN;
        m_speed = 1.0f/RESET_DELAY_ZOOM;
        m_progress = 0.0f;
        return ERR_CONTINUE;
    }

    m_object->SetAngle(0, m_angle);
    m_object->SetZoom(0, 1.0f);

    power = m_object->RetPower();
    if ( power != 0 )
    {
        power->SetEnergy(power->RetCapacity());  // refueling
    }

    m_brain->RunProgram(m_object->RetResetRun());
    m_object->SetResetBusy(FALSE);
    return ERR_STOP;
}


// Seeks if a vehicle is too close.

BOOL CTaskReset::SearchVehicle()
{
    CObject*    pObj;
    D3DVECTOR   oPos;
    ObjectType  type;
    float       oRadius, dist;
    int         i;

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        if ( pObj == m_object )  continue;

        type = pObj->RetType();
        if ( type != OBJECT_HUMAN    &&
             type != OBJECT_TECH     &&
             type != OBJECT_MOBILEfa &&
             type != OBJECT_MOBILEta &&
             type != OBJECT_MOBILEwa &&
             type != OBJECT_MOBILEia &&
             type != OBJECT_MOBILEfc &&
             type != OBJECT_MOBILEtc &&
             type != OBJECT_MOBILEwc &&
             type != OBJECT_MOBILEic &&
             type != OBJECT_MOBILEfi &&
             type != OBJECT_MOBILEti &&
             type != OBJECT_MOBILEwi &&
             type != OBJECT_MOBILEii &&
             type != OBJECT_MOBILEfs &&
             type != OBJECT_MOBILEts &&
             type != OBJECT_MOBILEws &&
             type != OBJECT_MOBILEis &&
             type != OBJECT_MOBILErt &&
             type != OBJECT_MOBILErc &&
             type != OBJECT_MOBILErr &&
             type != OBJECT_MOBILErs &&
             type != OBJECT_MOBILEsa &&
             type != OBJECT_MOBILEtg &&
             type != OBJECT_MOBILEft &&
             type != OBJECT_MOBILEtt &&
             type != OBJECT_MOBILEwt &&
             type != OBJECT_MOBILEit &&
             type != OBJECT_MOBILEdr &&
             type != OBJECT_MOTHER   &&
             type != OBJECT_ANT      &&
             type != OBJECT_SPIDER   &&
             type != OBJECT_BEE      &&
             type != OBJECT_WORM     )  continue;

        if ( !pObj->GetCrashSphere(0, oPos, oRadius) )  continue;
        dist = Length(oPos, m_goal)-oRadius;

        if ( dist < 5.0f )  return TRUE;
    }

    return FALSE;
}

