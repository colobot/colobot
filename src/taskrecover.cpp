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

// taskrecover.cpp

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
#include "displaytext.h"
#include "task.h"
#include "taskrecover.h"


#define ENERGY_RECOVER  0.25f       // energy consumed by recovery
#define RECOVER_DIST    11.8f



// Object's constructor.

CTaskRecover::CTaskRecover(CInstanceManager* iMan, CObject* object)
                               : CTask(iMan, object)
{
    m_ruin = 0;
    m_soundChannel = -1;
}

// Object's constructor.

CTaskRecover::~CTaskRecover()
{
}


// Management of an event.

BOOL CTaskRecover::EventProcess(const Event &event)
{
    CObject*    power;
    D3DVECTOR   pos, speed;
    FPOINT      dim;
    float       a, g, cirSpeed, angle, energy, dist, linSpeed;

    if ( m_engine->RetPause() )  return TRUE;
    if ( event.event != EVENT_FRAME )  return TRUE;
    if ( m_bError )  return FALSE;

    if ( m_phase == TRP_TURN )  // preliminary rotation?
    {
        a = m_object->RetAngleY(0);
        g = m_angle;
        cirSpeed = Direction(a, g)*1.0f;
        if ( cirSpeed >  1.0f )  cirSpeed =  1.0f;
        if ( cirSpeed < -1.0f )  cirSpeed = -1.0f;

        m_physics->SetMotorSpeedZ(cirSpeed);  // turns left / right
        return TRUE;
    }

    m_progress += event.rTime*m_speed;  // others advance
    m_time += event.rTime;

    if ( m_phase == TRP_DOWN )
    {
        angle = Prop(126, -10, m_progress);
        m_object->SetAngleZ(2, angle);
        m_object->SetAngleZ(4, angle);

        angle = Prop(-144, 0, m_progress);
        m_object->SetAngleZ(3, angle);
        m_object->SetAngleZ(5, angle);
    }

    if ( m_phase == TRP_MOVE )  // preliminary forward/backward?
    {
        dist = Length(m_object->RetPosition(0), m_ruin->RetPosition(0));
        linSpeed = 0.0f;
        if ( dist > RECOVER_DIST )  linSpeed =  1.0f;
        if ( dist < RECOVER_DIST )  linSpeed = -1.0f;
        m_physics->SetMotorSpeedX(linSpeed);  // forward/backward
        return TRUE;
    }

    if ( m_phase == TRP_OPER )
    {
        power = m_object->RetPower();
        if ( power != 0 )
        {
            energy = power->RetEnergy();
            power->SetEnergy(energy-ENERGY_RECOVER*event.rTime*m_speed);
        }

        speed.x = (Rand()-0.5f)*0.1f*m_progress;
        speed.y = (Rand()-0.5f)*0.1f*m_progress;
        speed.z = (Rand()-0.5f)*0.1f*m_progress;
        m_ruin->SetCirVibration(speed);

        if ( m_progress >= 0.75f )
        {
            m_ruin->SetZoom(0, 1.0f-(m_progress-0.75f)/0.25f);
        }

        if ( m_progress > 0.5f && m_progress < 0.8f )
        {
            m_metal->SetZoom(0, (m_progress-0.5f)/0.3f);
        }

        if ( m_lastParticule+m_engine->ParticuleAdapt(0.02f) <= m_time )
        {
            m_lastParticule = m_time;

            pos = m_recoverPos;
            pos.x += (Rand()-0.5f)*8.0f*(1.0f-m_progress);
            pos.z += (Rand()-0.5f)*8.0f*(1.0f-m_progress);
            pos.y -= 4.0f;
            speed.x = (Rand()-0.5f)*0.0f;
            speed.z = (Rand()-0.5f)*0.0f;
            speed.y = Rand()*15.0f;
            dim.x = Rand()*2.0f+1.5f;
            dim.y = dim.x;
            m_particule->CreateParticule(pos, speed, dim, PARTIRECOVER, 1.0f, 0.0f, 0.0f);
        }
    }

    if ( m_phase == TRP_UP )
    {
        angle = Prop(-10, 126, m_progress);
        m_object->SetAngleZ(2, angle);
        m_object->SetAngleZ(4, angle);

        angle = Prop(0, -144, m_progress);
        m_object->SetAngleZ(3, angle);
        m_object->SetAngleZ(5, angle);

        if ( m_lastParticule+m_engine->ParticuleAdapt(0.02f) <= m_time )
        {
            m_lastParticule = m_time;

            pos = m_recoverPos;
            pos.y -= 4.0f;
            speed.x = (Rand()-0.5f)*0.0f;
            speed.z = (Rand()-0.5f)*0.0f;
            speed.y = Rand()*15.0f;
            dim.x = Rand()*2.0f+1.5f;
            dim.y = dim.x;
            m_particule->CreateParticule(pos, speed, dim, PARTIRECOVER, 1.0f, 0.0f, 0.0f);
        }
    }

    return TRUE;
}


