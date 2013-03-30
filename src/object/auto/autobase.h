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

// autobase.h

#pragma once


#include "object/auto/auto.h"



enum AutoBaseParam
{
    PARAM_STOP      = 0,        // run=0 -> stops and open
    PARAM_LANDING   = 1,        // run=1 -> landing
    PARAM_PORTICO   = 2,        // run=2 -> gate on the ground
    PARAM_FIXSCENE  = 3,        // run=3 -> open and stops to win / lost
    PARAM_TRANSIT1  = 11,       // run=11 -> transit in space
    PARAM_TRANSIT2  = 12,       // run=12 -> transit in space
    PARAM_TRANSIT3  = 13        // run=13 -> transit in space
};

enum AutoBasePhase
{
    ABP_WAIT        = 1,    // expected
    ABP_START       = 2,    // start-up

    ABP_LAND        = 3,    // landing
    ABP_OPENWAIT        = 4,    // wait before opening
    ABP_OPEN        = 5,    // opens the gate
    ABP_OPEN2       = 6,    // opens supplements
    ABP_LDWAIT      = 7,    // expected

    ABP_CLOSE2      = 8,    // closes supplements
    ABP_CLOSE       = 9,    // closes gate
    ABP_TOWAIT      = 10,   // wait before takeoff
    ABP_TAKEOFF     = 11,   // take-off

    ABP_PORTICO_MOVE = 12,  // gate advance
    ABP_PORTICO_WAIT1= 13,  // gate expected
    ABP_PORTICO_DOWN = 14,  // gate down
    ABP_PORTICO_WAIT2= 15,  // gate expected
    ABP_PORTICO_OPEN = 16,  // gate opens

    ABP_TRANSIT_MOVE = 17,  // transit - moving
};



class CAutoBase : public CAuto
{
public:
    CAutoBase(CObject* object);
    ~CAutoBase();

    void        DeleteObject(bool bAll=false);

    void        Init();
    void        Start(int param);
    bool        EventProcess(const Event &event);
    bool        Abort();
    Error       GetError();

    bool        CreateInterface(bool bSelect);

protected:
    void        UpdateInterface();
    void        FreezeCargo(bool bFreeze);
    void        MoveCargo();
    Error       CheckCloseDoor();
    void        BeginTransit();
    void        EndTransit();

protected:
    AutoBasePhase   m_phase;
    bool            m_bOpen;
    float           m_progress;
    float           m_speed;
    float           m_lastParticle;
    float           m_lastMotorParticle;
    float           m_fogStart;
    float           m_deepView;
    Math::Vector    m_pos;
    Math::Vector    m_posSound;
    Math::Vector    m_finalPos;
    Math::Vector    m_lastPos;
    int             m_param;
    int             m_soundChannel;
    int             m_partiChannel[8];

    std::string     m_bgBack;
    std::string     m_bgName;
    Gfx::Color      m_bgUp;
    Gfx::Color      m_bgDown;
    Gfx::Color      m_bgCloudUp;
    Gfx::Color      m_bgCloudDown;
};

