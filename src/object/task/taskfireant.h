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


enum TaskFireAnt
{
    TFA_NULL        = 0,    // nothing to do
    TFA_TURN        = 1,    // turns
    TFA_PREPARE     = 2,    // prepares shooting position
    TFA_FIRE        = 3,    // shooting
    TFA_TERMINATE       = 4,    // ends shooting position
};



class CTaskFireAnt : public CForegroundTask
{
public:
    CTaskFireAnt(COldObject* object);
    ~CTaskFireAnt();

    bool    EventProcess(const Event &event) override;

    Error   Start(Math::Vector impact);
    Error   IsEnded() override;
    bool    Abort() override;

protected:

protected:
    Math::Vector    m_impact;
    TaskFireAnt m_phase = TFA_NULL;
    float       m_progress = 0.0f;
    float       m_speed = 0.0f;
    float       m_angle = 0.0f;
    bool        m_bError = false;
    bool        m_bFire = false;
    float       m_time = 0.0f;
    float       m_lastParticle = 0.0f;
};
