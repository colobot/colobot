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


#include "object/auto/autodestroyer.h"

#include "graphics/engine/engine.h"
#include "graphics/engine/pyro_manager.h"

#include "level/robotmain.h"

#include "level/parser/parserline.h"
#include "level/parser/parserparam.h"

#include "math/func.h"

#include "object/object_manager.h"
#include "object/old_object.h"

#include "sound/sound.h"

#include "ui/controls/interface.h"
#include "ui/controls/window.h"

#include <limits>


// Object's constructor.

CAutoDestroyer::CAutoDestroyer(COldObject* object) : CAuto(object)
{
    Init();
    m_phase = ADEP_WAIT;  // paused until the first Init ()
}

// Destructive of the object.

CAutoDestroyer::~CAutoDestroyer()
{
}


// Destroys the object.

void CAutoDestroyer::DeleteObject(bool bAll)
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
    m_lastParticle = 0.0f;

    CAuto::Init();
}


// Starts an action
Error CAutoDestroyer::StartAction(int param)
{
    CObject* scrap;

    if ( m_object->GetVirusMode() )  // contaminated by a virus?
    {
        return ERR_BAT_VIRUS;
    }

    scrap = SearchPlastic();
    if ( scrap == nullptr )
        return ERR_DESTROY_NOTFOUND;
    else
    {
        if ( m_phase == ADEP_WAIT )
        {
            scrap->SetLock(true);  // usable waste
//?         scrap->SetTransporter(m_object);  // usable waste

            m_sound->Play(SOUND_PSHHH2, m_object->GetPosition(), 1.0f, 1.0f);

            m_phase    = ADEP_DOWN;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.0f;
            m_bExplo   = false;
        }
        else
            return ERR_OBJ_BUSY;
    }
    return ERR_OK;
}

// Management of an event.

