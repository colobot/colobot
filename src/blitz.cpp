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
#include "d3dutil.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "terrain.h"
#include "math3d.h"
#include "object.h"
#include "camera.h"
#include "auto.h"
#include "autopara.h"
#include "sound.h"
#include "blitz.h"




// Constructor of the terrain.

CBlitz::CBlitz(CInstanceManager* iMan, CD3DEngine* engine)
{
    m_iMan = iMan;
    m_iMan->AddInstance(CLASS_BLITZ, this);

    m_engine = engine;
    m_terrain = 0;
    m_camera = 0;
    m_sound = 0;
    Flush();
}

// Destructor of the terrain.

CBlitz::~CBlitz()
{
}


// Removes lightning.

void CBlitz::Flush()
{
    int     i;

    m_bBlitzExist = FALSE;
    m_time = 0.0f;
    m_phase = BPH_WAIT;
    m_speed = 0.0f;
    m_progress = 0.0f;

    for ( i=0 ; i<BLITZMAX ; i++ )
    {
        m_shift[i] = FPOINT(0.0f, 0.0f);
        m_width[i] = 1.0f;
    }
}


// Management of an event

BOOL CBlitz::EventProcess(const Event &event)
{
    if ( event.event == EVENT_FRAME )
    {
        return EventFrame(event);
    }
    return TRUE;
}

// Evolved lightning.

BOOL CBlitz::EventFrame(const Event &event)
{
    CObject*    pObj;
    CAutoPara*  automat;
    ObjectType  type;
    D3DVECTOR   eye, pos;
    float       dist, deep, max;
    int         i;

    if ( m_engine->RetPause() )  return TRUE;
    if ( m_engine->RetMovieLock() )  return TRUE;

    m_time += event.rTime;
    m_progress += event.rTime*m_speed;

    if ( m_phase == BPH_WAIT )
    {
        if ( m_progress >= 1.0f )
        {
#if 1
            m_pos.x = (Rand()-0.5f)*(3200.0f-200.0f);
            m_pos.z = (Rand()-0.5f)*(3200.0f-200.0f);
#else
            m_pos.x = (Rand()-0.5f)*(3200.0f-2800.0f);
            m_pos.z = (Rand()-0.5f)*(3200.0f-2800.0f);
#endif
            m_pos.y = 0.0f;

            pObj = SearchObject(m_pos);
            if ( pObj == 0 )
            {
                m_terrain->MoveOnFloor(m_pos, TRUE);
            }
            else
            {
                m_pos = pObj->RetPosition(0);
                m_terrain->MoveOnFloor(m_pos, TRUE);

                type = pObj->RetType();
                if ( type == OBJECT_BASE )
                {
                    m_pos.y += 120.0f;  // top of the rocket
                }
                else if ( type == OBJECT_PARA )
                {
                    automat = (CAutoPara*)pObj->RetAuto();
                    if ( automat != 0 )
                    {
                        automat->StartBlitz();
                    }
                    m_pos.y += 67.0f;  // top of lightning rod
                }
                else
                {
                    pObj->ExploObject(EXPLO_BOUM, 1.0f);
                }
            }

            eye = m_engine->RetEyePt();
            dist = Length(m_pos, eye);
            deep = m_engine->RetDeepView();

            if ( dist < deep )
            {
                pos = eye+((m_pos-eye)*0.2f);  // like so close!
                m_sound->Play(SOUND_BLITZ, pos);

                m_camera->StartOver(OE_BLITZ, m_pos, 1.0f);

                m_phase    = BPH_BLITZ;
                m_progress = 0.0f;
                m_speed    = 1.0f/1.0f;
            }
        }
    }

    if ( m_phase == BPH_BLITZ )
    {
        if ( m_progress < 1.0f )
        {
            max = 5.0f;
            for ( i=0 ; i<BLITZMAX ; i++ )
            {
                max += 0.4f;

                m_shift[i].x += (Rand()-0.5f)*max*2.0f;
                if ( m_shift[i].x < -max )  m_shift[i].x = -max;
                if ( m_shift[i].x >  max )  m_shift[i].x =  max;

                m_shift[i].y += (Rand()-0.5f)*max*2.0f;
                if ( m_shift[i].y < -max )  m_shift[i].y = -max;
                if ( m_shift[i].y >  max )  m_shift[i].y =  max;

                m_width[i] += (Rand()-0.5f)*2.0f;
                if ( m_width[i] < 1.0f )  m_width[i] = 1.0f;
                if ( m_width[i] > 6.0f )  m_width[i] = 6.0f;
            }
            m_shift[0].x = 0.0f;
            m_shift[0].y = 0.0f;
            m_width[0]   = 0.0f;
        }
        else
        {
            m_phase    = BPH_WAIT;
            m_progress = 0.0f;
            m_speed    = 1.0f/(1.0f+Rand()*m_delay);
        }
    }

    return TRUE;
}


