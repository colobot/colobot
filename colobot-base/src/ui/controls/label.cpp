/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
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


#include "ui/controls/label.h"

#include "graphics/engine/engine.h"

#include <sstream>

namespace Ui
{


// Object's constructor.

CLabel::CLabel() : CControl()
{
}

// Object's destructor.

CLabel::~CLabel()
{
}


// Creates a new button.

bool CLabel::Create(const glm::vec2& pos, const glm::vec2& dim, int icon, EventType eventMsg)
{
    if (eventMsg == EVENT_NULL)
        eventMsg = GetUniqueEventType();

    CControl::Create(pos, dim, icon, eventMsg);
    return true;
}


// Management of an event.

bool CLabel::EventProcess(const Event &event)
{
//? CControl::EventProcess(event);
    return true;
}


// Draws button.

void CLabel::Draw()
{
    glm::vec2 pos = { 0, 0 };

    if ( (m_state & STATE_VISIBLE) == 0 )
        return;

    pos.y = m_pos.y + m_dim.y / 2.0f;

    switch (m_textAlign)
    {
        case Gfx::TEXT_ALIGN_LEFT: pos.x = m_pos.x; break;
        case Gfx::TEXT_ALIGN_CENTER: pos.x = m_pos.x + m_dim.x / 2.0f; break;
        case Gfx::TEXT_ALIGN_RIGHT: pos.x = m_pos.x + m_dim.x; break;
    }

    const float hLine = m_engine->GetText()->GetHeight(m_fontType, m_fontSize);

    std::istringstream iss(m_name);
    int nLines = 0;
    for (std::string line; std::getline(iss, line); )
    {
        nLines++;
    }
    pos.y += hLine / 2 * (nLines - 1);
    iss.clear();
    iss.seekg(0);
    for (std::string line; std::getline(iss, line); )
    {
        m_engine->GetText()->DrawText(line, m_fontType, m_fontSize, pos, m_dim.x, m_textAlign, 0);
        pos.y -= hLine;
    }
}

}

