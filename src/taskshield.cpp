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

// taskshield.cpp

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
#include "light.h"
#include "sound.h"
#include "task.h"
#include "taskshield.h"


#define ENERGY_TIME     20.0f       // maximum duration if full battery



// Object's constructor.

CTaskShield::CTaskShield(CInstanceManager* iMan, CObject* object)
                               : CTask(iMan, object)
{
    CTask::CTask(iMan, object);

    m_rankSphere = -1;
    m_soundChannel = -1;
    m_effectLight = -1;
}

// Object's destructor.

CTaskShield::~CTaskShield()
{
    Abort();
}


// Management of an event.

BOOL CTaskShield::EventProcess(const Event &event)
{
    CObject*        power;
    D3DMATRIX*      mat;
    D3DMATRIX       matrix;
    D3DVECTOR       pos, speed, goal, angle;
    D3DCOLORVALUE   color;
    FPOINT          dim;
    float           energy;

    if ( m_engine->RetPause() )  return TRUE;
    if ( event.event != EVENT_FRAME )  return TRUE;
    if ( m_bError )  return FALSE;

    m_progress += event.rTime*m_speed;  // others advance
    m_time += event.rTime;
    m_delay -= event.rTime;

    mat = m_object->RetWorldMatrix(0);
    pos = D3DVECTOR(7.0f, 15.0f, 0.0f);
    pos = Transform(*mat, pos);  // sphere position
    m_shieldPos = pos;

    if ( m_rankSphere != -1 )
    {
        m_particule->SetPosition(m_rankSphere, m_shieldPos);
        dim.x = RetRadius();
        dim.y = dim.x;
        m_particule->SetDimension(m_rankSphere, dim);
    }

    if ( m_phase == TS_UP1 )
    {
        pos.x = 7.0f;
        pos.y = 4.5f+Bounce(m_progress)*3.0f;
        pos.z = 0.0f;
        m_object->SetPosition(2, pos);
    }

    if ( m_phase == TS_UP2 )
    {
        pos.x = 0.0f;
        pos.y = 1.0f+Bounce(m_progress)*3.0f;
        pos.z = 0.0f;
        m_object->SetPosition(3, pos);
    }

    if ( m_phase == TS_SHIELD )
    {
        energy = (1.0f/ENERGY_TIME)*event.rTime;
        energy *= RetRadius()/RADIUS_SHIELD_MAX;
        power = m_object->RetPower();
        if ( power != 0 )
        {
            power->SetEnergy(power->RetEnergy()-energy/power->RetCapacity());
        }
        m_energyUsed += energy;

        if ( m_soundChannel == -1 )
        {
            m_soundChannel = m_sound->Play(SOUND_SHIELD, m_shieldPos, 0.5f, 0.5f, TRUE);
            m_sound->AddEnvelope(m_soundChannel, 1.0f, 1.0f, 2.0f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 1.0f, 1.0f, 1.0f, SOPER_LOOP);
        }
        else
        {
            m_sound->Position(m_soundChannel, m_shieldPos);
        }

        pos = m_shieldPos;
        pos.y += RetRadius()*(2.0f+sinf(m_time*9.0f)*0.2f);
        if ( m_effectLight == -1 )
        {
            CreateLight(pos);
        }
        else
        {
            m_light->SetLightPos(m_effectLight, pos);

            color.r = 0.0f+sinf(m_time*33.2f)*0.2f;
            color.g = 0.5f+sinf(m_time*20.0f)*0.5f;
            color.b = 0.5f+sinf(m_time*21.3f)*1.0f;
            color.a = 0.0f;
            m_light->SetLightColor(m_effectLight, color);
        }

        if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
        {
            m_lastParticule = m_time;

            pos = m_shieldPos;
            pos.x += (Rand()-0.5f)*5.0f;
            pos.z += (Rand()-0.5f)*5.0f;
            speed.x = (Rand()-0.5f)*0.0f;
            speed.z = (Rand()-0.5f)*0.0f;
            speed.y = Rand()*15.0f;
            dim.x = Rand()*6.0f+4.0f;
            dim.y = dim.x;
            m_particule->CreateParticule(pos, speed, dim, PARTIBLUE, 1.0f, 0.0f, 0.0f);
        }

        if ( m_lastRay+m_engine->ParticuleAdapt(0.05f) <= m_time )
        {
            m_lastRay = m_time;

            pos = m_shieldPos;
            dim.x = RetRadius()/20.0f;
            dim.y = dim.x;
            angle.x = (Rand()-0.5f)*PI*1.2f;
            angle.y = 0.0f;
            angle.z = (Rand()-0.5f)*PI*1.2f;
            MatRotateXZY(matrix, angle);
            goal = Transform(matrix, D3DVECTOR(0.0f, RetRadius()-dim.x, 0.0f));
            goal += pos;
            m_particule->CreateRay(pos, goal, PARTIRAY2, dim, 0.3f);
        }

        if ( m_lastIncrease+0.2f <= m_time )
        {
            m_lastIncrease = m_time;
            IncreaseShield();
        }
    }

    if ( m_phase == TS_SMOKE )
    {
        if ( m_soundChannel != -1 )
        {
            m_sound->FlushEnvelope(m_soundChannel);
            m_sound->AddEnvelope(m_soundChannel, 0.5f, 0.5f, 2.0f, SOPER_STOP);
            m_soundChannel = -1;
        }

        if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
        {
            m_lastParticule = m_time;

            pos = m_shieldPos;
            pos.x += (Rand()-0.5f)*5.0f;
            pos.z += (Rand()-0.5f)*5.0f;
            speed.x = (Rand()-0.5f)*3.0f;
            speed.z = (Rand()-0.5f)*3.0f;
            speed.y = (Rand()-0.5f)*3.0f;
            dim.x = Rand()*1.5f+2.0f;
            dim.y = dim.x;
            m_particule->CreateParticule(pos, speed, dim, PARTISMOKE3, 4.0f);
        }
    }

    if ( m_phase == TS_DOWN1 )
    {
        pos.x = 0.0f;
        pos.y = 1.0f+(1.0f-Bounce(m_progress))*3.0f;
        pos.z = 0.0f;
        m_object->SetPosition(3, pos);
    }

    if ( m_phase == TS_DOWN2 )
    {
        pos.x = 7.0f;
        pos.y = 4.5f+(1.0f-Bounce(m_progress))*3.0f;
        pos.z = 0.0f;
        m_object->SetPosition(2, pos);
    }

    return TRUE;
}


