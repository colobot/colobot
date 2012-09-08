// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// * Copyright (C) 2012 Polish Portal of Colobot (PPC)
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

#include "object/auto/autodestroyer.h"

#include "common/iman.h"
#include "script/cmdtoken.h"
#include "ui/interface.h"
#include "ui/window.h"




// Object's constructor.

CAutoDestroyer::CAutoDestroyer(CInstanceManager* iMan, CObject* object)
                         : CAuto(iMan, object)
{
    Init();
    m_phase = ADEP_WAIT;  // paused until the first Init ()
}

// Destructive of the object.

CAutoDestroyer::~CAutoDestroyer()
{
}


// Destroys the object.

void CAutoDestroyer::DeleteObject(bool bAll)
{
    CAuto::DeleteObject(bAll);
}


// Initialize the object.

void CAutoDestroyer::Init()
{
    m_phase    = ADEP_WAIT;
    m_progress = 0.0f;
    m_speed    = 1.0f/0.5f;

    m_time     = 0.0f;
    m_timeVirus = 0.0f;
    m_lastParticle = 0.0f;

    CAuto::Init();
}


// Management of an event.

bool CAutoDestroyer::EventProcess(const Event &event)
{
    CObject*    scrap;
    Gfx::CPyro*      pyro;
    Math::Vector    pos, speed;
    Math::Point     dim;

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

    if ( m_phase == ADEP_WAIT )
    {
        if ( m_progress >= 1.0f )
        {
            scrap = SearchPlastic();
            if ( scrap == 0 )
            {
                m_phase    = ADEP_WAIT;  // still waiting ...
                m_progress = 0.0f;
                m_speed    = 1.0f/0.5f;
            }
            else
            {
                scrap->SetLock(true);  // usable waste
//?             scrap->SetTruck(m_object);  // usable waste

                if ( SearchVehicle() )
                {
                    m_phase    = ADEP_WAIT;  // still waiting ...
                    m_progress = 0.0f;
                    m_speed    = 1.0f/0.5f;
                }
                else
                {
                    m_sound->Play(SOUND_PSHHH2, m_object->GetPosition(0), 1.0f, 1.0f);

                    m_phase    = ADEP_DOWN;
                    m_progress = 0.0f;
                    m_speed    = 1.0f/1.0f;
                    m_bExplo   = false;
                }
            }
        }
    }

    if ( m_phase == ADEP_DOWN )
    {
        if ( m_progress >= 0.3f-0.05f && !m_bExplo )
        {
            scrap = SearchPlastic();
            if ( scrap != nullptr )
            {
                pyro = new Gfx::CPyro(m_iMan);
                pyro->Create(Gfx::PT_FRAGT, scrap);
            }
            m_bExplo = true;
        }

        if ( m_progress < 1.0f )
        {
            pos = Math::Vector(0.0f, -10.0f, 0.0f);
            pos.y = -Math::Bounce(m_progress, 0.3f)*10.0f;
            m_object->SetPosition(1, pos);
        }
        else
        {
            m_object->SetPosition(1, Math::Vector(0.0f, -10.0f, 0.0f));
            m_sound->Play(SOUND_REPAIR, m_object->GetPosition(0));

            m_phase    = ADEP_REPAIR;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.0f;
        }
    }

    if ( m_phase == ADEP_REPAIR )
    {
        if ( m_progress < 1.0f )
        {
        }
        else
        {
            m_sound->Play(SOUND_OPEN, m_object->GetPosition(0), 1.0f, 0.8f);

            m_phase    = ADEP_UP;
            m_progress = 0.0f;
            m_speed    = 1.0f/3.0f;
        }
    }

    if ( m_phase == ADEP_UP )
    {
        if ( m_progress < 1.0f )
        {
            pos = Math::Vector(0.0f, -10.0f, 0.0f);
            pos.y = -(1.0f-m_progress)*10.0f;
            m_object->SetPosition(1, pos);
        }
        else
        {
            m_object->SetPosition(1, Math::Vector(0.0f, 0.0f, 0.0f));

            m_phase    = ADEP_WAIT;
            m_progress = 0.0f;
            m_speed    = 1.0f/0.5f;
        }
    }

    return true;
}


// Creates all the interface when the object is selected.

bool CAutoDestroyer::CreateInterface(bool bSelect)
{
    Ui::CWindow*    pw;
    Math::Point     pos, ddim;
    float       ox, oy, sx, sy;

    CAuto::CreateInterface(bSelect);

    if ( !bSelect )  return true;

    pw = static_cast< Ui::CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw == nullptr )  return false;

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


// Seeks plate waste in the destroyer.

CObject* CAutoDestroyer::SearchPlastic()
{
    CObject*    pObj;
    Math::Vector    sPos, oPos;
    ObjectType  type;
    float       dist;
    int         i;

    sPos = m_object->GetPosition(0);

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = static_cast< CObject* >(m_iMan->SearchInstance(CLASS_OBJECT, i));
        if ( pObj == nullptr )  break;

        type = pObj->GetType();
        if ( type != OBJECT_SCRAP4 &&
             type != OBJECT_SCRAP5 )  continue;

        oPos = pObj->GetPosition(0);
        dist = Math::Distance(oPos, sPos);
        if ( dist <= 5.0f )  return pObj;
    }

    return nullptr;
}

// Seeks if one vehicle is too close.

bool CAutoDestroyer::SearchVehicle()
{
    CObject*    pObj;
    Math::Vector    cPos, oPos;
    ObjectType  type;
    float       oRadius, dist;
    int         i;

    cPos = m_object->GetPosition(0);

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = static_cast< CObject* >(m_iMan->SearchInstance(CLASS_OBJECT, i));
        if ( pObj == nullptr )  break;

        type = pObj->GetType();
        if ( type != OBJECT_HUMAN    &&
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
        dist = Math::Distance(oPos, cPos)-oRadius;

        if ( dist < 20.0f )  return true;
    }

    return false;
}


// Geturns an error due the state of the automation.

Error CAutoDestroyer::GetError()
{
    if ( m_object->GetVirusMode() )
    {
        return ERR_BAT_VIRUS;
    }

    return ERR_OK;
}


// Saves all parameters of the controller.

bool CAutoDestroyer::Write(char *line)
{
    char    name[100];

    if ( m_phase == ADEP_WAIT )  return false;

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

bool CAutoDestroyer::Read(char *line)
{
    if ( OpInt(line, "aExist", 0) == 0 )  return false;

    CAuto::Read(line);

    m_phase = static_cast< AutoDestroyerPhase >(OpInt(line, "aPhase", ADEP_WAIT));
    m_progress = OpFloat(line, "aProgress", 0.0f);
    m_speed = OpFloat(line, "aSpeed", 1.0f);

    m_lastParticle = 0.0f;

    return true;
}


