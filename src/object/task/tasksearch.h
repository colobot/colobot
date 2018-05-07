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

#include "object/object_type.h"



enum TaskSearchHand
{
    TSH_UP      = 1,    // sensor at the top
    TSH_DOWN    = 2,    // sensor at the bottom
};

enum TaskSearchPhase
{
    TSP_DOWN    = 1,    // descends
    TSP_SEARCH  = 2,    // seeks
    TSP_UP      = 3,    // rises
};



class CTaskSearch : public CForegroundTask
{
public:
    CTaskSearch(COldObject* object);
    ~CTaskSearch();

    bool        EventProcess(const Event &event) override;

    Error       Start();
    Error       IsEnded() override;
    bool        Abort() override;

protected:
    void    InitAngle();
    bool    CreateMark();
    void    DeleteMark(ObjectType type);

protected:
    TaskSearchHand  m_hand = TSH_UP;
    TaskSearchPhase m_phase = TSP_DOWN;
    float           m_progress = 0.0f;
    float           m_speed = 0.0f;
    float           m_time = 0.0f;
    float           m_lastParticle = 0.0f;
    float           m_initialAngle[3] = {};
    float           m_finalAngle[3] = {};
    bool            m_bError = false;
};
