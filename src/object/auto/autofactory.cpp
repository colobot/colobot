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


#include "object/auto/autofactory.h"

#include "common/global.h"
#include "common/iman.h"

#include "math/geometry.h"

#include "object/robotmain.h"
#include "object/brain.h"

#include "physics/physics.h"

#include "script/cmdtoken.h"

#include "ui/interface.h"
#include "ui/window.h"

#include <stdio.h>
#include <string.h>



// Object's constructor.

CAutoFactory::CAutoFactory(CObject* object) : CAuto(object)
{
    Init();
    m_type  = OBJECT_MOBILEws;
    m_phase = AFP_WAIT;  // paused until the first Init ()
    m_channelSound = -1;
}

// Object's destructor.

CAutoFactory::~CAutoFactory()
{
}


// Destroys the object.

void CAutoFactory::DeleteObject(bool bAll)
{
    CObject*    fret;
    CObject*    vehicle;

    if ( !bAll )
    {
        fret = SearchFret();  // transform metal?
        if ( fret != 0 )
        {
            fret->DeleteObject();  // destroys the metal
            delete fret;
        }

        vehicle = SearchVehicle();
        if ( vehicle != 0 )
        {
            vehicle->DeleteObject();  // destroys the vehicle
            delete vehicle;
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

void CAutoFactory::Init()
{
    m_phase    = AFP_WAIT;
    m_progress = 0.0f;
    m_speed    = 1.0f/2.0f;

    m_time = 0.0f;
    m_lastParticle = 0.0f;

    m_fretPos = m_object->GetPosition(0);

    m_program = nullptr;

    CAuto::Init();
}


// Starts an action

Error CAutoFactory::StartAction(int param)
{
    CObject*   fret;
    ObjectType type = static_cast<ObjectType>(param);

    if ( type != OBJECT_NULL )
    {
        if ( m_phase != AFP_WAIT )
        {
            return ERR_OK;
        }

        m_type = type;

        fret = SearchFret();  // transform metal?
        if ( fret == 0 )
        {
            return ERR_FACTORY_NULL;
        }
        if ( NearestVehicle() )
        {
            return ERR_FACTORY_NEAR;
        }

        SetBusy(true);
        InitProgressTotal(3.0f+2.0f+15.0f+2.0f+3.0f);
        UpdateInterface();

        fret->SetLock(true);  // usable metal
        SoundManip(3.0f, 1.0f, 0.5f);

        m_phase    = AFP_CLOSE_S;
        m_progress = 0.0f;
        m_speed    = 1.0f/3.0f;
        return ERR_OK;
    }
    return ERR_GENERIC;
}


// Sets program for created robot

void CAutoFactory::SetProgram(const char* program)
{
    m_program = new char[strlen(program)+1];
    strcpy(m_program, program);
}


// Management of an event.

bool CAutoFactory::EventProcess(const Event &event)
{
    ObjectType  type;
    CObject*    fret;
    CObject*    vehicle;
    Math::Matrix*   mat;
    CPhysics*   physics;
    Math::Vector    pos, speed;
    Math::Point     dim;
    float       zoom, angle, prog;
    int         i;

    CAuto::EventProcess(event);

    if ( m_engine->GetPause() )  return true;

    if ( m_object->GetSelect() )  // factory selected?
    {
        if ( event.type == EVENT_UPDINTERFACE )
        {
            CreateInterface(true);
        }

        type = OBJECT_NULL;
        if ( event.type == EVENT_OBJECT_FACTORYwa )  type = OBJECT_MOBILEwa;
        if ( event.type == EVENT_OBJECT_FACTORYta )  type = OBJECT_MOBILEta;
        if ( event.type == EVENT_OBJECT_FACTORYfa )  type = OBJECT_MOBILEfa;
        if ( event.type == EVENT_OBJECT_FACTORYia )  type = OBJECT_MOBILEia;
        if ( event.type == EVENT_OBJECT_FACTORYws )  type = OBJECT_MOBILEws;
        if ( event.type == EVENT_OBJECT_FACTORYts )  type = OBJECT_MOBILEts;
        if ( event.type == EVENT_OBJECT_FACTORYfs )  type = OBJECT_MOBILEfs;
        if ( event.type == EVENT_OBJECT_FACTORYis )  type = OBJECT_MOBILEis;
        if ( event.type == EVENT_OBJECT_FACTORYwc )  type = OBJECT_MOBILEwc;
        if ( event.type == EVENT_OBJECT_FACTORYtc )  type = OBJECT_MOBILEtc;
        if ( event.type == EVENT_OBJECT_FACTORYfc )  type = OBJECT_MOBILEfc;
        if ( event.type == EVENT_OBJECT_FACTORYic )  type = OBJECT_MOBILEic;
        if ( event.type == EVENT_OBJECT_FACTORYwi )  type = OBJECT_MOBILEwi;
        if ( event.type == EVENT_OBJECT_FACTORYti )  type = OBJECT_MOBILEti;
        if ( event.type == EVENT_OBJECT_FACTORYfi )  type = OBJECT_MOBILEfi;
        if ( event.type == EVENT_OBJECT_FACTORYii )  type = OBJECT_MOBILEii;
        if ( event.type == EVENT_OBJECT_FACTORYrt )  type = OBJECT_MOBILErt;
        if ( event.type == EVENT_OBJECT_FACTORYrc )  type = OBJECT_MOBILErc;
        if ( event.type == EVENT_OBJECT_FACTORYrr )  type = OBJECT_MOBILErr;
        if ( event.type == EVENT_OBJECT_FACTORYrs )  type = OBJECT_MOBILErs;
        if ( event.type == EVENT_OBJECT_FACTORYsa )  type = OBJECT_MOBILEsa;

        Error err = StartAction(type);
        if( err != ERR_OK && err != ERR_GENERIC )
            m_main->DisplayError(err, m_object);

        if( err != ERR_GENERIC )
            return false;
    }

    if ( event.type != EVENT_FRAME )  return true;

    m_progress += event.rTime*m_speed;
    EventProgress(event.rTime);

    if ( m_phase == AFP_WAIT )
    {
        if ( m_progress >= 1.0f )
        {
            m_phase    = AFP_WAIT;  // still waiting ...
            m_progress = 0.0f;
            m_speed    = 1.0f/2.0f;
        }
    }

    if ( m_phase == AFP_CLOSE_S )
    {
        if ( m_progress < 1.0f )
        {
            for ( i=0 ; i<9 ; i++ )
            {
                zoom = 0.30f+(m_progress-0.5f+i/16.0f)*2.0f*0.70f;
                if ( zoom < 0.30f )  zoom = 0.30f;
                if ( zoom > 1.00f )  zoom = 1.00f;
                m_object->SetZoomZ( 1+i, zoom);
                m_object->SetZoomZ(10+i, zoom);
            }
        }
        else
        {
            for ( i=0 ; i<9 ; i++ )
            {
                m_object->SetZoomZ( 1+i, 1.0f);
                m_object->SetZoomZ(10+i, 1.0f);
            }

            SoundManip(2.0f, 1.0f, 1.2f);

            m_phase    = AFP_CLOSE_T;
            m_progress = 0.0f;
            m_speed    = 1.0f/2.0f;
        }
    }

    if ( m_phase == AFP_CLOSE_T )
    {
        if ( m_progress < 1.0f )
        {
            for ( i=0 ; i<9 ; i++ )
            {
                angle = -m_progress*(Math::PI/2.0f)+Math::PI/2.0f;
                m_object->SetAngleZ( 1+i,  angle);
                m_object->SetAngleZ(10+i, -angle);
            }
        }
        else
        {
            for ( i=0 ; i<9 ; i++ )
            {
                m_object->SetAngleZ( 1+i, 0.0f);
                m_object->SetAngleZ(10+i, 0.0f);
            }

            m_channelSound = m_sound->Play(SOUND_FACTORY, m_object->GetPosition(0), 0.0f, 1.0f, true);
            m_sound->AddEnvelope(m_channelSound, 1.0f, 1.0f,  2.0f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_channelSound, 1.0f, 1.0f, 11.0f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_channelSound, 0.0f, 1.0f,  2.0f, SOPER_STOP);

            m_phase    = AFP_BUILD;
            m_progress = 0.0f;
            m_speed    = 1.0f/15.0f;
        }
    }

    if ( m_phase == AFP_BUILD )
    {
        if ( m_progress == 0.0f )
        {
            if ( !CreateVehicle() )
            {
                fret = SearchFret();  // transform metal?
                if ( fret != 0 )
                {
                    fret->SetLock(false);  // metal usable again
                }

                if ( m_channelSound != -1 )
                {
                    m_sound->FlushEnvelope(m_channelSound);
                    m_sound->AddEnvelope(m_channelSound, 0.0f, 1.0f, 1.0f, SOPER_STOP);
                    m_channelSound = -1;
                }

                m_phase    = AFP_OPEN_T;
                m_progress = 0.0f;
                m_speed    = 1.0f/2.0f;
                return true;
            }
        }

        if ( m_progress < 1.0f )
        {
            if ( m_type == OBJECT_MOBILErt ||
                 m_type == OBJECT_MOBILErc ||
                 m_type == OBJECT_MOBILErr ||
                 m_type == OBJECT_MOBILErs )
            {
                prog = 1.0f-m_progress*1.5f;
                if ( prog < 0.0f )  prog = 0.0f;
            }
            else
            {
                prog = 1.0f-m_progress;
            }
            angle = powf(prog*10.0f, 2.0f)+m_object->GetAngleY(0);

            vehicle = SearchVehicle();
            if ( vehicle != 0 )
            {
                vehicle->SetAngleY(0, angle+Math::PI);
                vehicle->SetZoom(0, m_progress);
            }

            fret = SearchFret();  // transform metal?
            if ( fret != 0 )
            {
                fret->SetZoom(0, 1.0f-m_progress);
            }

            if ( m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
            {
                m_lastParticle = m_time;

#if 0
                pos = m_fretPos;
                pos.x += (Math::Rand()-0.5f)*20.0f;
                pos.z += (Math::Rand()-0.5f)*20.0f;
                pos.y += 1.0f;
                speed.x = (Math::Rand()-0.5f)*12.0f;
                speed.z = (Math::Rand()-0.5f)*12.0f;
                speed.y = Math::Rand()*12.0f;
                dim.x = Math::Rand()*12.0f+10.0f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, PARTIBLUE, 1.0f, 0.0f, 0.0f);
#else
                mat = m_object->GetWorldMatrix(0);
                pos = Math::Vector(-12.0f, 20.0f, -4.0f);  // position of chimney
                pos = Math::Transform(*mat, pos);
                pos.y += 2.0f;
                pos.x += (Math::Rand()-0.5f)*2.0f;
                pos.z += (Math::Rand()-0.5f)*2.0f;
                speed.x = 0.0f;
                speed.z = 0.0f;
                speed.y = 6.0f+Math::Rand()*6.0f;
                dim.x = Math::Rand()*1.5f+1.0f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISMOKE3, 4.0f);
#endif
            }
        }
        else
        {
            m_main->DisplayError(INFO_FACTORY, m_object);
            SoundManip(2.0f, 1.0f, 1.2f);

            fret = SearchFret();  // transform metal?
            if ( fret != 0 )
            {
                fret->DeleteObject();  // removes the metal
                delete fret;
            }

            m_vehicle = vehicle = SearchVehicle();
            if ( vehicle != 0 )
            {
                physics = vehicle->GetPhysics();
                if ( physics != 0 )
                {
                    physics->SetFreeze(false);  // can move
                }

                vehicle->SetLock(false);  // vehicle useable
//?             vehicle->GetPhysics()->GetBrain()->StartTaskAdvance(16.0f);
                vehicle->SetAngleY(0, m_object->GetAngleY(0)+Math::PI);
                vehicle->SetZoom(0, 1.0f);
            }

            m_main->CreateShortcuts();

            m_phase    = AFP_OPEN_T;
            m_progress = 0.0f;
            m_speed    = 1.0f/2.0f;
        }
    }

    if ( m_phase == AFP_OPEN_T )
    {
        if ( m_progress < 1.0f )
        {
            for ( i=0 ; i<9 ; i++ )
            {
                angle = -(1.0f-m_progress)*(Math::PI/2.0f)+Math::PI/2.0f;
                m_object->SetAngleZ( 1+i,  angle);
                m_object->SetAngleZ(10+i, -angle);
            }

            if ( m_lastParticle+m_engine->ParticleAdapt(0.1f) <= m_time )
            {
                m_lastParticle = m_time;

                pos = m_fretPos;
                pos.x += (Math::Rand()-0.5f)*10.0f;
                pos.z += (Math::Rand()-0.5f)*10.0f;
                pos.y += Math::Rand()*10.0f;
                speed = Math::Vector(0.0f, 0.0f, 0.0f);
                dim.x = 2.0f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIGLINT, 2.0f, 0.0f, 0.0f);
            }
        }
        else
        {
            for ( i=0 ; i<9 ; i++ )
            {
                m_object->SetAngleZ( 1+i,  Math::PI/2.0f);
                m_object->SetAngleZ(10+i, -Math::PI/2.0f);
            }

            SoundManip(3.0f, 1.0f, 0.5f);

            m_phase    = AFP_OPEN_S;
            m_progress = 0.0f;
            m_speed    = 1.0f/3.0f;
        }
    }

    if ( m_phase == AFP_OPEN_S )
    {
        if ( m_progress < 1.0f )
        {
            for ( i=0 ; i<9 ; i++ )
            {
                zoom = 0.30f+((1.0f-m_progress)-0.5f+i/16.0f)*2.0f*0.70f;
                if ( zoom < 0.30f )  zoom = 0.30f;
                if ( zoom > 1.00f )  zoom = 1.00f;
                m_object->SetZoomZ( 1+i, zoom);
                m_object->SetZoomZ(10+i, zoom);
            }

            if ( m_lastParticle+m_engine->ParticleAdapt(0.1f) <= m_time )
            {
                m_lastParticle = m_time;

                pos = m_fretPos;
                pos.x += (Math::Rand()-0.5f)*10.0f;
                pos.z += (Math::Rand()-0.5f)*10.0f;
                pos.y += Math::Rand()*10.0f;
                speed = Math::Vector(0.0f, 0.0f, 0.0f);
                dim.x = 2.0f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIGLINT, 2.0f, 0.0f, 0.0f);
            }
        }
        else
        {
            for ( i=0 ; i<9 ; i++ )
            {
                m_object->SetZoomZ( 1+i, 0.30f);
                m_object->SetZoomZ(10+i, 0.30f);
            }

            if ( m_program != nullptr )
            {
                CBrain* brain = m_vehicle->GetBrain();
                if ( brain != nullptr )
                {
                    brain->SendProgram(0, const_cast<const char*>(m_program));
                    brain->SetScriptRun(0);
                    brain->RunProgram(0);
                }
            }

            SetBusy(false);
            UpdateInterface();

            m_phase    = AFP_WAIT;
            m_progress = 0.0f;
            m_speed    = 1.0f/2.0f;
        }
    }

    return true;
}


