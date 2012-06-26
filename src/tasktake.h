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

// tasktake.h

#ifndef _TASKTAKE_H_
#define _TASKTAKE_H_


#include "misc.h"
#include "object.h"


class CInstanceManager;
class CTerrain;
class CBrain;
class CPhysics;
class CObject;



enum TaskTakeOrder
{
    TTO_TAKE    = 1,    // takes an object
    TTO_DEPOSE  = 2,    // deposes the object
};

enum TaskTakeArm
{
    TTA_NEUTRAL = 1,    // empty arm at rest
    TTA_FFRONT  = 2,    // arm on the ground
    TTA_FRIEND  = 3,    // arm behind a friend robot
};



class CTaskTake : public CTask
{
public:
    CTaskTake(CInstanceManager* iMan, CObject* object);
    ~CTaskTake();

    BOOL        EventProcess(const Event &event);

    Error       Start();
    Error       IsEnded();
    BOOL        Abort();

protected:
    CObject*    SearchTakeObject(float &angle, float dLimit, float aLimit);
    CObject*    SearchFriendObject(float &angle, float dLimit, float aLimit);
    BOOL        TruckTakeObject();
    BOOL        TruckDeposeObject();
    BOOL        IsFreeDeposeObject(D3DVECTOR pos);

protected:
    CTerrain*       m_terrain;

    TaskTakeOrder   m_order;
    TaskTakeArm     m_arm;
    int             m_step;
    float           m_speed;
    float           m_progress;
    float           m_height;
    BOOL            m_bError;
    BOOL            m_bTurn;
    float           m_angle;
    ObjectType      m_fretType;
};


#endif //_TASKTAKE_H_
