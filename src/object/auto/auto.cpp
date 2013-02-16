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


#include "object/auto/auto.h"


#include "app/app.h"

#include "common/event.h"
#include "common/iman.h"

#include "script/cmdtoken.h"

#include "ui/interface.h"
#include "ui/gauge.h"
#include "ui/window.h"

#include <stdio.h>
#include <string.h>


// Object's constructor.

CAuto::CAuto(CObject* object)
{
    m_iMan = CInstanceManager::GetInstancePointer();

    m_object      = object;
    m_engine      = Gfx::CEngine::GetInstancePointer();
    m_main        = CRobotMain::GetInstancePointer();
    m_eventQueue  = CApplication::GetInstancePointer()->GetEventQueue();
    m_sound       = CApplication::GetInstancePointer()->GetSound();
    m_particle    = m_engine->GetParticle();
    m_terrain     = m_engine->GetTerrain();
    m_water       = m_engine->GetWater();
    m_cloud       = m_engine->GetCloud();
    m_planet      = m_engine->GetPlanet();
    m_lightning   = m_engine->GetLightning();
    m_camera      = m_main->GetCamera();
    m_interface   = m_main->GetInterface();
    m_displayText = m_main->GetDisplayText();

    m_type = m_object->GetType();
    m_time = 0.0f;
    m_lastUpdateTime = 0.0f;
    m_bMotor = false;
    m_progressTime = 0.0f;
    m_progressTotal = 1.0f;

    Init();
}

// Object's destructor.

CAuto::~CAuto()
{
    m_iMan = nullptr;

    m_object      = nullptr;
    m_engine      = nullptr;
    m_main        = nullptr;
    m_eventQueue  = nullptr;
    m_sound       = nullptr;
    m_particle    = nullptr;
    m_terrain     = nullptr;
    m_water       = nullptr;
    m_cloud       = nullptr;
    m_planet      = nullptr;
    m_lightning   = nullptr;
    m_camera      = nullptr;
    m_interface   = nullptr;
    m_displayText = nullptr;
}


// Destroys the object.

void CAuto::DeleteObject(bool bAll)
{
}


// Initialize the object.

void CAuto::Init()
{
    m_bBusy = false;
}

// Starts the object.

void CAuto::Start(int param)
{
}


// Gete a type.

bool CAuto::SetType(ObjectType type)
{
    return false;
}

// Getes a value.

bool CAuto::SetValue(int rank, float value)
{
    return false;
}

// Getes the string.

bool CAuto::SetString(char *string)
{
    return false;
}


// Management of an event.

bool CAuto::EventProcess(const Event &event)
{
    if ( event.type == EVENT_FRAME &&
         !m_engine->GetPause() )
    {
        m_time += event.rTime;
        UpdateInterface(event.rTime);
    }

    if ( !m_object->GetSelect() )  // robot not selected?
    {
        return true;
    }

    return true;
}

// Indicates whether the controller has finished its activity.

Error CAuto::IsEnded()
{
    return ERR_CONTINUE;
}

// Stops the controller

bool CAuto::Abort()
{
    return false;
}


// Creates all the interface when the object is selected.

