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

#ifndef _TASKMANAGER_H_
#define _TASKMANAGER_H_

#include "misc.h"
#include "object.h"
#include "taskmanip.h"
#include "taskgoto.h"
#include "taskshield.h"
#include "taskflag.h"


class CInstanceManager;
class CTask;



class CTaskManager
{
public:
    CTaskManager(CInstanceManager* iMan, CObject* object);
    ~CTaskManager();

    Error   StartTaskWait(float time);
    Error   StartTaskAdvance(float length);
    Error   StartTaskTurn(float angle);
    Error   StartTaskGoto(D3DVECTOR pos, float altitude, TaskGotoGoal goalMode, TaskGotoCrash crashMode);
    Error   StartTaskTake();
    Error   StartTaskManip(TaskManipOrder order, TaskManipArm arm);
    Error   StartTaskFlag(TaskFlagOrder order, int rank);
    Error   StartTaskBuild(ObjectType type);
    Error   StartTaskSearch();
    Error   StartTaskInfo(char *name, float value, float power, BOOL bSend);
    Error   StartTaskTerraform();
    Error   StartTaskPen(BOOL bDown, int color);
    Error   StartTaskRecover();
    Error   StartTaskShield(TaskShieldMode mode, float delay);
    Error   StartTaskFire(float delay);
    Error   StartTaskFireAnt(D3DVECTOR impact);
    Error   StartTaskGunGoal(float dirV, float dirH);
    Error   StartTaskSpiderExplo();
    Error   StartTaskReset(D3DVECTOR goal, D3DVECTOR angle);

    BOOL    EventProcess(const Event &event);
    Error   IsEnded();
    BOOL    IsBusy();
    BOOL    IsPilot();
    BOOL    Abort();

protected:

protected:
    CInstanceManager* m_iMan;
    CTask*          m_task;
    CObject*        m_object;
    BOOL            m_bPilot;
};


#endif //_TASKMANAGER_H_