// Draw lightning.

void CBlitz::Draw()
{
    LPDIRECT3DDEVICE7 device;
    D3DVERTEX2  vertex[4];  // 2 triangles
    D3DVECTOR   corner[4], eye, n, p, p1, p2;
    D3DMATRIX   matrix;
    FPOINT      texInf, texSup, rot;
    float       a;
    int         i;

    if ( !m_bBlitzExist )  return;
    if ( m_phase != BPH_BLITZ )  return;

    device = m_engine->RetD3DDevice();
    device->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);

    D3DUtil_SetIdentityMatrix(matrix);
    device->SetTransform(D3DTRANSFORMSTATE_WORLD, &matrix);

    m_engine->SetTexture("effect00.tga");
    m_engine->SetState(D3DSTATETTb);
    texInf.x = 64.5f/256.0f;
    texInf.y = 33.0f/256.0f;
    texSup.x = 95.5f/256.0f;
    texSup.y = 34.0f/256.0f;  // blank

    p1 = m_pos;
    eye = m_engine->RetEyePt();
    a = RotateAngle(eye.x-p1.x, eye.z-p1.z);
    n = Normalize(p1-eye);

    for ( i=0 ; i<BLITZMAX-1 ; i++ )
    {
        p2 = p1;
        p2.y += 8.0f+0.2f*i;

        p = p1;
        p.x += m_width[i];
        rot = RotatePoint(FPOINT(p1.x, p1.z), a+PI/2.0f, FPOINT(p.x, p.z));
        corner[0].x = rot.x+m_shift[i].x;
        corner[0].y = p1.y;
        corner[0].z = rot.y+m_shift[i].y;
        rot = RotatePoint(FPOINT(p1.x, p1.z), a-PI/2.0f, FPOINT(p.x, p.z));
        corner[1].x = rot.x+m_shift[i].x;
        corner[1].y = p1.y;
        corner[1].z = rot.y+m_shift[i].y;

        p = p2;
        p.x += m_width[i+1];
        rot = RotatePoint(FPOINT(p2.x, p2.z), a+PI/2.0f, FPOINT(p.x, p.z));
        corner[2].x = rot.x+m_shift[i+1].x;
        corner[2].y = p2.y;
        corner[2].z = rot.y+m_shift[i+1].y;
        rot = RotatePoint(FPOINT(p2.x, p2.z), a-PI/2.0f, FPOINT(p.x, p.z));
        corner[3].x = rot.x+m_shift[i+1].x;
        corner[3].y = p2.y;
        corner[3].z = rot.y+m_shift[i+1].y;

        if ( p2.y < p1.y )
        {
            vertex[0] = D3DVERTEX2(corner[1], n, texSup.x, texSup.y);
            vertex[1] = D3DVERTEX2(corner[0], n, texInf.x, texSup.y);
            vertex[2] = D3DVERTEX2(corner[3], n, texSup.x, texInf.y);
            vertex[3] = D3DVERTEX2(corner[2], n, texInf.x, texInf.y);
        }
        else
        {
            vertex[0] = D3DVERTEX2(corner[0], n, texSup.x, texSup.y);
            vertex[1] = D3DVERTEX2(corner[1], n, texInf.x, texSup.y);
            vertex[2] = D3DVERTEX2(corner[2], n, texSup.x, texInf.y);
            vertex[3] = D3DVERTEX2(corner[3], n, texInf.x, texInf.y);
        }

        device->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 4, NULL);
        m_engine->AddStatisticTriangle(2);

        p1 = p2;
    }

    device->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
}


// Triggers lightning.

BOOL CBlitz::Create(float sleep, float delay, float magnetic)
{
    m_bBlitzExist = TRUE;
    if ( sleep < 1.0f )  sleep = 1.0f;
    m_sleep = sleep;
    m_delay = delay;
    m_magnetic = magnetic;

    m_phase    = BPH_WAIT;
    m_progress = 0.0f;
    m_speed    = 1.0f/m_sleep;

    if ( m_terrain == 0 )
    {
        m_terrain = (CTerrain*)m_iMan->SearchInstance(CLASS_TERRAIN);
    }

    if ( m_camera == 0 )
    {
        m_camera = (CCamera*)m_iMan->SearchInstance(CLASS_CAMERA);
    }

    if ( m_sound == 0 )
    {
        m_sound = (CSound*)m_iMan->SearchInstance(CLASS_SOUND);
    }

    return FALSE;
}


