/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsitec.ch; http://colobot.info; http://github.com/colobot
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://gnu.org/licenses
 */


#include "object/auto/autolabo.h"

#include "graphics/engine/engine.h"

#include "level/robotmain.h"

#include "level/parser/parserline.h"
#include "level/parser/parserparam.h"

#include "math/geometry.h"

#include "object/object_manager.h"
#include "object/old_object.h"

#include "object/interface/slotted_object.h"

#include "sound/sound.h"

#include "ui/controls/interface.h"
#include "ui/controls/window.h"



const float LABO_DELAY = 20.0f; // duration of the analysis




// Object's constructor.

CAutoLabo::CAutoLabo(COldObject* object) : CAuto(object)
{
    for (int i = 0; i < 3; i++)
    {
        m_partiRank[i] = -1;
    }
    m_partiSphere = -1;

    m_soundChannel = -1;
    Init();

    assert(object->GetNumSlots() == 1);
}

// Object's destructor.

CAutoLabo::~CAutoLabo()
{
}


// Destroys the object.

void CAutoLabo::DeleteObject(bool bAll)
{
    for (int i = 0; i < 3; i++)
    {
        if ( m_partiRank[i] != -1 )
        {
            m_particle->DeleteParticle(m_partiRank[i]);
            m_partiRank[i] = -1;
        }
    }

    if ( m_partiSphere != -1 )
    {
        m_particle->DeleteParticle(m_partiSphere);
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
    m_lastParticle = 0.0f;

    m_phase    = ALAP_WAIT;  // waiting ...
    m_progress = 0.0f;
    m_speed    = 1.0f/2.0f;

    CAuto::Init();
}


// Starts an action

Error CAutoLabo::StartAction(int param)
{
    if ( m_phase != ALAP_WAIT )
    {
        return ERR_OBJ_BUSY;
    }

    m_research = static_cast<ResearchType>(param);

    if ( m_main->IsResearchDone(m_research, m_object->GetTeam()) )
    {
        return ERR_LABO_ALREADY;
    }

    CObject* power = dynamic_cast<CSlottedObject&>(*m_object).GetSlotContainedObject(0);
    if (power == nullptr)
    {
        return ERR_LABO_NULL;
    }
    if ( m_research != RESEARCH_TARGET && power->GetType() != OBJECT_BULLET )
    {
        return ERR_LABO_BAD;
    }
    if ( m_research == RESEARCH_TARGET && power->GetType() != OBJECT_TNT )
    {
        return ERR_LABO_BAD;
    }

    SetBusy(true);
    InitProgressTotal(1.0f+1.5f+1.5f+LABO_DELAY+1.5f+1.5f+1.0f);
    UpdateInterface();

    power->SetLock(true);  // ball longer usable

    SoundManip(1.0f, 1.0f, 1.0f);
    m_phase    = ALAP_OPEN1;
    m_progress = 0.0f;
    m_speed    = 1.0f/1.0f;
    return ERR_OK;
}


// Management of an event.

bool CAutoLabo::EventProcess(const Event &event)
{
    CObject*    power;
    glm::vec3    pos, goal, speed;
    glm::vec2     dim, rot;
    float       angle;
    int         i;

    CAuto::EventProcess(event);

    if ( m_engine->GetPause() )  return true;

    if ( event.type == EVENT_UPDINTERFACE )
    {
        if ( m_object->GetSelect() )  CreateInterface(true);
    }

    if ( m_object->GetSelect() )  // center selected?
    {
        Error err = ERR_UNKNOWN;
        if ( event.type == EVENT_OBJECT_RTARGET ) err = StartAction(RESEARCH_TARGET);
        if ( event.type == EVENT_OBJECT_RiPAW   ) err = StartAction(RESEARCH_iPAW);
        if ( event.type == EVENT_OBJECT_RiGUN   ) err = StartAction(RESEARCH_iGUN);

        if( err != ERR_OK && err != ERR_UNKNOWN )
            m_main->DisplayError(err, m_object);

        if( err != ERR_UNKNOWN )
            return false;
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
            m_object->SetPartRotationZ(3, angle*Math::PI/180.0f);
            m_object->SetPartRotationZ(4, angle*Math::PI/180.0f);
            m_object->SetPartRotationZ(5, angle*Math::PI/180.0f);
        }
        else
        {
            m_object->SetPartRotationZ(3, 45.0f*Math::PI/180.0f);
            m_object->SetPartRotationZ(4, 45.0f*Math::PI/180.0f);
            m_object->SetPartRotationZ(5, 45.0f*Math::PI/180.0f);

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
            m_object->SetPartPosition(1, pos);
        }
        else
        {
            m_object->SetPartPosition(1, glm::vec3(-9.0f, 13.0f, 0.0f));

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
            angle = (1.0f-m_progress)*Math::PI/2.0f;
            m_object->SetPartRotationZ(1, angle);
        }
        else
        {
            m_object->SetPartRotationZ(1, 0.0f);

            goal = m_object->GetPosition();
            goal.y += 3.0f;
            pos = goal;
            pos.x -= 4.0f;
            pos.y += 4.0f;
            for ( i=0 ; i<3 ; i++ )
            {
                m_partiRank[i] = m_particle->CreateRay(pos, goal,
                                                        Gfx::PARTIRAY2,
                                                        { 2.9f, 2.9f },
                                                        LABO_DELAY);
            }

            m_soundChannel = m_sound->Play(SOUND_LABO, m_object->GetPosition(), 0.0f, 0.25f, true);
            m_sound->AddEnvelope(m_soundChannel, 1.0f, 0.60f, 2.0f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 1.0f, 2.00f, 8.0f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 1.0f, 0.60f, 8.0f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 0.0f, 0.25f, 2.0f, SOPER_STOP);

            pos = m_object->GetPosition();
            pos.y += 4.0f;
            speed = glm::vec3(0.0f, 0.0f, 0.0f);
            dim.x = 4.0f;
            dim.y = dim.x;
            m_partiSphere = m_particle->CreateParticle(pos, speed,
                    dim, Gfx::PARTISPHERE2, LABO_DELAY, 0.0f, 0.0f);

            m_phase    = ALAP_ANALYSE;
            m_progress = 0.0f;
            m_speed    = 1.0f/LABO_DELAY;
        }
    }

    if ( m_phase == ALAP_ANALYSE )
    {
        if ( m_progress < 1.0f )
        {
            power = dynamic_cast<CSlottedObject&>(*m_object).GetSlotContainedObject(0);
            if ( power != nullptr )
            {
                power->SetScale(1.0f-m_progress);
            }

            angle = m_object->GetPartRotationY(2);
            if ( m_progress < 0.5f )
            {
                angle -= event.rTime*m_progress*20.0f;
            }
            else
            {
                angle -= event.rTime*(20.0f-m_progress*20.0f);
            }
            m_object->SetPartRotationY(2, angle);  // rotates the analyzer

            angle += m_object->GetRotationY();
            for ( i=0 ; i<3 ; i++ )
            {
                rot = Math::RotatePoint(-angle, -4.0f);
                pos = m_object->GetPosition();
                pos.x += rot.x;
                pos.z += rot.y;
                pos.y += 3.0f+4.0f;;
                m_particle->SetPosition(m_partiRank[i], pos);  // adjusts ray

                angle += Math::PI*2.0f/3.0f;
            }

            if ( m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
            {
                m_lastParticle = m_time;

                if ( m_progress > 0.25f &&
                     m_progress < 0.80f )
                {
                    pos = m_object->GetPosition();
                    pos.y += 3.0f;
                    pos.x += (Math::Rand()-0.5f)*2.0f;
                    pos.z += (Math::Rand()-0.5f)*2.0f;
                    speed.y = Math::Rand()*5.0f+5.0f;
                    speed.x = (Math::Rand()-0.5f)*10.0f;
                    speed.z = (Math::Rand()-0.5f)*10.0f;
                    dim.x = Math::Rand()*0.4f*m_progress+1.0f;
                    dim.y = dim.x;
                    m_particle->CreateTrack(pos, speed, dim, Gfx::PARTITRACK2,
                                             2.0f+2.0f*m_progress, 10.0f, 1.5f, 1.4f);
                }
            }
        }
        else
        {
            m_main->MarkResearchDone(m_research, m_object->GetTeam());  // research done

            m_eventQueue->AddEvent(Event(EVENT_UPDINTERFACE));
            UpdateInterface();

            power = dynamic_cast<CSlottedObject&>(*m_object).GetSlotContainedObject(0);
            if ( power != nullptr )
            {
                CObjectManager::GetInstancePointer()->DeleteObject(power);
            }

            m_main->DisplayError(INFO_LABO, m_object);

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
            angle = m_progress*Math::PI/2.0f;
            m_object->SetPartRotationZ(1, angle);
        }
        else
        {
            m_object->SetPartRotationZ(1, Math::PI/2.0f);

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
            m_object->SetPartPosition(1, pos);
        }
        else
        {
            m_object->SetPartPosition(1, glm::vec3(-9.0f, 3.0f, 0.0f));

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
            m_object->SetPartRotationZ(3, angle*Math::PI/180.0f);
            m_object->SetPartRotationZ(4, angle*Math::PI/180.0f);
            m_object->SetPartRotationZ(5, angle*Math::PI/180.0f);
        }
        else
        {
            m_object->SetPartRotationZ(3, 80.0f*Math::PI/180.0f);
            m_object->SetPartRotationZ(4, 80.0f*Math::PI/180.0f);
            m_object->SetPartRotationZ(5, 80.0f*Math::PI/180.0f);

            SetBusy(false);
            UpdateInterface();

            m_phase    = ALAP_WAIT;
            m_progress = 0.0f;
            m_speed    = 1.0f/2.0f;
        }
    }

    return true;
}


// Returns an error due the state of the automation.

Error CAutoLabo::GetError()
{
    if ( m_object->GetVirusMode() )
    {
        return ERR_BAT_VIRUS;
    }

    CObject* obj = dynamic_cast<CSlottedObject&>(*m_object).GetSlotContainedObject(0);
    if (obj == nullptr)  return ERR_LABO_NULL;
    ObjectType type = obj->GetType();
    if ( type != OBJECT_BULLET && type != OBJECT_TNT )  return ERR_LABO_BAD;

    return ERR_OK;
}


// Creates all the interface when the object is selected.

bool CAutoLabo::CreateInterface(bool bSelect)
{
    Ui::CWindow*    pw;
    glm::vec2     pos, dim, ddim;
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
    if( !m_object->GetTrainer() )
    {
        pos.x = ox+sx*6.0f;
        pos.y = oy+sy*0.5f;
        pw->CreateButton(pos, dim, 192+8, EVENT_OBJECT_RTARGET);

        pos.x = ox+sx*7.0f;
        pos.y = oy+sy*0.5f;
        pw->CreateButton(pos, dim, 64+45, EVENT_OBJECT_RiPAW);

        pos.x = ox+sx*8.0f;
        pos.y = oy+sy*0.5f;
        pw->CreateButton(pos, dim, 64+46, EVENT_OBJECT_RiGUN);
    }

    pos.x = ox+sx*0.0f;
    pos.y = oy+sy*0;
    ddim.x = 66.0f/640.0f;
    ddim.y = 66.0f/480.0f;
    pw->CreateGroup(pos, ddim, 111, EVENT_OBJECT_TYPE);

    UpdateInterface();

    return true;
}

// Updates the status of all interface buttons.

void CAutoLabo::UpdateInterface()
{
    Ui::CWindow*    pw;

    if ( !m_object->GetSelect() )  return;

    CAuto::UpdateInterface();

    pw = static_cast< Ui::CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw == nullptr )  return;

    DeadInterface(pw, EVENT_OBJECT_RTARGET, m_main->IsResearchEnabled(RESEARCH_TARGET));
    DeadInterface(pw, EVENT_OBJECT_RiPAW,   m_main->IsResearchEnabled(RESEARCH_iPAW));
    DeadInterface(pw, EVENT_OBJECT_RiGUN,   m_main->IsResearchEnabled(RESEARCH_iGUN));

    OkayButton(pw, EVENT_OBJECT_RTARGET);
    OkayButton(pw, EVENT_OBJECT_RiPAW);
    OkayButton(pw, EVENT_OBJECT_RiGUN);

    VisibleInterface(pw, EVENT_OBJECT_RTARGET, !m_bBusy);
    VisibleInterface(pw, EVENT_OBJECT_RiPAW,   !m_bBusy);
    VisibleInterface(pw, EVENT_OBJECT_RiGUN,   !m_bBusy);
}

