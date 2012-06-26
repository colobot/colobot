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
#include "d3dmath.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "math3d.h"
#include "particule.h"
#include "terrain.h"
#include "camera.h"
#include "object.h"
#include "interface.h"
#include "button.h"
#include "window.h"
#include "auto.h"
#include "autohuston.h"




// Object's constructor.

CAutoHuston::CAutoHuston(CInstanceManager* iMan, CObject* object)
                         : CAuto(iMan, object)
{
    D3DVECTOR   pos;
    int         i;

    for ( i=0 ; i<HUSTONMAXLENS ; i++ )
    {
        m_lens[i].parti = -1;
    }

    pos = m_object->RetPosition(0);
    m_lens[0].type = PARTISELR;
    m_lens[1].type = PARTISELR;
    m_lens[2].type = PARTISELR;
    m_lens[3].type = PARTISELR;
    m_lens[0].pos = pos+D3DVECTOR(0.0f+13.0f, 34.0f, 30.0f      );
    m_lens[1].pos = pos+D3DVECTOR(0.0f-13.0f, 34.0f, 30.0f      );
    m_lens[2].pos = pos+D3DVECTOR(0.0f      , 34.0f, 30.0f+13.0f);
    m_lens[3].pos = pos+D3DVECTOR(0.0f      , 34.0f, 30.0f-13.0f);
    m_lens[0].dim = 4.0f;
    m_lens[1].dim = 4.0f;
    m_lens[2].dim = 4.0f;
    m_lens[3].dim = 4.0f;
    m_lens[0].total = 1.0f;
    m_lens[1].total = 1.0f;
    m_lens[2].total = 1.0f;
    m_lens[3].total = 1.0f;
    m_lens[0].off = 0.4f;
    m_lens[1].off = 0.4f;
    m_lens[2].off = 0.4f;
    m_lens[3].off = 0.4f;

    // Part under the radar.
    i = 4;
    m_lens[i].type = PARTISELR;
    m_lens[i].pos = pos+D3DVECTOR(-7.0f, 9.9f, 40.1f);
    m_lens[i].dim = 1.8f;
    m_lens[i].total = 0.4f;
    m_lens[i].off = 0.2f;
    i ++;

    m_lens[i].type = PARTISELY;
    m_lens[i].pos = pos+D3DVECTOR(-7.0f, 7.2f, 34.8f);
    m_lens[i].dim = 0.4f;
    m_lens[i].total = 0.7f;
    m_lens[i].off = 0.3f;
    i ++;
    m_lens[i].type = PARTISELY;
    m_lens[i].pos = pos+D3DVECTOR(-7.0f, 6.5f, 34.3f);
    m_lens[i].dim = 0.4f;
    m_lens[i].total = 0.7f;
    m_lens[i].off = 0.3f;
    i ++;
    m_lens[i].type = PARTISELR;
    m_lens[i].pos = pos+D3DVECTOR(-7.0f, 6.5f, 33.4f);
    m_lens[i].dim = 0.4f;
    m_lens[i].total = 0.0f;
    m_lens[i].off = 0.0f;
    i ++;
    m_lens[i].type = PARTISELR;
    m_lens[i].pos = pos+D3DVECTOR(-7.0f, 6.5f, 33.0f);
    m_lens[i].dim = 0.4f;
    m_lens[i].total = 1.0f;
    m_lens[i].off = 0.5f;
    i ++;

    m_lens[i].type = PARTISELY;
    m_lens[i].pos = pos+D3DVECTOR(-7.0f, 8.5f, 14.0f);
    m_lens[i].dim = 1.2f;
    m_lens[i].total = 0.8f;
    m_lens[i].off = 0.2f;
    i ++;

    m_lens[i].type = PARTISELR;
    m_lens[i].pos = pos+D3DVECTOR(4.0f, 6.0f, 8.6f);
    m_lens[i].dim = 1.0f;
    m_lens[i].total = 0.9f;
    m_lens[i].off = 0.7f;
    i ++;

    // Part with three windows.
    m_lens[i].type = PARTISELR;
    m_lens[i].pos = pos+D3DVECTOR(-7.0f, 9.9f, -19.9f);
    m_lens[i].dim = 1.0f;
    m_lens[i].total = 0.6f;
    m_lens[i].off = 0.3f;
    i ++;

    m_lens[i].type = PARTISELY;
    m_lens[i].pos = pos+D3DVECTOR(-7.0f, 7.2f, 34.8f-60.0f);
    m_lens[i].dim = 0.4f;
    m_lens[i].total = 0.7f;
    m_lens[i].off = 0.3f;
    i ++;
    m_lens[i].type = PARTISELY;
    m_lens[i].pos = pos+D3DVECTOR(-7.0f, 6.5f, 34.3f-60.0f);
    m_lens[i].dim = 0.4f;
    m_lens[i].total = 0.0f;
    m_lens[i].off = 0.0f;
    i ++;
    m_lens[i].type = PARTISELR;
    m_lens[i].pos = pos+D3DVECTOR(-7.0f, 6.5f, 33.4f-60.0f);
    m_lens[i].dim = 0.4f;
    m_lens[i].total = 0.6f;
    m_lens[i].off = 0.4f;
    i ++;
    m_lens[i].type = PARTISELR;
    m_lens[i].pos = pos+D3DVECTOR(-7.0f, 6.5f, 33.0f-60.0f);
    m_lens[i].dim = 0.4f;
    m_lens[i].total = 0.8f;
    m_lens[i].off = 0.2f;
    i ++;

    m_lens[i].type = PARTISELY;
    m_lens[i].pos = pos+D3DVECTOR(-6.5f, 13.5f, -37.0f);
    m_lens[i].dim = 1.0f;
    m_lens[i].total = 0.0f;
    m_lens[i].off = 0.0f;
    i ++;

    m_lens[i].type = PARTISELY;
    m_lens[i].pos = pos+D3DVECTOR(-7.0f, 12.2f, -39.8f);
    m_lens[i].dim = 1.8f;
    m_lens[i].total = 1.5f;
    m_lens[i].off = 0.5f;
    i ++;

    m_lens[i].type = PARTISELY;
    m_lens[i].pos = pos+D3DVECTOR(-7.0f, 8.5f, -47.0f);
    m_lens[i].dim = 0.6f;
    m_lens[i].total = 0.7f;
    m_lens[i].off = 0.5f;
    i ++;

    m_lensTotal = i;

    Init();
}

