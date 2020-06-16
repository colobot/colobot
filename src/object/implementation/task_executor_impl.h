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

#include "object/interface/task_executor_object.h"

#include <memory>

class CObject;

class CTaskExecutorObjectImpl : public CTaskExecutorObject
{
public:
    explicit CTaskExecutorObjectImpl(ObjectInterfaceTypes& types, CObject* object);
    virtual ~CTaskExecutorObjectImpl();

    bool EventProcess(const Event& event);

    bool IsForegroundTask() override;
    bool IsBackgroundTask() override;

    CForegroundTask* GetForegroundTask() override;
    CBackgroundTask* GetBackgroundTask() override;

    void StopForegroundTask() override;
    void StopBackgroundTask() override;

    Error StartTaskTake() override;
    Error StartTaskManip(TaskManipOrder order, TaskManipArm arm) override;
    Error StartTaskFlag(TaskFlagOrder order, int rank) override;
    Error StartTaskBuild(ObjectType type) override;
    Error StartTaskSearch() override;
    Error StartTaskDeleteMark() override;
    Error StartTaskTerraform() override;
    Error StartTaskRecover() override;
    Error StartTaskFire(float delay) override;
    Error StartTaskFireAnt(Math::Vector impact) override;
    Error StartTaskSpiderExplo() override;
    Error StartTaskPen(bool down, TraceColor color = TraceColor::Default) override;

    Error StartTaskWait(float time) override;
    Error StartTaskAdvance(float length) override;
    Error StartTaskTurn(float angle) override;
    Error StartTaskGoto(Math::Vector pos, float altitude, TaskGotoGoal goalMode, TaskGotoCrash crashMode) override;
    Error StartTaskInfo(const char *name, float value, float power, bool bSend) override;

    Error StartTaskShield(TaskShieldMode mode, float delay = 1000.0f) override;
    Error StartTaskGunGoal(float dirV, float dirH) override;

private:
    Error EndedTask();

    template<typename TaskType, typename... Args>
    Error StartForegroundTask(Args&&... args);

    template<typename TaskType, typename... Args>
    Error StartBackgroundTask(Args&&... args);

protected:
    std::unique_ptr<CForegroundTask> m_foregroundTask;
    std::unique_ptr<CBackgroundTask> m_backgroundTask;

private:
    CObject* m_object;
};
