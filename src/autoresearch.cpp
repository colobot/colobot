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
#include "global.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "math3d.h"
#include "particule.h"
#include "terrain.h"
#include "camera.h"
#include "object.h"
#include "interface.h"
#include "button.h"
#include "gauge.h"
#include "window.h"
#include "displaytext.h"
#include "sound.h"
#include "robotmain.h"
#include "cmdtoken.h"
#include "auto.h"
#include "autoresearch.h"



#define SEARCH_TIME     30.0f       // duration of a research



// Object's constructor.

CAutoResearch::CAutoResearch(CInstanceManager* iMan, CObject* object)
                             : CAuto(iMan, object)
{
    int     i;

    for ( i=0 ; i<6 ; i++ )
    {
        m_partiStop[i] = -1;
    }
    m_channelSound = -1;

    Init();
}

// Object's destructor.

CAutoResearch::~CAutoResearch()
{
    this->CAuto::~CAuto();
}


// Destroys the object.

void CAutoResearch::DeleteObject(BOOL bAll)
{
    if ( m_channelSound != -1 )
    {
        m_sound->FlushEnvelope(m_channelSound);
        m_sound->AddEnvelope(m_channelSound, 0.0f, 1.0f, 1.0f, SOPER_STOP);
        m_channelSound = -1;
    }

    FireStopUpdate(0.0f, FALSE);
    CAuto::DeleteObject(bAll);
}


// Initialize the object.

void CAutoResearch::Init()
{
    m_phase    = ALP_WAIT;
    m_progress = 0.0f;
    m_speed    = 1.0f/1.0f;

    m_time = 0.0f;
    m_timeVirus = 0.0f;
    m_lastUpdateTime = 0.0f;
    m_lastParticule = 0.0f;
}


// Management of an event.

