/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2021, Daniel Roux, EPSITEC SA & TerranovaTeam
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


#include "ui/controls/check.h"

#include "common/event.h"
#include "common/restext.h"

#include "graphics/core/renderers.h"
#include "graphics/core/transparency.h"

#include "graphics/engine/engine.h"
#include "graphics/engine/text.h"

#include <string.h>


namespace Ui
{

// Object's constructor.

CCheck::CCheck() : CControl()
{
}

// Object's destructor.

CCheck::~CCheck()
{
}


// Creates a new button.

bool CCheck::Create(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventType)
{
    if ( eventType == EVENT_NULL )  eventType = GetUniqueEventType();

    CControl::Create(pos, dim, icon, eventType);

    std::string name = GetResourceName(eventType);
    SetName(name);

    return true;
}


// Management of an event.

bool CCheck::EventProcess(const Event &event)
{
    if ( m_state & STATE_DEAD )  return true;

    CControl::EventProcess(event);

    if ( event.type == EVENT_MOUSE_BUTTON_DOWN &&
         event.GetData<MouseButtonEventData>()->button == MOUSE_BUTTON_LEFT &&
         (m_state & STATE_VISIBLE)             &&
         (m_state & STATE_ENABLE)         )
    {
        if ( CControl::Detect(event.mousePos) )
        {
            m_event->AddEvent(Event(m_eventType));
            return false;
        }
    }

    return true;
}


// Draw button.

void CCheck::Draw()
{
    glm::vec2   iDim, pos;
    float       zoomExt, zoomInt;
    int         icon;

    auto renderer = m_engine->GetUIRenderer();

    if ( (m_state & STATE_VISIBLE) == 0 )  return;

    iDim = m_dim;
    m_dim.x = m_dim.y * 0.75f;  // square

    if ( m_state & STATE_SHADOW )
    {
        DrawShadow(m_pos, m_dim);
    }

    auto texture = m_engine->LoadTexture("textures/interface/button1.png");
    renderer->SetTexture(texture);
    renderer->SetTransparency(Gfx::TransparencyMode::NONE);

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
        renderer->SetTransparency(Gfx::TransparencyMode::WHITE);

        if ( m_state & STATE_CHECK )
        {
            icon = 16;  // seen
            DrawPart(icon, zoomInt, 0.0f);  // draw the icon
        }
    }

    m_dim = iDim;

    // Draw the name.
    pos.x = m_pos.x + m_dim.y / 0.9f;
    pos.y = m_pos.y + m_dim.y * 0.50f;
    pos.y -= m_engine->GetText()->GetHeight(m_fontType, m_fontSize)/2.0f;
    m_engine->GetText()->DrawText(m_name, m_fontType, m_fontSize, pos, m_dim.x, Gfx::TEXT_ALIGN_LEFT, 0);
}

}
