/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
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
 * \file graphics/engine/particle.h
 * \brief Particle rendering - CParticle class (aka particle)
 */

#pragma once

#include "graphics/core/color.h"

#include "object/interface/trace_drawing_object.h"

#include "sound/sound_type.h"

#include <vector>

class CRobotMain;
class CObject;
class CSoundInterface;


// Graphics module namespace
namespace Gfx
{

class CDevice;
class CEngine;
class CParticleRenderer;
class CTerrain;
class CWater;

struct EngineTriangle;

const short MAXPARTICULE = 500;
const short MAXPARTITYPE = 6;
const short MAXTRACK = 100;
const short MAXTRACKLEN = 10;
const short MAXPARTIFOG = 100;
const short MAXWHEELTRACE = 1000;

const short SH_WORLD = 0;       // particle in the world in the interface
const short SH_FRONT = 1;       // particle in the world on the interface
const short SH_INTERFACE = 2;   // particle in the interface
const short SH_MAX = 3;

// type == 0    ->  triangles
// type == 1    ->  effect00 (black background)
// type == 2    ->  effect01 (black background)
// type == 3    ->  effect02 (black background)
// type == 4    ->  text     (white background)


enum ParticleType
{
    PARTIEXPLOT     = 1,        //! < technology explosion
    PARTIEXPLOO     = 2,        //! < organic explosion
    PARTIMOTOR      = 3,        //! < the engine exhaust gas
    PARTIGLINT      = 4,        //! < reflection
    PARTIBLITZ      = 5,        //! < lightning recharging battery
    PARTICRASH      = 6,        //! < dust after fall
    PARTIGAS        = 7,        //! < gas from the reactor
    PARTIFIRE       = 9,        //! < fireball shrinks
    PARTIFIREZ      = 10,       //! < fireball grows
    PARTIBLUE       = 11,       //! < blue ball
    PARTISELY       = 12,       //! < yellow robot lights
    PARTISELR       = 13,       //! < red robot lights
    PARTIGUN1       = 18,       //! < bullet 1 (fireball)
    PARTIGUN2       = 19,       //! < bullet 2 (ant)
    PARTIGUN3       = 20,       //! < bullet 3 (spider)
    PARTIGUN4       = 21,       //! < bullet 4 (orgaball)
    PARTIFRAG       = 22,       //! < triangular fragment
    PARTIQUEUE      = 23,       //! < inflamed tail (TODO: unused?)
    PARTIORGANIC1   = 24,       //! < organic ball mother
    PARTIORGANIC2   = 25,       //! < organic ball daughter
    PARTISMOKE1     = 26,       //! < black smoke
    PARTISMOKE2     = 27,       //! < black smoke
    PARTISMOKE3     = 28,       //! < black smoke
    PARTIBLOOD      = 30,       //! < human blood
    PARTIBLOODM     = 31,       //! < AlienQueen blood
    PARTIVAPOR      = 32,       //! < steam
    PARTIVIRUS      = 33,       //! < virus (random letter)
    PARTIRAY1       = 43,       //! < ray 1 (turn)
    PARTIRAY2       = 44,       //! < ray 2 (electric arc)
    PARTIRAY3       = 45,       //! < ray 3 (ExchangePost)
    PARTIFLAME      = 47,       //! < flame
    PARTIBUBBLE     = 48,       //! < bubble
    PARTIFLIC       = 49,       //! < circles in the water
    PARTIEJECT      = 50,       //! < ejection from the reactor
    PARTISCRAPS     = 51,       //! < waste from the reactor
    PARTITOTO       = 52,       //! < Robby's reactor
    PARTIERROR      = 53,       //! < Robby says no
    PARTIWARNING    = 54,       //! < Robby says blah
    PARTIINFO       = 54,       //! < Robby says yes
    PARTIQUARTZ     = 55,       //! < reflection crystal
    PARTISPHERE0    = 56,       //! < explosion sphere
    PARTISPHERE1    = 57,       //! < energy sphere
    PARTISPHERE2    = 58,       //! < analysis sphere
    PARTISPHERE3    = 59,       //! < shield sphere
    PARTISPHERE4    = 60,       //! < information sphere (emit)
    PARTISPHERE5    = 61,       //! < botanical sphere (gravity root)
    PARTISPHERE6    = 62,       //! < information sphere (receive)
    PARTIGUNDEL     = 66,       //! < bullet destroyed by shield
    PARTIPART       = 67,       //! < object part
    PARTITRACK1     = 68,       //! < drag 1
    PARTITRACK2     = 69,       //! < drag 2
    PARTITRACK3     = 70,       //! < drag 3
    PARTITRACK4     = 71,       //! < drag 4
    PARTITRACK5     = 72,       //! < drag 5
    PARTITRACK6     = 73,       //! < drag 6
    PARTITRACK7     = 74,       //! < drag 7
    PARTITRACK8     = 75,       //! < drag 8
    PARTITRACK9     = 76,       //! < drag 9
    PARTITRACK10    = 77,       //! < drag 10
    PARTITRACK11    = 78,       //! < drag 11
    PARTITRACK12    = 79,       //! < drag 12 (TODO: unused?)
    PARTIGLINTb     = 88,       //! < blue reflection
    PARTIGLINTr     = 89,       //! < red reflection
    PARTILENS1      = 90,       //! < brilliance 1 (orange)
    PARTILENS2      = 91,       //! < brilliance 2 (yellow)
    PARTILENS3      = 92,       //! < brilliance 3 (red)
    PARTILENS4      = 93,       //! < brilliance 4 (violet)
    PARTICONTROL    = 94,       //! < reflection on button
    PARTISHOW       = 95,       //! < shows a place
    PARTICHOC       = 96,       //! < shock wave
    PARTIGFLAT      = 97,       //! < shows if the ground is flat
    PARTIRECOVER    = 98,       //! < blue ball recycler
    PARTIROOT       = 100,      //! < gravity root smoke
    PARTIPLOUF0     = 101,      //! < splash
    PARTIDROP       = 106,      //! < drop
    PARTIFOG0       = 107,      //! < fog 0
    PARTIFOG1       = 108,      //! < fog 1
    PARTIFOG2       = 109,      //! < fog 2
    PARTIFOG3       = 110,      //! < fog 3
    PARTIFOG4       = 111,      //! < fog 4
    PARTIFOG5       = 112,      //! < fog 5
    PARTIFOG6       = 113,      //! < fog 6
    PARTIFOG7       = 114,      //! < fog 7
    PARTILIMIT1     = 117,      //! < shows the limits 1
    PARTILIMIT2     = 118,      //! < shows the limits 2
    PARTILIMIT3     = 119,      //! < shows the limits 3
    PARTIWATER      = 121,      //! < drop of water
    PARTIEXPLOG1    = 122,      //! < ball explosion 1
    PARTIEXPLOG2    = 123,      //! < ball explosion 2
    PARTIBASE       = 124,      //! < gases of spaceship
};

enum ParticlePhase
{
    PARPHSTART      = 0,
    PARPHEND        = 1,
};

struct Particle
{
    bool            used = false;      // TRUE -> particle used
    bool            ray = false;       // TRUE -> ray with goal
    unsigned short  uniqueStamp = 0;    // unique mark
    short           sheet = 0;      // sheet (0..n)
    ParticleType    type = {};       // type PARTI*
    ParticlePhase   phase = {};      // phase PARPH*
    float           mass = 0.0f;       // mass of the particle (in rebounding)
    float           weight = 0.0f;     // weight of the particle (for noise)
    float           duration = 0.0f;   // length of life
    glm::vec3       pos = { 0, 0, 0 };        // absolute position (relative if object links)
    glm::vec3       goal = { 0, 0, 0 };       // goal position (if ray)
    glm::vec3       speed = { 0, 0, 0 };      // speed of displacement
    float           windSensitivity = 0.0f;
    short           bounce = 0;     // number of rebounds
    glm::vec2       dim;        // dimensions of the rectangle
    float           zoom = 0.0f;       // zoom (0..1)
    float           angle = 0.0f;      // angle of rotation
    float           intensity = 0.0f;  // intensity
    glm::vec2       texSup;     // coordinated upper texture
    glm::vec2       texInf;     // coordinated lower texture
    float           time = 0.0f;       // age of the particle (0..n)
    float           phaseTime = 0.0f;  // age at the beginning of phase
    float           testTime = 0.0f;   // time since last test
    CObject*        objLink = nullptr;    // father object (for example reactor)
    CObject*        objFather = nullptr;  // father object (for example reactor)
    short           objRank = 0;    // rank of the object, or -1
    short           trackRank = 0;  // rank of the drag
    char            text = 0;
    Color           color = Color(1.0f, 1.0f, 1.0f, 1.0f);
};

struct Track
{
    char            used = 0;      // TRUE -> drag used
    char            drawParticle = 0;
    float           step = 0.0f;       // duration of not
    float           last = 0.0f;       // increase last not memorized
    float           intensity = 0.0f;  // intensity at starting (0..1)
    float           width = 0.0f;      // tail width
    int             posUsed = 0;    // number of positions in "pos"
    int             head = 0;       // head to write index
    glm::vec3    pos[MAXTRACKLEN];
    float           len[MAXTRACKLEN] = {};
};

struct WheelTrace
{
    TraceColor      color = TraceColor::Black;
    glm::vec3    pos[4];
};


/**
 * \class CParticle
 * \brief Particle engine
 *
 * TODO: documentation
 */
class CParticle
{
public:
    CParticle(CEngine* engine);
    ~CParticle();

