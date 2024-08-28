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


#include "object/task/taskrecover.h"

#include "common/global.h"

#include "graphics/engine/engine.h"
#include "graphics/engine/particle.h"

#include "level/robotmain.h"

#include "math/geometry.h"

#include "object/object_manager.h"
#include "object/old_object.h"

#include "object/interface/slotted_object.h"

#include "physics/physics.h"

#include "sound/sound.h"


const float ENERGY_RECOVER  = 0.25f;        // energy consumed by recovery
const float RECOVER_DIST    = 11.8f;



// Object's constructor.

CTaskRecover::CTaskRecover(COldObject* object) : CForegroundTask(object)
{
    m_ruin = nullptr;
    m_soundChannel = -1;
}

// Object's constructor.

CTaskRecover::~CTaskRecover()
{
}


// Management of an event.

bool CTaskRecover::EventProcess(const Event &event)
{
    glm::vec3    pos, speed;
    glm::vec2       dim;
    float       a, g, cirSpeed, angle, energy, dist, linSpeed;

    if ( m_engine->GetPause() )  return true;
    if ( event.type != EVENT_FRAME )  return true;
    if ( m_bError )  return false;

    if ( m_phase == TRP_TURN )  // preliminary rotation?
    {
        a = m_object->GetRotationY();
        g = m_angle;
        cirSpeed = Math::Direction(a, g)*1.0f;
        if ( cirSpeed >  1.0f )  cirSpeed =  1.0f;
        if ( cirSpeed < -1.0f )  cirSpeed = -1.0f;

        m_physics->SetMotorSpeedZ(cirSpeed);  // turns left / right
        return true;
    }

    m_progress += event.rTime*m_speed;  // others advance
    m_time += event.rTime;

    if ( m_phase == TRP_DOWN )
    {
        angle = Math::PropAngle(126, -10, m_progress);
        m_object->SetPartRotationZ(2, angle);
        m_object->SetPartRotationZ(4, angle);

        angle = Math::PropAngle(-144, 0, m_progress);
        m_object->SetPartRotationZ(3, angle);
        m_object->SetPartRotationZ(5, angle);
    }

    if ( m_phase == TRP_MOVE )  // preliminary forward/backward?
    {
        dist = glm::distance(m_object->GetPosition(), m_ruin->GetPosition());
        linSpeed = 0.0f;
        if ( dist > RECOVER_DIST )  linSpeed =  1.0f;
        if ( dist < RECOVER_DIST )  linSpeed = -1.0f;
        m_physics->SetMotorSpeedX(linSpeed);  // forward/backward
        return true;
    }

    if ( m_phase == TRP_OPER )
    {
        assert(HasPowerCellSlot(m_object));
        if (CPowerContainerObject* power = GetObjectPowerCell(m_object))
        {
            energy = power->GetEnergy();
            energy -= event.rTime * ENERGY_RECOVER * m_speed;
            power->SetEnergy(energy);
        }


        speed.x = (Math::Rand()-0.5f)*0.1f*m_progress;
        speed.y = (Math::Rand()-0.5f)*0.1f*m_progress;
        speed.z = (Math::Rand()-0.5f)*0.1f*m_progress;
        m_ruin->SetCirVibration(speed);

        if ( m_progress >= 0.75f )
        {
            m_ruin->SetScale(1.0f-(m_progress-0.75f)/0.25f);
        }

        if ( m_progress > 0.5f && m_progress < 0.8f )
        {
            m_metal->SetScale((m_progress-0.5f)/0.3f);
        }

        if ( m_lastParticle+m_engine->ParticleAdapt(0.02f) <= m_time )
        {
            m_lastParticle = m_time;

            pos = m_recoverPos;
            pos.x += (Math::Rand()-0.5f)*8.0f*(1.0f-m_progress);
            pos.z += (Math::Rand()-0.5f)*8.0f*(1.0f-m_progress);
            pos.y -= 4.0f;
            speed.x = (Math::Rand()-0.5f)*0.0f;
            speed.z = (Math::Rand()-0.5f)*0.0f;
            speed.y = Math::Rand()*15.0f;
            dim.x = Math::Rand()*2.0f+1.5f;
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIRECOVER, 1.0f, 0.0f, 0.0f);
        }
    }

    if ( m_phase == TRP_UP )
    {
        angle = Math::PropAngle(-10, 126, m_progress);
        m_object->SetPartRotationZ(2, angle);
        m_object->SetPartRotationZ(4, angle);

        angle = Math::PropAngle(0, -144, m_progress);
        m_object->SetPartRotationZ(3, angle);
        m_object->SetPartRotationZ(5, angle);

        if ( m_lastParticle+m_engine->ParticleAdapt(0.02f) <= m_time )
        {
            m_lastParticle = m_time;

            pos = m_recoverPos;
            pos.y -= 4.0f;
            speed.x = (Math::Rand()-0.5f)*0.0f;
            speed.z = (Math::Rand()-0.5f)*0.0f;
            speed.y = Math::Rand()*15.0f;
            dim.x = Math::Rand()*2.0f+1.5f;
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIRECOVER, 1.0f, 0.0f, 0.0f);
        }
    }

    return true;
}


