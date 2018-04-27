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

#include "object/interface/trace_drawing_object.h"


enum TaskPenPhase
{
    TPP_UP      = 1,    // rises the pencil
    TPP_TURN    = 2,    // turns the carousel
    TPP_DOWN    = 3,    // descends the pencil
};



class CTaskPen : public CForegroundTask
{
public:
    CTaskPen(COldObject* object);
    ~CTaskPen();

    bool        EventProcess(const Event &event) override;

    Error       Start(bool bDown, TraceColor color);
    Error       IsEnded() override;
    bool        Abort() override;

protected:
    void        SoundManip(float time, float amplitude, float frequency);
    int         AngleToRank(float angle);
    float       ColorToAngle(TraceColor color);
    int         ColorToRank(TraceColor color);

protected:
    bool            m_bError = false;
    TaskPenPhase    m_phase = TPP_UP;
    float           m_progress = 0.0f;
    float           m_delay = 0.0f;
    float           m_time = 0.0f;
    float           m_lastParticle = 0.0f;
    Math::Vector    m_supportPos;

    float           m_timeUp = 0.0f;
    float           m_oldAngle = 0.0f;
    float           m_newAngle = 0.0f;
    float           m_timeDown = 0.0f;
};
