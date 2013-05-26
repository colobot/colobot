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


#include "ui/compass.h"

#include "common/event.h"
#include "common/misc.h"

#include "graphics/core/device.h"
#include "graphics/engine/engine.h"

#include "math/geometry.h"

namespace Ui {
// Object's constructor.

CCompass::CCompass() : CControl()
{
    m_dir = 0.0f;
}

// Object's destructor.

CCompass::~CCompass()
{
}


// Creates a new button.

bool CCompass::Create(Math::Point pos, Math::Point dim, int icon, EventType eventType)
{
    if ( eventType == EVENT_NULL )  eventType = GetUniqueEventType();

    CControl::Create(pos, dim, icon, eventType);
    return true;
}


// Management of an event.

bool CCompass::EventProcess(const Event &event)
{
    CControl::EventProcess(event);

    if ( event.type == EVENT_MOUSE_BUTTON_DOWN &&
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


// Draw button.

void CCompass::Draw()
{
    Gfx::CDevice* device;
    Gfx::Vertex   vertex[4];  // 2 triangles
    Math::Point   p1, p2, p3, c, uv1, uv2;
    Math::Vector  n;
    float        dp;

    if ( (m_state & STATE_VISIBLE) == 0 )  return;

    device = m_engine->GetDevice();

    m_engine->SetTexture("button2.png");
    m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);

    p1.x = m_pos.x;
    p1.y = m_pos.y;
    p2.x = m_pos.x + m_dim.x;
    p2.y = m_pos.y + m_dim.y;

    c.x = (p1.x + p2.x) / 2.0f;
    c.y = (p1.y + p2.y) / 2.0f;  // center

    uv1.x = 64.0f / 256.0f;
    uv1.y = 32.0f / 256.0f;
    uv2.x = 96.0f / 256.0f;
    uv2.y = 64.0f / 256.0f;

    dp = 0.5f / 256.0f;
    uv1.x += dp;
    uv1.y += dp;
    uv2.x -= dp;
    uv2.y -= dp;

    n = Math::Vector(0.0f, 0.0f, -1.0f);  // normal

    vertex[0] = Gfx::Vertex(Math::Vector(p1.x, p1.y, 0.0f), n, Math::Point(uv1.x, uv2.y));
    vertex[1] = Gfx::Vertex(Math::Vector(p1.x, p2.y, 0.0f), n, Math::Point(uv1.x, uv1.y));
    vertex[2] = Gfx::Vertex(Math::Vector(p2.x, p1.y, 0.0f), n, Math::Point(uv2.x, uv2.y));
    vertex[3] = Gfx::Vertex(Math::Vector(p2.x, p2.y, 0.0f), n, Math::Point(uv2.x, uv1.y));

    device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLE_STRIP, vertex, 4);
    m_engine->AddStatisticTriangle(2);

    if ( m_state & STATE_ENABLE )
    {
        p1.x = c.x;
        p1.y = c.y + m_dim.x * 0.40f;
        p1 = Math::RotatePoint(c, m_dir, p1);
        p1.x = c.x + (p1.x - c.x) * (m_dim.x / m_dim.y);

        p2.x = c.x + m_dim.x * 0.20f;
        p2.y = c.y - m_dim.x * 0.40f;
        p2 = Math::RotatePoint(c, m_dir, p2);
        p2.x = c.x + (p2.x - c.x) * (m_dim.x / m_dim.y);

        p3.x = c.x - m_dim.x * 0.20f;
        p3.y = c.y - m_dim.x * 0.40f;
        p3 = Math::RotatePoint(c, m_dir, p3);
        p3.x = c.x + (p3.x - c.x) * (m_dim.x / m_dim.y);

        uv1.x =  96.0f / 256.0f;
        uv1.y =  32.0f / 256.0f;
        uv2.x = 104.0f / 256.0f;
        uv2.y =  64.0f / 256.0f;

        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;

        vertex[0] = Gfx::Vertex(Math::Vector(p1.x, p1.y, 0.0f), n, Math::Point(uv1.x, uv1.y));
        vertex[1] = Gfx::Vertex(Math::Vector(p2.x, p2.y, 0.0f), n, Math::Point(uv1.x, uv2.y));
        vertex[2] = Gfx::Vertex(Math::Vector(p3.x, p3.y, 0.0f), n, Math::Point(uv2.x, uv2.y));

        device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLES, vertex, 3);
        m_engine->AddStatisticTriangle(1);
    }
}


// Management directions of the compass.

void CCompass::SetDirection(float dir)
{
    m_dir = dir;
}

float CCompass::GetDirection()
{
    return m_dir;
}


}

