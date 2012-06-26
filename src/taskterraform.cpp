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

// taskterraform.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "language.h"
#include "math3d.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "particule.h"
#include "terrain.h"
#include "object.h"
#include "physics.h"
#include "pyro.h"
#include "brain.h"
#include "camera.h"
#include "sound.h"
#include "motion.h"
#include "motionant.h"
#include "motionspider.h"
#include "task.h"
#include "taskterraform.h"


#define ENERGY_TERRA    0.40f       // energy consumed by blow
#define ACTION_RADIUS   400.0f



// Object's constructor.

CTaskTerraform::CTaskTerraform(CInstanceManager* iMan, CObject* object)
                               : CTask(iMan, object)
{
    CTask::CTask(iMan, object);
    m_lastParticule = 0.0f;
    m_soundChannel = -1;
}

// Object's destructor.

CTaskTerraform::~CTaskTerraform()
{
}


// Management of an event.

BOOL CTaskTerraform::EventProcess(const Event &event)
{
    CObject*    power;
    D3DMATRIX*  mat;
    D3DVECTOR   pos, dir, speed;
    FPOINT      dim;
    float       energy;

    if ( m_engine->RetPause() )  return TRUE;
    if ( event.event != EVENT_FRAME )  return TRUE;
    if ( m_bError )  return FALSE;

    m_progress += event.rTime*m_speed;  // others advance
    m_time += event.rTime;

    if ( m_phase == TTP_CHARGE )
    {
        if ( m_soundChannel == -1 )
        {
#if _TEEN
            m_soundChannel = m_sound->Play(SOUND_GGG, m_object->RetPosition(0), 1.0f, 0.5f, TRUE);
            m_sound->AddEnvelope(m_soundChannel, 1.0f, 2.0f, 1.5f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 0.0f, 0.5f, 0.5f, SOPER_STOP);
#else
            m_soundChannel = m_sound->Play(SOUND_GGG, m_object->RetPosition(0), 1.0f, 0.5f, TRUE);
            m_sound->AddEnvelope(m_soundChannel, 1.0f, 2.0f, 4.0f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 0.0f, 0.5f, 0.5f, SOPER_STOP);
#endif
        }

        dir.x = 0.0f;
        dir.y = (Rand()-0.5f)*0.2f*m_progress;
        dir.z = 0.0f;
        m_object->SetCirVibration(dir);

        m_object->SetZoom(0, 1.0f+m_progress*0.2f);

        power = m_object->RetPower();
        if ( power != 0 )
        {
            power->SetZoom(0, 1.0f+m_progress*1.0f);

            energy = power->RetEnergy();
            energy -= event.rTime*ENERGY_TERRA/power->RetCapacity()/4.0f;
            if ( energy < 0.0f )  energy = 0.0f;
            power->SetEnergy(energy);
        }
    }

    if ( m_phase == TTP_DOWN )
    {
        pos.x = 9.0f;
#if _TEEN
        pos.y = 4.0f-m_progress*4.0f;
#else
        pos.y = 4.0f-m_progress*5.8f;
#endif
        pos.z = 0.0f;
        m_object->SetPosition(2, pos);
    }

    if ( m_phase == TTP_UP )
    {
        pos.x = 9.0f;
#if _TEEN
        pos.y = 4.0f-(1.0f-m_progress)*4.0f;
#else
        pos.y = 4.0f-(1.0f-m_progress)*5.8f;
#endif
        pos.z = 0.0f;
        m_object->SetPosition(2, pos);
    }

    dir.x = 0.0f;
    dir.y = 0.0f;
    dir.z = 0.0f;
    pos = m_object->RetPosition(2);
    if ( pos.y < 0.0f )
    {
        dir.z = -atanf((pos.y/2.0f)/9.0f);
    }
    m_object->SetInclinaison(dir);

    if ( m_time-m_lastParticule >= m_engine->ParticuleAdapt(0.05f) )
    {
        m_lastParticule = m_time;

        mat = m_object->RetWorldMatrix(0);

        if ( m_phase == TTP_CHARGE )
        {
            // Battery.
            pos = D3DVECTOR(-6.0f, 5.5f+2.0f*m_progress, 0.0f);
            pos.x += (Rand()-0.5f)*1.0f;
            pos.z += (Rand()-0.5f)*1.0f;
            pos   = Transform(*mat, pos);
            speed.x = (Rand()-0.5f)*6.0f*(1.0f+m_progress*4.0f);
            speed.z = (Rand()-0.5f)*6.0f*(1.0f+m_progress*4.0f);
            speed.y = 6.0f+Rand()*4.0f*(1.0f+m_progress*2.0f);
            dim.x = 0.5f+1.5f*m_progress;
            dim.y = dim.x;
            m_particule->CreateParticule(pos, speed, dim, PARTIBLITZ, 2.0f, 20.0f);
        }

        if ( m_phase != TTP_CHARGE )
        {
            // Left grid.
            pos = D3DVECTOR(-1.0f, 5.8f, 3.5f);
            pos.x += (Rand()-0.5f)*1.0f;
            pos.z += (Rand()-0.5f)*1.0f;
            pos   = Transform(*mat, pos);
            speed.x = Rand()*4.0f;
            speed.z = Rand()*2.0f;
            speed.y = 2.5f+Rand()*1.0f;
            speed = Transform(*mat, speed);
            speed -= m_object->RetPosition(0);
            dim.x = Rand()*1.0f+1.0f;
            dim.y = dim.x;
            m_particule->CreateParticule(pos, speed, dim, PARTISMOKE1, 3.0f);

            // Right grid.
            pos = D3DVECTOR(-1.0f, 5.8f, -3.5f);
            pos.x += (Rand()-0.5f)*1.0f;
            pos.z += (Rand()-0.5f)*1.0f;
            pos   = Transform(*mat, pos);
            speed.x =  Rand()*4.0f;
            speed.z = -Rand()*2.0f;
            speed.y = 2.5f+Rand()*1.0f;
            speed = Transform(*mat, speed);
            speed -= m_object->RetPosition(0);
            dim.x = Rand()*1.0f+1.0f;
            dim.y = dim.x;
            m_particule->CreateParticule(pos, speed, dim, PARTISMOKE1, 3.0f);
        }
    }

    return TRUE;
}


