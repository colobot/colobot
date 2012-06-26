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
#include "math3d.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "restext.h"
#include "text.h"
#include "check.h"




// Object's constructor.

CCheck::CCheck(CInstanceManager* iMan) : CControl(iMan)
{
}

// Object's destructor.

CCheck::~CCheck()
{
}


// Creates a new button.

BOOL CCheck::Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
    char    name[100];
    char*   p;

    if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

    CControl::Create(pos, dim, icon, eventMsg);

    GetResource(RES_EVENT, eventMsg, name);
    p = strchr(name, '\\');
    if ( p != 0 )  *p = 0;
    SetName(name);

    return TRUE;
}


// Management of an event.

BOOL CCheck::EventProcess(const Event &event)
{
    if ( m_state & STATE_DEAD )  return TRUE;

    CControl::EventProcess(event);

    if ( event.event == EVENT_LBUTTONDOWN &&
         (m_state & STATE_VISIBLE)        &&
         (m_state & STATE_ENABLE)         )
    {
        if ( CControl::Detect(event.pos) )
        {
            Event newEvent = event;
            newEvent.event = m_eventMsg;
            m_event->AddEvent(newEvent);
            return FALSE;
        }
    }

    return TRUE;
}


// Draw button.

void CCheck::Draw()
{
    FPOINT      iDim, pos;
    float       zoomExt, zoomInt;
    int         icon;

    if ( (m_state & STATE_VISIBLE) == 0 )  return;

    iDim = m_dim;
    m_dim.x = m_dim.y*0.75f;  // square

    if ( m_state & STATE_SHADOW )
    {
        DrawShadow(m_pos, m_dim);
    }

    m_engine->SetTexture("button1.tga");
    m_engine->SetState(D3DSTATENORMAL);

    zoomExt = 1.00f;
    zoomInt = 0.95f;

    icon = 2;
    if ( m_state & STATE_DEFAULT )
    {
        DrawPart(23, 1.3f, 0.0f);

        zoomExt *= 1.15f;
        zoomInt *= 1.15f;
    }
    if ( m_state & STATE_HILIGHT )
    {
        icon = 1;
    }
    if ( m_state & STATE_PRESS )
    {
        icon = 3;
        zoomInt *= 0.9f;
    }
    if ( (m_state & STATE_ENABLE) == 0 )
    {
        icon = 7;
    }
    if ( m_state & STATE_DEAD )
    {
        icon = 17;
    }
    DrawPart(icon, zoomExt, 0.0f);  // draws the button

    if ( (m_state & STATE_DEAD) == 0 )
    {
        m_engine->SetState(D3DSTATETTw);

        if ( m_state & STATE_CHECK )
        {
            icon = 16;  // seen
            DrawPart(icon, zoomInt, 0.0f);  // draw the icon
        }
    }

    m_dim = iDim;

    if ( m_state & STATE_DEAD )  return;

    // Draw the name.
    pos.x = m_pos.x+m_dim.y/0.9f;
    pos.y = m_pos.y+m_dim.y*0.50f;
    pos.y -= m_engine->RetText()->RetHeight(m_fontSize, m_fontType)/2.0f;
    m_engine->RetText()->DrawText(m_name, pos, m_dim.x, 1, m_fontSize, m_fontStretch, m_fontType, 0);
}