    //! Sets the device to use
    void        SetDevice(CDevice* device);

    //! Removes all particles
    void        FlushParticle();

    //! Removes all particles of a sheet
    void        FlushParticle(int sheet);

    //! Creates a new particle
    int         CreateParticle(glm::vec3 pos, glm::vec3 speed, const glm::vec2& dim,
                               ParticleType type, float duration = 1.0f, float mass = 0.0f,
                               float windSensitivity = 1.0f, int sheet = 0);

    //! Creates a new triangular particle (debris)
    int         CreateFrag(glm::vec3 pos, glm::vec3 speed, EngineTriangle* triangle,
                           ParticleType type, float duration = 1.0f, float mass = 0.0f,
                           float windSensitivity = 1.0f, int sheet = 0);

    //! Creates a new particle being a part of object
    int         CreatePart(glm::vec3 pos, glm::vec3 speed, ParticleType type,
                           float duration = 1.0f, float mass = 0.0f, float weight = 0.0f,
                           float windSensitivity = 1.0f, int sheet = 0);

    //! Creates a new linear particle (radius)
    int         CreateRay(glm::vec3 pos, glm::vec3 goal, ParticleType type, const glm::vec2& dim,
                          float duration = 1.0f, int sheet = 0);

    //! Creates a particle with a trail
    int         CreateTrack(glm::vec3 pos, glm::vec3 speed, const glm::vec2& dim, ParticleType type,
                            float duration = 1.0f, float mass = 0.0f, float length = 10.0f, float width = 1.0f);

