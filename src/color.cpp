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

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "d3dengine.h"
#include "language.h"
#include "math3d.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "restext.h"
#include "color.h"



#define DELAY1      0.4f
#define DELAY2      0.1f



// Object's constructor.

CColor::CColor(CInstanceManager* iMan) : CControl(iMan)
{
    m_bRepeat = FALSE;
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

BOOL CColor::Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
    if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

    CControl::Create(pos, dim, icon, eventMsg);

    if ( icon == -1 )
    {
        char    name[100];
        char*   p;

        GetResource(RES_EVENT, eventMsg, name);
        p = strchr(name, '\\');
        if ( p != 0 )  *p = 0;
        SetName(name);
    }

    return TRUE;
}


// Management of an event.

BOOL CColor::EventProcess(const Event &event)
{
    if ( m_state & STATE_DEAD )  return TRUE;

    CControl::EventProcess(event);

    if ( event.event == EVENT_FRAME && m_bRepeat )
    {
        if ( m_repeat != 0.0f )
        {
            m_repeat -= event.rTime;
            if ( m_repeat <= 0.0f )
            {
                m_repeat = DELAY2;

                Event newEvent = event;
                newEvent.event = m_eventMsg;
                m_event->AddEvent(newEvent);
                return FALSE;
            }
        }
    }

    if ( event.event == EVENT_LBUTTONDOWN &&
         (m_state & STATE_VISIBLE)        &&
         (m_state & STATE_ENABLE)         )
    {
        if ( CControl::Detect(event.pos) )
        {
            m_repeat = DELAY1;

            Event newEvent = event;
            newEvent.event = m_eventMsg;
            m_event->AddEvent(newEvent);
            return FALSE;
        }
    }

    if ( event.event == EVENT_LBUTTONUP )
    {
        m_repeat = 0.0f;
    }

    return TRUE;
}


// Dessine le bouton.

void CColor::Draw()
{
    LPDIRECT3DDEVICE7 device;
    D3DLVERTEX  vertex[4];  // 2 triangles
    D3DCOLOR    color;
    FPOINT      p1, p2;

    if ( (m_state & STATE_VISIBLE) == 0 )  return;

    if ( m_state & STATE_SHADOW )
    {
        DrawShadow(m_pos, m_dim);
    }

    m_engine->SetTexture("button1.tga");
    m_engine->SetState(D3DSTATENORMAL);
    CControl::Draw();

#if _TEEN
    color = ::RetColor(m_color);

    m_engine->SetTexture("xxx.tga");  // no texture
    m_engine->SetState(D3DSTATENORMAL);

    device = m_engine->RetD3DDevice();

    p1.x = m_pos.x+(4.0f/640.0f);
    p1.y = m_pos.y+(4.0f/480.0f);
    p2.x = m_pos.x+m_dim.x-(4.0f/640.0f);
    p2.y = m_pos.y+m_dim.y-(4.0f/480.0f);
    vertex[0] = D3DLVERTEX(D3DVECTOR(p1.x, p1.y, 0.0f), 0x00000000,0x00000000, 0.0f,0.0f);
    vertex[1] = D3DLVERTEX(D3DVECTOR(p1.x, p2.y, 0.0f), 0x00000000,0x00000000, 0.0f,0.0f);
    vertex[2] = D3DLVERTEX(D3DVECTOR(p2.x, p1.y, 0.0f), 0x00000000,0x00000000, 0.0f,0.0f);
    vertex[3] = D3DLVERTEX(D3DVECTOR(p2.x, p2.y, 0.0f), 0x00000000,0x00000000, 0.0f,0.0f);
    device->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_LVERTEX, vertex, 4, NULL);

    p1.x = m_pos.x+(5.0f/640.0f);
    p1.y = m_pos.y+(5.0f/480.0f);
    p2.x = m_pos.x+m_dim.x-(5.0f/640.0f);
    p2.y = m_pos.y+m_dim.y-(5.0f/480.0f);
    vertex[0] = D3DLVERTEX(D3DVECTOR(p1.x, p1.y, 0.0f), color,0x00000000, 0.0f,0.0f);
    vertex[1] = D3DLVERTEX(D3DVECTOR(p1.x, p2.y, 0.0f), color,0x00000000, 0.0f,0.0f);
    vertex[2] = D3DLVERTEX(D3DVECTOR(p2.x, p1.y, 0.0f), color,0x00000000, 0.0f,0.0f);
    vertex[3] = D3DLVERTEX(D3DVECTOR(p2.x, p2.y, 0.0f), color,0x00000000, 0.0f,0.0f);
    device->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_LVERTEX, vertex, 4, NULL);

    m_engine->AddStatisticTriangle(4);
#else
    p1.x = m_pos.x+(3.0f/640.0f);
    p1.y = m_pos.y+(3.0f/480.0f);
    p2.x = m_pos.x+m_dim.x-(3.0f/640.0f);
    p2.y = m_pos.y+m_dim.y-(3.0f/480.0f);

    color = ::RetColor(m_color);

    m_engine->SetTexture("xxx.tga");  // no texture
    m_engine->SetState(D3DSTATENORMAL);

    vertex[0] = D3DLVERTEX(D3DVECTOR(p1.x, p1.y, 0.0f), color,0x00000000, 0.0f,0.0f);
    vertex[1] = D3DLVERTEX(D3DVECTOR(p1.x, p2.y, 0.0f), color,0x00000000, 0.0f,0.0f);
    vertex[2] = D3DLVERTEX(D3DVECTOR(p2.x, p1.y, 0.0f), color,0x00000000, 0.0f,0.0f);
    vertex[3] = D3DLVERTEX(D3DVECTOR(p2.x, p2.y, 0.0f), color,0x00000000, 0.0f,0.0f);

    device = m_engine->RetD3DDevice();
    device->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_LVERTEX, vertex, 4, NULL);
    m_engine->AddStatisticTriangle(2);
#endif
}


void CColor::SetRepeat(BOOL bRepeat)
{
    m_bRepeat = bRepeat;
}

BOOL CColor::RetRepeat()
{
    return m_bRepeat;
}


void CColor::SetColor(D3DCOLORVALUE color)
{
    m_color = color;
}

D3DCOLORVALUE CColor::RetColor()
{
    return m_color;
}


