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

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "d3dengine.h"
#include "d3dmath.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "math3d.h"
#include "particule.h"
#include "light.h"
#include "terrain.h"
#include "camera.h"
#include "object.h"
#include "physics.h"
#include "sound.h"
#include "interface.h"
#include "button.h"
#include "window.h"
#include "robotmain.h"
#include "cmdtoken.h"
#include "auto.h"
#include "autorepair.h"




// Object's constructor.

CAutoRepair::CAutoRepair(CInstanceManager* iMan, CObject* object)
                         : CAuto(iMan, object)
{
    Init();
    m_phase = ARP_WAIT;  // paused until the first Init ()
}

// Object's destructor.

CAutoRepair::~CAutoRepair()
{
    this->CAuto::~CAuto();
}


// Destroys the object.

void CAutoRepair::DeleteObject(BOOL bAll)
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
    m_lastParticule = 0.0f;

    CAuto::Init();
}


// Management of an event.

BOOL CAutoRepair::EventProcess(const Event &event)
{
    CObject*    vehicule;
    D3DVECTOR   pos, speed;
    FPOINT      dim;
    float       angle, shield;

    CAuto::EventProcess(event);

    if ( m_engine->RetPause() )  return TRUE;
    if ( event.event != EVENT_FRAME )  return TRUE;

    m_progress += event.rTime*m_speed;
    m_timeVirus -= event.rTime;

    if ( m_object->RetVirusMode() )  // contaminated by a virus?
    {
        if ( m_timeVirus <= 0.0f )
        {
            m_timeVirus = 0.1f+Rand()*0.3f;
        }
        return TRUE;
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
                m_sound->Play(SOUND_OPEN, m_object->RetPosition(0), 1.0f, 0.8f);

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
            angle = -m_progress*(PI/2.0f)+PI/2.0f;
            m_object->SetAngleZ(1, angle);
        }
        else
        {
            m_object->SetAngleZ(1, 0.0f);
            m_sound->Play(SOUND_REPAIR, m_object->RetPosition(0));

            m_phase    = ARP_REPAIR;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.0f;
        }
    }

    if ( m_phase == ARP_REPAIR )
    {
        vehicule = SearchVehicle();
        if ( m_progress < 1.0f ||
             (vehicule != 0 && vehicule->RetShield() < 1.0f) )
        {
            if ( vehicule != 0 )
            {
                shield = vehicule->RetShield();
                shield += event.rTime*0.2f;
                if ( shield > 1.0f )  shield = 1.0f;
                vehicule->SetShield(shield);
            }

            if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
            {
                m_lastParticule = m_time;

                pos = m_object->RetPosition(0);
                pos.x += (Rand()-0.5f)*5.0f;
                pos.z += (Rand()-0.5f)*5.0f;
                pos.y += 1.0f;
                speed.x = (Rand()-0.5f)*12.0f;
                speed.z = (Rand()-0.5f)*12.0f;
                speed.y = Rand()*15.0f;
                dim.x = Rand()*6.0f+4.0f;
                dim.y = dim.x;
                m_particule->CreateParticule(pos, speed, dim, PARTIBLUE, 1.0f, 0.0f, 0.0f);
            }
        }
        else
        {
            m_sound->Play(SOUND_OPEN, m_object->RetPosition(0), 1.0f, 0.8f);

            m_phase    = ARP_UP;
            m_progress = 0.0f;
            m_speed    = 1.0f/3.0f;
        }
    }

    if ( m_phase == ARP_UP )
    {
        if ( m_progress < 1.0f )
        {
            angle = -(1.0f-m_progress)*(PI/2.0f)+PI/2.0f;
            m_object->SetAngleZ(1, angle);
        }
        else
        {
            m_object->SetAngleZ(1, PI/2.0f);

            m_phase    = ARP_WAIT;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.0f;
        }
    }

    return TRUE;
}


// Creates all the interface when the object is selected.

BOOL CAutoRepair::CreateInterface(BOOL bSelect)
{
    CWindow*    pw;
    FPOINT      pos, ddim;
    float       ox, oy, sx, sy;

    CAuto::CreateInterface(bSelect);

    if ( !bSelect )  return TRUE;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW0);
    if ( pw == 0 )  return FALSE;

    ox = 3.0f/640.0f;
    oy = 3.0f/480.0f;
    sx = 33.0f/640.0f;
    sy = 33.0f/480.0f;

    pos.x = ox+sx*0.0f;
    pos.y = oy+sy*0;
    ddim.x = 66.0f/640.0f;
    ddim.y = 66.0f/480.0f;
    pw->CreateGroup(pos, ddim, 106, EVENT_OBJECT_TYPE);

    return TRUE;
}


// Seeking the vehicle on the station.

CObject* CAutoRepair::SearchVehicle()
{
    CObject*    pObj;
    CPhysics*   physics;
    D3DVECTOR   sPos, oPos;
    ObjectType  type;
    float       dist;
    int         i;

    sPos = m_object->RetPosition(0);

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        type = pObj->RetType();
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

        physics = pObj->RetPhysics();
        if ( physics != 0 && !physics->RetLand() )  continue;  // in flight?

        oPos = pObj->RetPosition(0);
        dist = Length(oPos, sPos);
        if ( dist <= 5.0f )  return pObj;
    }

    return 0;
}


// Returns an error due the state of the automation.

Error CAutoRepair::RetError()
{
    if ( m_object->RetVirusMode() )
    {
        return ERR_BAT_VIRUS;
    }

    return ERR_OK;
}


// Saves all parameters of the controller.

BOOL CAutoRepair::Write(char *line)
{
    char    name[100];

    if ( m_phase == ARP_WAIT )  return FALSE;

    sprintf(name, " aExist=%d", 1);
    strcat(line, name);

    CAuto::Write(line);

    sprintf(name, " aPhase=%d", m_phase);
    strcat(line, name);

    sprintf(name, " aProgress=%.2f", m_progress);
    strcat(line, name);

    sprintf(name, " aSpeed=%.2f", m_speed);
    strcat(line, name);

    return TRUE;
}

// Restores all parameters of the controller.

BOOL CAutoRepair::Read(char *line)
{
    if ( OpInt(line, "aExist", 0) == 0 )  return FALSE;

    CAuto::Read(line);

    m_phase = (AutoRepairPhase)OpInt(line, "aPhase", ARP_WAIT);
    m_progress = OpFloat(line, "aProgress", 0.0f);
    m_speed = OpFloat(line, "aSpeed", 1.0f);

    m_lastParticule = 0.0f;

    return TRUE;
}


