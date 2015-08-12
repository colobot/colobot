/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2015, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "object/implementation/task_executor_impl.h"

#include "object/object.h"

#include "object/task/taskmanager.h"

CTaskExecutorObjectImpl::CTaskExecutorObjectImpl(ObjectInterfaceTypes& types, CObject* object)
    : CTaskExecutorObject(types)
    , m_object(object)
{}

CTaskExecutorObjectImpl::~CTaskExecutorObjectImpl()
{}

bool CTaskExecutorObjectImpl::EventProcess(const Event &event)
{
    // NOTE: This function CAN'T BE CALLED BETWEEN CTaskManager::EventProcess AND CScriptFunctions::Process, otherwise weird stuff may happen to scripts (they'll be stuck executing the same task over and over again)
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

CTaskManager* CTaskExecutorObjectImpl::GetForegroundTask()
{
    return m_foregroundTask.get();
}

CTaskManager* CTaskExecutorObjectImpl::GetBackgroundTask()
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