// Assigns the goal was achieved.

Error CTaskRecover::Start()
{
    CObject*    power;
    D3DMATRIX*  mat;
    D3DVECTOR   pos, iPos, oPos;
    float       energy;

    ObjectType  type;

    m_bError = TRUE;  // operation impossible
    if ( !m_physics->RetLand() )  return ERR_RECOVER_VEH;

    type = m_object->RetType();
    if ( type != OBJECT_MOBILErr )  return ERR_RECOVER_VEH;

    power = m_object->RetPower();
    if ( power == 0 )  return ERR_RECOVER_ENERGY;
    energy = power->RetEnergy();
    if ( energy < ENERGY_RECOVER/power->RetCapacity()+0.05f )  return ERR_RECOVER_ENERGY;

    mat = m_object->RetWorldMatrix(0);
    pos = D3DVECTOR(RECOVER_DIST, 3.3f, 0.0f);
    pos = Transform(*mat, pos);  // position in front
    m_recoverPos = pos;

    m_ruin = SearchRuin();
    if ( m_ruin == 0 )  return ERR_RECOVER_NULL;
    m_ruin->SetLock(TRUE);  // ruin no longer usable

    iPos = m_object->RetPosition(0);
    oPos = m_ruin->RetPosition(0);
    m_angle = RotateAngle(oPos.x-iPos.x, iPos.z-oPos.z);  // CW !

    m_metal = 0;

    m_phase    = TRP_TURN;
    m_progress = 0.0f;
    m_speed    = 1.0f/1.0f;
    m_time     = 0.0f;
    m_lastParticule = 0.0f;

    m_bError = FALSE;  // ok

    m_camera->StartCentering(m_object, PI*0.85f, 99.9f, 10.0f, 3.0f);
    return ERR_OK;
}

// Indicates whether the action is finished.

