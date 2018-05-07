/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsitec.ch; http://colobot.info; http://github.com/colobot
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

#include "object/implementation/task_executor_impl.h"

#include "object/object.h"
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

CTaskExecutorObjectImpl::CTaskExecutorObjectImpl(ObjectInterfaceTypes& types, CObject* object)
    : CTaskExecutorObject(types)
    , m_object(object)
{}

CTaskExecutorObjectImpl::~CTaskExecutorObjectImpl()
{}

bool CTaskExecutorObjectImpl::EventProcess(const Event &event)
{
    // NOTE: This function CAN'T BE CALLED BETWEEN CTask::EventProcess AND CScriptFunctions::Process, otherwise weird stuff may happen to scripts (they'll be stuck executing the same task over and over again)
    EndedTask();

    if ( m_foregroundTask != nullptr )
    {
        m_foregroundTask->EventProcess(event);
    }

    if ( m_backgroundTask != nullptr )
    {
        m_backgroundTask->EventProcess(event);
    }

    return true;
}

bool CTaskExecutorObjectImpl::IsForegroundTask()
{
    return m_foregroundTask != nullptr;
}

bool CTaskExecutorObjectImpl::IsBackgroundTask()
{
    return m_backgroundTask != nullptr;
}

CForegroundTask* CTaskExecutorObjectImpl::GetForegroundTask()
{
    return m_foregroundTask.get();
}

CBackgroundTask* CTaskExecutorObjectImpl::GetBackgroundTask()
{
    return m_backgroundTask.get();
}

// Stops the current task.

void CTaskExecutorObjectImpl::StopForegroundTask()
{
    if (m_foregroundTask != nullptr)
    {
        m_foregroundTask->Abort();
        m_foregroundTask.reset();
    }
}

// Stops the current secondary task.

void CTaskExecutorObjectImpl::StopBackgroundTask()
{
    if (m_backgroundTask != nullptr)
    {
        m_backgroundTask->Abort();
        m_backgroundTask.reset();
    }
}

// Completes the task when the time came.

Error CTaskExecutorObjectImpl::EndedTask()
{
    if (m_backgroundTask != nullptr)  // current task?
    {
        Error err = m_backgroundTask->IsEnded();
        if ( err != ERR_CONTINUE )  // job ended?
        {
            m_backgroundTask.reset();
            m_object->UpdateInterface();
        }
    }

    if (m_foregroundTask != nullptr)  // current task?
    {
        Error err = m_foregroundTask->IsEnded();
        if ( err != ERR_CONTINUE )  // job ended?
        {
            m_foregroundTask.reset();
            m_object->UpdateInterface();
        }
        return err;
    }

    return ERR_STOP;
}

template<typename TaskType, typename... Args>
Error CTaskExecutorObjectImpl::StartForegroundTask(Args&&... args)
{
    static_assert(std::is_base_of<CForegroundTask, TaskType>::value, "not a foreground task");

    StopForegroundTask();

    assert(m_object->Implements(ObjectInterfaceType::Old)); //TODO
    std::unique_ptr<TaskType> task = MakeUnique<TaskType>(dynamic_cast<COldObject*>(m_object));
    Error err = task->Start(std::forward<Args>(args)...);
    if (err == ERR_OK)
        m_foregroundTask = std::move(task);
    m_object->UpdateInterface();
    return err;
}

template<typename TaskType, typename... Args>
Error CTaskExecutorObjectImpl::StartBackgroundTask(Args&&... args)
{
    static_assert(std::is_base_of<CBackgroundTask, TaskType>::value, "not a background task");

    Error err;
    TaskType* task = dynamic_cast<TaskType*>(m_backgroundTask.get());
    if (task != nullptr)
    {
        err = task->Start(std::forward<Args>(args)...);
        if (err != ERR_OK)
            m_backgroundTask.reset();
    }
    else
    {
        m_backgroundTask.reset(); // In case the old task was of a different type

        assert(m_object->Implements(ObjectInterfaceType::Old)); //TODO
        std::unique_ptr<TaskType> newTask = MakeUnique<TaskType>(dynamic_cast<COldObject*>(m_object));
        err = newTask->Start(std::forward<Args>(args)...);
        if (err == ERR_OK)
            m_backgroundTask = std::move(newTask);
    }
    m_object->UpdateInterface();
    return err;
}

Error CTaskExecutorObjectImpl::StartTaskTake()
{
    return StartForegroundTask<CTaskTake>();
}

Error CTaskExecutorObjectImpl::StartTaskManip(TaskManipOrder order, TaskManipArm arm)
{
    return StartForegroundTask<CTaskManip>(order, arm);
}

Error CTaskExecutorObjectImpl::StartTaskFlag(TaskFlagOrder order, int rank)
{
    return StartForegroundTask<CTaskFlag>(order, rank);
}

Error CTaskExecutorObjectImpl::StartTaskBuild(ObjectType type)
{
    return StartForegroundTask<CTaskBuild>(type);
}

Error CTaskExecutorObjectImpl::StartTaskSearch()
{
    return StartForegroundTask<CTaskSearch>();
}

Error CTaskExecutorObjectImpl::StartTaskDeleteMark()
{
    return StartForegroundTask<CTaskDeleteMark>();
}

Error CTaskExecutorObjectImpl::StartTaskTerraform()
{
    return StartForegroundTask<CTaskTerraform>();
}

Error CTaskExecutorObjectImpl::StartTaskRecover()
{
    return StartForegroundTask<CTaskRecover>();
}

Error CTaskExecutorObjectImpl::StartTaskFire(float delay)
{
    return StartForegroundTask<CTaskFire>(delay);
}

Error CTaskExecutorObjectImpl::StartTaskFireAnt(Math::Vector impact)
{
    return StartForegroundTask<CTaskFireAnt>(impact);
}

Error CTaskExecutorObjectImpl::StartTaskSpiderExplo()
{
    return StartForegroundTask<CTaskSpiderExplo>();
}

Error CTaskExecutorObjectImpl::StartTaskPen(bool down, TraceColor color)
{
    return StartForegroundTask<CTaskPen>(down, color);
}


Error CTaskExecutorObjectImpl::StartTaskWait(float time)
{
    return StartForegroundTask<CTaskWait>(time);
}

Error CTaskExecutorObjectImpl::StartTaskAdvance(float length)
{
    return StartForegroundTask<CTaskAdvance>(length);
}

Error CTaskExecutorObjectImpl::StartTaskTurn(float angle)
{
    return StartForegroundTask<CTaskTurn>(angle);
}

Error CTaskExecutorObjectImpl::StartTaskGoto(Math::Vector pos, float altitude, TaskGotoGoal goalMode, TaskGotoCrash crashMode)
{
    return StartForegroundTask<CTaskGoto>(pos, altitude, goalMode, crashMode);
}

Error CTaskExecutorObjectImpl::StartTaskInfo(const char *name, float value, float power, bool bSend)
{
    return StartForegroundTask<CTaskInfo>(name, value, power, bSend);
}


Error CTaskExecutorObjectImpl::StartTaskShield(TaskShieldMode mode, float delay)
{
    return StartBackgroundTask<CTaskShield>(mode, delay);
}

Error CTaskExecutorObjectImpl::StartTaskGunGoal(float dirV, float dirH)
{
    return StartBackgroundTask<CTaskGunGoal>(dirV, dirH);
}
