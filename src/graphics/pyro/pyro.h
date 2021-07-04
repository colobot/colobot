/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2020, Daniel Roux, EPSITEC SA & TerranovaTeam
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
 * \file graphics/pyro/pyro.h
 * \brief Fire effect rendering - CPyro class
 */

#pragma once


#include "common/error.h"

#include "graphics/core/color.h"

#include "graphics/pyro/pyro_type.h"

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
    bool        Create();
    //! Destroys the object
    void        DeleteObject();

    virtual void AfterCreate();
    virtual void UpdateEffect();
    virtual void AfterEnd();

public:
    CPyro(PyroType type, CObject *obj);
    virtual ~CPyro();

    //! Indicates whether the pyrotechnic effect is complete
    virtual Error IsEnded();

    //! Indicates that the object binds to the effect no longer exists, without deleting it
    void        CutObjectLink(CObject* obj);

    //! Management of an event
    virtual bool EventProcess(const Event& event);

protected:
    //! Creates light to accompany a pyrotechnic effect
    void        CreateLight(Math::Vector pos, float height);
    //! Removes the binding to a pyrotechnic effect
    void        DeleteObject(bool primary, bool secondary);

    //! Creates an explosion with triangular form of particles
    void        CreateTriangle(CObject* obj, ObjectType oType, int part,
            float maxHParticleSpeed = 15.0f,
            float maxVParticleSpeed = 30.0f,
            float minParticleMass = 15.0f,
            float maxParticleMass = 25.0f,
            float overridePercent = -1);

    //! Empty the table of operations of animation of light
    void        LightOperFlush();
    //! Adds an animation operation of the light
    void        LightOperAdd(float progress, float intensity, float r, float g, float b);
    //! Updates the associated light
    void        LightOperFrame();

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

class CFlagCreatePyro : public CPyro
{
public:
    CFlagCreatePyro(CObject *pObj);
    void AfterCreate() override;
    void UpdateEffect() override;
    void AfterEnd() override;
};

class CFlagDeletePyro : public CPyro
{
public:
    CFlagDeletePyro(CObject *pObj);
    void AfterCreate() override;
    void UpdateEffect() override;
    void AfterEnd() override;
};

class CWaypointHitPyro : public CPyro
{
public:
    CWaypointHitPyro(CObject *obj);
    void AfterCreate() override;
    void UpdateEffect() override;
    void AfterEnd() override;
};

class CFallPyro : public CPyro
{
public:
    CFallPyro(CObject *obj);

    //! Start of an object freight falling
    void        AfterCreate() override;
    //! Seeks an object to explode by the falling ball of bees
    CObject*    FallSearchBeeExplo();
    //! Fall of an object's freight
    void        FallProgress(float rTime);
    //! Indicates whether the fall is over
    Error       IsEnded() override;
    //! Makes a sound and unlocks the object after fall is done
    void        AfterEnd() override;

    bool EventProcess(const Event&) override;
};

class CResetPyro : public CPyro
{
public:
    CResetPyro(CObject *obj);
    void AfterCreate() override;
    void UpdateEffect() override;
    void AfterEnd() override;
};

class CLostPyro : public CPyro
{
public:
    CLostPyro(CObject *obj);
    Error IsEnded() override;
    void UpdateEffect() override;
};

class CWinPyro : public CPyro
{
public:
    CWinPyro(CObject *obj);
    Error IsEnded() override;
    void UpdateEffect() override;
};

class CSpiderPyro : public CPyro
{
public:
    CSpiderPyro(CObject *obj);
    bool EventProcess(const Event& event) override;
    void AfterCreate() override;
};

class CEggPyro : public CPyro
{
public:
    CEggPyro(CObject *obj);
    bool EventProcess(const Event& event) override;
    void AfterCreate() override;
};

class CDeadGPyro : public CPyro
{
public:
    CDeadGPyro(CObject *obj);
    void AfterCreate() override;
};

class CDeadWPyro : public CPyro
{
public:
    CDeadWPyro(CObject *obj);
    void AfterCreate() override;
};

class CFindingPyro : public CPyro
{
public:
    CFindingPyro(CObject *obj);
    void AfterCreate() override;
    void UpdateEffect() override;
};

class CSquashPyro : public CPyro
{
public:
    CSquashPyro(CObject *obj);
    void AfterCreate() override;
    void UpdateEffect() override;
    void AfterEnd() override;
};

class CFragVPyro : public CPyro
{
public:
    CFragVPyro(CObject *obj);
    bool EventProcess(const Event& event) override;
    void AfterCreate() override;
};

class CBurnPyro : public CPyro
{
public:
    CBurnPyro(PyroType type, CObject *obj);
    void AfterCreate() override;
    void UpdateEffect() override;

    //! Starts a vehicle fire
    void        BurnStart();
    //! Adds a part move
    void        BurnAddPart(int part, Math::Vector pos, Math::Vector angle);
    //! Advances of a vehicle fire
    void        BurnProgress();
    //! Indicates whether a part should be retained
    bool        BurnIsKeepPart(int part);
    //! Ends the fire of an insect or a vehicle
    void        AfterEnd() override;
};

class CFragExploOrShotPyro : public CPyro
{
    float m_force;
public:
    CFragExploOrShotPyro(PyroType type, CObject *obj, float force = 1.0f);
    void AfterCreate() override;
    bool EventProcess(const Event& event) override;
    void UpdateEffect() override;

    //! Starts the explosion of a vehicle
    void        ExploStart();
    //! Ends the explosion of a vehicle
    void        AfterEnd() override;
};

} // namespace Gfx
