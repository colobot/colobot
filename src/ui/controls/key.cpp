/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
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


#include "ui/controls/key.h"

#include "common/global.h"
#include "common/stringutils.h"

#include "graphics/engine/engine.h"

#include "sound/sound.h"

#include <cstring>

namespace Ui
{


CKey::CKey() : CControl()
{
    m_catch = false;

    m_input = CInput::GetInstancePointer();
}

CKey::~CKey()
{
    m_input = nullptr;
}

bool CKey::Create(Math::Point pos, Math::Point dim, int icon, EventType eventMsg)
{
    if (eventMsg == EVENT_NULL)
        eventMsg = GetUniqueEventType();

    CControl::Create(pos, dim, icon, eventMsg);

    std::string name;
    GetResource(RES_EVENT, eventMsg, name);
    SetName(name);

    return true;
}

bool CKey::EventProcess(const Event &event)
{
    if (m_state & STATE_DEAD)
        return true;

    CControl::EventProcess(event);

    if (event.type == EVENT_MOUSE_BUTTON_DOWN)
    {
        if (event.GetData<MouseButtonEventData>()->button == MOUSE_BUTTON_LEFT) // left
            m_catch = Detect(event.mousePos);
    }

    if (event.type == EVENT_KEY_DOWN && m_catch)
    {
        m_catch = false;
        unsigned int key = GetVirtualKey(event.GetData<KeyEventData>()->key);

        if (TestKey(key)) // impossible ?
        {
            m_sound->Play(SOUND_TZOING);
        }
        else
        {
            if (key == m_binding.primary || key == m_binding.secondary)
            {
                m_binding.secondary = KEY_INVALID;
                m_binding.primary = key;
            }
            else
            {
                m_binding.secondary = m_binding.primary;
                m_binding.primary = key;
            }
            m_sound->Play(SOUND_CLICK);

            m_event->AddEvent(Event(m_eventType));
        }
        return false;
    }

    return true;
}

bool CKey::TestKey(unsigned int key)
{
    if (key == KEY(PAUSE)) return true;  // blocked key

    for (int i = 0; i < INPUT_SLOT_MAX; i++)
    {
        InputSlot slot = static_cast<InputSlot>(i);
        InputBinding b = m_input->GetInputBinding(slot);
        if (key == b.primary)
            m_input->SetInputBinding(slot, InputBinding(b.secondary, KEY_INVALID));  // nothing!
        if (key == b.secondary)
            m_input->SetInputBinding(slot, InputBinding(b.primary, KEY_INVALID));  // nothing!

        if (b.primary == KEY_INVALID) // first free option?
            m_input->SetInputBinding(slot, InputBinding(b.secondary, b.primary));  // shift
    }

    return false;  // not used
}

void CKey::Draw()
{
    if ((m_state & STATE_VISIBLE) == 0)
        return;

    Math::Point iDim = m_dim;
    m_dim.x = 100.0f/640.0f;

    if (m_state & STATE_SHADOW)
        DrawShadow(m_pos, m_dim);


    m_engine->SetTexture("textures/interface/button1.png");
    m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);

    float zoomExt = 1.00f;
    float zoomInt = 0.95f;

    int icon = 2;
    if (m_binding.primary == KEY_INVALID && m_binding.secondary == KEY_INVALID)  // no shortcut?
        icon = 3;

    if (m_state & STATE_DEFAULT)
    {
        DrawPart(23, 1.3f, 0.0f);

        zoomExt *= 1.15f;
        zoomInt *= 1.15f;
    }

    if (m_state & STATE_HILIGHT)
        icon = 1;

    if (m_state & STATE_CHECK)
        icon = 0;

    if (m_state & STATE_PRESS)
    {
        icon = 3;
        zoomInt *= 0.9f;
    }

    if ((m_state & STATE_ENABLE) == 0)
        icon = 7;

    if (m_state & STATE_DEAD)
        icon = 17;

    if (m_catch)
        icon = 23;

    DrawPart(icon, zoomExt, 8.0f / 256.0f);  // draws the button

    float h = m_engine->GetText()->GetHeight(m_fontType, m_fontSize) / 2.0f;

    std::string keyName = m_input->GetKeysString(m_binding);

    Math::Point pos;
    pos.x = m_pos.x + m_dim.x * 0.5f;
    pos.y = m_pos.y + m_dim.y * 0.5f;
    pos.y -= h;
    m_engine->GetText()->DrawText(keyName, m_fontType, m_fontSize, pos, m_dim.x, Gfx::TEXT_ALIGN_CENTER, 0);

    m_dim = iDim;

    if (m_state & STATE_DEAD)
        return;

    // Draws the name.
    pos.x = m_pos.x + (114.0f / 640.0f);
    pos.y = m_pos.y + m_dim.y * 0.5f;
    pos.y -= h;
    m_engine->GetText()->DrawText(m_name, m_fontType, m_fontSize, pos, m_dim.x, Gfx::TEXT_ALIGN_LEFT, 0);
}

void CKey::SetBinding(InputBinding b)
{
    m_binding = b;
}

InputBinding CKey::GetBinding()
{
    return m_binding;
}


} // namespace Ui
