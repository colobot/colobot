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

// taskfireant.h

#pragma once


#include "object/task/task.h"
#include "math/vector.h"



enum TaskFireAnt
{
    TFA_NULL        = 0,    // nothing to do
    TFA_TURN        = 1,    // turns
    TFA_PREPARE     = 2,    // prepares shooting position
    TFA_FIRE        = 3,    // shooting
    TFA_TERMINATE       = 4,    // ends shooting position
};



class CTaskFireAnt : public CTask
{
public:
    CTaskFireAnt(CObject* object);
    ~CTaskFireAnt();

    bool    EventProcess(const Event &event);

    Error   Start(Math::Vector impact);
    Error   IsEnded();
    bool    Abort();

protected:

protected:
    Math::Vector    m_impact;
    TaskFireAnt m_phase;
    float       m_progress;
    float       m_speed;
    float       m_angle;
    bool        m_bError;
    bool        m_bFire;
    float       m_time;
    float       m_lastParticle;
};