BOOL CAutoResearch::EventProcess(const Event &event)
{
    CObject*    power;
    D3DVECTOR   pos, speed;
    Error       message;
    FPOINT      dim;
    float       angle, time;

    CAuto::EventProcess(event);

    if ( m_engine->RetPause() )  return TRUE;

    if ( event.event == EVENT_UPDINTERFACE )
    {
        if ( m_object->RetSelect() )  CreateInterface(TRUE);
    }

    if ( m_object->RetSelect() &&  // center selected?
         (event.event == EVENT_OBJECT_RTANK   ||
          event.event == EVENT_OBJECT_RFLY    ||
          event.event == EVENT_OBJECT_RTHUMP  ||
          event.event == EVENT_OBJECT_RCANON  ||
          event.event == EVENT_OBJECT_RTOWER  ||
          event.event == EVENT_OBJECT_RPHAZER ||
          event.event == EVENT_OBJECT_RSHIELD ||
          event.event == EVENT_OBJECT_RATOMIC ) )
    {
        if ( m_phase != ALP_WAIT )
        {
            return FALSE;
        }

        m_research = event.event;

        if ( TestResearch(m_research) )
        {
            m_displayText->DisplayError(ERR_RESEARCH_ALREADY, m_object);
            return FALSE;
        }

        power = m_object->RetPower();
        if ( power == 0 )
        {
            m_displayText->DisplayError(ERR_RESEARCH_POWER, m_object);
            return FALSE;
        }
        if ( power->RetCapacity() > 1.0f )
        {
            m_displayText->DisplayError(ERR_RESEARCH_TYPE, m_object);
            return FALSE;
        }
        if ( power->RetEnergy() < 1.0f )
        {
            m_displayText->DisplayError(ERR_RESEARCH_ENERGY, m_object);
            return FALSE;
        }

        time = SEARCH_TIME;
        if ( event.event == EVENT_OBJECT_RTANK   )  time *= 0.3f;
        if ( event.event == EVENT_OBJECT_RFLY    )  time *= 0.3f;
        if ( event.event == EVENT_OBJECT_RATOMIC )  time *= 2.0f;

        SetBusy(TRUE);
        InitProgressTotal(time);
        UpdateInterface();

        m_channelSound = m_sound->Play(SOUND_RESEARCH, m_object->RetPosition(0), 0.0f, 1.0f, TRUE);
        m_sound->AddEnvelope(m_channelSound, 1.0f, 1.0f,      2.0f, SOPER_CONTINUE);
        m_sound->AddEnvelope(m_channelSound, 1.0f, 1.0f, time-4.0f, SOPER_CONTINUE);
        m_sound->AddEnvelope(m_channelSound, 0.0f, 1.0f,      2.0f, SOPER_STOP);

        m_phase    = ALP_SEARCH;
        m_progress = 0.0f;
        m_speed    = 1.0f/time;
        return TRUE;
    }

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

    UpdateInterface(event.rTime);
    EventProgress(event.rTime);

    angle = m_time*0.1f;
    m_object->SetAngleY(1, angle);  // rotates the antenna

    angle = (30.0f+sinf(m_time*0.3f)*20.0f)*PI/180.0f;
    m_object->SetAngleZ(2, angle);  // directs the antenna

    if ( m_phase == ALP_WAIT )
    {
        FireStopUpdate(m_progress, FALSE);  // extinguished
        return TRUE;
    }

    if ( m_phase == ALP_SEARCH )
    {
        FireStopUpdate(m_progress, TRUE);  // flashes
        if ( m_progress < 1.0f )
        {
            power = m_object->RetPower();
            if ( power == 0 )  // more battery?
            {
                SetBusy(FALSE);
                UpdateInterface();

                m_phase    = ALP_WAIT;
                m_progress = 0.0f;
                m_speed    = 1.0f/1.0f;
                return TRUE;
            }
            power->SetEnergy(1.0f-m_progress);

            if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
            {
                m_lastParticule = m_time;

                pos = m_object->RetPosition(0);
                pos.x += (Rand()-0.5f)*6.0f;
                pos.z += (Rand()-0.5f)*6.0f;
                pos.y += 11.0f;
                speed.x = (Rand()-0.5f)*2.0f;
                speed.z = (Rand()-0.5f)*2.0f;
                speed.y = Rand()*20.0f;
                dim.x = Rand()*1.0f+1.0f;
                dim.y = dim.x;
                m_particule->CreateParticule(pos, speed, dim, PARTIVAPOR);
            }
        }
        else
        {
            SetResearch(m_research);  // research done
            m_displayText->DisplayError(INFO_RESEARCH, m_object);

            message = ERR_OK;
            if ( m_research == EVENT_OBJECT_RTANK   )  message = INFO_RESEARCHTANK;
            if ( m_research == EVENT_OBJECT_RFLY    )  message = INFO_RESEARCHFLY;
            if ( m_research == EVENT_OBJECT_RTHUMP  )  message = INFO_RESEARCHTHUMP;
            if ( m_research == EVENT_OBJECT_RCANON  )  message = INFO_RESEARCHCANON;
            if ( m_research == EVENT_OBJECT_RTOWER  )  message = INFO_RESEARCHTOWER;
            if ( m_research == EVENT_OBJECT_RPHAZER )  message = INFO_RESEARCHPHAZER;
            if ( m_research == EVENT_OBJECT_RSHIELD )  message = INFO_RESEARCHSHIELD;
            if ( m_research == EVENT_OBJECT_RATOMIC )  message = INFO_RESEARCHATOMIC;
            if ( message != ERR_OK )
            {
                m_displayText->DisplayError(message, m_object);
            }

            SetBusy(FALSE);
            UpdateInterface();

            m_phase    = ALP_WAIT;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.0f;
        }
    }

    return TRUE;
}


// Returns an error due the state of the automation.

Error CAutoResearch::RetError()
{
    CObject*    power;

    if ( m_phase == ALP_SEARCH )
    {
        return ERR_OK;
    }

    if ( m_object->RetVirusMode() )
    {
        return ERR_BAT_VIRUS;
    }

    power = m_object->RetPower();
    if ( power == 0 )
    {
        return ERR_RESEARCH_POWER;
    }
    if ( power != 0 && power->RetCapacity() > 1.0f )
    {
        return ERR_RESEARCH_TYPE;
    }
    if ( power != 0 && power->RetEnergy() < 1.0f )
    {
        return ERR_RESEARCH_ENERGY;
    }

    return ERR_OK;
}


