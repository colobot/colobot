// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// * Copyright (C) 2012 Polish Portal of Colobot (PPC)
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


#include "ui/label.h"

namespace Ui {


// Object's constructor.

CLabel::CLabel() : CControl()
{
}

// Object's destructor.

CLabel::~CLabel()
{
}


// Creates a new button.

bool CLabel::Create(Math::Point pos, Math::Point dim, int icon, EventType eventMsg)
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
    Math::Point pos;

    if ( (m_state & STATE_VISIBLE) == 0 )
        return;

    pos.y = m_pos.y + m_dim.y / 2.0f;

    switch (m_textAlign) {
        case Gfx::TEXT_ALIGN_LEFT: pos.x = m_pos.x; break;
        case Gfx::TEXT_ALIGN_CENTER: pos.x = m_pos.x + m_dim.x / 2.0f; break;
        case Gfx::TEXT_ALIGN_RIGHT: pos.x = m_pos.x + m_dim.x; break;
    }

    m_engine->GetText()->DrawText(std::string(m_name), m_fontType, m_fontSize, pos, m_dim.x, m_textAlign, 0);
}

}
