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


#include "object/auto/autopara.h"


#include "common/iman.h"

#include "math/geometry.h"

#include "script/cmdtoken.h"

#include "ui/interface.h"
#include "ui/window.h"

#include <stdio.h>
#include <string.h>



// Object's constructor.

CAutoPara::CAutoPara(CObject* object) : CAuto(object)
{
    m_channelSound = -1;
    Init();
}

// Object's destructor.

CAutoPara::~CAutoPara()
{
}


// Destroys the object.

void CAutoPara::DeleteObject(bool bAll)
{
    if ( m_channelSound != -1 )
    {
        m_sound->FlushEnvelope(m_channelSound);
        m_sound->AddEnvelope(m_channelSound, 0.0f, 1.0f, 1.0f, SOPER_STOP);
        m_channelSound = -1;
    }

    CAuto::DeleteObject(bAll);
}


// Initialize the object.

void CAutoPara::Init()
{
    Math::Matrix*   mat;

    m_time = 0.0f;
    m_timeVirus = 0.0f;
    m_lastParticle = 0.0f;

    mat = m_object->GetWorldMatrix(0);
    m_pos = Math::Transform(*mat, Math::Vector(22.0f, 4.0f, 0.0f));

    m_phase    = APAP_WAIT;  // waiting ...
    m_progress = 0.0f;
    m_speed    = 1.0f/1.0f;

    CAuto::Init();
}


// Reception of lightning.

void CAutoPara::StartLightning()
{
    m_phase    = APAP_LIGHTNING;
    m_progress = 0.0f;
    m_speed    = 1.0f/2.0f;
}


// Management of an event.

bool CAutoPara::EventProcess(const Event &event)
{
    Math::Vector    pos, speed;
    Math::Point     dim;
    int         i;

    CAuto::EventProcess(event);

    if ( m_engine->GetPause() )  return true;
    if ( event.type != EVENT_FRAME )  return true;

    m_progress += event.rTime*m_speed;
    m_timeVirus -= event.rTime;

    if ( m_object->GetVirusMode() )  // contaminated by a virus?
    {
        if ( m_timeVirus <= 0.0f )
        {
            m_timeVirus = 0.1f+Math::Rand()*0.3f;
        }
        return true;
    }

    EventProgress(event.rTime);

    if ( m_phase == APAP_LIGHTNING )
    {
        if ( m_progress < 1.0f )
        {
            if ( m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
            {
                m_lastParticle = m_time;

                for ( i=0 ; i<10 ; i++ )
                {
                    pos = m_object->GetPosition(0);
                    pos.x += (Math::Rand()-0.5f)*m_progress*40.0f;
                    pos.z += (Math::Rand()-0.5f)*m_progress*40.0f;
                    pos.y += 50.0f-m_progress*50.0f;
                    speed.x = (Math::Rand()-0.5f)*20.0f;
                    speed.z = (Math::Rand()-0.5f)*20.0f;
                    speed.y = 5.0f+Math::Rand()*5.0f;
                    dim.x = 2.0f;
                    dim.y = dim.x;
                    m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIBLITZ, 1.0f, 20.0f, 0.5f);
                }
            }
        }
        else
        {
            m_phase    = APAP_CHARGE;
            m_progress = 0.0f;
            m_speed    = 1.0f/2.0f;
        }
    }

    if ( m_phase == APAP_CHARGE )
    {
        if ( m_progress < 1.0f )
        {
            if ( m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
            {
                m_lastParticle = m_time;

                for ( i=0 ; i<2 ; i++ )
                {
                    pos = m_object->GetPosition(0);
                    pos.y += 16.0f;
                    speed.x = (Math::Rand()-0.5f)*10.0f;
                    speed.z = (Math::Rand()-0.5f)*10.0f;
                    speed.y = -Math::Rand()*30.0f;
                    dim.x = 1.0f;
                    dim.y = dim.x;
                    m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIBLITZ, 1.0f, 0.0f, 0.0f);
                }
            }

            ChargeObject(event.rTime);
        }
        else
        {
            m_phase    = APAP_WAIT;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.0f;
        }
    }

    return true;
}


