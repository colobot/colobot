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
#include "language.h"
#include "math3d.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "restext.h"
#include "button.h"



#define DELAY1      0.4f
#define DELAY2      0.1f



// Object's constructor.

CButton::CButton(CInstanceManager* iMan) : CControl(iMan)
{
    m_bCapture = FALSE;
    m_bImmediat = FALSE;
    m_bRepeat = FALSE;
    m_repeat = 0.0f;
}

// Object's destructor.

CButton::~CButton()
{
}


// Creates a new button.

BOOL CButton::Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
    if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

    CControl::Create(pos, dim, icon, eventMsg);

    if ( icon == -1 )
    {
        char    name[100];
        char*   p;

        GetResource(RES_EVENT, eventMsg, name);
        p = strchr(name, '\\');
        if ( p != 0 )  *p = 0;
        SetName(name);
    }

    return TRUE;
}


// Management of an event.

BOOL CButton::EventProcess(const Event &event)
{
    if ( (m_state & STATE_VISIBLE) == 0 )  return TRUE;
    if ( m_state & STATE_DEAD )  return TRUE;

    CControl::EventProcess(event);

    if ( event.event == EVENT_FRAME )
    {
        if ( m_bRepeat && m_repeat != 0.0f )
        {
            m_repeat -= event.rTime;
            if ( m_repeat <= 0.0f )
            {
                m_repeat = DELAY2;

                Event newEvent = event;
                newEvent.event = m_eventMsg;
                m_event->AddEvent(newEvent);
                return FALSE;
            }
        }
    }

    if ( event.event == EVENT_LBUTTONDOWN &&
         (m_state & STATE_VISIBLE)        &&
         (m_state & STATE_ENABLE)         )
    {
        if ( CControl::Detect(event.pos) )
        {
            m_bCapture = TRUE;
            m_repeat = DELAY1;

            if ( m_bImmediat || m_bRepeat )
            {
                Event newEvent = event;
                newEvent.event = m_eventMsg;
                m_event->AddEvent(newEvent);
            }
            return FALSE;
        }
    }

    if ( event.event == EVENT_MOUSEMOVE && m_bCapture )
    {
    }

    if ( event.event == EVENT_LBUTTONUP && m_bCapture )
    {
        if ( CControl::Detect(event.pos) )
        {
            if ( !m_bImmediat && !m_bRepeat )
            {
                Event newEvent = event;
                newEvent.event = m_eventMsg;
                m_event->AddEvent(newEvent);
            }
        }

        m_bCapture = FALSE;
        m_repeat = 0.0f;
    }

    return TRUE;
}


// Draw button.

void CButton::Draw()
{
    FPOINT  pos, dim, uv1, uv2;
#if !_NEWLOOK
    float   dp;
#endif

    if ( (m_state & STATE_VISIBLE) == 0 )  return;

    if ( m_state & STATE_WARNING )  // shading yellow-black?
    {
        pos.x = m_pos.x-( 8.0f/640.0f);
        pos.y = m_pos.y-( 4.0f/480.0f);
        dim.x = m_dim.x+(16.0f/640.0f);
        dim.y = m_dim.y+( 8.0f/480.0f);
        if ( m_state & STATE_SHADOW )
        {
            DrawShadow(pos, dim);
        }
        DrawWarning(pos, dim);
    }

    if ( m_state & STATE_SHADOW )
    {
        DrawShadow(m_pos, m_dim);
    }

    CControl::Draw();

#if !_NEWLOOK
    if ( m_name[0] != 0                &&  // button with the name?
         (m_state & STATE_CARD  ) == 0 &&
         (m_state & STATE_SIMPLY) == 0 )
    {
        m_engine->SetTexture("button2.tga");
        m_engine->SetState(D3DSTATENORMAL);

        dp = 0.5f/256.0f;

        uv1.x = 128.0f/256.0f;
        uv1.y =  96.0f/256.0f;
        uv2.x = 136.0f/256.0f;
        uv2.y = 128.0f/256.0f;

        if ( (m_state & STATE_ENABLE) == 0 )
        {
            uv1.x += 16.0f/256.0f;
            uv2.x += 16.0f/256.0f;
        }

        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;

        pos.y = m_pos.y+5.0f/480.0f;
        dim.y = m_dim.y-10.0f/480.0f;
        pos.x = m_pos.x+5.0f/640.0f;
        dim.x = 3.0f/640.0f;
        DrawIcon(pos, dim, uv1, uv2, 0.0f);

        uv1.x += 8.0f/256.0f;
        uv2.x += 8.0f/256.0f;
        pos.x = m_pos.x+m_dim.x-5.0f/640.0f-3.0f/640.0f;
        DrawIcon(pos, dim, uv1, uv2, 0.0f);
    }
#endif
}


// Management of immediate mode, which sends the event "press"
// before the mouse button is released.

void CButton::SetImmediat(BOOL bImmediat)
{
    m_bImmediat = bImmediat;
}

BOOL CButton::RetImmediat()
{
    return m_bImmediat;
}


// Mode management "autorepeat", when the button
// mouse is held down.

void CButton::SetRepeat(BOOL bRepeat)
{
    m_bRepeat = bRepeat;
}

BOOL CButton::RetRepeat()
{
    return m_bRepeat;
}

