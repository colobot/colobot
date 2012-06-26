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

// autotower.h

#ifndef _AUTOTOWER_H_
#define _AUTOTOWER_H_


#include "auto.h"
#include "misc.h"


class CInstanceManager;
class CD3DEngine;
class CParticule;
class CTerrain;
class CCamera;
class CObject;



enum AutoTowerPhase
{
    ATP_WAIT        = 1,
    ATP_ZERO        = 2,    // more energy
    ATP_SEARCH      = 3,    // search a target
    ATP_TURN        = 4,    // turns to the target
    ATP_FIRE        = 5,    // shoots on the target
};



class CAutoTower : public CAuto
{
public:
    CAutoTower(CInstanceManager* iMan, CObject* object);
    ~CAutoTower();

    void        DeleteObject(BOOL bAll=FALSE);

    void        Init();
    BOOL        EventProcess(const Event &event);
    Error       RetError();

    BOOL        CreateInterface(BOOL bSelect);

    BOOL        Write(char *line);
    BOOL        Read(char *line);

protected:
    void        UpdateInterface(float rTime);

    CObject*    SearchTarget(D3DVECTOR &impact);
    void        FireStopUpdate(float progress, BOOL bLightOn);

protected:
    AutoTowerPhase  m_phase;
    float           m_progress;
    float           m_speed;
    float           m_timeVirus;
    float           m_lastUpdateTime;
    float           m_lastParticule;
    D3DVECTOR       m_targetPos;
    float           m_angleYactual;
    float           m_angleZactual;
    float           m_angleYfinal;
    float           m_angleZfinal;
    int             m_partiStop[4];
};


#endif //_AUTOTOWER_H_
