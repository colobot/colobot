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
#include "cmdtoken.h"
#include "sound.h"
#include "auto.h"
#include "automush.h"




// Object's constructor.

CAutoMush::CAutoMush(CInstanceManager* iMan, CObject* object)
                     : CAuto(iMan, object)
{
    Init();
}

// Object's destructor.

CAutoMush::~CAutoMush()
{
    this->CAuto::~CAuto();
}


// Destroys the object.

void CAutoMush::DeleteObject(BOOL bAll)
{
    CAuto::DeleteObject(bAll);
}


// Initialize the object.

void CAutoMush::Init()
{
    m_phase    = AMP_WAIT;
    m_progress = 0.0f;
    m_speed    = 1.0f/4.0f;

    m_time     = 0.0f;
    m_lastParticule = 0.0f;
}


// Management of an event.

BOOL CAutoMush::EventProcess(const Event &event)
{
    D3DVECTOR   pos, speed, dir;
    FPOINT      dim;
    float       factor, zoom, size, angle;
    int         i, channel;

    CAuto::EventProcess(event);

    if ( m_engine->RetPause() )  return TRUE;
    if ( event.event != EVENT_FRAME )  return TRUE;

    m_progress += event.rTime*m_speed;

    factor = 0.0f;
    size = 1.0f;

    if ( m_phase == AMP_WAIT )
    {
        if ( m_progress >= 1.0f )
        {
            if ( !SearchTarget() )
            {
                m_phase    = AMP_WAIT;
                m_progress = 0.0f;
                m_speed    = 1.0f/(2.0f+Rand()*2.0f);
            }
            else
            {
                m_phase    = AMP_SNIF;
                m_progress = 0.0f;
                m_speed    = 1.0f/1.5f;
            }
        }
    }

    if ( m_phase == AMP_SNIF )
    {
        if ( m_progress < 1.0f )
        {
            factor = m_progress;
        }
        else
        {
            m_phase    = AMP_ZOOM;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.0f;
        }
    }

    if ( m_phase == AMP_ZOOM )
    {
        if ( m_progress < 1.0f )
        {
            factor = 1.0f;
            size = 1.0f+m_progress*0.3f;
        }
        else
        {
            m_sound->Play(SOUND_MUSHROOM, m_object->RetPosition(0));

            m_phase    = AMP_FIRE;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.0f;
        }
    }

    if ( m_phase == AMP_FIRE )
    {
        if ( m_progress < 1.0f )
        {
            factor = 1.0f-m_progress;
            size = 1.0f+(1.0f-m_progress)*0.3f;

            if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
            {
                m_lastParticule = m_time;

                for ( i=0 ; i<10 ; i++ )
                {
                    pos = m_object->RetPosition(0);
                    pos.y += 5.0f;
                    speed.x = (Rand()-0.5f)*200.0f;
                    speed.z = (Rand()-0.5f)*200.0f;
                    speed.y = -(20.0f+Rand()*20.0f);
                    dim.x = 1.0f;
                    dim.y = dim.x;
                    channel = m_particule->CreateParticule(pos, speed, dim, PARTIGUN2, 2.0f, 100.0f, 0.0f);
                    m_particule->SetObjectFather(channel, m_object);
                }
            }
        }
        else
        {
            m_phase    = AMP_SMOKE;
            m_progress = 0.0f;
            m_speed    = 1.0f/2.0f;
        }
    }

    if ( m_phase == AMP_SMOKE )
    {
        if ( m_progress < 1.0f )
        {
            if ( m_lastParticule+m_engine->ParticuleAdapt(0.10f) <= m_time )
            {
                m_lastParticule = m_time;

                pos = m_object->RetPosition(0);
                pos.y += 5.0f;
                speed.x = (Rand()-0.5f)*4.0f;
                speed.z = (Rand()-0.5f)*4.0f;
                speed.y = -(0.5f+Rand()*0.5f);
                dim.x = Rand()*2.5f+2.0f;
                dim.y = dim.x;
                m_particule->CreateParticule(pos, speed, dim, PARTISMOKE3, 4.0f, 0.0f, 0.0f);
            }
        }
        else
        {
            m_phase    = AMP_WAIT;
            m_progress = 0.0f;
            m_speed    = 1.0f/(2.0f+Rand()*2.0f);
        }
    }

    if ( factor != 0.0f || size != 1.0f )
    {
        dir.x = sinf(m_time*PI*4.0f);
        dir.z = cosf(m_time*PI*4.0f);

        angle = sinf(m_time*10.0f)*factor*0.04f;
        m_object->SetAngleX(0, angle*dir.z);
        m_object->SetAngleZ(0, angle*dir.x);

        zoom = 1.0f+sinf(m_time*8.0f)*factor*0.06f;
        m_object->SetZoomX(0, zoom*size);
        zoom = 1.0f+sinf(m_time*5.0f)*factor*0.06f;
        m_object->SetZoomY(0, zoom*size);
        zoom = 1.0f+sinf(m_time*7.0f)*factor*0.06f;
        m_object->SetZoomZ(0, zoom*size);
    }
    else
    {
        m_object->SetAngleX(0, 0.0f);
        m_object->SetAngleZ(0, 0.0f);
        m_object->SetZoom(0, D3DVECTOR(1.0f, 1.0f, 1.0f));
    }

    return TRUE;
}