bool CAuto::CreateInterface(bool bSelect)
{
    Ui::CWindow*    pw;
    Math::Point     pos, dim, ddim;
    float       ox, oy, sx, sy;
    char        name[100];

    pw = static_cast<Ui::CWindow*>(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw != nullptr )
    {
        pw->Flush();  // destroys the window buttons
        m_interface->DeleteControl(EVENT_WINDOW0);  // destroys the window
    }

    if ( !bSelect )  return true;

    pos.x = 0.0f;
    pos.y = 0.0f;
    dim.x = 540.0f/640.0f;
//? dim.y = 70.0f/480.0f;
    dim.y = 86.0f/480.0f;
    m_interface->CreateWindows(pos, dim, 3, EVENT_WINDOW0);
    pw = static_cast<Ui::CWindow*>(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw == 0 )  return false;

    m_object->GetTooltipName(name);
    pos.x = 0.0f;
    pos.y = 64.0f/480.0f;
    ddim.x = 540.0f/640.0f;
    ddim.y = 16.0f/480.0f;
    pw->CreateLabel(pos, ddim, 0, EVENT_LABEL0, name);

    dim.x = 33.0f/640.0f;
    dim.y = 33.0f/480.0f;
    ox = 3.0f/640.0f;
    oy = 3.0f/480.0f;
    sx = 33.0f/640.0f;
    sy = 33.0f/480.0f;

    pos.x = ox+sx*7.0f;
    pos.y = oy+sy*0.6f;
    ddim.x = 160.0f/640.0f;
    ddim.y =  26.0f/480.0f;
    pw->CreateGauge(pos, ddim, 0, EVENT_OBJECT_GPROGRESS);

    if ( m_type != OBJECT_BASE   &&
         m_type != OBJECT_SAFE   &&
         m_type != OBJECT_HUSTON )
    {
        pos.x = ox+sx*2.1f;
        pos.y = oy+sy*0;
        ddim.x = dim.x*0.6f;
        ddim.y = dim.y*0.6f;
        pw->CreateButton(pos, ddim, 12, EVENT_OBJECT_DELETE);
    }

#if 0
    pos.x = ox+sx*12.4f;
    pos.y = oy+sy*1;
    pw->CreateButton(pos, dim, 63, EVENT_OBJECT_BHELP);

    pos.x = ox+sx*12.4f;
    pos.y = oy+sy*0;
    pw->CreateButton(pos, dim, 19, EVENT_OBJECT_HELP);

    if ( m_main->GetSceneSoluce() )
    {
        pos.x = ox+sx*13.4f;
        pos.y = oy+sy*1;
        pw->CreateButton(pos, dim, 20, EVENT_OBJECT_SOLUCE);
    }

    pos.x = ox+sx*13.4f;
    pos.y = oy+sy*0;
    pw->CreateButton(pos, dim, 10, EVENT_OBJECT_DESELECT);
#else
    pos.x = ox+sx*12.3f;
    pos.y = oy+sy*-0.1f;
    ddim.x = dim.x*1.0f;
    ddim.y = dim.y*2.1f;
    pw->CreateGroup(pos, ddim, 20, EVENT_NULL);  // solid blue background

    pos.x = ox+sx*12.3f;
    pos.y = oy+sy*1;
    pw->CreateGroup(pos, dim, 19, EVENT_NULL);  // sign SatCom

    pos.x = ox+sx*12.4f;
    pos.y = oy+sy*0.5f;
    ddim.x = dim.x*0.8f;
    ddim.y = dim.y*0.5f;
    pw->CreateButton(pos, ddim, 18, EVENT_OBJECT_BHELP);
    pos.y = oy+sy*0.0f;
    pw->CreateButton(pos, ddim, 19, EVENT_OBJECT_HELP);

    pos.x = ox+sx*13.4f;
    pos.y = oy+sy*0;
    pw->CreateButton(pos, dim, 10, EVENT_OBJECT_DESELECT);
#endif

    pos.x = ox+sx*14.9f;
    pos.y = oy+sy*0;
    ddim.x = 14.0f/640.0f;
    ddim.y = 66.0f/480.0f;
    pw->CreateGauge(pos, ddim, 3, EVENT_OBJECT_GSHIELD);

    UpdateInterface();
    m_lastUpdateTime = 0.0f;
    UpdateInterface(0.0f);

    return true;
}

// Change the state of a button interface.

void CAuto::CheckInterface(Ui::CWindow *pw, EventType event, bool bState)
{
    Ui::CControl*   control;

    control = pw->SearchControl(event);
    if ( control == nullptr )  return;

    control->SetState(Ui::STATE_CHECK, bState);
}

// Change the state of a button interface.

