/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2014, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsiteс.ch; http://colobot.info; http://github.com/colobot
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


#include "common/event.h"
#include "common/global.h"

#include "graphics/core/color.h"
#include "graphics/engine/pyro_type.h"

#include "math/sphere.h"

#include "object/object_type.h"

#include <vector>

class CObject;
class CRobotMain;
class CSoundInterface;


// Graphics module namespace
namespace Gfx {

class CEngine;
class CTerrain;
class CCamera;
class CParticle;
class CLight;
class CLightManager;


struct PyroBurnPart
{
    int             part;
    Math::Vector    initialPos;
    Math::Vector    finalPos;
    Math::Vector    initialAngle;
    Math::Vector    finalAngle;
};

struct PyroLightOper
{
    float      progress;
    float      intensity;
    Color      color;
};


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

    CPyro();
    //! Creates pyrotechnic effect
    bool        Create(PyroType type, CObject* obj, float force);
    //! Destroys the object
    void        DeleteObject();

public:
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
    void        CreateTriangle(CObject* obj, ObjectType type, int part);

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
    CEngine*          m_engine;
    CTerrain*         m_terrain;
    CCamera*          m_camera;
    CParticle*        m_particle;
    CLightManager*    m_lightMan;
    CObject*          m_object;
    CRobotMain*       m_main;
    CSoundInterface*  m_sound;

    Math::Vector    m_pos;          // center of the effect
    Math::Vector    m_posPower;     // center of the battery
    bool            m_power;       // battery exists?
    PyroType        m_type;
    float           m_force;
    float           m_size;
    float           m_progress;
    float           m_speed;
    float           m_time;
    float           m_lastParticle;
    float           m_lastParticleSmoke;
    int             m_soundChannel;

    int             m_lightRank;
    int             m_lightOperTotal;
    PyroLightOper   m_lightOper[10];
    float           m_lightHeight;

    ObjectType      m_burnType;
    int             m_burnPartTotal;
    PyroBurnPart    m_burnPart[10];
    int             m_burnKeepPart[10];
    float           m_burnFall;

    float           m_fallFloor;
    float           m_fallSpeed;
    float           m_fallBulletTime;
    bool            m_fallEnding;

    std::vector<Math::Sphere> m_crashSpheres;
};


} // namespace Gfx

