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

// taskbuild.h

#ifndef _TASKBUILD_H_
#define _TASKBUILD_H_


#include "misc.h"
#include "object.h"


class CInstanceManager;
class CTerrain;
class CBrain;
class CPhysics;



#define BUILDMARGIN     16.0f
#define TBMAXLIGHT      4


enum TaskBuildPhase
{
    TBP_TURN    = 1,    // turns
    TBP_MOVE    = 2,    // forward/backward
    TBP_TAKE    = 3,    // takes gun
    TBP_PREP    = 4,    // prepares
    TBP_BUILD   = 5,    // builds
    TBP_TERM    = 6,    // ends
    TBP_RECEDE  = 7,    // back terminal
};



class CTaskBuild : public CTask
{
public:
    CTaskBuild(CInstanceManager* iMan, CObject* object);
    ~CTaskBuild();

    BOOL        EventProcess(const Event &event);

    Error       Start(ObjectType type);
    Error       IsEnded();
    BOOL        Abort();

protected:
    Error       FlatFloor();
    BOOL        CreateBuilding(D3DVECTOR pos, float angle);
    void        CreateLight();
    void        BlackLight();
    CObject*    SearchMetalObject(float &angle, float dMin, float dMax, float aLimit, Error &err);
    void        DeleteMark(D3DVECTOR pos, float radius);

protected:
    ObjectType      m_type;         // type of construction
    CObject*        m_metal;        // transforms metal object
    CObject*        m_power;        // the vehicle battery
    CObject*        m_building;     // building built
    TaskBuildPhase      m_phase;        // phase of the operation
    BOOL            m_bError;       // TRUE -> operation impossible
    BOOL            m_bBuild;       // TRUE -> building built
    BOOL            m_bBlack;       // TRUE -> lights black -> white
    float           m_time;         // absolute time
    float           m_lastParticule;    // time of generation last particle
    float           m_progress;     // progression (0..1)
    float           m_speed;        // speed of progression
    float           m_angleY;       // rotation angle of the vehicle
    float           m_angleZ;       // angle of rotation of the gun
    D3DVECTOR       m_buildingPos;      // initial position of the building
    float           m_buildingHeight;   // height of the building
    int         m_lightRank[TBMAXLIGHT];// lights for the effects
    int         m_soundChannel;
};


#endif //_TASKBUILD_H_