// Gives the status of lightning.

BOOL CBlitz::GetStatus(float &sleep, float &delay, float &magnetic, float &progress)
{
    if ( !m_bBlitzExist )  return FALSE;

    sleep = m_sleep;
    delay = m_delay;
    magnetic = m_magnetic;
    progress = m_progress;

    return TRUE;
}

// Specifies the status of lightning.

BOOL CBlitz::SetStatus(float sleep, float delay, float magnetic, float progress)
{
    m_bBlitzExist = TRUE;

    m_sleep = sleep;
    m_delay = delay;
    m_magnetic = magnetic;
    m_progress = progress;
    m_phase = BPH_WAIT;
    m_speed = 1.0f/m_sleep;

    return TRUE;
}


// Seeking the object closest to the lightning.

CObject* CBlitz::SearchObject(D3DVECTOR pos)
{
    CObject     *pObj, *pBest, *pObjPara[100];
    D3DVECTOR   oPos, pPos[100];
    ObjectType  type;
    float       min, dist, detect;
    int         i, nbPara;

    // Seeking the object closest to the point of impact of lightning.
    pBest = 0;
    min = 100000.0f;
    nbPara = 0;
    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        if ( !pObj->RetActif() )  continue;  // inactive object?
        if ( pObj->RetTruck() != 0 )  continue;  // object transported?

        type = pObj->RetType();
        if ( type == OBJECT_BASE ||
             type == OBJECT_PARA )  // building a lightning effect?
        {
            pObjPara[nbPara] = pObj;
            pPos[nbPara] = pObj->RetPosition(0);
            nbPara ++;
        }

        detect = 0.0f;
        if ( type == OBJECT_BASE     ||
             type == OBJECT_DERRICK  ||
             type == OBJECT_FACTORY  ||
             type == OBJECT_REPAIR   ||
             type == OBJECT_DESTROYER||
             type == OBJECT_STATION  ||
             type == OBJECT_CONVERT  ||
             type == OBJECT_TOWER    ||
             type == OBJECT_RESEARCH ||
             type == OBJECT_RADAR    ||
             type == OBJECT_INFO     ||
             type == OBJECT_ENERGY   ||
             type == OBJECT_LABO     ||
             type == OBJECT_NUCLEAR  ||
             type == OBJECT_PARA     ||
             type == OBJECT_SAFE     ||
             type == OBJECT_HUSTON   )
        {
            detect = m_magnetic;
        }
        if ( type == OBJECT_METAL    ||
             type == OBJECT_POWER    ||
             type == OBJECT_ATOMIC   )
        {
            detect = m_magnetic*0.3f;
        }
        if ( type == OBJECT_MOBILEfa ||
             type == OBJECT_MOBILEta ||
             type == OBJECT_MOBILEwa ||
             type == OBJECT_MOBILEia ||
             type == OBJECT_MOBILEfc ||
             type == OBJECT_MOBILEtc ||
             type == OBJECT_MOBILEwc ||
             type == OBJECT_MOBILEic ||
             type == OBJECT_MOBILEfi ||
             type == OBJECT_MOBILEti ||
             type == OBJECT_MOBILEwi ||
             type == OBJECT_MOBILEii ||
             type == OBJECT_MOBILEfs ||
             type == OBJECT_MOBILEts ||
             type == OBJECT_MOBILEws ||
             type == OBJECT_MOBILEis ||
             type == OBJECT_MOBILErt ||
             type == OBJECT_MOBILErc ||
             type == OBJECT_MOBILErr ||
             type == OBJECT_MOBILErs ||
             type == OBJECT_MOBILEsa ||
             type == OBJECT_MOBILEft ||
             type == OBJECT_MOBILEtt ||
             type == OBJECT_MOBILEwt ||
             type == OBJECT_MOBILEit ||
             type == OBJECT_MOBILEdr )
        {
            detect = m_magnetic*0.5f;
        }
        if ( detect == 0.0f )  continue;

        oPos = pObj->RetPosition(0);
        dist = Length2d(oPos, pos);
        if ( dist > detect )  continue;
        if ( dist < min )
        {
            min = dist;
            pBest = pObj;
        }
    }
    if ( pBest == 0 )  return 0;  // nothing found

    // Under the protection of a lightning conductor?
    oPos = pBest->RetPosition(0);
    for ( i=nbPara-1 ; i>=0 ; i-- )
    {
        dist = Length2d(oPos, pPos[i]);
        if ( dist <= BLITZPARA )
        {
            return pObjPara[i];
        }
    }
    return pBest;
}

