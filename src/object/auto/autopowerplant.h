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

// autopowerplant.h

#pragma once


#include "object/auto/auto.h"


class CObject;


enum AutoPowerPlantPhase
{
    AENP_STOP       = 1,
    AENP_WAIT       = 2,
    AENP_BLITZ      = 3,
    AENP_CREATE     = 4,
    AENP_SMOKE      = 5,
};



class CAutoPowerPlant : public CAuto
{
public:
    CAutoPowerPlant(COldObject* object);
    ~CAutoPowerPlant();

    void        DeleteObject(bool bAll=false);

    void        Init();
    bool        EventProcess(const Event &event);
    Error       GetError();

    bool        CreateInterface(bool bSelect);

    bool        Write(CLevelParserLine* line);
    bool        Read(CLevelParserLine* line);

protected:
    void        UpdateInterface(float rTime);

    CObject*    SearchMetal();
    bool        SearchVehicle();
    void        CreatePower();
    CObject*    SearchPower();

protected:
    AutoPowerPlantPhase     m_phase = AENP_STOP;
    float               m_progress = 0.0f;
    float               m_speed = 0.0;
    float               m_timeVirus = 0.0f;
    float               m_lastUpdateTime = 0.0f;
    float               m_lastParticle = 0.0f;
    int                 m_partiSphere = 0;
};

