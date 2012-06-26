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

// taskpen.h

#ifndef _TASKSPEN_H_
#define _TASKSPEN_H_


#include "d3dengine.h"


class CInstanceManager;
class CTerrain;
class CBrain;
class CPhysics;
class CObject;



enum TaskPenPhase
{
    TPP_UP      = 1,    // rises the pencil
    TPP_TURN    = 2,    // turns the carousel
    TPP_DOWN    = 3,    // descends the pencil
};



class CTaskPen : public CTask
{
public:
    CTaskPen(CInstanceManager* iMan, CObject* object);
    ~CTaskPen();

    BOOL        EventProcess(const Event &event);

    Error       Start(BOOL bDown, int color);
    Error       IsEnded();
    BOOL        Abort();

protected:
    void        SoundManip(float time, float amplitude, float frequency);
    int         AngleToRank(float angle);
    float       ColorToAngle(int color);
    int         ColorToRank(int color);

protected:
    BOOL            m_bError;
    TaskPenPhase    m_phase;
    float           m_progress;
    float           m_delay;
    float           m_time;
    float           m_lastParticule;
    D3DVECTOR       m_supportPos;

    float           m_timeUp;
    float           m_oldAngle;
    float           m_newAngle;
    float           m_timeDown;
};


#endif //_TASKSPEN_H_
