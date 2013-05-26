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


#include <stdio.h>

#include "object/task/taskmanager.h"

#include "common/iman.h"

#include "object/task/taskwait.h"
#include "object/task/taskadvance.h"
#include "object/task/taskturn.h"
#include "object/task/tasktake.h"
#include "object/task/taskbuild.h"
#include "object/task/tasksearch.h"
#include "object/task/taskterraform.h"
#include "object/task/taskpen.h"
#include "object/task/taskrecover.h"
#include "object/task/taskinfo.h"
#include "object/task/taskfire.h"
#include "object/task/taskfireant.h"
#include "object/task/taskgungoal.h"
#include "object/task/taskspiderexplo.h"
#include "object/task/taskreset.h"




// Object's constructor.

CTaskManager::CTaskManager(CObject* object)
{
    m_task = nullptr;
    m_object = object;
    m_bPilot = false;
}

// Object's destructor.

CTaskManager::~CTaskManager()
{
    delete m_task;
}



// Waits for a while.

Error CTaskManager::StartTaskWait(float time)
{
    m_task = new CTaskWait(m_object);
    return (static_cast<CTaskWait*>(m_task))->Start(time);
}

// Advance straight ahead a certain distance.

Error CTaskManager::StartTaskAdvance(float length)
{
    m_task = new CTaskAdvance(m_object);
    return (static_cast<CTaskAdvance*>(m_task))->Start(length);
}

// Turns through an certain angle.

Error CTaskManager::StartTaskTurn(float angle)
{
    m_task = new CTaskTurn(m_object);
    return (static_cast<CTaskTurn*>(m_task))->Start(angle);
}

// Reaches a given position.

Error CTaskManager::StartTaskGoto(Math::Vector pos, float altitude, TaskGotoGoal goalMode, TaskGotoCrash crashMode)
{
    m_task = new CTaskGoto(m_object);
    return (static_cast<CTaskGoto*>(m_task))->Start(pos, altitude, goalMode, crashMode);
}

// Move the manipulator arm.

Error CTaskManager::StartTaskTake()
{
    m_task = new CTaskTake(m_object);
    return (static_cast<CTaskTake*>(m_task))->Start();
}

// Move the manipulator arm.

Error CTaskManager::StartTaskManip(TaskManipOrder order, TaskManipArm arm)
{
    m_task = new CTaskManip(m_object);
    return (static_cast<CTaskManip*>(m_task))->Start(order, arm);
}

// Puts or removes a flag.

Error CTaskManager::StartTaskFlag(TaskFlagOrder order, int rank)
{
    m_task = new CTaskFlag(m_object);
    return (static_cast<CTaskFlag*>(m_task))->Start(order, rank);
}

// Builds a building.

Error CTaskManager::StartTaskBuild(ObjectType type)
{
    m_task = new CTaskBuild(m_object);
    return (static_cast<CTaskBuild*>(m_task))->Start(type);
}

// Probe the ground.

Error CTaskManager::StartTaskSearch()
{
    m_task = new CTaskSearch(m_object);
    return (static_cast<CTaskSearch*>(m_task))->Start();
}

// Reads an information terminal.

Error CTaskManager::StartTaskInfo(const char *name, float value, float power, bool bSend)
{
    m_task = new CTaskInfo(m_object);
    return (static_cast<CTaskInfo*>(m_task))->Start(name, value, power, bSend);
}

// Terraforms the ground.

Error CTaskManager::StartTaskTerraform()
{
    m_task = new CTaskTerraform(m_object);
    return (static_cast<CTaskTerraform*>(m_task))->Start();
}

// Changes the pencil.

Error CTaskManager::StartTaskPen(bool bDown, int color)
{
    m_task = new CTaskPen(m_object);
    return (static_cast<CTaskPen*>(m_task))->Start(bDown, color);
}

// Recovers a ruin.

Error CTaskManager::StartTaskRecover()
{
    m_task = new CTaskRecover(m_object);
    return (static_cast<CTaskRecover*>(m_task))->Start();
}

// Deploys the shield.

Error CTaskManager::StartTaskShield(TaskShieldMode mode, float delay)
{
    if ( mode == TSM_UP )
    {
        m_task = new CTaskShield(m_object);
        return (static_cast<CTaskShield*>(m_task))->Start(mode, delay);
    }
    if ( mode == TSM_DOWN && m_task != 0 )
    {
        return (static_cast<CTaskShield*>(m_task))->Start(mode, delay);
    }
    if ( mode == TSM_UPDATE && m_task != 0 )
    {
        return (static_cast<CTaskShield*>(m_task))->Start(mode, delay);
    }
    return ERR_GENERIC;
}

// Shoots.

Error CTaskManager::StartTaskFire(float delay)
{
    m_bPilot = true;
    m_task = new CTaskFire(m_object);
    return (static_cast<CTaskFire*>(m_task))->Start(delay);
}

// Shoots with the ant.

Error CTaskManager::StartTaskFireAnt(Math::Vector impact)
{
    m_task = new CTaskFireAnt(m_object);
    return (static_cast<CTaskFireAnt*>(m_task))->Start(impact);
}

// Adjusts higher.

Error CTaskManager::StartTaskGunGoal(float dirV, float dirH)
{
    m_task = new CTaskGunGoal(m_object);
    return (static_cast<CTaskGunGoal*>(m_task))->Start(dirV, dirH);
}

// Suicide of the spider.

Error CTaskManager::StartTaskSpiderExplo()
{
    m_task = new CTaskSpiderExplo(m_object);
    return (static_cast<CTaskSpiderExplo*>(m_task))->Start();
}

// Reset.

Error CTaskManager::StartTaskReset(Math::Vector goal, Math::Vector angle)
{
    m_task = new CTaskReset(m_object);
    return (static_cast<CTaskReset*>(m_task))->Start(goal, angle);
}





// Management of an event.

bool CTaskManager::EventProcess(const Event &event)
{
    if ( m_task == 0 )  return false;
    return m_task->EventProcess(event);
}


// Indicates whether the action is finished.

Error CTaskManager::IsEnded()
{
    if ( m_task == 0 )  return ERR_GENERIC;
    return m_task->IsEnded();
}


// Indicates whether the action is pending.

bool CTaskManager::IsBusy()
{
    if ( m_task == 0 )  return false;
    return m_task->IsBusy();
}


// Indicates whether it is possible to control the robot
// during the execution of the current task.

bool CTaskManager::IsPilot()
{
    return m_bPilot;
}


// Suddenly ends the current action.

bool CTaskManager::Abort()
{
    if ( m_task == 0 )  return false;
    return m_task->Abort();
}

