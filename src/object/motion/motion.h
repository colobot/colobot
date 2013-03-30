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

#pragma once


#include "common/event.h"
#include "common/global.h"

#include "object/object.h"


namespace Gfx {
class CEngine;
class CParticle;
class CTerrain;
class CWater;
class CCamera;
}

class CApplication;
class CBrain;
class CPhysics;
class CObject;
class CRobotMain;
class CSoundInterface;


class CMotion
{
public:
    CMotion(CObject* object);
    virtual ~CMotion();

    void    SetPhysics(CPhysics* physics);
    void    SetBrain(CBrain* brain);

    virtual void            DeleteObject(bool bAll=false);
    virtual bool            Create(Math::Vector pos, float angle, ObjectType type, float power);
    virtual bool            EventProcess(const Event &event);
    virtual Error           SetAction(int action, float time=0.2f);
    virtual int             GetAction();

    virtual bool            SetParam(int rank, float value);
    virtual float           GetParam(int rank);

    virtual bool            Write(char *line);
    virtual bool            Read(char *line);

    virtual void            SetLinVibration(Math::Vector dir);
    virtual Math::Vector    GetLinVibration();
    virtual void            SetCirVibration(Math::Vector dir);
    virtual Math::Vector    GetCirVibration();
    virtual void            SetInclinaison(Math::Vector dir);
    virtual Math::Vector    GetInclinaison();

protected:
    CApplication*       m_app;
    Gfx::CEngine*       m_engine;
    Gfx::CParticle*     m_particle;
    Gfx::CTerrain*      m_terrain;
    Gfx::CWater*        m_water;
    Gfx::CCamera*       m_camera;
    CObject*            m_object;
    CBrain*             m_brain;
    CPhysics*           m_physics;
    CRobotMain*         m_main;
    CSoundInterface*    m_sound;

    int                 m_actionType;
    float               m_actionTime;
    float               m_progress;

    Math::Vector        m_linVibration;     // linear vibration
    Math::Vector        m_cirVibration;     // circular vibration
    Math::Vector        m_inclinaison;      // tilt
};