bool CAutoDestroyer::EventProcess(const Event &event)
{
    CObject*        scrap;
    glm::vec3    pos;
    Ui::CWindow*    pw;

    CAuto::EventProcess(event);

    if ( m_engine->GetPause() )  return true;

    if (m_main->GetSelect() == m_object)
    {
        if ( event.type == EVENT_OBJECT_BDESTROY )
        {
            Error err = StartAction(0);
            if ( err != ERR_OK )
                m_main->DisplayError(err, m_object);

            return false;
        }
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

    pw = static_cast< Ui::CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
    if ( m_phase == ADEP_WAIT )
    {
        if ( m_progress >= 1.0f )
        {
            m_phase    = ADEP_WAIT;  // still waiting ...
            m_progress = 0.0f;
            m_speed    = 1.0f/0.5f;
            if (m_main->GetSelect() == m_object)
            {
                scrap = SearchPlastic();
                if ( pw != nullptr ) EnableInterface(pw, EVENT_OBJECT_BDESTROY, (scrap != nullptr));
            }
        }
    }
    else if ( pw != nullptr ) EnableInterface(pw, EVENT_OBJECT_BDESTROY, false);

    if ( m_phase == ADEP_DOWN )
    {
        if ( m_progress >= 0.3f-0.05f && !m_bExplo )
        {
            scrap = SearchPlastic();
            if ( scrap != nullptr )
            {
                assert(scrap->Implements(ObjectInterfaceType::Destroyable));
                dynamic_cast<CDestroyableObject&>(*scrap).DestroyObject(DestructionType::Explosion);
            }
            m_bExplo = true;
        }

        if ( m_progress < 1.0f )
        {
            pos = glm::vec3(0.0f, -10.0f, 0.0f);
            pos.y = -Math::Bounce(m_progress, 0.3f)*10.0f;
            m_object->SetPartPosition(1, pos);
        }
        else
        {
            m_object->SetPartPosition(1, glm::vec3(0.0f, -10.0f, 0.0f));
            m_sound->Play(SOUND_REPAIR, m_object->GetPosition());

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
            m_sound->Play(SOUND_OPEN, m_object->GetPosition(), 1.0f, 0.8f);

            m_phase    = ADEP_UP;
            m_progress = 0.0f;
            m_speed    = 1.0f/3.0f;
        }
    }

    if ( m_phase == ADEP_UP )
    {
        if ( m_progress < 1.0f )
        {
            pos = glm::vec3(0.0f, -10.0f, 0.0f);
            pos.y = -(1.0f-m_progress)*10.0f;
            m_object->SetPartPosition(1, pos);
        }
        else
        {
            m_object->SetPartPosition(1, glm::vec3(0.0f, 0.0f, 0.0f));

            m_phase    = ADEP_WAIT;
            m_progress = 0.0f;
            m_speed    = 1.0f/0.5f;
        }
    }

    return true;
}


// Creates all the interface when the object is selected.

bool CAutoDestroyer::CreateInterface(bool bSelect)
{
    Ui::CWindow*    pw;
    glm::vec2     pos, ddim;
    float       ox, oy, sx, sy;
    CObject*        scrap;

    CAuto::CreateInterface(bSelect);

    if ( !bSelect )  return true;

    pw = static_cast< Ui::CWindow* >(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw == nullptr )  return false;

    ox = 3.0f/640.0f;
    oy = 3.0f/480.0f;
    sx = 33.0f/640.0f;
    sy = 33.0f/480.0f;

    pos.x = ox+sx*0.0f;
    pos.y = oy+sy*0;
    ddim.x = 66.0f/640.0f;
    ddim.y = 66.0f/480.0f;
    pw->CreateGroup(pos, ddim, 106, EVENT_OBJECT_TYPE);

    pos.x = ox+sx*8.00f;
    pos.y = oy+sy*0.25f;
    ddim.x = (33.0f/640.0f)*1.5f;
    ddim.y = (33.0f/480.0f)*1.5f;
    pw->CreateButton(pos, ddim, 12, EVENT_OBJECT_BDESTROY);

    scrap = SearchPlastic();
    EnableInterface(pw, EVENT_OBJECT_BDESTROY, (scrap != nullptr));

    return true;
}


// Seeks plate waste in the destroyer.

CObject* CAutoDestroyer::SearchPlastic()
{
    glm::vec3 sPos = m_object->GetPosition();

    for (CObject* obj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        if (obj == m_object) continue;
        if (!obj->Implements(ObjectInterfaceType::Destroyable)) continue;
        if (obj->GetType() == OBJECT_HUMAN || obj->GetType() == OBJECT_TECH) continue;

        glm::vec3 oPos = obj->GetPosition();
        float dist = glm::distance(oPos, sPos);
        if ( dist <= 5.0f )  return obj;
    }

    return nullptr;
}

// Returns an error due the state of the automation.

Error CAutoDestroyer::GetError()
{
    if ( m_object->GetVirusMode() )
    {
        return ERR_BAT_VIRUS;
    }

    return ERR_OK;
}


// Saves all parameters of the controller.

bool CAutoDestroyer::Write(CLevelParserLine* line)
{
    if ( m_phase == ADEP_WAIT )  return false;

    line->AddParam("aExist", std::make_unique<CLevelParserParam>(true));
    CAuto::Write(line);
    line->AddParam("aPhase", std::make_unique<CLevelParserParam>(static_cast<int>(m_phase)));
    line->AddParam("aProgress", std::make_unique<CLevelParserParam>(m_progress));
    line->AddParam("aSpeed", std::make_unique<CLevelParserParam>(m_speed));

    return true;
}

// Restores all parameters of the controller.

bool CAutoDestroyer::Read(CLevelParserLine* line)
{
    if ( !line->GetParam("aExist")->AsBool(false) )  return false;

    CAuto::Read(line);
    m_phase = static_cast< AutoDestroyerPhase >(line->GetParam("aPhase")->AsInt(ADEP_WAIT));
    m_progress = line->GetParam("aProgress")->AsFloat(0.0f);
    m_speed = line->GetParam("aSpeed")->AsFloat(1.0f);

    m_lastParticle = 0.0f;

    return true;
}

// Changes the state of a button interface.

void CAutoDestroyer::EnableInterface(Ui::CWindow *pw, EventType event, bool bState)
{
    Ui::CControl*   control;

    control = pw->SearchControl(event);
    if ( control == nullptr )  return;

    control->SetState(Ui::STATE_ENABLE, bState);
}
