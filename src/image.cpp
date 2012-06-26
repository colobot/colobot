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

// image.cpp

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
#include "image.h"




// Object's constructor.

CImage::CImage(CInstanceManager* iMan) : CControl(iMan)
{
    m_filename[0] = 0;
}

// Object's destructor.

CImage::~CImage()
{
    if ( m_filename[0] != 0 )
    {
        m_engine->FreeTexture(m_filename);
    }
}


// Creates a new button.

BOOL CImage::Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
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


// Specifies the name of the image display.

void CImage::SetFilenameImage(char *name)
{
    if ( m_filename[0] != 0 )
    {
        m_engine->FreeTexture(m_filename);
    }

    strcpy(m_filename, name);
}

char* CImage::RetFilenameImage()
{
    return m_filename;
}


// Management of an event.

BOOL CImage::EventProcess(const Event &event)
{
    return TRUE;
}


// Draws button.

void CImage::Draw()
{
    FPOINT      uv1,uv2, corner, pos, dim;
    float       dp;

    if ( (m_state & STATE_VISIBLE) == 0 )  return;

    if ( m_state & STATE_SHADOW )
    {
        DrawShadow(m_pos, m_dim);
    }

    dp = 0.5f/256.0f;

    if ( m_icon == 0 )  // hollow frame?
    {
        m_engine->SetTexture("button2.tga");
        m_engine->SetState(D3DSTATENORMAL);
        uv1.x = 160.0f/256.0f;
        uv1.y = 192.0f/256.0f;  // u-v texture
        uv2.x = 192.0f/256.0f;
        uv2.y = 224.0f/256.0f;
        uv1.x += dp;
        uv1.y += dp;
        uv2.x -= dp;
        uv2.y -= dp;
        corner.x = 10.0f/640.0f;
        corner.y = 10.0f/480.0f;
        DrawIcon(m_pos, m_dim, uv1, uv2, corner, 8.0f/256.0f);
    }

    if ( m_filename[0] != 0 )  // displays an image?
    {
        m_engine->LoadTexture(m_filename);
        m_engine->SetTexture(m_filename);
        m_engine->SetState(D3DSTATENORMAL);
        pos = m_pos;
        dim = m_dim;
        pos.x +=  5.0f/640.0f;
        pos.y +=  5.0f/480.0f;
        dim.x -= 10.0f/640.0f;
        dim.y -= 10.0f/480.0f;
        uv1.x = 0.0f;
        uv1.y = 0.0f;
        uv2.x = 1.0f;
        uv2.y = 1.0f;
        DrawIcon(pos, dim, uv1, uv2);
    }
}


