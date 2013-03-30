// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// * Copyright (C) 2012 Polish Portal of Colobot (PPC)
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

// autoinfo.h

#pragma once


#include "object/auto/auto.h"



enum AutoInfoPhase
{
    AIP_WAIT        = 1,
    AIP_EMETTE      = 2,
    AIP_RECEIVE     = 3,
    AIP_ERROR       = 4,
};



class CAutoInfo : public CAuto
{
public:
    CAutoInfo(CObject* object);
    ~CAutoInfo();

    void        DeleteObject(bool bAll=false);

    void        Init();
    void        Start(int param);
    bool        EventProcess(const Event &event);
    Error       GetError();

    bool        CreateInterface(bool bSelect);

    bool        Write(char *line);
    bool        Read(char *line);

protected:
    void        UpdateInterface(float rTime);
    void        UpdateList();
    void        UpdateListVirus();

protected:
    AutoInfoPhase   m_phase;
    float           m_progress;
    float           m_speed;
    float           m_timeVirus;
    float           m_lastParticle;
    Math::Vector        m_goal;
    bool            m_bLastVirus;
};

