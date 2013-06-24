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


#include <stdio.h>

#include "object/task/taskterraform.h"

#include "common/iman.h"

#include "graphics/engine/pyro.h"
#include "graphics/engine/particle.h"
#include "graphics/engine/terrain.h"

#include "math/geometry.h"

#include "object/brain.h"
#include "object/robotmain.h"
#include "object/motion/motionant.h"
#include "object/motion/motionspider.h"

#include "physics/physics.h"


const float ENERGY_TERRA    = 0.40f;        // energy consumed by blow
const float ACTION_RADIUS   = 400.0f;



// Object's constructor.

CTaskTerraform::CTaskTerraform(CObject* object) : CTask(object)
{
    m_lastParticle = 0.0f;
    m_soundChannel = -1;
}

// Object's destructor.

CTaskTerraform::~CTaskTerraform()
{
}


// Management of an event.

bool CTaskTerraform::EventProcess(const Event &event)
{
    CObject*    power;
    Math::Matrix*   mat;
    Math::Vector    pos, dir, speed;
    Math::Point     dim;
    float       energy;

    if ( m_engine->GetPause() )  return true;
    if ( event.type != EVENT_FRAME )  return true;
    if ( m_bError )  return false;

    m_progress += event.rTime*m_speed;  // others advance
    m_time += event.rTime;

    if ( m_phase == TTP_CHARGE )
    {
        if ( m_soundChannel == -1 )
        {
#if _TEEN
            m_soundChannel = m_sound->Play(SOUND_GGG, m_object->GetPosition(0), 1.0f, 0.5f, true);
            m_sound->AddEnvelope(m_soundChannel, 1.0f, 2.0f, 1.5f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 0.0f, 0.5f, 0.5f, SOPER_STOP);
#else
            m_soundChannel = m_sound->Play(SOUND_GGG, m_object->GetPosition(0), 1.0f, 0.5f, true);
            m_sound->AddEnvelope(m_soundChannel, 1.0f, 2.0f, 4.0f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 0.0f, 0.5f, 0.5f, SOPER_STOP);
#endif
        }

        dir.x = 0.0f;
        dir.y = (Math::Rand()-0.5f)*0.2f*m_progress;
        dir.z = 0.0f;
        m_object->SetCirVibration(dir);

        m_object->SetZoom(0, 1.0f+m_progress*0.2f);

        power = m_object->GetPower();
        if ( power != 0 )
        {
            power->SetZoom(0, 1.0f+m_progress*1.0f);

            energy = power->GetEnergy();
            energy -= event.rTime*ENERGY_TERRA/power->GetCapacity()/4.0f;
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
    pos = m_object->GetPosition(2);
    if ( pos.y < 0.0f )
    {
        dir.z = -atanf((pos.y/2.0f)/9.0f);
    }
    m_object->SetInclinaison(dir);

    if ( m_time-m_lastParticle >= m_engine->ParticleAdapt(0.05f) )
    {
        m_lastParticle = m_time;

        mat = m_object->GetWorldMatrix(0);

        if ( m_phase == TTP_CHARGE )
        {
            // Battery.
            pos = Math::Vector(-6.0f, 5.5f+2.0f*m_progress, 0.0f);
            pos.x += (Math::Rand()-0.5f)*1.0f;
            pos.z += (Math::Rand()-0.5f)*1.0f;
            pos   = Math::Transform(*mat, pos);
            speed.x = (Math::Rand()-0.5f)*6.0f*(1.0f+m_progress*4.0f);
            speed.z = (Math::Rand()-0.5f)*6.0f*(1.0f+m_progress*4.0f);
            speed.y = 6.0f+Math::Rand()*4.0f*(1.0f+m_progress*2.0f);
            dim.x = 0.5f+1.5f*m_progress;
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIBLITZ, 2.0f, 20.0f);
        }

        if ( m_phase != TTP_CHARGE )
        {
            // Left grid.
            pos = Math::Vector(-1.0f, 5.8f, 3.5f);
            pos.x += (Math::Rand()-0.5f)*1.0f;
            pos.z += (Math::Rand()-0.5f)*1.0f;
            pos   = Math::Transform(*mat, pos);
            speed.x = Math::Rand()*4.0f;
            speed.z = Math::Rand()*2.0f;
            speed.y = 2.5f+Math::Rand()*1.0f;
            speed = Math::Transform(*mat, speed);
            speed -= m_object->GetPosition(0);
            dim.x = Math::Rand()*1.0f+1.0f;
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISMOKE1, 3.0f);

            // Right grid.
            pos = Math::Vector(-1.0f, 5.8f, -3.5f);
            pos.x += (Math::Rand()-0.5f)*1.0f;
            pos.z += (Math::Rand()-0.5f)*1.0f;
            pos   = Math::Transform(*mat, pos);
            speed.x =  Math::Rand()*4.0f;
            speed.z = -Math::Rand()*2.0f;
            speed.y = 2.5f+Math::Rand()*1.0f;
            speed = Math::Transform(*mat, speed);
            speed -= m_object->GetPosition(0);
            dim.x = Math::Rand()*1.0f+1.0f;
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISMOKE1, 3.0f);
        }
    }

    return true;
}


// Assigns the goal was achieved.

