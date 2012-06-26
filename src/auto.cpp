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
#include "water.h"
#include "cloud.h"
#include "planet.h"
#include "blitz.h"
#include "camera.h"
#include "object.h"
#include "modfile.h"
#include "interface.h"
#include "button.h"
#include "list.h"
#include "label.h"
#include "gauge.h"
#include "window.h"
#include "robotmain.h"
#include "displaytext.h"
#include "sound.h"
#include "cmdtoken.h"
#include "auto.h"




// Object's constructor.

CAuto::CAuto(CInstanceManager* iMan, CObject* object)
{
    m_iMan = iMan;
    m_iMan->AddInstance(CLASS_AUTO, this, 100);

    m_object      = object;
    m_event       = (CEvent*)m_iMan->SearchInstance(CLASS_EVENT);
    m_engine      = (CD3DEngine*)m_iMan->SearchInstance(CLASS_ENGINE);
    m_particule   = (CParticule*)m_iMan->SearchInstance(CLASS_PARTICULE);
    m_light       = (CLight*)m_iMan->SearchInstance(CLASS_LIGHT);
    m_terrain     = (CTerrain*)m_iMan->SearchInstance(CLASS_TERRAIN);
    m_water       = (CWater*)m_iMan->SearchInstance(CLASS_WATER);
    m_cloud       = (CCloud*)m_iMan->SearchInstance(CLASS_CLOUD);
    m_planet      = (CPlanet*)m_iMan->SearchInstance(CLASS_PLANET);
    m_blitz       = (CBlitz*)m_iMan->SearchInstance(CLASS_BLITZ);
    m_camera      = (CCamera*)m_iMan->SearchInstance(CLASS_CAMERA);
    m_interface   = (CInterface*)m_iMan->SearchInstance(CLASS_INTERFACE);
    m_main        = (CRobotMain*)m_iMan->SearchInstance(CLASS_MAIN);
    m_displayText = (CDisplayText*)m_iMan->SearchInstance(CLASS_DISPLAYTEXT);
    m_sound       = (CSound*)m_iMan->SearchInstance(CLASS_SOUND);

    m_type = m_object->RetType();
    m_time = 0.0f;
    m_lastUpdateTime = 0.0f;
    m_bMotor = FALSE;
    m_progressTime = 0.0f;
    m_progressTotal = 1.0f;

    Init();
}

// Object's destructor.

CAuto::~CAuto()
{
    m_iMan->DeleteInstance(CLASS_AUTO, this);
}


// Destroys the object.

void CAuto::DeleteObject(BOOL bAll)
{
}


// Initialize the object.

void CAuto::Init()
{
    m_bBusy = FALSE;
}

// Starts the object.

void CAuto::Start(int param)
{
}


// Give a type.

BOOL CAuto::SetType(ObjectType type)
{
    return FALSE;
}

// Gives a value.

BOOL CAuto::SetValue(int rank, float value)
{
    return FALSE;
}

// Gives the string.

BOOL CAuto::SetString(char *string)
{
    return FALSE;
}


// Management of an event.

BOOL CAuto::EventProcess(const Event &event)
{
    if ( event.event == EVENT_FRAME &&
         !m_engine->RetPause() )
    {
        m_time += event.rTime;
        UpdateInterface(event.rTime);
    }

    if ( !m_object->RetSelect() )  // robot not selected?
    {
        return TRUE;
    }

    return TRUE;
}

// Indicates whether the controller has finished its activity.

Error CAuto::IsEnded()
{
    return ERR_CONTINUE;
}

// Stops the controller

BOOL CAuto::Abort()
{
    return FALSE;
}


// Creates all the interface when the object is selected.