// Creates all the interface when the object is selected.

bool CAutoPara::CreateInterface(bool bSelect)
{
    Ui::CWindow*    pw;
    Math::Point     pos, ddim;
    float       ox, oy, sx, sy;

    CAuto::CreateInterface(bSelect);

    if ( !bSelect )  return true;

    pw = static_cast< Ui::CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw == 0 )  return false;

    ox = 3.0f/640.0f;
    oy = 3.0f/480.0f;
    sx = 33.0f/640.0f;
    sy = 33.0f/480.0f;

    pos.x = ox+sx*0.0f;
    pos.y = oy+sy*0;
    ddim.x = 66.0f/640.0f;
    ddim.y = 66.0f/480.0f;
    pw->CreateGroup(pos, ddim, 113, EVENT_OBJECT_TYPE);

    pos.x = ox+sx*10.2f;
    pos.y = oy+sy*0.5f;
    ddim.x = 33.0f/640.0f;
    ddim.y = 33.0f/480.0f;
    pw->CreateButton(pos, ddim, 41, EVENT_OBJECT_LIMIT);

    return true;
}


// Returns an error due the state of the automation.

Error CAutoPara::GetError()
{
    if ( m_object->GetVirusMode() )
    {
        return ERR_BAT_VIRUS;
    }
    return ERR_OK;
}


// Load all objects under the lightning rod.

void CAutoPara::ChargeObject(float rTime)
{
    CObject*    pObj;
    CObject*    power;
    Math::Vector    sPos, oPos;
    float       dist, energy;
    int         i;

    sPos = m_object->GetPosition(0);

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = static_cast< CObject* >(m_iMan->SearchInstance(CLASS_OBJECT, i));
        if ( pObj == 0 )  break;

        oPos = pObj->GetPosition(0);
        dist = Math::Distance(oPos, sPos);
        if ( dist > 20.0f )  continue;

        if ( pObj->GetTruck() == 0 && pObj->GetType() == OBJECT_POWER )
        {
            energy = pObj->GetEnergy();
            energy += rTime/2.0f;
            if ( energy > 1.0f )  energy = 1.0f;
            pObj->SetEnergy(energy);
        }

        power = pObj->GetPower();
        if ( power != 0 && power->GetType() == OBJECT_POWER )
        {
            energy = power->GetEnergy();
            energy += rTime/2.0f;
            if ( energy > 1.0f )  energy = 1.0f;
            power->SetEnergy(energy);
        }

        power = pObj->GetFret();
        if ( power != 0 && power->GetType() == OBJECT_POWER )
        {
            energy = power->GetEnergy();
            energy += rTime/2.0f;
            if ( energy > 1.0f )  energy = 1.0f;
            power->SetEnergy(energy);
        }
    }
}


// Saves all parameters of the controller.

bool CAutoPara::Write(char *line)
{
    char    name[100];

    if ( m_phase == APAP_WAIT )  return false;

    sprintf(name, " aExist=%d", 1);
    strcat(line, name);

    CAuto::Write(line);

    sprintf(name, " aPhase=%d", m_phase);
    strcat(line, name);

    sprintf(name, " aProgress=%.2f", m_progress);
    strcat(line, name);

    sprintf(name, " aSpeed=%.2f", m_speed);
    strcat(line, name);

    return true;
}

// Restores all parameters of the controller.

bool CAutoPara::Read(char *line)
{
    if ( OpInt(line, "aExist", 0) == 0 )  return false;

    CAuto::Read(line);

    m_phase = static_cast< AutoParaPhase >(OpInt(line, "aPhase", APAP_WAIT));
    m_progress = OpFloat(line, "aProgress", 0.0f);
    m_speed = OpFloat(line, "aSpeed", 1.0f);

    m_lastParticle = 0.0f;

    return true;
}


