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


#include "ui/shortcut.h"

#include "common/event.h"
#include "common/misc.h"

#include "graphics/engine/engine.h"
#include "graphics/core/device.h"

#include <math.h>

namespace Ui {

// Object's constructor.

CShortcut::CShortcut() : CControl()
{
    m_time = 0.0f;
}

// Object's destructor.

CShortcut::~CShortcut()
{
}


// Creates a new button.

bool CShortcut::Create(Math::Point pos, Math::Point dim, int icon, EventType eventType)
{
    if ( eventType == EVENT_NULL )  eventType = GetUniqueEventType();

    CControl::Create(pos, dim, icon, eventType);
    return true;
}


// Management of an event.

bool CShortcut::EventProcess(const Event &event)
{
    CControl::EventProcess(event);

    if ( event.type == EVENT_FRAME )
    {
        m_time += event.rTime;
    }

    if ( event.type == EVENT_MOUSE_BUTTON_DOWN  &&
            event.mouseButton.button == MOUSE_BUTTON_LEFT)
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


// Draws the button.

void CShortcut::Draw()
{
    float       zoom;
    int         icon, mode;

    icon = 0;
    zoom = 0.8f;
    mode = Gfx::ENG_RSTATE_TTEXTURE_WHITE;
    if ( m_state & STATE_HILIGHT )
    {
        icon = 4;
        zoom = 0.9f;
        mode = Gfx::ENG_RSTATE_NORMAL;
    }
    if ( m_state & STATE_CHECK )
    {
        icon = 1;
        zoom = 0.8f;
        mode = Gfx::ENG_RSTATE_NORMAL;
    }
    if ( m_state & STATE_PRESS )
    {
        icon = 1;
        zoom = 1.0f;
        mode = Gfx::ENG_RSTATE_NORMAL;
    }
    if ( m_icon == 6 || m_icon == 7 )  // pause or film?
    {
        icon = -1;  // no bottom
        zoom = 1.0f;
    }

    m_engine->SetTexture("button3.png");

    if ( icon != -1 )
    {
        m_engine->SetState(mode);
        DrawVertex(icon, 0.95f);
    }

    m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_BLACK);
    DrawVertex(m_icon, zoom);

    if ( m_state & STATE_FRAME )
    {
        Math::Point p1, p2, c, uv1, uv2;
        float   dp;

        m_engine->SetTexture("button2.png");
        m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_WHITE);

        zoom = 0.9f+sinf(m_time*8.0f)*0.1f;

        p1.x = m_pos.x;
        p1.y = m_pos.y;
        p2.x = m_pos.x + m_dim.x;
        p2.y = m_pos.y + m_dim.y;

        c.x = (p1.x+p2.x)/2.0f;
        c.y = (p1.y+p2.y)/2.0f;  // center

        p1.x = (p1.x-c.x)*zoom + c.x;
        p1.y = (p1.y-c.y)*zoom + c.y;
        p2.x = (p2.x-c.x)*zoom + c.x;
        p2.y = (p2.y-c.y)*zoom + c.y;

        p2.x -= p1.x;
        p2.y -= p1.y;

        uv1.x = 176.0f/256.0f;
        uv1.y = 224.0f/256.0f;
        uv2.x = 192.0f/256.0f;
        uv2.y = 240.0f/256.0f;

        dp = 0.5f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;

        DrawIcon(p1, p2, uv1, uv2);
    }

    if ( (m_state & STATE_RUN) && Math::Mod(m_time, 0.7f) >= 0.3f )
    {
        Math::Point uv1, uv2;
        float   dp;

        m_engine->SetTexture("button3.png");
        m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_WHITE);

        uv1.x = 160.0f/256.0f;
        uv1.y =   0.0f/256.0f;
        uv2.x = 192.0f/256.0f;
        uv2.y =  32.0f/256.0f;

        dp = 0.5f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;

        DrawIcon(m_pos, m_dim, uv1, uv2);
    }
}

// Draw the vertex array.

void CShortcut::DrawVertex(int icon, float zoom)
{
    Gfx::CDevice* device;
    Gfx::Vertex  vertex[4];  // 2 triangles
    Math::Point     p1, p2, c;
    Math::Vector    n;
    float       u1, u2, v1, v2, dp;

    device = m_engine->GetDevice();

    p1.x = m_pos.x;
    p1.y = m_pos.y;
    p2.x = m_pos.x + m_dim.x;
    p2.y = m_pos.y + m_dim.y;

    c.x = (p1.x+p2.x)/2.0f;
    c.y = (p1.y+p2.y)/2.0f;  // center

    p1.x = (p1.x-c.x)*zoom + c.x;
    p1.y = (p1.y-c.y)*zoom + c.y;

    p2.x = (p2.x-c.x)*zoom + c.x;
    p2.y = (p2.y-c.y)*zoom + c.y;

    u1 = (32.0f/256.0f)*(icon%8);
    v1 = (32.0f/256.0f)*(icon/8);  // u-v texture
    u2 = (32.0f/256.0f)+u1;
    v2 = (32.0f/256.0f)+v1;

    dp = 0.5f/256.0f;
    u1 += dp;
    v1 += dp;
    u2 -= dp;
    v2 -= dp;

    n = Math::Vector(0.0f, 0.0f, -1.0f);  // normal

    vertex[0] = Gfx::Vertex(Math::Vector(p1.x, p1.y, 0.0f), n, Math::Point(u1, v2));
    vertex[1] = Gfx::Vertex(Math::Vector(p1.x, p2.y, 0.0f), n, Math::Point(u1, v1));
    vertex[2] = Gfx::Vertex(Math::Vector(p2.x, p1.y, 0.0f), n, Math::Point(u2, v2));
    vertex[3] = Gfx::Vertex(Math::Vector(p2.x, p2.y, 0.0f), n, Math::Point(u2, v1));

    device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLE_STRIP, vertex, 4);
    m_engine->AddStatisticTriangle(2);
}

}

