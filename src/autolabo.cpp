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
#include "light.h"
#include "terrain.h"
#include "camera.h"
#include "object.h"
#include "interface.h"
#include "button.h"
#include "window.h"
#include "displaytext.h"
#include "sound.h"
#include "robotmain.h"
#include "cmdtoken.h"
#include "auto.h"
#include "autolabo.h"



#define LABO_DELAY      20.0f       // duration of the analysis




// Object's constructor.

CAutoLabo::CAutoLabo(CInstanceManager* iMan, CObject* object)
                     : CAuto(iMan, object)
{
    int     i;

    for ( i=0 ; i<3 ; i++ )
    {
        m_partiRank[i] = -1;
    }
    m_partiSphere = -1;

    m_soundChannel = -1;
    Init();
}

// Object's destructor.

CAutoLabo::~CAutoLabo()
{
    this->CAuto::~CAuto();
}


// Destroys the object.

void CAutoLabo::DeleteObject(BOOL bAll)
{
    int     i;

    for ( i=0 ; i<3 ; i++ )
    {
        if ( m_partiRank[i] != -1 )
        {
            m_particule->DeleteParticule(m_partiRank[i]);
            m_partiRank[i] = -1;
        }
    }

    if ( m_partiSphere != -1 )
    {
        m_particule->DeleteParticule(m_partiSphere);
        m_partiSphere = -1;
    }

    if ( m_soundChannel != -1 )
    {
        m_sound->FlushEnvelope(m_soundChannel);
        m_sound->AddEnvelope(m_soundChannel, 0.0f, 1.0f, 1.0f, SOPER_STOP);
        m_soundChannel = -1;
    }

    CAuto::DeleteObject(bAll);
}


// Initialize the object.

void CAutoLabo::Init()
{
    m_time = 0.0f;
    m_timeVirus = 0.0f;
    m_lastParticule = 0.0f;

    m_phase    = ALAP_WAIT;  // waiting ...
    m_progress = 0.0f;
    m_speed    = 1.0f/2.0f;

    CAuto::Init();
}


// Management of an event.