Error CTaskTerraform::Start()
{
    CObject*    power;
    Math::Matrix*   mat;
    Math::Vector    pos, speed;
    float       energy;

    ObjectType  type;

    m_bError = true;  // operation impossible
    if ( !m_physics->GetLand() )  return ERR_TERRA_VEH;

    type = m_object->GetType();
    if ( type != OBJECT_MOBILErt )  return ERR_TERRA_VEH;

    power = m_object->GetPower();
    if ( power == 0 )  return ERR_TERRA_ENERGY;
    energy = power->GetEnergy();
    if ( energy < ENERGY_TERRA/power->GetCapacity()+0.05f )  return ERR_TERRA_ENERGY;

    speed = m_physics->GetMotorSpeed();
    if ( speed.x != 0.0f ||
         speed.z != 0.0f )  return ERR_MANIP_MOTOR;

    mat = m_object->GetWorldMatrix(0);
    pos = Math::Vector(9.0f, 0.0f, 0.0f);
    pos = Math::Transform(*mat, pos);  // battery position
    m_terraPos = pos;

    m_phase    = TTP_CHARGE;
    m_progress = 0.0f;
#if _TEEN
    m_speed    = 1.0f/1.5f;
#else
    m_speed    = 1.0f/4.0f;
#endif
    m_time     = 0.0f;

    m_bError = false;  // ok

    m_camera->StartCentering(m_object, Math::PI*0.35f, 99.9f, 20.0f, 2.0f);
    return ERR_OK;
}

// Indicates whether the action is finished.

Error CTaskTerraform::IsEnded()
{
    CObject*    power;
    Math::Vector    pos, speed;
    Math::Point     dim;
    float       dist, duration;
    int         i, max;

    if ( m_engine->GetPause() )  return ERR_CONTINUE;
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

        m_object->SetCirVibration(Math::Vector(0.0f, 0.0f, 0.0f));
        m_object->SetZoom(0, 1.0f);

        power = m_object->GetPower();
        if ( power != 0 )
        {
            power->SetZoom(0, 1.0f);
        }

        max= static_cast<int>(50.0f*m_engine->GetParticleDensity());
        for ( i=0 ; i<max ; i++ )
        {
            pos.x = m_terraPos.x+(Math::Rand()-0.5f)*80.0f;
            pos.z = m_terraPos.z+(Math::Rand()-0.5f)*80.0f;
            pos.y = m_terraPos.y;
            m_terrain->AdjustToFloor(pos);
            dist = Math::Distance(pos, m_terraPos);
            speed = Math::Vector(0.0f, 0.0f, 0.0f);
            dim.x = 2.0f+(40.0f-dist)/(1.0f+Math::Rand()*4.0f);
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTICRASH, 2.0f);

            pos = m_terraPos;
            speed.x = (Math::Rand()-0.5f)*40.0f;
            speed.z = (Math::Rand()-0.5f)*40.0f;
            speed.y = Math::Rand()*15.0f+15.0f;
            dim.x = 0.6f;
            dim.y = dim.x;
            pos.y += dim.y;
            duration = Math::Rand()*3.0f+3.0f;
            m_particle->CreateTrack(pos, speed, dim, Gfx::PARTITRACK5,
                                     duration, Math::Rand()*10.0f+15.0f,
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

bool CTaskTerraform::Abort()
{
    CObject*    power;

    if ( m_soundChannel != -1 )
    {
        m_sound->FlushEnvelope(m_soundChannel);
        m_sound->AddEnvelope(m_soundChannel, 0.0f, 0.5f, 0.3f, SOPER_STOP);
        m_soundChannel = -1;
    }

    m_object->SetPosition(2, Math::Vector(9.0f, 4.0f, 0.0f));
    m_object->SetInclinaison(Math::Vector(0.0f, 0.0f, 0.0f));
    m_object->SetCirVibration(Math::Vector(0.0f, 0.0f, 0.0f));
    m_object->SetZoom(0, 1.0f);

    power = m_object->GetPower();
    if ( power != 0 )
    {
        power->SetZoom(0, 1.0f);
    }

    m_camera->StopCentering(m_object, 2.0f);
    return true;
}


// Returns all the close ants and spiders.

bool CTaskTerraform::Terraform()
{
    CObject*    pObj;
    CBrain*     brain;
    CMotion*    motion;
    Gfx::CPyro* pyro;
    ObjectType  type;
    float       dist;
    int         i;

    m_camera->StartEffect(Gfx::CAM_EFFECT_TERRAFORM, m_terraPos, 1.0f);

    m_sound->Play(SOUND_THUMP, m_terraPos);

    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
        if ( pObj == 0 )  break;

        type = pObj->GetType();
        if ( type == OBJECT_NULL )  continue;

        if ( type == OBJECT_TEEN34 )  // stone?
        {
            dist = Math::Distance(m_terraPos, pObj->GetPosition(0));
            if ( dist > 20.0f )  continue;

            pyro = new Gfx::CPyro();
            pyro->Create(Gfx::PT_FRAGT, pObj);
        }
        else
        {
            motion = pObj->GetMotion();
            if ( motion == 0 )  continue;

            dist = Math::Distance(m_terraPos, pObj->GetPosition(0));
            if ( dist > ACTION_RADIUS )  continue;

            if ( type == OBJECT_ANT )
            {
                brain = pObj->GetBrain();
                if ( brain != 0 )  brain->StopTask();
                motion->SetAction(MAS_BACK1, 0.8f+Math::Rand()*0.3f);
                pObj->SetFixed(true);  // not moving
            }
            if ( type == OBJECT_SPIDER )
            {
                brain = pObj->GetBrain();
                if ( brain != 0 )  brain->StopTask();
                motion->SetAction(MSS_BACK1, 0.8f+Math::Rand()*0.3f);
                pObj->SetFixed(true);  // not moving
            }
        }
    }

    return true;
}

