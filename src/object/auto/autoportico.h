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



enum AutoPorticoPhase
{
    APOP_WAIT       = 1,    // waits
    APOP_START      = 2,    // start of the action
    APOP_MOVE       = 3,    // advance
    APOP_WAIT1      = 4,    // waits
    APOP_DOWN       = 5,    // down
    APOP_WAIT2      = 6,    // waits
    APOP_OPEN       = 7,    // opens
};



class CAutoPortico : public CAuto
{
public:
    CAutoPortico(COldObject* object);
    ~CAutoPortico();

    void        DeleteObject(bool bAll=false) override;

    void        Init() override;
    void        Start(int param) override;
    bool        EventProcess(const Event &event) override;
    bool        Abort() override;
    Error       GetError() override;

protected:
    void        UpdateTrackMapping(float left, float right);

protected:
    AutoPorticoPhase m_phase = APOP_WAIT;
    float           m_progress = 0.0f;
    float           m_speed = 0.0f;
    float           m_cameraProgress = 0.0f;
    float           m_cameraSpeed = 0.0f;
    float           m_lastParticle = 0.0f;
    Math::Vector        m_finalPos;
    Math::Vector        m_startPos;
    float           m_posTrack = 0.0f;
    int             m_param = 0;
    int             m_soundChannel = 0;
};
