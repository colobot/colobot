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
#include "gauge.h"
#include "sound.h"
#include "auto.h"
#include "autoradar.h"




// Object's constructor.

CAutoRadar::CAutoRadar(CInstanceManager* iMan, CObject* object)
                     : CAuto(iMan, object)
{
    Init();
    m_phase = ARAP_WAIT;
    m_totalDetect = 0;
}

// Object's destructor.

CAutoRadar::~CAutoRadar()
{
    this->CAuto::~CAuto();
}


// Destroys the object.

void CAutoRadar::DeleteObject(BOOL bAll)
{
    CAuto::DeleteObject(bAll);
}


// Initialize the object.

void CAutoRadar::Init()
{
    m_phase    = ARAP_SEARCH;
    m_progress = 0.0f;
    m_speed    = 1.0f/3.0f;

    m_aTime    = 0.0f;
    m_time     = 0.0f;
    m_timeVirus = 0.0f;
}


// Management of an event.

BOOL CAutoRadar::EventProcess(const Event &event)
{
    D3DVECTOR   pos, ePos;
    float       speed, angle, prog, freq, ampl;

    CAuto::EventProcess(event);

    if ( m_engine->RetPause() )  return TRUE;
    if ( event.event != EVENT_FRAME )  return TRUE;
    if ( m_phase == ARAP_WAIT )  return TRUE;

    m_progress += event.rTime*m_speed;
    m_aTime += event.rTime;
    m_timeVirus -= event.rTime;

    if ( m_object->RetVirusMode() )  // contaminated by a virus?
    {
        if ( m_timeVirus <= 0.0f )
        {
            m_timeVirus = 0.1f+Rand()*0.3f;

            angle = m_object->RetAngleY(1);
            angle += (Rand()-0.2f)*0.5f;
            m_object->SetAngleY(1, angle);

            angle = m_object->RetAngleY(2);
            angle += (Rand()-0.8f)*1.0f;
            m_object->SetAngleY(2, angle);

            m_object->SetAngleX(3, (Rand()-0.5f)*0.3f);

            m_totalDetect = (int)(Rand()*10.0f);
            UpdateInterface();
        }
        return TRUE;
    }

    if ( m_phase == ARAP_SEARCH )
    {
        if ( m_progress < 1.0f )
        {
            speed = Min(10.0f, m_progress*50.0f);
            angle = m_object->RetAngleY(1);
            angle += event.rTime*speed;
            m_object->SetAngleY(1, angle);
        }
        else
        {
            if ( !SearchEnemy(ePos) )
            {
                m_phase    = ARAP_SEARCH;
                m_progress = 10.0f/50.0f;  // full speed immediately
                m_speed    = 1.0f/3.0f;
            }
            else
            {
                pos = m_object->RetPosition(0);
                m_start = m_object->RetAngleY(1);
                m_angle = m_start-NormAngle(m_start)+PI*2.0f;
                m_angle += RotateAngle(pos.x-ePos.x, ePos.z-pos.z);
                m_angle += PI-m_object->RetAngleY(0);

                m_phase    = ARAP_SHOW;
                m_progress = 0.0f;
                m_speed    = 1.0f/(Abs(m_angle-m_start)/10.0f);
            }
        }
    }

    if ( m_phase == ARAP_SHOW )
    {
        if ( m_progress < 1.0f )
        {
            angle = m_start + (m_angle-m_start)*m_progress;
            m_object->SetAngleY(1, angle);
        }
        else
        {
            m_sound->Play(SOUND_RADAR, m_object->RetPosition(0));

            m_phase    = ARAP_SINUS;
            m_progress = 0.0f;
            m_speed    = 1.0f/4.0f;
            m_time     = 0.0f;
        }
    }

    if ( m_phase == ARAP_SINUS )
    {
        if ( m_progress < 1.0f )
        {
            prog = Min(1.0f, m_progress*2.0f);
            freq = 16.0f*(prog+1.0f);
            ampl = 0.2f-prog*0.2f;
            angle = m_angle + sinf(m_time*freq)*ampl;
            m_object->SetAngleY(1, angle);
        }
        else
        {
            m_phase    = ARAP_SEARCH;
            m_progress = 0.0f;
            m_speed    = 1.0f/3.0f;
        }
    }

    angle = -m_aTime*2.0f;
    m_object->SetAngleY(2, angle);

    angle = sinf(m_aTime*4.0f)*0.3f;
    m_object->SetAngleX(3, angle);

    return TRUE;
}


// Returns an error due the state of the automation.

Error CAutoRadar::RetError()
{
    if ( m_object->RetVirusMode() )
    {
        return ERR_BAT_VIRUS;
    }

    return ERR_OK;
}


// Creates all the interface when the object is selected.

BOOL CAutoRadar::CreateInterface(BOOL bSelect)
{
    CWindow*    pw;
    FPOINT      pos, dim, ddim;
    float       ox, oy, sx, sy;

    CAuto::CreateInterface(bSelect);

    if ( !bSelect )  return TRUE;

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW0);
    if ( pw == 0 )  return FALSE;

    ox = 3.0f/640.0f;
    oy = 3.0f/480.0f;
    sx = 33.0f/640.0f;
    sy = 33.0f/480.0f;

    pos.x = ox+sx*7.0f;
    pos.y = oy+sy*0.6f;
    dim.x = 160.0f/640.0f;
    dim.y =  26.0f/480.0f;
    pw->CreateGauge(pos, dim, 1, EVENT_OBJECT_GRADAR);

    pos.x = ox+sx*0.0f;
    pos.y = oy+sy*0;
    ddim.x = 66.0f/640.0f;
    ddim.y = 66.0f/480.0f;
    pw->CreateGroup(pos, ddim, 105, EVENT_OBJECT_TYPE);

    UpdateInterface();
    return TRUE;
}

// Updates the status of all interface buttons.

void CAutoRadar::UpdateInterface()
{
    CWindow*    pw;
    CGauge*     pg;
    float       level;

    if ( !m_object->RetSelect() )  return;

    CAuto::UpdateInterface();

    pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW0);
    if ( pw == 0 )  return;

    pg = (CGauge*)pw->SearchControl(EVENT_OBJECT_GRADAR);
    if ( pg != 0 )
    {
        level = (float)m_totalDetect*(1.0f/8.0f);
        if ( level > 1.0f )  level = 1.0f;
        pg->SetLevel(level);
    }
}


// Seeking the position of an enemy.

BOOL CAutoRadar::SearchEnemy(D3DVECTOR &pos)
{
    CObject*    pObj;
    CObject*    pBest = 0;
    D3DVECTOR   iPos, oPos;
    ObjectType  oType;
    float       distance, min;
    int         i;

    iPos = m_object->RetPosition(0);
    min = 1000000.0f;
    m_totalDetect = 0;

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        if ( !pObj->RetActif() )  continue;

        oType = pObj->RetType();
        if ( oType != OBJECT_ANT    &&
             oType != OBJECT_SPIDER &&
             oType != OBJECT_BEE    &&
             oType != OBJECT_WORM   &&
             oType != OBJECT_MOTHER )  continue;

        m_totalDetect ++;

        oPos = pObj->RetPosition(0);
        distance = Length(oPos, iPos);
        if ( distance < min )
        {
            min = distance;
            pBest = pObj;
        }
    }

    UpdateInterface();

    if ( pBest == 0 )  return FALSE;
    pos = pBest->RetPosition(0);
    return TRUE;
}


