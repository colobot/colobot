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


#include "object/auto/autoresearch.h"

#include "common/global.h"

#include "math/geometry.h"

#include "object/robotmain.h"

#include "script/cmdtoken.h"

#include "ui/interface.h"
#include "ui/gauge.h"
#include "ui/window.h"
#include "ui/displaytext.h"

#include <stdio.h>
#include <string.h>


const float SEARCH_TIME = 30.0f;        // duration of a research



// Object's constructor.

CAutoResearch::CAutoResearch(CObject* object) : CAuto(object)
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
}


// Destroys the object.

void CAutoResearch::DeleteObject(bool bAll)
{
    if ( m_channelSound != -1 )
    {
        m_sound->FlushEnvelope(m_channelSound);
        m_sound->AddEnvelope(m_channelSound, 0.0f, 1.0f, 1.0f, SOPER_STOP);
        m_channelSound = -1;
    }

    FireStopUpdate(0.0f, false);
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
    m_lastParticle = 0.0f;
}


// Management of an event.

bool CAutoResearch::EventProcess(const Event &event)
{
    CObject*    power;
    Math::Vector    pos, speed;
    Error       message;
    Math::Point     dim;
    float       angle, time;

    CAuto::EventProcess(event);

    if ( m_engine->GetPause() )  return true;

    if ( event.type == EVENT_UPDINTERFACE )
    {
        if ( m_object->GetSelect() )  CreateInterface(true);
    }

    if ( m_object->GetSelect() &&  // center selected?
         (event.type == EVENT_OBJECT_RTANK   ||
          event.type == EVENT_OBJECT_RFLY    ||
          event.type == EVENT_OBJECT_RTHUMP  ||
          event.type == EVENT_OBJECT_RCANON  ||
          event.type == EVENT_OBJECT_RTOWER  ||
          event.type == EVENT_OBJECT_RPHAZER ||
          event.type == EVENT_OBJECT_RSHIELD ||
          event.type == EVENT_OBJECT_RATOMIC ) )
    {
        if ( m_phase != ALP_WAIT )
        {
            return false;
        }

        m_research = event.type;

        if ( TestResearch(m_research) )
        {
            m_displayText->DisplayError(ERR_RESEARCH_ALREADY, m_object);
            return false;
        }

        power = m_object->GetPower();
        if ( power == 0 )
        {
            m_displayText->DisplayError(ERR_RESEARCH_POWER, m_object);
            return false;
        }
        if ( power->GetCapacity() > 1.0f )
        {
            m_displayText->DisplayError(ERR_RESEARCH_TYPE, m_object);
            return false;
        }
        if ( power->GetEnergy() < 1.0f )
        {
            m_displayText->DisplayError(ERR_RESEARCH_ENERGY, m_object);
            return false;
        }

        time = SEARCH_TIME;
        if ( event.type == EVENT_OBJECT_RTANK   )  time *= 0.3f;
        if ( event.type == EVENT_OBJECT_RFLY    )  time *= 0.3f;
        if ( event.type == EVENT_OBJECT_RATOMIC )  time *= 2.0f;

        SetBusy(true);
        InitProgressTotal(time);
        UpdateInterface();

        m_channelSound = m_sound->Play(SOUND_RESEARCH, m_object->GetPosition(0), 0.0f, 1.0f, true);
        m_sound->AddEnvelope(m_channelSound, 1.0f, 1.0f,      2.0f, SOPER_CONTINUE);
        m_sound->AddEnvelope(m_channelSound, 1.0f, 1.0f, time-4.0f, SOPER_CONTINUE);
        m_sound->AddEnvelope(m_channelSound, 0.0f, 1.0f,      2.0f, SOPER_STOP);

        m_phase    = ALP_SEARCH;
        m_progress = 0.0f;
        m_speed    = 1.0f/time;
        return true;
    }

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

    UpdateInterface(event.rTime);
    EventProgress(event.rTime);

    angle = m_time*0.1f;
    m_object->SetAngleY(1, angle);  // rotates the antenna

    angle = (30.0f+sinf(m_time*0.3f)*20.0f)*Math::PI/180.0f;
    m_object->SetAngleZ(2, angle);  // directs the antenna

    if ( m_phase == ALP_WAIT )
    {
        FireStopUpdate(m_progress, false);  // extinguished
        return true;
    }

    if ( m_phase == ALP_SEARCH )
    {
        FireStopUpdate(m_progress, true);  // flashes
        if ( m_progress < 1.0f )
        {
            power = m_object->GetPower();
            if ( power == 0 )  // more battery?
            {
                SetBusy(false);
                UpdateInterface();

                m_phase    = ALP_WAIT;
                m_progress = 0.0f;
                m_speed    = 1.0f/1.0f;
                return true;
            }
            power->SetEnergy(1.0f-m_progress);

            if ( m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
            {
                m_lastParticle = m_time;

                pos = m_object->GetPosition(0);
                pos.x += (Math::Rand()-0.5f)*6.0f;
                pos.z += (Math::Rand()-0.5f)*6.0f;
                pos.y += 11.0f;
                speed.x = (Math::Rand()-0.5f)*2.0f;
                speed.z = (Math::Rand()-0.5f)*2.0f;
                speed.y = Math::Rand()*20.0f;
                dim.x = Math::Rand()*1.0f+1.0f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIVAPOR);
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

            SetBusy(false);
            UpdateInterface();

            m_phase    = ALP_WAIT;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.0f;
        }
    }

    return true;
}


// Returns an error due the state of the automation.

Error CAutoResearch::GetError()
{
    CObject*    power;

    if ( m_phase == ALP_SEARCH )
    {
        return ERR_OK;
    }

    if ( m_object->GetVirusMode() )
    {
        return ERR_BAT_VIRUS;
    }

    power = m_object->GetPower();
    if ( power == 0 )
    {
        return ERR_RESEARCH_POWER;
    }
    if ( power != 0 && power->GetCapacity() > 1.0f )
    {
        return ERR_RESEARCH_TYPE;
    }
    if ( power != 0 && power->GetEnergy() < 1.0f )
    {
        return ERR_RESEARCH_ENERGY;
    }

    return ERR_OK;
}


// Creates all the interface when the object is selected.

bool CAutoResearch::CreateInterface(bool bSelect)
{
    Ui::CWindow*    pw;
    Math::Point     pos, dim, ddim;
    float       ox, oy, sx, sy;

    CAuto::CreateInterface(bSelect);

    if ( !bSelect )  return true;

    pw = static_cast< Ui::CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw == 0 )  return false;

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

    return true;
}