// Deploys the shield.
// The period is only useful with TSM_UP!

Error CTaskShield::Start(TaskShieldMode mode, float delay)
{
    CObject*    power;
    D3DMATRIX*  mat;
    D3DVECTOR   pos, iPos, oPos, speed;
    ObjectType  type;
    float       energy;

    if ( mode == TSM_DOWN )
    {
        return Stop();
    }

    if ( mode == TSM_UPDATE )
    {
        if ( m_object->RetSelect() )
        {
            m_brain->UpdateInterface();
        }
        return ERR_OK;
    }

    type = m_object->RetType();
    if ( type != OBJECT_MOBILErs )  return ERR_SHIELD_VEH;

    m_bError = TRUE;  // operation impossible
    if ( !m_physics->RetLand() )  return ERR_SHIELD_VEH;

    power = m_object->RetPower();
    if ( power == 0 )  return ERR_SHIELD_ENERGY;
    energy = power->RetEnergy();
    if ( energy == 0.0f )  return ERR_SHIELD_ENERGY;

    mat = m_object->RetWorldMatrix(0);
    pos = D3DVECTOR(7.0f, 15.0f, 0.0f);
    pos = Transform(*mat, pos);  // sphere position
    m_shieldPos = pos;

    m_sound->Play(SOUND_PSHHH2, m_shieldPos, 1.0f, 0.7f);

    m_phase    = TS_UP1;
    m_progress = 0.0f;
    m_speed    = 1.0f/1.0f;
    m_time     = 0.0f;
    m_delay    = delay;
    m_lastParticule = 0.0f;
    m_lastRay = 0.0f;
    m_lastIncrease = 0.0f;
    m_energyUsed = 0.0f;

    m_bError = FALSE;  // ok

    if ( m_object->RetSelect() )
    {
        m_brain->UpdateInterface();
    }
//? m_camera->StartCentering(m_object, PI*0.85f, -PI*0.15f, RetRadius()+40.0f, 3.0f);
    return ERR_OK;
}

