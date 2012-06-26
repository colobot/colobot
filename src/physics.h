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

// physics.h

#ifndef _PHYSICS_H_
#define _PHYSICS_H_


#include "d3dengine.h"
#include "misc.h"


class CInstanceManager;
class CD3DEngine;
class CLight;
class CParticule;
class CTerrain;
class CWater;
class CCamera;
class CObject;
class CBrain;
class CMotion;
class CSound;


enum PhysicsType
{
    TYPE_ROLLING    = 1,
    TYPE_FLYING     = 2,
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
};


typedef struct
{
    D3DVECTOR   advanceAccel;   // acceleration starting (+)
    D3DVECTOR   recedeAccel;    // acceleration starting (+)
    D3DVECTOR   stopAccel;  // acceleration stoping (+)
    D3DVECTOR   motorAccel; // current acceleration (+/-)

    D3DVECTOR   advanceSpeed;   // forward speed (+)
    D3DVECTOR   recedeSpeed;    // reversing speed (+)
    D3DVECTOR   motorSpeed; // desired speed (+/-)
    D3DVECTOR   currentSpeed;   // current speed (+/-)

    D3DVECTOR   terrainForce;   // power of resistance of the ground (+)
    D3DVECTOR   terrainSpeed;   // speed of the ground (+/-)
    D3DVECTOR   terrainSlide;   // limit sliding speed (+)

    D3DVECTOR   realSpeed;  // real speed(+/-)

    D3DVECTOR   finalInclin;    // final inclination
}
Motion;




class CPhysics
{
public:
    CPhysics(CInstanceManager* iMan, CObject* object);
    ~CPhysics();

    void        DeleteObject(BOOL bAll=FALSE);

    BOOL        EventProcess(const Event &event);

    void        SetBrain(CBrain* brain);
    void        SetMotion(CMotion* motion);

    void        SetType(PhysicsType type);
    PhysicsType RetType();

    BOOL        Write(char *line);
    BOOL        Read(char *line);

    void        SetGravity(float value);
    float       RetGravity();

    float       RetFloorHeight();

    void        SetLinMotion(PhysicsMode mode, D3DVECTOR value);
    D3DVECTOR   RetLinMotion(PhysicsMode mode);
    void        SetLinMotionX(PhysicsMode mode, float value);
    void        SetLinMotionY(PhysicsMode mode, float value);
    void        SetLinMotionZ(PhysicsMode mode, float value);
    float       RetLinMotionX(PhysicsMode mode);
    float       RetLinMotionY(PhysicsMode mode);
    float       RetLinMotionZ(PhysicsMode mode);

    void        SetCirMotion(PhysicsMode mode, D3DVECTOR value);
    D3DVECTOR   RetCirMotion(PhysicsMode mode);
    void        SetCirMotionX(PhysicsMode mode, float value);
    void        SetCirMotionY(PhysicsMode mode, float value);
    void        SetCirMotionZ(PhysicsMode mode, float value);
    float       RetCirMotionX(PhysicsMode mode);
    float       RetCirMotionY(PhysicsMode mode);
    float       RetCirMotionZ(PhysicsMode mode);

    float       RetLinStopLength(PhysicsMode sMode=MO_ADVSPEED, PhysicsMode aMode=MO_STOACCEL);
    float       RetCirStopLength();
    float       RetLinMaxLength(float dir);
    float       RetLinTimeLength(float dist, float dir=1.0f);
    float       RetLinLength(float dist);

    void        SetMotor(BOOL bState);
    BOOL        RetMotor();
    void        SetLand(BOOL bState);
    BOOL        RetLand();
    void        SetSwim(BOOL bState);
    BOOL        RetSwim();
    void        SetCollision(BOOL bCollision);
    BOOL        RetCollision();
    void        SetFreeze(BOOL bFreeze);
    BOOL        RetFreeze();
    void        SetReactorRange(float range);
    float       RetReactorRange();

    void        SetMotorSpeed(D3DVECTOR speed);
    void        SetMotorSpeedX(float speed);
    void        SetMotorSpeedY(float speed);
    void        SetMotorSpeedZ(float speed);
    D3DVECTOR   RetMotorSpeed();
    float       RetMotorSpeedX();
    float       RetMotorSpeedY();
    float       RetMotorSpeedZ();

