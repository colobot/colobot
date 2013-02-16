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

// taskmanip.h

#pragma once


#include "object/task/task.h"
#include "object/object.h"
#include "math/vector.h"



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
    TMA_GRAB    = 7,    // takes immediately
};

enum TaskManipHand
{
    TMH_OPEN    = 1,    // open clamp
    TMH_CLOSE   = 2,    // closed clamp
};



class CTaskManip : public CTask
{
public:
    CTaskManip(CObject* object);
    ~CTaskManip();

    bool        EventProcess(const Event &event);

    Error       Start(TaskManipOrder order, TaskManipArm arm);
    Error       IsEnded();
    bool        Abort();

protected:
    void        InitAngle();
    CObject*    SearchTakeUnderObject(Math::Vector &pos, float dLimit);
    CObject*    SearchTakeFrontObject(bool bAdvance, Math::Vector &pos, float &distance, float &angle);
    CObject*    SearchTakeBackObject(bool bAdvance, Math::Vector &pos, float &distance, float &angle);
    CObject*    SearchOtherObject(bool bAdvance, Math::Vector &pos, float &distance, float &angle, float &height);
    bool        TruckTakeObject();
    bool        TruckDeposeObject();
    bool        IsFreeDeposeObject(Math::Vector pos);
    void        SoundManip(float time, float amplitude=1.0f, float frequency=1.0f);

protected:
    TaskManipOrder  m_order;
    TaskManipArm    m_arm;
    TaskManipHand   m_hand;
    int             m_step;
    float           m_speed;
    float           m_progress;
    float           m_initialAngle[5];
    float           m_finalAngle[5];
    float           m_height;
    float           m_advanceLength;
    float           m_energy;
    bool            m_bError;
    bool            m_bTurn;
    bool            m_bSubm;
    bool            m_bBee;
    float           m_angle;
    float           m_move;
    Math::Vector        m_targetPos;
    float           m_timeLimit;
    ObjectType      m_fretType;
};