BOOL CAutoLabo::EventProcess(const Event &event)
{
    CObject*    power;
    D3DVECTOR   pos, goal, speed;
    FPOINT      dim, rot;
    float       angle;
    int         i;

    CAuto::EventProcess(event);

    if ( m_engine->RetPause() )  return TRUE;

    if ( event.event == EVENT_UPDINTERFACE )
    {
        if ( m_object->RetSelect() )  CreateInterface(TRUE);
    }

    if ( m_object->RetSelect() &&  // center selected?
         (event.event == EVENT_OBJECT_RiPAW ||
          event.event == EVENT_OBJECT_RiGUN) )
    {
        if ( m_phase != ALAP_WAIT )
        {
            return FALSE;
        }

        m_research = event.event;

        if ( TestResearch(m_research) )
        {
            m_displayText->DisplayError(ERR_LABO_ALREADY, m_object);
            return FALSE;
        }

        power = m_object->RetPower();
        if ( power == 0 )
        {
            m_displayText->DisplayError(ERR_LABO_NULL, m_object);
            return FALSE;
        }
        if ( power->RetType() != OBJECT_BULLET )
        {
            m_displayText->DisplayError(ERR_LABO_BAD, m_object);
            return FALSE;
        }

        SetBusy(TRUE);
        InitProgressTotal(1.0f+1.5f+1.5f+LABO_DELAY+1.5f+1.5f+1.0f);
        UpdateInterface();

        power->SetLock(TRUE);  // ball longer usable

        SoundManip(1.0f, 1.0f, 1.0f);
        m_phase    = ALAP_OPEN1;
        m_progress = 0.0f;
        m_speed    = 1.0f/1.0f;
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

    EventProgress(event.rTime);

    if ( m_phase == ALAP_WAIT )
    {
        if ( m_progress >= 1.0f )
        {
            m_phase    = ALAP_WAIT;  // still waiting ...
            m_progress = 0.0f;
            m_speed    = 1.0f/2.0f;
        }
    }

    if ( m_phase == ALAP_OPEN1 )
    {
        if ( m_progress < 1.0f )
        {
            angle = 80.0f-(35.0f*m_progress);
            m_object->SetAngleZ(3, angle*PI/180.0f);
            m_object->SetAngleZ(4, angle*PI/180.0f);
            m_object->SetAngleZ(5, angle*PI/180.0f);
        }
        else
        {
            m_object->SetAngleZ(3, 45.0f*PI/180.0f);
            m_object->SetAngleZ(4, 45.0f*PI/180.0f);
            m_object->SetAngleZ(5, 45.0f*PI/180.0f);

            SoundManip(1.5f, 1.0f, 0.7f);
            m_phase    = ALAP_OPEN2;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.5f;
        }
    }

    if ( m_phase == ALAP_OPEN2 )
    {
        if ( m_progress < 1.0f )
        {
            pos.x = -9.0f;
            pos.y =  3.0f+m_progress*10.0f;
            pos.z =  0.0f;
            m_object->SetPosition(1, pos);
        }
        else
        {
            m_object->SetPosition(1, D3DVECTOR(-9.0f, 13.0f, 0.0f));

            SoundManip(1.5f, 1.0f, 0.5f);
            m_phase    = ALAP_OPEN3;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.5f;
        }
    }

    if ( m_phase == ALAP_OPEN3 )
    {
        if ( m_progress < 1.0f )
        {
            angle = (1.0f-m_progress)*PI/2.0f;
            m_object->SetAngleZ(1, angle);
        }
        else
        {
            m_object->SetAngleZ(1, 0.0f);

            goal = m_object->RetPosition(0);
            goal.y += 3.0f;
            pos = goal;
            pos.x -= 4.0f;
            pos.y += 4.0f;
            for ( i=0 ; i<3 ; i++ )
            {
                m_partiRank[i] = m_particule->CreateRay(pos, goal,
                                                        PARTIRAY2,
                                                        FPOINT(2.9f, 2.9f),
                                                        LABO_DELAY);
            }

            m_soundChannel = m_sound->Play(SOUND_LABO, m_object->RetPosition(0), 0.0f, 0.25f, TRUE);
            m_sound->AddEnvelope(m_soundChannel, 1.0f, 0.60f, 2.0f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 1.0f, 2.00f, 8.0f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 1.0f, 0.60f, 8.0f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 0.0f, 0.25f, 2.0f, SOPER_STOP);

            pos = m_object->RetPosition(0);
            pos.y += 4.0f;
            speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
            dim.x = 4.0f;
            dim.y = dim.x;
            m_partiSphere = m_particule->CreateParticule(pos, speed, dim, PARTISPHERE2, LABO_DELAY, 0.0f, 0.0f);

            m_phase    = ALAP_ANALYSE;
            m_progress = 0.0f;
            m_speed    = 1.0f/LABO_DELAY;
        }
    }

    if ( m_phase == ALAP_ANALYSE )
    {
        if ( m_progress < 1.0f )
        {
            power = m_object->RetPower();
            if ( power != 0 )
            {
                power->SetZoom(0, 1.0f-m_progress);
            }

            angle = m_object->RetAngleY(2);
            if ( m_progress < 0.5f )
            {
                angle -= event.rTime*m_progress*20.0f;
            }
            else
            {
                angle -= event.rTime*(20.0f-m_progress*20.0f);
            }
            m_object->SetAngleY(2, angle);  // rotates the analyzer

            angle += m_object->RetAngleY(0);
            for ( i=0 ; i<3 ; i++ )
            {
                rot = RotatePoint(-angle, -4.0f);
                pos = m_object->RetPosition(0);
                pos.x += rot.x;
                pos.z += rot.y;
                pos.y += 3.0f+4.0f;;
                m_particule->SetPosition(m_partiRank[i], pos);  // adjusts ray

                angle += PI*2.0f/3.0f;
            }

            if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
            {
                m_lastParticule = m_time;

                if ( m_progress > 0.25f &&
                     m_progress < 0.80f )
                {
                    pos = m_object->RetPosition(0);
                    pos.y += 3.0f;
                    pos.x += (Rand()-0.5f)*2.0f;
                    pos.z += (Rand()-0.5f)*2.0f;
                    speed.y = Rand()*5.0f+5.0f;
                    speed.x = (Rand()-0.5f)*10.0f;
                    speed.z = (Rand()-0.5f)*10.0f;
                    dim.x = Rand()*0.4f*m_progress+1.0f;
                    dim.y = dim.x;
                    m_particule->CreateTrack(pos, speed, dim, PARTITRACK2,
                                             2.0f+2.0f*m_progress, 10.0f, 1.5f, 1.4f);
                }
            }
        }
        else
        {
            SetResearch(m_research);  // research done

            power = m_object->RetPower();
            if ( power != 0 )
            {
                m_object->SetPower(0);
                power->DeleteObject();  // destroys the ball
                delete power;
            }

            m_displayText->DisplayError(INFO_LABO, m_object);

            SoundManip(1.5f, 1.0f, 0.5f);
            m_phase    = ALAP_CLOSE1;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.5f;
        }
    }

    if ( m_phase == ALAP_CLOSE1 )
    {
        if ( m_progress < 1.0f )
        {
            angle = m_progress*PI/2.0f;
            m_object->SetAngleZ(1, angle);
        }
        else
        {
            m_object->SetAngleZ(1, PI/2.0f);

            SoundManip(1.5f, 1.0f, 0.7f);
            m_phase    = ALAP_CLOSE2;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.5f;
        }
    }

    if ( m_phase == ALAP_CLOSE2 )
    {
        if ( m_progress < 1.0f )
        {
            pos.x = -9.0f;
            pos.y =  3.0f+(1.0f-m_progress)*10.0f;;
            pos.z =  0.0f;
            m_object->SetPosition(1, pos);
        }
        else
        {
            m_object->SetPosition(1, D3DVECTOR(-9.0f, 3.0f, 0.0f));

            SoundManip(1.0f, 1.0f, 1.0f);
            m_phase    = ALAP_CLOSE3;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.0f;
        }
    }

    if ( m_phase == ALAP_CLOSE3 )
    {
        if ( m_progress < 1.0f )
        {
            angle = 45.0f+(35.0f*m_progress);
            m_object->SetAngleZ(3, angle*PI/180.0f);
            m_object->SetAngleZ(4, angle*PI/180.0f);
            m_object->SetAngleZ(5, angle*PI/180.0f);
        }
        else
        {
            m_object->SetAngleZ(3, 80.0f*PI/180.0f);
            m_object->SetAngleZ(4, 80.0f*PI/180.0f);
            m_object->SetAngleZ(5, 80.0f*PI/180.0f);

            SetBusy(FALSE);
            UpdateInterface();

            m_phase    = ALAP_WAIT;
            m_progress = 0.0f;
            m_speed    = 1.0f/2.0f;
        }
    }

    return TRUE;
}


// Returns an error due the state of the automation.

Error CAutoLabo::RetError()
{
    CObject*    pObj;
    ObjectType  type;

    if ( m_object->RetVirusMode() )
    {
        return ERR_BAT_VIRUS;
    }

    pObj = m_object->RetPower();
    if ( pObj == 0 )  return ERR_LABO_NULL;
    type = pObj->RetType();
    if ( type != OBJECT_BULLET )  return ERR_LABO_BAD;

    return ERR_OK;
}


// Creates all the interface when the object is selected.

BOOL CAutoLabo::CreateInterface(BOOL bSelect)
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
    pos.y = oy+sy*0.5f;
    pw->CreateButton(pos, dim, 64+45, EVENT_OBJECT_RiPAW);

    pos.x = ox+sx*8.0f;
    pos.y = oy+sy*0.5f;
    pw->CreateButton(pos, dim, 64+46, EVENT_OBJECT_RiGUN);

    pos.x = ox+sx*0.0f;
    pos.y = oy+sy*0;
    ddim.x = 66.0f/640.0f;
    ddim.y = 66.0f/480.0f;
    pw->CreateGroup(pos, ddim, 111, EVENT_OBJECT_TYPE);

    UpdateInterface();

    return TRUE;
}

