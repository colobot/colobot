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

// taskrecover.h

#ifndef _TASKSRECOVER_H_
#define _TASKSRECOVER_H_


#include "d3dengine.h"


class CInstanceManager;
class CTerrain;
class CBrain;
class CPhysics;
class CObject;



enum TaskRecoverPhase
{
    TRP_TURN    = 1,    // turns
    TRP_MOVE    = 2,    // advance
    TRP_DOWN    = 3,    // descends
    TRP_OPER    = 4,    // operates
    TRP_UP      = 5,    // back
};



class CTaskRecover : public CTask
{
public:
    CTaskRecover(CInstanceManager* iMan, CObject* object);
    ~CTaskRecover();

    BOOL        EventProcess(const Event &event);

    Error       Start();
    Error       IsEnded();
    BOOL        Abort();

protected:
    CObject*    SearchRuin();

protected:
    TaskRecoverPhase m_phase;
    float           m_progress;
    float           m_speed;
    float           m_time;
    float           m_angle;
    float           m_lastParticule;
    BOOL            m_bError;
    CObject*        m_ruin;
    CObject*        m_metal;
    D3DVECTOR       m_recoverPos;
    int             m_soundChannel;
};


#endif //_TASKSRECOVER_H_
