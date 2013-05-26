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


#include "ui/gauge.h"


namespace Ui {
// Object's constructor.

CGauge::CGauge() : CControl()
{
    m_level = 0.0f;
}

// Object's destructor.

CGauge::~CGauge()
{
}


// Creates a new button.

bool CGauge::Create(Math::Point pos, Math::Point dim, int icon, EventType eventType)
{
    if ( eventType == EVENT_NULL )  eventType = GetUniqueEventType();

    CControl::Create(pos, dim, icon, eventType);
    return true;
}


// Management of an event.

bool CGauge::EventProcess(const Event &event)
{
    CControl::EventProcess(event);

    if ( event.type == EVENT_MOUSE_BUTTON_DOWN )
    {
        if ( CControl::Detect(event.mousePos) )
        {
            Event newEvent = event;
            newEvent.type = m_eventType;
            m_event->AddEvent(newEvent);
            return false;
        }
    }

    return true;
}


// Draw the gauge.

void CGauge::Draw()
{
    Math::Point     pos, dim, ddim, uv1, uv2, corner;
    float       dp;

    if ( (m_state & STATE_VISIBLE) == 0 )  return;

    m_engine->SetTexture("button2.png");
    m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);

    dp = 0.5f/256.0f;

    pos = m_pos;
    dim = m_dim;

    uv1.x = 32.0f / 256.0f;
    uv1.y = 32.0f / 256.0f;
    uv2.x = 64.0f / 256.0f;
    uv2.y = 64.0f / 256.0f;

    uv1.x += dp;
    uv1.y += dp;
    uv2.x -= dp;
    uv2.y -= dp;

    corner.x = 10.0f / 640.0f;
    corner.y = 10.0f / 480.0f;

    DrawIcon(pos, dim, uv1, uv2, corner, 8.0f/256.0f);


    pos.x += 3.0f / 640.0f;
    pos.y += 3.0f / 480.0f;
    dim.x -= 6.0f / 640.0f;
    dim.y -= 6.0f / 480.0f;

    if ( m_dim.x < m_dim.y )  // vertical gauge?
    {
        uv1.x = (0.0f + m_icon * 16.0f) / 256.0f;
        uv2.x = uv1.x + 16.0f / 256.0f;
        uv1.y = 128.0f/256.0f + m_level * (64.0f/256.0f);
        uv2.y = uv1.y + 64.0f/256.0f;
    }
    else    // horizontal gauge?
    {
        uv1.x = 64.0f/256.0f + (1.0f - m_level) * (64.0f/256.0f);
        uv2.x = uv1.x + 64.0f/256.0f;
        uv1.y = (128.0f + m_icon*16.0f) / 256.0f;
        uv2.y = uv1.y + 16.0f/256.0f;
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

float CGauge::GetLevel()
{
    return m_level;
}


}

