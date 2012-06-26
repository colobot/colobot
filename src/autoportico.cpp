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
#include "displaytext.h"
#include "robotmain.h"
#include "sound.h"
#include "auto.h"
#include "autoportico.h"



#define PARAM_DEPOSE        2       // run=2 -> deposits the spaceship

#define PORTICO_POSa        75.0f
#define PORTICO_POSb        65.0f
#define PORTICO_ANGLE1a     ( 25.0f*PI/180.0f)
#define PORTICO_ANGLE1b     ( 70.0f*PI/180.0f)
#define PORTICO_ANGLE2a     (-37.5f*PI/180.0f)
#define PORTICO_ANGLE2b     (-62.5f*PI/180.0f)
#define PORTICO_ANGLE3a     (-77.5f*PI/180.0f)
#define PORTICO_ANGLE3b     (-30.0f*PI/180.0f)

#define PORTICO_TIME_MOVE   16.0f
#define PORTICO_TIME_DOWN    4.0f
#define PORTICO_TIME_OPEN   12.0f




// Si progress=0, return a.
// Si progress=1, return b.

float Progress(float a, float b, float progress)
{
    return a+(b-a)*progress;
}



// Object's constructor.

CAutoPortico::CAutoPortico(CInstanceManager* iMan, CObject* object)
                         : CAuto(iMan, object)
{
    Init();
    m_phase = APOP_WAIT;
    m_soundChannel = -1;
}

// Object's destructor.

CAutoPortico::~CAutoPortico()
{
    this->CAuto::~CAuto();
}


// Destroys the object.

void CAutoPortico::DeleteObject(BOOL bAll)
{
    if ( m_soundChannel != -1 )
    {
        m_sound->FlushEnvelope(m_soundChannel);
        m_sound->AddEnvelope(m_soundChannel, 0.0f, 1.0f, 1.0f, SOPER_STOP);
        m_soundChannel = -1;
    }

    CAuto::DeleteObject(bAll);
}


// Initialize the object.

void CAutoPortico::Init()
{
    m_time = 0.0f;
    m_lastParticule = 0.0f;
    m_posTrack = 0.0f;

    m_phase    = APOP_WAIT;
    m_progress = 0.0f;
    m_speed    = 1.0f/2.0f;

    m_cameraProgress = 0.0f;
    m_cameraSpeed    = 1.0f/(PORTICO_TIME_MOVE-2.0f);
}


// Starts the object.

void CAutoPortico::Start(int param)
{
    D3DVECTOR   pos;

    pos = m_object->RetPosition(0);
    m_finalPos = pos;
    pos.z += PORTICO_TIME_MOVE*5.0f;  // back to start
    m_object->SetPosition(0, pos);
    m_finalPos.z += PORTICO_TIME_OPEN*5.3f;

    m_object->SetPosition(1, D3DVECTOR(0.0f, PORTICO_POSa, 0.0f));
    m_object->SetAngleY(2,  PORTICO_ANGLE1a);
    m_object->SetAngleY(3,  PORTICO_ANGLE2a);
    m_object->SetAngleY(4,  PORTICO_ANGLE3a);
    m_object->SetAngleY(5, -PORTICO_ANGLE1a);
    m_object->SetAngleY(6, -PORTICO_ANGLE2a);
    m_object->SetAngleY(7, -PORTICO_ANGLE3a);

    m_phase    = APOP_START;
    m_progress = 0.0f;
    m_speed    = 1.0f/1.0f;

    m_param = param;
}


// Management of an event.