// Saves all parameters of the controller.

bool CAutoFactory::Write(char *line)
{
    char    name[100];

    if ( m_phase == AFP_WAIT )  return false;

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

// Restores all parameters of the controller

bool CAutoFactory::Read(char *line)
{
    if ( OpInt(line, "aExist", 0) == 0 )  return false;

    CAuto::Read(line);

    m_phase = static_cast< AutoFactoryPhase >(OpInt(line, "aPhase", AFP_WAIT));
    m_progress = OpFloat(line, "aProgress", 0.0f);
    m_speed = OpFloat(line, "aSpeed", 1.0f);

    m_lastParticle = 0.0f;
    m_fretPos = m_object->GetPosition(0);

    return true;
}


//Seeks the cargo.

CObject* CAutoFactory::SearchFret()
{
    CObject*    pObj;
    Math::Vector    oPos;
    ObjectType  type;
    float       dist;
    int         i;

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = static_cast< CObject* >(m_iMan->SearchInstance(CLASS_OBJECT, i));
        if ( pObj == 0 )  break;

        type = pObj->GetType();
        if ( type != OBJECT_METAL )  continue;
        if ( pObj->GetTruck() != 0 )  continue;

        oPos = pObj->GetPosition(0);
        dist = Math::Distance(oPos, m_fretPos);

        if ( dist < 8.0f )  return pObj;
    }

    return 0;
}