// Updates the status of all interface buttons.

void CAutoResearch::UpdateInterface()
{
    Ui::CWindow*    pw;

    if ( !m_object->GetSelect() )  return;

    CAuto::UpdateInterface();

    pw = static_cast< Ui::CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
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
    Ui::CWindow*    pw;
    Ui::CGauge*     pg;
    CObject*    power;
    float       energy;

    CAuto::UpdateInterface(rTime);

    if ( m_time < m_lastUpdateTime+0.1f )  return;
    m_lastUpdateTime = m_time;

    if ( !m_object->GetSelect() )  return;

    pw = static_cast< Ui::CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw == 0 )  return;

    pg = static_cast< Ui::CGauge* >(pw->SearchControl(EVENT_OBJECT_GENERGY));
    if ( pg != 0 )
    {
        energy = 0.0f;
        power = m_object->GetPower();
        if ( power != 0 )
        {
            energy = power->GetEnergy();
        }
        pg->SetLevel(energy);
    }
}

// Research shows already performed button.

void CAutoResearch::OkayButton(Ui::CWindow *pw, EventType event)
{
    Ui::CControl*   control;

    control = pw->SearchControl(event);
    if ( control == 0 )  return;

    control->SetState(Ui::STATE_OKAY, TestResearch(event));
}


// Test whether a search has already been done.

