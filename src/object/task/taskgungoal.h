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



class CTaskGunGoal : public CBackgroundTask
{
public:
    CTaskGunGoal(COldObject* object);
    ~CTaskGunGoal();

    bool        EventProcess(const Event &event) override;

    Error       Start(float dirV, float dirH);
    Error       IsEnded() override;
    bool        Abort() override;

protected:
    float       m_progress = 0.0f;
    float       m_speed = 0.0f;
    float       m_initialDirV = 0.0f;  // initial direction
    float       m_finalDirV = 0.0f;    // direction to reach
    float       m_initialDirH = 0.0f;  // initial direction
    float       m_finalDirH = 0.0f;    // direction to reach

    bool        m_aimImpossible = false; // set to true if impossible aim was set
};