// Search if a vehicle is too close.

bool CAutoFactory::NearestVehicle()
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
        dist = Math::Distance(oPos, cPos)-oRadius;

        if ( dist < 10.0f )  return true;
    }

    return false;
}


// Creates a vehicle.

bool CAutoFactory::CreateVehicle()
{
    CObject*    vehicle;
    Math::Matrix*   mat;
    CPhysics*   physics;
    Math::Vector    pos;
    float       angle;
    char*       name;
    int         i;

    angle = m_object->GetAngleY(0);

    mat = m_object->GetWorldMatrix(0);
    if ( m_type == OBJECT_MOBILErt ||
         m_type == OBJECT_MOBILErc ||
         m_type == OBJECT_MOBILErr ||
         m_type == OBJECT_MOBILErs )
    {
        pos = Math::Vector(2.0f, 0.0f, 0.0f);
    }
    else
    {
        pos = Math::Vector(4.0f, 0.0f, 0.0f);
    }
    pos = Transform(*mat, pos);

    vehicle = new CObject();
    if ( !vehicle->CreateVehicle(pos, angle, m_type, -1.0f, false, false) )
    {
        delete vehicle;
        m_main->DisplayError(ERR_TOOMANY, m_object);
        return false;
    }
    vehicle->UpdateMapping();
    vehicle->SetLock(true);  // not usable
    vehicle->SetRange(30.0f);

    physics = vehicle->GetPhysics();
    if ( physics != 0 )
    {
        physics->SetFreeze(true);  // it doesn't move
    }

    for ( i=0 ; i<10 ; i++ )
    {
        name = m_main->GetNewScriptName(m_type, i);
        if ( name == 0 )  break;
        vehicle->ReadProgram(i, name);
    }

    return true;
}

