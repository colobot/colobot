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


#include "object/auto/auto.h"


class CObject;

enum AutoNuclearPlantPhase
{
    ANUP_STOP       = 1,
    ANUP_WAIT       = 2,
    ANUP_CLOSE      = 3,
    ANUP_GENERATE   = 4,
    ANUP_OPEN       = 5,
};



class CAutoNuclearPlant : public CAuto
{
public:
    CAutoNuclearPlant(COldObject* object);
    ~CAutoNuclearPlant();

    void        DeleteObject(bool all=false) override;

    void        Init() override;
    bool        EventProcess(const Event &event) override;
    Error       GetError() override;

    bool        CreateInterface(bool bSelect) override;

    bool        Write(CLevelParserLine* line) override;
    bool        Read(CLevelParserLine* line) override;

protected:
    CObject*    SearchUranium();
    bool        SearchVehicle();
    void        CreatePower();

protected:
    AutoNuclearPlantPhase    m_phase = ANUP_STOP;
    float               m_progress = 0.0f;
    float               m_speed = 0.0f;
    float               m_timeVirus = 0.0f;
    float               m_lastParticle = 0.0f;
    Math::Vector            m_pos;
    int                 m_channelSound = 0;
};
