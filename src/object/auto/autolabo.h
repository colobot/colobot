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

#include "level/research_type.h"


enum AutoLaboPhase
{
    ALAP_WAIT       = 1,
    ALAP_OPEN1      = 2,
    ALAP_OPEN2      = 3,
    ALAP_OPEN3      = 4,
    ALAP_ANALYSE    = 5,
    ALAP_CLOSE1     = 6,
    ALAP_CLOSE2     = 7,
    ALAP_CLOSE3     = 8,
};



class CAutoLabo : public CAuto
{
public:
    CAutoLabo(COldObject* object);
    ~CAutoLabo();

    void        DeleteObject(bool bAll=false) override;

    void        Init() override;
    Error       StartAction(int param) override;
    bool        EventProcess(const Event &event) override;
    Error       GetError() override;

    bool        CreateInterface(bool bSelect) override;

    bool        Write(CLevelParserLine* line) override;
    bool        Read(CLevelParserLine* line) override;

protected:
    void        UpdateInterface();
    void        OkayButton(Ui::CWindow *pw, EventType event);
    bool        TestResearch(EventType event);
    void        SoundManip(float time, float amplitude, float frequency);

protected:
    AutoLaboPhase       m_phase = ALAP_WAIT;
    float               m_progress = 0.0f;
    float               m_speed = 0.0f;
    float               m_timeVirus = 0.0f;
    float               m_lastParticle = 0.0f;
    ResearchType        m_research = {};
    int                 m_partiRank[3] = {};
    int                 m_partiSphere = 0;
    int                 m_soundChannel = 0;
};
