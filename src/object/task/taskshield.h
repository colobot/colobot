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
    TSM_START   = 4,    // start with shield up
};

class CShielder;



class CTaskShield : public CBackgroundTask
{
public:
    CTaskShield(COldObject* object);
    ~CTaskShield();

    bool        EventProcess(const Event &event) override;

    Error       Start(TaskShieldMode mode, float delay);
    Error       IsEnded() override;
    bool        IsBusy() override;
    bool        Abort() override;

    float       GetActiveRadius();

protected:
    Error       Stop();
    bool        CreateLight(Math::Vector pos);
    void        IncreaseShield();
    float       GetRadius();

protected:
    CShielder*      m_shielder;
    TaskShieldPhase m_phase = TS_UP1;
    float           m_progress = 0.0f;
    float           m_speed = 0.0f;
    float           m_time = 0.0f;
    float           m_delay = 0.0f;
    float           m_lastParticle = 0.0f;
    float           m_lastRay = 0.0f;
    float           m_lastIncrease = 0.0f;
    float           m_energyUsed = 0.0f;
    bool            m_bError = false;
    Math::Vector    m_shieldPos;
    int             m_rankSphere = 0;
    int             m_soundChannel = 0;
    int             m_effectLight = 0;
};
