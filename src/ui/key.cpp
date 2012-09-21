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

// key.cpp


#include "ui/key.h"

#include <string.h>

namespace Ui {

void GetKeyName(char *name, int key)
{
    if ( !GetResource(RES_KEY, key, name) ) {
        if (isalnum(key)) {
            name[0] = key;
            name[1] = 0;
        }
        else {
            sprintf(name, "Code %d", key);
        }
    }
}


// Object's constructor.

CKey::CKey() : CControl()
{
    m_key[0] = 0;
    m_key[1] = 0;
    m_bCatch = false;

    m_app = CApplication::GetInstancePointer();
}

// Object's destructor.

CKey::~CKey()
{
}


// Creates a new button.

bool CKey::Create(Math::Point pos, Math::Point dim, int icon, EventType eventMsg)
{
    char name[100];
    if (eventMsg == EVENT_NULL)
        eventMsg = GetUniqueEventType();

    CControl::Create(pos, dim, icon, eventMsg);
    GetResource(RES_EVENT, eventMsg, name);
    SetName(std::string(name));

    return true;
}


// Management of an event.

bool CKey::EventProcess(const Event &event)
{
    if (m_state & STATE_DEAD)
        return true;

    CControl::EventProcess(event);

    if (event.type == EVENT_MOUSE_BUTTON_DOWN) {
        if (event.mouseButton.button == MOUSE_BUTTON_LEFT) // left
            m_bCatch = Detect(event.mousePos);
    }

    if (event.type == EVENT_KEY_DOWN && m_bCatch) {
        m_bCatch = false;

        if ( TestKey(event.key.key) ) { // impossible ?
            m_sound->Play(SOUND_TZOING);
        } else {
            // TODO: test for virtual, joystick, etc.
            if ( event.key.key == m_key[0] || event.key.key == m_key[1] ) {
                m_key[0] = event.key.key;
                m_key[1] = 0;
            } else {
                m_key[1] = m_key[0];
                m_key[0] = event.key.key;
            }
            m_sound->Play(SOUND_CLICK);

            Event newEvent = event;
            newEvent.type = m_eventType;
            m_event->AddEvent(newEvent);
        }
        return false;
    }

    return true;
}


// Seeks when a key is already used.

bool CKey::TestKey(int key)
{
    if ( key == KEY(PAUSE) || key == KEY(PRINT) )  return true;  // blocked key

    /* TODO: input bindings 
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 2; j++) {
            if (key == m_app->GetKey(i, j) )  // key used?
                m_app->SetKey(i, j, 0);  // nothing!
        }

        if ( m_app->GetKey(i, 0) == 0 ) { // first free option?
            m_app->SetKey(i, 0, m_app->GetKey(i, 1));  // shift
            m_app->SetKey(i, 1, 0);
        }
    } */

    return false;  // not used
}


// Draws button.

void CKey::Draw()
{
    Math::Point iDim, pos;
    float zoomExt, zoomInt, h;
    int icon;
    char text[100];

    if ( (m_state & STATE_VISIBLE) == 0 )
        return;

    iDim = m_dim;
    m_dim.x = 200.0f/640.0f;

    if ( m_state & STATE_SHADOW )
        DrawShadow(m_pos, m_dim);


    m_engine->SetTexture("button1.png");
    m_engine->SetState(Gfx::ENG_RSTATE_NORMAL); // was D3DSTATENORMAL

    zoomExt = 1.00f;
    zoomInt = 0.95f;

    icon = 2;
    if ( m_key[0] == 0 && m_key[1] == 0 )  // no shortcut?
        icon = 3;

    if ( m_state & STATE_DEFAULT ) {
        DrawPart(23, 1.3f, 0.0f);

        zoomExt *= 1.15f;
        zoomInt *= 1.15f;
    }

    if ( m_state & STATE_HILIGHT )
        icon = 1;

    if ( m_state & STATE_CHECK )
        icon = 0;

    if ( m_state & STATE_PRESS ) {
        icon = 3;
        zoomInt *= 0.9f;
    }

    if ( (m_state & STATE_ENABLE) == 0 )
        icon = 7;

    if ( m_state & STATE_DEAD )
        icon = 17;

    if ( m_bCatch )
        icon = 23;

    DrawPart(icon, zoomExt, 8.0f / 256.0f);  // draws the button

    h = m_engine->GetText()->GetHeight(m_fontType, m_fontSize) / 2.0f;

    GetKeyName(text, m_key[0]);
    if ( m_key[1] != 0 ) {
        GetResource(RES_TEXT, RT_KEY_OR, text+strlen(text));
        GetKeyName(text+strlen(text), m_key[1]);
    }

    pos.x = m_pos.x + m_dim.x * 0.5f;
    pos.y = m_pos.y + m_dim.y * 0.5f;
    pos.y -= h;
    m_engine->GetText()->DrawText(std::string(text), m_fontType, m_fontSize, pos, m_dim.x, Gfx::TEXT_ALIGN_CENTER, 0);

    m_dim = iDim;

    if ( m_state & STATE_DEAD )
        return;

    // Draws the name.
    pos.x = m_pos.x + (214.0f / 640.0f);
    pos.y = m_pos.y + m_dim.y * 0.5f;
    pos.y -= h;
    m_engine->GetText()->DrawText(std::string(m_name), m_fontType, m_fontSize, pos, m_dim.x, Gfx::TEXT_ALIGN_LEFT, 0);
}



void CKey::SetKey(int option, int key)
{
    if ( option < 0 || option > 1 )  return;

    m_key[option] = key;
}

int CKey::GetKey(int option)
{
    if ( option < 0 || option > 1 )  return 0;

    return m_key[option];
}

}
