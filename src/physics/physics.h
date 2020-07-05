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
 * \file physics/physics.h
 * \brief Responsible for physics "and more" in game
 */

#pragma once


#include "common/error.h"

#include "math/vector.h"

#include "object/object_type.h"

#include "object/interface/trace_drawing_object.h"


class CObject;
class COldObject;
class CMotion;
class CSoundInterface;
class CLevelParserLine;
class CJostleableObject;
struct Event;

namespace Gfx
{
class CCamera;
class CEngine;
class CParticle;
class CTerrain;
class CWater;
class CLightManager;
} // namespace Gfx


enum PhysicsMode
{
    MO_ADVACCEL     = 0,
    MO_RECACCEL     = 1,
    MO_STOACCEL     = 2,
    MO_MOTACCEL     = 3,
    MO_ADVSPEED     = 4,
    MO_RECSPEED     = 5,
    MO_MOTSPEED     = 6,
    MO_CURSPEED     = 7,
    MO_TERFORCE     = 8,
    MO_TERSPEED     = 9,
    MO_TERSLIDE     = 10,
    MO_REASPEED     = 11,
};


struct Motion
{
    Math::Vector    advanceAccel;   // acceleration starting (+)
    Math::Vector    recedeAccel;    // acceleration starting (+)
    Math::Vector    stopAccel;  // acceleration stoping (+)
    Math::Vector    motorAccel; // current acceleration (+/-)

    Math::Vector    advanceSpeed;   // forward speed (+)
    Math::Vector    recedeSpeed;    // reversing speed (+)
    Math::Vector    motorSpeed; // desired speed (+/-)
    Math::Vector    currentSpeed;   // current speed (+/-)

    Math::Vector    terrainForce;   // power of resistance of the ground (+)
    Math::Vector    terrainSpeed;   // speed of the ground (+/-)
    Math::Vector    terrainSlide;   // limit sliding speed (+)

    Math::Vector    realSpeed;  // real speed(+/-)

    Math::Vector    finalInclin;    // final inclination
};




class CPhysics
{
public:
    CPhysics(COldObject* object);
    ~CPhysics();

    void        DeleteObject(bool bAll=false);

    bool        EventProcess(const Event &event);

    void        SetMotion(CMotion* motion);

    bool        Write(CLevelParserLine* line);
    bool        Read(CLevelParserLine* line);

    void        SetGravity(float value);
    float       GetGravity();

    float       GetFloorHeight();

    void        SetLinMotion(PhysicsMode mode, Math::Vector value);
    Math::Vector    GetLinMotion(PhysicsMode mode);
    void        SetLinMotionX(PhysicsMode mode, float value);
    void        SetLinMotionY(PhysicsMode mode, float value);
    void        SetLinMotionZ(PhysicsMode mode, float value);
    float       GetLinMotionX(PhysicsMode mode);
    float       GetLinMotionY(PhysicsMode mode);
    float       GetLinMotionZ(PhysicsMode mode);

    void        SetCirMotion(PhysicsMode mode, Math::Vector value);
    Math::Vector    GetCirMotion(PhysicsMode mode);
    void        SetCirMotionX(PhysicsMode mode, float value);
    void        SetCirMotionY(PhysicsMode mode, float value);
    void        SetCirMotionZ(PhysicsMode mode, float value);
    float       GetCirMotionX(PhysicsMode mode);
    float       GetCirMotionY(PhysicsMode mode);
    float       GetCirMotionZ(PhysicsMode mode);

    float       GetLinStopLength(PhysicsMode sMode=MO_ADVSPEED, PhysicsMode aMode=MO_STOACCEL);
    float       GetCirStopLength();
    float       GetLinMaxLength(float dir);
    float       GetLinTimeLength(float dist, float dir=1.0f);
    float       GetLinLength(float dist);

    void        SetMotor(bool bState);
    bool        GetMotor();
    void        SetLand(bool bState);
    bool        GetLand();
    void        SetSwim(bool bState);
    bool        GetSwim();
    void        SetCollision(bool bCollision);
    bool        GetCollision();
    void        SetFreeze(bool bFreeze);
    bool        GetFreeze();