// Indicates the research conducted for a button.

void CAutoLabo::OkayButton(Ui::CWindow *pw, EventType event)
{
    Ui::CControl*   control;

    control = pw->SearchControl(event);
    if ( control == nullptr )  return;

    control->SetState(Ui::STATE_OKAY, TestResearch(event));
}


// Test whether a search has already been done.

bool CAutoLabo::TestResearch(EventType event)
{
    if ( event == EVENT_OBJECT_RTARGET )  return m_main->IsResearchDone(RESEARCH_TARGET, m_object->GetTeam());
    if ( event == EVENT_OBJECT_RiPAW   )  return m_main->IsResearchDone(RESEARCH_iPAW, m_object->GetTeam());
    if ( event == EVENT_OBJECT_RiGUN   )  return m_main->IsResearchDone(RESEARCH_iGUN, m_object->GetTeam());

    return m_main;
}

// Plays the sound of the manipulator arm.

void CAutoLabo::SoundManip(float time, float amplitude, float frequency)
{
    int     i;

    i = m_sound->Play(SOUND_MANIP, m_object->GetPosition(), 0.0f, 0.3f*frequency, true);
    m_sound->AddEnvelope(i, 0.5f*amplitude, 1.0f*frequency, 0.1f, SOPER_CONTINUE);
    m_sound->AddEnvelope(i, 0.5f*amplitude, 1.0f*frequency, time-0.1f, SOPER_CONTINUE);
    m_sound->AddEnvelope(i, 0.0f, 0.3f*frequency, 0.1f, SOPER_STOP);
}