BOOL CAuto::CreateInterface(BOOL bSelect)
{
    CWindow*    pw;
    FPOINT      pos, dim, ddim;
    float       ox, oy, sx, sy;
    char        name[100];

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW0);
    if ( pw != 0 )
    {
        pw->Flush();  // destroys the window buttons
        m_interface->DeleteControl(EVENT_WINDOW0);  // destroys the window
    }

    if ( !bSelect )  return TRUE;

    pos.x = 0.0f;
    pos.y = 0.0f;
    dim.x = 540.0f/640.0f;
//? dim.y = 70.0f/480.0f;
    dim.y = 86.0f/480.0f;
    m_interface->CreateWindows(pos, dim, 3, EVENT_WINDOW0);
    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW0);
    if ( pw == 0 )  return FALSE;

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

    if ( m_main->RetSceneSoluce() )
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

    return TRUE;
}

// Change the state of a button interface.

void CAuto::CheckInterface(CWindow *pw, EventMsg event, BOOL bState)
{
    CControl*   control;

    control = pw->SearchControl(event);
    if ( control == 0 )  return;

    control->SetState(STATE_CHECK, bState);
}

// Change the state of a button interface.

void CAuto::EnableInterface(CWindow *pw, EventMsg event, BOOL bState)
{
    CControl*   control;

    control = pw->SearchControl(event);
    if ( control == 0 )  return;

    control->SetState(STATE_ENABLE, bState);
}

// Change the state of a button interface.

void CAuto::VisibleInterface(CWindow *pw, EventMsg event, BOOL bState)
{
    CControl*   control;

    control = pw->SearchControl(event);
    if ( control == 0 )  return;

    control->SetState(STATE_VISIBLE, bState);
}

// Change the state of a button interface.

void CAuto::DeadInterface(CWindow *pw, EventMsg event, BOOL bState)
{
    CControl*   control;

    control = pw->SearchControl(event);
    if ( control == 0 )  return;

    control->SetState(STATE_DEAD, !bState);
}

// Change the state of a button interface.

void CAuto::UpdateInterface()
{
    CWindow*    pw;

    if ( !m_object->RetSelect() )  return;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW0);
    if ( pw == 0 )  return;

    VisibleInterface(pw, EVENT_OBJECT_GPROGRESS, m_bBusy);
}

// Updates the state of all buttons on the interface,
// following the time that elapses ...

void CAuto::UpdateInterface(float rTime)
{
    CWindow*    pw;
    CGauge*     pg;

    if ( m_time < m_lastUpdateTime+0.1f )  return;
    m_lastUpdateTime = m_time;

    if ( !m_object->RetSelect() )  return;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW0);
    if ( pw == 0 )  return;

    pg = (CGauge*)pw->SearchControl(EVENT_OBJECT_GSHIELD);
    if ( pg != 0 )
    {
        pg->SetLevel(m_object->RetShield());
    }

    pg = (CGauge*)pw->SearchControl(EVENT_OBJECT_GPROGRESS);
    if ( pg != 0 )
    {
        pg->SetLevel(m_progressTime);
    }
}


// Returns an error due the state of the automation.

Error CAuto::RetError()
{
    return ERR_OK;
}


// Management of the occupation.

BOOL CAuto::RetBusy()
{
    return m_bBusy;
}

void CAuto::SetBusy(BOOL bBusy)
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

BOOL CAuto::RetMotor()
{
    return m_bMotor;
}

void CAuto::SetMotor(BOOL bMotor)
{
    m_bMotor = bMotor;
}


// Saves all parameters of the controller.

BOOL CAuto::Write(char *line)
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

    return FALSE;
}

// Return all settings to the controller.

BOOL CAuto::Read(char *line)
{
    m_type = (ObjectType)OpInt(line, "aType", OBJECT_NULL);
    m_bBusy = OpInt(line, "aBusy", 0);
    m_time = OpFloat(line, "aTime", 0.0f);
    m_progressTime = OpFloat(line, "aProgressTime", 0.0f);
    m_progressTotal = OpFloat(line, "aProgressTotal", 0.0f);

    return FALSE;
}

