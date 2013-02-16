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

// taskshield.h

#pragma once


#include "object/task/task.h"

#include "math/vector.h"


const float RADIUS_SHIELD_MIN = 40.0f;      // minimum radius of the protected zone
const float RADIUS_SHIELD_MAX = 100.0f;     // maximum radius of the protected zone


enum TaskShieldPhase
{
    TS_UP1      = 1,    // up
    TS_UP2      = 2,    // up
    TS_SHIELD   = 3,    // shield deployed
    TS_SMOKE    = 4,    // smoke
    TS_DOWN1    = 5,    // down
    TS_DOWN2    = 6,    // down
};

enum TaskShieldMode
{
    TSM_UP      = 1,    // deploys shield
    TSM_DOWN    = 2,    // returns the shield
    TSM_UPDATE  = 3,    // radius change
};



class CTaskShield : public CTask
{
public:
    CTaskShield(CObject* object);
    ~CTaskShield();

    bool        EventProcess(const Event &event);

    Error       Start(TaskShieldMode mode, float delay);
    Error       IsEnded();
    bool        IsBusy();
    bool        Abort();

protected:
    Error       Stop();
    bool        CreateLight(Math::Vector pos);
    void        IncreaseShield();
    float       GetRadius();

protected:
    TaskShieldPhase m_phase;
    float           m_progress;
    float           m_speed;
    float           m_time;
    float           m_delay;
    float           m_lastParticle;
    float           m_lastRay;
    float           m_lastIncrease;
    float           m_energyUsed;
    bool            m_bError;
    Math::Vector    m_shieldPos;
    int             m_rankSphere;
    int             m_soundChannel;
    int             m_effectLight;
};

