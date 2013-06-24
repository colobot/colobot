// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// * Copyright (C) 2012, Polish Portal of Colobot (PPC)
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

/**
 * \file graphics/engine/pyro.h
 * \brief Fire effect rendering - CPyro class
 */

#pragma once


#include "common/event.h"
#include "common/global.h"

#include "graphics/engine/engine.h"

#include "object/object.h"


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


/**
 * \enum PyroType
 * \brief Type of pyro effect
 */
enum PyroType
{
    PT_NULL     = 0,
    PT_FRAGT    = 1,        //! < fragmentation of technical object
    PT_FRAGO    = 2,        //! < fragmentation of organic object
    PT_FRAGW    = 4,        //! < fragmentation of object under water
    PT_EXPLOT   = 5,        //! < explosion of technical object
    PT_EXPLOO   = 6,        //! < explosion of organic object
    PT_EXPLOW   = 8,        //! < explosion of object under water
    PT_SHOTT    = 9,        //! < hit technical object
    PT_SHOTH    = 10,       //! < hit human
    PT_SHOTM    = 11,       //! < hit queen
    PT_SHOTW    = 12,       //! < hit under water
    PT_EGG      = 13,       //! < break the egg
    PT_BURNT    = 14,       //! < burning of technical object
    PT_BURNO    = 15,       //! < burning of organic object
    PT_SPIDER   = 16,       //! < spider explosion
    PT_FALL     = 17,       //! < cargo falling
    PT_WPCHECK  = 18,       //! < indicator reaches
    PT_FLCREATE = 19,       //! < flag create
    PT_FLDELETE = 20,       //! < flag destroy
    PT_RESET    = 21,       //! < reset position of the object
    PT_WIN      = 22,       //! < fireworks
    PT_LOST     = 23,       //! < black smoke
    PT_DEADG    = 24,       //! < shooting death
    PT_DEADW    = 25,       //! < drowning death
    PT_FINDING  = 26,       //! < object discovered
};


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
public:
    CPyro();
    ~CPyro();

    //! Creates pyrotechnic effect
    bool        Create(PyroType type, CObject* obj, float force=1.0f);
    //! Destroys the object
    void        DeleteObject();

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

    int             m_crashSphereUsed;  // number of spheres used
    Math::Vector    m_crashSpherePos[50];
    float           m_crashSphereRadius[50];
};


} // namespace Gfx

