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

// taskgungoal.h

#pragma once


#include "object/task/task.h"



class CTaskGunGoal : public CTask
{
public:
    CTaskGunGoal(CObject* object);
    ~CTaskGunGoal();

    bool        EventProcess(const Event &event);

    Error       Start(float dirV, float dirH);
    Error       IsEnded();
    bool        Abort();

protected:

protected:
    float       m_progress;
    float       m_speed;
    float       m_initialDirV;  // initial direction
    float       m_finalDirV;    // direction to reach
    float       m_initialDirH;  // initial direction
    float       m_finalDirH;    // direction to reach

    bool        m_aimImpossible; // set to true if impossible aim was set
};

