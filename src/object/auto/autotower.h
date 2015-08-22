/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2015, Daniel Roux, EPSITEC SA & TerranovaTeam
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


#include "object/auto/auto.h"


class CObject;

enum AutoTowerPhase
{
    ATP_WAIT        = 1,
    ATP_ZERO        = 2,    // more energy
    ATP_SEARCH      = 3,    // search a target
    ATP_TURN        = 4,    // turns to the target
    ATP_FIRE        = 5,    // shoots on the target
};



class CAutoTower : public CAuto
{
public:
    CAutoTower(COldObject* object);
    ~CAutoTower();

    void        DeleteObject(bool bAll=false) override;

    void        Init() override;
    bool        EventProcess(const Event &event) override;
    Error       GetError() override;

    bool        CreateInterface(bool bSelect) override;

    bool        Write(CLevelParserLine* line) override;
    bool        Read(CLevelParserLine* line) override;

protected:
    void        UpdateInterface(float rTime);

    CObject*    SearchTarget(Math::Vector &impact);
    void        FireStopUpdate(float progress, bool bLightOn);

protected:
    AutoTowerPhase  m_phase = ATP_WAIT;
    float           m_progress = 0.0f;
    float           m_speed = 0.0f;
    float           m_timeVirus = 0.0f;
    float           m_lastUpdateTime = 0.0f;
    float           m_lastParticle = 0.0f;
    Math::Vector        m_targetPos;
    float           m_angleYactual = 0.0f;
    float           m_angleZactual = 0.0f;
    float           m_angleYfinal = 0.0f;
    float           m_angleZfinal = 0.0f;
    int             m_partiStop[4] = {};
};
