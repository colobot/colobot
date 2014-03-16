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

#include "object/task/taskrecover.h"

#include "common/iman.h"

#include "graphics/engine/particle.h"

#include "math/geometry.h"

#include "physics/physics.h"

#include "object/robotmain.h"


const float ENERGY_RECOVER  = 0.25f;        // energy consumed by recovery
const float RECOVER_DIST    = 11.8f;



// Object's constructor.

CTaskRecover::CTaskRecover(CObject* object) : CTask(object)
{
    m_ruin = 0;
    m_soundChannel = -1;
}

// Object's constructor.

CTaskRecover::~CTaskRecover()
{
}


// Management of an event.

bool CTaskRecover::EventProcess(const Event &event)
{
    CObject*    power;
    Math::Vector    pos, speed;
    Math::Point     dim;
    float       a, g, cirSpeed, angle, energy, dist, linSpeed;

    if ( m_engine->GetPause() )  return true;
    if ( event.type != EVENT_FRAME )  return true;
    if ( m_bError )  return false;

    if ( m_phase == TRP_TURN )  // preliminary rotation?
    {
        a = m_object->GetAngleY(0);
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
        m_object->SetAngleZ(2, angle);
        m_object->SetAngleZ(4, angle);

        angle = Math::PropAngle(-144, 0, m_progress);
        m_object->SetAngleZ(3, angle);
        m_object->SetAngleZ(5, angle);
    }

    if ( m_phase == TRP_MOVE )  // preliminary forward/backward?
    {
        dist = Math::Distance(m_object->GetPosition(0), m_ruin->GetPosition(0));
        linSpeed = 0.0f;
        if ( dist > RECOVER_DIST )  linSpeed =  1.0f;
        if ( dist < RECOVER_DIST )  linSpeed = -1.0f;
        m_physics->SetMotorSpeedX(linSpeed);  // forward/backward
        return true;
    }

    if ( m_phase == TRP_OPER )
    {
        power = m_object->GetPower();
        if ( power != 0 )
        {
            energy = power->GetEnergy();
            energy -= event.rTime * ENERGY_RECOVER / power->GetCapacity() * m_speed;
            power->SetEnergy(energy);
        }


        speed.x = (Math::Rand()-0.5f)*0.1f*m_progress;
        speed.y = (Math::Rand()-0.5f)*0.1f*m_progress;
        speed.z = (Math::Rand()-0.5f)*0.1f*m_progress;
        m_ruin->SetCirVibration(speed);

        if ( m_progress >= 0.75f )
        {
            m_ruin->SetZoom(0, 1.0f-(m_progress-0.75f)/0.25f);
        }

        if ( m_progress > 0.5f && m_progress < 0.8f )
        {
            m_metal->SetZoom(0, (m_progress-0.5f)/0.3f);
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
        m_object->SetAngleZ(2, angle);
        m_object->SetAngleZ(4, angle);

        angle = Math::PropAngle(0, -144, m_progress);
        m_object->SetAngleZ(3, angle);
        m_object->SetAngleZ(5, angle);

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
    CObject*    power;
    Math::Matrix*   mat;
    Math::Vector    pos, iPos, oPos;
    float       energy;

    ObjectType  type;

    m_bError = true;  // operation impossible
    if ( !m_physics->GetLand() )  return ERR_RECOVER_VEH;

    type = m_object->GetType();
    if ( type != OBJECT_MOBILErr )  return ERR_RECOVER_VEH;

    power = m_object->GetPower();
    if ( power == 0 )  return ERR_RECOVER_ENERGY;
    energy = power->GetEnergy();
    if ( energy < ENERGY_RECOVER/power->GetCapacity()+0.05f )  return ERR_RECOVER_ENERGY;

    mat = m_object->GetWorldMatrix(0);
    pos = Math::Vector(RECOVER_DIST, 3.3f, 0.0f);
    pos = Transform(*mat, pos);  // position in front
    m_recoverPos = pos;

    m_ruin = SearchRuin();
    if ( m_ruin == 0 )  return ERR_RECOVER_NULL;
    m_ruin->SetLock(true);  // ruin no longer usable

    iPos = m_object->GetPosition(0);
    oPos = m_ruin->GetPosition(0);
    m_angle = Math::RotateAngle(oPos.x-iPos.x, iPos.z-oPos.z);  // CW !

    m_metal = 0;

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
    Math::Matrix*   mat;
    Math::Vector    pos, speed, goal;
    Math::Point     dim;
    float       angle, dist, time;
    int         i;

    if ( m_engine->GetPause() )  return ERR_CONTINUE;
    if ( m_bError )  return ERR_STOP;

    if ( m_phase == TRP_TURN )  // preliminary rotation?
    {
        angle = m_object->GetAngleY(0);
        angle = Math::NormAngle(angle);  // 0..2*Math::PI

        if ( Math::TestAngle(angle, m_angle-Math::PI*0.01f, m_angle+Math::PI*0.01f) )
        {
            m_physics->SetMotorSpeedZ(0.0f);

            dist = Math::Distance(m_object->GetPosition(0), m_ruin->GetPosition(0));
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
        dist = Math::Distance(m_object->GetPosition(0), m_ruin->GetPosition(0));

        if ( dist >= RECOVER_DIST-1.0f &&
             dist <= RECOVER_DIST+1.0f )
        {
            m_physics->SetMotorSpeedX(0.0f);

            mat = m_object->GetWorldMatrix(0);
            pos = Math::Vector(RECOVER_DIST, 3.3f, 0.0f);
            pos = Transform(*mat, pos);  // position in front
            m_recoverPos = pos;

            i = m_sound->Play(SOUND_MANIP, m_object->GetPosition(0), 0.0f, 0.9f, true);
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
        m_metal = new CObject();
        if ( !m_metal->CreateResource(m_recoverPos, 0.0f, OBJECT_METAL) )
        {
            delete m_metal;
            m_metal = 0;
            Abort();
            m_bError = true;
            m_main->DisplayError(ERR_TOOMANY, m_object);
            return ERR_STOP;
        }
        m_metal->SetLock(true);  // metal not yet usable
        m_metal->SetZoom(0, 0.0f);

        mat = m_object->GetWorldMatrix(0);
        pos = Math::Vector(RECOVER_DIST, 3.1f, 3.9f);
        pos = Transform(*mat, pos);
        goal = Math::Vector(RECOVER_DIST, 3.1f, -3.9f);
        goal = Transform(*mat, goal);
        m_particle->CreateRay(pos, goal, Gfx::PARTIRAY2,
                               Math::Point(2.0f, 2.0f), 8.0f);

        m_soundChannel = m_sound->Play(SOUND_RECOVER, m_ruin->GetPosition(0), 0.0f, 1.0f, true);
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

        i = m_sound->Play(SOUND_MANIP, m_object->GetPosition(0), 0.0f, 0.9f, true);
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
    m_object->SetAngleZ(2,  126.0f*Math::PI/180.0f);
    m_object->SetAngleZ(4,  126.0f*Math::PI/180.0f);
    m_object->SetAngleZ(3, -144.0f*Math::PI/180.0f);
    m_object->SetAngleZ(5, -144.0f*Math::PI/180.0f);  // rest

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
    CObject     *pObj, *pBest;
    Math::Vector    oPos;
    ObjectType  type;
    float       dist, min;
    int         i;

    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();

    pBest = 0;
    min = 100000.0f;
    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
        if ( pObj == 0 )  break;

        type = pObj->GetType();
        if ( type == OBJECT_RUINmobilew1 ||
             type == OBJECT_RUINmobilew2 ||
             type == OBJECT_RUINmobilet1 ||
             type == OBJECT_RUINmobilet2 ||
             type == OBJECT_RUINmobiler1 ||
             type == OBJECT_RUINmobiler2 )  // vehicle in ruin?
        {
            oPos = pObj->GetPosition(0);
            dist = Math::Distance(oPos, m_recoverPos);
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

