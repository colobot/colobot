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
#include "interface.h"
#include "button.h"
#include "list.h"
#include "window.h"
#include "sound.h"
#include "cmdtoken.h"
#include "auto.h"
#include "autoinfo.h"




// Object's constructor.

CAutoInfo::CAutoInfo(CInstanceManager* iMan, CObject* object)
                           : CAuto(iMan, object)
{
    Init();
}

// Object's destructor.

CAutoInfo::~CAutoInfo()
{
    this->CAuto::~CAuto();
}


// Destroys the object.

void CAutoInfo::DeleteObject(BOOL bAll)
{
    CAuto::DeleteObject(bAll);
}


// Initialize the object.

void CAutoInfo::Init()
{
    m_phase = AIP_WAIT;
    m_time = 0.0f;
    m_timeVirus = 0.0f;
    m_bLastVirus = FALSE;

    CAuto::Init();
}


// Start a emission.

void CAutoInfo::Start(int param)
{
    D3DVECTOR   pos, speed;
    FPOINT      dim;

    if ( param == 0 )  // instruction "receive" ?
    {
        m_phase = AIP_EMETTE;
        m_progress = 0.0f;
        m_speed = 1.0f/2.0f;
    }
    else if ( param == 2 )  // instruction "send" ?
    {
        m_phase = AIP_RECEIVE;
        m_progress = 0.0f;
        m_speed = 1.0f/2.0f;
    }
    else
    {
        m_phase = AIP_ERROR;
        m_progress = 0.0f;
        m_speed = 1.0f/2.0f;
    }

    m_lastParticule = 0;
    m_goal = m_object->RetPosition(0);

    if ( m_phase == AIP_EMETTE )
    {
        pos = m_goal;
        pos.y += 9.5f;
        speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
        dim.x = 30.0f;
        dim.y = dim.x;
        m_particule->CreateParticule(pos, speed, dim, PARTISPHERE4, 1.5f, 0.0f, 0.0f);

        m_sound->Play(SOUND_LABO, pos, 1.0f, 2.0f);
    }
    if ( m_phase == AIP_RECEIVE )
    {
        pos = m_goal;
        pos.y += 9.5f;
        speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
        dim.x = 50.0f;
        dim.y = dim.x;
        m_particule->CreateParticule(pos, speed, dim, PARTISPHERE6, 1.5f, 0.0f, 0.0f);

        m_sound->Play(SOUND_LABO, pos, 1.0f, 2.0f);
    }
    if ( m_phase == AIP_ERROR )
    {
        m_sound->Play(SOUND_GGG, pos, 1.0f, 0.5f);
    }
}


// Management of an event.