// Creates all the interface when the object is selected.

BOOL CAutoResearch::CreateInterface(BOOL bSelect)
{
    CWindow*    pw;
    FPOINT      pos, dim, ddim;
    float       ox, oy, sx, sy;

    CAuto::CreateInterface(bSelect);

    if ( !bSelect )  return TRUE;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW0);
    if ( pw == 0 )  return FALSE;

    dim.x = 33.0f/640.0f;
    dim.y = 33.0f/480.0f;
    ox = 3.0f/640.0f;
    oy = 3.0f/480.0f;
    sx = 33.0f/640.0f;
    sy = 33.0f/480.0f;

    pos.x = ox+sx*7.0f;
    pos.y = oy+sy*1.0f;
    pw->CreateButton(pos, dim, 64+0, EVENT_OBJECT_RTANK);

    pos.x = ox+sx*8.0f;
    pos.y = oy+sy*1.0f;
    pw->CreateButton(pos, dim, 64+1, EVENT_OBJECT_RFLY);

    pos.x = ox+sx*9.0f;
    pos.y = oy+sy*1.0f;
    pw->CreateButton(pos, dim, 64+3, EVENT_OBJECT_RCANON);

    pos.x = ox+sx*10.0f;
    pos.y = oy+sy*1.0f;
    pw->CreateButton(pos, dim, 64+4, EVENT_OBJECT_RTOWER);

    pos.x = ox+sx*7.0f;
    pos.y = oy+sy*0.0f;
    pw->CreateButton(pos, dim, 64+7, EVENT_OBJECT_RATOMIC);

    pos.x = ox+sx*8.0f;
    pos.y = oy+sy*0.0f;
    pw->CreateButton(pos, dim, 64+2, EVENT_OBJECT_RTHUMP);

    pos.x = ox+sx*9.0f;
    pos.y = oy+sy*0.0f;
    pw->CreateButton(pos, dim, 64+6, EVENT_OBJECT_RSHIELD);

    pos.x = ox+sx*10.0f;
    pos.y = oy+sy*0.0f;
    pw->CreateButton(pos, dim, 64+5, EVENT_OBJECT_RPHAZER);

    pos.x = ox+sx*14.5f;
    pos.y = oy+sy*0;
    ddim.x = 14.0f/640.0f;
    ddim.y = 66.0f/480.0f;
    pw->CreateGauge(pos, ddim, 0, EVENT_OBJECT_GENERGY);

    pos.x = ox+sx*0.0f;
    pos.y = oy+sy*0;
    ddim.x = 66.0f/640.0f;
    ddim.y = 66.0f/480.0f;
    pw->CreateGroup(pos, ddim, 102, EVENT_OBJECT_TYPE);

    UpdateInterface();

    return TRUE;
}

// Updates the status of all interface buttons.

