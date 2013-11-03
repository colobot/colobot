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


#include "object/auto/autonuclear.h"

#include "common/iman.h"

#include "math/geometry.h"

#include "script/cmdtoken.h"

#include "ui/interface.h"
#include "ui/window.h"

#include <stdio.h>
#include <string.h>


const float NUCLEAR_DELAY = 30.0f;  // duration of the generation




// Object's constructor.

CAutoNuclear::CAutoNuclear(CObject* object) : CAuto(object)
{
    m_channelSound = -1;
    Init();
}

// Object's destructor.

CAutoNuclear::~CAutoNuclear()
{
}


// Destroys the object.

void CAutoNuclear::DeleteObject(bool bAll)
{
    CObject*    fret;

    if ( !bAll )
    {
        fret = SearchUranium();
        if ( fret != 0 )
        {
            fret->DeleteObject();  // destroys the metal
            delete fret;
        }
    }

    if ( m_channelSound != -1 )
    {
        m_sound->FlushEnvelope(m_channelSound);
        m_sound->AddEnvelope(m_channelSound, 0.0f, 1.0f, 1.0f, SOPER_STOP);
        m_channelSound = -1;
    }

    CAuto::DeleteObject(bAll);
}


// Initialize the object.

void CAutoNuclear::Init()
{
    Math::Matrix*   mat;

    m_time = 0.0f;
    m_timeVirus = 0.0f;
    m_lastParticle = 0.0f;

    mat = m_object->GetWorldMatrix(0);
    m_pos = Math::Transform(*mat, Math::Vector(22.0f, 4.0f, 0.0f));

    m_phase    = ANUP_WAIT;  // waiting ...
    m_progress = 0.0f;
    m_speed    = 1.0f/2.0f;

    CAuto::Init();
}


// Management of an event.

bool CAutoNuclear::EventProcess(const Event &event)
{
    CObject*    fret;
    Math::Matrix*   mat;
    Math::Vector    pos, goal, speed;
    Math::Point     dim, rot;
    float       angle;
    int         i, max;

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

    if ( m_phase == ANUP_WAIT )
    {
        if ( m_progress >= 1.0f )
        {
            fret = SearchUranium();  // transform uranium?
            if ( fret == 0 || SearchVehicle() )
            {
                m_phase    = ANUP_WAIT;  // still waiting ...
                m_progress = 0.0f;
                m_speed    = 1.0f/2.0f;
            }
            else
            {
                fret->SetLock(true);  // usable uranium

                SetBusy(true);
                InitProgressTotal(1.5f+NUCLEAR_DELAY+1.5f);
                UpdateInterface();

                m_sound->Play(SOUND_OPEN, m_object->GetPosition(0), 1.0f, 1.4f);

                m_phase    = ANUP_CLOSE;
                m_progress = 0.0f;
                m_speed    = 1.0f/1.5f;
            }
        }
    }

    if ( m_phase == ANUP_CLOSE )
    {
        if ( m_progress < 1.0f )
        {
            angle = (1.0f-m_progress)*(135.0f*Math::PI/180.0f);
            m_object->SetAngleZ(1, angle);
        }
        else
        {
            m_object->SetAngleZ(1, 0.0f);

            mat = m_object->GetWorldMatrix(0);
            max = static_cast< int >(10.0f*m_engine->GetParticleDensity());
            for ( i=0 ; i<max ; i++ )
            {
                pos.x = 27.0f;
                pos.y =  0.0f;
                pos.z = (Math::Rand()-0.5f)*8.0f;
                pos = Transform(*mat, pos);
                speed.y = 0.0f;
                speed.x = 0.0f;
                speed.z = 0.0f;
                dim.x = Math::Rand()*1.0f+1.0f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTICRASH);
            }

            m_sound->Play(SOUND_CLOSE, m_object->GetPosition(0), 1.0f, 1.0f);

            m_channelSound = m_sound->Play(SOUND_NUCLEAR, m_object->GetPosition(0), 1.0f, 0.1f, true);
            m_sound->AddEnvelope(m_channelSound, 1.0f, 1.0f, NUCLEAR_DELAY-1.0f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_channelSound, 0.0f, 1.0f, 2.0f, SOPER_STOP);

            m_phase    = ANUP_GENERATE;
            m_progress = 0.0f;
            m_speed    = 1.0f/NUCLEAR_DELAY;
        }
    }

    if ( m_phase == ANUP_GENERATE )
    {
        if ( m_progress < 1.0f )
        {
            if ( m_lastParticle+m_engine->ParticleAdapt(0.10f) <= m_time )
            {
                m_lastParticle = m_time;

                pos = m_object->GetPosition(0);
                pos.y += 30.0f;
                pos.x += (Math::Rand()-0.5f)*6.0f;
                pos.z += (Math::Rand()-0.5f)*6.0f;
                speed.y = Math::Rand()*15.0f+15.0f;
                speed.x = 0.0f;
                speed.z = 0.0f;
                dim.x = Math::Rand()*8.0f+8.0f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTICRASH);

                pos = m_pos;
                speed.x = (Math::Rand()-0.5f)*20.0f;
                speed.y = (Math::Rand()-0.5f)*20.0f;
                speed.z = (Math::Rand()-0.5f)*20.0f;
                dim.x = 2.0f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIBLITZ, 1.0f, 0.0f, 0.0f);
            }
        }
        else
        {
            fret = SearchUranium();
            if ( fret != 0 )
            {
                fret->DeleteObject();  // destroyed uranium
                delete fret;
                m_object->SetPower(0);
            }

            CreatePower();  // creates the atomic cell

            max = static_cast< int >(20.0f*m_engine->GetParticleDensity());
            for ( i=0 ; i<max ; i++ )
            {
                pos = m_pos;
                pos.x += (Math::Rand()-0.5f)*3.0f;
                pos.y += (Math::Rand()-0.5f)*3.0f;
                pos.z += (Math::Rand()-0.5f)*3.0f;
                speed.y = 0.0f;
                speed.x = 0.0f;
                speed.z = 0.0f;
                dim.x = Math::Rand()*2.0f+2.0f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIBLUE, Math::Rand()*5.0f+5.0f, 0.0f, 0.0f);
            }

            m_sound->Play(SOUND_OPEN, m_object->GetPosition(0), 1.0f, 1.4f);

            m_phase    = ANUP_OPEN;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.5f;
        }
    }

    if ( m_phase == ANUP_OPEN )
    {
        if ( m_progress < 1.0f )
        {
            angle = m_progress*(135.0f*Math::PI/180.0f);
            m_object->SetAngleZ(1, angle);
        }
        else
        {
            m_object->SetAngleZ(1, 135.0f*Math::PI/180.0f);

            SetBusy(false);
            UpdateInterface();

            m_main->DisplayError(INFO_NUCLEAR, m_object);

            m_phase    = ANUP_WAIT;
            m_progress = 0.0f;
            m_speed    = 1.0f/2.0f;
        }
    }

    return true;
}


