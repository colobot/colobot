// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// * Copyright (C) 2012, Polish Portal of Colobot (PPC)
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.

// autolabo.h

#pragma once


#include "object/auto/auto.h"



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
    CAutoLabo(CObject* object);
    ~CAutoLabo();

    void        DeleteObject(bool bAll=false);

    void        Init();
    Error       StartAction(int param);
    bool        EventProcess(const Event &event);
    Error       GetError();

    bool        CreateInterface(bool bSelect);

    bool        Write(char *line);
    bool        Read(char *line);

protected:
    void        UpdateInterface();
    void        OkayButton(Ui::CWindow *pw, EventType event);
    bool        TestResearch(EventType event);
    void        SetResearch(EventType event);
    void        SoundManip(float time, float amplitude, float frequency);

protected:
    AutoLaboPhase       m_phase;
    float               m_progress;
    float               m_speed;
    float               m_timeVirus;
    float               m_lastParticle;
    ResearchType        m_research;
    int                 m_partiRank[3];
    int                 m_partiSphere;
    int                 m_soundChannel;
};

