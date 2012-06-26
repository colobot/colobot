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

// taskmanager.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "d3dengine.h"
#include "misc.h"
#include "iman.h"
#include "event.h"
#include "object.h"
#include "task.h"
#include "taskwait.h"
#include "taskadvance.h"
#include "taskturn.h"
#include "taskgoto.h"
#include "tasktake.h"
#include "taskmanip.h"
#include "taskflag.h"
#include "taskbuild.h"
#include "tasksearch.h"
#include "taskterraform.h"
#include "taskpen.h"
#include "taskrecover.h"
#include "taskshield.h"
#include "taskinfo.h"
#include "taskfire.h"
#include "taskfireant.h"
#include "taskgungoal.h"
#include "taskspiderexplo.h"
#include "taskreset.h"
#include "taskmanager.h"




// Object's constructor.

CTaskManager::CTaskManager(CInstanceManager* iMan, CObject* object)
{
    m_iMan = iMan;
    m_iMan->AddInstance(CLASS_TASKMANAGER, this, 100);

    m_task = 0;
    m_object = object;
    m_bPilot = FALSE;
}

// Object's destructor.

CTaskManager::~CTaskManager()
{
    delete m_task;
}



// Waits for a while.

Error CTaskManager::StartTaskWait(float time)
{
    m_task = new CTaskWait(m_iMan, m_object);
    return ((CTaskWait*)m_task)->Start(time);
}

// Advance straight ahead a certain distance.

Error CTaskManager::StartTaskAdvance(float length)
{
    m_task = new CTaskAdvance(m_iMan, m_object);
    return ((CTaskAdvance*)m_task)->Start(length);
}

// Turns through an certain angle.

Error CTaskManager::StartTaskTurn(float angle)
{
    m_task = new CTaskTurn(m_iMan, m_object);
    return ((CTaskTurn*)m_task)->Start(angle);
}

// Reaches a given position.

Error CTaskManager::StartTaskGoto(D3DVECTOR pos, float altitude, TaskGotoGoal goalMode, TaskGotoCrash crashMode)
{
    m_task = new CTaskGoto(m_iMan, m_object);
    return ((CTaskGoto*)m_task)->Start(pos, altitude, goalMode, crashMode);
}

// Move the manipulator arm.

Error CTaskManager::StartTaskTake()
{
    m_task = new CTaskTake(m_iMan, m_object);
    return ((CTaskTake*)m_task)->Start();
}

// Move the manipulator arm.

Error CTaskManager::StartTaskManip(TaskManipOrder order, TaskManipArm arm)
{
    m_task = new CTaskManip(m_iMan, m_object);
    return ((CTaskManip*)m_task)->Start(order, arm);
}

// Puts or removes a flag.

Error CTaskManager::StartTaskFlag(TaskFlagOrder order, int rank)
{
    m_task = new CTaskFlag(m_iMan, m_object);
    return ((CTaskFlag*)m_task)->Start(order, rank);
}

// Builds a building.

Error CTaskManager::StartTaskBuild(ObjectType type)
{
    m_task = new CTaskBuild(m_iMan, m_object);
    return ((CTaskBuild*)m_task)->Start(type);
}

// Probe the ground.

Error CTaskManager::StartTaskSearch()
{
    m_task = new CTaskSearch(m_iMan, m_object);
    return ((CTaskSearch*)m_task)->Start();
}

// Reads an information terminal.

Error CTaskManager::StartTaskInfo(char *name, float value, float power, BOOL bSend)
{
    m_task = new CTaskInfo(m_iMan, m_object);
    return ((CTaskInfo*)m_task)->Start(name, value, power, bSend);
}

// Terraforms the ground.

Error CTaskManager::StartTaskTerraform()
{
    m_task = new CTaskTerraform(m_iMan, m_object);
    return ((CTaskTerraform*)m_task)->Start();
}

// Changes the pencil.

Error CTaskManager::StartTaskPen(BOOL bDown, int color)
{
    m_task = new CTaskPen(m_iMan, m_object);
    return ((CTaskPen*)m_task)->Start(bDown, color);
}

// Recovers a ruin.

Error CTaskManager::StartTaskRecover()
{
    m_task = new CTaskRecover(m_iMan, m_object);
    return ((CTaskRecover*)m_task)->Start();
}

// Deploys the shield.

Error CTaskManager::StartTaskShield(TaskShieldMode mode, float delay)
{
    if ( mode == TSM_UP )
    {
        m_task = new CTaskShield(m_iMan, m_object);
        return ((CTaskShield*)m_task)->Start(mode, delay);
    }
    if ( mode == TSM_DOWN && m_task != 0 )
    {
        return ((CTaskShield*)m_task)->Start(mode, delay);
    }
    if ( mode == TSM_UPDATE && m_task != 0 )
    {
        return ((CTaskShield*)m_task)->Start(mode, delay);
    }
    return ERR_GENERIC;
}

// Shoots.

Error CTaskManager::StartTaskFire(float delay)
{
    m_bPilot = TRUE;
    m_task = new CTaskFire(m_iMan, m_object);
    return ((CTaskFire*)m_task)->Start(delay);
}

// Shoots with the ant.

Error CTaskManager::StartTaskFireAnt(D3DVECTOR impact)
{
    m_task = new CTaskFireAnt(m_iMan, m_object);
    return ((CTaskFireAnt*)m_task)->Start(impact);
}

// Adjusts higher.

Error CTaskManager::StartTaskGunGoal(float dirV, float dirH)
{
    m_task = new CTaskGunGoal(m_iMan, m_object);
    return ((CTaskGunGoal*)m_task)->Start(dirV, dirH);
}

// Suicide of the spider.

Error CTaskManager::StartTaskSpiderExplo()
{
    m_task = new CTaskSpiderExplo(m_iMan, m_object);
    return ((CTaskSpiderExplo*)m_task)->Start();
}

// Reset.

Error CTaskManager::StartTaskReset(D3DVECTOR goal, D3DVECTOR angle)
{
    m_task = new CTaskReset(m_iMan, m_object);
    return ((CTaskReset*)m_task)->Start(goal, angle);
}





// Management of an event.

BOOL CTaskManager::EventProcess(const Event &event)
{
    if ( m_task == 0 )  return FALSE;
    return m_task->EventProcess(event);
}


// Indicates whether the action is finished.

Error CTaskManager::IsEnded()
{
    if ( m_task == 0 )  return ERR_GENERIC;
    return m_task->IsEnded();
}


// Indicates whether the action is pending.

BOOL CTaskManager::IsBusy()
{
    if ( m_task == 0 )  return FALSE;
    return m_task->IsBusy();
}


// Indicates whether it is possible to control the robot
// during the execution of the current task.

BOOL CTaskManager::IsPilot()
{
    return m_bPilot;
}


// Suddenly ends the current action.

BOOL CTaskManager::Abort()
{
    if ( m_task == 0 )  return FALSE;
    return m_task->Abort();
}


