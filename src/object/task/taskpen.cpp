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


#include "object/task/taskpen.h"

#include "graphics/engine/particle.h"

#include "object/object.h"

#include "math/geometry.h"



// Object's constructor.

CTaskPen::CTaskPen(CObject* object) : CTask(object)
{
}

// Object's destructor.

CTaskPen::~CTaskPen()
{
}


// Management of an event.

bool CTaskPen::EventProcess(const Event &event)
{
    Math::Vector    pos, speed;
    Math::Point     dim;
    int         i;

    if ( m_engine->GetPause() )  return true;
    if ( event.type != EVENT_FRAME )  return true;
    if ( m_bError )  return false;

    if ( m_delay == 0.0f )
    {
        m_progress = 1.0f;
    }
    else
    {
        m_progress += event.rTime*(1.0f/m_delay);  // others advance
        if ( m_progress > 1.0f )  m_progress = 1.0f;
    }

    m_time += event.rTime;

    if ( m_phase == TPP_UP )  // back the pencil
    {
        i = AngleToRank(m_object->GetAngleY(1));
        pos = m_object->GetPosition(10+i);
        pos.y = -3.2f*(1.0f-m_progress);
        m_object->SetPosition(10+i, pos);
    }

    if ( m_phase == TPP_TURN )  // turns the carousel?
    {
        if ( m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
        {
            m_lastParticle = m_time;

            pos = m_supportPos;
            pos.x += (Math::Rand()-0.5f)*5.0f;
            pos.z += (Math::Rand()-0.5f)*5.0f;
            speed.x = (Math::Rand()-0.5f)*3.0f;
            speed.z = (Math::Rand()-0.5f)*3.0f;
            speed.y = Math::Rand()*2.0f;
            dim.x = Math::Rand()*1.5f+2.0f;
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISMOKE3, 4.0f);
        }

        m_object->SetAngleY(1, m_oldAngle+(m_newAngle-m_oldAngle)*m_progress);
    }

    if ( m_phase == TPP_DOWN )  // down the pencil?
    {
        if ( m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
        {
            m_lastParticle = m_time;

            pos = m_supportPos;
            pos.x += (Math::Rand()-0.5f)*5.0f;
            pos.z += (Math::Rand()-0.5f)*5.0f;
            speed.x = (Math::Rand()-0.5f)*3.0f;
            speed.z = (Math::Rand()-0.5f)*3.0f;
            speed.y = Math::Rand()*5.0f;
            dim.x = Math::Rand()*1.0f+1.0f;
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIVAPOR, 4.0f);
        }

        i = AngleToRank(m_object->GetAngleY(1));
        pos = m_object->GetPosition(10+i);
        if ( m_timeDown == 0.0f )
        {
            pos.y = 0.0f;
        }
        else
        {
            pos.y = -3.2f*Math::Bounce(Math::Min(m_progress*1.8f, 1.0f));
        }
        m_object->SetPosition(10+i, pos);
    }

    return true;
}


// Assigns the goal has achieved.

Error CTaskPen::Start(bool bDown, int color)
{
    Math::Vector    pos;
    Math::Matrix*   mat;
    ObjectType  type;
    int         i;

    m_bError = true;  // operation impossible

    type = m_object->GetType();
    if ( type != OBJECT_MOBILEdr )  return ERR_FIRE_VEH;

    m_bError = false;  // ok

    m_oldAngle = m_object->GetAngleY(1);
    m_newAngle = ColorToAngle(color);

    i = AngleToRank(m_oldAngle);
    pos = m_object->GetPosition(10+i);

    if ( pos.y == 0.0f )  // pencil at the top?
    {
        m_timeUp = 0.0f;
    }
    else    // pencil on the bottom?
    {
        m_timeUp = 1.0f;  // must rise up
    }

    if ( bDown )  // must go down ?
    {
        m_timeDown = 0.7f;
    }
    else
    {
        m_timeDown = 0.0f;
    }

    mat = m_object->GetWorldMatrix(0);
    pos = Math::Vector(-3.0f, 7.0f, 0.0f);
    pos = Math::Transform(*mat, pos);  // position of carousel
    m_supportPos = pos;

    m_phase    = TPP_UP;
    m_progress = 0.0f;
    m_delay    = m_timeUp;
    m_time     = 0.0f;

    if ( m_timeUp > 0.0f )
    {
        SoundManip(m_timeUp, 1.0f, 0.5f);
    }

    m_lastParticle = 0.0f;

//? m_camera->StartCentering(m_object, Math::PI*0.60f, 99.9f, 5.0f, 0.5f);

    return ERR_OK;
}

