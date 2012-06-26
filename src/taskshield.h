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

// taskshield.h

#ifndef _TASKSHIELD_H_
#define _TASKSHIELD_H_


#include "misc.h"
#include "d3dengine.h"


class CInstanceManager;
class CTerrain;
class CBrain;
class CPhysics;
class CObject;


#define RADIUS_SHIELD_MIN    40.0f      // minimal radius of the protected zone
#define RADIUS_SHIELD_MAX   100.0f      // maximal radius of the protected zone


enum TaskShieldPhase
{
    TS_UP1      = 1,    // up
    TS_UP2      = 2,    // up
    TS_SHIELD   = 3,    // shield deployed
    TS_SMOKE    = 4,    // smoke
    TS_DOWN1    = 5,    // down
    TS_DOWN2    = 6,    // down
};

enum TaskShieldMode
{
    TSM_UP      = 1,    // deploys shield
    TSM_DOWN    = 2,    // returns the shield
    TSM_UPDATE  = 3,    // radius change
};



class CTaskShield : public CTask
{
public:
    CTaskShield(CInstanceManager* iMan, CObject* object);
    ~CTaskShield();

    BOOL        EventProcess(const Event &event);

    Error       Start(TaskShieldMode mode, float delay);
    Error       IsEnded();
    BOOL        IsBusy();
    BOOL        Abort();

protected:
    Error       Stop();
    BOOL        CreateLight(D3DVECTOR pos);
    void        IncreaseShield();
    float       RetRadius();

protected:
    TaskShieldPhase m_phase;
    float           m_progress;
    float           m_speed;
    float           m_time;
    float           m_delay;
    float           m_lastParticule;
    float           m_lastRay;
    float           m_lastIncrease;
    float           m_energyUsed;
    BOOL            m_bError;
    D3DVECTOR       m_shieldPos;
    int             m_rankSphere;
    int             m_soundChannel;
    int             m_effectLight;
};


#endif //_TASKSHIELD_H_
