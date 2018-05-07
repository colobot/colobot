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

#include "math/vector.h"

#include "object/object_type.h"

namespace Gfx
{
class CEngine;
class CParticle;
class CTerrain;
class CWater;
class CCamera;
class COldModelManager;
} // namespace Gfx

class CApplication;
class CPhysics;
class COldObject;
class CRobotMain;
class CSoundInterface;
class CLevelParserLine;
struct Event;


class CMotion
{
public:
    CMotion(COldObject* object);
    virtual ~CMotion();

    void    SetPhysics(CPhysics* physics);

    virtual void            DeleteObject(bool bAll=false) = 0;
    virtual void            Create(Math::Vector pos, float angle, ObjectType type, float power, Gfx::COldModelManager* modelManager) = 0;
    virtual bool            EventProcess(const Event &event);
    virtual Error           SetAction(int action, float time=0.2f);
    virtual int             GetAction();

    virtual bool            SetParam(int rank, float value);
    virtual float           GetParam(int rank);

    virtual bool            Write(CLevelParserLine* line);
    virtual bool            Read(CLevelParserLine* line);

    virtual void            SetLinVibration(Math::Vector dir);
    virtual Math::Vector    GetLinVibration();
    virtual void            SetCirVibration(Math::Vector dir);
    virtual Math::Vector    GetCirVibration();
    virtual void            SetTilt(Math::Vector dir);
    virtual Math::Vector    GetTilt();

protected:
    CApplication*       m_app;
    Gfx::CEngine*       m_engine;
    Gfx::CParticle*     m_particle;
    Gfx::CTerrain*      m_terrain;
    Gfx::CWater*        m_water;
    Gfx::CCamera*       m_camera;
    COldObject*         m_object;
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
