/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsitec.ch; http://colobot.info; http://github.com/colobot
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://gnu.org/licenses
 */

#pragma once

#include "common/error.h"

#include "math/const.h"


class CPhysics;
class CMotion;
class COldObject;
class CProgrammableObject;
class CRobotMain;
class CSoundInterface;
struct Event;

namespace Gfx
{
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
    CTask(COldObject* object);
    virtual ~CTask();

    virtual bool    EventProcess(const Event &event);
    virtual Error   IsEnded();
    virtual bool    IsBusy();
    virtual bool    Abort();

    //! Returns true if you can control the robot while the task is executing
    virtual bool    IsPilot() = 0;

    //! Returns true if this task is meant to be run as a background task
    virtual bool    IsBackground() = 0;

protected:
    Gfx::CEngine*       m_engine = nullptr;
    Gfx::CLightManager* m_lightMan = nullptr;
    Gfx::CParticle*     m_particle = nullptr;
    Gfx::CTerrain*      m_terrain = nullptr;
    Gfx::CWater*        m_water = nullptr;
    Gfx::CCamera*       m_camera = nullptr;
    CRobotMain*         m_main = nullptr;
    CSoundInterface*    m_sound = nullptr;

    COldObject*         m_object = nullptr;
    CProgrammableObject* m_programmable = nullptr;
    CMotion*            m_motion = nullptr;
    CPhysics*           m_physics = nullptr;
};

class CForegroundTask : public CTask
{
public:
    CForegroundTask(COldObject* object) : CTask(object) {}

    bool IsBackground() final { return false; }
    bool IsPilot() override { return false; }
};

class CBackgroundTask : public CTask
{
public:
    CBackgroundTask(COldObject* object) : CTask(object) {}

    bool IsBackground() final { return true; }
    bool IsPilot() final { return true; }
};