// Assigns the goal was achieved.

Error CTaskRecover::Start()
{
    m_bError = true;  // operation impossible
    if ( !m_physics->GetLand() )  return ERR_WRONG_BOT;

    ObjectType type = m_object->GetType();
    if ( type != OBJECT_MOBILErr )  return ERR_WRONG_BOT;

    if ( GetObjectEnergy(m_object) < ENERGY_RECOVER+0.05f )  return ERR_RECOVER_ENERGY;

    glm::mat4 mat = m_object->GetWorldMatrix(0);
    glm::vec3 pos = glm::vec3(RECOVER_DIST, 3.3f, 0.0f);
    pos = Math::Transform(mat, pos);  // position in front
    m_recoverPos = pos;

    m_ruin = SearchRuin();
    if ( m_ruin == nullptr )  return ERR_RECOVER_NULL;
    m_ruin->SetLock(true);  // ruin no longer usable

    glm::vec3 iPos = m_object->GetPosition();
    glm::vec3 oPos = m_ruin->GetPosition();
    m_angle = Math::RotateAngle(oPos.x-iPos.x, iPos.z-oPos.z);  // CW !

    m_metal = nullptr;

    m_phase    = TRP_TURN;
    m_progress = 0.0f;
    m_speed    = 1.0f/1.0f;
    m_time     = 0.0f;
    m_lastParticle = 0.0f;

    m_bError = false;  // ok

    m_camera->StartCentering(m_object, Math::PI*0.85f, 99.9f, 10.0f, 3.0f);
    return ERR_OK;
}

// Indicates whether the action is finished.

