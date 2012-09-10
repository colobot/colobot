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


// #include <stdio.h>

#include "object/task/task.h"

#include "common/iman.h"
#include "object/object.h"



// Object's constructor.

CTask::CTask(CInstanceManager* iMan, CObject* object)
{
    m_iMan = iMan;

    m_engine      = static_cast<Gfx::CEngine*>(m_iMan->SearchInstance(CLASS_ENGINE));
    m_lightMan    = static_cast<Gfx::CLightManager*>(m_iMan->SearchInstance(CLASS_LIGHT));
    m_terrain     = static_cast<Gfx::CTerrain*>(m_iMan->SearchInstance(CLASS_TERRAIN));
    m_water       = static_cast<Gfx::CWater*>(m_iMan->SearchInstance(CLASS_WATER));
    m_particle    = static_cast<Gfx::CParticle*>(m_iMan->SearchInstance(CLASS_PARTICULE));
    m_camera      = static_cast<Gfx::CCamera*>(m_iMan->SearchInstance(CLASS_CAMERA));
    m_displayText = static_cast<Ui::CDisplayText*>(m_iMan->SearchInstance(CLASS_DISPLAYTEXT));
    m_main        = static_cast<CRobotMain*>(m_iMan->SearchInstance(CLASS_MAIN));
    m_sound       = static_cast<CSoundInterface*>(m_iMan->SearchInstance(CLASS_SOUND));

    m_object      = object;
    m_physics     = m_object->GetPhysics();
    m_brain       = m_object->GetBrain();
    m_motion      = m_object->GetMotion();
}

// Object's destructor.

CTask::~CTask()
{
}


// Management of an event.

bool CTask::EventProcess(const Event &event)
{
    return true;
}


// Indicates whether the action is finished.

Error CTask::IsEnded()
{
    return ERR_STOP;
}


// Indicates whether the action is pending.

bool CTask::IsBusy()
{
    return true;
}


// Suddenly ends the current action.

bool CTask::Abort()
{
    return true;
}