BOOL CAutoPortico::EventProcess(const Event &event)
{
    CObject*    pObj;
    D3DVECTOR   pos;
    float       angle;

    CAuto::EventProcess(event);

    if ( m_engine->RetPause() )  return TRUE;

    if ( m_phase == APOP_START )
    {
        if ( m_param == PARAM_DEPOSE )  // deposits the ship?
        {
            m_startPos = m_object->RetPosition(0);

            m_soundChannel = m_sound->Play(SOUND_MOTORr, m_object->RetPosition(0), 0.0f, 0.3f, TRUE);
            m_sound->AddEnvelope(m_soundChannel, 0.5f, 0.6f, 0.5f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 0.5f, 0.6f, PORTICO_TIME_MOVE-0.5f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 0.0f, 0.3f, 0.5f, SOPER_STOP);

            m_phase    = APOP_MOVE;
            m_progress = 0.0f;
            m_speed    = 1.0f/PORTICO_TIME_MOVE;

            m_main->SetMovieLock(TRUE);  // blocks everything until the end of the landing

            m_camera->SetType(CAMERA_SCRIPT);

            pos = m_startPos;
            pos.x += -100.0f;
            pos.y +=    9.0f;
            pos.z += -200.0f;
            m_camera->SetScriptEye(pos);

            pos = m_object->RetPosition(0);
            pos.x +=   0.0f;
            pos.y +=  10.0f;
            pos.z += -40.0f;
            m_camera->SetScriptLookat(pos);

            m_camera->FixCamera();
        }
    }

    angle = -m_time*1.0f;
    m_object->SetAngleY(8, angle);  // rotates the radar right
    angle = sinf(m_time*4.0f)*0.3f;
    m_object->SetAngleX(9, angle);

    angle = -m_time*1.0f+PI/2.3f;
    m_object->SetAngleY(10, angle);  // turns the left side radar
    angle = sinf(m_time*4.0f)*0.3f;
    m_object->SetAngleX(11, angle);

    if ( event.event != EVENT_FRAME )  return TRUE;
    if ( m_phase == APOP_WAIT )  return TRUE;

    m_progress += event.rTime*m_speed;
    m_cameraProgress += event.rTime*m_cameraSpeed;

    if ( m_phase == APOP_MOVE )
    {
        if ( m_progress < 1.0f )
        {
            pos = m_object->RetPosition(0);
            pos.z -= event.rTime*5.0f;  // advance
            m_object->SetPosition(0, pos);

            m_posTrack += event.rTime*0.5f;
            UpdateTrackMapping(m_posTrack, m_posTrack);
        }
        else
        {
            m_phase    = APOP_WAIT1;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.0f;
        }
    }

    if ( m_phase == APOP_WAIT1 )
    {
        if ( m_progress >= 1.0f )
        {
            m_soundChannel = m_sound->Play(SOUND_MANIP, m_object->RetPosition(0), 0.0f, 0.3f, TRUE);
            m_sound->AddEnvelope(m_soundChannel, 0.3f, 0.5f, 1.0f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 0.3f, 0.6f, PORTICO_TIME_DOWN-1.5f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 0.0f, 0.3f, 1.0f, SOPER_STOP);

            m_phase    = APOP_DOWN;
            m_progress = 0.0f;
            m_speed    = 1.0f/PORTICO_TIME_DOWN;
        }
    }

    if ( m_phase == APOP_DOWN )
    {
        if ( m_progress < 1.0f )
        {
            pos.x = 0.0f;
            pos.y = Progress(PORTICO_POSa, PORTICO_POSb, m_progress);
            pos.z = 0.0f;
            m_object->SetPosition(1, pos);
        }
        else
        {
            pos.x = 0.0f;
            pos.y = PORTICO_POSb;
            pos.z = 0.0f;
            m_object->SetPosition(1, pos);

            m_phase    = APOP_WAIT2;
            m_progress = 0.0f;
            m_speed    = 1.0f/1.0f;
        }
    }

    if ( m_phase == APOP_WAIT2 )
    {
        if ( m_progress >= 1.0f )
        {
            m_soundChannel = m_sound->Play(SOUND_MANIP, m_object->RetPosition(0), 0.0f, 0.5f, TRUE);
            m_sound->AddEnvelope(m_soundChannel, 0.5f, 1.0f, 0.5f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 0.5f, 1.0f, PORTICO_TIME_OPEN/2.0f-0.5f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 0.0f, 0.5f, 0.5f, SOPER_STOP);

            m_soundChannel = m_sound->Play(SOUND_MOTORr, m_object->RetPosition(0), 0.0f, 0.3f, TRUE);
            m_sound->AddEnvelope(m_soundChannel, 0.5f, 0.6f, 0.5f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 0.5f, 0.6f, PORTICO_TIME_OPEN-0.5f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 0.0f, 0.3f, 0.5f, SOPER_STOP);

            m_phase    = APOP_OPEN;
            m_progress = 0.0f;
            m_speed    = 1.0f/PORTICO_TIME_OPEN;
        }
    }

    if ( m_phase == APOP_OPEN )
    {
        if ( m_progress < 1.0f )
        {
            pos = m_object->RetPosition(0);
            pos.z += event.rTime*5.3f;  // back
            m_object->SetPosition(0, pos);

            m_posTrack -= event.rTime*1.0f;
            UpdateTrackMapping(m_posTrack, m_posTrack);

            if ( m_progress < 0.5f )
            {
                angle = Progress(PORTICO_ANGLE1a, PORTICO_ANGLE1b, m_progress/0.5f);
                m_object->SetAngleY(2,  angle);
                m_object->SetAngleY(5, -angle);
                angle = Progress(PORTICO_ANGLE2a, PORTICO_ANGLE2b, m_progress/0.5f);
                m_object->SetAngleY(3,  angle);
                m_object->SetAngleY(6, -angle);
                angle = Progress(PORTICO_ANGLE3a, PORTICO_ANGLE3b, m_progress/0.5f);
                m_object->SetAngleY(4,  angle);
                m_object->SetAngleY(7, -angle);
            }
            else
            {
                m_object->SetAngleY(2,  PORTICO_ANGLE1b);
                m_object->SetAngleY(3,  PORTICO_ANGLE2b);
                m_object->SetAngleY(4,  PORTICO_ANGLE3b);
                m_object->SetAngleY(5, -PORTICO_ANGLE1b);
                m_object->SetAngleY(6, -PORTICO_ANGLE2b);
                m_object->SetAngleY(7, -PORTICO_ANGLE3b);
            }
        }
        else
        {
            m_main->SetMovieLock(FALSE);  // you can play!

            pObj = m_main->SearchHuman();
            m_main->SelectObject(pObj);
            m_camera->SetObject(pObj);
            m_camera->SetType(CAMERA_BACK);

            m_phase    = APOP_WAIT;
            m_progress = 0.0f;
            m_speed    = 1.0f/2.0f;
        }
    }

    if ( m_soundChannel != -1 )
    {
//?     m_sound->Position(m_soundChannel, m_object->RetPosition(0));
        pos = m_engine->RetEyePt();
        m_sound->Position(m_soundChannel, pos);
    }

    if ( m_cameraProgress < 1.0f )
    {
        if ( m_cameraProgress < 0.5f )
        {
        }
        else
        {
            pos = m_startPos;
            pos.x += -100.0f-(m_cameraProgress-0.5f)*1.0f*120.0f;
            pos.y +=    9.0f;
            pos.z += -200.0f+(m_cameraProgress-0.5f)*1.0f*210.0f;
            m_camera->SetScriptEye(pos);
        }

        pos = m_object->RetPosition(0);
        pos.x +=   0.0f;
        pos.y +=  10.0f;
        pos.z += -40.0f;
        m_camera->SetScriptLookat(pos);
    }

    return TRUE;
}