// Saves all parameters of the controller.

bool CAutoLabo::Write(CLevelParserLine* line)
{
    if ( m_phase == ALAP_WAIT )  return false;

    line->AddParam("aExist", std::make_unique<CLevelParserParam>(true));
    CAuto::Write(line);
    line->AddParam("aPhase", std::make_unique<CLevelParserParam>(static_cast<int>(m_phase)));
    line->AddParam("aProgress", std::make_unique<CLevelParserParam>(m_progress));
    line->AddParam("aSpeed", std::make_unique<CLevelParserParam>(m_speed));
    line->AddParam("aResearch", std::make_unique<CLevelParserParam>(static_cast<int>(m_research)));

    return true;
}

// Restores all parameters of the controller.

bool CAutoLabo::Read(CLevelParserLine* line)
{
    if ( !line->GetParam("aExist")->AsBool(false) )  return false;

    CAuto::Read(line);
    m_phase = static_cast< AutoLaboPhase >(line->GetParam("aPhase")->AsInt(ALAP_WAIT));
    m_progress = line->GetParam("aProgress")->AsFloat(0.0f);
    m_speed = line->GetParam("aSpeed")->AsFloat(1.0f);
    m_research = static_cast< ResearchType >(line->GetParam("aResearch")->AsInt(0));

    m_lastParticle = 0.0f;

    return true;
}
