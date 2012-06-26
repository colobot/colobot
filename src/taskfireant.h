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

// taskfireant.h

#ifndef _TASKFIREANT_H_
#define _TASKTIREANT_H_


#include "d3dengine.h"


class CInstanceManager;
class CTerrain;
class CBrain;
class CPhysics;
class CObject;



enum TaskFireAnt
{
    TFA_NULL        = 0,    // nothing to do
    TFA_TURN        = 1,    // turns
    TFA_PREPARE     = 2,    // prepares shooting position
    TFA_FIRE        = 3,    // shooting
    TFA_TERMINATE       = 4,    // ends shooting position
};



class CTaskFireAnt : public CTask
{
public:
    CTaskFireAnt(CInstanceManager* iMan, CObject* object);
    ~CTaskFireAnt();

    BOOL    EventProcess(const Event &event);

    Error   Start(D3DVECTOR impact);
    Error   IsEnded();
    BOOL    Abort();

protected:

protected:
    D3DVECTOR   m_impact;
    TaskFireAnt m_phase;
    float       m_progress;
    float       m_speed;
    float       m_angle;
    BOOL        m_bError;
    BOOL        m_bFire;
    float       m_time;
    float       m_lastParticule;
};


#endif //_TASKFIREANT_H_