// Returns the shield.

Error CTaskShield::Stop()
{
    float   time;

    if ( m_phase == TS_SHIELD )
    {
        m_object->SetShieldRadius(0.0f);

        if ( m_rankSphere != -1 )
        {
            m_particule->SetPhase(m_rankSphere, PARPHEND, 3.0f);
            m_rankSphere = -1;
        }

        if ( m_effectLight != -1 )
        {
            m_light->DeleteLight(m_effectLight);
            m_effectLight = -1;
        }

        time = m_energyUsed*4.0f;
        if ( time < 1.0f )  time = 1.0f;
        if ( time > 4.0f )  time = 4.0f;

        m_phase = TS_SMOKE;
        m_speed = 1.0f/time;

        m_camera->StopCentering(m_object, 4.0f);

        if ( m_object->RetSelect() )
        {
            m_brain->UpdateInterface();
        }
        return ERR_CONTINUE;
    }

    return ERR_OK;
}

// Indicates whether the action is finished.

Error CTaskShield::IsEnded()
{
    CObject*    power;
    D3DVECTOR   pos, speed;
    FPOINT      dim;
    float       energy;

    if ( m_engine->RetPause() )  return ERR_CONTINUE;
    if ( m_bError )  return ERR_STOP;

    if ( m_phase == TS_SHIELD )
    {
        m_object->SetShieldRadius(RetRadius());

        power = m_object->RetPower();
        if ( power == 0 )
        {
            energy = 0.0f;
        }
        else
        {
            energy = power->RetEnergy();
        }

        if ( energy == 0.0f || m_delay <= 0.0f )
        {
            Stop();
        }
        return ERR_CONTINUE;
    }

    if ( m_progress < 1.0f )  return ERR_CONTINUE;
    m_progress = 0.0f;

    if ( m_phase == TS_UP1 )
    {
        pos.x = 7.0f;
        pos.y = 4.5f+3.0f;
        pos.z = 0.0f;
        m_object->SetPosition(2, pos);

        m_sound->Play(SOUND_PSHHH2, m_shieldPos, 1.0f, 1.0f);

        m_phase = TS_UP2;
        m_speed = 1.0f/0.8f;
        return ERR_CONTINUE;
    }

    if ( m_phase == TS_UP2 )
    {
        pos.x = 0.0f;
        pos.y = 1.0f+3.0f;
        pos.z = 0.0f;
        m_object->SetPosition(3, pos);

        m_object->SetShieldRadius(RetRadius());

        pos = m_shieldPos;
        speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
        dim.x = RetRadius();
        dim.y = dim.x;
        m_rankSphere = m_particule->CreateParticule(pos, speed, dim, PARTISPHERE3, 2.0f, 0.0f, 0.0f);

        m_phase = TS_SHIELD;
        m_speed = 1.0f/999.9f;

        if ( m_object->RetSelect() )
        {
            m_brain->UpdateInterface();
        }
        return ERR_CONTINUE;
    }

    if ( m_phase == TS_SMOKE )
    {
        m_sound->Play(SOUND_PSHHH2, m_shieldPos, 1.0f, 1.0f);

        m_phase = TS_DOWN1;
        m_speed = 1.0f/0.8f;
        return ERR_CONTINUE;
    }

    if ( m_phase == TS_DOWN1 )
    {
        m_sound->Play(SOUND_PSHHH2, m_shieldPos, 1.0f, 0.7f);

        m_phase = TS_DOWN2;
        m_speed = 1.0f/1.0f;
        return ERR_CONTINUE;
    }

    Abort();
    return ERR_STOP;
}

