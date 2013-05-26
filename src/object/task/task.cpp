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


#include "object/task/task.h"

#include "app/app.h"

#include "object/object.h"
#include "object/robotmain.h"


// Object's constructor.

CTask::CTask(CObject* object)
{
    m_sound       = CApplication::GetInstancePointer()->GetSound();
    m_engine      = Gfx::CEngine::GetInstancePointer();
    m_lightMan    = m_engine->GetLightManager();
    m_water       = m_engine->GetWater();
    m_particle    = m_engine->GetParticle();
    m_main        = CRobotMain::GetInstancePointer();
    m_terrain     = m_main->GetTerrain();
    m_camera      = m_main->GetCamera();
    m_displayText = m_main->GetDisplayText();

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