// Seeking the vehicle during manufacture.

CObject* CAutoFactory::SearchVehicle()
{
    CObject*    pObj;
    Math::Vector    oPos;
    ObjectType  type;
    float       dist;
    int         i;

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = static_cast< CObject* >(m_iMan->SearchInstance(CLASS_OBJECT, i));
        if ( pObj == 0 )  break;

        if ( !pObj->GetLock() )  continue;

        type = pObj->GetType();
        if ( type != m_type )  continue;
        if ( pObj->GetTruck() != 0 )  continue;

        oPos = pObj->GetPosition(0);
        dist = Math::Distance(oPos, m_fretPos);

        if ( dist < 8.0f )  return pObj;
    }

    return 0;
}


// Creates all the interface when the object is selected.

bool CAutoFactory::CreateInterface(bool bSelect)
{
    Ui::CWindow*    pw;
    Math::Point     pos, dim, ddim;
    float       ox, oy, sx, sy;

    CAuto::CreateInterface(bSelect);

    if ( !bSelect )  return true;

    pw = static_cast< Ui::CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw == nullptr )  return false;

    dim.x = 33.0f/640.0f;
    dim.y = 33.0f/480.0f;
    ox = 3.0f/640.0f;
    oy = 3.0f/480.0f;
    sx = 33.0f/640.0f;
    sy = 33.0f/480.0f;

    pos.x = 0.0f;
    pos.y = oy+sy*2.6f;
    ddim.x = 138.0f/640.0f;
    ddim.y = 222.0f/480.0f;
    pw->CreateGroup(pos, ddim, 6, EVENT_WINDOW3);

    pos.x = ox+sx*0.0f;
    pos.y = oy+sy*8.2f;
    pw->CreateButton(pos, dim, 128+9, EVENT_OBJECT_FACTORYwa);
    pos.x += dim.x;
    pw->CreateButton(pos, dim, 128+10, EVENT_OBJECT_FACTORYta);
    pos.x += dim.x;
    pw->CreateButton(pos, dim, 128+11, EVENT_OBJECT_FACTORYfa);
    pos.x += dim.x;
    pw->CreateButton(pos, dim, 128+22, EVENT_OBJECT_FACTORYia);

    pos.x = ox+sx*0.0f;
    pos.y = oy+sy*7.1f;
    pw->CreateButton(pos, dim, 128+12, EVENT_OBJECT_FACTORYws);
    pos.x += dim.x;
    pw->CreateButton(pos, dim, 128+13, EVENT_OBJECT_FACTORYts);
    pos.x += dim.x;
    pw->CreateButton(pos, dim, 128+14, EVENT_OBJECT_FACTORYfs);
    pos.x += dim.x;
    pw->CreateButton(pos, dim, 128+24, EVENT_OBJECT_FACTORYis);

    pos.x = ox+sx*0.0f;
    pos.y = oy+sy*6.0f;
    pw->CreateButton(pos, dim, 128+15, EVENT_OBJECT_FACTORYwc);
    pos.x += dim.x;
    pw->CreateButton(pos, dim, 128+16, EVENT_OBJECT_FACTORYtc);
    pos.x += dim.x;
    pw->CreateButton(pos, dim, 128+17, EVENT_OBJECT_FACTORYfc);
    pos.x += dim.x;
    pw->CreateButton(pos, dim, 128+23, EVENT_OBJECT_FACTORYic);

    pos.x = ox+sx*0.0f;
    pos.y = oy+sy*4.9f;
    pw->CreateButton(pos, dim, 128+25, EVENT_OBJECT_FACTORYwi);
    pos.x += dim.x;
    pw->CreateButton(pos, dim, 128+26, EVENT_OBJECT_FACTORYti);
    pos.x += dim.x;
    pw->CreateButton(pos, dim, 128+27, EVENT_OBJECT_FACTORYfi);
    pos.x += dim.x;
    pw->CreateButton(pos, dim, 128+28, EVENT_OBJECT_FACTORYii);

    pos.x = ox+sx*0.0f;
    pos.y = oy+sy*3.8f;
    pw->CreateButton(pos, dim, 128+18, EVENT_OBJECT_FACTORYrt);
    pos.x += dim.x;
    pw->CreateButton(pos, dim, 128+19, EVENT_OBJECT_FACTORYrc);
    pos.x += dim.x;
    pw->CreateButton(pos, dim, 128+20, EVENT_OBJECT_FACTORYrr);
    pos.x += dim.x;
    pw->CreateButton(pos, dim, 128+29, EVENT_OBJECT_FACTORYrs);

    pos.x = ox+sx*0.0f;
    pos.y = oy+sy*2.7f;
    pw->CreateButton(pos, dim, 128+21, EVENT_OBJECT_FACTORYsa);

    pos.x = ox+sx*0.0f;
    pos.y = oy+sy*0;
    ddim.x = 66.0f/640.0f;
    ddim.y = 66.0f/480.0f;
    pw->CreateGroup(pos, ddim, 101, EVENT_OBJECT_TYPE);

    UpdateInterface();
    return true;
}

