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

// key.cpp

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
#include "restext.h"
#include "sound.h"
#include "text.h"
#include "key.h"




// Constructs the name of a button.

void GetKeyName(char *name, int key)
{
    if ( !GetResource(RES_KEY, key, name) )
    {
        if ( (key >= '0' && key <= '9') ||
             (key >= 'A' && key <= 'Z') ||
             (key >= 'a' && key <= 'z') )
        {
            name[0] = key;
            name[1] = 0;
        }
        else
        {
            sprintf(name, "Code %d", key);
        }
    }
}




// Object's constructor.

CKey::CKey(CInstanceManager* iMan) : CControl(iMan)
{
    m_key[0] = 0;
    m_key[1] = 0;
    m_bCatch = FALSE;
}

// Object's destructor.

CKey::~CKey()
{
}


// Creates a new button.

BOOL CKey::Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
    char    name[100];
    char*   p;

    if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

    CControl::Create(pos, dim, icon, eventMsg);

    GetResource(RES_EVENT, eventMsg, name);
    p = strchr(name, '\\');
    if ( p != 0 )  *p = 0;
    SetName(name);

    return TRUE;
}


// Management of an event.

BOOL CKey::EventProcess(const Event &event)
{
    if ( m_state & STATE_DEAD )  return TRUE;

    CControl::EventProcess(event);

    if ( event.event == EVENT_LBUTTONDOWN )
    {
        if ( Detect(event.pos) )
        {
            m_bCatch = TRUE;
        }
        else
        {
            m_bCatch = FALSE;
        }
    }

    if ( event.event == EVENT_KEYDOWN && m_bCatch )
    {
        m_bCatch = FALSE;

        if ( TestKey(event.param) )  // impossible ?
        {
            m_sound->Play(SOUND_TZOING);
        }
        else
        {
            if ( event.param == m_key[0] ||
                 event.param == m_key[1] )
            {
                m_key[0] = event.param;
                m_key[1] = 0;
            }
            else
            {
                m_key[1] = m_key[0];
                m_key[0] = event.param;
            }
            m_sound->Play(SOUND_CLICK);

            Event newEvent = event;
            newEvent.event = m_eventMsg;
            m_event->AddEvent(newEvent);
        }
        return FALSE;
    }

    return TRUE;
}


// Seeks when a key is already used.

BOOL CKey::TestKey(int key)
{
    int     i, j;

    if ( key == VK_PAUSE    ||
         key == VK_SNAPSHOT )  return TRUE;  // blocked key

    for ( i=0 ; i<20 ; i++ )
    {
        for ( j=0 ; j<2 ; j++ )
        {
            if ( key == m_engine->RetKey(i, j) )  // key used?
            {
                m_engine->SetKey(i, j, 0);  // nothing!
            }
        }

        if ( m_engine->RetKey(i, 0) == 0 )  // first free option?
        {
            m_engine->SetKey(i, 0, m_engine->RetKey(i, 1));  // shift
            m_engine->SetKey(i, 1, 0);
        }
    }

    return FALSE;  // not used
}


// Draws button.

void CKey::Draw()
{
    FPOINT      iDim, pos;
    float       zoomExt, zoomInt, h;
    int         icon;
    char        text[100];

    if ( (m_state & STATE_VISIBLE) == 0 )  return;

    iDim = m_dim;
    m_dim.x = 200.0f/640.0f;

    if ( m_state & STATE_SHADOW )
    {
        DrawShadow(m_pos, m_dim);
    }

    m_engine->SetTexture("button1.tga");
    m_engine->SetState(D3DSTATENORMAL);

    zoomExt = 1.00f;
    zoomInt = 0.95f;

    icon = 2;
    if ( m_key[0] == 0 &&
         m_key[1] == 0 )  // no shortcut?
    {
        icon = 3;
    }
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
    if ( m_state & STATE_CHECK )
    {
        icon = 0;
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
    if ( m_bCatch )
    {
        icon = 23;
    }
    DrawPart(icon, zoomExt, 8.0f/256.0f);  // draws the button

    h = m_engine->RetText()->RetHeight(m_fontSize, m_fontType)/2.0f;

    GetKeyName(text, m_key[0]);
    if ( m_key[1] != 0 )
    {
        GetResource(RES_TEXT, RT_KEY_OR, text+strlen(text));
        GetKeyName(text+strlen(text), m_key[1]);
    }

    pos.x = m_pos.x+m_dim.x*0.5f;
    pos.y = m_pos.y+m_dim.y*0.5f;
    pos.y -= h;
    m_engine->RetText()->DrawText(text, pos, m_dim.x, 0, m_fontSize, m_fontStretch, m_fontType, 0);

    m_dim = iDim;

    if ( m_state & STATE_DEAD )  return;

    // Draws the name.
    pos.x = m_pos.x+(214.0f/640.0f);
    pos.y = m_pos.y+m_dim.y*0.5f;
    pos.y -= h;
    m_engine->RetText()->DrawText(m_name, pos, m_dim.x, 1, m_fontSize, m_fontStretch, m_fontType, 0);
}



void CKey::SetKey(int option, int key)
{
    if ( option < 0 ||
         option > 1 )  return;

    m_key[option] = key;
}

int CKey::RetKey(int option)
{
    if ( option < 0 ||
         option > 1 )  return 0;

    return m_key[option];
}

