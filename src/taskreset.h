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

// taskreset.h

#ifndef _TASKRESET_H_
#define _TASKRESET_H_


#include "misc.h"
#include "d3dengine.h"


class CInstanceManager;
class CTerrain;
class CBrain;
class CPhysics;
class CObject;



enum TaskResetPhase
{
    TRSP_ZOUT   = 1,    // disappears
    TRSP_MOVE   = 2,    // moves
    TRSP_ZIN    = 3,    // reappears
};



class CTaskReset : public CTask
{
public:
    CTaskReset(CInstanceManager* iMan, CObject* object);
    ~CTaskReset();

    BOOL    EventProcess(const Event &event);

    Error   Start(D3DVECTOR goal, D3DVECTOR angle);
    Error   IsEnded();

protected:
    BOOL    SearchVehicle();

protected:
    D3DVECTOR       m_begin;
    D3DVECTOR       m_goal;
    D3DVECTOR       m_angle;

    TaskResetPhase  m_phase;
    BOOL            m_bError;
    float           m_time;
    float           m_speed;
    float           m_progress;
    float           m_lastParticule;  // time of generation last particle
    float           m_iAngle;
};


#endif //_TASKRESET_H_
