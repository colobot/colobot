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

// autoradar.h

#pragma once


#include "object/auto/auto.h"



enum AutoRadarPhase
{
    ARAP_WAIT       = 1,    // waiting
    ARAP_SEARCH     = 2,    // seeking
    ARAP_SHOW       = 3,    // watching
    ARAP_SINUS      = 4,    // oscillates
};



class CAutoRadar : public CAuto
{
public:
    CAutoRadar(CObject* object);
    ~CAutoRadar();

    void        DeleteObject(bool bAll=false);

    void        Init();
    bool        EventProcess(const Event &event);
    bool        CreateInterface(bool bSelect);
    Error       GetError();

protected:
    void        UpdateInterface();
    bool        SearchEnemy(Math::Vector &pos);

protected:
    AutoRadarPhase  m_phase;
    float           m_progress;
    float           m_speed;
    float           m_aTime;
    float           m_timeVirus;
    float           m_lastParticle;
    float           m_angle;
    float           m_start;
    int             m_totalDetect;
};

