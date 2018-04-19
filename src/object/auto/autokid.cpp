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


#include "object/auto/autokid.h"

#include "graphics/engine/particle.h"
#include "graphics/engine/water.h"

#include "object/old_object.h"

#include "sound/sound.h"

#include <stdio.h>


// Object's constructor.

CAutoKid::CAutoKid(COldObject* object) : CAuto(object)
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

void CAutoKid::DeleteObject(bool all)
{
    CAuto::DeleteObject(all);
}


// Initialize the object.

void CAutoKid::Init()
{
    m_speed = 1.0f/1.0f;
    m_progress = 0.0f;
    m_lastParticle = 0.0f;

    if ( m_type == OBJECT_TEEN36 )  // trunk ?
    {
        Math::Vector pos = m_object->GetPosition();
        m_speed = 1.0f/(1.0f+(Math::Mod(pos.x/10.0f-0.5f, 1.0f)*0.2f));
        m_progress = Math::Mod(pos.x/10.0f, 1.0f);
    }

    if ( m_type == OBJECT_TEEN37 )  // boat?
    {
        Math::Vector pos = m_object->GetPosition();
        m_speed = 1.0f/(1.0f+(Math::Mod(pos.x/10.0f-0.5f, 1.0f)*0.2f))*2.5f;
        m_progress = Math::Mod(pos.x/10.0f, 1.0f);
    }

    if ( m_type == OBJECT_TEEN38 )  // fan?
    {
        if ( m_soundChannel == -1 )
        {
//?         m_soundChannel = m_sound->Play(SOUND_MANIP, m_object->GetPosition(0), 1.0f, 0.5f, true);
            m_silent = false;
        }
    }
}


// Management of an event.

bool CAutoKid::EventProcess(const Event &event)
{
    CAuto::EventProcess(event);

    if ( m_soundChannel != -1 )
    {
        if ( m_engine->GetPause() )
        {
            if ( !m_silent )
            {
                m_sound->AddEnvelope(m_soundChannel, 0.0f, 0.5f, 0.1f, SOPER_CONTINUE);
                m_silent = true;
            }
        }
        else
        {
            if ( m_silent )
            {
                m_sound->AddEnvelope(m_soundChannel, 1.0f, 0.5f, 0.1f, SOPER_CONTINUE);
                m_silent = false;
            }
        }
    }

    if ( m_engine->GetPause() )  return true;
    if ( event.type != EVENT_FRAME )  return true;

    m_progress += event.rTime*m_speed;

    if ( m_type == OBJECT_TEEN36 )  // trunk?
    {
        Math::Vector vib;
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

            Math::Vector pos;
            pos = m_object->GetPosition();
            pos.y = m_water->GetLevel()+1.0f;
            pos.x += (Math::Rand()-0.5f)*50.0f;
            pos.z += (Math::Rand()-0.5f)*50.0f;
            Math::Vector speed;
            speed.y = 0.0f;
            speed.x = 0.0f;
            speed.z = 0.0f;
            Math::Point dim;
            dim.x = 50.0f;
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIFLIC, 3.0f, 0.0f, 0.0f);
        }
    }

    if ( m_type == OBJECT_TEEN37 )  // boat?
    {
        Math::Vector vib;
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

            Math::Vector pos;
            pos = m_object->GetPosition();
            pos.y = m_water->GetLevel()+1.0f;
            pos.x += (Math::Rand()-0.5f)*20.0f;
            pos.z += (Math::Rand()-0.5f)*20.0f;
            Math::Vector speed;
            speed.y = 0.0f;
            speed.x = 0.0f;
            speed.z = 0.0f;
            Math::Point dim;
            dim.x = 20.0f;
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIFLIC, 3.0f, 0.0f, 0.0f);
        }
    }

    if ( m_type == OBJECT_TEEN38 )  // fan?
    {
        m_object->SetPartRotationY(1, sinf(m_progress*0.6f)*0.4f);
        m_object->SetPartRotationX(2, m_progress*5.0f);
    }

    return true;
}


// Returns an error due the state of the automation.

Error CAutoKid::GetError()
{
    return ERR_OK;
}
