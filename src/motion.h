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

// motion.h

#ifndef _MOTION_H_
#define _MOTION_H_


#include "d3dengine.h"


class CInstanceManager;
class CEngine;
class CLight;
class CParticule;
class CTerrain;
class CWater;
class CCamera;
class CBrain;
class CPhysics;
class CObject;
class CRobotMain;
class CSound;


class CMotion
{
public:
    CMotion(CInstanceManager* iMan, CObject* object);
    virtual ~CMotion();

    void    SetPhysics(CPhysics* physics);
    void    SetBrain(CBrain* brain);

    virtual void    DeleteObject(BOOL bAll=FALSE);
    virtual BOOL    Create(D3DVECTOR pos, float angle, ObjectType type, float power);
    virtual BOOL    EventProcess(const Event &event);
    virtual Error   SetAction(int action, float time=0.2f);
    virtual int     RetAction();

    virtual BOOL    SetParam(int rank, float value);
    virtual float   RetParam(int rank);

    virtual BOOL    Write(char *line);
    virtual BOOL    Read(char *line);

    virtual void        SetLinVibration(D3DVECTOR dir);
    virtual D3DVECTOR   RetLinVibration();
    virtual void        SetCirVibration(D3DVECTOR dir);
    virtual D3DVECTOR   RetCirVibration();
    virtual void        SetInclinaison(D3DVECTOR dir);
    virtual D3DVECTOR   RetInclinaison();

protected:

protected:
    CInstanceManager* m_iMan;
    CD3DEngine*     m_engine;
    CLight*         m_light;
    CParticule*     m_particule;
    CTerrain*       m_terrain;
    CWater*         m_water;
    CCamera*        m_camera;
    CObject*        m_object;
    CBrain*         m_brain;
    CPhysics*       m_physics;
    CRobotMain*     m_main;
    CSound*         m_sound;

    int             m_actionType;
    float           m_actionTime;
    float           m_progress;

    D3DVECTOR       m_linVibration;     // linear vibration
    D3DVECTOR       m_cirVibration;     // circular vibration
    D3DVECTOR       m_inclinaison;      // tilt
};


#endif //_MOTION_H_