// Object's destructor.

CAutoHuston::~CAutoHuston()
{
    this->CAuto::~CAuto();
}


// Destroys the object.

void CAutoHuston::DeleteObject(BOOL bAll)
{
    CAuto::DeleteObject(bAll);
}


// Initialize the object.

void CAutoHuston::Init()
{
    m_time = 0.0f;

    m_progress = 0.0f;
    m_speed    = 1.0f/2.0f;
}


// Start the object.

void CAutoHuston::Start(int param)
{
}


// Management of an event.

BOOL CAutoHuston::EventProcess(const Event &event)
{
    D3DVECTOR   speed;
    FPOINT      dim;
    float       angle;
    int         i;

    CAuto::EventProcess(event);

    if ( m_engine->RetPause() )  return TRUE;

    angle = -m_time*1.0f;
    m_object->SetAngleY(1, angle);  // rotates the radar
    angle = sinf(m_time*4.0f)*0.3f;
    m_object->SetAngleX(2, angle);

    if ( event.event != EVENT_FRAME )  return TRUE;

    m_progress += event.rTime*m_speed;

    // Flashes the keys.
    speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
    for ( i=0 ; i<m_lensTotal ; i++ )
    {
        if ( m_lens[i].total != 0.0f &&
             Mod(m_time, m_lens[i].total) < m_lens[i].off )
        {
            if ( m_lens[i].parti != -1 )
            {
                m_particule->DeleteParticule(m_lens[i].parti);
                m_lens[i].parti = -1;
            }
        }
        else
        {
            if ( m_lens[i].parti == -1 )
            {
                dim.x = m_lens[i].dim;
                dim.y = dim.x;
                m_lens[i].parti = m_particule->CreateParticule(m_lens[i].pos, speed, dim, m_lens[i].type, 1.0f, 0.0f, 0.0f);
            }
        }
    }

    return TRUE;
}

// Stops the controller.

BOOL CAutoHuston::Abort()
{
    return TRUE;
}


// Creates all the interface when the object is selected.

BOOL CAutoHuston::CreateInterface(BOOL bSelect)
{
    CWindow*    pw;
    FPOINT      pos, ddim;
    float       ox, oy, sx, sy;

    CAuto::CreateInterface(bSelect);

    if ( !bSelect )  return TRUE;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW0);
    if ( pw == 0 )  return FALSE;

    ox = 3.0f/640.0f;
    oy = 3.0f/480.0f;
    sx = 33.0f/640.0f;
    sy = 33.0f/480.0f;

    pos.x = ox+sx*0.0f;
    pos.y = oy+sy*0;
    ddim.x = 66.0f/640.0f;
    ddim.y = 66.0f/480.0f;
    pw->CreateGroup(pos, ddim, 115, EVENT_OBJECT_TYPE);

    return TRUE;
}


// Returns an error due to state of the automation.

Error CAutoHuston::RetError()
{
    return ERR_OK;
}

