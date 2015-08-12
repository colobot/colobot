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

#pragma once

#include "object/interface/task_executor_object.h"

#include <memory>

class CObject;
class CTaskManager;

class CTaskExecutorObjectImpl : public CTaskExecutorObject
{
public:
    explicit CTaskExecutorObjectImpl(ObjectInterfaceTypes& types, CObject* object);
    virtual ~CTaskExecutorObjectImpl();

    bool EventProcess(const Event& event);

    bool IsForegroundTask() override;
    bool IsBackgroundTask() override;

    CTaskManager* GetForegroundTask() override;
    CTaskManager* GetBackgroundTask() override;

    void StopForegroundTask() override;
    void StopBackgroundTask() override;

    Error StartTaskTake() { return ERR_WRONG_BOT; }
    Error StartTaskManip(TaskManipOrder order, TaskManipArm arm) { return ERR_WRONG_BOT; }
    Error StartTaskFlag(TaskFlagOrder order, int rank) { return ERR_WRONG_BOT; }
    Error StartTaskBuild(ObjectType type) { return ERR_WRONG_BOT; }
    Error StartTaskSearch() { return ERR_WRONG_BOT; }
    Error StartTaskDeleteMark() { return ERR_WRONG_BOT; }
    Error StartTaskTerraform() { return ERR_WRONG_BOT; }
    Error StartTaskRecover() { return ERR_WRONG_BOT; }
    Error StartTaskFire(float delay) { return ERR_WRONG_BOT; }
    Error StartTaskFireAnt(Math::Vector impact) { return ERR_WRONG_BOT; }
    Error StartTaskSpiderExplo() { return ERR_WRONG_BOT; }
    Error StartTaskPen(bool down, TraceColor color = TraceColor::Default) { return ERR_WRONG_BOT; }

    Error StartTaskWait(float time) { return ERR_UNKNOWN; }
    Error StartTaskAdvance(float length) { return ERR_UNKNOWN; }
    Error StartTaskTurn(float angle) { return ERR_UNKNOWN; }
    Error StartTaskGoto(Math::Vector pos, float altitude, TaskGotoGoal goalMode, TaskGotoCrash crashMode) { return ERR_UNKNOWN; }
    Error StartTaskInfo(const char *name, float value, float power, bool bSend) { return ERR_UNKNOWN; }

    Error StartTaskShield(TaskShieldMode mode, float delay = 1000.0f) { return ERR_WRONG_BOT; }
    Error StartTaskGunGoal(float dirV, float dirH) { return ERR_WRONG_BOT; }

private:
    Error EndedTask();

protected:
    std::unique_ptr<CTaskManager> m_foregroundTask;
    std::unique_ptr<CTaskManager> m_backgroundTask;

private:
    CObject* m_object;
};