bool CAutoResearch::TestResearch(EventType event)
{
    if ( event == EVENT_OBJECT_RTANK   )  return (g_researchDone & RESEARCH_TANK  );
    if ( event == EVENT_OBJECT_RFLY    )  return (g_researchDone & RESEARCH_FLY   );
    if ( event == EVENT_OBJECT_RTHUMP  )  return (g_researchDone & RESEARCH_THUMP );
    if ( event == EVENT_OBJECT_RCANON  )  return (g_researchDone & RESEARCH_CANON );
    if ( event == EVENT_OBJECT_RTOWER  )  return (g_researchDone & RESEARCH_TOWER );
    if ( event == EVENT_OBJECT_RPHAZER )  return (g_researchDone & RESEARCH_PHAZER  );
    if ( event == EVENT_OBJECT_RSHIELD )  return (g_researchDone & RESEARCH_SHIELD);
    if ( event == EVENT_OBJECT_RATOMIC )  return (g_researchDone & RESEARCH_ATOMIC);

    return false;
}

// Indicates a search as made.

void CAutoResearch::SetResearch(EventType event)
{

    if ( event == EVENT_OBJECT_RTANK   )  g_researchDone |= RESEARCH_TANK;
    if ( event == EVENT_OBJECT_RFLY    )  g_researchDone |= RESEARCH_FLY;
    if ( event == EVENT_OBJECT_RTHUMP  )  g_researchDone |= RESEARCH_THUMP;
    if ( event == EVENT_OBJECT_RCANON  )  g_researchDone |= RESEARCH_CANON;
    if ( event == EVENT_OBJECT_RTOWER  )  g_researchDone |= RESEARCH_TOWER;
    if ( event == EVENT_OBJECT_RPHAZER )  g_researchDone |= RESEARCH_PHAZER;
    if ( event == EVENT_OBJECT_RSHIELD )  g_researchDone |= RESEARCH_SHIELD;
    if ( event == EVENT_OBJECT_RATOMIC )  g_researchDone |= RESEARCH_ATOMIC;

    m_main->WriteFreeParam();

    Event   newEvent(EVENT_UPDINTERFACE);
    m_eventQueue->AddEvent(newEvent);
    UpdateInterface();
}


// Updates the stop lights.

void CAutoResearch::FireStopUpdate(float progress, bool bLightOn)
{
    Math::Matrix*   mat;
    Math::Vector    pos, speed;
    Math::Point     dim;
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
                m_particle->DeleteParticle(m_partiStop[i]);
                m_partiStop[i] = -1;
            }
        }
        return;
    }

    mat = m_object->GetWorldMatrix(0);

    speed = Math::Vector(0.0f, 0.0f, 0.0f);
    dim.x = 2.0f;
    dim.y = dim.x;

    for ( i=0 ; i<6 ; i++ )
    {
        if ( Math::Mod(progress, 0.025f) < 0.005f )
        {
            if ( m_partiStop[i] != -1 )
            {
                m_particle->DeleteParticle(m_partiStop[i]);
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
                pos = Math::Transform(*mat, pos);
                m_partiStop[i] = m_particle->CreateParticle(pos, speed,
                                                              dim, Gfx::PARTISELY,
                                                              1.0f, 0.0f, 0.0f);
            }
        }
    }
}


// Saves all parameters of the controller.

bool CAutoResearch::Write(char *line)
{
    char    name[100];

    if ( m_phase == ALP_WAIT )  return false;

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

    return true;
}

// Restores all parameters of the controller.

bool CAutoResearch::Read(char *line)
{
    if ( OpInt(line, "aExist", 0) == 0 )  return false;

    CAuto::Read(line);

    m_phase = static_cast< AutoResearchPhase >(OpInt(line, "aPhase", ALP_WAIT));
    m_progress = OpFloat(line, "aProgress", 0.0f);
    m_speed = OpFloat(line, "aSpeed", 1.0f);
    m_research = static_cast< EventType >(OpInt(line, "aResearch", 0));

    m_lastUpdateTime = 0.0f;
    m_lastParticle = 0.0f;

    return true;
}


