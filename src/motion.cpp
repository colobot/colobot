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

// motion.cpp

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
#include "light.h"
#include "particule.h"
#include "terrain.h"
#include "water.h"
#include "object.h"
#include "physics.h"
#include "brain.h"
#include "camera.h"
#include "robotmain.h"
#include "sound.h"
#include "cmdtoken.h"
#include "motion.h"




// Object's constructor.

CMotion::CMotion(CInstanceManager* iMan, CObject* object)
{
    m_iMan = iMan;
    m_iMan->AddInstance(CLASS_MOTION, this, 100);

    m_engine    = (CD3DEngine*)m_iMan->SearchInstance(CLASS_ENGINE);
    m_light     = (CLight*)m_iMan->SearchInstance(CLASS_LIGHT);
    m_particule = (CParticule*)m_iMan->SearchInstance(CLASS_PARTICULE);
    m_terrain   = (CTerrain*)m_iMan->SearchInstance(CLASS_TERRAIN);
    m_water     = (CWater*)m_iMan->SearchInstance(CLASS_WATER);
    m_camera    = (CCamera*)m_iMan->SearchInstance(CLASS_CAMERA);
    m_main      = (CRobotMain*)m_iMan->SearchInstance(CLASS_MAIN);
    m_sound     = (CSound*)m_iMan->SearchInstance(CLASS_SOUND);

    m_object    = object;
    m_physics   = 0;
    m_brain     = 0;

    m_actionType = -1;
    m_actionTime = 0.0f;
    m_progress   = 0.0f;

    m_linVibration  = D3DVECTOR(0.0f, 0.0f, 0.0f);
    m_cirVibration  = D3DVECTOR(0.0f, 0.0f, 0.0f);
    m_inclinaison   = D3DVECTOR(0.0f, 0.0f, 0.0f);
}

// Object's destructor.

CMotion::~CMotion()
{
    m_iMan->DeleteInstance(CLASS_MOTION, this);
}

// Deletes the object.

void CMotion::DeleteObject(BOOL bAll)
{
}


void CMotion::SetPhysics(CPhysics* physics)
{
    m_physics = physics;
}

void CMotion::SetBrain(CBrain* brain)
{
    m_brain = brain;
}


// Creates.

BOOL CMotion::Create(D3DVECTOR pos, float angle, ObjectType type, float power)
{
    return TRUE;
}

// Management of an event.

BOOL CMotion::EventProcess(const Event &event)
{
    D3DVECTOR   pos, dir;
    float       time;

    if ( m_object->RetType() != OBJECT_TOTO &&
         m_engine->RetPause() )  return TRUE;

    if ( event.event != EVENT_FRAME )  return TRUE;

    m_progress += event.rTime*m_actionTime;
    if ( m_progress > 1.0f )  m_progress = 1.0f;  // (*)

    pos = m_object->RetPosition(0);
    if ( pos.y < m_water->RetLevel(m_object) )  // underwater?
    {
        time = event.rTime*3.0f;  // everything is slower
    }
    else
    {
        time = event.rTime*10.0f;
    }

    dir = m_object->RetLinVibration();
    dir.x = Smooth(dir.x, m_linVibration.x, time);
    dir.y = Smooth(dir.y, m_linVibration.y, time);
    dir.z = Smooth(dir.z, m_linVibration.z, time);
    m_object->SetLinVibration(dir);

    dir = m_object->RetCirVibration();
    dir.x = Smooth(dir.x, m_cirVibration.x, time);
    dir.y = Smooth(dir.y, m_cirVibration.y, time);
    dir.z = Smooth(dir.z, m_cirVibration.z, time);
    m_object->SetCirVibration(dir);

    dir = m_object->RetInclinaison();
    dir.x = Smooth(dir.x, m_inclinaison.x, time);
    dir.y = Smooth(dir.y, m_inclinaison.y, time);
    dir.z = Smooth(dir.z, m_inclinaison.z, time);
    m_object->SetInclinaison(dir);

    return TRUE;
}

// (*)  Avoids the bug of ants returned by the thumper and
//      whose abdomen grown to infinity!


// Start an action.

Error CMotion::SetAction(int action, float time)
{
    m_actionType = action;
    m_actionTime = 1.0f/time;
    m_progress = 0.0f;
    return ERR_OK;
}

// Returns the current action.

int CMotion::RetAction()
{
    return m_actionType;
}


// Specifies a special parameter.

BOOL CMotion::SetParam(int rank, float value)
{
    return FALSE;
}

float CMotion::RetParam(int rank)
{
    return 0.0f;
}


// Saves all parameters of the object.

BOOL CMotion::Write(char *line)
{
    char    name[100];

    if ( m_actionType == -1 )  return FALSE;

    sprintf(name, " mType=%d", m_actionType);
    strcat(line, name);

    sprintf(name, " mTime=%.2f", m_actionTime);
    strcat(line, name);

    sprintf(name, " mProgress=%.2f", m_progress);
    strcat(line, name);

    return FALSE;
}

// Restores all parameters of the object.

BOOL CMotion::Read(char *line)
{
    m_actionType = OpInt(line, "mType", -1);
    m_actionTime = OpFloat(line, "mTime", 0.0f);
    m_progress = OpFloat(line, "mProgress", 0.0f);

    return FALSE;
}


// Gives the linear vibration.

void CMotion::SetLinVibration(D3DVECTOR dir)
{
    m_linVibration = dir;
}

D3DVECTOR CMotion::RetLinVibration()
{
    return m_linVibration;
}

// Gives the circular vibration.

void CMotion::SetCirVibration(D3DVECTOR dir)
{
    m_cirVibration = dir;
}

D3DVECTOR CMotion::RetCirVibration()
{
    return m_cirVibration;
}

// Gives the tilt.

void CMotion::SetInclinaison(D3DVECTOR dir)
{
    m_inclinaison = dir;
}

D3DVECTOR CMotion::RetInclinaison()
{
    return m_inclinaison;
}