void CAutoResearch::UpdateInterface()
{
    CWindow*    pw;

    if ( !m_object->RetSelect() )  return;

    CAuto::UpdateInterface();

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW0);
    if ( pw == 0 )  return;

    DeadInterface(pw, EVENT_OBJECT_RTANK,   g_researchEnable&RESEARCH_TANK);
    DeadInterface(pw, EVENT_OBJECT_RFLY,    g_researchEnable&RESEARCH_FLY);
    DeadInterface(pw, EVENT_OBJECT_RTHUMP,  g_researchEnable&RESEARCH_THUMP);
    DeadInterface(pw, EVENT_OBJECT_RCANON,  g_researchEnable&RESEARCH_CANON);
    DeadInterface(pw, EVENT_OBJECT_RTOWER,  g_researchEnable&RESEARCH_TOWER);
    DeadInterface(pw, EVENT_OBJECT_RPHAZER, g_researchEnable&RESEARCH_PHAZER);
    DeadInterface(pw, EVENT_OBJECT_RSHIELD, g_researchEnable&RESEARCH_SHIELD);
    DeadInterface(pw, EVENT_OBJECT_RATOMIC, g_researchEnable&RESEARCH_ATOMIC);

    OkayButton(pw, EVENT_OBJECT_RTANK);
    OkayButton(pw, EVENT_OBJECT_RFLY);
    OkayButton(pw, EVENT_OBJECT_RTHUMP);
    OkayButton(pw, EVENT_OBJECT_RCANON);
    OkayButton(pw, EVENT_OBJECT_RTOWER);
    OkayButton(pw, EVENT_OBJECT_RPHAZER);
    OkayButton(pw, EVENT_OBJECT_RSHIELD);
    OkayButton(pw, EVENT_OBJECT_RATOMIC);

    VisibleInterface(pw, EVENT_OBJECT_RTANK,   !m_bBusy);
    VisibleInterface(pw, EVENT_OBJECT_RFLY,    !m_bBusy);
    VisibleInterface(pw, EVENT_OBJECT_RTHUMP,  !m_bBusy);
    VisibleInterface(pw, EVENT_OBJECT_RCANON,  !m_bBusy);
    VisibleInterface(pw, EVENT_OBJECT_RTOWER,  !m_bBusy);
    VisibleInterface(pw, EVENT_OBJECT_RPHAZER, !m_bBusy);
    VisibleInterface(pw, EVENT_OBJECT_RSHIELD, !m_bBusy);
    VisibleInterface(pw, EVENT_OBJECT_RATOMIC, !m_bBusy);
}

// Updates the state of all buttons on the interface,
// following the time that elapses ...

void CAutoResearch::UpdateInterface(float rTime)
{
    CWindow*    pw;
    CGauge*     pg;
    CObject*    power;
    float       energy;

    CAuto::UpdateInterface(rTime);

    if ( m_time < m_lastUpdateTime+0.1f )  return;
    m_lastUpdateTime = m_time;

    if ( !m_object->RetSelect() )  return;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW0);
    if ( pw == 0 )  return;

    pg = (CGauge*)pw->SearchControl(EVENT_OBJECT_GENERGY);
    if ( pg != 0 )
    {
        energy = 0.0f;
        power = m_object->RetPower();
        if ( power != 0 )
        {
            energy = power->RetEnergy();
        }
        pg->SetLevel(energy);
    }
}

// Research shows already performed button.

void CAutoResearch::OkayButton(CWindow *pw, EventMsg event)
{
    CControl*   control;

    control = pw->SearchControl(event);
    if ( control == 0 )  return;

    control->SetState(STATE_OKAY, TestResearch(event));
}


// Test whether a search has already been done.

BOOL CAutoResearch::TestResearch(EventMsg event)
{
    if ( event == EVENT_OBJECT_RTANK   )  return (g_researchDone & RESEARCH_TANK  );
    if ( event == EVENT_OBJECT_RFLY    )  return (g_researchDone & RESEARCH_FLY   );
    if ( event == EVENT_OBJECT_RTHUMP  )  return (g_researchDone & RESEARCH_THUMP );
    if ( event == EVENT_OBJECT_RCANON  )  return (g_researchDone & RESEARCH_CANON );
    if ( event == EVENT_OBJECT_RTOWER  )  return (g_researchDone & RESEARCH_TOWER );
    if ( event == EVENT_OBJECT_RPHAZER )  return (g_researchDone & RESEARCH_PHAZER  );
    if ( event == EVENT_OBJECT_RSHIELD )  return (g_researchDone & RESEARCH_SHIELD);
    if ( event == EVENT_OBJECT_RATOMIC )  return (g_researchDone & RESEARCH_ATOMIC);

    return FALSE;
}

// Indicates a search as made.

