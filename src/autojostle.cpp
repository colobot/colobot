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
#include "light.h"
#include "terrain.h"
#include "camera.h"
#include "object.h"
#include "interface.h"
#include "button.h"
#include "list.h"
#include "window.h"
#include "sound.h"
#include "auto.h"
#include "autojostle.h"




// Object's constructor.

CAutoJostle::CAutoJostle(CInstanceManager* iMan, CObject* object)
                           : CAuto(iMan, object)
{
    Init();
}

// Object's destructor.

CAutoJostle::~CAutoJostle()
{
    this->CAuto::~CAuto();
}


// Destroys the object.

void CAutoJostle::DeleteObject(BOOL bAll)
{
    CAuto::DeleteObject(bAll);
}


// Initialize the object.

void CAutoJostle::Init()
{
    m_time = 0.0f;
    m_error = ERR_CONTINUE;

    CAuto::Init();
}


// Start an emission.

void CAutoJostle::Start(int param, float force)
{
    ObjectType  type;

    if ( force < 0.0f )  force = 0.0f;
    if ( force > 1.0f )  force = 1.0f;

    m_force = force;
    m_progress = 0.0f;
    m_speed = 1.0f/(0.5f+force*1.0f);  // 0.5 .. 1.5
    m_time = 0.0f;
    m_error = ERR_CONTINUE;

    type = m_object->RetType();
    if ( type >= OBJECT_PLANT5 &&
         type <= OBJECT_PLANT7 )  // clover?
    {
        m_force *= 3.0f;
    }
}


// Management of an event.

BOOL CAutoJostle::EventProcess(const Event &event)
{
    D3DVECTOR   dir;
    float       factor, angle, zoom;

    CAuto::EventProcess(event);

    if ( m_engine->RetPause() )  return TRUE;
    if ( event.event != EVENT_FRAME )  return TRUE;

    if ( m_progress < 1.0f )
    {
        m_progress += event.rTime*m_speed;

        if ( m_progress < 0.5f )
        {
            factor = m_progress/0.5f;
        }
        else
        {
            factor = 2.0f-m_progress/0.5f;
        }
        factor *= m_force;

        dir.x = sinf(m_progress*PI*4.0f);
        dir.z = cosf(m_progress*PI*4.0f);

        angle = sinf(m_time*10.0f)*factor*0.04f;
        m_object->SetAngleX(0, angle*dir.z);
        m_object->SetAngleZ(0, angle*dir.x);

        zoom = 1.0f+sinf(m_time*8.0f)*factor*0.06f;
        m_object->SetZoomX(0, zoom);
        zoom = 1.0f+sinf(m_time*5.0f)*factor*0.06f;
        m_object->SetZoomY(0, zoom);
        zoom = 1.0f+sinf(m_time*7.0f)*factor*0.06f;
        m_object->SetZoomZ(0, zoom);
    }
    else
    {
        m_object->SetAngleX(0, 0.0f);
        m_object->SetAngleZ(0, 0.0f);
        m_object->SetZoom(0, D3DVECTOR(1.0f, 1.0f, 1.0f));
        m_error = ERR_STOP;
    }

    return TRUE;
}


// Indicates whether the controller has completed its activity.

Error CAutoJostle::IsEnded()
{
    return m_error;
}


