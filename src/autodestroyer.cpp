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
#include "pyro.h"
#include "sound.h"
#include "interface.h"
#include "button.h"
#include "window.h"
#include "robotmain.h"
#include "cmdtoken.h"
#include "auto.h"
#include "autodestroyer.h"




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
    this->CAuto::~CAuto();
}


// Destroys the object.

void CAutoDestroyer::DeleteObject(BOOL bAll)
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
    m_lastParticule = 0.0f;

    CAuto::Init();
}


// Management of an event.

BOOL CAutoDestroyer::EventProcess(const Event &event)
{
    CObject*    scrap;
    CPyro*      pyro;
    D3DVECTOR   pos, speed;
    FPOINT      dim;

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
                scrap->SetLock(TRUE);  // usable waste
//?             scrap->SetTruck(m_object);  // usable waste

                if ( SearchVehicle() )
                {
                    m_phase    = ADEP_WAIT;  // still waiting ...
                    m_progress = 0.0f;
                    m_speed    = 1.0f/0.5f;
                }
                else
                {
                    m_sound->Play(SOUND_PSHHH2, m_object->RetPosition(0), 1.0f, 1.0f);

                    m_phase    = ADEP_DOWN;
                    m_progress = 0.0f;
                    m_speed    = 1.0f/1.0f;
                    m_bExplo   = FALSE;
                }
            }
        }
    }

    if ( m_phase == ADEP_DOWN )
    {
        if ( m_progress >= 0.3f-0.05f && !m_bExplo )
        {
            scrap = SearchPlastic();
            if ( scrap != 0 )
            {
                pyro = new CPyro(m_iMan);
                pyro->Create(PT_FRAGT, scrap);
            }
            m_bExplo = TRUE;
        }

        if ( m_progress < 1.0f )
        {
            pos = D3DVECTOR(0.0f, -10.0f, 0.0f);
            pos.y = -Bounce(m_progress, 0.3f)*10.0f;
            m_object->SetPosition(1, pos);
        }
        else
        {
            m_object->SetPosition(1, D3DVECTOR(0.0f, -10.0f, 0.0f));
            m_sound->Play(SOUND_REPAIR, m_object->RetPosition(0));

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
            m_sound->Play(SOUND_OPEN, m_object->RetPosition(0), 1.0f, 0.8f);

            m_phase    = ADEP_UP;
            m_progress = 0.0f;
            m_speed    = 1.0f/3.0f;
        }
    }

    if ( m_phase == ADEP_UP )
    {
        if ( m_progress < 1.0f )
        {
            pos = D3DVECTOR(0.0f, -10.0f, 0.0f);
            pos.y = -(1.0f-m_progress)*10.0f;
            m_object->SetPosition(1, pos);
        }
        else
        {
            m_object->SetPosition(1, D3DVECTOR(0.0f, 0.0f, 0.0f));

            m_phase    = ADEP_WAIT;
            m_progress = 0.0f;
            m_speed    = 1.0f/0.5f;
        }
    }

    return TRUE;
}


// Creates all the interface when the object is selected.

BOOL CAutoDestroyer::CreateInterface(BOOL bSelect)
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


// Seeks plate waste in the destroyer.

CObject* CAutoDestroyer::SearchPlastic()
{
    CObject*    pObj;
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
        if ( type != OBJECT_SCRAP4 &&
             type != OBJECT_SCRAP5 )  continue;

        oPos = pObj->RetPosition(0);
        dist = Length(oPos, sPos);
        if ( dist <= 5.0f )  return pObj;
    }

    return 0;
}

// Seeks if one vehicle is too close.

BOOL CAutoDestroyer::SearchVehicle()
{
    CObject*    pObj;
    D3DVECTOR   cPos, oPos;
    ObjectType  type;
    float       oRadius, dist;
    int         i;

    cPos = m_object->RetPosition(0);

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        type = pObj->RetType();
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
        dist = Length(oPos, cPos)-oRadius;

        if ( dist < 20.0f )  return TRUE;
    }

    return FALSE;
}


// Returns an error due the state of the automation.

Error CAutoDestroyer::RetError()
{
    if ( m_object->RetVirusMode() )
    {
        return ERR_BAT_VIRUS;
    }

    return ERR_OK;
}


// Saves all parameters of the controller.

BOOL CAutoDestroyer::Write(char *line)
{
    char    name[100];

    if ( m_phase == ADEP_WAIT )  return FALSE;

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

BOOL CAutoDestroyer::Read(char *line)
{
    if ( OpInt(line, "aExist", 0) == 0 )  return FALSE;

    CAuto::Read(line);

    m_phase = (AutoDestroyerPhase)OpInt(line, "aPhase", ADEP_WAIT);
    m_progress = OpFloat(line, "aProgress", 0.0f);
    m_speed = OpFloat(line, "aSpeed", 1.0f);

    m_lastParticule = 0.0f;

    return TRUE;
}