// Stops the controller.

BOOL CAutoPortico::Abort()
{
    CObject*    pObj;

    m_object->SetPosition(0, m_finalPos);
    m_object->SetPosition(1, D3DVECTOR(0.0f, PORTICO_POSb, 0.0f));
    m_object->SetAngleY(2,  PORTICO_ANGLE1b);
    m_object->SetAngleY(3,  PORTICO_ANGLE2b);
    m_object->SetAngleY(4,  PORTICO_ANGLE3b);
    m_object->SetAngleY(5, -PORTICO_ANGLE1b);
    m_object->SetAngleY(6, -PORTICO_ANGLE2b);
    m_object->SetAngleY(7, -PORTICO_ANGLE3b);

    m_main->SetMovieLock(FALSE);  // you can play!

    pObj = m_main->SearchHuman();
    m_main->SelectObject(pObj);
    m_camera->SetObject(pObj);
    m_camera->SetType(CAMERA_BACK);

    if ( m_soundChannel != -1 )
    {
        m_sound->FlushEnvelope(m_soundChannel);
        m_sound->AddEnvelope(m_soundChannel, 0.0f, 1.0f, 1.0f, SOPER_STOP);
        m_soundChannel = -1;
    }

    m_phase    = APOP_WAIT;
    m_progress = 0.0f;
    m_speed    = 1.0f/2.0f;

    return TRUE;
}


// Returns an error due the state of the automation.

Error CAutoPortico::RetError()
{
    return ERR_OK;
}


// Updates the mapping of the texture of the caterpillars.

void CAutoPortico::UpdateTrackMapping(float left, float right)
{
    D3DMATERIAL7    mat;
    float           limit[2];
    int             rank;

    ZeroMemory( &mat, sizeof(D3DMATERIAL7) );
    mat.diffuse.r = 1.0f;
    mat.diffuse.g = 1.0f;
    mat.diffuse.b = 1.0f;  // blank
    mat.ambient.r = 0.5f;
    mat.ambient.g = 0.5f;
    mat.ambient.b = 0.5f;

    rank = m_object->RetObjectRank(0);

    limit[0] = 0.0f;
    limit[1] = 1000000.0f;

    m_engine->TrackTextureMapping(rank, mat, D3DSTATEPART1, "lemt.tga", "",
                                  limit[0], limit[1], D3DMAPPINGX,
                                  right, 8.0f, 8.0f, 192.0f, 256.0f);

    m_engine->TrackTextureMapping(rank, mat, D3DSTATEPART2, "lemt.tga", "",
                                  limit[0], limit[1], D3DMAPPINGX,
                                  left, 8.0f, 8.0f, 192.0f, 256.0f);
}

