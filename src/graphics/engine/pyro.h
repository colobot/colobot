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

/**
 * \file graphics/engine/pyro.h
 * \brief Fire effect rendering - CPyro class
 */

#pragma once


#include "common/error.h"

#include "graphics/core/color.h"

#include "graphics/engine/pyro_type.h"

#include "math/sphere.h"

#include "object/object_type.h"

#include <vector>

class CObject;
class CRobotMain;
class CSoundInterface;
struct Event;


// Graphics module namespace
namespace Gfx
{

class CEngine;
class CTerrain;
class CCamera;
class CParticle;
class CLightManager;

/**
 * \class CPyro
 * \brief Fire effect renderer
 *
 * TODO: documentation
 */
class CPyro
{
protected:
    friend class CPyroManager;

    //! Creates pyrotechnic effect
    bool        Create(PyroType type, CObject* obj, float force);
    //! Destroys the object
    void        DeleteObject();

public:
    CPyro(); // should only be called by CPyroManager
    ~CPyro();

    //! Indicates whether the pyrotechnic effect is complete
    Error       IsEnded();

    //! Indicates that the object binds to the effect no longer exists, without deleting it
    void        CutObjectLink(CObject* obj);

    //! Management of an event
    bool        EventProcess(const Event& event);

protected:
    //! Displays the error or eventual information
    //! Information can be linked to the destruction of an insect, a vehicle or building
    void        DisplayError(PyroType type, CObject* obj);

    //! Creates light to accompany a pyrotechnic effect
    void        CreateLight(Math::Vector pos, float height);
    //! Removes the binding to a pyrotechnic effect
    void        DeleteObject(bool primary, bool secondary);

    //! Creates an explosion with triangular form of particles
    void        CreateTriangle(CObject* obj, ObjectType oType, int part);

    //! Starts the explosion of a vehicle
    void        ExploStart();
    //! Ends the explosion of a vehicle
    void        ExploTerminate();

    //! Starts a vehicle fire
    void        BurnStart();
    //! Adds a part move
    void        BurnAddPart(int part, Math::Vector pos, Math::Vector angle);
    //! Advances of a vehicle fire
    void        BurnProgress();
    //! Indicates whether a part should be retained
    bool        BurnIsKeepPart(int part);
    //! Ends the fire of an insect or a vehicle
    void        BurnTerminate();

    //! Start of an object freight falling
    void        FallStart();
    //! Seeks an object to explode by the falling ball of bees
    CObject*    FallSearchBeeExplo();
    //! Fall of an object's freight
    void        FallProgress(float rTime);
    //! Indicates whether the fall is over
    Error       FallIsEnded();

    //! Empty the table of operations of animation of light
    void        LightOperFlush();
    //! Adds an animation operation of the light
    void        LightOperAdd(float progress, float intensity, float r, float g, float b);
    //! Updates the associated light
    void        LightOperFrame(float rTime);

protected:
    CEngine*          m_engine = nullptr;
    CTerrain*         m_terrain = nullptr;
    CCamera*          m_camera = nullptr;
    CParticle*        m_particle = nullptr;
    CLightManager*    m_lightMan = nullptr;
    CObject*          m_object = nullptr;
    CRobotMain*       m_main = nullptr;
    CSoundInterface*  m_sound = nullptr;

    Math::Vector    m_pos;          // center of the effect
    Math::Vector    m_posPower;     // center of the battery
    bool            m_power = false;       // battery exists?
    PyroType        m_type = PT_NULL;
    float           m_force = 0.0f;
    float           m_size = 0.0f;
    float           m_progress = 0.0f;
    float           m_speed = 0.0f;
    float           m_time = 0.0f;
    float           m_lastParticle = 0.0f;
    float           m_lastParticleSmoke = 0.0f;
    int             m_soundChannel = -1;

    int             m_lightRank = -1;
    float           m_lightHeight = 0.0f;

    struct PyroLightOper
    {
        float      progress = 0.0f;
        float      intensity = 0.0f;
        Color      color;
    };
    std::vector<PyroLightOper> m_lightOper;

    ObjectType      m_burnType = OBJECT_NULL;
    int             m_burnPartTotal = 0;

    struct PyroBurnPart
    {
        int             part = 0;
        Math::Vector    initialPos;
        Math::Vector    finalPos;
        Math::Vector    initialAngle;
        Math::Vector    finalAngle;
    };
    PyroBurnPart    m_burnPart[10];
    int             m_burnKeepPart[10] = {};
    float           m_burnFall = 0.0f;

    float           m_fallFloor = 0.0f;
    float           m_fallSpeed = 0.0f;
    float           m_fallBulletTime = 0.0f;
    bool            m_fallEnding = false;

    std::vector<Math::Sphere> m_crashSpheres;
    float           m_resetAngle = 0.0f;
};


} // namespace Gfx
