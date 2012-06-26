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

// taskgungoal.h

#ifndef _TASKGUNGOAL_H_
#define _TASKGUNGOAL_H_


#include "misc.h"


class CInstanceManager;
class CTerrain;
class CBrain;
class CPhysics;
class CObject;


class CTaskGunGoal : public CTask
{
public:
    CTaskGunGoal(CInstanceManager* iMan, CObject* object);
    ~CTaskGunGoal();

    BOOL        EventProcess(const Event &event);

    Error       Start(float dirV, float dirH);
    Error       IsEnded();
    BOOL        Abort();

protected:

protected:
    float       m_progress;
    float       m_speed;
    float       m_initialDirV;  // initial direction
    float       m_finalDirV;    // direction to reach
    float       m_initialDirH;  // initial direction
    float       m_finalDirH;    // direction to reach
};


#endif //_TASKGUNGOAL_H_
