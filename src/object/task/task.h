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

#pragma once


#include "common/event.h"
#include "common/global.h"

#include "math/const.h"


class CBrain;
class CPhysics;
class CMotion;
class CObject;
class CRobotMain;
class CSoundInterface;


namespace Gfx {
class CEngine;
class CLightManager;
class CParticle;
class CTerrain;
class CWater;
class CCamera;
} /* Gfx */


const float TAKE_DIST       = 6.0f; // distance to an object to pick it
const float TAKE_DIST_OTHER = 1.5f; // additional distance if on friend

//?const float ARM_NEUTRAL_ANGLE1 = 155.0f*Math::PI/180.0f;
//?const float ARM_NEUTRAL_ANGLE2 = -125.0f*Math::PI/180.0f;
//?const float ARM_NEUTRAL_ANGLE3 = -45.0f*Math::PI/180.0f;
const float ARM_NEUTRAL_ANGLE1 = 110.0f*Math::PI/180.0f;
const float ARM_NEUTRAL_ANGLE2 = -130.0f*Math::PI/180.0f;
const float ARM_NEUTRAL_ANGLE3 = -50.0f*Math::PI/180.0f;

const float ARM_STOCK_ANGLE1 = 110.0f*Math::PI/180.0f;
const float ARM_STOCK_ANGLE2 = -100.0f*Math::PI/180.0f;
const float ARM_STOCK_ANGLE3 = -70.0f*Math::PI/180.0f;


class CTask
{
public:
    CTask(CObject* object);
    virtual ~CTask();

    virtual bool    EventProcess(const Event &event);
    virtual Error   IsEnded();
    virtual bool    IsBusy();
    virtual bool    Abort();

protected:
    Gfx::CEngine*       m_engine;
    Gfx::CLightManager* m_lightMan;
    Gfx::CParticle*     m_particle;
    Gfx::CTerrain*      m_terrain;
    Gfx::CWater*        m_water;
    Gfx::CCamera*       m_camera;
    CMotion*            m_motion;
    CBrain*             m_brain;
    CPhysics*           m_physics;
    CObject*            m_object;
    CRobotMain*         m_main;
    CSoundInterface*    m_sound;
};

