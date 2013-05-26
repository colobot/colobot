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


#include "ui/group.h"

#include "common/event.h"
#include "common/misc.h"
#include "common/restext.h"

#include "graphics/engine/engine.h"


#include <stdio.h>
#include <string.h>


namespace Ui {
// Object's constructor.

CGroup::CGroup() : CControl()
{
}

// Object's destructor.

CGroup::~CGroup()
{
}


// Creates a new button.

bool CGroup::Create(Math::Point pos, Math::Point dim, int icon, EventType eventType)
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

bool CGroup::EventProcess(const Event &event)
{
    return true;
}


// Draw button.

void CGroup::Draw()
{
    Math::Point     uv1,uv2, corner, pos, dim;
    float       dp;
    int         icon;

    if ( (m_state & STATE_VISIBLE) == 0 )  return;

    if ( m_state & STATE_SHADOW )
    {
        DrawShadow(m_pos, m_dim);
    }

    dp = 0.5f / 256.0f;

    if ( m_icon == 0 )  // hollow frame?
    {
        m_engine->SetTexture("button2.png");
        m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);
        uv1.x = 160.0f / 256.0f;
        uv1.y = 192.0f / 256.0f;  // u-v texture
        uv2.x = 192.0f / 256.0f;
        uv2.y = 224.0f / 256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        corner.x = 10.0f/640.0f;
        corner.y = 10.0f/480.0f;
        DrawIcon(m_pos, m_dim, uv1, uv2, corner, 8.0f/256.0f);
    }
    if ( m_icon == 1 )  // orange solid opaque?
    {
        m_engine->SetTexture("button2.png");
        m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);
        uv1.x = 104.0f / 256.0f;
        uv1.y =  48.0f / 256.0f;
        uv2.x = 112.0f / 256.0f;
        uv2.y =  64.0f / 256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(m_pos, m_dim, uv1, uv2);
    }
    if ( m_icon == 2 )  // orange degrade -> transparent?
    {
        m_engine->SetTexture("button2.png");
        m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_WHITE);
        uv1.x = 112.0f / 256.0f;
        uv1.y =  48.0f / 256.0f;
        uv2.x = 120.0f / 256.0f;
        uv2.y =  64.0f / 256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(m_pos, m_dim, uv1, uv2);
    }
    if ( m_icon == 3 )  // transparent gradient -> gray?
    {
        m_engine->SetTexture("button2.png");
        m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_WHITE);
        uv1.x = 120.0f / 256.0f;
        uv1.y =  48.0f / 256.0f;
        uv2.x = 128.0f / 256.0f;
        uv2.y =  64.0f / 256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(m_pos, m_dim, uv1, uv2);
    }
    if ( m_icon == 4 )  // degrade blue corner?
    {
        m_engine->SetTexture("button2.png");
        m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_WHITE);
        uv1.x = 192.0f / 256.0f;
        uv1.y = 128.0f / 256.0f;
        uv2.x = 224.0f / 256.0f;
        uv2.y = 160.0f / 256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(m_pos, m_dim, uv1, uv2);
    }
    if ( m_icon == 5 )  // degrade orange corner?
    {
        m_engine->SetTexture("button2.png");
        m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_WHITE);
        uv1.x = 224.0f / 256.0f;
        uv1.y = 128.0f / 256.0f;
        uv2.x = 256.0f / 256.0f;
        uv2.y = 160.0f / 256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(m_pos, m_dim, uv1, uv2);
    }
    if ( m_icon == 6 )
    {
        m_engine->SetTexture("button2.png");
        m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_BLACK);
        uv1.x =   0.0f / 256.0f;  // brown transparent
        uv1.y =  75.0f / 256.0f;
        uv2.x =  64.0f / 256.0f;
        uv2.y = 128.0f / 256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        corner.x = 4.0f / 640.0f;
        corner.y = 4.0f / 480.0f;
        DrawIcon(m_pos, m_dim, uv1, uv2, corner, 8.0f/256.0f);
    }
    if ( m_icon == 7 )
    {
        m_engine->SetTexture("button1.png");
        m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);
        uv1.x =  64.0f / 256.0f;
        uv1.y =   0.0f / 256.0f;
        uv2.x =  96.0f / 256.0f;
        uv2.y =  32.0f / 256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(m_pos, m_dim, uv1, uv2, 8.0f/256.0f);
    }
    if ( m_icon == 8 )
    {
        m_engine->SetTexture("button2.png");
        m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_BLACK);
        uv1.x =  64.0f / 256.0f;  // green transparent
        uv1.y = 160.0f / 256.0f;
        uv2.x = 160.0f / 256.0f;
        uv2.y = 176.0f / 256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(m_pos, m_dim, uv1, uv2, 8.0f/256.0f);
    }
    if ( m_icon == 9 )
    {
        m_engine->SetTexture("button2.png");
        m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_BLACK);
        uv1.x =  64.0f / 256.0f;  // red transparent
        uv1.y = 176.0f/256.0f;
        uv2.x = 160.0f/256.0f;
        uv2.y = 192.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(m_pos, m_dim, uv1, uv2, 8.0f/256.0f);
    }
    if ( m_icon == 10 )
    {
        m_engine->SetTexture("button2.png");
        m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_BLACK);
        uv1.x =  64.0f / 256.0f;  // blue transparent
        uv1.y = 192.0f / 256.0f;
        uv2.x = 160.0f / 256.0f;
        uv2.y = 208.0f / 256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(m_pos, m_dim, uv1, uv2, 8.0f/256.0f);
    }
    if ( m_icon == 11 )
    {
        m_engine->SetTexture("button2.png");
        m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_BLACK);
        uv1.x =  64.0f / 256.0f;  // yellow transparent
        uv1.y = 224.0f / 256.0f;
        uv2.x = 160.0f / 256.0f;
        uv2.y = 240.0f / 256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(m_pos, m_dim, uv1, uv2, 8.0f/256.0f);
    }
    if ( m_icon == 12 )  // viewfinder cross?
    {
        dim.x = m_dim.x / 2.0f;
        dim.y = m_dim.y / 2.0f;

        m_engine->SetTexture("mouse.png");
        m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_BLACK);
        pos.x = m_pos.x-m_dim.x/300.0f;
        pos.y = m_pos.y+m_dim.y/300.0f+dim.y;
        uv1.x =   0.5f / 256.0f;
        uv1.y = 192.5f / 256.0f;
        uv2.x =  63.5f / 256.0f;
        uv2.y = 255.5f / 256.0f;
        DrawIcon(pos, dim, uv1, uv2);  // ul
        pos.x += dim.x;
        Math::Swap(uv1.x, uv2.x);
        DrawIcon(pos, dim, uv1, uv2);  // ur
        pos.y -= dim.y;
        Math::Swap(uv1.y, uv2.y);
        DrawIcon(pos, dim, uv1, uv2);  // dr
        pos.x -= dim.x;
        Math::Swap(uv1.x, uv2.x);
        DrawIcon(pos, dim, uv1, uv2);  // dl

        m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_WHITE);
        pos.x = m_pos.x+m_dim.x/300.0f;
        pos.y = m_pos.y-m_dim.y/300.0f+dim.y;
        uv1.x =  64.5f / 256.0f;
        uv1.y = 192.5f / 256.0f;
        uv2.x = 127.5f / 256.0f;
        uv2.y = 255.5f / 256.0f;
        DrawIcon(pos, dim, uv1, uv2);  // ul
        pos.x += dim.x;
        Math::Swap(uv1.x, uv2.x);
        DrawIcon(pos, dim, uv1, uv2);  // ur
        pos.y -= dim.y;
        Math::Swap(uv1.y, uv2.y);
        DrawIcon(pos, dim, uv1, uv2);  // dr
        pos.x -= dim.x;
        Math::Swap(uv1.x, uv2.x);
        DrawIcon(pos, dim, uv1, uv2);  // dl
    }
    if ( m_icon == 13 )  // corner upper / left?
    {
        m_engine->SetTexture("mouse.png");
        m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_BLACK);
        pos.x = m_pos.x-m_dim.x/150.0f;
        pos.y = m_pos.y+m_dim.y/150.0f;
        uv1.x = 128.5f / 256.0f;
        uv1.y = 192.5f / 256.0f;
        uv2.x = 191.5f / 256.0f;
        uv2.y = 255.5f / 256.0f;
        DrawIcon(pos, m_dim, uv1, uv2);

        m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_WHITE);
        pos.x = m_pos.x+m_dim.x/150.0f;
        pos.y = m_pos.y-m_dim.y/150.0f;
        uv1.x = 192.5f / 256.0f;
        uv1.y = 192.5f / 256.0f;
        uv2.x = 255.5f / 256.0f;
        uv2.y = 255.5f / 256.0f;
        DrawIcon(pos, m_dim, uv1, uv2);
    }
    if ( m_icon == 14 )  // corner upper / right?
    {
        m_engine->SetTexture("mouse.png");
        m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_BLACK);
        pos.x = m_pos.x-m_dim.x/150.0f;
        pos.y = m_pos.y+m_dim.y/150.0f;
        uv2.x = 128.5f / 256.0f;
        uv1.y = 192.5f / 256.0f;
        uv1.x = 191.5f / 256.0f;
        uv2.y = 255.5f / 256.0f;
        DrawIcon(pos, m_dim, uv1, uv2);

        m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_WHITE);
        pos.x = m_pos.x+m_dim.x/150.0f;
        pos.y = m_pos.y-m_dim.y/150.0f;
        uv2.x = 192.5f / 256.0f;
        uv1.y = 192.5f / 256.0f;
        uv1.x = 255.5f / 256.0f;
        uv2.y = 255.5f / 256.0f;
        DrawIcon(pos, m_dim, uv1, uv2);
    }
    if ( m_icon == 15 )  // corner lower / left?
    {
        m_engine->SetTexture("mouse.png");
        m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_BLACK);
        pos.x = m_pos.x-m_dim.x/150.0f;
        pos.y = m_pos.y+m_dim.y/150.0f;
        uv1.x = 128.5f / 256.0f;
        uv2.y = 192.5f / 256.0f;
        uv2.x = 191.5f / 256.0f;
        uv1.y = 255.5f / 256.0f;
        DrawIcon(pos, m_dim, uv1, uv2);

        m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_WHITE);
        pos.x = m_pos.x+m_dim.x/150.0f;
        pos.y = m_pos.y-m_dim.y/150.0f;
        uv1.x = 192.5f / 256.0f;
        uv2.y = 192.5f / 256.0f;
        uv2.x = 255.5f / 256.0f;
        uv1.y = 255.5f / 256.0f;
        DrawIcon(pos, m_dim, uv1, uv2);
    }
    if ( m_icon == 16 )  // corner lower / left?
    {
        m_engine->SetTexture("mouse.png");
        m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_BLACK);
        pos.x = m_pos.x-m_dim.x/150.0f;
        pos.y = m_pos.y+m_dim.y/150.0f;
        uv2.x = 128.5f / 256.0f;
        uv2.y = 192.5f / 256.0f;
        uv1.x = 191.5f / 256.0f;
        uv1.y = 255.5f / 256.0f;
        DrawIcon(pos, m_dim, uv1, uv2);

        m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_WHITE);
        pos.x = m_pos.x+m_dim.x/150.0f;
        pos.y = m_pos.y-m_dim.y/150.0f;
        uv2.x = 192.5f / 256.0f;
        uv2.y = 192.5f / 256.0f;
        uv1.x = 255.5f / 256.0f;
        uv1.y = 255.5f / 256.0f;
        DrawIcon(pos, m_dim, uv1, uv2);
    }
    if ( m_icon == 17 )
    {
        m_engine->SetTexture("button2.png");
        m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);
        uv1.x =   0.0f / 256.0f;  // blue frame
        uv1.y =  75.0f / 256.0f;
        uv2.x =  64.0f / 256.0f;
        uv2.y = 128.0f / 256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        corner.x = 6.0f / 640.0f;
        corner.y = 6.0f / 480.0f;
        DrawIcon(m_pos, m_dim, uv1, uv2, corner, 2.0f/256.0f);
    }
    if ( m_icon == 18 )  // arrow> for SatCom?
    {
        m_engine->SetTexture("button1.png");
        m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_WHITE);
        uv1.x =   0.0f / 256.0f;   // >
        uv1.y = 192.0f / 256.0f;
        uv2.x =  32.0f / 256.0f;
        uv2.y = 224.0f / 256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(m_pos, m_dim, uv1, uv2);
    }
    if ( m_icon == 19 )  // SatCom symbol?
    {
        m_engine->SetTexture("button1.png");
        m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_WHITE);
        uv1.x = 224.0f / 256.0f;  // SatCom symbol
        uv1.y = 224.0f / 256.0f;
        uv2.x = 256.0f / 256.0f;
        uv2.y = 256.0f / 256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(m_pos, m_dim, uv1, uv2);
    }
    if ( m_icon == 20 )  // solid blue background?
    {
        m_engine->SetTexture("button1.png");
        m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_WHITE);
        uv1.x = 224.0f / 256.0f;
        uv1.y =  32.0f / 256.0f;
        uv2.x = 256.0f / 256.0f;
        uv2.y =  64.0f / 256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(m_pos, m_dim, uv1, uv2);
    }
    if ( m_icon == 21 )  // stand-by symbol?
    {
        m_engine->SetTexture("button2.png");
        m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_WHITE);
        uv1.x = 160.0f / 256.0f;
        uv1.y =  32.0f / 256.0f;
        uv2.x = 192.0f / 256.0f;
        uv2.y =  64.0f / 256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(m_pos, m_dim, uv1, uv2);
    }
    if ( m_icon == 22 )
    {
        m_engine->SetTexture("button2.png");
        m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);
        uv1.x =  64.0f / 256.0f;  // opaque yellow
        uv1.y = 224.0f / 256.0f;
        uv2.x = 160.0f / 256.0f;
        uv2.y = 240.0f / 256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        corner.x = 5.0f / 640.0f;
        corner.y = 5.0f / 480.0f;
        DrawIcon(m_pos, m_dim, uv1, uv2, corner, 3.0f/256.0f);
    }

    if ( m_icon == 23 )
    {
        m_engine->SetTexture("button3.png");
        m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);
        uv1.x =  64.0f / 256.0f;  // yellow
        uv1.y = 192.0f / 256.0f;
        uv2.x =  80.0f / 256.0f;
        uv2.y = 208.0f / 256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        corner.x = 4.0f / 640.0f;
        corner.y = 4.0f / 480.0f;
        DrawIcon(m_pos, m_dim, uv1, uv2, corner, 2.0f/256.0f);
    }
    if ( m_icon == 24 )
    {
        m_engine->SetTexture("button3.png");
        m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);
        uv1.x =  80.0f / 256.0f;  // orange
        uv1.y = 192.0f / 256.0f;
        uv2.x =  96.0f / 256.0f;
        uv2.y = 208.0f / 256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        corner.x = 4.0f / 640.0f;
        corner.y = 4.0f / 480.0f;
        DrawIcon(m_pos, m_dim, uv1, uv2, corner, 2.0f/256.0f);
    }
    if ( m_icon == 25 )
    {
        m_engine->SetTexture("button3.png");
        m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);
        uv1.x =  64.0f / 256.0f;  // orange
        uv1.y = 208.0f / 256.0f;
        uv2.x =  80.0f / 256.0f;
        uv2.y = 224.0f / 256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        corner.x = 4.0f / 640.0f;
        corner.y = 4.0f / 480.0f;
        DrawIcon(m_pos, m_dim, uv1, uv2, corner, 2.0f/256.0f);
    }
    if ( m_icon == 26 )
    {
        m_engine->SetTexture("button3.png");
        m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);
        uv1.x =  80.0f / 256.0f;   // red
        uv1.y = 208.0f / 256.0f;
        uv2.x =  96.0f / 256.0f;
        uv2.y = 224.0f / 256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        corner.x = 4.0f / 640.0f;
        corner.y = 4.0f / 480.0f;
        DrawIcon(m_pos, m_dim, uv1, uv2, corner, 2.0f/256.0f);
    }
    if ( m_icon == 27 )
    {
        m_engine->SetTexture("button3.png");
        m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);
        uv1.x =  32.0f / 256.0f;
        uv1.y =   0.0f / 256.0f;
        uv2.x =  64.0f / 256.0f;
        uv2.y =  32.0f / 256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(m_pos, m_dim, uv1, uv2);
    }

    if ( m_icon >= 100 && m_icon <= 120 )  // building?
    {
        pos = m_pos;
        dim = m_dim;

        m_engine->SetTexture("button2.png");
        m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);
        uv1.x = 32.0f / 256.0f;
        uv1.y = 32.0f / 256.0f;
        uv2.x = uv1.x+32.0f/256.0f;
        uv2.y = uv1.y+32.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(pos, dim, uv1, uv2);

        m_engine->SetTexture("button3.png");
        m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);
        pos.x +=  8.0f / 640.0f;
        pos.y +=  8.0f / 480.0f;
        dim.x -= 16.0f / 640.0f;
        dim.y -= 16.0f / 480.0f;
        uv1.x = 32.0f / 256.0f;
        uv1.y =  0.0f / 256.0f;
        uv2.x = uv1.x+32.0f/256.0f;
        uv2.y = uv1.y+32.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(pos, dim, uv1, uv2);

        m_engine->SetState(Gfx::ENG_RSTATE_NORMAL);
        pos.x += 2.0f / 640.0f;
        pos.y += 2.0f / 480.0f;
        dim.x -= 4.0f / 640.0f;
        dim.y -= 4.0f / 480.0f;
        uv1.x = 0.0f / 256.0f;
        uv1.y = 0.0f / 256.0f;
        uv2.x = uv1.x+32.0f/256.0f;
        uv2.y = uv1.y+32.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(pos, dim, uv1, uv2);

        m_engine->SetState(Gfx::ENG_RSTATE_TTEXTURE_BLACK);
        pos.x +=  8.0f / 640.0f;
        pos.y +=  8.0f / 480.0f;
        dim.x -= 16.0f / 640.0f;
        dim.y -= 16.0f / 480.0f;
        if ( m_icon == 100 )  icon = 43;  // base ?
        if ( m_icon == 101 )  icon = 32;  // factory ?
        if ( m_icon == 102 )  icon = 35;  // research ?
        if ( m_icon == 103 )  icon = 34;  // convert ?
        if ( m_icon == 104 )  icon = 36;  // station ?
        if ( m_icon == 105 )  icon = 40;  // radar ?
        if ( m_icon == 106 )  icon = 41;  // repair ?
        if ( m_icon == 107 )  icon = 37;  // tower ?
        if ( m_icon == 108 )  icon = 39;  // energy ?
        if ( m_icon == 109 )  icon = 33;  // derrick ?
        if ( m_icon == 110 )  icon = 42;  // nuclear ?
        if ( m_icon == 111 )  icon = 38;  // labo ?
        if ( m_icon == 112 )  icon = 44;  // info ?
        if ( m_icon == 113 )  icon = 46;  // lightning protection ?
        if ( m_icon == 114 )  icon = 47;  // vault ?
        if ( m_icon == 115 )  icon = 48;  // control center?
        uv1.x = (32.0f/256.0f)*(icon%8);
        uv1.y = (32.0f/256.0f)*(icon/8);  // uv texture
        uv2.x = uv1.x+32.0f/256.0f;
        uv2.y = uv1.y+32.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        DrawIcon(pos, dim, uv1, uv2);
    }
}


}

