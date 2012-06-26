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
#include "math3d.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "compass.h"




// Object's constructor.

CCompass::CCompass(CInstanceManager* iMan) : CControl(iMan)
{
    m_dir = 0.0f;
}

// Object's destructor.

CCompass::~CCompass()
{
}


// Creates a new button.

BOOL CCompass::Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
    if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

    CControl::Create(pos, dim, icon, eventMsg);
    return TRUE;
}


// Management of an event.

BOOL CCompass::EventProcess(const Event &event)
{
    CControl::EventProcess(event);

    if ( event.event == EVENT_LBUTTONDOWN )
    {
        if ( CControl::Detect(event.pos) )
        {
            Event newEvent = event;
            newEvent.event = m_eventMsg;
            m_event->AddEvent(newEvent);
            return FALSE;
        }
    }

    return TRUE;
}


// Draw button.

void CCompass::Draw()
{
    LPDIRECT3DDEVICE7 device;
    D3DVERTEX2      vertex[4];  // 2 triangles
    FPOINT          p1, p2, p3, c, uv1, uv2;
    D3DVECTOR       n;
    float           dp;

    if ( (m_state & STATE_VISIBLE) == 0 )  return;

    device = m_engine->RetD3DDevice();

    m_engine->SetTexture("button2.tga");
    m_engine->SetState(D3DSTATENORMAL);

    p1.x = m_pos.x;
    p1.y = m_pos.y;
    p2.x = m_pos.x + m_dim.x;
    p2.y = m_pos.y + m_dim.y;

    c.x = (p1.x+p2.x)/2.0f;
    c.y = (p1.y+p2.y)/2.0f;  // center

    uv1.x = 64.0f/256.0f;
    uv1.y = 32.0f/256.0f;
    uv2.x = 96.0f/256.0f;
    uv2.y = 64.0f/256.0f;

    dp = 0.5f/256.0f;
    uv1.x += dp;
    uv1.y += dp;
    uv2.x -= dp;
    uv2.y -= dp;

    n = D3DVECTOR(0.0f, 0.0f, -1.0f);  // normal

    vertex[0] = D3DVERTEX2(D3DVECTOR(p1.x, p1.y, 0.0f), n, uv1.x,uv2.y);
    vertex[1] = D3DVERTEX2(D3DVECTOR(p1.x, p2.y, 0.0f), n, uv1.x,uv1.y);
    vertex[2] = D3DVERTEX2(D3DVECTOR(p2.x, p1.y, 0.0f), n, uv2.x,uv2.y);
    vertex[3] = D3DVERTEX2(D3DVECTOR(p2.x, p2.y, 0.0f), n, uv2.x,uv1.y);

    device->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 4, NULL);
    m_engine->AddStatisticTriangle(2);

    if ( m_state & STATE_ENABLE )
    {
        p1.x = c.x;
        p1.y = c.y+m_dim.x*0.40f;
        p1 = RotatePoint(c, m_dir, p1);
        p1.x = c.x+(p1.x-c.x)*(m_dim.x/m_dim.y);

        p2.x = c.x+m_dim.x*0.20f;
        p2.y = c.y-m_dim.x*0.40f;
        p2 = RotatePoint(c, m_dir, p2);
        p2.x = c.x+(p2.x-c.x)*(m_dim.x/m_dim.y);

        p3.x = c.x-m_dim.x*0.20f;
        p3.y = c.y-m_dim.x*0.40f;
        p3 = RotatePoint(c, m_dir, p3);
        p3.x = c.x+(p3.x-c.x)*(m_dim.x/m_dim.y);

        uv1.x =  96.0f/256.0f;
        uv1.y =  32.0f/256.0f;
        uv2.x = 104.0f/256.0f;
        uv2.y =  64.0f/256.0f;

        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;

        vertex[0] = D3DVERTEX2(D3DVECTOR(p1.x, p1.y, 0.0f), n, uv1.x,uv1.y);
        vertex[1] = D3DVERTEX2(D3DVECTOR(p2.x, p2.y, 0.0f), n, uv1.x,uv2.y);
        vertex[2] = D3DVERTEX2(D3DVECTOR(p3.x, p3.y, 0.0f), n, uv2.x,uv2.y);

        device->DrawPrimitive(D3DPT_TRIANGLELIST, D3DFVF_VERTEX2, vertex, 3, NULL);
        m_engine->AddStatisticTriangle(1);
    }
}


// Management directions of the compass.

void CCompass::SetDirection(float dir)
{
    m_dir = dir;
}

float CCompass::RetDirection()
{
    return m_dir;
}