// Creates all the interface when the object is selected.

bool CAutoNuclear::CreateInterface(bool bSelect)
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
    pw->CreateGroup(pos, ddim, 110, EVENT_OBJECT_TYPE);

    return true;
}


// Seeking the uranium.

CObject* CAutoNuclear::SearchUranium()
{
    CObject*    pObj;

    pObj = m_object->GetPower();
    if ( pObj == 0 )  return 0;
    if ( pObj->GetType() == OBJECT_URANIUM )  return pObj;
    return 0;
}

// Seeks if a vehicle is too close.

bool CAutoNuclear::SearchVehicle()
{
    CObject*    pObj;
    Math::Vector    oPos;
    ObjectType  type;
    float       oRadius, dist;
    int         i;

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = static_cast< CObject* >(m_iMan->SearchInstance(CLASS_OBJECT, i));
        if ( pObj == 0 )  break;

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
        dist = Math::Distance(oPos, m_pos)-oRadius;

        if ( dist < 10.0f )  return true;
    }

    return false;
}

// Creates an object stack.

void CAutoNuclear::CreatePower()
{
    CObject*        power;
    Math::Vector        pos;
    float           angle;

    pos = m_object->GetPosition(0);
    angle = m_object->GetAngleY(0);

    power = new CObject();
    if ( !power->CreateResource(pos, angle, OBJECT_ATOMIC) )
    {
        delete power;
        m_main->DisplayError(ERR_TOOMANY, m_object);
        return;
    }

    power->SetTruck(m_object);
    power->SetPosition(0, Math::Vector(22.0f, 3.0f, 0.0f));
    m_object->SetPower(power);
}


// Returns an error due the state of the automation.

Error CAutoNuclear::GetError()
{
    CObject*    pObj;
    ObjectType  type;
//? TerrainRes  res;

    if ( m_object->GetVirusMode() )
    {
        return ERR_BAT_VIRUS;
    }

//? res = m_terrain->GetResource(m_object->GetPosition(0));
//? if ( res != TR_POWER )  return ERR_NUCLEAR_NULL;

//? if ( m_object->GetEnergy() < ENERGY_POWER )  return ERR_NUCLEAR_LOW;

    pObj = m_object->GetPower();
    if ( pObj == 0 )  return ERR_NUCLEAR_EMPTY;
    if ( pObj->GetLock() )  return ERR_OK;
    type = pObj->GetType();
    if ( type == OBJECT_ATOMIC  )  return ERR_OK;
    if ( type != OBJECT_URANIUM )  return ERR_NUCLEAR_BAD;

    return ERR_OK;
}


// Saves all parameters of the controller.

bool CAutoNuclear::Write(char *line)
{
    char    name[100];

    if ( m_phase == ANUP_STOP ||
         m_phase == ANUP_WAIT )  return false;

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

bool CAutoNuclear::Read(char *line)
{
    if ( OpInt(line, "aExist", 0) == 0 )  return false;

    CAuto::Read(line);

    m_phase = static_cast< AutoNuclearPhase >(OpInt(line, "aPhase", ANUP_WAIT));
    m_progress = OpFloat(line, "aProgress", 0.0f);
    m_speed = OpFloat(line, "aSpeed", 1.0f);

    m_lastParticle = 0.0f;

    return true;
}