// Assigns the goal was achieved.

Error CTaskTerraform::Start()
{
    CObject*    power;
    D3DMATRIX*  mat;
    D3DVECTOR   pos, speed;
    float       energy;

    ObjectType  type;

    m_bError = TRUE;  // operation impossible
    if ( !m_physics->RetLand() )  return ERR_TERRA_VEH;

    type = m_object->RetType();
    if ( type != OBJECT_MOBILErt )  return ERR_TERRA_VEH;

    power = m_object->RetPower();
    if ( power == 0 )  return ERR_TERRA_ENERGY;
    energy = power->RetEnergy();
    if ( energy < ENERGY_TERRA/power->RetCapacity()+0.05f )  return ERR_TERRA_ENERGY;

    speed = m_physics->RetMotorSpeed();
    if ( speed.x != 0.0f ||
         speed.z != 0.0f )  return ERR_MANIP_MOTOR;

    mat = m_object->RetWorldMatrix(0);
    pos = D3DVECTOR(9.0f, 0.0f, 0.0f);
    pos = Transform(*mat, pos);  // battery position
    m_terraPos = pos;

    m_phase    = TTP_CHARGE;
    m_progress = 0.0f;
#if _TEEN
    m_speed    = 1.0f/1.5f;
#else
    m_speed    = 1.0f/4.0f;
#endif
    m_time     = 0.0f;

    m_bError = FALSE;  // ok

    m_camera->StartCentering(m_object, PI*0.35f, 99.9f, 20.0f, 2.0f);
    return ERR_OK;
}

// Indicates whether the action is finished.

