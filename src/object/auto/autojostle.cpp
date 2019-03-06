/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
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


#include "object/auto/autojostle.h"

#include "object/old_object.h"


// Object's constructor.

CAutoJostle::CAutoJostle(COldObject* object) : CAuto(object)
{
    Init();
}

// Object's destructor.

CAutoJostle::~CAutoJostle()
{
}


// Destroys the object.

void CAutoJostle::DeleteObject(bool bAll)
{
    CAuto::DeleteObject(bAll);
}


// Initialize the object.

void CAutoJostle::Init()
{
    m_time = 0.0f;
    m_error = ERR_CONTINUE;

    CAuto::Init();
}


// Start an emission.

void CAutoJostle::Start(int param, float force)
{
    ObjectType  type;

    if ( force < 0.0f )  force = 0.0f;
    if ( force > 1.0f )  force = 1.0f;

    m_force = force;
    m_progress = 0.0f;
    m_speed = 1.0f/(0.5f+force*1.0f);  // 0.5 .. 1.5
    m_time = 0.0f;
    m_error = ERR_CONTINUE;

    type = m_object->GetType();
    if ( type >= OBJECT_PLANT5 &&
         type <= OBJECT_PLANT7 )  // clover?
    {
        m_force *= 3.0f;
    }
}

// Should never be called
void CAutoJostle::Start(int param)
{
}


// Management of an event.

bool CAutoJostle::EventProcess(const Event &event)
{
    Math::Vector    dir;
    float       factor, angle, zoom;

    CAuto::EventProcess(event);

    if ( m_engine->GetPause() )  return true;
    if ( event.type != EVENT_FRAME )  return true;

    if ( m_progress < 1.0f )
    {
        m_progress += event.rTime*m_speed;

        if ( m_progress < 0.5f )
        {
            factor = m_progress/0.5f;
        }
        else
        {
            factor = 2.0f-m_progress/0.5f;
        }
        factor *= m_force;

        dir.x = sinf(m_progress*Math::PI*4.0f);
        dir.z = cosf(m_progress*Math::PI*4.0f);

        angle = sinf(m_time*10.0f)*factor*0.04f;
        m_object->SetRotationX(angle*dir.z);
        m_object->SetRotationZ(angle*dir.x);

        zoom = 1.0f+sinf(m_time*8.0f)*factor*0.06f;
        m_object->SetScaleX(zoom);
        zoom = 1.0f+sinf(m_time*5.0f)*factor*0.06f;
        m_object->SetScaleY(zoom);
        zoom = 1.0f+sinf(m_time*7.0f)*factor*0.06f;
        m_object->SetScaleZ(zoom);
    }
    else
    {
        m_object->SetRotationX(0.0f);
        m_object->SetRotationZ(0.0f);
        m_object->SetScale(Math::Vector(1.0f, 1.0f, 1.0f));
        m_error = ERR_STOP;
    }

    return true;
}


// Indicates whether the controller has completed its activity.

Error CAutoJostle::IsEnded()
{
    return m_error;
}

