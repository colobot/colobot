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

#pragma once

#include "object/object.h"
#include "object/object_interface_type.h"

#include "object/interface/trace_drawing_object.h"

#include "object/task/taskflag.h"
#include "object/task/taskgoto.h"
#include "object/task/taskmanip.h"
#include "object/task/taskshield.h"

/**
 * \class CTaskExecutorObject
 * \brief Interface for objects that can execute tasks
 */
class CTaskExecutorObject
{
public:
    explicit CTaskExecutorObject(ObjectInterfaceTypes& types)
    {
        types[static_cast<int>(ObjectInterfaceType::TaskExecutor)] = true;
    }
    virtual ~CTaskExecutorObject()
    {}

    //! Start a foreground task
    //@{
    virtual Error StartTaskTake() = 0;
    virtual Error StartTaskManip(TaskManipOrder order, TaskManipArm arm) = 0;
    virtual Error StartTaskFlag(TaskFlagOrder order, int rank) = 0;
    virtual Error StartTaskBuild(ObjectType type) = 0;
    virtual Error StartTaskSearch() = 0;
    virtual Error StartTaskDeleteMark() = 0;
    virtual Error StartTaskTerraform() = 0;
    virtual Error StartTaskRecover() = 0;
    virtual Error StartTaskFire(float delay) = 0;
    virtual Error StartTaskFireAnt(Math::Vector impact) = 0;
    virtual Error StartTaskSpiderExplo() = 0;
    virtual Error StartTaskPen(bool down, TraceColor color = TraceColor::Default) = 0;
    //@}
    //! Start a foreground task (scriptable tasks, not in UI)
    //@{
    virtual Error StartTaskWait(float time) = 0;
    virtual Error StartTaskAdvance(float length) = 0;
    virtual Error StartTaskTurn(float angle) = 0;
    virtual Error StartTaskGoto(Math::Vector pos, float altitude, TaskGotoGoal goalMode, TaskGotoCrash crashMode) = 0;
    virtual Error StartTaskInfo(const char *name, float value, float power, bool bSend) = 0;
    //@}
    //! Starts a background task
    //@{
    virtual Error StartTaskShield(TaskShieldMode mode, float delay = 1000.0f) = 0;
    virtual Error StartTaskGunGoal(float dirV, float dirH) = 0;
    //@}

    //! Is executing foreground task?
    virtual bool IsForegroundTask() = 0;
    //! Is executing background task?
    virtual bool IsBackgroundTask() = 0;

    //! Return the foreground task
    virtual CForegroundTask* GetForegroundTask() = 0;
    //! Return the background task
    virtual CBackgroundTask* GetBackgroundTask() = 0;

    //! Stop foreground task
    virtual void StopForegroundTask() = 0;
    //! Stop background task
    virtual void StopBackgroundTask() = 0;
};
