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


#include "ui/color.h"

#include "common/event.h"
#include "common/misc.h"
#include "common/restext.h"

#include "graphics/core/device.h"
#include "graphics/engine/engine.h"

#include <string.h>


namespace Ui {
const float DELAY1 = 0.4f;
const float DELAY2 = 0.1f;



// Object's constructor.

CColor::CColor() : CControl()
{
    m_bRepeat = false;
    m_repeat = 0.0f;

    m_color.r = 0.0f;
    m_color.g = 0.0f;
    m_color.b = 0.0f;
    m_color.a = 0.0f;
}

// Object's destructor.

CColor::~CColor()
{
}


// Creates a new button.

bool CColor::Create(Math::Point pos, Math::Point dim, int icon, EventType eventType)
{
    if ( eventType == EVENT_NULL )  eventType = GetUniqueEventType();

    CControl::Create(pos, dim, icon, eventType);

    if ( icon == -1 )
    {
        char    name[100];
        char*   p;

        GetResource(RES_EVENT, eventType, name);
        p = strchr(name, '\\');
        if ( p != 0 )  *p = 0;
        SetName(name);
    }

    return true;
}


// Management of an event.

bool CColor::EventProcess(const Event &event)
{
    if ( m_state & STATE_DEAD )  return true;

    CControl::EventProcess(event);

    if ( event.type == EVENT_FRAME && m_bRepeat )
    {
        if ( m_repeat != 0.0f )
        {
            m_repeat -= event.rTime;
            if ( m_repeat <= 0.0f )
            {
                m_repeat = DELAY2;

                Event newEvent = event;
                newEvent.type = m_eventType;
                m_event->AddEvent(newEvent);
                return false;
            }
        }
    }

    if ( event.type == EVENT_MOUSE_BUTTON_DOWN &&
         event.mouseButton.button == MOUSE_BUTTON_LEFT &&
         (m_state & STATE_VISIBLE)        &&
         (m_state & STATE_ENABLE)         )
    {
        if ( CControl::Detect(event.mousePos) )
        {
            m_repeat = DELAY1;

            Event newEvent = event;
            newEvent.type = m_eventType;
            m_event->AddEvent(newEvent);
            return false;
        }
    }

    if ( event.type == EVENT_MOUSE_BUTTON_UP && event.mouseButton.button == MOUSE_BUTTON_LEFT)
    {
        m_repeat = 0.0f;
    }

    return true;
}


// Dessine le bouton.

void CColor::Draw()
{
    Gfx::CDevice* device;
    Gfx::VertexCol  vertex[4];  // 2 triangles
    Gfx::Color   color;
    Math::Point     p1, p2;

    if ( (m_state & STATE_VISIBLE) == 0 )  return;

    if ( m_state & STATE_SHADOW )
    {
        DrawShadow(m_pos, m_dim);
    }

    m_engine->SetTexture("button1.png");
    m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);
    CControl::Draw();

#if _TEEN
//    color = GetColor(m_color);
    color = GetColor();

    m_engine->SetTexture("");  // no texture
    m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);

    device = m_engine->GetDevice();

    p1.x = m_pos.x + (4.0f / 640.0f);
    p1.y = m_pos.y + (4.0f / 480.0f);
    p2.x = m_pos.x + m_dim.x - (4.0f / 640.0f);
    p2.y = m_pos.y + m_dim.y - (4.0f / 480.0f);
    vertex[0] = Gfx::Vertex(Math::Vector(p1.x, p1.y, 0.0f), 0x00000000,0x00000000, Math::Point( 0.0f, 0.0f));
    vertex[1] = Gfx::Vertex(Math::Vector(p1.x, p2.y, 0.0f), 0x00000000,0x00000000, Math::Point( 0.0f, 0.0f));
    vertex[2] = Gfx::Vertex(Math::Vector(p2.x, p1.y, 0.0f), 0x00000000,0x00000000, Math::Point( 0.0f, 0.0f));
    vertex[3] = Gfx::Vertex(Math::Vector(p2.x, p2.y, 0.0f), 0x00000000,0x00000000, Math::Point( 0.0f, 0.0f));
    device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLE_STRIP, vertex, 4);

    p1.x = m_pos.x + (5.0f / 640.0f);
    p1.y = m_pos.y + (5.0f / 480.0f);
    p2.x = m_pos.x + m_dim.x - (5.0f / 640.0f);
    p2.y = m_pos.y + m_dim.y - (5.0f / 480.0f);
    vertex[0] = Gfx::Vertex(Math::Vector(p1.x, p1.y, 0.0f), color,0x00000000, Math::Point( 0.0f, 0.0f));
    vertex[1] = Gfx::Vertex(Math::Vector(p1.x, p2.y, 0.0f), color,0x00000000, Math::Point( 0.0f, 0.0f));
    vertex[2] = Gfx::Vertex(Math::Vector(p2.x, p1.y, 0.0f), color,0x00000000, Math::Point( 0.0f, 0.0f));
    vertex[3] = Gfx::Vertex(Math::Vector(p2.x, p2.y, 0.0f), color,0x00000000, Math::Point( 0.0f, 0.0f));
    device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLE_STRIP, vertex, 4);

    m_engine->AddStatisticTriangle(4);
#else
    p1.x = m_pos.x + (3.0f / 640.0f);
    p1.y = m_pos.y + (3.0f / 480.0f);
    p2.x = m_pos.x + m_dim.x - (3.0f / 640.0f);
    p2.y = m_pos.y + m_dim.y - (3.0f / 480.0f);

    color = GetColor();

    m_engine->SetTexture("");  // no texture
    m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);

    vertex[0] = Gfx::VertexCol(Math::Vector(p1.x, p1.y, 0.0f), color);
    vertex[1] = Gfx::VertexCol(Math::Vector(p1.x, p2.y, 0.0f), color);
    vertex[2] = Gfx::VertexCol(Math::Vector(p2.x, p1.y, 0.0f), color);
    vertex[3] = Gfx::VertexCol(Math::Vector(p2.x, p2.y, 0.0f), color);

    device = m_engine->GetDevice();
    device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLE_STRIP, vertex, 4);
    m_engine->AddStatisticTriangle(2);
#endif
}


void CColor::SetRepeat(bool bRepeat)
{
    m_bRepeat = bRepeat;
}

bool CColor::GetRepeat()
{
    return m_bRepeat;
}


void CColor::SetColor(Gfx::Color color)
{
    m_color = color;
}

Gfx::Color CColor::GetColor()
{
    return m_color;
}


}

