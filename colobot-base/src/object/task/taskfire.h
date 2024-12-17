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



class CTaskFire : public CForegroundTask
{
public:
    CTaskFire(COldObject* object);
    ~CTaskFire();

    bool        EventProcess(const Event &event) override;

    Error       Start(float delay);
    Error       IsEnded() override;
    bool        Abort() override;
    bool        IsPilot() override;

protected:
    float       m_delay = 0.0f;
    float       m_progress = 0.0f;
    Error       m_error = ERR_OK;
    bool        m_bRay = false;
    bool        m_bOrganic = false;
    float       m_time = 0.0f;
    float       m_speed = 0.0f;
    float       m_lastParticle = 0.0f;
    float       m_lastSound = 0.0f;
    int     m_soundChannel = 0;
};
