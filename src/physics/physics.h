/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2016, Daniel Roux, EPSITEC SA & TerranovaTeam
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
#include "math/point.h"
#include "math/matrix.h"

#include "object/object_type.h"

#include "object/interface/trace_drawing_object.h"

#include "sound/sound_type.h"


class CObject;
class COldObject;
class CMotion;
class CRobotMain;
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


// TODO: Clean this up
enum PhysicsType
{
    TYPE_NORMAL     = 0, //!< normal mode (using new object interface structure)

    // BuzzingCars specific
    TYPE_RACE       = 1,
    TYPE_TANK       = 2,
    TYPE_MASS       = 3,
};


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
    MO_REAACCEL     = 12,
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
    Math::Vector    realAccel;  // real acceleration(+/-)

    Math::Vector    finalInclin;    // final inclination
};

// TODO: ?
struct Corner
{
    Math::Vector p[4];    // fr,fl,rl,rr
    Math::Point min;
    Math::Point max;
};




class CPhysics
{
public:
    CPhysics(COldObject* object);
    ~CPhysics();

    void        DeleteObject(bool bAll=false);

    bool        EventProcess(const Event &event);

    void        SetMotion(CMotion* motion);

    void        SetType(PhysicsType type);
    PhysicsType GetType();

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
    float        GetWheelSpeed(bool bFront); // BC

    void        SetMotor(bool bState);
    bool        GetMotor();
    void        SetSilent(bool bState); // BC
    bool        GetSilent(); // BC
    void        SetForceSlow(bool bState); // BC
    bool        GetForceSlow(); // BC
    void        SetLand(bool bState);
    bool        GetLand();
    void        SetSwim(bool bState);
    bool        GetSwim();
    void        SetCollision(bool bCollision);
    bool        GetCollision();
    void        SetFreeze(bool bFreeze);
    bool        GetFreeze();
    bool        GetLight(int rank); // BC
    void        SuspForce(int rank, float force, float front, float rTime); // BC

    void        SetMotorSpeed(Math::Vector speed);
    void        SetMotorSpeedX(float speed);
    void        SetMotorSpeedY(float speed);
    void        SetMotorSpeedZ(float speed);
    void        SetMotorSpeedW(float speed); // BC
    Math::Vector    GetMotorSpeed();
    float       GetMotorSpeedX();
    float       GetMotorSpeedY();
    float       GetMotorSpeedZ();
    float       GetMotorSpeedW(); // BC
    void        ForceMotorSpeedX(float speed); // BC
    void        ForceMotorSpeedY(float speed); // BC
    void        ForceMotorSpeedZ(float speed); // BC
    float        GetMotorRPM(); // BC
    int            GetMotorState(); // BC

    int            CrashCornerRect(CObject *pObj, const Math::Vector &pos, const Math::Vector &angle, Math::Point &adjust, Math::Point &inter, float &chocAngle, float &priority, float &hardness, SoundType &sound, float aTime, float rTime); // BC
    bool        CrashValidity(CObject *pObj, Math::Point adjust); // BC
    int            CrashCornerCircle(CObject *pObj, const Math::Vector &pos, const Math::Vector &angle, Math::Point &adjust, Math::Point &inter, float &chocAngle, float &priority, float &hardness, SoundType &sound, float aTime, float rTime); // BC
    int            CrashCornerDo(CObject *pObj, Math::Vector &pos, Math::Point adjust, Math::Point inter, float angle, float priority, float hardness, SoundType sound, float aTime, float rTime); // BC
    void        UpdateCorner(); // BC
    void        UpdateCorner(const Math::Vector &pos, const Math::Vector &angle); // BC
    void        UpdateGlideSound(float rTime); // BC

    Error       GetError();

    void        FFBCrash(float amplitude, float speed, float frequency); // BC
    void        FFBForce(float amplitude); // BC

    float       GetFallingHeight();

    void        SetMinFallingHeight(float value);
    float       GetMinFallingHeight();

    void        SetFallDamageFraction(float value);
    float       GetFallDamageFraction();

protected:
    void        HornEvent(bool bPress); // BC
    bool        EventFrame(const Event &event);
    void        WaterFrame(float aTime, float rTime);
    void        FFBFrame(float aTime, float rTime); // BC
    bool        GetLock(); // BC
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
    int         ObjectAdapt(Math::Vector &pos, Math::Vector &angle, float aTime, float rTime);
    Math::Vector ChocRepulse(CObject *pObj1, CObject *pObj2, Math::Vector p1, Math::Vector p2); // BC
    void        ChocObject(CObject *pObj, Math::Vector repulse); // BC
    float       ChocSpin(Math::Vector obstacle, float force); // BC
    bool        JostleObject(CJostleableObject* pObj, Math::Vector iPos, float iRad);
    bool        JostleObject(CObject* pObj, float force);

    bool        ExploOther(ObjectType iType, CObject *pObj, ObjectType oType, float force, float hardness);
    int         ExploHimself(ObjectType iType, ObjectType oType, float force, float hardness, Math::Vector impact);
    void        DoorCounter(COldObject* pObj, ObjectType oType); // BC
    void        HornAction(); // BC