// Updates the status of all interface buttons.

void CAutoLabo::UpdateInterface()
{
    CWindow*    pw;

    if ( !m_object->RetSelect() )  return;

    CAuto::UpdateInterface();

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW0);
    if ( pw == 0 )  return;

    DeadInterface(pw, EVENT_OBJECT_RiPAW, g_researchEnable&RESEARCH_iPAW);
    DeadInterface(pw, EVENT_OBJECT_RiGUN, g_researchEnable&RESEARCH_iGUN);

    OkayButton(pw, EVENT_OBJECT_RiPAW);
    OkayButton(pw, EVENT_OBJECT_RiGUN);

    VisibleInterface(pw, EVENT_OBJECT_RiPAW, !m_bBusy);
    VisibleInterface(pw, EVENT_OBJECT_RiGUN, !m_bBusy);
}

// Indicates the research conducted for a button.

void CAutoLabo::OkayButton(CWindow *pw, EventMsg event)
{
    CControl*   control;

    control = pw->SearchControl(event);
    if ( control == 0 )  return;

    control->SetState(STATE_OKAY, TestResearch(event));
}


// Test whether a search has already been done.

BOOL CAutoLabo::TestResearch(EventMsg event)
{
    if ( event == EVENT_OBJECT_RiPAW )  return (g_researchDone & RESEARCH_iPAW);
    if ( event == EVENT_OBJECT_RiGUN )  return (g_researchDone & RESEARCH_iGUN);

    return FALSE;
}

