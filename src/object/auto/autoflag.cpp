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


#include "object/auto/autoflag.h"

#include "graphics/engine/terrain.h"

#include "math/geometry.h"

#include "object/old_object.h"


// Object's constructor.

CAutoFlag::CAutoFlag(COldObject* object) : CAuto(object)
{
    Init();
}

// Object's destructor.

CAutoFlag::~CAutoFlag()
{
}


// Destroys the object.

void CAutoFlag::DeleteObject(bool bAll)
{
    CAuto::DeleteObject(bAll);
}


// Initialize the object.

void CAutoFlag::Init()
{
    Math::Vector    wind;
    float       angle;

    m_time = 0.0f;
    m_param = 0;
    m_progress = 0.0f;

    wind = m_terrain->GetWind();
    angle = Math::RotateAngle(wind.x, -wind.z);
    m_object->SetRotationY(angle);  // directs the flag in the wind

    m_strong = wind.Length();
}


// Beginning of an action (1 = shakes).

void CAutoFlag::Start(int param)
{
    if ( m_param == 0 )
    {
        m_param = param;
        m_progress = 0.0f;
    }
}


// Management of an event.

bool CAutoFlag::EventProcess(const Event &event)
{
    float   angle;
    int     i;

    CAuto::EventProcess(event);

    if ( m_engine->GetPause() )  return true;
    if ( event.type != EVENT_FRAME )  return true;

    if ( m_param == 1 )  // shakes?
    {
        m_progress += event.rTime*(1.0f/2.0f);
        if ( m_progress < 1.0f )
        {
            angle = sinf(m_progress*Math::PI*8.0f)*0.3f*(1.0f-m_progress);
            m_object->SetRotationX(angle);
            angle = sinf(m_progress*Math::PI*4.0f)*0.3f*(1.0f-m_progress);
            m_object->SetRotationZ(angle);
        }
        else
        {
            m_object->SetRotationX(0.0f);
            m_object->SetRotationZ(0.0f);
            m_param = 0;
            m_progress = 0.0f;
        }
    }

    if ( m_strong == 0.0f )  return true;  // no wind?

    for ( i=0 ; i<4 ; i++ )
    {
        angle = sinf(m_time*6.0f+i*2.0f)*((i+2.0f)*0.1f);
        m_object->SetPartRotationY(1+i, angle);
    }

    return true;
}


// Returns an error due the state of the automation

Error CAutoFlag::GetError()
{
    return ERR_OK;
}
