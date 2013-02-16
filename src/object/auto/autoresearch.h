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

// autoresearch.h

#pragma once


#include "object/auto/auto.h"



enum AutoResearchPhase
{
    ALP_WAIT        = 1,
    ALP_SEARCH      = 2,    // research in progress
};



class CAutoResearch : public CAuto
{
public:
    CAutoResearch(CObject* object);
    ~CAutoResearch();

    void        DeleteObject(bool bAll=false);

    void        Init();
    bool        EventProcess(const Event &event);
    Error       GetError();

    bool        CreateInterface(bool bSelect);

    bool        Write(char *line);
    bool        Read(char *line);

protected:
    void        UpdateInterface();
    void        UpdateInterface(float rTime);
    void        OkayButton(Ui::CWindow *pw, EventType event);
    bool        TestResearch(EventType event);
    void        SetResearch(EventType event);
    void        FireStopUpdate(float progress, bool bLightOn);

protected:
    AutoResearchPhase   m_phase;
    float               m_progress;
    float               m_speed;
    float               m_timeVirus;
    float               m_lastUpdateTime;
    float               m_lastParticle;
    EventType            m_research;
    int                 m_partiStop[6];
    int                 m_channelSound;
};

