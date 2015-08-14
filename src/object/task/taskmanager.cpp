/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2014, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsiteс.ch; http://colobot.info; http://github.com/colobot
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://gnu.org/licenses
 */


#include "object/task/taskmanager.h"

#include "common/make_unique.h"

#include "object/old_object.h"

#include "object/task/taskadvance.h"
#include "object/task/taskbuild.h"
#include "object/task/taskdeletemark.h"
#include "object/task/taskfire.h"
#include "object/task/taskfireant.h"
#include "object/task/taskgungoal.h"
#include "object/task/taskinfo.h"
#include "object/task/taskpen.h"
#include "object/task/taskrecover.h"
#include "object/task/tasksearch.h"
#include "object/task/taskspiderexplo.h"
#include "object/task/tasktake.h"
#include "object/task/taskterraform.h"
#include "object/task/taskturn.h"
#include "object/task/taskwait.h"


// Object's constructor.

CTaskManager::CTaskManager(COldObject* object)
    : m_object(object),
      m_bPilot(false)
{
}

// Object's destructor.

CTaskManager::~CTaskManager()
{
}

template<typename TaskType, typename... Args>
Error CTaskManager::StartTask(Args&&... args)
{
    auto task = MakeUnique<TaskType>(m_object);
    auto* taskPtr = task.get();
    m_task = std::move(task);
    return taskPtr->Start(std::forward<Args>(args)...);
}


// Waits for a while.

Error CTaskManager::StartTaskWait(float time)
{
    return StartTask<CTaskWait>(time);
}

// Advance straight ahead a certain distance.

Error CTaskManager::StartTaskAdvance(float length)
{
    return StartTask<CTaskAdvance>(length);
}

// Turns through an certain angle.

Error CTaskManager::StartTaskTurn(float angle)
{
    return StartTask<CTaskTurn>(angle);
}

// Reaches a given position.

Error CTaskManager::StartTaskGoto(Math::Vector pos, float altitude, TaskGotoGoal goalMode, TaskGotoCrash crashMode)
{
    return StartTask<CTaskGoto>(pos, altitude, goalMode, crashMode);
}

// Move the manipulator arm.

Error CTaskManager::StartTaskTake()
{
    return StartTask<CTaskTake>();
}

// Move the manipulator arm.

Error CTaskManager::StartTaskManip(TaskManipOrder order, TaskManipArm arm)
{
    return StartTask<CTaskManip>(order, arm);
}

// Puts or removes a flag.

Error CTaskManager::StartTaskFlag(TaskFlagOrder order, int rank)
{
    return StartTask<CTaskFlag>(order, rank);
}

// Builds a building.

Error CTaskManager::StartTaskBuild(ObjectType type)
{
    return StartTask<CTaskBuild>(type);
}

// Probe the ground.

Error CTaskManager::StartTaskSearch()
{
    return StartTask<CTaskSearch>();
}

// Delete mark on ground

Error CTaskManager::StartTaskDeleteMark()
{
    return StartTask<CTaskDeleteMark>();
}


// Reads an information terminal.

Error CTaskManager::StartTaskInfo(const char *name, float value, float power, bool bSend)
{
    return StartTask<CTaskInfo>(name, value, power, bSend);
}

// Terraforms the ground.

Error CTaskManager::StartTaskTerraform()
{
    return StartTask<CTaskTerraform>();
}

// Changes the pencil.

Error CTaskManager::StartTaskPen(bool bDown, TraceColor color)
{
    return StartTask<CTaskPen>(bDown, color);
}

// Recovers a ruin.

Error CTaskManager::StartTaskRecover()
{
    return StartTask<CTaskRecover>();
}

// Deploys the shield.

Error CTaskManager::StartTaskShield(TaskShieldMode mode, float delay)
{
    if (mode == TSM_UP || mode == TSM_START)
    {
        return StartTask<CTaskShield>(mode, delay);
    }
    else if (m_task != nullptr)
    {
        // TODO: is this static_cast really safe?
        return (static_cast<CTaskShield*>(m_task.get()))->Start(mode, delay);
    }
    return ERR_UNKNOWN;
}

// Shoots.

Error CTaskManager::StartTaskFire(float delay)
{
    m_bPilot = true; // TODO: this is set here, but never unset - is this right?
    return StartTask<CTaskFire>(delay);
}

// Shoots with the ant.

Error CTaskManager::StartTaskFireAnt(Math::Vector impact)
{
    return StartTask<CTaskFireAnt>(impact);
}

// Adjusts higher.

Error CTaskManager::StartTaskGunGoal(float dirV, float dirH)
{
    return StartTask<CTaskGunGoal>(dirV, dirH);
}

// Suicide of the spider.

Error CTaskManager::StartTaskSpiderExplo()
{
    return StartTask<CTaskSpiderExplo>();
}


// Management of an event.

bool CTaskManager::EventProcess(const Event &event)
{
    if (m_task == nullptr)
        return false;

    return m_task->EventProcess(event);
}


// Indicates whether the action is finished.

Error CTaskManager::IsEnded()
{
    if (m_task == nullptr)
        return ERR_UNKNOWN;

    return m_task->IsEnded();
}


// Indicates whether the action is pending.

bool CTaskManager::IsBusy()
{
    if (m_task == nullptr)
        return false;

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
    if (m_task == nullptr)
        return false;

    return m_task->Abort();
}
