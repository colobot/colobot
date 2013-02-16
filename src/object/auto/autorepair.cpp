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


#include "object/auto/autorepair.h"

#include "common/iman.h"

#include "physics/physics.h"

#include "script/cmdtoken.h"

#include "ui/interface.h"
#include "ui/window.h"

#include <stdio.h>
#include <string.h>


// Object's constructor.

CAutoRepair::CAutoRepair(CObject* object) : CAuto(object)
{
    Init();
    m_phase = ARP_WAIT;  // paused until the first Init ()
}

// Object's destructor.

CAutoRepair::~CAutoRepair()
{
}


// Destroys the object.

void CAutoRepair::DeleteObject(bool bAll)
{
    CAuto::DeleteObject(bAll);
}


// Initialize the object.

void CAutoRepair::Init()
{
    m_phase    = ARP_WAIT;
    m_progress = 0.0f;
    m_speed    = 1.0f/1.0f;

    m_time     = 0.0f;
    m_timeVirus = 0.0f;
    m_lastParticle = 0.0f;

    CAuto::Init();
}


// Management of an event.

bool CAutoRepair::EventProcess(const Event &event)
{
    CObject*    vehicule;
    Math::Vector    pos, speed;
    Math::Point     dim;
    float       angle, shield;

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

    if ( m_phase == ARP_WAIT )
    {
        if ( m_progress >= 1.0f )
        {
            if ( SearchVehicle() == 0 )
            {
                m_phase    = ARP_WAIT;  // still waiting ...
                m_progress = 0.0f;
                m_speed    = 1.0f/1.0f;
            }
            else
            {
                m_sound->Play(SOUND_OPEN, m_object->GetPosition(0), 1.0f, 0.8f);

                m_phase    = ARP_DOWN;
                m_progress = 0.0f;
                m_speed    = 1.0f/3.0f;
            }
        }
    }

    if ( m_phase == ARP_DOWN )
    {
        if ( m_progress < 1.0f )
        {
            angle = -m_progress*(Math::PI/2.0f)+Math::PI/2.0f;
            m_object->SetAngleZ(1, angle);
        }
        else
        {
            m_object->SetAngleZ(1, 0.0f);
            m_sound->Play(SOUND_REPAIR, m_object->GetPosition(0));

            m_phase    = ARP_REPAIR;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.0f;
        }
    }

    if ( m_phase == ARP_REPAIR )
    {
        vehicule = SearchVehicle();
        if ( m_progress < 1.0f ||
             (vehicule != 0 && vehicule->GetShield() < 1.0f) )
        {
            if ( vehicule != 0 )
            {
                shield = vehicule->GetShield();
                shield += event.rTime*0.2f;
                if ( shield > 1.0f )  shield = 1.0f;
                vehicule->SetShield(shield);
            }

            if ( m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
            {
                m_lastParticle = m_time;

                pos = m_object->GetPosition(0);
                pos.x += (Math::Rand()-0.5f)*5.0f;
                pos.z += (Math::Rand()-0.5f)*5.0f;
                pos.y += 1.0f;
                speed.x = (Math::Rand()-0.5f)*12.0f;
                speed.z = (Math::Rand()-0.5f)*12.0f;
                speed.y = Math::Rand()*15.0f;
                dim.x = Math::Rand()*6.0f+4.0f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIBLUE, 1.0f, 0.0f, 0.0f);
            }
        }
        else
        {
            m_sound->Play(SOUND_OPEN, m_object->GetPosition(0), 1.0f, 0.8f);

            m_phase    = ARP_UP;
            m_progress = 0.0f;
            m_speed    = 1.0f/3.0f;
        }
    }

    if ( m_phase == ARP_UP )
    {
        if ( m_progress < 1.0f )
        {
            angle = -(1.0f-m_progress)*(Math::PI/2.0f)+Math::PI/2.0f;
            m_object->SetAngleZ(1, angle);
        }
        else
        {
            m_object->SetAngleZ(1, Math::PI/2.0f);

            m_phase    = ARP_WAIT;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.0f;
        }
    }

    return true;
}


// Creates all the interface when the object is selected.

bool CAutoRepair::CreateInterface(bool bSelect)
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
    pw->CreateGroup(pos, ddim, 106, EVENT_OBJECT_TYPE);

    return true;
}


// Seeking the vehicle on the station.

CObject* CAutoRepair::SearchVehicle()
{
    CObject*    pObj;
    CPhysics*   physics;
    Math::Vector    sPos, oPos;
    ObjectType  type;
    float       dist;
    int         i;

    sPos = m_object->GetPosition(0);

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = static_cast< CObject* >(m_iMan->SearchInstance(CLASS_OBJECT, i));
        if ( pObj == 0 )  break;

        type = pObj->GetType();
        if ( type != OBJECT_MOBILEfa &&
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
             type != OBJECT_MOBILEdr )  continue;

        physics = pObj->GetPhysics();
        if ( physics != 0 && !physics->GetLand() )  continue;  // in flight?

        oPos = pObj->GetPosition(0);
        dist = Math::Distance(oPos, sPos);
        if ( dist <= 5.0f )  return pObj;
    }

    return 0;
}


// Returns an error due the state of the automation.

Error CAutoRepair::GetError()
{
    if ( m_object->GetVirusMode() )
    {
        return ERR_BAT_VIRUS;
    }

    return ERR_OK;
}


// Saves all parameters of the controller.

bool CAutoRepair::Write(char *line)
{
    char    name[100];

    if ( m_phase == ARP_WAIT )  return false;

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

bool CAutoRepair::Read(char *line)
{
    if ( OpInt(line, "aExist", 0) == 0 )  return false;

    CAuto::Read(line);

    m_phase = static_cast< AutoRepairPhase >(OpInt(line, "aPhase", ARP_WAIT));
    m_progress = OpFloat(line, "aProgress", 0.0f);
    m_speed = OpFloat(line, "aSpeed", 1.0f);

    m_lastParticle = 0.0f;

    return true;
}


