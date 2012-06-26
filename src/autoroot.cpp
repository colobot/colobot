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
#include "auto.h"
#include "autoroot.h"




// Object's constructor.

CAutoRoot::CAutoRoot(CInstanceManager* iMan, CObject* object)
                     : CAuto(iMan, object)
{
    Init();
}

// Object's destructor.

CAutoRoot::~CAutoRoot()
{
    this->CAuto::~CAuto();
}


// Destroys the object.

void CAutoRoot::DeleteObject(BOOL bAll)
{
    CAuto::DeleteObject(bAll);
}


// Initialize the object.

void CAutoRoot::Init()
{
    D3DMATRIX*  mat;
    D3DVECTOR   pos, speed;
    FPOINT      dim;

    m_time = 0.0f;
    m_lastParticule = 0.0f;

    mat = m_object->RetWorldMatrix(0);
    pos = D3DVECTOR(-5.0f, 28.0f, -4.0f);  // peak position
    pos = Transform(*mat, pos);
    m_center = pos;

    speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
    dim.x = 100.0f;
    dim.y = dim.x;
    m_particule->CreateParticule(m_center, speed, dim, PARTISPHERE5, 0.5f, 0.0f, 0.0f);

    m_terrain->AddFlyingLimit(pos, 100.0f, 80.0f, pos.y-60.0f);
}


// Management of an event.

BOOL CAutoRoot::EventProcess(const Event &event)
{
    D3DVECTOR   pos, speed;
    FPOINT      dim;

    CAuto::EventProcess(event);

    if ( m_engine->RetPause() )  return TRUE;
    if ( event.event != EVENT_FRAME )  return TRUE;

    m_object->SetZoomX(1, 1.0f+sinf(m_time*2.0f)*0.2f);
    m_object->SetZoomY(1, 1.0f+sinf(m_time*2.3f)*0.2f);
    m_object->SetZoomZ(1, 1.0f+sinf(m_time*2.7f)*0.2f);

    if ( m_lastParticule+m_engine->ParticuleAdapt(0.10f) <= m_time )
    {
        m_lastParticule = m_time;

        pos = m_center;
        pos.x += (Rand()-0.5f)*8.0f;
        pos.z += (Rand()-0.5f)*8.0f;
        pos.y += 0.0f;
        speed.x = (Rand()-0.5f)*12.0f;
        speed.z = (Rand()-0.5f)*12.0f;
        speed.y = Rand()*12.0f;
        dim.x = Rand()*6.0f+4.0f;
        dim.y = dim.x;
        m_particule->CreateParticule(pos, speed, dim, PARTIROOT, 1.0f, 0.0f, 0.0f);
    }

    return TRUE;
}


// Returns an error due the state of the automation.

Error CAutoRoot::RetError()
{
    return ERR_OK;
}