Error CTaskRecover::IsEnded()
{
    glm::vec3    pos, goal;
    float       angle, dist, time;
    int         i;

    if ( m_engine->GetPause() )  return ERR_CONTINUE;
    if ( m_bError )  return ERR_STOP;

    if ( m_phase == TRP_TURN )  // preliminary rotation?
    {
        angle = m_object->GetRotationY();
        angle = Math::NormAngle(angle);  // 0..2*Math::PI

        if ( Math::TestAngle(angle, m_angle-Math::PI*0.01f, m_angle+Math::PI*0.01f) )
        {
            m_physics->SetMotorSpeedZ(0.0f);

            dist = glm::distance(m_object->GetPosition(), m_ruin->GetPosition());
            if ( dist > RECOVER_DIST )
            {
                time = m_physics->GetLinTimeLength(dist-RECOVER_DIST, 1.0f);
                m_speed = 1.0f/time;
            }
            else
            {
                time = m_physics->GetLinTimeLength(RECOVER_DIST-dist, -1.0f);
                m_speed = 1.0f/time;
            }
            m_phase = TRP_MOVE;
            m_progress = 0.0f;
        }
        return ERR_CONTINUE;
    }

    if ( m_phase == TRP_MOVE )  // preliminary advance?
    {
        dist = glm::distance(m_object->GetPosition(), m_ruin->GetPosition());

        if ( dist >= RECOVER_DIST-1.0f &&
             dist <= RECOVER_DIST+1.0f )
        {
            m_physics->SetMotorSpeedX(0.0f);

            glm::mat4 mat = m_object->GetWorldMatrix(0);
            pos = glm::vec3(RECOVER_DIST, 3.3f, 0.0f);
            pos = Math::Transform(mat, pos);  // position in front
            m_recoverPos = pos;

            i = m_sound->Play(SOUND_MANIP, m_object->GetPosition(), 0.0f, 0.9f, true);
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
                m_ruin->SetLock(false);  // usable again
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
        m_metal = CObjectManager::GetInstancePointer()->CreateObject(m_recoverPos, 0.0f, OBJECT_METAL);
        m_metal->SetLock(true);  // metal not yet usable
        m_metal->SetScale(0.0f);

        glm::mat4 mat = m_object->GetWorldMatrix(0);
        pos = glm::vec3(RECOVER_DIST, 3.1f, 3.9f);
        pos = Math::Transform(mat, pos);
        goal = glm::vec3(RECOVER_DIST, 3.1f, -3.9f);
        goal = Math::Transform(mat, goal);
        m_particle->CreateRay(pos, goal, Gfx::PARTIRAY2,
                              { 2.0f, 2.0f }, 8.0f);

        m_soundChannel = m_sound->Play(SOUND_RECOVER, m_ruin->GetPosition(), 0.0f, 1.0f, true);
        m_sound->AddEnvelope(m_soundChannel, 0.6f, 1.0f, 2.0f, SOPER_CONTINUE);
        m_sound->AddEnvelope(m_soundChannel, 0.6f, 1.0f, 4.0f, SOPER_CONTINUE);
        m_sound->AddEnvelope(m_soundChannel, 0.0f, 0.7f, 2.0f, SOPER_STOP);

        m_phase = TRP_OPER;
        m_speed = 1.0f/8.0f;
        return ERR_CONTINUE;
    }

    if ( m_phase == TRP_OPER )
    {
        m_metal->SetScale(1.0f);

        CObjectManager::GetInstancePointer()->DeleteObject(m_ruin);
        m_ruin = nullptr;

        m_soundChannel = -1;

        i = m_sound->Play(SOUND_MANIP, m_object->GetPosition(), 0.0f, 0.9f, true);
        m_sound->AddEnvelope(i, 1.0f, 1.5f, 0.3f, SOPER_CONTINUE);
        m_sound->AddEnvelope(i, 1.0f, 1.5f, 1.0f, SOPER_CONTINUE);
        m_sound->AddEnvelope(i, 0.0f, 0.9f, 0.3f, SOPER_STOP);

        m_phase = TRP_UP;
        m_speed = 1.0f/1.5f;
        return ERR_CONTINUE;
    }

    m_metal->SetLock(false);  // metal usable

    Abort();
    return ERR_STOP;
}

// Suddenly ends the current action.

bool CTaskRecover::Abort()
{
    m_object->SetPartRotationZ(2,  126.0f*Math::PI/180.0f);
    m_object->SetPartRotationZ(4,  126.0f*Math::PI/180.0f);
    m_object->SetPartRotationZ(3, -144.0f*Math::PI/180.0f);
    m_object->SetPartRotationZ(5, -144.0f*Math::PI/180.0f);  // rest

    if ( m_soundChannel != -1 )
    {
        m_sound->FlushEnvelope(m_soundChannel);
        m_sound->AddEnvelope(m_soundChannel, 1.0f, 1.0f, 1.0f, SOPER_STOP);
        m_soundChannel = -1;
    }

    m_camera->StopCentering(m_object, 2.0f);
    return true;
}


// Seeks if a ruin is in front of the vehicle.

CObject* CTaskRecover::SearchRuin()
{
    return CObjectManager::GetInstancePointer()->FindNearest(nullptr, m_recoverPos, {OBJECT_RUINmobilew1, OBJECT_RUINmobilew2, OBJECT_RUINmobilet1, OBJECT_RUINmobilet2, OBJECT_RUINmobiler1, OBJECT_RUINmobiler2}, 40.0f/g_unit);
}