void CAutoResearch::SetResearch(EventMsg event)
{
    Event   newEvent;

    if ( event == EVENT_OBJECT_RTANK   )  g_researchDone |= RESEARCH_TANK;
    if ( event == EVENT_OBJECT_RFLY    )  g_researchDone |= RESEARCH_FLY;
    if ( event == EVENT_OBJECT_RTHUMP  )  g_researchDone |= RESEARCH_THUMP;
    if ( event == EVENT_OBJECT_RCANON  )  g_researchDone |= RESEARCH_CANON;
    if ( event == EVENT_OBJECT_RTOWER  )  g_researchDone |= RESEARCH_TOWER;
    if ( event == EVENT_OBJECT_RPHAZER )  g_researchDone |= RESEARCH_PHAZER;
    if ( event == EVENT_OBJECT_RSHIELD )  g_researchDone |= RESEARCH_SHIELD;
    if ( event == EVENT_OBJECT_RATOMIC )  g_researchDone |= RESEARCH_ATOMIC;

    m_main->WriteFreeParam();

    m_event->MakeEvent(newEvent, EVENT_UPDINTERFACE);
    m_event->AddEvent(newEvent);
    UpdateInterface();
}


// Updates the stop lights.

void CAutoResearch::FireStopUpdate(float progress, BOOL bLightOn)
{
    D3DMATRIX*  mat;
    D3DVECTOR   pos, speed;
    FPOINT      dim;
    int         i;

    static float listpos[12] =
    {
         9.5f,   0.0f,
         4.7f,   8.2f,
        -4.7f,   8.2f,
        -9.5f,   0.0f,
        -4.7f,  -8.2f,
         4.7f,  -8.2f,
    };

    if ( !bLightOn )  // �teint ?
    {
        for ( i=0 ; i<6 ; i++ )
        {
            if ( m_partiStop[i] != -1 )
            {
                m_particule->DeleteParticule(m_partiStop[i]);
                m_partiStop[i] = -1;
            }
        }
        return;
    }

    mat = m_object->RetWorldMatrix(0);

    speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
    dim.x = 2.0f;
    dim.y = dim.x;

    for ( i=0 ; i<6 ; i++ )
    {
        if ( Mod(progress, 0.025f) < 0.005f )
        {
            if ( m_partiStop[i] != -1 )
            {
                m_particule->DeleteParticule(m_partiStop[i]);
                m_partiStop[i] = -1;
            }
        }
        else
        {
            if ( m_partiStop[i] == -1 )
            {
                pos.x = listpos[i*2+0];
                pos.y = 11.5f;
                pos.z = listpos[i*2+1];
                pos = Transform(*mat, pos);
                m_partiStop[i] = m_particule->CreateParticule(pos, speed,
                                                              dim, PARTISELY,
                                                              1.0f, 0.0f, 0.0f);
            }
        }
    }
}


// Saves all parameters of the controller.

BOOL CAutoResearch::Write(char *line)
{
    char    name[100];

    if ( m_phase == ALP_WAIT )  return FALSE;

    sprintf(name, " aExist=%d", 1);
    strcat(line, name);

    CAuto::Write(line);

    sprintf(name, " aPhase=%d", m_phase);
    strcat(line, name);

    sprintf(name, " aProgress=%.2f", m_progress);
    strcat(line, name);

    sprintf(name, " aSpeed=%.2f", m_speed);
    strcat(line, name);

    sprintf(name, " aResearch=%d", m_research);
    strcat(line, name);

    return TRUE;
}

// Restores all parameters of the controller.

BOOL CAutoResearch::Read(char *line)
{
    if ( OpInt(line, "aExist", 0) == 0 )  return FALSE;

    CAuto::Read(line);

    m_phase = (AutoResearchPhase)OpInt(line, "aPhase", ALP_WAIT);
    m_progress = OpFloat(line, "aProgress", 0.0f);
    m_speed = OpFloat(line, "aSpeed", 1.0f);
    m_research = (EventMsg)OpInt(line, "aResearch", 0);

    m_lastUpdateTime = 0.0f;
    m_lastParticule = 0.0f;

    return TRUE;
}