// Indicates whether the action is finished.

Error CTaskPen::IsEnded()
{
    if ( m_engine->GetPause() )  return ERR_CONTINUE;
    if ( m_bError )  return ERR_STOP;

    if ( m_progress < 1.0f )  return ERR_CONTINUE;
    m_progress = 0.0f;

    if ( m_phase == TPP_UP )
    {
        m_phase    = TPP_TURN;
        m_progress = 0.0f;
        m_delay    = fabs(m_oldAngle-m_newAngle)/Math::PI;
        m_time     = 0.0f;
        m_lastParticle = 0.0f;
        if ( m_delay > 0.0f )
        {
            SoundManip(m_delay, 1.0f, 1.0f);
        }
        return ERR_CONTINUE;
    }

    if ( m_phase == TPP_TURN )
    {
        m_sound->Play(SOUND_PSHHH2, m_supportPos, 1.0f, 1.4f);
        m_phase    = TPP_DOWN;
        m_progress = 0.0f;
        m_delay    = m_timeDown;
        m_time     = 0.0f;
        m_lastParticle = 0.0f;
        return ERR_CONTINUE;
    }

    Abort();
    return ERR_STOP;
}

// Suddenly ends the current action.

bool CTaskPen::Abort()
{
//? m_camera->StopCentering(m_object, 0.5f);
    return true;
}


// Plays the sound of the manipulator arm.

void CTaskPen::SoundManip(float time, float amplitude, float frequency)
{
    int     i;

    i = m_sound->Play(SOUND_MANIP, m_object->GetPosition(0), 0.0f, 0.3f*frequency, true);
    m_sound->AddEnvelope(i, 0.5f*amplitude, 1.0f*frequency, 0.1f, SOPER_CONTINUE);
    m_sound->AddEnvelope(i, 0.5f*amplitude, 1.0f*frequency, time-0.1f, SOPER_CONTINUE);
    m_sound->AddEnvelope(i, 0.0f, 0.3f*frequency, 0.1f, SOPER_STOP);
}


// Converting a angle to number of pencil.

int CTaskPen::AngleToRank(float angle)
{
//? return (int)(angle/(-45.0f*Math::PI/180.0f));
    angle = -angle;
    angle += (45.0f*Math::PI/180.0f)/2.0f;
    return static_cast<int>(angle/(45.0f*Math::PI/180.0f));
}

// Converting a color to the angle of carousel of pencils.

float CTaskPen::ColorToAngle(int color)
{
    return -45.0f*Math::PI/180.0f*ColorToRank(color);
}

// Converting a color number to the pencil (0 .. 7).

int CTaskPen::ColorToRank(int color)
{
    if ( color ==  8 )  return 1;  // yellow
    if ( color ==  7 )  return 2;  // orange
    if ( color ==  5 )  return 2;  // pink
    if ( color ==  4 )  return 3;  // red
    if ( color ==  6 )  return 4;  // purple
    if ( color == 14 )  return 5;  // blue
    if ( color == 15 )  return 5;  // light blue
    if ( color == 12 )  return 6;  // green
    if ( color == 13 )  return 6;  // light green
    if ( color == 10 )  return 7;  // brown
    if ( color ==  9 )  return 7;  // beige
    return 0;  // black
}