// Indicates a search as made.

void CAutoLabo::SetResearch(EventMsg event)
{
    Event   newEvent;

    if ( event == EVENT_OBJECT_RiPAW )  g_researchDone |= RESEARCH_iPAW;
    if ( event == EVENT_OBJECT_RiGUN )  g_researchDone |= RESEARCH_iGUN;

    m_main->WriteFreeParam();

    m_event->MakeEvent(newEvent, EVENT_UPDINTERFACE);
    m_event->AddEvent(newEvent);
    UpdateInterface();
}

// Plays the sound of the manipulator arm.

void CAutoLabo::SoundManip(float time, float amplitude, float frequency)
{
    int     i;

    i = m_sound->Play(SOUND_MANIP, m_object->RetPosition(0), 0.0f, 0.3f*frequency, TRUE);
    m_sound->AddEnvelope(i, 0.5f*amplitude, 1.0f*frequency, 0.1f, SOPER_CONTINUE);
    m_sound->AddEnvelope(i, 0.5f*amplitude, 1.0f*frequency, time-0.1f, SOPER_CONTINUE);
    m_sound->AddEnvelope(i, 0.0f, 0.3f*frequency, 0.1f, SOPER_STOP);
}


// Saves all parameters of the controller.

BOOL CAutoLabo::Write(char *line)
{
    D3DVECTOR   pos;
    char        name[100];

    if ( m_phase == ALAP_WAIT )  return FALSE;

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

BOOL CAutoLabo::Read(char *line)
{
    D3DVECTOR   pos;

    if ( OpInt(line, "aExist", 0) == 0 )  return FALSE;

    CAuto::Read(line);

    m_phase = (AutoLaboPhase)OpInt(line, "aPhase", ALAP_WAIT);
    m_progress = OpFloat(line, "aProgress", 0.0f);
    m_speed = OpFloat(line, "aSpeed", 1.0f);
    m_research = (EventMsg)OpInt(line, "aResearch", 0);

    m_lastParticule = 0.0f;

    return TRUE;
}