// Indicates whether the action is pending.

BOOL CTaskShield::IsBusy()
{
    if ( m_phase == TS_SHIELD )
    {
        return FALSE;
    }

    return TRUE;
}

// Suddenly ends the current action.

BOOL CTaskShield::Abort()
{
    D3DVECTOR   pos;

    m_object->SetShieldRadius(0.0f);

    pos.x = 7.0f;
    pos.y = 4.5f;
    pos.z = 0.0f;
    m_object->SetPosition(2, pos);

    pos.x = 0.0f;
    pos.y = 1.0f;
    pos.z = 0.0f;
    m_object->SetPosition(3, pos);

    if ( m_soundChannel != -1 )
    {
        m_sound->FlushEnvelope(m_soundChannel);
        m_sound->AddEnvelope(m_soundChannel, 0.5f, 0.5f, 2.0f, SOPER_STOP);
        m_soundChannel = -1;
    }

    if ( m_rankSphere != -1 )
    {
        m_particule->SetPhase(m_rankSphere, PARPHEND, 3.0f);
        m_rankSphere = -1;
    }

    if ( m_effectLight != -1 )
    {
        m_light->DeleteLight(m_effectLight);
        m_effectLight = -1;
    }

    m_camera->StopCentering(m_object, 2.0f);
    return TRUE;
}


// Creates the light to accompany a pyrotechnic effect.

BOOL CTaskShield::CreateLight(D3DVECTOR pos)
{
    D3DLIGHT7   light;

    if ( !m_engine->RetLightMode() )  return TRUE;

    ZeroMemory( &light, sizeof(light) );
    light.dltType       = D3DLIGHT_SPOT;
    light.dcvDiffuse.r  = 0.0f;
    light.dcvDiffuse.g  = 1.0f;
    light.dcvDiffuse.b  = 2.0f;
    light.dvPosition.x  = pos.x;
    light.dvPosition.y  = pos.y;
    light.dvPosition.z  = pos.z;
    light.dvDirection.x =  0.0f;
    light.dvDirection.y = -1.0f;  // against the bottom
    light.dvDirection.z =  0.0f;
    light.dvRange = D3DLIGHT_RANGE_MAX;
    light.dvFalloff = 1.0f;
    light.dvAttenuation0 = 1.0f;
    light.dvAttenuation1 = 0.0f;
    light.dvAttenuation2 = 0.0f;
    light.dvTheta = 0.0f;
    light.dvPhi = PI/4.0f;

    m_effectLight = m_light->CreateLight();
    if ( m_effectLight == -1 )  return FALSE;

    m_light->SetLight(m_effectLight, light);
    m_light->SetLightIntensity(m_effectLight, 1.0f);

    return TRUE;
}


// Repaired the shielded objects within the sphere of the shield.

void CTaskShield::IncreaseShield()
{
    ObjectType  type;
    CObject*    pObj;
    D3DVECTOR   oPos;
    float       dist, shield;
    int         i;

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        type = pObj->RetType();
        if ( type == OBJECT_MOTHER ||
             type == OBJECT_ANT    ||
             type == OBJECT_SPIDER ||
             type == OBJECT_BEE    ||
             type == OBJECT_WORM   )  continue;

        oPos = pObj->RetPosition(0);
        dist = Length(oPos, m_shieldPos);
        if ( dist <= RetRadius()+10.0f )
        {
            shield = pObj->RetShield();
            shield += 0.1f;
            if ( shield > 1.0f )  shield = 1.0f;
            pObj->SetShield(shield);
        }
    }
}


// Returns the radius of the shield.

float CTaskShield::RetRadius()
{
    return RADIUS_SHIELD_MIN + (RADIUS_SHIELD_MAX-RADIUS_SHIELD_MIN)*m_object->RetParam();
}



