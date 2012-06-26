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

// gauge.cpp

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
#include "gauge.h"




// Object's constructor.

CGauge::CGauge(CInstanceManager* iMan) : CControl(iMan)
{
    m_level = 0.0f;
}

// Object's destructor.

CGauge::~CGauge()
{
}


// Creates a new button.

BOOL CGauge::Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
    if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

    CControl::Create(pos, dim, icon, eventMsg);
    return TRUE;
}


// Management of an event.

BOOL CGauge::EventProcess(const Event &event)
{
    CControl::EventProcess(event);

    if ( event.event == EVENT_LBUTTONDOWN )
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


// Draw the gauge.

void CGauge::Draw()
{
    FPOINT      pos, dim, ddim, uv1, uv2, corner;
    float       dp;

    if ( (m_state & STATE_VISIBLE) == 0 )  return;

    m_engine->SetTexture("button2.tga");
    m_engine->SetState(D3DSTATENORMAL);

    dp = 0.5f/256.0f;

    pos = m_pos;
    dim = m_dim;

    uv1.x = 32.0f/256.0f;
    uv1.y = 32.0f/256.0f;
    uv2.x = 64.0f/256.0f;
    uv2.y = 64.0f/256.0f;

    uv1.x += dp;
    uv1.y += dp;
    uv2.x -= dp;
    uv2.y -= dp;

    corner.x = 10.0f/640.0f;
    corner.y = 10.0f/480.0f;

    DrawIcon(pos, dim, uv1, uv2, corner, 8.0f/256.0f);


    pos.x += 3.0f/640.0f;
    pos.y += 3.0f/480.0f;
    dim.x -= 6.0f/640.0f;
    dim.y -= 6.0f/480.0f;

    if ( m_dim.x < m_dim.y )  // vertical gauge?
    {
        uv1.x = (0.0f+m_icon*16.0f)/256.0f;
        uv2.x = uv1.x+16.0f/256.0f;
        uv1.y = 128.0f/256.0f+m_level*(64.0f/256.0f);
        uv2.y = uv1.y+64.0f/256.0f;
    }
    else    // horizontal gauge?
    {
        uv1.x = 64.0f/256.0f+(1.0f-m_level)*(64.0f/256.0f);
        uv2.x = uv1.x+64.0f/256.0f;
        uv1.y = (128.0f+m_icon*16.0f)/256.0f;
        uv2.y = uv1.y+16.0f/256.0f;
    }

    uv1.x += dp;
    uv1.y += dp;
    uv2.x -= dp;
    uv2.y -= dp;

    DrawIcon(pos, dim, uv1, uv2);
}


// Management of level of the gauge.

void CGauge::SetLevel(float level)
{
    if ( level < 0.0f )  level = 0.0f;
    if ( level > 1.0f )  level = 1.0f;
    m_level = level;
}

float CGauge::RetLevel()
{
    return m_level;
}


