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


#include "object/task/taskreset.h"

#include "common/iman.h"

#include "object/brain.h"
#include "object/robotmain.h"



const float RESET_DELAY_ZOOM = 0.7f;
const float RESET_DELAY_MOVE = 0.7f;




// Object's constructor.

CTaskReset::CTaskReset(CObject* object) : CTask(object)
{
}

// Object's destructor.

CTaskReset::~CTaskReset()
{
}


// Management of an event.

bool CTaskReset::EventProcess(const Event &event)
{
    Math::Vector    pos, speed;
    Math::Point     dim;
    float       angle, duration;

    if ( m_engine->GetPause() )  return true;
    if ( event.type != EVENT_FRAME )  return true;
    if ( m_bError )  return false;

    m_time += event.rTime;
    m_progress += event.rTime*m_speed;

    if ( m_phase == TRSP_ZOUT )
    {
        angle = m_iAngle;
        angle += powf(m_progress*5.0f, 2.0f);  // accelerates
        m_object->SetAngleY(0, angle);
        m_object->SetZoom(0, 1.0f-m_progress);

        if ( m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
        {
            m_lastParticle = m_time;

            pos = m_begin;
            pos.x += (Math::Rand()-0.5f)*5.0f;
            pos.z += (Math::Rand()-0.5f)*5.0f;
            speed.x = 0.0f;
            speed.z = 0.0f;
            speed.y = 5.0f+Math::Rand()*5.0f;
            dim.x = Math::Rand()*2.0f+2.0f;
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIGLINTb, 2.0f);

            pos = m_begin;
            speed.x = (Math::Rand()-0.5f)*20.0f;
            speed.z = (Math::Rand()-0.5f)*20.0f;
            speed.y = Math::Rand()*10.0f;
            speed *= 1.0f-m_progress*0.5f;
            pos += speed*1.5f;
            speed = -speed;
            dim.x = 0.6f;
            dim.y = dim.x;
            pos.y += dim.y;
            duration = Math::Rand()*1.5f+1.5f;
            m_particle->CreateTrack(pos, speed, dim, Gfx::PARTITRACK6,
                                     duration, 0.0f,
                                     duration*0.9f, 0.7f);
        }
    }

    if ( m_phase == TRSP_MOVE )
    {
        pos = m_begin+(m_goal-m_begin)*m_progress;
        m_object->SetPosition(0, pos);

        if ( m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
        {
            m_lastParticle = m_time;

            pos.x += (Math::Rand()-0.5f)*5.0f;
            pos.z += (Math::Rand()-0.5f)*5.0f;
            speed.x = 0.0f;
            speed.z = 0.0f;
            speed.y = 2.0f+Math::Rand()*2.0f;
            dim.x = Math::Rand()*2.0f+2.0f;
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIGLINTb, 2.0f);
        }
    }

    if ( m_phase == TRSP_ZIN )
    {
        angle = m_angle.y;
        angle += -powf((1.0f-m_progress)*5.0f, 2.0f);  // slows
        m_object->SetAngleY(0, angle);
        m_object->SetZoom(0, m_progress);

        if ( m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
        {
            m_lastParticle = m_time;

            pos = m_goal;
            pos.x += (Math::Rand()-0.5f)*5.0f;
            pos.z += (Math::Rand()-0.5f)*5.0f;
            speed.x = 0.0f;
            speed.z = 0.0f;
            speed.y = 5.0f+Math::Rand()*5.0f;
            dim.x = Math::Rand()*2.0f+2.0f;
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIGLINTb, 2.0f);

            pos = m_goal;
            speed.x = (Math::Rand()-0.5f)*20.0f;
            speed.z = (Math::Rand()-0.5f)*20.0f;
            speed.y = Math::Rand()*10.0f;
            speed *= 0.5f+m_progress*0.5f;
            dim.x = 0.6f;
            dim.y = dim.x;
            pos.y += dim.y;
            duration = Math::Rand()*1.5f+1.5f;
            m_particle->CreateTrack(pos, speed, dim, Gfx::PARTITRACK6,
                                     duration, 0.0f,
                                     duration*0.9f, 0.7f);
        }
    }

    return true;
}