    void        PowerParticle(float factor, bool bBreak);
    void        CrashParticle(float crash, bool bCrash); // BC
    void        SlideParticle(float aTime, float rTime, float fDusty, float fSound); // BC
    void        WheelParticle(float aTime, float rTime, float force, bool bBrake); // BC ?
    void        SuspParticle(float aTime, float rTime); // BC
    void        GazCompute(int model, Math::Matrix *mat, int i, float dir, float factor, Math::Vector &pos, Math::Vector &speed); // BC
    void        MotorParticle(float aTime, float rTime);
    void        WaterParticle(float aTime, Math::Vector pos, ObjectType type, float floor, float advance, float turn);
    void        DrawingPenParticle(TraceColor color, float width);
    
    void        SetFalling();

protected:
    CRobotMain*         m_main;
    Gfx::CEngine*       m_engine;
    Gfx::CLightManager* m_lightMan;
    Gfx::CParticle*     m_particle;
    Gfx::CTerrain*      m_terrain;
    Gfx::CWater*        m_water;
    Gfx::CCamera*       m_camera;
    CSoundInterface*    m_sound;

    COldObject*         m_object;
    CMotion*            m_motion;

    PhysicsType m_type;        // TYPE_*
    float       m_gravity;      // force of gravity
    float       m_time;         // absolute time
    Math::Vector    m_motorSpeed;       // motor speed (-1..1)
    float           m_motorSpeedW;    // frein ŕ main
    float           m_motorSpeedWk;    // frein ŕ main (selon clavier)
    Math::Vector    m_forceSpeed;    // vitesse du moteur forcée (męme si lock)
    Math::Vector    m_imprecisionA;    // imprécision mécanique actuelle
    Math::Vector    m_imprecisionH;    // imprécision mécanique souhaitée
    Math::Vector    m_imprecisionT;    // imprécision mécanique temps
    Motion      m_linMotion;        // linear motion
    Motion      m_cirMotion;        // circular motion
    float       m_graviSpeed;    // vitesse de chute dű ŕ la gravitation
    float       m_graviGlu;        // colle au sol aprčs un choc (0..1)
    float       m_centriSpeed;    // vitesse de déport dű ŕ la force centripčte
    float       m_wheelSlide;    // patinage ŕ l'accélération
    float       m_overTurn;        // facteur de survirage
    float       m_overTurnCur;    // facteur de survirage
    float       m_overBrake;    // facteur de survirage avec frein ŕ main
    float       m_suspTime[2];    // temps de l'oscillation de la suspension
    float       m_suspHeight[2];// hauteur actuelle des amortisseurs
    float       m_suspEnergy[2][5];// énergies de la suspension
    float       m_suspDelayed[2];// force de la suspension
    float       m_chocSpin;        // vitesse circulaire de choc
    float       m_chocAngle;
    float       m_motorAngle;
    float       m_motorRPM;        // vitesse du moteur (0..1..2)
    int         m_motorState;    // rapport de la boîte de vitesses
    bool        m_bMotor;        // moteur en marche
    bool        m_bSilent;        // moteur stoppé
    bool        m_bForceSlow;    // force moteur au ralenti
    bool        m_bBrake;        // freinage ŕ pied en cours
    bool        m_bHandbrake;    // freinage ŕ main en cours
    bool        m_bLand;
    bool        m_bSwim;
    bool        m_bCollision;
    bool        m_bObstacle;
    bool        m_bFreeze;
    bool        m_bWater;        // sous l'eau ?
    bool        m_bOldWater;
    int         m_repeatCollision;
    int            m_doorRank;
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
    float        m_lastCrashParticle;
    float        m_lastWheelParticle;
    float        m_lastSuspParticle;
    bool        m_bWheelParticleBrake;
    Math::Vector    m_wheelParticlePos[4];
    float       m_absorbWater;
    float       m_reactorTemperature;
    float       m_timeReactorFail;
    float       m_timeUnderWater;
    float        m_timeWheelBurn;
    float        m_timeMotorBurn;
    float        m_timeLock;
    float        m_lastSoundCollision;
    float        m_lastSoundCrash;
    float       m_lastEnergy;
    float       m_lastSoundWater;
    float       m_lastSoundInsect;
    float       m_restBreakParticle;
    float       m_floorLevel;       // ground level
    float       m_floorHeight;      // height above the ground
    float       m_terrainHard;
    int         m_soundChannel;
    int         m_soundChannelMotor1;
    int         m_soundChannelMotor2;
    int         m_soundChannelSlide;
    int         m_soundChannelBrake;
    int         m_soundChannelBoost;
    int         m_soundChannelHorn;
    int         m_soundChannelGlide;
    float       m_soundTimeJostle;
    float       m_soundTimeBoum;
    float       m_soundTimePshhh;
    float       m_soundAmplitudeMotor1;
    float       m_soundAmplitudeMotor2;
    float       m_soundFrequencyMotor1;
    float       m_soundFrequencyMotor2;
    bool        m_bSoundSlow;
    float       m_glideTime;
    float       m_glideVolume;
    float       m_glideAmplitude;
    Math::Vector    m_glideImpact;
    bool        m_bForceUpdate;
    bool        m_bLowLevel;
    bool        m_bHornPress;
    Corner      m_curCorner;
    Corner      m_newCorner;
    CObject*    m_lastDoorCounter;
    float       m_FFBamplitude;
    float       m_FFBspeed;
    float       m_FFBfrequency;
    float       m_FFBprogress;
    float       m_FFBforce;
    float       m_FFBbrake;
    float       m_fallingHeight;
    float       m_fallDamageFraction;
    float       m_minFallingHeight;
};
