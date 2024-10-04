/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsitec.ch; http://colobot.info; http://github.com/colobot
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://gnu.org/licenses
 */


#include "object/task/taskfire.h"

#include "graphics/engine/engine.h"
#include "graphics/engine/particle.h"

#include "math/geometry.h"

#include "object/old_object.h"

#include "object/interface/slotted_object.h"

#include "physics/physics.h"

#include "sound/sound.h"


const float ENERGY_FIRE     = (0.25f/2.5f); // energy consumed/shot
const float ENERGY_FIREr    = (0.25f/1.5f); // energy consumed/ray
const float ENERGY_FIREi    = (0.10f/2.5f); // energy consumed/organic


// Object's constructor.

CTaskFire::CTaskFire(COldObject* object) : CForegroundTask(object)
{
    m_soundChannel = -1;

    assert(HasPowerCellSlot(m_object));
}

// Object's destructor.

CTaskFire::~CTaskFire()
{
    if ( m_soundChannel != -1 )
    {
        m_sound->FlushEnvelope(m_soundChannel);
        m_sound->AddEnvelope(m_soundChannel, 0.0f, 1.0f, 1.0f, SOPER_STOP);
        m_soundChannel = -1;
    }
}


// Management of an event.

bool CTaskFire::EventProcess(const Event &event)
{
    CPhysics*   physics;
    glm::vec3    pos, speed, dir, vib;
    ObjectType  type;
    glm::vec2   dim;
    float       energy, fire;
    int         i, channel;

    if ( m_engine->GetPause() )  return true;
    if ( event.type != EVENT_FRAME )  return true;
    if ( m_bError )  return false;

    m_time += event.rTime;
    m_lastSound -= event.rTime;
    m_progress += event.rTime*m_speed;

    if (CPowerContainerObject* power = GetObjectPowerCell(m_object))
    {
        energy = power->GetEnergy();
             if ( m_bOrganic )  fire = ENERGY_FIREi;
        else if ( m_bRay     )  fire = ENERGY_FIREr;
        else                    fire = ENERGY_FIRE;
        energy -= event.rTime*fire;
        power->SetEnergy(energy);
    }

    if ( m_lastParticle+0.05f <= m_time )
    {
        m_lastParticle = m_time;

        if ( m_bOrganic )
        {
            glm::mat4 mat = m_object->GetWorldMatrix(1);  // insect-cannon

            for ( i=0 ; i<6 ; i++ )
            {
                pos = glm::vec3(0.0f, 2.5f, 0.0f);
                pos = Math::Transform(mat, pos);

                speed = glm::vec3(200.0f, 0.0f, 0.0f);

                physics = m_object->GetPhysics();
                if ( physics != nullptr )
                {
                    speed += physics->GetLinMotion(MO_REASPEED);
                }

                speed.x += (Math::Rand()-0.5f)*10.0f;
                speed.y += (Math::Rand()-0.5f)*20.0f;
                speed.z += (Math::Rand()-0.5f)*30.0f;
                speed = Math::Transform(mat, speed);
                speed -= pos;

                dim.x = Math::Rand()*0.5f+0.5f;
                dim.y = dim.x;

                channel = m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIGUN4, 0.8f, 0.0f, 0.0f);
                m_particle->SetObjectFather(channel, m_object);
            }
        }
        else if ( m_bRay )
        {
            glm::mat4 mat = m_object->GetWorldMatrix(2);  // cannon

            for ( i=0 ; i<4 ; i++ )
            {
                pos = glm::vec3(4.0f, 0.0f, 0.0f);
                pos.y += (rand()%3-1)*1.5f;
                pos.z += (rand()%3-1)*1.5f;
                pos = Math::Transform(mat, pos);

                speed = glm::vec3(200.0f, 0.0f, 0.0f);
                speed.x += (Math::Rand()-0.5f)*6.0f;
                speed.y += (Math::Rand()-0.5f)*12.0f;
                speed.z += (Math::Rand()-0.5f)*12.0f;
                speed = Math::Transform(mat, speed);
                speed -= pos;

                dim.x = 1.0f;
                dim.y = dim.x;
                channel = m_particle->CreateTrack(pos, speed, dim, Gfx::PARTITRACK11,
                                                   2.0f, 200.0f, 0.5f, 1.0f);
                m_particle->SetObjectFather(channel, m_object);

                speed = glm::vec3(5.0f, 0.0f, 0.0f);
                speed.x += (Math::Rand()-0.5f)*1.0f;
                speed.y += (Math::Rand()-0.5f)*2.0f;
                speed.z += (Math::Rand()-0.5f)*2.0f;
                speed = Math::Transform(mat, speed);
                speed -= pos;
                speed.y += 5.0f;

                dim.x = 2.0f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISMOKE2, 2.0f, 0.0f, 0.5f);
            }
        }
        else
        {
            type = m_object->GetType();

            glm::mat4 mat = glm::mat4(1.0f);

            if ( type == OBJECT_MOBILErc )
            {
                mat = m_object->GetWorldMatrix(2);  // cannon
            }
            else
            {
                mat = m_object->GetWorldMatrix(1);  // cannon
            }

            for ( i=0 ; i<3 ; i++ )
            {
                if ( type == OBJECT_MOBILErc )
                {
                    pos = glm::vec3(0.0f, 0.0f, 0.0f);
                }
                else
                {
                    pos = glm::vec3(3.0f, 1.0f, 0.0f);
                }
                pos.y += (Math::Rand()-0.5f)*1.0f;
                pos.z += (Math::Rand()-0.5f)*1.0f;
                pos = Math::Transform(mat, pos);

                speed = glm::vec3(200.0f, 0.0f, 0.0f);

                physics = m_object->GetPhysics();
                if ( physics != nullptr )
                {
                    speed += physics->GetLinMotion(MO_REASPEED);
                }

                speed.x += (Math::Rand()-0.5f)*3.0f;
                speed.y += (Math::Rand()-0.5f)*6.0f;
                speed.z += (Math::Rand()-0.5f)*6.0f;
                speed = Math::Transform(mat, speed);
                speed -= pos;

                dim.x = Math::Rand()*0.7f+0.7f;
                dim.y = dim.x;

                channel = m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIGUN1, 0.8f, 0.0f, 0.0f);
                m_particle->SetObjectFather(channel, m_object);
            }

            if ( type != OBJECT_MOBILErc &&
                 m_progress > 0.3f )
            {
                pos = glm::vec3(-1.0f, 1.0f, 0.0f);
                pos.y += (Math::Rand()-0.5f)*0.4f;
                pos.z += (Math::Rand()-0.5f)*0.4f;
                pos = Math::Transform(mat, pos);

                speed = glm::vec3(-4.0f, 0.0f, 0.0f);
                speed.x += (Math::Rand()-0.5f)*2.0f;
                speed.y += (Math::Rand()-0.2f)*4.0f;
                speed.z += (Math::Rand()-0.5f)*4.0f;
                speed = Math::Transform(mat, speed);
                speed -= pos;

                dim.x = Math::Rand()*1.2f+1.2f;
                dim.y = dim.x;

                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTICRASH, 2.0f, 0.0f, 0.0f);
