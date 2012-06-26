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

// task.h

#ifndef _TASK_H_
#define _TASK_H_


#include "misc.h"


class CInstanceManager;
class CD3DEngine;
class CEngine;
class CLight;
class CParticule;
class CTerrain;
class CWater;
class CCamera;
class CBrain;
class CPhysics;
class CMotion;
class CObject;
class CRobotMain;
class CDisplayText;
class CSound;


#define TAKE_DIST       6.0f        // distance to an object to pick it
#define TAKE_DIST_OTHER     1.5f        // additional distance if on friend

//?#define ARM_NEUTRAL_ANGLE1    155.0f*PI/180.0f
//?#define ARM_NEUTRAL_ANGLE2   -125.0f*PI/180.0f
//?#define ARM_NEUTRAL_ANGLE3    -45.0f*PI/180.0f
#define ARM_NEUTRAL_ANGLE1   110.0f*PI/180.0f
#define ARM_NEUTRAL_ANGLE2  -130.0f*PI/180.0f
#define ARM_NEUTRAL_ANGLE3   -50.0f*PI/180.0f

#define ARM_STOCK_ANGLE1     110.0f*PI/180.0f
#define ARM_STOCK_ANGLE2    -100.0f*PI/180.0f
#define ARM_STOCK_ANGLE3     -70.0f*PI/180.0f


class CTask
{
public:
    CTask(CInstanceManager* iMan, CObject* object);
    virtual ~CTask();

    virtual BOOL    EventProcess(const Event &event);
    virtual Error   IsEnded();
    virtual BOOL    IsBusy();
    virtual BOOL    Abort();

protected:

protected:
    CInstanceManager* m_iMan;
    CD3DEngine*     m_engine;
    CLight*         m_light;
    CParticule*     m_particule;
    CTerrain*       m_terrain;
    CWater*         m_water;
    CCamera*        m_camera;
    CMotion*        m_motion;
    CBrain*         m_brain;
    CPhysics*       m_physics;
    CObject*        m_object;
    CRobotMain*     m_main;
    CDisplayText*   m_displayText;
    CSound*         m_sound;
};


#endif //_TASK_H_