BOOL CAutoInfo::EventProcess(const Event &event)
{
    D3DVECTOR   pos, speed;
    FPOINT      dim;
    float       duration, angle, rTime;
    int         i;

    CAuto::EventProcess(event);

    if ( m_engine->RetPause() )  return TRUE;
    if ( event.event != EVENT_FRAME )  return TRUE;

    m_timeVirus -= event.rTime;

    if ( m_object->RetVirusMode() )  // contaminated by a virus?
    {
        if ( m_timeVirus <= 0.0f )
        {
            m_timeVirus = 0.1f+Rand()*0.3f;

            angle = m_object->RetAngleY(1);
            angle += Rand()*0.3f;
            m_object->SetAngleY(1, angle);

            m_object->SetAngleX(2, (Rand()-0.5f)*0.3f);
            m_object->SetAngleX(4, (Rand()-0.5f)*0.3f);
            m_object->SetAngleX(6, (Rand()-0.5f)*0.3f);

            m_object->SetAngleZ(2, (Rand()-0.5f)*0.3f);
            m_object->SetAngleZ(4, (Rand()-0.5f)*0.3f);
            m_object->SetAngleZ(6, (Rand()-0.5f)*0.3f);

            UpdateListVirus();
        }
        m_bLastVirus = TRUE;
        return TRUE;
    }
    else
    {
        if ( m_bLastVirus )
        {
            m_bLastVirus = FALSE;
            UpdateList();  // normally returns the list
        }
        else
        {
            if ( m_object->RetInfoUpdate() )
            {
                UpdateList();  // updates the list
            }
        }
    }

    UpdateInterface(event.rTime);

    rTime = event.rTime;

    if ( m_phase == AIP_EMETTE )  // instruction "receive" ?
    {
        if ( m_progress < 0.5f &&
             m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
        {
            m_lastParticule = m_time;

            for ( i=0 ; i<4 ; i++ )
            {
                pos = m_goal;
                pos.y += 9.5f;
                speed.x = (Rand()-0.5f)*50.0f;
                speed.z = (Rand()-0.5f)*50.0f;
                speed.y = (Rand()-0.5f)*50.0f;
                speed *= 0.5f+m_progress*0.5f;
                dim.x = 0.6f;
                dim.y = dim.x;
                duration = Rand()*0.5f+0.5f;
                m_particule->CreateTrack(pos, speed, dim, PARTITRACK6,
                                         duration, 0.0f,
                                         duration*0.9f, 0.7f);
            }
        }

        if ( m_progress < 1.0f )
        {
            m_progress += rTime*m_speed;

            m_object->SetAngleZ(2, m_progress*2.0f*PI);
            m_object->SetAngleZ(4, m_progress*2.0f*PI);
            m_object->SetAngleZ(6, m_progress*2.0f*PI);
        }
        else
        {
            m_phase = AIP_WAIT;

            m_object->SetAngleX(2, 0.0f);
            m_object->SetAngleX(4, 0.0f);
            m_object->SetAngleX(6, 0.0f);

            m_object->SetAngleZ(2, 0.0f);
            m_object->SetAngleZ(4, 0.0f);
            m_object->SetAngleZ(6, 0.0f);
        }
    }

    if ( m_phase == AIP_RECEIVE )  // instruction "send" ?
    {
        if ( m_progress < 0.5f &&
             m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
        {
            m_lastParticule = m_time;

            for ( i=0 ; i<4 ; i++ )
            {
                pos = m_goal;
                pos.y += 9.5f;
                speed = pos;
                pos.x += (Rand()-0.5f)*40.0f;
                pos.y += (Rand()-0.5f)*40.0f;
                pos.z += (Rand()-0.5f)*40.0f;
                speed = (speed-pos)*1.0f;
//?             speed *= 0.5f+m_progress*0.5f;
                dim.x = 0.6f;
                dim.y = dim.x;
                duration = Rand()*0.5f+0.5f;
                m_particule->CreateTrack(pos, speed, dim, PARTITRACK6,
                                         duration, 0.0f,
                                         duration*0.9f, 0.7f);
            }
        }

        if ( m_progress < 1.0f )
        {
            m_progress += rTime*m_speed;

            m_object->SetAngleZ(2, m_progress*2.0f*PI);
            m_object->SetAngleZ(4, m_progress*2.0f*PI);
            m_object->SetAngleZ(6, m_progress*2.0f*PI);
        }
        else
        {
            m_phase = AIP_WAIT;

            m_object->SetAngleX(2, 0.0f);
            m_object->SetAngleX(4, 0.0f);
            m_object->SetAngleX(6, 0.0f);

            m_object->SetAngleZ(2, 0.0f);
            m_object->SetAngleZ(4, 0.0f);
            m_object->SetAngleZ(6, 0.0f);
        }
    }

    if ( m_phase == AIP_ERROR )
    {
        if ( m_progress < 0.5f &&
             m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
        {
            m_lastParticule = m_time;

            pos = m_goal;
            speed.x = (Rand()-0.5f)*5.0f;
            speed.z = (Rand()-0.5f)*5.0f;
            speed.y = 5.0f+Rand()*5.0f;
            dim.x = 5.0f+Rand()*5.0f;
            dim.y = dim.x;
            duration = Rand()*0.5f+0.5f;
            m_particule->CreateParticule(pos, speed, dim, PARTISMOKE1, 4.0f);
        }

        if ( m_progress < 1.0f )
        {
            m_progress += rTime*m_speed;
            rTime = 0.0f;  // stops the rotation

            if ( m_progress < 0.5f )
            {
                angle = m_progress/0.5f;
            }
            else
            {
                angle = 1.0f-(m_progress-0.5f)/0.5f;
            }
            m_object->SetAngleX(2, angle*0.5f);
            m_object->SetAngleX(4, angle*0.5f);
            m_object->SetAngleX(6, angle*0.5f);

            m_object->SetAngleZ(2, (Rand()-0.5f)*0.2f);
            m_object->SetAngleZ(4, (Rand()-0.5f)*0.2f);
            m_object->SetAngleZ(6, (Rand()-0.5f)*0.2f);
        }
        else
        {
            m_phase = AIP_WAIT;

            m_object->SetAngleX(2, 0.0f);
            m_object->SetAngleX(4, 0.0f);
            m_object->SetAngleX(6, 0.0f);

            m_object->SetAngleZ(2, 0.0f);
            m_object->SetAngleZ(4, 0.0f);
            m_object->SetAngleZ(6, 0.0f);
        }
    }

    angle = m_object->RetAngleY(1);
    angle += rTime*0.5f;
    m_object->SetAngleY(1, angle);

    m_object->SetAngleX(3, sinf(m_time*6.0f+PI*0.0f/3.0f)*0.3f);
    m_object->SetAngleX(5, sinf(m_time*6.0f+PI*2.0f/3.0f)*0.3f);
    m_object->SetAngleX(7, sinf(m_time*6.0f+PI*4.0f/3.0f)*0.3f);

    return TRUE;
}


// Returns an error due the state of the automation.

Error CAutoInfo::RetError()
{
    if ( m_object->RetVirusMode() )
    {
        return ERR_BAT_VIRUS;
    }

    return ERR_OK;
}


// Creates all the interface when the object is selected.

BOOL CAutoInfo::CreateInterface(BOOL bSelect)
{
    CWindow*    pw;
    CList*      pl;
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

    pos.x = ox+sx*7.0f;
    pos.y = oy+sy*0.0f;
    ddim.x = 160.0f/640.0f;
    ddim.y =  66.0f/480.0f;
    pl = pw->CreateList(pos, ddim, 1, EVENT_OBJECT_GINFO, 1.10f);
    pl->SetSelectCap(FALSE);

    pos.x = ox+sx*0.0f;
    pos.y = oy+sy*0;
    ddim.x = 66.0f/640.0f;
    ddim.y = 66.0f/480.0f;
    pw->CreateGroup(pos, ddim, 112, EVENT_OBJECT_TYPE);

    UpdateList();
    return TRUE;
}

// Updates the state of all buttons on the interface,
// following the time that elapses ...

void CAutoInfo::UpdateInterface(float rTime)
{
    CAuto::UpdateInterface(rTime);
}


// Updates the contents of the list.

void CAutoInfo::UpdateList()
{
    CWindow*    pw;
    CList*      pl;
    Info        info;
    int         total, i;
    char        text[100];

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW0);
    if ( pw == 0 )  return;

    pl = (CList*)pw->SearchControl(EVENT_OBJECT_GINFO);
    if ( pl == 0 )  return;

    pl->Flush();
    total = m_object->RetInfoTotal();
    if ( total == 0 )
    {
        pl->ClearState(STATE_ENABLE);
    }
    else
    {
        pl->SetState(STATE_ENABLE);

        for ( i=0 ; i<total ; i++ )
        {
            info = m_object->RetInfo(i);
            sprintf(text, "%s = %.2f", info.name, info.value);
            pl->SetName(i, text);
        }
    }

    m_object->SetInfoUpdate(FALSE);
}

// Updates the content of contaminating the list.

void CAutoInfo::UpdateListVirus()
{
    CWindow*    pw;
    CList*      pl;
    int         i, j, max;
    char        text[100];

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW0);
    if ( pw == 0 )  return;

    pl = (CList*)pw->SearchControl(EVENT_OBJECT_GINFO);
    if ( pl == 0 )  return;

    pl->SetState(STATE_ENABLE);

    pl->Flush();
    for ( i=0 ; i<4 ; i++ )
    {
        max = (int)(2.0f+Rand()*10.0f);
        for ( j=0 ; j<max ; j++ )
        {
            do
            {
                text[j] = ' '+(int)(Rand()*94.0f);
            }
            while ( text[j] == '\\' );
        }
        text[j] = 0;

        pl->SetName(i, text);
    }
}


// Saves all parameters of the controller.

BOOL CAutoInfo::Write(char *line)
{
    char    name[100];

    if ( m_phase == AIP_WAIT )  return FALSE;

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

BOOL CAutoInfo::Read(char *line)
{
    if ( OpInt(line, "aExist", 0) == 0 )  return FALSE;

    CAuto::Read(line);

    m_phase = (AutoInfoPhase)OpInt(line, "aPhase", AIP_WAIT);
    m_progress = OpFloat(line, "aProgress", 0.0f);
    m_speed = OpFloat(line, "aSpeed", 1.0f);

    m_lastParticule = 0.0f;

    return TRUE;
}


