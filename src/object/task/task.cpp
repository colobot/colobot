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

// task.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "common/struct.h"
#include "graphics/d3d/d3dengine.h"
#include "math/old/math3d.h"
#include "common/event.h"
#include "common/misc.h"
#include "common/iman.h"
#include "graphics/common/light.h"
#include "graphics/common/particule.h"
#include "graphics/common/terrain.h"
#include "graphics/common/water.h"
#include "object/object.h"
#include "physics/physics.h"
#include "object/brain.h"
#include "object/motion/motion.h"
#include "graphics/common/camera.h"
#include "sound/sound.h"
#include "object/robotmain.h"
#include "ui/displaytext.h"
#include "object/task/task.h"




// Object's constructor.

CTask::CTask(CInstanceManager* iMan, CObject* object)
{
	m_iMan = iMan;

	m_engine      = (CD3DEngine*)m_iMan->SearchInstance(CLASS_ENGINE);
	m_light       = (CLight*)m_iMan->SearchInstance(CLASS_LIGHT);
	m_particule   = (CParticule*)m_iMan->SearchInstance(CLASS_PARTICULE);
	m_terrain     = (CTerrain*)m_iMan->SearchInstance(CLASS_TERRAIN);
	m_water       = (CWater*)m_iMan->SearchInstance(CLASS_WATER);
	m_camera      = (CCamera*)m_iMan->SearchInstance(CLASS_CAMERA);
	m_main        = (CRobotMain*)m_iMan->SearchInstance(CLASS_MAIN);
	m_displayText = (CDisplayText*)m_iMan->SearchInstance(CLASS_DISPLAYTEXT);
	m_sound       = (CSound*)m_iMan->SearchInstance(CLASS_SOUND);

	m_object      = object;
	m_physics     = m_object->RetPhysics();
	m_brain       = m_object->RetBrain();
	m_motion      = m_object->RetMotion();
}

// Object's destructor.

CTask::~CTask()
{
}


// Management of an event.

BOOL CTask::EventProcess(const Event &event)
{
	return TRUE;
}


// Indicates whether the action is finished.

Error CTask::IsEnded()
{
	return ERR_STOP;
}


// Indicates whether the action is pending.

BOOL CTask::IsBusy()
{
	return TRUE;
}


// Suddenly ends the current action.

BOOL CTask::Abort()
{
	return TRUE;
}