    //! Creates a tire mark
    void        CreateWheelTrace(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3,
                                 const glm::vec3 &p4, TraceColor color);

    //! Removes all particles of a given type
    void        DeleteParticle(ParticleType type);
    //! Removes all particles of a given channel
    void        DeleteParticle(int channel);
    //! Specifies the object to which the particle is bound
    void        SetObjectLink(int channel, CObject *object);
    //! Specifies the parent object that created the particle
    void        SetObjectFather(int channel, CObject *object);
    void        SetPosition(int channel, glm::vec3 pos);
    void        SetDimension(int channel, const glm::vec2& dim);
    void        SetZoom(int channel, float zoom);
    void        SetAngle(int channel, float angle);
    void        SetIntensity(int channel, float intensity);
    void        SetParam(int channel, glm::vec3 pos, const glm::vec2& dim, float zoom, float angle, float intensity);
    void        SetPhase(int channel, ParticlePhase phase, float duration);

    //! Returns the position of the particle
    bool        GetPosition(int channel, glm::vec3 &pos);

    //! Returns the color if you're in the fog or black if you're not
    Color       GetFogColor(glm::vec3 pos);

    //! Indicates whether a sheet is updated or not
    void        SetFrameUpdate(int sheet, bool update);
    //! Updates all the particles.
    void        FrameParticle(float rTime);
    //! Draws all the particles
    void        DrawParticle(int sheet);