void CAuto::EnableInterface(Ui::CWindow *pw, EventType event, bool bState)
{
    Ui::CControl*   control;

    control = pw->SearchControl(event);
    if ( control == nullptr )  return;

    control->SetState(Ui::STATE_ENABLE, bState);
}

// Change the state of a button interface.

void CAuto::VisibleInterface(Ui::CWindow *pw, EventType event, bool bState)
{
    Ui::CControl*   control;

    control = pw->SearchControl(event);
    if ( control == nullptr )  return;

    control->SetState(Ui::STATE_VISIBLE, bState);
}

// Change the state of a button interface.

void CAuto::DeadInterface(Ui::CWindow *pw, EventType event, bool bState)
{
    Ui::CControl*   control;

    control = pw->SearchControl(event);
    if ( control == nullptr )  return;

    control->SetState(Ui::STATE_DEAD, !bState);
}

// Change the state of a button interface.

void CAuto::UpdateInterface()
{
    Ui::CWindow*    pw;

    if ( !m_object->GetSelect() )  return;

    pw = static_cast<Ui::CWindow*>(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw == nullptr )  return;

    VisibleInterface(pw, EVENT_OBJECT_GPROGRESS, m_bBusy);
}

// Updates the state of all buttons on the interface,
// following the time that elapses ...

void CAuto::UpdateInterface(float rTime)
{
    Ui::CWindow*    pw;
    Ui::CGauge*     pg;

    if ( m_time < m_lastUpdateTime+0.1f )  return;
    m_lastUpdateTime = m_time;

    if ( !m_object->GetSelect() )  return;

    pw = static_cast<Ui::CWindow*>(m_interface->SearchControl(EVENT_WINDOW0));
    if ( pw == nullptr )  return;

    pg = static_cast<Ui::CGauge*>(pw->SearchControl(EVENT_OBJECT_GSHIELD));
    if ( pg != nullptr )
    {
        pg->SetLevel(m_object->GetShield());
    }

    pg = static_cast<Ui::CGauge*>(pw->SearchControl(EVENT_OBJECT_GPROGRESS));
    if ( pg != nullptr )
    {
        pg->SetLevel(m_progressTime);
    }
}


// Returns an error due the state of the automation.

Error CAuto::GetError()
{
    return ERR_OK;
}


// Management of the occupation.

bool CAuto::GetBusy()
{
    return m_bBusy;
}

void CAuto::SetBusy(bool bBusy)
{
    m_bBusy = bBusy;
}

void CAuto::InitProgressTotal(float total)
{
    m_progressTime = 0.0f;
    m_progressTotal = total;
}

void CAuto::EventProgress(float rTime)
{
    m_progressTime += rTime/m_progressTotal;
}


// Engine management.

bool CAuto::GetMotor()
{
    return m_bMotor;
}

void CAuto::SetMotor(bool bMotor)
{
    m_bMotor = bMotor;
}


// Saves all parameters of the controller.

bool CAuto::Write(char *line)
{
    char    name[100];

    sprintf(name, " aType=%d", m_type);
    strcat(line, name);

    sprintf(name, " aBusy=%d", m_bBusy);
    strcat(line, name);

    sprintf(name, " aTime=%.2f", m_time);
    strcat(line, name);

    sprintf(name, " aProgressTime=%.2f", m_progressTime);
    strcat(line, name);

    sprintf(name, " aProgressTotal=%.2f", m_progressTotal);
    strcat(line, name);

    return false;
}

// Return all settings to the controller.

bool CAuto::Read(char *line)
{
    m_type = static_cast<ObjectType>(OpInt(line, "aType", OBJECT_NULL));
    m_bBusy = OpInt(line, "aBusy", 0);
    m_time = OpFloat(line, "aTime", 0.0f);
    m_progressTime = OpFloat(line, "aProgressTime", 0.0f);
    m_progressTotal = OpFloat(line, "aProgressTotal", 0.0f);

    return false;
}