// Seeking a nearby target.

BOOL CAutoMush::SearchTarget()
{
    CObject*    pObj;
    D3DVECTOR   iPos, oPos;
    ObjectType  type;
    float       dist;
    int         i;

    iPos = m_object->RetPosition(0);

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        if ( pObj->RetLock() )  continue;

        type = pObj->RetType();
        if ( type != OBJECT_MOBILEfa &&
             type != OBJECT_MOBILEta &&
             type != OBJECT_MOBILEwa &&
             type != OBJECT_MOBILEia &&
             type != OBJECT_MOBILEfc &&
             type != OBJECT_MOBILEtc &&
             type != OBJECT_MOBILEwc &&
             type != OBJECT_MOBILEic &&
             type != OBJECT_MOBILEfi &&
             type != OBJECT_MOBILEti &&
             type != OBJECT_MOBILEwi &&
             type != OBJECT_MOBILEii &&
             type != OBJECT_MOBILEfs &&
             type != OBJECT_MOBILEts &&
             type != OBJECT_MOBILEws &&
             type != OBJECT_MOBILEis &&
             type != OBJECT_MOBILErt &&
             type != OBJECT_MOBILErc &&
             type != OBJECT_MOBILErr &&
             type != OBJECT_MOBILErs &&
             type != OBJECT_MOBILEsa &&
             type != OBJECT_MOBILEtg &&
             type != OBJECT_MOBILEft &&
             type != OBJECT_MOBILEtt &&
             type != OBJECT_MOBILEwt &&
             type != OBJECT_MOBILEit &&
             type != OBJECT_MOBILEdr &&
             type != OBJECT_DERRICK  &&
             type != OBJECT_STATION  &&
             type != OBJECT_FACTORY  &&
             type != OBJECT_REPAIR   &&
             type != OBJECT_DESTROYER&&
             type != OBJECT_CONVERT  &&
             type != OBJECT_TOWER    &&
             type != OBJECT_RESEARCH &&
             type != OBJECT_RADAR    &&
             type != OBJECT_INFO     &&
             type != OBJECT_ENERGY   &&
             type != OBJECT_LABO     &&
             type != OBJECT_NUCLEAR  &&
             type != OBJECT_PARA     &&
             type != OBJECT_HUMAN    )  continue;

        oPos = pObj->RetPosition(0);
        dist = Length(oPos, iPos);
        if ( dist < 50.0f )  return TRUE;
    }

    return FALSE;
}


// Returns an error due the state of the automation.

Error CAutoMush::RetError()
{
    return ERR_OK;
}


// Saves all parameters of the controller.

BOOL CAutoMush::Write(char *line)
{
    D3DVECTOR   pos;
    char        name[100];

    if ( m_phase == AMP_WAIT )  return FALSE;

    sprintf(name, " aExist=%d", 1);
    strcat(line, name);

    CAuto::Write(line);

    sprintf(name, " aPhase=%d", m_phase);
    strcat(line, name);

    sprintf(name, " aProgress=%.2f", m_progress);
    strcat(line, name);

    sprintf(name, " aSpeed=%.2f", m_speed);
    strcat(line, name);

    return TRUE;
}

// Restores all parameters of the controller.

BOOL CAutoMush::Read(char *line)
{
    D3DVECTOR   pos;

    if ( OpInt(line, "aExist", 0) == 0 )  return FALSE;

    CAuto::Read(line);

    m_phase = (AutoMushPhase)OpInt(line, "aPhase", AMP_WAIT);
    m_progress = OpFloat(line, "aProgress", 0.0f);
    m_speed = OpFloat(line, "aSpeed", 1.0f);

    m_lastParticule = 0.0f;

    return TRUE;
}


