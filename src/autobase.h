// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
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

#ifndef _AUTOBASE_H_
#define _AUTOBASE_H_


#include "auto.h"
#include "misc.h"


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



#define PARAM_STOP          0       // run=0 -> stops and open
#define PARAM_LANDING           1       // run=1 -> landing
#define PARAM_PORTICO           2       // run=2 -> gate on the ground
#define PARAM_FIXSCENE          3       // run=3 -> open and stops to win / lost
#define PARAM_TRANSIT1          11      // run=11 -> transit in space
#define PARAM_TRANSIT2          12      // run=12 -> transit in space
#define PARAM_TRANSIT3          13      // run=13 -> transit in space


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
    CAutoBase(CInstanceManager* iMan, CObject* object);
    ~CAutoBase();

    void        DeleteObject(BOOL bAll=FALSE);

    void        Init();
    void        Start(int param);
    BOOL        EventProcess(const Event &event);
    BOOL        Abort();
    Error       RetError();

    BOOL        CreateInterface(BOOL bSelect);

protected:
    void        UpdateInterface();
    void        FreezeCargo(BOOL bFreeze);
    void        MoveCargo();
    Error       CheckCloseDoor();
    void        BeginTransit();
    void        EndTransit();

protected:
    AutoBasePhase   m_phase;
    BOOL            m_bOpen;
    float           m_progress;
    float           m_speed;
    float           m_lastParticule;
    float           m_lastMotorParticule;
    float           m_fogStart;
    float           m_deepView;
    D3DVECTOR       m_pos;
    D3DVECTOR       m_posSound;
    D3DVECTOR       m_finalPos;
    D3DVECTOR       m_lastPos;
    int             m_param;
    int             m_soundChannel;
    int             m_partiChannel[8];

    char            m_bgBack[100];
    char            m_bgName[100];
    D3DCOLOR        m_bgUp;
    D3DCOLOR        m_bgDown;
    D3DCOLOR        m_bgCloudUp;
    D3DCOLOR        m_bgCloudDown;
};


#endif //_AUTOBASE_H_
