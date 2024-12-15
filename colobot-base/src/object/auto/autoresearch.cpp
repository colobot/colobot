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


#include "object/auto/autoresearch.h"

#include "common/global.h"

#include "graphics/engine/engine.h"

#include "level/robotmain.h"

#include "level/parser/parserline.h"
#include "level/parser/parserparam.h"

#include "math/geometry.h"

#include "object/old_object.h"

#include "object/interface/slotted_object.h"

#include "sound/sound.h"

#include "ui/controls/gauge.h"
#include "ui/controls/interface.h"
#include "ui/controls/window.h"


const float SEARCH_TIME = 30.0f;        // duration of a research



// Object's constructor.

CAutoResearch::CAutoResearch(COldObject* object) : CAuto(object)
{
    for (int i = 0; i < 6; i++)
    {
        m_partiStop[i] = -1;
    }
    m_channelSound = -1;

    Init();

    assert(m_object->GetNumSlots() == 1);
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


// Starts an action

Error CAutoResearch::StartAction(int param)
{
    if ( m_phase != ALP_WAIT )
    {
        return ERR_OBJ_BUSY;
    }

    m_research = static_cast<ResearchType>(param);

    if ( m_main->IsResearchDone(m_research, m_object->GetTeam()) )
    {
        return ERR_RESEARCH_ALREADY;
    }

    if (m_object->GetSlotContainedObject(0) == nullptr || !m_object->GetSlotContainedObject(0)->Implements(ObjectInterfaceType::PowerContainer))
    {
        return ERR_RESEARCH_POWER;
    }
    CPowerContainerObject* power = dynamic_cast<CPowerContainerObject*>(m_object->GetSlotContainedObject(0));
    if ( power->GetCapacity() > 1.0f )
    {
        return ERR_RESEARCH_TYPE;
    }
    if ( power->GetEnergy() < 1.0f )
    {
        return ERR_RESEARCH_ENERGY;
    }

    float time = SEARCH_TIME;
    if ( m_research == RESEARCH_TANK   )  time *= 0.3f;
    if ( m_research == RESEARCH_FLY    )  time *= 0.3f;
    if ( m_research == RESEARCH_ATOMIC )  time *= 2.0f;

    SetBusy(true);
    InitProgressTotal(time);
    UpdateInterface();

    m_channelSound = m_sound->Play(SOUND_RESEARCH, m_object->GetPosition(), 0.0f, 1.0f, true);
    m_sound->AddEnvelope(m_channelSound, 1.0f, 1.0f,      2.0f, SOPER_CONTINUE);
    m_sound->AddEnvelope(m_channelSound, 1.0f, 1.0f, time-4.0f, SOPER_CONTINUE);
    m_sound->AddEnvelope(m_channelSound, 0.0f, 1.0f,      2.0f, SOPER_STOP);

    m_phase    = ALP_SEARCH;
    m_progress = 0.0f;
    m_speed    = 1.0f/time;
    return ERR_OK;
}


// Management of an event.

bool CAutoResearch::EventProcess(const Event &event)
{
    CPowerContainerObject*    power;
    glm::vec3    pos, speed;
    Error       message;
    glm::vec2     dim;
    float       angle;

    CAuto::EventProcess(event);

    if ( m_engine->GetPause() )  return true;

    if ( event.type == EVENT_UPDINTERFACE )
    {
        if ( m_object->GetSelect() )  CreateInterface(true);
    }

    if ( m_object->GetSelect() )  // center selected?
    {
        Error err = ERR_UNKNOWN;
        if ( event.type == EVENT_OBJECT_RTANK   ) err = StartAction(RESEARCH_TANK);
        if ( event.type == EVENT_OBJECT_RFLY    ) err = StartAction(RESEARCH_FLY);
        if ( event.type == EVENT_OBJECT_RTHUMP  ) err = StartAction(RESEARCH_THUMP);
        if ( event.type == EVENT_OBJECT_RCANON  ) err = StartAction(RESEARCH_CANON);
        if ( event.type == EVENT_OBJECT_RTOWER  ) err = StartAction(RESEARCH_TOWER);
        if ( event.type == EVENT_OBJECT_RPHAZER ) err = StartAction(RESEARCH_PHAZER);
        if ( event.type == EVENT_OBJECT_RSHIELD ) err = StartAction(RESEARCH_SHIELD);
        if ( event.type == EVENT_OBJECT_RATOMIC ) err = StartAction(RESEARCH_ATOMIC);
        if ( event.type == EVENT_OBJECT_RBUILDER ) err = StartAction(RESEARCH_BUILDER);

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

    UpdateInterface(event.rTime);
    EventProgress(event.rTime);

    angle = m_time*0.1f;
    m_object->SetPartRotationY(1, angle);  // rotates the antenna

    angle = (30.0f+sinf(m_time*0.3f)*20.0f)*Math::PI/180.0f;
    m_object->SetPartRotationZ(2, angle);  // directs the antenna

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
            CObject* batteryObj = dynamic_cast<CSlottedObject&>(*m_object).GetSlotContainedObject(0);

            if ( batteryObj == nullptr || !batteryObj->Implements(ObjectInterfaceType::PowerContainer) )  // more battery?
            {
                SetBusy(false);
                UpdateInterface();

                m_phase    = ALP_WAIT;
                m_progress = 0.0f;
                m_speed    = 1.0f/1.0f;
                return true;
            }
            power = dynamic_cast<CPowerContainerObject*>(batteryObj);
            power->SetEnergyLevel(1.0f-m_progress);

            if ( m_lastParticle+m_engine->ParticleAdapt(0.05f) <= m_time )
            {
                m_lastParticle = m_time;

                pos = m_object->GetPosition();
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
            m_main->MarkResearchDone(m_research, m_object->GetTeam());  // research done

            m_eventQueue->AddEvent(Event(EVENT_UPDINTERFACE));
            UpdateInterface();

            m_main->DisplayError(INFO_RESEARCH, m_object);

            message = ERR_OK;
            if ( m_research == RESEARCH_TANK   )  message = INFO_RESEARCHTANK;
            if ( m_research == RESEARCH_FLY    )  message = INFO_RESEARCHFLY;
            if ( m_research == RESEARCH_THUMP  )  message = INFO_RESEARCHTHUMP;
            if ( m_research == RESEARCH_CANON  )  message = INFO_RESEARCHCANON;
            if ( m_research == RESEARCH_TOWER  )  message = INFO_RESEARCHTOWER;
            if ( m_research == RESEARCH_PHAZER )  message = INFO_RESEARCHPHAZER;
            if ( m_research == RESEARCH_SHIELD )  message = INFO_RESEARCHSHIELD;
            if ( m_research == RESEARCH_ATOMIC )  message = INFO_RESEARCHATOMIC;
            if ( m_research == RESEARCH_BUILDER )  message = INFO_RESEARCHBUILDER;
            if ( message != ERR_OK )
            {
                m_main->DisplayError(message, m_object);
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
    if ( m_phase == ALP_SEARCH )
    {
        return ERR_OK;
    }

    if ( m_object->GetVirusMode() )
    {
        return ERR_BAT_VIRUS;
    }

    if (m_object->GetSlotContainedObject(0) == nullptr || !m_object->GetSlotContainedObject(0)->Implements(ObjectInterfaceType::PowerContainer))
    {
        return ERR_RESEARCH_POWER;
    }
    CPowerContainerObject* power = dynamic_cast<CPowerContainerObject*>(m_object->GetSlotContainedObject(0));
    if ( power->GetCapacity() > 1.0f )
    {
        return ERR_RESEARCH_TYPE;
    }
    if ( power->GetEnergy() < 1.0f )
    {
        return ERR_RESEARCH_ENERGY;
    }

    return ERR_OK;
}


// Creates all the interface when the object is selected.

bool CAutoResearch::CreateInterface(bool bSelect)
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
        pos.x = ox+sx*3.0f;
        pos.y = oy+sy*0.5f;
        pw->CreateButton(pos, dim, 64+0, EVENT_OBJECT_RTANK);

        pos.x = ox+sx*4.0f;
        pos.y = oy+sy*0.5f;
        pw->CreateButton(pos, dim, 64+1, EVENT_OBJECT_RFLY);

        pos.x = ox+sx*5.0f;
        pos.y = oy+sy*0.5f;
        pw->CreateButton(pos, dim, 64+3, EVENT_OBJECT_RCANON);

        pos.x = ox+sx*6.0f;
        pos.y = oy+sy*0.5f;
        pw->CreateButton(pos, dim, 64+4, EVENT_OBJECT_RTOWER);

        pos.x = ox+sx*7.0f;
        pos.y = oy+sy*0.5f;
        pw->CreateButton(pos, dim, 64+7, EVENT_OBJECT_RATOMIC);

        pos.x = ox+sx*8.0f;
        pos.y = oy+sy*0.5f;
        pw->CreateButton(pos, dim, 64+2, EVENT_OBJECT_RTHUMP);

        pos.x = ox+sx*9.0f;
        pos.y = oy+sy*0.5f;
        pw->CreateButton(pos, dim, 64+6, EVENT_OBJECT_RSHIELD);

        pos.x = ox+sx*10.0f;
        pos.y = oy+sy*0.5f;
        pw->CreateButton(pos, dim, 64+5, EVENT_OBJECT_RPHAZER);

        pos.x = ox+sx*11.0f;
        pos.y = oy+sy*0.5f;
        pw->CreateButton(pos, dim, 192+4, EVENT_OBJECT_RBUILDER);
    }

    pos.x = ox+sx*14.5f;
    pos.y = oy+sy*0;
    ddim.x = 14.0f/640.0f;
    ddim.y = 66.0f/480.0f;
    Ui::CGauge* pg = pw->CreateGauge(pos, ddim, 0, EVENT_OBJECT_GENERGY);
    pg->SetLevel(GetObjectEnergy(m_object));

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
    if ( pw == nullptr )  return;

    DeadInterface(pw, EVENT_OBJECT_RTANK,   m_main->IsResearchEnabled(RESEARCH_TANK));
    DeadInterface(pw, EVENT_OBJECT_RFLY,    m_main->IsResearchEnabled(RESEARCH_FLY));
    DeadInterface(pw, EVENT_OBJECT_RTHUMP,  m_main->IsResearchEnabled(RESEARCH_THUMP));
    DeadInterface(pw, EVENT_OBJECT_RCANON,  m_main->IsResearchEnabled(RESEARCH_CANON));
    DeadInterface(pw, EVENT_OBJECT_RTOWER,  m_main->IsResearchEnabled(RESEARCH_TOWER));
    DeadInterface(pw, EVENT_OBJECT_RPHAZER, m_main->IsResearchEnabled(RESEARCH_PHAZER));
    DeadInterface(pw, EVENT_OBJECT_RSHIELD, m_main->IsResearchEnabled(RESEARCH_SHIELD));
    DeadInterface(pw, EVENT_OBJECT_RATOMIC, m_main->IsResearchEnabled(RESEARCH_ATOMIC));
    DeadInterface(pw, EVENT_OBJECT_RBUILDER, m_main->IsResearchEnabled(RESEARCH_BUILDER));

    OkayButton(pw, EVENT_OBJECT_RTANK);
    OkayButton(pw, EVENT_OBJECT_RFLY);
    OkayButton(pw, EVENT_OBJECT_RTHUMP);
    OkayButton(pw, EVENT_OBJECT_RCANON);
    OkayButton(pw, EVENT_OBJECT_RTOWER);
    OkayButton(pw, EVENT_OBJECT_RPHAZER);
    OkayButton(pw, EVENT_OBJECT_RSHIELD);
    OkayButton(pw, EVENT_OBJECT_RATOMIC);
    OkayButton(pw, EVENT_OBJECT_RBUILDER);

    VisibleInterface(pw, EVENT_OBJECT_RTANK,   !m_bBusy);
    VisibleInterface(pw, EVENT_OBJECT_RFLY,    !m_bBusy);
    VisibleInterface(pw, EVENT_OBJECT_RTHUMP,  !m_bBusy);
    VisibleInterface(pw, EVENT_OBJECT_RCANON,  !m_bBusy);
    VisibleInterface(pw, EVENT_OBJECT_RTOWER,  !m_bBusy);
    VisibleInterface(pw, EVENT_OBJECT_RPHAZER, !m_bBusy);
    VisibleInterface(pw, EVENT_OBJECT_RSHIELD, !m_bBusy);
    VisibleInterface(pw, EVENT_OBJECT_RATOMIC, !m_bBusy);
    VisibleInterface(pw, EVENT_OBJECT_RBUILDER, !m_bBusy);
}

// Updates the state of all buttons on the interface,
// following the time that elapses ...

void CAutoResearch::UpdateInterface(float rTime)
{
    CAuto::UpdateInterface(rTime);

    if ( m_time < m_lastUpdateTime+0.1f )  return;
    m_lastUpdateTime = m_time;

    if ( !m_object->GetSelect() )  return;

    Ui::CWindow* pw = static_cast< Ui::CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw == nullptr )  return;

    Ui::CGauge* pg = static_cast< Ui::CGauge* >(pw->SearchControl(EVENT_OBJECT_GENERGY));
    if ( pg != nullptr )
    {
        float energy = GetObjectEnergy(m_object);
        pg->SetLevel(energy);
    }
}

// Research shows already performed button.

void CAutoResearch::OkayButton(Ui::CWindow *pw, EventType event)
{
    Ui::CControl*   control;

    control = pw->SearchControl(event);
    if ( control == nullptr )  return;

    control->SetState(Ui::STATE_OKAY, TestResearch(event));
}


// Test whether a search has already been done.

bool CAutoResearch::TestResearch(EventType event)
{
    if ( event == EVENT_OBJECT_RTANK   )  return m_main->IsResearchDone(RESEARCH_TANK, m_object->GetTeam());
    if ( event == EVENT_OBJECT_RFLY    )  return m_main->IsResearchDone(RESEARCH_FLY, m_object->GetTeam());
    if ( event == EVENT_OBJECT_RTHUMP  )  return m_main->IsResearchDone(RESEARCH_THUMP, m_object->GetTeam());
    if ( event == EVENT_OBJECT_RCANON  )  return m_main->IsResearchDone(RESEARCH_CANON, m_object->GetTeam());
    if ( event == EVENT_OBJECT_RTOWER  )  return m_main->IsResearchDone(RESEARCH_TOWER, m_object->GetTeam());
    if ( event == EVENT_OBJECT_RPHAZER )  return m_main->IsResearchDone(RESEARCH_PHAZER, m_object->GetTeam());
    if ( event == EVENT_OBJECT_RSHIELD )  return m_main->IsResearchDone(RESEARCH_SHIELD, m_object->GetTeam());
    if ( event == EVENT_OBJECT_RATOMIC )  return m_main->IsResearchDone(RESEARCH_ATOMIC, m_object->GetTeam());
    if ( event == EVENT_OBJECT_RBUILDER )  return m_main->IsResearchDone(RESEARCH_BUILDER, m_object->GetTeam());

    return false;
}


// Updates the stop lights.

void CAutoResearch::FireStopUpdate(float progress, bool bLightOn)
{
    glm::vec3    pos, speed;
    glm::vec2     dim;
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

    if ( !bLightOn )  // light-off ?
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

    glm::mat4 mat = m_object->GetWorldMatrix(0);

    speed = glm::vec3(0.0f, 0.0f, 0.0f);
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
                pos = Math::Transform(mat, pos);
                m_partiStop[i] = m_particle->CreateParticle(pos, speed,
                                                              dim, Gfx::PARTISELY,
                                                              1.0f, 0.0f, 0.0f);
            }
        }
    }
}


