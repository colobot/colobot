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


#include "object/task/taskfireant.h"

#include "graphics/engine/particle.h"

#include "math/geometry.h"

#include "object/motion/motionant.h"

#include "physics/physics.h"




// Object's constructor.

CTaskFireAnt::CTaskFireAnt(CObject* object) : CTask(object)
{
    m_phase = TFA_NULL;
}

// Object's destructor.

CTaskFireAnt::~CTaskFireAnt()
{
}


// Management of an event.

bool CTaskFireAnt::EventProcess(const Event &event)
{
    Math::Vector    dir, vib;
    float       a, g, cirSpeed;

    if ( m_engine->GetPause() )  return true;
    if ( event.type != EVENT_FRAME )  return true;
    if ( m_bError )  return false;

    if ( m_object->GetFixed() )  // insect on its back?
    {
        m_bError = true;
        return false;
    }

    m_time += event.rTime;
    m_progress += event.rTime*m_speed;

    if ( m_phase == TFA_TURN )  // preliminary rotation?
    {
        a = m_object->GetAngleY(0);
        g = m_angle;
        cirSpeed = Math::Direction(a, g)*2.0f;
        if ( cirSpeed >  2.0f )  cirSpeed =  2.0f;
        if ( cirSpeed < -2.0f )  cirSpeed = -2.0f;

        m_physics->SetMotorSpeedZ(cirSpeed);  // turns left/right
    }

    return true;
}


// Assigns the goal was achieved.

Error CTaskFireAnt::Start(Math::Vector impact)
{
    Math::Vector    pos;
    ObjectType  type;

    m_impact = impact;

    m_bError = true;  // operation impossible
    if ( !m_physics->GetLand() )  return ERR_FIRE_VEH;

    type = m_object->GetType();
    if ( type != OBJECT_ANT )  return ERR_FIRE_VEH;

    // Insect on its back?
    if ( m_object->GetFixed() )  return ERR_FIRE_VEH;

    m_physics->SetMotorSpeed(Math::Vector(0.0f, 0.0f, 0.0f));

    pos = m_object->GetPosition(0);
    m_angle = Math::RotateAngle(m_impact.x-pos.x, pos.z-m_impact.z);  // CW !

    m_phase = TFA_TURN;
    m_speed = 1.0f/1.0f;
    m_progress = 0.0f;
    m_time = 0.0f;
    m_lastParticle = 0.0f;
    m_bError = false;  // ok
    m_bFire = false;  // once!

    return ERR_OK;
}

// Indicates whether the action is finished.

Error CTaskFireAnt::IsEnded()
{
    Math::Matrix*   mat;
    Math::Vector    pos, speed;
    Math::Point     dim;
    float       angle, dist;
    int         i, channel;

    if ( m_engine->GetPause() )  return ERR_CONTINUE;
    if ( m_bError )  return ERR_STOP;
    if ( m_object->GetFixed() )  return ERR_STOP;  // insect on its back?

    if ( m_phase == TFA_TURN )  // rotation ?
    {
        angle = m_object->GetAngleY(0);
        angle = Math::NormAngle(angle);  // 0..2*Math::PI
        if ( !Math::TestAngle(angle, m_angle-Math::PI*0.05f, m_angle+Math::PI*0.05f) )  return ERR_CONTINUE;

        m_physics->SetMotorSpeedZ(0.0f);  // rotation ended

        m_phase = TFA_PREPARE;
//?     m_speed = 1.0f/1.5f;
        m_speed = 1.0f/0.4f;
        m_progress = 0.0f;
//?     m_motion->SetAction(MAS_PREPARE, 1.5f);
        m_motion->SetAction(MAS_PREPARE, 0.4f);
    }

    if ( m_phase == TFA_PREPARE )  // preparation?
    {
        if ( m_progress < 1.0f )  return ERR_CONTINUE;

        m_phase = TFA_FIRE;
//?     m_speed = 1.0f/2.0f;
        m_speed = 1.0f/0.5f;
        m_progress = 0.0f;
//?     m_motion->SetAction(MAS_FIRE, 2.0f);
        m_motion->SetAction(MAS_FIRE, 0.5f);
    }

    if ( m_phase == TFA_FIRE )  // shooting?
    {
        if ( m_progress > 0.75f && !m_bFire )
        {
            m_bFire = true;  // once

            for ( i=0 ; i<20 ; i++ )
            {
                pos = Math::Vector(-2.5f, -0.7f, 0.0f);
                mat = m_object->GetWorldMatrix(2);
                pos = Math::Transform(*mat, pos);
                dist = Math::Distance(pos, m_impact);
                speed = m_impact-pos;
                speed.x += (Math::Rand()-0.5f)*dist*1.2f;
                speed.y += (Math::Rand()-0.5f)*dist*0.4f+50.0f;
                speed.z += (Math::Rand()-0.5f)*dist*1.2f;
                dim.x = 1.0f;
                dim.y = dim.x;
                channel = m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIGUN2, 2.0f, 100.0f, 0.0f);
                m_particle->SetObjectFather(channel, m_object);
            }
        }

        if ( m_progress < 1.0f )  return ERR_CONTINUE;

        m_phase = TFA_TERMINATE;
//?     m_speed = 1.0f/0.9f;
        m_speed = 1.0f/0.4f;
        m_progress = 0.0f;
//?     m_motion->SetAction(MAS_TERMINATE, 0.9f);
        m_motion->SetAction(MAS_TERMINATE, 0.4f);
    }

    if ( m_phase == TFA_TERMINATE )  // ends?
    {
        if ( m_progress < 1.0f )  return ERR_CONTINUE;

        m_phase = TFA_NULL;
        m_speed = 1.0f/1.0f;
        m_progress = 0.0f;
    }

    Abort();
    return ERR_STOP;
}


// Suddenly ends the current action.

bool CTaskFireAnt::Abort()
{
    m_motion->SetAction(-1);
    return true;
}

