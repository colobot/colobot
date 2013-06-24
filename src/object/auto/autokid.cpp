// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// * Copyright (C) 2012, Polish Portal of Colobot (PPC)
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


#include "object/auto/autokid.h"

#include "graphics/engine/particle.h"
#include "graphics/engine/water.h"

#include <stdio.h>


// Object's constructor.

CAutoKid::CAutoKid(CObject* object) : CAuto(object)
{
    m_soundChannel = -1;
    Init();
}

// Object's constructor.

CAutoKid::~CAutoKid()
{
    if ( m_soundChannel != -1 )
    {
        m_sound->FlushEnvelope(m_soundChannel);
        m_sound->AddEnvelope(m_soundChannel, 0.0f, 1.0f, 1.0f, SOPER_STOP);
        m_soundChannel = -1;
    }
}


// Destroys the object.

void CAutoKid::DeleteObject(bool bAll)
{
    CAuto::DeleteObject(bAll);
}


// Initialize the object.

void CAutoKid::Init()
{
    Math::Vector    pos;

    m_speed = 1.0f/1.0f;
    m_progress = 0.0f;
    m_lastParticle = 0.0f;

    if ( m_type == OBJECT_TEEN36 )  // trunk ?
    {
        pos = m_object->GetPosition(0);
        m_speed = 1.0f/(1.0f+(Math::Mod(pos.x/10.0f-0.5f, 1.0f)*0.2f));
        m_progress = Math::Mod(pos.x/10.0f, 1.0f);
    }

    if ( m_type == OBJECT_TEEN37 )  // boat?
    {
        pos = m_object->GetPosition(0);
        m_speed = 1.0f/(1.0f+(Math::Mod(pos.x/10.0f-0.5f, 1.0f)*0.2f))*2.5f;
        m_progress = Math::Mod(pos.x/10.0f, 1.0f);
    }

    if ( m_type == OBJECT_TEEN38 )  // fan?
    {
        if ( m_soundChannel == -1 )
        {
//?         m_soundChannel = m_sound->Play(SOUND_MANIP, m_object->GetPosition(0), 1.0f, 0.5f, true);
            m_bSilent = false;
        }
    }
}


// Management of an event.

bool CAutoKid::EventProcess(const Event &event)
{
    Math::Vector    vib, pos, speed;
    Math::Point     dim;

    CAuto::EventProcess(event);

    if ( m_soundChannel != -1 )
    {
        if ( m_engine->GetPause() )
        {
            if ( !m_bSilent )
            {
                m_sound->AddEnvelope(m_soundChannel, 0.0f, 0.5f, 0.1f, SOPER_CONTINUE);
                m_bSilent = true;
            }
        }
        else
        {
            if ( m_bSilent )
            {
                m_sound->AddEnvelope(m_soundChannel, 1.0f, 0.5f, 0.1f, SOPER_CONTINUE);
                m_bSilent = false;
            }
        }
    }

    if ( m_engine->GetPause() )  return true;
    if ( event.type != EVENT_FRAME )  return true;

    m_progress += event.rTime*m_speed;

    if ( m_type == OBJECT_TEEN36 )  // trunk?
    {
        vib.x = 0.0f;
        vib.y = sinf(m_progress)*1.0f;
        vib.z = 0.0f;
        m_object->SetLinVibration(vib);

        vib.x = 0.0f;
        vib.y = 0.0f;
        vib.z = sinf(m_progress*0.5f)*0.05f;
        m_object->SetCirVibration(vib);

        if ( m_lastParticle+m_engine->ParticleAdapt(0.15f) <= m_time )
        {
            m_lastParticle = m_time;

            pos = m_object->GetPosition(0);
            pos.y = m_water->GetLevel()+1.0f;
            pos.x += (Math::Rand()-0.5f)*50.0f;
            pos.z += (Math::Rand()-0.5f)*50.0f;
            speed.y = 0.0f;
            speed.x = 0.0f;
            speed.z = 0.0f;
            dim.x = 50.0f;
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIFLIC, 3.0f, 0.0f, 0.0f);
        }
    }

    if ( m_type == OBJECT_TEEN37 )  // boat?
    {
        vib.x = 0.0f;
        vib.y = sinf(m_progress)*1.0f;
        vib.z = 0.0f;
        m_object->SetLinVibration(vib);

        vib.x = 0.0f;
        vib.y = 0.0f;
        vib.z = sinf(m_progress*0.5f)*0.15f;
        m_object->SetCirVibration(vib);

        if ( m_lastParticle+m_engine->ParticleAdapt(0.15f) <= m_time )
        {
            m_lastParticle = m_time;

            pos = m_object->GetPosition(0);
            pos.y = m_water->GetLevel()+1.0f;
            pos.x += (Math::Rand()-0.5f)*20.0f;
            pos.z += (Math::Rand()-0.5f)*20.0f;
            speed.y = 0.0f;
            speed.x = 0.0f;
            speed.z = 0.0f;
            dim.x = 20.0f;
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIFLIC, 3.0f, 0.0f, 0.0f);
        }
    }

    if ( m_type == OBJECT_TEEN38 )  // fan?
    {
        m_object->SetAngleY(1, sinf(m_progress*0.6f)*0.4f);
        m_object->SetAngleX(2, m_progress*5.0f);
    }

    return true;
}


// Returns an error due the state of the automation.

Error CAutoKid::GetError()
{
    return ERR_OK;
}