Error CTaskRecover::IsEnded()
{
    D3DMATRIX*  mat;
    D3DVECTOR   pos, speed, goal;
    FPOINT      dim;
    float       angle, dist, time;
    int         i;

    if ( m_engine->RetPause() )  return ERR_CONTINUE;
    if ( m_bError )  return ERR_STOP;

    if ( m_phase == TRP_TURN )  // preliminary rotation?
    {
        angle = m_object->RetAngleY(0);
        angle = NormAngle(angle);  // 0..2*PI

        if ( TestAngle(angle, m_angle-PI*0.01f, m_angle+PI*0.01f) )
        {
            m_physics->SetMotorSpeedZ(0.0f);

            dist = Length(m_object->RetPosition(0), m_ruin->RetPosition(0));
            if ( dist > RECOVER_DIST )
            {
                time = m_physics->RetLinTimeLength(dist-RECOVER_DIST, 1.0f);
                m_speed = 1.0f/time;
            }
            else
            {
                time = m_physics->RetLinTimeLength(RECOVER_DIST-dist, -1.0f);
                m_speed = 1.0f/time;
            }
            m_phase = TRP_MOVE;
            m_progress = 0.0f;
        }
        return ERR_CONTINUE;
    }

    if ( m_phase == TRP_MOVE )  // preliminary advance?
    {
        dist = Length(m_object->RetPosition(0), m_ruin->RetPosition(0));

        if ( dist >= RECOVER_DIST-1.0f &&
             dist <= RECOVER_DIST+1.0f )
        {
            m_physics->SetMotorSpeedX(0.0f);

            mat = m_object->RetWorldMatrix(0);
            pos = D3DVECTOR(RECOVER_DIST, 3.3f, 0.0f);
            pos = Transform(*mat, pos);  // position in front
            m_recoverPos = pos;

            i = m_sound->Play(SOUND_MANIP, m_object->RetPosition(0), 0.0f, 0.9f, TRUE);
            m_sound->AddEnvelope(i, 1.0f, 1.5f, 0.3f, SOPER_CONTINUE);
            m_sound->AddEnvelope(i, 1.0f, 1.5f, 1.0f, SOPER_CONTINUE);
            m_sound->AddEnvelope(i, 0.0f, 0.9f, 0.3f, SOPER_STOP);

            m_phase    = TRP_DOWN;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.5f;
            m_time     = 0.0f;
        }
        else
        {
            if ( m_progress > 1.0f )  // timeout?
            {
                m_ruin->SetLock(FALSE);  // usable again
                m_camera->StopCentering(m_object, 2.0f);
                return ERR_RECOVER_NULL;
            }
        }
        return ERR_CONTINUE;
    }

    if ( m_progress < 1.0f )  return ERR_CONTINUE;
    m_progress = 0.0f;

    if ( m_phase == TRP_DOWN )
    {
        m_metal = new CObject(m_iMan);
        if ( !m_metal->CreateResource(m_recoverPos, 0.0f, OBJECT_METAL) )
        {
            delete m_metal;
            m_metal = 0;
            Abort();
            m_bError = TRUE;
            m_displayText->DisplayError(ERR_TOOMANY, m_object);
            return ERR_STOP;
        }
        m_metal->SetLock(TRUE);  // metal not yet usable
        m_metal->SetZoom(0, 0.0f);

        mat = m_object->RetWorldMatrix(0);
        pos = D3DVECTOR(RECOVER_DIST, 3.1f, 3.9f);
        pos = Transform(*mat, pos);
        goal = D3DVECTOR(RECOVER_DIST, 3.1f, -3.9f);
        goal = Transform(*mat, goal);
        m_particule->CreateRay(pos, goal, PARTIRAY2,
                               FPOINT(2.0f, 2.0f), 8.0f);

        m_soundChannel = m_sound->Play(SOUND_RECOVER, m_ruin->RetPosition(0), 0.0f, 1.0f, TRUE);
        m_sound->AddEnvelope(m_soundChannel, 0.6f, 1.0f, 2.0f, SOPER_CONTINUE);
        m_sound->AddEnvelope(m_soundChannel, 0.6f, 1.0f, 4.0f, SOPER_CONTINUE);
        m_sound->AddEnvelope(m_soundChannel, 0.0f, 0.7f, 2.0f, SOPER_STOP);

        m_phase = TRP_OPER;
        m_speed = 1.0f/8.0f;
        return ERR_CONTINUE;
    }

    if ( m_phase == TRP_OPER )
    {
        m_metal->SetZoom(0, 1.0f);

        m_ruin->DeleteObject();  // destroys the ruin
        delete m_ruin;
        m_ruin = 0;

        m_soundChannel = -1;

        i = m_sound->Play(SOUND_MANIP, m_object->RetPosition(0), 0.0f, 0.9f, TRUE);
        m_sound->AddEnvelope(i, 1.0f, 1.5f, 0.3f, SOPER_CONTINUE);
        m_sound->AddEnvelope(i, 1.0f, 1.5f, 1.0f, SOPER_CONTINUE);
        m_sound->AddEnvelope(i, 0.0f, 0.9f, 0.3f, SOPER_STOP);

        m_phase = TRP_UP;
        m_speed = 1.0f/1.5f;
        return ERR_CONTINUE;
    }

    m_metal->SetLock(FALSE);  // metal usable

    Abort();
    return ERR_STOP;
}

// Suddenly ends the current action.

BOOL CTaskRecover::Abort()
{
    m_object->SetAngleZ(2,  126.0f*PI/180.0f);
    m_object->SetAngleZ(4,  126.0f*PI/180.0f);
    m_object->SetAngleZ(3, -144.0f*PI/180.0f);
    m_object->SetAngleZ(5, -144.0f*PI/180.0f);  // rest

    if ( m_soundChannel != -1 )
    {
        m_sound->FlushEnvelope(m_soundChannel);
        m_sound->AddEnvelope(m_soundChannel, 1.0f, 1.0f, 1.0f, SOPER_STOP);
        m_soundChannel = -1;
    }

    m_camera->StopCentering(m_object, 2.0f);
    return TRUE;
}


// Seeks if a ruin is in front of the vehicle.

CObject* CTaskRecover::SearchRuin()
{
    CObject     *pObj, *pBest;
    D3DVECTOR   oPos;
    ObjectType  type;
    float       dist, min;
    int         i;

    pBest = 0;
    min = 100000.0f;
    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        type = pObj->RetType();
        if ( type == OBJECT_RUINmobilew1 ||
             type == OBJECT_RUINmobilew2 ||
             type == OBJECT_RUINmobilet1 ||
             type == OBJECT_RUINmobilet2 ||
             type == OBJECT_RUINmobiler1 ||
             type == OBJECT_RUINmobiler2 )  // vehicle in ruin?
        {
            oPos = pObj->RetPosition(0);
            dist = Length(oPos, m_recoverPos);
            if ( dist > 40.0f )  continue;

            if ( dist < min )
            {
                min = dist;
                pBest = pObj;
            }
        }

    }
    return pBest;
}