Error CTaskTerraform::IsEnded()
{
    CObject*    power;
    D3DVECTOR   pos, speed;
    FPOINT      dim;
    float       dist, duration;
    int         i, max;

    if ( m_engine->RetPause() )  return ERR_CONTINUE;
    if ( m_bError )  return ERR_STOP;

    if ( m_progress < 1.0f )  return ERR_CONTINUE;
    m_progress = 0.0f;

    if ( m_phase == TTP_CHARGE )
    {
#if _TEEN
        Terraform();  // changes the terrain.
#endif

        m_phase = TTP_DOWN;
        m_speed = 1.0f/0.2f;
        return ERR_CONTINUE;
    }

    if ( m_phase == TTP_DOWN )
    {
#if !_TEEN
        Terraform();  // changes the terrain.
#endif

        m_object->SetCirVibration(D3DVECTOR(0.0f, 0.0f, 0.0f));
        m_object->SetZoom(0, 1.0f);

        power = m_object->RetPower();
        if ( power != 0 )
        {
            power->SetZoom(0, 1.0f);
        }

        max= (int)(50.0f*m_engine->RetParticuleDensity());
        for ( i=0 ; i<max ; i++ )
        {
            pos.x = m_terraPos.x+(Rand()-0.5f)*80.0f;
            pos.z = m_terraPos.z+(Rand()-0.5f)*80.0f;
            pos.y = m_terraPos.y;
            m_terrain->MoveOnFloor(pos);
            dist = Length(pos, m_terraPos);
            speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
            dim.x = 2.0f+(40.0f-dist)/(1.0f+Rand()*4.0f);
            dim.y = dim.x;
            m_particule->CreateParticule(pos, speed, dim, PARTICRASH, 2.0f);

            pos = m_terraPos;
            speed.x = (Rand()-0.5f)*40.0f;
            speed.z = (Rand()-0.5f)*40.0f;
            speed.y = Rand()*15.0f+15.0f;
            dim.x = 0.6f;
            dim.y = dim.x;
            pos.y += dim.y;
            duration = Rand()*3.0f+3.0f;
            m_particule->CreateTrack(pos, speed, dim, PARTITRACK5,
                                     duration, Rand()*10.0f+15.0f,
                                     duration*0.2f, 1.0f);
        }

        m_phase = TTP_TERRA;
        m_speed = 1.0f/2.0f;
        return ERR_CONTINUE;
    }

    if ( m_phase == TTP_TERRA )
    {
        m_phase = TTP_UP;
        m_speed = 1.0f/1.0f;
        return ERR_CONTINUE;
    }

    Abort();
    return ERR_STOP;
}

// Suddenly ends the current action.

BOOL CTaskTerraform::Abort()
{
    CObject*    power;

    if ( m_soundChannel != -1 )
    {
        m_sound->FlushEnvelope(m_soundChannel);
        m_sound->AddEnvelope(m_soundChannel, 0.0f, 0.5f, 0.3f, SOPER_STOP);
        m_soundChannel = -1;
    }

    m_object->SetPosition(2, D3DVECTOR(9.0f, 4.0f, 0.0f));
    m_object->SetInclinaison(D3DVECTOR(0.0f, 0.0f, 0.0f));
    m_object->SetCirVibration(D3DVECTOR(0.0f, 0.0f, 0.0f));
    m_object->SetZoom(0, 1.0f);

    power = m_object->RetPower();
    if ( power != 0 )
    {
        power->SetZoom(0, 1.0f);
    }

    m_camera->StopCentering(m_object, 2.0f);
    return TRUE;
}


// Returns all the close ants and spiders.

BOOL CTaskTerraform::Terraform()
{
    CObject*    pObj;
    CBrain*     brain;
    CMotion*    motion;
    CPyro*      pyro;
    ObjectType  type;
    float       dist;
    int         i;

    m_camera->StartEffect(CE_TERRAFORM, m_terraPos, 1.0f);

    m_sound->Play(SOUND_THUMP, m_terraPos);

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        type = pObj->RetType();
        if ( type == OBJECT_NULL )  continue;

        if ( type == OBJECT_TEEN34 )  // stone?
        {
            dist = Length(m_terraPos, pObj->RetPosition(0));
            if ( dist > 20.0f )  continue;

            pyro = new CPyro(m_iMan);
            pyro->Create(PT_FRAGT, pObj);
        }
        else
        {
            motion = pObj->RetMotion();
            if ( motion == 0 )  continue;

            dist = Length(m_terraPos, pObj->RetPosition(0));
            if ( dist > ACTION_RADIUS )  continue;

            if ( type == OBJECT_ANT )
            {
                brain = pObj->RetBrain();
                if ( brain != 0 )  brain->StopTask();
                motion->SetAction(MAS_BACK1, 0.8f+Rand()*0.3f);
                pObj->SetFixed(TRUE);  // not moving
            }
            if ( type == OBJECT_SPIDER )
            {
                brain = pObj->RetBrain();
                if ( brain != 0 )  brain->StopTask();
                motion->SetAction(MSS_BACK1, 0.8f+Rand()*0.3f);
                pObj->SetFixed(TRUE);  // not moving
            }
        }
    }

    return TRUE;
}