// Saves all parameters of the controller.

bool CAutoResearch::Write(CLevelParserLine* line)
{
    if ( m_phase == ALP_WAIT )  return false;

    line->AddParam("aExist", std::make_unique<CLevelParserParam>(true));
    CAuto::Write(line);
    line->AddParam("aPhase", std::make_unique<CLevelParserParam>(static_cast<int>(m_phase)));
    line->AddParam("aProgress", std::make_unique<CLevelParserParam>(m_progress));
    line->AddParam("aSpeed", std::make_unique<CLevelParserParam>(m_speed));
    line->AddParam("aResearch", std::make_unique<CLevelParserParam>(static_cast<int>(m_research)));

    return true;
}

// Restores all parameters of the controller.

bool CAutoResearch::Read(CLevelParserLine* line)
{
    if ( !line->GetParam("aExist")->AsBool(false) )  return false;

    CAuto::Read(line);
    m_phase = static_cast< AutoResearchPhase >(line->GetParam("aPhase")->AsInt(ALP_WAIT));
    m_progress = line->GetParam("aProgress")->AsFloat(0.0f);
    m_speed = line->GetParam("aSpeed")->AsFloat(1.0f);
    m_research = static_cast< ResearchType >(line->GetParam("aResearch")->AsInt(0));

    m_lastUpdateTime = 0.0f;
    m_lastParticle = 0.0f;

    return true;
}