    void        CreateInterface(BOOL bSelect);
    Error       RetError();

protected:
    BOOL        EventFrame(const Event &event);
    void        WaterFrame(float aTime, float rTime);
    void        SoundMotor(float rTime);
    void        SoundMotorFull(float rTime, ObjectType type);
    void        SoundMotorSlow(float rTime, ObjectType type);
    void        SoundMotorStop(float rTime, ObjectType type);
    void        SoundReactorFull(float rTime, ObjectType type);
    void        SoundReactorStop(float rTime, ObjectType type);
    void        FrameParticule(float aTime, float rTime);
    void        MotorUpdate(float aTime, float rTime);
    void        EffectUpdate(float aTime, float rTime);
    void        UpdateMotionStruct(float rTime, Motion &motion);
    void        FloorAdapt(float aTime, float rTime, D3DVECTOR &pos, D3DVECTOR &angle);
    void        FloorAngle(const D3DVECTOR &pos, D3DVECTOR &angle);
    int         ObjectAdapt(const D3DVECTOR &pos, const D3DVECTOR &angle);
    BOOL        JostleObject(CObject* pObj, D3DVECTOR iPos, float iRad, D3DVECTOR oPos, float oRad);
    BOOL        JostleObject(CObject* pObj, float force);
    BOOL        ExploOther(ObjectType iType, CObject *pObj, ObjectType oType, float force);
    int         ExploHimself(ObjectType iType, ObjectType oType, float force);

    void        PowerParticule(float factor, BOOL bBreak);
    void        CrashParticule(float crash);
    void        MotorParticule(float aTime, float rTime);
    void        WaterParticule(float aTime, D3DVECTOR pos, ObjectType type, float floor, float advance, float turn);
    void        WheelParticule(int color, float width);

protected:
    CInstanceManager* m_iMan;
    CD3DEngine* m_engine;
    CLight*     m_light;
    CParticule* m_particule;
    CTerrain*   m_terrain;
    CWater*     m_water;
    CCamera*    m_camera;
    CObject*    m_object;
    CBrain*     m_brain;
    CMotion*    m_motion;
    CSound*     m_sound;

    PhysicsType m_type;         // TYPE_*
    float       m_gravity;      // force of gravity
    float       m_time;         // absolute time
    D3DVECTOR   m_motorSpeed;       // motor speed (-1..1)
    Motion      m_linMotion;        // linear motion
    Motion      m_cirMotion;        // circular motion
    BOOL        m_bMotor;
    BOOL        m_bLand;
    BOOL        m_bSwim;
    BOOL        m_bCollision;
    BOOL        m_bObstacle;
    BOOL        m_bFreeze;
    int         m_repeatCollision;
    float       m_linVibrationFactor;
    float       m_cirVibrationFactor;
    float       m_inclinaisonFactor;
    float       m_lastPowerParticule;
    float       m_lastSlideParticule;
    float       m_lastMotorParticule;
    float       m_lastWaterParticule;
    float       m_lastUnderParticule;
    float       m_lastPloufParticule;
    float       m_lastFlameParticule;
    BOOL        m_bWheelParticuleBrake;
    D3DVECTOR   m_wheelParticulePos[2];
    float       m_absorbWater;
    float       m_reactorTemperature;
    float       m_reactorRange;
    float       m_timeReactorFail;
    float       m_timeUnderWater;
    float       m_lastEnergy;
    float       m_lastSoundWater;
    float       m_lastSoundInsect;
    float       m_restBreakParticule;
    float       m_floorLevel;       // ground level
    float       m_floorHeight;      // height above the ground
    int     m_soundChannel;
    int     m_soundChannelSlide;
    float       m_soundTimePshhh;
    float       m_soundTimeJostle;
    float       m_soundTimeBoum;
    BOOL        m_bSoundSlow;
    BOOL        m_bForceUpdate;
    BOOL        m_bLowLevel;
};


#endif //_PHYSICS_H_
