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


#include "object/task/taskshield.h"

#include "common/iman.h"

#include "graphics/core/light.h"
#include "graphics/engine/particle.h"
#include "graphics/engine/lightman.h"

#include "math/geometry.h"

#include "object/brain.h"
#include "object/robotmain.h"

#include "physics/physics.h"

#include <string.h>

const float ENERGY_TIME = 20.0f;        // maximum duration if full battery



// Object's constructor.

CTaskShield::CTaskShield(CObject* object) : CTask(object)
{
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

bool CTaskShield::EventProcess(const Event &event)
{
    CObject*        power;
    Math::Matrix*   mat;
    Math::Matrix    matrix;
    Math::Vector    pos, speed, goal, angle;
    Gfx::Color      color;
    Math::Point     dim;
    float           energy;

    if ( m_engine->GetPause() )  return true;
    if ( event.type != EVENT_FRAME )  return true;
    if ( m_bError )  return false;

    m_progress += event.rTime*m_speed;  // others advance
    m_time += event.rTime;
    m_delay -= event.rTime;

    mat = m_object->GetWorldMatrix(0);
    pos = Math::Vector(7.0f, 15.0f, 0.0f);
    pos = Math::Transform(*mat, pos);  // sphere position
    m_shieldPos = pos;

    if ( m_rankSphere != -1 )
    {
        m_particle->SetPosition(m_rankSphere, m_shieldPos);
        dim.x = GetRadius();
        dim.y = dim.x;
        m_particle->SetDimension(m_rankSphere, dim);
    }

    if ( m_phase == TS_UP1 )
    {
        pos.x = 7.0f;
        pos.y = 4.5f+Math::Bounce(m_progress)*3.0f;
        pos.z = 0.0f;
        m_object->SetPosition(2, pos);
    }

    if ( m_phase == TS_UP2 )
    {
        pos.x = 0.0f;
        pos.y = 1.0f+Math::Bounce(m_progress)*3.0f;
        pos.z = 0.0f;
        m_object->SetPosition(3, pos);
    }

    if ( m_phase == TS_SHIELD )
    {
        energy = (1.0f/ENERGY_TIME)*event.rTime;
        energy *= GetRadius()/RADIUS_SHIELD_MAX;
        power = m_object->GetPower();
        if ( power != 0 )
        {
            power->SetEnergy(power->GetEnergy()-energy/power->GetCapacity());
        }
        m_energyUsed += energy;

        if ( m_soundChannel == -1 )
        {
            m_soundChannel = m_sound->Play(SOUND_SHIELD, m_shieldPos, 0.5f, 0.5f, true);
            m_sound->AddEnvelope(m_soundChannel, 1.0f, 1.0f, 2.0f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 1.0f, 1.0f, 1.0f, SOPER_LOOP);
        }
        else
        {
            m_sound->Position(m_soundChannel, m_shieldPos);
        }

        pos = m_shieldPos;
        pos.y += GetRadius()*(2.0f+sinf(m_time*9.0f)*0.2f);
        if ( m_effectLight == -1 )
        {
            CreateLight(pos);
        }
        else
        {
            m_lightMan->SetLightPos(m_effectLight, pos);

            color.r = 0.0f+sinf(m_time*33.2f)*0.2f;
            color.g = 0.5f+sinf(m_time*20.0f)*0.5f;
            color.b = 0.5f+sinf(m_time*21.3f)*1.0f;
            color.a = 0.0f;
            m_lightMan->SetLightColor(m_effectLight, color);
        }

        if ( m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
        {
            m_lastParticle = m_time;

            pos = m_shieldPos;
            pos.x += (Math::Rand()-0.5f)*5.0f;
            pos.z += (Math::Rand()-0.5f)*5.0f;
            speed.x = (Math::Rand()-0.5f)*0.0f;
            speed.z = (Math::Rand()-0.5f)*0.0f;
            speed.y = Math::Rand()*15.0f;
            dim.x = Math::Rand()*6.0f+4.0f;
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIBLUE, 1.0f, 0.0f, 0.0f);
        }

        if ( m_lastRay+m_engine->ParticleAdapt(0.05f) <= m_time )
        {
            m_lastRay = m_time;

            pos = m_shieldPos;
            dim.x = GetRadius()/20.0f;
            dim.y = dim.x;
            angle.x = (Math::Rand()-0.5f)*Math::PI*1.2f;
            angle.y = 0.0f;
            angle.z = (Math::Rand()-0.5f)*Math::PI*1.2f;
            Math::LoadRotationXZYMatrix(matrix, angle);
            goal = Math::Transform(matrix, Math::Vector(0.0f, GetRadius()-dim.x, 0.0f));
            goal += pos;
            m_particle->CreateRay(pos, goal, Gfx::PARTIRAY2, dim, 0.3f);
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

        if ( m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
        {
            m_lastParticle = m_time;

            pos = m_shieldPos;
            pos.x += (Math::Rand()-0.5f)*5.0f;
            pos.z += (Math::Rand()-0.5f)*5.0f;
            speed.x = (Math::Rand()-0.5f)*3.0f;
            speed.z = (Math::Rand()-0.5f)*3.0f;
            speed.y = (Math::Rand()-0.5f)*3.0f;
            dim.x = Math::Rand()*1.5f+2.0f;
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISMOKE3, 4.0f);
        }
    }

    if ( m_phase == TS_DOWN1 )
    {
        pos.x = 0.0f;
        pos.y = 1.0f+(1.0f-Math::Bounce(m_progress))*3.0f;
        pos.z = 0.0f;
        m_object->SetPosition(3, pos);
    }

    if ( m_phase == TS_DOWN2 )
    {
        pos.x = 7.0f;
        pos.y = 4.5f+(1.0f-Math::Bounce(m_progress))*3.0f;
        pos.z = 0.0f;
        m_object->SetPosition(2, pos);
    }

    return true;
}


// Deploys the shield.
// The period is only useful with TSM_UP!

Error CTaskShield::Start(TaskShieldMode mode, float delay)
{
    CObject*    power;
    Math::Matrix*   mat;
    Math::Vector    pos, iPos, oPos, speed;
    ObjectType  type;
    float       energy;

    if ( mode == TSM_DOWN )
    {
        return Stop();
    }

    if ( mode == TSM_UPDATE )
    {
        if ( m_object->GetSelect() )
        {
            m_brain->UpdateInterface();
        }
        return ERR_OK;
    }

    type = m_object->GetType();
    if ( type != OBJECT_MOBILErs )  return ERR_SHIELD_VEH;

    m_bError = true;  // operation impossible
    if ( !m_physics->GetLand() )  return ERR_SHIELD_VEH;

    power = m_object->GetPower();
    if ( power == 0 )  return ERR_SHIELD_ENERGY;
    energy = power->GetEnergy();
    if ( energy == 0.0f )  return ERR_SHIELD_ENERGY;

    mat = m_object->GetWorldMatrix(0);
    pos = Math::Vector(7.0f, 15.0f, 0.0f);
    pos = Transform(*mat, pos);  // sphere position
    m_shieldPos = pos;

    m_sound->Play(SOUND_PSHHH2, m_shieldPos, 1.0f, 0.7f);

    m_phase    = TS_UP1;
    m_progress = 0.0f;
    m_speed    = 1.0f/1.0f;
    m_time     = 0.0f;
    m_delay    = delay;
    m_lastParticle = 0.0f;
    m_lastRay = 0.0f;
    m_lastIncrease = 0.0f;
    m_energyUsed = 0.0f;

    m_bError = false;  // ok

    if ( m_object->GetSelect() )
    {
        m_brain->UpdateInterface();
    }
//? m_camera->StartCentering(m_object, Math::PI*0.85f, -Math::PI*0.15f, GetRadius()+40.0f, 3.0f);
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
            m_particle->SetPhase(m_rankSphere, Gfx::PARPHEND, 3.0f);
            m_rankSphere = -1;
        }

        if ( m_effectLight != -1 )
        {
            m_lightMan->DeleteLight(m_effectLight);
            m_effectLight = -1;
        }

        time = m_energyUsed*4.0f;
        if ( time < 1.0f )  time = 1.0f;
        if ( time > 4.0f )  time = 4.0f;

        m_phase = TS_SMOKE;
        m_speed = 1.0f/time;

        m_camera->StopCentering(m_object, 4.0f);

        if ( m_object->GetSelect() )
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
    Math::Vector    pos, speed;
    Math::Point     dim;
    float       energy;

    if ( m_engine->GetPause() )  return ERR_CONTINUE;
    if ( m_bError )  return ERR_STOP;

    if ( m_phase == TS_SHIELD )
    {
        m_object->SetShieldRadius(GetRadius());

        power = m_object->GetPower();
        if ( power == 0 )
        {
            energy = 0.0f;
        }
        else
        {
            energy = power->GetEnergy();
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

        m_object->SetShieldRadius(GetRadius());

        pos = m_shieldPos;
        speed = Math::Vector(0.0f, 0.0f, 0.0f);
        dim.x = GetRadius();
        dim.y = dim.x;
        m_rankSphere = m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISPHERE3, 2.0f, 0.0f, 0.0f);

        m_phase = TS_SHIELD;
        m_speed = 1.0f/999.9f;

        if ( m_object->GetSelect() )
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

bool CTaskShield::IsBusy()
{
    if ( m_phase == TS_SHIELD )
    {
        return false;
    }

    return true;
}

// Suddenly ends the current action.

bool CTaskShield::Abort()
{
    Math::Vector    pos;

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
        m_particle->SetPhase(m_rankSphere, Gfx::PARPHEND, 3.0f);
        m_rankSphere = -1;
    }

    if ( m_effectLight != -1 )
    {
        m_lightMan->DeleteLight(m_effectLight);
        m_effectLight = -1;
    }

    m_camera->StopCentering(m_object, 2.0f);
    return true;
}


// Creates the light to accompany a pyrotechnic effect.

bool CTaskShield::CreateLight(Math::Vector pos)
{
    Gfx::Light light;

    if ( !m_engine->GetLightMode() )  return true;

    memset(&light, 0, sizeof(light));
    light.type       = Gfx::LIGHT_SPOT;
    light.ambient    = Gfx::Color(0.0f, 0.0f, 0.0f);
    light.diffuse    = Gfx::Color(0.0f, 1.0f, 2.0f);
    light.position   = pos;
    light.direction  = Math::Vector(0.0f, -1.0f, 0.0f); // against the bottom
    light.spotIntensity = 128;
    light.attenuation0 = 1.0f;
    light.attenuation1 = 0.0f;
    light.attenuation2 = 0.0f;
    light.spotAngle = 90.0f*Math::PI/180.0f;

    m_effectLight = m_lightMan->CreateLight();
    if ( m_effectLight == -1 )  return false;

    m_lightMan->SetLight(m_effectLight, light);
    m_lightMan->SetLightIntensity(m_effectLight, 1.0f);

    return true;
}


// Repaired the shielded objects within the sphere of the shield.

void CTaskShield::IncreaseShield()
{
    ObjectType  type;
    CObject*    pObj;
    Math::Vector    oPos;
    float       dist, shield;
    int         i;

    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
        if ( pObj == 0 )  break;

        type = pObj->GetType();
        if ( type == OBJECT_MOTHER ||
             type == OBJECT_ANT    ||
             type == OBJECT_SPIDER ||
             type == OBJECT_BEE    ||
             type == OBJECT_WORM   )  continue;

        oPos = pObj->GetPosition(0);
        dist = Math::Distance(oPos, m_shieldPos);
        if ( dist <= GetRadius()+10.0f )
        {
            shield = pObj->GetShield();
            shield += 0.1f;
            if ( shield > 1.0f )  shield = 1.0f;
            pObj->SetShield(shield);
        }
    }
}


// Returns the radius of the shield.

float CTaskShield::GetRadius()
{
    return RADIUS_SHIELD_MIN + (RADIUS_SHIELD_MAX-RADIUS_SHIELD_MIN)*m_object->GetParam();
}