// Assigns the goal was achieved.
// A positive angle makes a turn right.

Error CTaskReset::Start(Math::Vector goal, Math::Vector angle)
{
    CObject*    fret;
    int     i;

    fret = m_object->GetFret();
    if ( fret != 0 && fret->GetResetCap() == RESET_MOVE )
    {
        fret->SetTruck(0);
        m_object->SetFret(0);  // does nothing
    }

    if ( !m_main->GetNiceReset() )  // quick return?
    {
        m_object->SetPosition(0, goal);
        m_object->SetAngle(0, angle);
        m_brain->RunProgram(m_object->GetResetRun());

        m_bError = false;
        return ERR_OK;
    }

    m_begin = m_object->GetPosition(0);
    m_goal = goal;
    m_angle = angle;

    if ( SearchVehicle() )  // starting location occupied?
    {
        m_bError = true;
        return ERR_RESET_NEAR;
    }

    m_iAngle = m_object->GetAngleY(0);
    m_time = 0.0f;
    m_phase = TRSP_ZOUT;
    m_speed = 1.0f/RESET_DELAY_ZOOM;
    m_progress = 0.0f;
    m_lastParticle = 0.0f;

    m_object->SetResetBusy(true);

    i = m_sound->Play(SOUND_GGG, m_begin, 1.0f, 2.0f, true);
    m_sound->AddEnvelope(i, 0.0f, 0.5f, RESET_DELAY_ZOOM, SOPER_STOP);

    m_bError = false;
    return ERR_OK;
}

// Indicates whether the action is finished.

Error CTaskReset::IsEnded()
{
    CObject*    power;
    float       dist;
    int         i;

    if ( !m_main->GetNiceReset() )  // quick return?
    {
        return ERR_STOP;
    }

    if ( m_engine->GetPause() )  return ERR_CONTINUE;
    if ( m_bError )  return ERR_STOP;
    if ( m_progress < 1.0f )  return ERR_CONTINUE;

    if ( m_phase == TRSP_ZOUT )
    {
        dist = Math::Distance(m_begin, m_goal);
        m_phase = TRSP_MOVE;
        m_speed = 1.0f/(dist*RESET_DELAY_MOVE/100.0f);
        m_progress = 0.0f;
        return ERR_CONTINUE;
    }

    if ( m_phase == TRSP_MOVE )
    {
        m_object->SetPosition(0, m_goal);
        m_object->SetAngle(0, m_angle);

        i = m_sound->Play(SOUND_GGG, m_goal, 1.0f, 0.5f, true);
        m_sound->AddEnvelope(i, 0.0f, 2.0f, RESET_DELAY_ZOOM, SOPER_STOP);

        m_phase = TRSP_ZIN;
        m_speed = 1.0f/RESET_DELAY_ZOOM;
        m_progress = 0.0f;
        return ERR_CONTINUE;
    }

    m_object->SetAngle(0, m_angle);
    m_object->SetZoom(0, 1.0f);

    power = m_object->GetPower();
    if ( power != 0 )
    {
        power->SetEnergy(power->GetCapacity());  // refueling
    }

    m_brain->RunProgram(m_object->GetResetRun());
    m_object->SetResetBusy(false);
    return ERR_STOP;
}


// Seeks if a vehicle is too close.

bool CTaskReset::SearchVehicle()
{
    CObject*    pObj;
    Math::Vector    oPos;
    ObjectType  type;
    float       oRadius, dist;
    int         i;

    CInstanceManager* iMan = CInstanceManager::GetInstancePointer();

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = static_cast<CObject*>(iMan->SearchInstance(CLASS_OBJECT, i));
        if ( pObj == 0 )  break;

        if ( pObj == m_object )  continue;

        type = pObj->GetType();
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
        dist = Math::Distance(oPos, m_goal)-oRadius;

        if ( dist < 5.0f )  return true;
    }

    return false;
}

