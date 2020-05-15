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

#include "object/task/task.h"

#include "math/vector.h"

#include "object/object_type.h"


class CObject;

enum TaskTakeOrder
{
    TTO_TAKE    = 1,    // takes an object
    TTO_DEPOSE  = 2,    // deposes the object
};

enum TaskTakeArm
{
    TTA_NEUTRAL = 1,    // empty arm at rest
    TTA_FFRONT  = 2,    // arm on the ground
    TTA_FRIEND  = 3,    // arm behind a friend robot
};



class CTaskTake : public CForegroundTask
{
public:
    CTaskTake(COldObject* object);
    ~CTaskTake();

    bool        EventProcess(const Event &event) override;

    Error       Start();
    Error       IsEnded() override;
    bool        Abort() override;

protected:
    CObject*    SearchTakeObject(float &angle, float dLimit, float aLimit);
    CObject*    SearchFriendObject(float &angle, float dLimit, float aLimit);
    bool        TransporterTakeObject();
    bool        TransporterDeposeObject();
    bool        IsFreeDeposeObject(Math::Vector pos);

protected:
    TaskTakeOrder   m_order = TTO_TAKE;
    TaskTakeArm     m_arm = TTA_NEUTRAL;
    int             m_step = 0;
    float           m_speed = 0.0f;
    float           m_progress = 0.0f;
    float           m_height = 0.0f;
    bool            m_bError = false;
    bool            m_bTurn = false;
    float           m_angle = 0.0f;
    ObjectType      m_cargoType = OBJECT_NULL;
};
