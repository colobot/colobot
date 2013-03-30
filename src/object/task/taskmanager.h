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

// taskmanager.h

#pragma once


#include "object/object.h"
#include "object/task/task.h"
#include "object/task/taskmanip.h"
#include "object/task/taskgoto.h"
#include "object/task/taskshield.h"
#include "object/task/taskflag.h"



class CTaskManager
{
public:
    CTaskManager(CObject* object);
    ~CTaskManager();

    Error   StartTaskWait(float time);
    Error   StartTaskAdvance(float length);
    Error   StartTaskTurn(float angle);
    Error   StartTaskGoto(Math::Vector pos, float altitude, TaskGotoGoal goalMode, TaskGotoCrash crashMode);
    Error   StartTaskTake();
    Error   StartTaskManip(TaskManipOrder order, TaskManipArm arm);
    Error   StartTaskFlag(TaskFlagOrder order, int rank);
    Error   StartTaskBuild(ObjectType type);
    Error   StartTaskSearch();
    Error   StartTaskInfo(const char *name, float value, float power, bool bSend);
    Error   StartTaskTerraform();
    Error   StartTaskPen(bool bDown, int color);
    Error   StartTaskRecover();
    Error   StartTaskShield(TaskShieldMode mode, float delay);
    Error   StartTaskFire(float delay);
    Error   StartTaskFireAnt(Math::Vector impact);
    Error   StartTaskGunGoal(float dirV, float dirH);
    Error   StartTaskSpiderExplo();
    Error   StartTaskReset(Math::Vector goal, Math::Vector angle);

    bool    EventProcess(const Event &event);
    Error   IsEnded();
    bool    IsBusy();
    bool    IsPilot();
    bool    Abort();

protected:
    CTask*          m_task;
    CObject*        m_object;
    bool            m_bPilot;
};

