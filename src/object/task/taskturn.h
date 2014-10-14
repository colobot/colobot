/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2014, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsiteс.ch; http://colobot.info; http://github.com/colobot
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

// taskturn.h

#pragma once


#include "object/task/task.h"



class CTaskTurn : public CTask
{
public:
    CTaskTurn(CObject* object);
    ~CTaskTurn();

    bool    EventProcess(const Event &event);

    Error   Start(float angle);
    Error   IsEnded();

protected:

protected:
    float       m_angle;
    float       m_startAngle;
    float       m_finalAngle;
    bool        m_bLeft;
    bool        m_bError;
};

