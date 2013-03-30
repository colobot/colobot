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

// autoportico.h

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
    CAutoPortico(CObject* object);
    ~CAutoPortico();

    void        DeleteObject(bool bAll=false);

    void        Init();
    void        Start(int param);
    bool        EventProcess(const Event &event);
    bool        Abort();
    Error       GetError();

protected:
    void        UpdateTrackMapping(float left, float right);

protected:
    AutoPorticoPhase m_phase;
    float           m_progress;
    float           m_speed;
    float           m_cameraProgress;
    float           m_cameraSpeed;
    float           m_lastParticle;
    Math::Vector        m_finalPos;
    Math::Vector        m_startPos;
    float           m_posTrack;
    int             m_param;
    int             m_soundChannel;
};

