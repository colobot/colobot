/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "object/object_type.h"

#include <glm/glm.hpp>



class CObject;

enum TaskManipOrder
{
    TMO_AUTO    = 0,    // deposits or takes automatically
    TMO_GRAB    = 1,    // takes an object
    TMO_DROP    = 2,    // deposits the object
};

enum TaskManipArm
{
    TMA_NEUTRAL = 1,    // empty arm at rest
    TMA_STOCK   = 2,    // right arm resting
    TMA_FFRONT  = 3,    // arm on the ground
    TMA_FBACK   = 4,    // arm behind the robot
    TMA_POWER   = 5,    // arm behind the robot
    TMA_OTHER   = 6,    // arm behind a friend robot
};

enum TaskManipHand
{
    TMH_OPEN    = 1,    // open clamp
    TMH_CLOSE   = 2,    // closed clamp
};



class CTaskManip : public CForegroundTask
{
public:
    CTaskManip(COldObject* object);
    ~CTaskManip();

    bool        EventProcess(const Event &event) override;

    Error       Start(TaskManipOrder order, TaskManipArm arm);
    Error       IsEnded() override;
    bool        Abort() override;
    static void InstantGrab(CObject* obj, CObject* cargo);

protected:
    void        InitAngle();
    CObject*    SearchTakeUnderObject(glm::vec3 &pos, float dLimit);
    CObject*    SearchTakeFrontObject(bool bAdvance, glm::vec3 &pos, float &distance, float &angle);
    CObject*    SearchTakeBackObject(bool bAdvance, glm::vec3 &pos, float &distance, float &angle);
    CObject*    SearchOtherObject(bool bAdvance, glm::vec3 &pos, float &distance, float &angle, float &height, int &slotNumOut);
    bool        TransporterTakeObject();
    bool        TransporterDeposeObject();
    bool        IsFreeDeposeObject(glm::vec3 pos);
    void        SoundManip(float time, float amplitude=1.0f, float frequency=1.0f);

protected:
    TaskManipOrder  m_order = TMO_AUTO;
    TaskManipArm    m_arm = TMA_NEUTRAL;
    TaskManipHand   m_hand = TMH_OPEN;
    int             m_step = 0;
    float           m_speed = 0.0f;
    float           m_progress = 0.0f;
    float           m_initialAngle[5] = {};
    float           m_finalAngle[5] = {};
    float           m_height = 0.0f;
    float           m_advanceLength = 0.0f;
    float           m_energy = 0.0f;
    bool            m_bError = false;
    bool            m_bTurn = false;
    bool            m_bSubm = false;
    bool            m_bBee = false;
    float           m_angle = 0.0f;
    float           m_move = 0.0f;
    glm::vec3       m_targetPos = { 0, 0, 0 };
    float           m_timeLimit = 0.0f;
    ObjectType      m_cargoType = OBJECT_NULL;
};