//?             m_particle->CreateParticle(pos, speed, dim, PARTISMOKE2, 4.0f, 0.0f, 0.0f);
            }
        }

        dir = glm::vec3(0.0f, 0.0f, 0.0f);
        if ( m_progress < 0.1f )
        {
            dir.z = (Math::PI*0.04f)*(m_progress*10.0f);
        }
        else if ( m_progress < 0.9f )
        {
            dir.z = (Math::PI*0.04f);
        }
        else
        {
            dir.z = (Math::PI*0.04f)*(1.0f-(m_progress-0.9f)*10.0f);
        }
        m_object->SetTilt(dir);

        vib.x = (Math::Rand()-0.5f)*0.01f;
        vib.y = (Math::Rand()-0.5f)*0.02f;
        vib.z = (Math::Rand()-0.5f)*0.02f;
        m_object->SetCirVibration(vib);

        vib.x = (Math::Rand()-0.5f)*0.20f;
        vib.y = (Math::Rand()-0.5f)*0.05f;
        vib.z = (Math::Rand()-0.5f)*0.20f;
        m_object->SetLinVibration(vib);
    }

    if ( m_bRay && m_lastSound <= 0.0f )
    {
        m_lastSound = Math::Rand()*0.4f+0.4f;
        m_sound->Play(SOUND_FIREp, m_object->GetPosition());
    }

    return true;
}


