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

// taskreset.h

#pragma once


#include "object/task/task.h"
#include "math/vector.h"



enum TaskResetPhase
{
    TRSP_ZOUT   = 1,    // disappears
    TRSP_MOVE   = 2,    // moves
    TRSP_ZIN    = 3,    // reappears
};



class CTaskReset : public CTask
{
public:
    CTaskReset(CObject* object);
    ~CTaskReset();

    bool    EventProcess(const Event &event);

    Error   Start(Math::Vector goal, Math::Vector angle);
    Error   IsEnded();

protected:
    bool    SearchVehicle();

protected:
    Math::Vector        m_begin;
    Math::Vector        m_goal;
    Math::Vector        m_angle;

    TaskResetPhase  m_phase;
    bool            m_bError;
    float           m_time;
    float           m_speed;
    float           m_progress;
    float           m_lastParticle;  // time of generation last particle
    float           m_iAngle;
};