    void        SetMotorSpeed(Math::Vector speed);
    void        SetMotorSpeedX(float speed);
    void        SetMotorSpeedY(float speed);
    void        SetMotorSpeedZ(float speed);
    Math::Vector    GetMotorSpeed();
    float       GetMotorSpeedX();
    float       GetMotorSpeedY();
    float       GetMotorSpeedZ();

    Error       GetError();

    float       GetFallingHeight();

    void        SetMinFallingHeight(float value);
    float       GetMinFallingHeight();

    void        SetFallDamageFraction(float value);
    float       GetFallDamageFraction();

protected:
    bool        EventFrame(const Event &event);
    void        WaterFrame(float aTime, float rTime);
    void        SoundMotor(float rTime);
    void        SoundMotorFull(float rTime, ObjectType type);
    void        SoundMotorSlow(float rTime, ObjectType type);
    void        SoundMotorStop(float rTime, ObjectType type);
    void        SoundReactorFull(float rTime, ObjectType type);
    void        SoundReactorStop(float rTime, ObjectType type);
    void        FrameParticle(float aTime, float rTime);
    void        MotorUpdate(float aTime, float rTime);
    void        EffectUpdate(float aTime, float rTime);
    void        UpdateMotionStruct(float rTime, Motion &motion);
    void        FloorAdapt(float aTime, float rTime, Math::Vector &pos, Math::Vector &angle);
    void        FloorAngle(const Math::Vector &pos, Math::Vector &angle);
    int         ObjectAdapt(const Math::Vector &pos, const Math::Vector &angle);
    bool        JostleObject(CJostleableObject* pObj, Math::Vector iPos, float iRad);
    bool        JostleObject(CObject* pObj, float force);
    bool        ExploOther(ObjectType iType, CObject *pObj, ObjectType oType, float force);
    int         ExploHimself(ObjectType iType, ObjectType oType, float force);

    void        PowerParticle(float factor, bool bBreak);
    void        CrashParticle(float crash);
    void        MotorParticle(float aTime, float rTime);
    void        WaterParticle(float aTime, Math::Vector pos, ObjectType type, float floor, float advance, float turn);
    void        WheelParticle(TraceColor color, float width);
    void        SetFalling();

protected:
    Gfx::CEngine*       m_engine;
    Gfx::CLightManager* m_lightMan;
    Gfx::CParticle*     m_particle;
    Gfx::CTerrain*      m_terrain;
    Gfx::CWater*        m_water;
    Gfx::CCamera*       m_camera;
    CSoundInterface*    m_sound;

    COldObject*         m_object;
    CMotion*            m_motion;

    float       m_gravity;      // force of gravity
    float       m_time;         // absolute time
    Math::Vector    m_motorSpeed;       // motor speed (-1..1)
    Motion      m_linMotion;        // linear motion
    Motion      m_cirMotion;        // circular motion
    bool        m_bMotor;
    bool        m_bLand;
    bool        m_bSwim;
    bool        m_bCollision;
    bool        m_bObstacle;
    bool        m_bFreeze;
    int         m_repeatCollision;
    float       m_linVibrationFactor;
    float       m_cirVibrationFactor;
    float       m_inclinaisonFactor;
    float       m_lastPowerParticle;
    float       m_lastSlideParticle;
    float       m_lastMotorParticle;
    float       m_lastWaterParticle;
    float       m_lastUnderParticle;
    float       m_lastPloufParticle;
    float       m_lastFlameParticle;
    bool        m_bWheelParticleBrake;
    Math::Vector    m_wheelParticlePos[2];
    float       m_absorbWater;
    float       m_reactorTemperature;
    float       m_timeReactorFail;
    float       m_timeUnderWater;
    float       m_lastEnergy;
    float       m_lastSoundWater;
    float       m_lastSoundInsect;
    float       m_restBreakParticle;
    float       m_floorLevel;       // ground level
    float       m_floorHeight;      // height above the ground
    int         m_soundChannel;
    int         m_soundChannelSlide;
    float       m_soundTimePshhh;
    float       m_soundTimeJostle;
    float       m_soundTimeBoum;
    bool        m_bSoundSlow;
    bool        m_bForceUpdate;
    bool        m_bLowLevel;
    float       m_fallingHeight;
    float       m_fallDamageFraction;
    float       m_minFallingHeight;
};