// Assigns the goal was achieved.

Error CTaskFire::Start(float delay)
{
    glm::vec3   speed;
    float       energy, fire;
    ObjectType  type;

    m_bError = true;  // operation impossible

    type = m_object->GetType();
    if ( type != OBJECT_MOBILEfc &&
         type != OBJECT_MOBILEtc &&
         type != OBJECT_MOBILEwc &&
         type != OBJECT_MOBILEic &&
         type != OBJECT_MOBILEfi &&
         type != OBJECT_MOBILEti &&
         type != OBJECT_MOBILEwi &&
         type != OBJECT_MOBILEii &&
         type != OBJECT_MOBILErc )  return ERR_WRONG_BOT;

//? if ( !m_physics->GetLand() )  return ERR_FIRE_FLY;

    speed = m_physics->GetMotorSpeed();
//? if ( speed.x != 0.0f ||
//?      speed.z != 0.0f )  return ERR_FIRE_MOTOR;

    m_bRay = (type == OBJECT_MOBILErc);

    m_bOrganic = false;
    if ( type == OBJECT_MOBILEfi ||
         type == OBJECT_MOBILEti ||
         type == OBJECT_MOBILEwi ||
         type == OBJECT_MOBILEii )
    {
        m_bOrganic = true;
    }

    if ( delay == 0.0f )
    {
        if ( m_bRay )  delay = 1.2f;
        else           delay = 2.0f;
    }
    m_delay = delay;

    assert(HasPowerCellSlot(m_object));
    CPowerContainerObject *power = GetObjectPowerCell(m_object);
    if (power == nullptr)  return ERR_FIRE_ENERGY;

    energy = power->GetEnergy();
         if ( m_bOrganic )  fire = m_delay*ENERGY_FIREi;
    else if ( m_bRay     )  fire = m_delay*ENERGY_FIREr;
    else                    fire = m_delay*ENERGY_FIRE;
    if ( energy < fire+0.05f )  return ERR_FIRE_ENERGY;

    m_speed = 1.0f/m_delay;
    m_progress = 0.0f;
    m_time = 0.0f;
    m_lastParticle = 0.0f;
    m_lastSound = 0.0f;
    m_bError = false;  // ok

//? m_camera->StartCentering(m_object, Math::PI*0.15f, 99.9f, 0.0f, 1.0f);

    if ( m_bOrganic )
    {
        m_soundChannel = m_sound->Play(SOUND_FIREi, m_object->GetPosition(), 1.0f, 1.0f, true);
        if ( m_soundChannel != -1 )
        {
            m_sound->AddEnvelope(m_soundChannel, 1.0f, 1.0f, m_delay, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 0.0f, 1.0f, 0.5f, SOPER_STOP);
        }
    }
    else if ( m_bRay )
    {
    }
    else
    {
        m_soundChannel = m_sound->Play(SOUND_FIRE, m_object->GetPosition(), 1.0f, 1.0f, true);
        if ( m_soundChannel != -1 )
        {
            m_sound->AddEnvelope(m_soundChannel, 1.0f, 1.0f, m_delay, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 0.0f, 1.0f, 0.5f, SOPER_STOP);
        }
    }

    return ERR_OK;
}

// Indicates whether the action is finished.

Error CTaskFire::IsEnded()
{
    if ( m_engine->GetPause() )  return ERR_CONTINUE;
    if ( m_bError )  return ERR_STOP;
    if ( m_progress < 1.0f )  return ERR_CONTINUE;

    Abort();
    return ERR_STOP;
}

// Suddenly ends the current action.

bool CTaskFire::Abort()
{
    m_object->SetTilt(glm::vec3(0.0f, 0.0f, 0.0f));
    m_object->SetCirVibration(glm::vec3(0.0f, 0.0f, 0.0f));
    m_object->SetLinVibration(glm::vec3(0.0f, 0.0f, 0.0f));

    if ( m_soundChannel != -1 )
    {
        m_sound->FlushEnvelope(m_soundChannel);
        m_sound->AddEnvelope(m_soundChannel, 0.0f, 1.0f, 1.0f, SOPER_STOP);
        m_soundChannel = -1;
    }

//? m_camera->StopCentering(m_object, 1.0f);
    return true;
}

bool CTaskFire::IsPilot()
{
    return true;
}