// Updates the status of all interface buttons.

void CAutoFactory::UpdateInterface()
{
    Ui::CWindow*    pw;

    if ( !m_object->GetSelect() )  return;

    CAuto::UpdateInterface();

    pw = static_cast< Ui::CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));

    UpdateButton(pw, EVENT_OBJECT_FACTORYwa, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYta, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYfa, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYia, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYws, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYts, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYfs, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYis, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYwc, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYtc, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYfc, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYic, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYwi, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYti, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYfi, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYii, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYrt, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYrc, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYrr, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYrs, m_bBusy);
    UpdateButton(pw, EVENT_OBJECT_FACTORYsa, m_bBusy);
}

// Updates the status of one interface button.

void CAutoFactory::UpdateButton(Ui::CWindow *pw, EventType event, bool bBusy)
{
    bool        bEnable = true;

    EnableInterface(pw, event, !bBusy);

    if ( event == EVENT_OBJECT_FACTORYta )
    {
        bEnable = g_researchDone&RESEARCH_TANK;
    }
    if ( event == EVENT_OBJECT_FACTORYfa )
    {
        bEnable = g_researchDone&RESEARCH_FLY;
    }
    if ( event == EVENT_OBJECT_FACTORYia )
    {
        bEnable = g_researchDone&RESEARCH_iPAW;
    }

    if ( event == EVENT_OBJECT_FACTORYws )
    {
        bEnable = g_researchDone&RESEARCH_SNIFFER;
    }
    if ( event == EVENT_OBJECT_FACTORYts )
    {
        bEnable = ( (g_researchDone&RESEARCH_SNIFFER) &&
                    (g_researchDone&RESEARCH_TANK)    );
    }
    if ( event == EVENT_OBJECT_FACTORYfs )
    {
        bEnable = ( (g_researchDone&RESEARCH_SNIFFER) &&
                    (g_researchDone&RESEARCH_FLY)     );
    }
    if ( event == EVENT_OBJECT_FACTORYis )
    {
        bEnable = ( (g_researchDone&RESEARCH_SNIFFER) &&
                    (g_researchDone&RESEARCH_iPAW)    );
    }

    if ( event == EVENT_OBJECT_FACTORYwc )
    {
        bEnable = g_researchDone&RESEARCH_CANON;
    }
    if ( event == EVENT_OBJECT_FACTORYtc )
    {
        bEnable = ( (g_researchDone&RESEARCH_CANON) &&
                    (g_researchDone&RESEARCH_TANK)  );
    }
    if ( event == EVENT_OBJECT_FACTORYfc )
    {
        bEnable = ( (g_researchDone&RESEARCH_CANON) &&
                    (g_researchDone&RESEARCH_FLY)   );
    }
    if ( event == EVENT_OBJECT_FACTORYic )
    {
        bEnable = ( (g_researchDone&RESEARCH_CANON) &&
                    (g_researchDone&RESEARCH_iPAW)  );
    }

    if ( event == EVENT_OBJECT_FACTORYwi )
    {
        bEnable = g_researchDone&RESEARCH_iGUN;
    }
    if ( event == EVENT_OBJECT_FACTORYti )
    {
        bEnable = ( (g_researchDone&RESEARCH_iGUN) &&
                    (g_researchDone&RESEARCH_TANK) );
    }
    if ( event == EVENT_OBJECT_FACTORYfi )
    {
        bEnable = ( (g_researchDone&RESEARCH_iGUN) &&
                    (g_researchDone&RESEARCH_FLY)  );
    }
    if ( event == EVENT_OBJECT_FACTORYii )
    {
        bEnable = ( (g_researchDone&RESEARCH_iGUN) &&
                    (g_researchDone&RESEARCH_iPAW) );
    }

    if ( event == EVENT_OBJECT_FACTORYrt )
    {
        bEnable = ( (g_researchDone&RESEARCH_THUMP) &&
                    (g_researchDone&RESEARCH_TANK)  );
    }
    if ( event == EVENT_OBJECT_FACTORYrc )
    {
        bEnable = ( (g_researchDone&RESEARCH_PHAZER) &&
                    (g_researchDone&RESEARCH_TANK)   );
    }
    if ( event == EVENT_OBJECT_FACTORYrr )
    {
        bEnable = ( (g_researchDone&RESEARCH_RECYCLER) &&
                    (g_researchDone&RESEARCH_TANK)     );
    }
    if ( event == EVENT_OBJECT_FACTORYrs )
    {
        bEnable = ( (g_researchDone&RESEARCH_SHIELD) &&
                    (g_researchDone&RESEARCH_TANK)   );
    }

    if ( event == EVENT_OBJECT_FACTORYsa )
    {
        bEnable = g_researchDone&RESEARCH_SUBM;
    }

    DeadInterface(pw, event, bEnable);
}

// Plays the sound of the manipulator arm.

void CAutoFactory::SoundManip(float time, float amplitude, float frequency)
{
    int     i;

    i = m_sound->Play(SOUND_MANIP, m_object->GetPosition(0), 0.0f, 0.3f*frequency, true);
    m_sound->AddEnvelope(i, 0.5f*amplitude, 1.0f*frequency, 0.1f, SOPER_CONTINUE);
    m_sound->AddEnvelope(i, 0.5f*amplitude, 1.0f*frequency, time-0.1f, SOPER_CONTINUE);
    m_sound->AddEnvelope(i, 0.0f, 0.3f*frequency, 0.1f, SOPER_STOP);
}

