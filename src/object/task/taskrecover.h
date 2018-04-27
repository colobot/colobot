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

class CObject;

enum TaskRecoverPhase
{
    TRP_TURN    = 1,    // turns
    TRP_MOVE    = 2,    // advance
    TRP_DOWN    = 3,    // descends
    TRP_OPER    = 4,    // operates
    TRP_UP      = 5,    // back
};



class CTaskRecover : public CForegroundTask
{
public:
    CTaskRecover(COldObject* object);
    ~CTaskRecover();

    bool        EventProcess(const Event &event) override;

    Error       Start();
    Error       IsEnded() override;
    bool        Abort() override;

protected:
    CObject*    SearchRuin();

protected:
    TaskRecoverPhase m_phase = TRP_TURN;
    float           m_progress = 0.0f;
    float           m_speed = 0.0f;
    float           m_time = 0.0f;
    float           m_angle = 0.0f;
    float           m_lastParticle = 0.0f;
    bool            m_bError = false;
    CObject*        m_ruin = nullptr;
    CObject*        m_metal = nullptr;
    Math::Vector    m_recoverPos;
    int             m_soundChannel = 0;
};