    //! Indicates that the object binds to the particle no longer exists, without deleting it
    void        CutObjectLink(CObject* obj);

protected:
    //! Removes a particle of given rank
    void        DeleteRank(int rank);
    /**
     * \brief Adapts the channel so it can be used as an offset in m_particle
     * \param channel Channel number to process, will be modified to be index of particle in m_particle
     * \return true if success, false if particle doesn't exist anymore
     **/
    bool        CheckChannel(int &channel);
    //! Draws a triangular particle
    void        DrawParticleTriangle(int i);
    //! Draw a normal particle
    void        DrawParticleNorm(int i);
    //! Draw a particle flat (horizontal)
    void        DrawParticleFlat(int i);
    //! Draw a particle to a flat sheet of fog
    void        DrawParticleFog(int i);
    //! Draw a particle in the form of radius
    void        DrawParticleRay(int i);
    //! Draws a spherical particle
    void        DrawParticleSphere(int i);
    //! Draws a cylindrical particle
    void        DrawParticleCylinder(int i);
    //! Draws a text particle
    void        DrawParticleText(int i);
    //! Draws a tire mark
    void        DrawParticleWheel(int i);
    //! Seeks if an object collided with a bullet
    CObject*    SearchObjectGun(glm::vec3 old, glm::vec3 pos, ParticleType type, CObject *father);
    //! Seeks if an object collided with a ray
    CObject*    SearchObjectRay(glm::vec3 pos, glm::vec3 goal, ParticleType type, CObject *father);
    //! Sounded one
    void        Play(SoundType sound, glm::vec3 pos, float amplitude);
    //! Moves a drag; returns true if the drag is finished
    bool        TrackMove(int i, glm::vec3 pos, float progress);
    //! Draws a drag
    void        TrackDraw(int i, ParticleType type);

protected:
    CEngine*     m_engine = nullptr;
    CDevice*     m_device = nullptr;
    CTerrain*    m_terrain = nullptr;
    CWater*      m_water = nullptr;
    CRobotMain*       m_main = nullptr;
    CSoundInterface*  m_sound = nullptr;
    CParticleRenderer* m_renderer = nullptr;

    Particle       m_particle[MAXPARTICULE*MAXPARTITYPE];
    std::vector<EngineTriangle> m_triangle;  // triangle if PartiType == 0
    Track          m_track[MAXTRACK];
    int           m_wheelTraceTotal = 0;
    int           m_wheelTraceIndex = 0;
    WheelTrace    m_wheelTrace[MAXWHEELTRACE];
    int           m_totalInterface[MAXPARTITYPE][SH_MAX] = {};
    bool          m_frameUpdate[SH_MAX] = {};
    int           m_fogTotal = 0;
    int           m_fog[MAXPARTIFOG] = {};
    int           m_uniqueStamp = 0;
    int           m_exploGunCounter = 0;
    float         m_lastTimeGunDel = 0.0f;
    float         m_absTime = 0.0f;
};


} // namespace Gfx
