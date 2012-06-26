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

// tasksearch.h

#ifndef _TASKSEARCH_H_
#define _TASKSEARCH_H_


#include "misc.h"


class CInstanceManager;
class CTerrain;
class CBrain;
class CPhysics;
class CObject;



enum TaskSearchHand
{
    TSH_UP      = 1,    // sensor at the top
    TSH_DOWN    = 2,    // sensor at the bottom
};

enum TaskSearchPhase
{
    TSP_DOWN    = 1,    // descends
    TSP_SEARCH  = 2,    // seeks
    TSP_UP      = 3,    // rises
};



class CTaskSearch : public CTask
{
public:
    CTaskSearch(CInstanceManager* iMan, CObject* object);
    ~CTaskSearch();

    BOOL        EventProcess(const Event &event);

    Error       Start();
    Error       IsEnded();
    BOOL        Abort();

protected:
    void    InitAngle();
    BOOL    CreateMark();
    void    DeleteMark(ObjectType type);

protected:
    TaskSearchHand  m_hand;
    TaskSearchPhase m_phase;
    float           m_progress;
    float           m_speed;
    float           m_time;
    float           m_lastParticule;
    float           m_initialAngle[3];
    float           m_finalAngle[3];
    BOOL            m_bError;
};


#endif //_TASKSEARCH_H_
