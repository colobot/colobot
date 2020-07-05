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


enum AutoDestroyerPhase
{
    ADEP_WAIT       = 1,    // expected metal
    ADEP_DOWN       = 2,    // down the cover
    ADEP_REPAIR     = 3,    // built the vehicle
    ADEP_UP         = 4,    // up the cover
};



class CAutoDestroyer : public CAuto
{
public:
    CAutoDestroyer(COldObject* object);
    ~CAutoDestroyer();

    void        DeleteObject(bool bAll=false) override;

    void        Init() override;
    bool        EventProcess(const Event &event) override;
    Error       GetError() override;

    Error       StartAction(int param) override;

    bool        CreateInterface(bool bSelect) override;

    bool        Write(CLevelParserLine* line) override;
    bool        Read(CLevelParserLine* line) override;

protected:
    CObject*    SearchPlastic();
    void        EnableInterface(Ui::CWindow *pw, EventType event, bool bState);

protected:
    AutoDestroyerPhase  m_phase = ADEP_WAIT;
    float           m_progress = 0.0f;
    float           m_speed = 0.0f;
    float           m_timeVirus = 0.0f;
    float           m_lastParticle = 0.0f;
    bool            m_bExplo = false;
};
