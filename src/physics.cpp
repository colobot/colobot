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

// physics.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "d3dengine.h"
#include "d3dmath.h"
#include "language.h"
#include "global.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "math3d.h"
#include "light.h"
#include "particule.h"
#include "terrain.h"
#include "water.h"
#include "camera.h"
#include "object.h"
#include "pyro.h"
#include "brain.h"
#include "motion.h"
#include "motionhuman.h"
#include "sound.h"
#include "task.h"
#include "cmdtoken.h"
#include "physics.h"



#define LANDING_SPEED   3.0f
#define LANDING_ACCEL   5.0f
#define LANDING_ACCELh  1.5f




// Object's constructor.

CPhysics::CPhysics(CInstanceManager* iMan, CObject* object)
{
    m_iMan = iMan;
    m_iMan->AddInstance(CLASS_PHYSICS, this, 100);

    m_object    = object;
    m_engine    = (CD3DEngine*)m_iMan->SearchInstance(CLASS_ENGINE);
    m_light     = (CLight*)m_iMan->SearchInstance(CLASS_LIGHT);
    m_particule = (CParticule*)m_iMan->SearchInstance(CLASS_PARTICULE);
    m_terrain   = (CTerrain*)m_iMan->SearchInstance(CLASS_TERRAIN);
    m_water     = (CWater*)m_iMan->SearchInstance(CLASS_WATER);
    m_camera    = (CCamera*)m_iMan->SearchInstance(CLASS_CAMERA);
    m_sound     = (CSound*)m_iMan->SearchInstance(CLASS_SOUND);
    m_brain     = 0;
    m_motion    = 0;

    m_type = TYPE_ROLLING;
    m_gravity = 9.81f;  // default gravity
    m_time = 0.0f;
    m_timeUnderWater = 0.0f;
    m_motorSpeed = D3DVECTOR(0.0f, 0.0f, 0.0f);
    m_bMotor = FALSE;
    m_bLand = TRUE;  // ground
    m_bSwim = FALSE;  // in air
    m_bCollision = FALSE;
    m_bObstacle = FALSE;
    m_repeatCollision = 0;
    m_linVibrationFactor = 1.0f;
    m_cirVibrationFactor = 1.0f;
    m_inclinaisonFactor  = 1.0f;
    m_lastPowerParticule = 0.0f;
    m_lastSlideParticule = 0.0f;
    m_lastMotorParticule = 0.0f;
    m_lastWaterParticule = 0.0f;
    m_lastUnderParticule = 0.0f;
    m_lastPloufParticule = 0.0f;
    m_lastFlameParticule = 0.0f;
    m_bWheelParticuleBrake = FALSE;
    m_absorbWater        = 0.0f;
    m_reactorTemperature = 0.0f;
    m_reactorRange       = 1.0f;
    m_timeReactorFail    = 0.0f;
    m_lastEnergy = 0.0f;
    m_lastSoundWater = 0.0f;
    m_lastSoundInsect = 0.0f;
    m_restBreakParticule = 0.0f;
    m_floorHeight = 0.0f;
    m_soundChannel = -1;
    m_soundChannelSlide = -1;
    m_soundTimePshhh = 0.0f;
    m_soundTimeJostle = 0.0f;
    m_soundTimeBoum = 0.0f;
    m_bSoundSlow = TRUE;
    m_bFreeze = FALSE;
    m_bForceUpdate = TRUE;
    m_bLowLevel = FALSE;

    ZeroMemory(&m_linMotion, sizeof(Motion));
    ZeroMemory(&m_cirMotion, sizeof(Motion));
}

// Object's destructor.

CPhysics::~CPhysics()
{
    m_iMan->DeleteInstance(CLASS_PHYSICS, this);
}


// Destroys the object.

void CPhysics::DeleteObject(BOOL bAll)
{
    if ( m_soundChannel != -1 )
    {
        m_sound->FlushEnvelope(m_soundChannel);
        m_sound->AddEnvelope(m_soundChannel, 0.0f, 1.0f, 0.3f, SOPER_STOP);
        m_soundChannel = -1;
    }
    if ( m_soundChannelSlide != -1 )
    {
        m_sound->FlushEnvelope(m_soundChannelSlide);
        m_sound->AddEnvelope(m_soundChannelSlide, 0.0f, 1.0f, 0.3f, SOPER_STOP);
        m_soundChannelSlide = -1;
    }
}



void CPhysics::SetBrain(CBrain* brain)
{
    m_brain = brain;
}

void CPhysics::SetMotion(CMotion* motion)
{
    m_motion = motion;
}

// Management of the type.

void CPhysics::SetType(PhysicsType type)
{
    m_type = type;
}

PhysicsType CPhysics::RetType()
{
    return m_type;
}



// Saves all parameters of the object.

BOOL CPhysics::Write(char *line)
{
    char        name[100];

    sprintf(name, " motor=%.2f;%.2f;%.2f", m_motorSpeed.x, m_motorSpeed.y, m_motorSpeed.z);
    strcat(line, name);

    if ( m_type == TYPE_FLYING )
    {
        sprintf(name, " reactorRange=%.2f", RetReactorRange());
        strcat(line, name);

        sprintf(name, " land=%d", RetLand());
        strcat(line, name);
    }

    return TRUE;
}

// Restores all parameters of the object.

BOOL CPhysics::Read(char *line)
{
    m_motorSpeed = OpDir(line, "motor");

    if ( m_type == TYPE_FLYING )
    {
        SetReactorRange(OpFloat(line, "reactorRange", 0.0f));
        SetLand(OpInt(line, "land", 0));
    }

    return TRUE;
}



// Management of the force of gravity.

void CPhysics::SetGravity(float value)
{
    m_gravity = value;
}

float CPhysics::RetGravity()
{
    return m_gravity;
}


// Returns the height above the ground.

float CPhysics::RetFloorHeight()
{
    return m_floorHeight;
}


// Managing the state of the engine.

void CPhysics::SetMotor(BOOL bState)
{
    int         light;

    m_bMotor = bState;

    light = m_object->RetShadowLight();
    if ( light != -1 )
    {
        m_light->SetLightIntensity(light, m_bMotor?1.0f:0.0f);
        m_light->SetLightIntensitySpeed(light, 3.0f);
    }
}

BOOL CPhysics::RetMotor()
{
    return m_bMotor;
}


// Management of the state in flight/ground.

void CPhysics::SetLand(BOOL bState)
{
    m_bLand = bState;
    SetMotor(!bState);  // lights if you leave the reactor in flight
}

BOOL CPhysics::RetLand()
{
    return m_bLand;
}


// Management of the state in air/water.

void CPhysics::SetSwim(BOOL bState)
{
    if ( !m_bSwim && bState )  // enters the water?
    {
        m_timeUnderWater = 0.0f;
    }
    m_bSwim = bState;
}

BOOL CPhysics::RetSwim()
{
    return m_bSwim;
}


// Indicates whether a collision occurred.

void CPhysics::SetCollision(BOOL bCollision)
{
    m_bCollision = bCollision;
}

BOOL CPhysics::RetCollision()
{
    return m_bCollision;
}


// Indicates whether the influence of soil is activated or not.

void CPhysics::SetFreeze(BOOL bFreeze)
{
    m_bFreeze = bFreeze;
}

BOOL CPhysics::RetFreeze()
{
    return m_bFreeze;
}


// Returns the range of the reactor.

void CPhysics::SetReactorRange(float range)
{
    m_reactorRange = range;
}

float CPhysics::RetReactorRange()
{
    return m_reactorRange;
}


// Specifies the engine speed.
// x = forward/backward
// y = up/down
// z = turn

void CPhysics::SetMotorSpeed(D3DVECTOR speed)
{
    m_motorSpeed = speed;
}

// Specifies the engine speed for forward/backward.
// +1 = forward
// -1 = backward

void CPhysics::SetMotorSpeedX(float speed)
{
    m_motorSpeed.x = speed;
}

// Specifies the motor speed for up/down.
// +1 = up
// -1 = down

void CPhysics::SetMotorSpeedY(float speed)
{
    m_motorSpeed.y = speed;
}

// Specifies the speed of the motor to turn.
// +1 = turn right(CW)
// -1 = turn left(CCW)

void CPhysics::SetMotorSpeedZ(float speed)
{
    m_motorSpeed.z = speed;
}

D3DVECTOR CPhysics::RetMotorSpeed()
{
    return m_motorSpeed;
}

float CPhysics::RetMotorSpeedX()
{
    return m_motorSpeed.x;
}

float CPhysics::RetMotorSpeedY()
{
    return m_motorSpeed.y;
}

float CPhysics::RetMotorSpeedZ()
{
    return m_motorSpeed.z;
}


// Management of linear and angular velocities.
// Specifies the speed parallel to the direction of travel.

void CPhysics::SetLinMotion(PhysicsMode mode, D3DVECTOR value)
{
    if ( mode == MO_ADVACCEL )  m_linMotion.advanceAccel  = value;
    if ( mode == MO_RECACCEL )  m_linMotion.recedeAccel   = value;
    if ( mode == MO_STOACCEL )  m_linMotion.stopAccel     = value;
    if ( mode == MO_TERSPEED )  m_linMotion.terrainSpeed  = value;
    if ( mode == MO_TERSLIDE )  m_linMotion.terrainSlide  = value;
    if ( mode == MO_MOTACCEL )  m_linMotion.motorAccel    = value;
    if ( mode == MO_TERFORCE )  m_linMotion.terrainForce  = value;
    if ( mode == MO_ADVSPEED )  m_linMotion.advanceSpeed  = value;
    if ( mode == MO_RECSPEED )  m_linMotion.recedeSpeed   = value;
    if ( mode == MO_MOTSPEED )  m_linMotion.motorSpeed    = value;
    if ( mode == MO_CURSPEED )  m_linMotion.currentSpeed  = value;
    if ( mode == MO_REASPEED )  m_linMotion.realSpeed     = value;
}

D3DVECTOR CPhysics::RetLinMotion(PhysicsMode mode)
{
    if ( mode == MO_ADVACCEL )  return m_linMotion.advanceAccel;
    if ( mode == MO_RECACCEL )  return m_linMotion.recedeAccel;
    if ( mode == MO_STOACCEL )  return m_linMotion.stopAccel;
    if ( mode == MO_TERSPEED )  return m_linMotion.terrainSpeed;
    if ( mode == MO_TERSLIDE )  return m_linMotion.terrainSlide;
    if ( mode == MO_MOTACCEL )  return m_linMotion.motorAccel;
    if ( mode == MO_TERFORCE )  return m_linMotion.terrainForce;
    if ( mode == MO_ADVSPEED )  return m_linMotion.advanceSpeed;
    if ( mode == MO_RECSPEED )  return m_linMotion.recedeSpeed;
    if ( mode == MO_MOTSPEED )  return m_linMotion.motorSpeed;
    if ( mode == MO_CURSPEED )  return m_linMotion.currentSpeed;
    if ( mode == MO_REASPEED )  return m_linMotion.realSpeed;
    return D3DVECTOR(0.0f, 0.0f, 0.0f);
}

void CPhysics::SetLinMotionX(PhysicsMode mode, float value)
{
    if ( mode == MO_ADVACCEL )  m_linMotion.advanceAccel.x  = value;
    if ( mode == MO_RECACCEL )  m_linMotion.recedeAccel.x   = value;
    if ( mode == MO_STOACCEL )  m_linMotion.stopAccel.x     = value;
    if ( mode == MO_TERSPEED )  m_linMotion.terrainSpeed.x  = value;
    if ( mode == MO_TERSLIDE )  m_linMotion.terrainSlide.x  = value;
    if ( mode == MO_MOTACCEL )  m_linMotion.motorAccel.x    = value;
    if ( mode == MO_TERFORCE )  m_linMotion.terrainForce.x  = value;
    if ( mode == MO_ADVSPEED )  m_linMotion.advanceSpeed.x  = value;
    if ( mode == MO_RECSPEED )  m_linMotion.recedeSpeed.x   = value;
    if ( mode == MO_MOTSPEED )  m_linMotion.motorSpeed.x    = value;
    if ( mode == MO_CURSPEED )  m_linMotion.currentSpeed.x  = value;
    if ( mode == MO_REASPEED )  m_linMotion.realSpeed.x     = value;
}

float CPhysics::RetLinMotionX(PhysicsMode mode)
{
    if ( mode == MO_ADVACCEL )  return m_linMotion.advanceAccel.x;
    if ( mode == MO_RECACCEL )  return m_linMotion.recedeAccel.x;
    if ( mode == MO_STOACCEL )  return m_linMotion.stopAccel.x;
    if ( mode == MO_TERSPEED )  return m_linMotion.terrainSpeed.x;
    if ( mode == MO_TERSLIDE )  return m_linMotion.terrainSlide.x;
    if ( mode == MO_MOTACCEL )  return m_linMotion.motorAccel.x;
    if ( mode == MO_TERFORCE )  return m_linMotion.terrainForce.x;
    if ( mode == MO_ADVSPEED )  return m_linMotion.advanceSpeed.x;
    if ( mode == MO_RECSPEED )  return m_linMotion.recedeSpeed.x;
    if ( mode == MO_MOTSPEED )  return m_linMotion.motorSpeed.x;
    if ( mode == MO_CURSPEED )  return m_linMotion.currentSpeed.x;
    if ( mode == MO_REASPEED )  return m_linMotion.realSpeed.x;
    return 0.0f;
}

// Specifies the speed of elevation.

void CPhysics::SetLinMotionY(PhysicsMode mode, float value)
{
    if ( mode == MO_ADVACCEL )  m_linMotion.advanceAccel.y  = value;
    if ( mode == MO_RECACCEL )  m_linMotion.recedeAccel.y   = value;
    if ( mode == MO_STOACCEL )  m_linMotion.stopAccel.y     = value;
    if ( mode == MO_TERSPEED )  m_linMotion.terrainSpeed.y  = value;
    if ( mode == MO_TERSLIDE )  m_linMotion.terrainSlide.y  = value;
    if ( mode == MO_MOTACCEL )  m_linMotion.motorAccel.y    = value;
    if ( mode == MO_TERFORCE )  m_linMotion.terrainForce.y  = value;
    if ( mode == MO_ADVSPEED )  m_linMotion.advanceSpeed.y  = value;
    if ( mode == MO_RECSPEED )  m_linMotion.recedeSpeed.y   = value;
    if ( mode == MO_MOTSPEED )  m_linMotion.motorSpeed.y    = value;
    if ( mode == MO_CURSPEED )  m_linMotion.currentSpeed.y  = value;
    if ( mode == MO_REASPEED )  m_linMotion.realSpeed.y     = value;
}

float CPhysics::RetLinMotionY(PhysicsMode mode)
{
    if ( mode == MO_ADVACCEL )  return m_linMotion.advanceAccel.y;
    if ( mode == MO_RECACCEL )  return m_linMotion.recedeAccel.y;
    if ( mode == MO_STOACCEL )  return m_linMotion.stopAccel.y;
    if ( mode == MO_TERSPEED )  return m_linMotion.terrainSpeed.y;
    if ( mode == MO_TERSLIDE )  return m_linMotion.terrainSlide.y;
    if ( mode == MO_MOTACCEL )  return m_linMotion.motorAccel.y;
    if ( mode == MO_TERFORCE )  return m_linMotion.terrainForce.y;
    if ( mode == MO_ADVSPEED )  return m_linMotion.advanceSpeed.y;
    if ( mode == MO_RECSPEED )  return m_linMotion.recedeSpeed.y;
    if ( mode == MO_MOTSPEED )  return m_linMotion.motorSpeed.y;
    if ( mode == MO_CURSPEED )  return m_linMotion.currentSpeed.y;
    if ( mode == MO_REASPEED )  return m_linMotion.realSpeed.y;
    return 0.0f;
}

// Specifies the velocity perpendicular to the direction of travel.

void CPhysics::SetLinMotionZ(PhysicsMode mode, float value)
{
    if ( mode == MO_ADVACCEL )  m_linMotion.advanceAccel.z  = value;
    if ( mode == MO_RECACCEL )  m_linMotion.recedeAccel.z   = value;
    if ( mode == MO_STOACCEL )  m_linMotion.stopAccel.z     = value;
    if ( mode == MO_TERSPEED )  m_linMotion.terrainSpeed.z  = value;
    if ( mode == MO_TERSLIDE )  m_linMotion.terrainSlide.z  = value;
    if ( mode == MO_MOTACCEL )  m_linMotion.motorAccel.z    = value;
    if ( mode == MO_TERFORCE )  m_linMotion.terrainForce.z  = value;
    if ( mode == MO_ADVSPEED )  m_linMotion.advanceSpeed.z  = value;
    if ( mode == MO_RECSPEED )  m_linMotion.recedeSpeed.z   = value;
    if ( mode == MO_MOTSPEED )  m_linMotion.motorSpeed.z    = value;
    if ( mode == MO_CURSPEED )  m_linMotion.currentSpeed.z  = value;
    if ( mode == MO_REASPEED )  m_linMotion.realSpeed.z     = value;
}

float CPhysics::RetLinMotionZ(PhysicsMode mode)
{
    if ( mode == MO_ADVACCEL )  return m_linMotion.advanceAccel.z;
    if ( mode == MO_RECACCEL )  return m_linMotion.recedeAccel.z;
    if ( mode == MO_STOACCEL )  return m_linMotion.stopAccel.z;
    if ( mode == MO_TERSPEED )  return m_linMotion.terrainSpeed.z;
    if ( mode == MO_TERSLIDE )  return m_linMotion.terrainSlide.z;
    if ( mode == MO_MOTACCEL )  return m_linMotion.motorAccel.z;
    if ( mode == MO_TERFORCE )  return m_linMotion.terrainForce.z;
    if ( mode == MO_ADVSPEED )  return m_linMotion.advanceSpeed.z;
    if ( mode == MO_RECSPEED )  return m_linMotion.recedeSpeed.z;
    if ( mode == MO_MOTSPEED )  return m_linMotion.motorSpeed.z;
    if ( mode == MO_CURSPEED )  return m_linMotion.currentSpeed.z;
    if ( mode == MO_REASPEED )  return m_linMotion.realSpeed.z;
    return 0.0f;
}

// Specifies the rotation around the axis of walk.

void CPhysics::SetCirMotion(PhysicsMode mode, D3DVECTOR value)
{
    if ( mode == MO_ADVACCEL )  m_cirMotion.advanceAccel  = value;
    if ( mode == MO_RECACCEL )  m_cirMotion.recedeAccel   = value;
    if ( mode == MO_STOACCEL )  m_cirMotion.stopAccel     = value;
    if ( mode == MO_TERSPEED )  m_cirMotion.terrainSpeed  = value;
    if ( mode == MO_TERSLIDE )  m_cirMotion.terrainSlide  = value;
    if ( mode == MO_MOTACCEL )  m_cirMotion.motorAccel    = value;
    if ( mode == MO_TERFORCE )  m_cirMotion.terrainForce  = value;
    if ( mode == MO_ADVSPEED )  m_cirMotion.advanceSpeed  = value;
    if ( mode == MO_RECSPEED )  m_cirMotion.recedeSpeed   = value;
    if ( mode == MO_MOTSPEED )  m_cirMotion.motorSpeed    = value;
    if ( mode == MO_CURSPEED )  m_cirMotion.currentSpeed  = value;
    if ( mode == MO_REASPEED )  m_cirMotion.realSpeed     = value;
}

D3DVECTOR CPhysics::RetCirMotion(PhysicsMode mode)
{
    if ( mode == MO_ADVACCEL )  return m_cirMotion.advanceAccel;
    if ( mode == MO_RECACCEL )  return m_cirMotion.recedeAccel;
    if ( mode == MO_STOACCEL )  return m_cirMotion.stopAccel;
    if ( mode == MO_TERSPEED )  return m_cirMotion.terrainSpeed;
    if ( mode == MO_TERSLIDE )  return m_cirMotion.terrainSlide;
    if ( mode == MO_MOTACCEL )  return m_cirMotion.motorAccel;
    if ( mode == MO_TERFORCE )  return m_cirMotion.terrainForce;
    if ( mode == MO_ADVSPEED )  return m_cirMotion.advanceSpeed;
    if ( mode == MO_RECSPEED )  return m_cirMotion.recedeSpeed;
    if ( mode == MO_MOTSPEED )  return m_cirMotion.motorSpeed;
    if ( mode == MO_CURSPEED )  return m_cirMotion.currentSpeed;
    if ( mode == MO_REASPEED )  return m_cirMotion.realSpeed;
    return D3DVECTOR(0.0f, 0.0f, 0.0f);
}

void CPhysics::SetCirMotionX(PhysicsMode mode, float value)
{
    if ( mode == MO_ADVACCEL )  m_cirMotion.advanceAccel.x  = value;
    if ( mode == MO_RECACCEL )  m_cirMotion.recedeAccel.x   = value;
    if ( mode == MO_STOACCEL )  m_cirMotion.stopAccel.x     = value;
    if ( mode == MO_TERSPEED )  m_cirMotion.terrainSpeed.x  = value;
    if ( mode == MO_TERSLIDE )  m_cirMotion.terrainSlide.x  = value;
    if ( mode == MO_MOTACCEL )  m_cirMotion.motorAccel.x    = value;
    if ( mode == MO_TERFORCE )  m_cirMotion.terrainForce.x  = value;
    if ( mode == MO_ADVSPEED )  m_cirMotion.advanceSpeed.x  = value;
    if ( mode == MO_RECSPEED )  m_cirMotion.recedeSpeed.x   = value;
    if ( mode == MO_MOTSPEED )  m_cirMotion.motorSpeed.x    = value;
    if ( mode == MO_CURSPEED )  m_cirMotion.currentSpeed.x  = value;
    if ( mode == MO_REASPEED )  m_cirMotion.realSpeed.x     = value;
}

float CPhysics::RetCirMotionX(PhysicsMode mode)
{
    if ( mode == MO_ADVACCEL )  return m_cirMotion.advanceAccel.x;
    if ( mode == MO_RECACCEL )  return m_cirMotion.recedeAccel.x;
    if ( mode == MO_STOACCEL )  return m_cirMotion.stopAccel.x;
    if ( mode == MO_TERSPEED )  return m_cirMotion.terrainSpeed.x;
    if ( mode == MO_TERSLIDE )  return m_cirMotion.terrainSlide.x;
    if ( mode == MO_MOTACCEL )  return m_cirMotion.motorAccel.x;
    if ( mode == MO_TERFORCE )  return m_cirMotion.terrainForce.x;
    if ( mode == MO_ADVSPEED )  return m_cirMotion.advanceSpeed.x;
    if ( mode == MO_RECSPEED )  return m_cirMotion.recedeSpeed.x;
    if ( mode == MO_MOTSPEED )  return m_cirMotion.motorSpeed.x;
    if ( mode == MO_CURSPEED )  return m_cirMotion.currentSpeed.x;
    if ( mode == MO_REASPEED )  return m_cirMotion.realSpeed.x;
    return 0.0f;
}

// Specifies the rotation direction.

void CPhysics::SetCirMotionY(PhysicsMode mode, float value)
{
    if ( mode == MO_ADVACCEL )  m_cirMotion.advanceAccel.y  = value;
    if ( mode == MO_RECACCEL )  m_cirMotion.recedeAccel.y   = value;
    if ( mode == MO_STOACCEL )  m_cirMotion.stopAccel.y     = value;
    if ( mode == MO_TERSPEED )  m_cirMotion.terrainSpeed.y  = value;
    if ( mode == MO_TERSLIDE )  m_cirMotion.terrainSlide.y  = value;
    if ( mode == MO_MOTACCEL )  m_cirMotion.motorAccel.y    = value;
    if ( mode == MO_TERFORCE )  m_cirMotion.terrainForce.y  = value;
    if ( mode == MO_ADVSPEED )  m_cirMotion.advanceSpeed.y  = value;
    if ( mode == MO_RECSPEED )  m_cirMotion.recedeSpeed.y   = value;
    if ( mode == MO_MOTSPEED )  m_cirMotion.motorSpeed.y    = value;
    if ( mode == MO_CURSPEED )  m_cirMotion.currentSpeed.y  = value;
    if ( mode == MO_REASPEED )  m_cirMotion.realSpeed.y     = value;
}

float CPhysics::RetCirMotionY(PhysicsMode mode)
{
    if ( mode == MO_ADVACCEL )  return m_cirMotion.advanceAccel.y;
    if ( mode == MO_RECACCEL )  return m_cirMotion.recedeAccel.y;
    if ( mode == MO_STOACCEL )  return m_cirMotion.stopAccel.y;
    if ( mode == MO_TERSPEED )  return m_cirMotion.terrainSpeed.y;
    if ( mode == MO_TERSLIDE )  return m_cirMotion.terrainSlide.y;
    if ( mode == MO_MOTACCEL )  return m_cirMotion.motorAccel.y;
    if ( mode == MO_TERFORCE )  return m_cirMotion.terrainForce.y;
    if ( mode == MO_ADVSPEED )  return m_cirMotion.advanceSpeed.y;
    if ( mode == MO_RECSPEED )  return m_cirMotion.recedeSpeed.y;
    if ( mode == MO_MOTSPEED )  return m_cirMotion.motorSpeed.y;
    if ( mode == MO_CURSPEED )  return m_cirMotion.currentSpeed.y;
    if ( mode == MO_REASPEED )  return m_cirMotion.realSpeed.y;
    return 0.0f;
}

// Specifies the rotation up/down.

void CPhysics::SetCirMotionZ(PhysicsMode mode, float value)
{
    if ( mode == MO_ADVACCEL )  m_cirMotion.advanceAccel.z  = value;
    if ( mode == MO_RECACCEL )  m_cirMotion.recedeAccel.z   = value;
    if ( mode == MO_STOACCEL )  m_cirMotion.stopAccel.z     = value;
    if ( mode == MO_TERSPEED )  m_cirMotion.terrainSpeed.z  = value;
    if ( mode == MO_TERSLIDE )  m_cirMotion.terrainSlide.z  = value;
    if ( mode == MO_MOTACCEL )  m_cirMotion.motorAccel.z    = value;
    if ( mode == MO_TERFORCE )  m_cirMotion.terrainForce.z  = value;
    if ( mode == MO_ADVSPEED )  m_cirMotion.advanceSpeed.z  = value;
    if ( mode == MO_RECSPEED )  m_cirMotion.recedeSpeed.z   = value;
    if ( mode == MO_MOTSPEED )  m_cirMotion.motorSpeed.z    = value;
    if ( mode == MO_CURSPEED )  m_cirMotion.currentSpeed.z  = value;
    if ( mode == MO_REASPEED )  m_cirMotion.realSpeed.z     = value;
}

float CPhysics::RetCirMotionZ(PhysicsMode mode)
{
    if ( mode == MO_ADVACCEL )  return m_cirMotion.advanceAccel.z;
    if ( mode == MO_RECACCEL )  return m_cirMotion.recedeAccel.z;
    if ( mode == MO_STOACCEL )  return m_cirMotion.stopAccel.z;
    if ( mode == MO_TERSPEED )  return m_cirMotion.terrainSpeed.z;
    if ( mode == MO_TERSLIDE )  return m_cirMotion.terrainSlide.z;
    if ( mode == MO_MOTACCEL )  return m_cirMotion.motorAccel.z;
    if ( mode == MO_TERFORCE )  return m_cirMotion.terrainForce.z;
    if ( mode == MO_ADVSPEED )  return m_cirMotion.advanceSpeed.z;
    if ( mode == MO_RECSPEED )  return m_cirMotion.recedeSpeed.z;
    if ( mode == MO_MOTSPEED )  return m_cirMotion.motorSpeed.z;
    if ( mode == MO_CURSPEED )  return m_cirMotion.currentSpeed.z;
    if ( mode == MO_REASPEED )  return m_cirMotion.realSpeed.z;
    return 0.0f;
}


// Returns the linear distance braking.
//
//           v*v
//  d = -----
//       2a

float CPhysics::RetLinStopLength(PhysicsMode sMode, PhysicsMode aMode)
{
    float       speed, accel;

    speed = RetLinMotionX(sMode);  // MO_ADVSPEED/MO_RECSPEED
    accel = RetLinMotionX(aMode);  // MO_ADVACCEL/MO_RECACCEL/MO_STOACCEL

    if ( m_type == TYPE_FLYING && m_bLand )  // flying on the ground?
    {
        speed /= LANDING_SPEED;
        accel *= LANDING_ACCEL;
    }

    return (speed*speed) / (accel*2.0f);
}

// Returns the angle of circular braking.

float CPhysics::RetCirStopLength()
{
    return m_cirMotion.advanceSpeed.y * m_cirMotion.advanceSpeed.y /
           m_cirMotion.stopAccel.y / 2.0f;
}

// Returns the length advanced into a second, on the ground, maximum speed.

float CPhysics::RetLinMaxLength(float dir)
{
    float       dist;

    if ( dir > 0.0f )  dist = m_linMotion.advanceSpeed.x;
    else               dist = m_linMotion.recedeSpeed.x;

    if ( m_type == TYPE_FLYING )
    {
        dist /= 5.0f;
    }

    return dist;
}

// Returns the time needed to travel some distance.

float CPhysics::RetLinTimeLength(float dist, float dir)
{
    float       accel, decel, dps;

    if ( dir > 0.0f )
    {
        accel = RetLinStopLength(MO_ADVSPEED, MO_ADVACCEL);
        decel = RetLinStopLength(MO_ADVSPEED, MO_STOACCEL);
    }
    else
    {
        accel = RetLinStopLength(MO_RECSPEED, MO_RECACCEL);
        decel = RetLinStopLength(MO_RECSPEED, MO_STOACCEL);
    }

    dps = RetLinMaxLength(dir);

    return (dist+accel+decel)/dps;
}

// Returns the length for a forward travel some distance, taking into account the accelerations / decelerations.

float CPhysics::RetLinLength(float dist)
{
    float   accDist, desDist;

    if ( dist > 0.0f )
    {
        accDist = RetLinStopLength(MO_ADVSPEED, MO_ADVACCEL);
        desDist = RetLinStopLength(MO_ADVSPEED, MO_STOACCEL);

        if ( dist > accDist+desDist )
        {
            return dist-desDist;
        }

        return dist*m_linMotion.stopAccel.x /
               (m_linMotion.advanceAccel.x+m_linMotion.stopAccel.x);
    }
    else
    {
        dist = -dist;
        accDist = RetLinStopLength(MO_RECSPEED, MO_RECACCEL);
        desDist = RetLinStopLength(MO_RECSPEED, MO_STOACCEL);

        if ( dist > accDist+desDist )
        {
            return dist-desDist;
        }

        return dist*m_linMotion.stopAccel.x /
               (m_linMotion.recedeAccel.x+m_linMotion.stopAccel.x);
    }
}


// Management of an event.
// Returns FALSE if the object is destroyed.

BOOL CPhysics::EventProcess(const Event &event)
{
    if ( !m_object->RetEnable() )  return TRUE;

    if ( m_brain != 0 )
    {
        m_brain->EventProcess(event);
    }

    if ( event.event == EVENT_FRAME )
    {
        return EventFrame(event);
    }
    return TRUE;
}


// Updates instructions for the motor speed.

void CPhysics::MotorUpdate(float aTime, float rTime)
{
    ObjectType  type;
    CObject*    power;
    D3DVECTOR   pos, motorSpeed;
    float       energy, speed, factor, h;

    type = m_object->RetType();

    motorSpeed = m_motorSpeed;

    if ( type == OBJECT_MOTHER   ||
         type == OBJECT_ANT      ||
         type == OBJECT_SPIDER   ||
         type == OBJECT_BEE      ||
         type == OBJECT_WORM     ||
         type == OBJECT_APOLLO2  ||
         type == OBJECT_MOBILEdr )
    {
        power = 0;
    }
    else if ( type == OBJECT_HUMAN ||
              type == OBJECT_TECH  )
    {
        power = 0;
        if ( m_object->RetFret() != 0 &&  // carries something?
             !m_object->RetCargo() )
        {
            motorSpeed.x *= 0.7f;  // forward more slowly
            motorSpeed.z *= 0.5f;
            motorSpeed.y = -1.0f;  // grave
        }
        if ( m_bSwim )
        {
            if ( m_bLand )  // deep in the water?
            {
                motorSpeed.x *= 0.4f;  // forward more slowly
                motorSpeed.z *= 0.5f;
                motorSpeed.y *= 0.5f;

                if ( m_object->RetFret() != 0 )  // carries something?
                {
                    motorSpeed.x *= 0.2f;
                    motorSpeed.z *= 0.9f;
                    motorSpeed.y *= 0.2f;
                }
            }
            else    // swimming?
            {
                motorSpeed.x *= 0.2f;  // forward more slowly
                motorSpeed.z *= 0.5f;
                motorSpeed.y *= 0.2f;
            }
        }
    }
    else
    {
        power = m_object->RetPower();  // searches for the object battery uses
        if ( power == 0 || power->RetEnergy() == 0.0f )  // no battery or flat?
        {
            motorSpeed.x =  0.0f;
            motorSpeed.z =  0.0f;
            if ( m_bFreeze || m_bLand )
            {
                motorSpeed.y = 0.0f;  // immobile
            }
            else
            {
                motorSpeed.y = -1.0f;  // grave
            }
            SetMotor(FALSE);
        }
    }

    if ( m_object->RetDead() )  // dead man?
    {
        motorSpeed.x = 0.0f;
        motorSpeed.z = 0.0f;
        if ( m_motion->RetAction() == MHS_DEADw )  // drowned?
        {
            motorSpeed.y = 0.0f;  // this is MHS_DEADw going back
        }
        else
        {
            motorSpeed.y = -1.0f;  // grave
        }
        SetMotor(FALSE);
    }

    if ( m_type == TYPE_FLYING && !m_bLand && motorSpeed.y > 0.0f )
    {
        pos = m_object->RetPosition(0);
        h = m_terrain->RetFlyingLimit(pos, type==OBJECT_BEE);
        h += m_object->RetCharacter()->height;
        if ( pos.y > h-40.0f )  // almost at the top?
        {
            factor = 1.0f-(pos.y-(h-40.0f))/40.0f;
            if ( factor < -1.0f )  factor = -1.0f;
            if ( factor >  1.0f )  factor =  1.0f;
            motorSpeed.y *= factor;  // limit the rate of rise
        }
    }

    if ( type != OBJECT_BEE &&
         m_object->RetRange() > 0.0f )  // limited flight range?
    {
        if ( m_bLand || m_bSwim || m_bObstacle )  // on the ground or in the water?
        {
            factor = 1.0f;
            if ( m_bObstacle )  factor = 3.0f;  // in order to leave!
            if ( m_bSwim )  factor = 3.0f;  // cools faster in water
            m_reactorRange += rTime*(1.0f/5.0f)*factor;
            if ( m_reactorRange > 1.0f )
            {
                m_reactorRange = 1.0f;
                if ( m_bLowLevel && m_object->RetSelect() )  // beep cool?
                {
                    m_sound->Play(SOUND_INFO, m_object->RetPosition(0), 1.0f, 2.0f);
                    m_bLowLevel = FALSE;
                }
            }
            m_bObstacle = FALSE;
        }
        else    // in flight?
        {
            m_reactorRange -= rTime*(1.0f/m_object->RetRange());
            if ( m_reactorRange < 0.0f )  m_reactorRange = 0.0f;
            if ( m_reactorRange < 0.5f )  m_bLowLevel = TRUE;
        }

        if ( m_reactorRange == 0.0f )  // reactor tilt?
        {
            motorSpeed.y = -1.0f;  // grave
        }
    }

//? MotorParticule(aTime);

    // Forward/backward.
    if ( motorSpeed.x > 0.0f )
    {
        m_linMotion.motorAccel.x = m_linMotion.advanceAccel.x;
        m_linMotion.motorSpeed.x = m_linMotion.advanceSpeed.x * motorSpeed.x;
    }
    if ( motorSpeed.x < 0.0f )
    {
        m_linMotion.motorAccel.x = m_linMotion.recedeAccel.x;
        m_linMotion.motorSpeed.x = m_linMotion.recedeSpeed.x * motorSpeed.x;
    }
    if ( motorSpeed.x == 0.0f )
    {
        m_linMotion.motorAccel.x = m_linMotion.stopAccel.x;
        m_linMotion.motorSpeed.x = 0.0f;
    }

    // Up/down.
    if ( motorSpeed.y > 0.0f )
    {
        m_linMotion.motorAccel.y = m_linMotion.advanceAccel.y;
        m_linMotion.motorSpeed.y = m_linMotion.advanceSpeed.y * motorSpeed.y;
    }
    if ( motorSpeed.y < 0.0f )
    {
        m_linMotion.motorAccel.y = m_linMotion.recedeAccel.y;
        m_linMotion.motorSpeed.y = m_linMotion.recedeSpeed.y * motorSpeed.y;
    }
    if ( motorSpeed.y == 0.0f )
    {
        m_linMotion.motorAccel.y = m_linMotion.stopAccel.y;
        m_linMotion.motorSpeed.y = 0.0f;
    }

    // Turn left/right.
    speed = motorSpeed.z;
//? if ( motorSpeed.x < 0.0f )  speed = -speed;  // reverse if running back

    if ( motorSpeed.z > 0.0f )
    {
        m_cirMotion.motorAccel.y = m_cirMotion.advanceAccel.y;
        m_cirMotion.motorSpeed.y = m_cirMotion.advanceSpeed.y * speed;
    }
    if ( motorSpeed.z < 0.0f )
    {
        m_cirMotion.motorAccel.y = m_cirMotion.recedeAccel.y;
        m_cirMotion.motorSpeed.y = m_cirMotion.recedeSpeed.y * speed;
    }
    if ( motorSpeed.z == 0.0f )
    {
        m_cirMotion.motorAccel.y = m_cirMotion.stopAccel.y;
        m_cirMotion.motorSpeed.y = 0.0f;
    }

    if ( m_type == TYPE_FLYING && m_bLand )  // flying on the ground?
    {
        if ( type == OBJECT_HUMAN ||
             type == OBJECT_TECH  )
        {
            factor = LANDING_ACCELh;
        }
        else
        {
            factor = LANDING_ACCEL;
        }
        m_linMotion.motorAccel.x = m_linMotion.stopAccel.x*factor;
        m_cirMotion.motorAccel.y = m_cirMotion.stopAccel.y*factor;

        pos = m_object->RetPosition(0);
        h = m_terrain->RetFlyingLimit(pos, type==OBJECT_BEE);
        h += m_object->RetCharacter()->height;
        if ( motorSpeed.y > 0.0f && m_reactorRange > 0.1f && pos.y < h )
        {
            m_bLand = FALSE;  // take off
            SetMotor(TRUE);
            pos.y += 0.05f;  // small initial height (startup)
            m_object->SetPosition(0, pos);
        }
    }

    if ( m_type == TYPE_ROLLING )
    {
        if ( motorSpeed.x == 0.0f &&
             motorSpeed.z == 0.0f )
        {
            SetMotor(FALSE);
        }
        else
        {
            SetMotor(TRUE);
        }
    }

    if ( power != 0 )  // battery transported?
    {
        factor = 1.0f;
        if ( type == OBJECT_MOBILEia ||
             type == OBJECT_MOBILEis ||
             type == OBJECT_MOBILEic ||
             type == OBJECT_MOBILEii )  factor = 0.5f;

        factor /= power->RetCapacity();

        energy = power->RetEnergy();
        energy -= Abs(motorSpeed.x)*rTime*factor*0.005f;
        energy -= Abs(motorSpeed.z)*rTime*factor*0.005f;

        if ( m_type == TYPE_FLYING && motorSpeed.y > 0.0f )
        {
            energy -= motorSpeed.y*rTime*factor*0.01f;
        }
        if ( energy < 0.0f )  energy = 0.0f;
        power->SetEnergy(energy);
    }
}


// Updates the effects of vibration and tilt.

void CPhysics::EffectUpdate(float aTime, float rTime)
{
    Character*  character;
    D3DVECTOR   vibLin, vibCir, incl;
    float       speedLin, speedCir, accel;
    ObjectType  type;
    BOOL        bOnBoard;

    if ( !m_engine->IsVisiblePoint(m_object->RetPosition(0)) )  return;

    type = m_object->RetType();
    character = m_object->RetCharacter();

    bOnBoard = FALSE;
    if ( m_object->RetSelect() &&
         m_camera->RetType() == CAMERA_ONBOARD )
    {
        bOnBoard = TRUE;
    }

    vibLin = m_motion->RetLinVibration();
    vibCir = m_motion->RetCirVibration();
    incl   = m_motion->RetInclinaison();

    if ( type == OBJECT_HUMAN ||  // human?
         type == OBJECT_TECH  )
    {
        if ( !m_bLand && !m_bSwim )  // in flight?
        {
            vibLin.y = sinf(aTime*2.00f)*0.5f+
                       sinf(aTime*2.11f)*0.3f;

            vibCir.z = sinf(aTime*PI* 2.01f)*(PI/150.0f)+
                       sinf(aTime*PI* 2.51f)*(PI/200.0f)+
                       sinf(aTime*PI*19.01f)*(PI/400.0f);

            vibCir.x = sinf(aTime*PI* 2.03f)*(PI/150.0f)+
                       sinf(aTime*PI* 2.52f)*(PI/200.0f)+
                       sinf(aTime*PI*19.53f)*(PI/400.0f);

            speedLin = m_linMotion.realSpeed.x / m_linMotion.advanceSpeed.x;
            speedCir = m_cirMotion.realSpeed.y / m_cirMotion.advanceSpeed.y;
            incl.x = -speedLin*speedCir*0.5f;  // looks if turn

//?         speedLin  = m_linMotion.currentSpeed.x - m_linMotion.motorSpeed.x*1.5f;
            speedLin  = m_linMotion.currentSpeed.x - m_linMotion.motorSpeed.x*1.2f;
            speedLin /= m_linMotion.advanceSpeed.x;
            m_linMotion.finalInclin.z = speedLin*1.4f;
            if ( incl.z < m_linMotion.finalInclin.z )
            {
                incl.z += rTime*0.4f;
                if ( incl.z > m_linMotion.finalInclin.z )
                {
                    incl.z = m_linMotion.finalInclin.z;
                }
            }
            else if ( incl.z > m_linMotion.finalInclin.z )
            {
                incl.z -= rTime*0.4f;
                if ( incl.z < m_linMotion.finalInclin.z )
                {
                    incl.z = m_linMotion.finalInclin.z;
                }
            }

            vibLin *= m_linVibrationFactor;
            vibCir *= m_cirVibrationFactor;
            incl *= m_inclinaisonFactor;

            m_motion->SetLinVibration(vibLin);
            m_motion->SetCirVibration(vibCir);
            m_motion->SetInclinaison(incl);
        }
        else if ( m_bSwim )  // swimming?
        {
            vibLin.y = sinf(aTime*2.00f)*0.5f+
                       sinf(aTime*2.11f)*0.3f;

            vibCir.z = sinf(aTime*PI* 2.01f)*(PI/150.0f)+
                       sinf(aTime*PI* 2.51f)*(PI/200.0f)+
//?                    sinf(aTime*PI*19.01f)*(PI/400.0f)-PI/2.0f;
                       sinf(aTime*PI*19.01f)*(PI/400.0f);

            vibCir.x = sinf(aTime*PI* 2.03f)*(PI/150.0f)+
                       sinf(aTime*PI* 2.52f)*(PI/200.0f)+
                       sinf(aTime*PI*19.53f)*(PI/400.0f);

            speedLin = m_linMotion.realSpeed.x / m_linMotion.advanceSpeed.x;
            speedCir = m_cirMotion.realSpeed.y / m_cirMotion.advanceSpeed.y;
            incl.x = -speedLin*speedCir*5.0f;  // looks if turn

//?         speedLin  = m_linMotion.currentSpeed.x - m_linMotion.motorSpeed.x*1.5f;
            speedLin  = m_linMotion.currentSpeed.x - m_linMotion.motorSpeed.x*1.2f;
            speedLin /= m_linMotion.advanceSpeed.x;
            m_linMotion.finalInclin.z = speedLin*1.4f;
            if ( incl.z < m_linMotion.finalInclin.z )
            {
                incl.z += rTime*0.4f;
                if ( incl.z > m_linMotion.finalInclin.z )
                {
                    incl.z = m_linMotion.finalInclin.z;
                }
            }
            else if ( incl.z > m_linMotion.finalInclin.z )
            {
                incl.z -= rTime*0.4f;
                if ( incl.z < m_linMotion.finalInclin.z )
                {
                    incl.z = m_linMotion.finalInclin.z;
                }
            }

            if ( m_linMotion.realSpeed.y > 0.0f )  // up?
            {
                vibCir.z += m_linMotion.realSpeed.y*0.05f;
            }
            else    // down?
            {
                vibCir.z += m_linMotion.realSpeed.y*0.12f;
            }
            vibCir.z -= PI*0.4f;

            vibLin *= m_linVibrationFactor;
            vibCir *= m_cirVibrationFactor;
            incl *= m_inclinaisonFactor;

            m_motion->SetLinVibration(vibLin);
            m_motion->SetCirVibration(vibCir);
            m_motion->SetInclinaison(incl);
        }
        else
        {
            m_motion->SetLinVibration(D3DVECTOR(0.0f, 0.0f, 0.0f));
//?         m_motion->SetCirVibration(D3DVECTOR(0.0f, 0.0f, 0.0f));
//?         m_motion->SetInclinaison(D3DVECTOR(0.0f, 0.0f, 0.0f));
        }
    }

    if ( type == OBJECT_MOBILEwa ||
         type == OBJECT_MOBILEwc ||
         type == OBJECT_MOBILEwi ||
         type == OBJECT_MOBILEws ||
         type == OBJECT_MOBILEwt ||
         type == OBJECT_MOBILEtg ||
         type == OBJECT_APOLLO2  )  // wheels?
    {
        speedLin = m_linMotion.realSpeed.x / m_linMotion.advanceSpeed.x;
        speedCir = m_cirMotion.realSpeed.y / m_cirMotion.advanceSpeed.y;
        incl.x = speedLin*speedCir*0.20f;  // looks if turn
        if ( type == OBJECT_APOLLO2 )  incl.x *= 0.25f;

        speedLin  = m_linMotion.currentSpeed.x - m_linMotion.motorSpeed.x;
        speedLin /= m_linMotion.advanceSpeed.x;
        if ( speedLin > 1.0f )  speedLin = 1.0f;
        m_linMotion.finalInclin.z = -speedLin*0.30f;
        accel = (0.40f-Abs(incl.z))*4.0f;
        if ( incl.z < m_linMotion.finalInclin.z )
        {
            incl.z += rTime*accel;
            if ( incl.z > m_linMotion.finalInclin.z )
            {
                incl.z = m_linMotion.finalInclin.z;
            }
        }
        else if ( incl.z > m_linMotion.finalInclin.z )
        {
            incl.z -= rTime*accel;
            if ( incl.z < m_linMotion.finalInclin.z )
            {
                incl.z = m_linMotion.finalInclin.z;
            }
        }
        if ( bOnBoard )  incl.z *= 0.1f;
        if ( type == OBJECT_APOLLO2 )  incl.z *= 0.25f;
        m_object->SetInclinaison(incl);

        vibLin.x = 0.0f;
        vibLin.z = 0.0f;
        vibLin.y = Abs(character->wheelFront*sinf(incl.z))*0.8f +
                   Abs(character->wheelRight*sinf(incl.x))*0.5f;
        m_motion->SetLinVibration(vibLin);
    }

    if ( type == OBJECT_MOBILEfa ||
         type == OBJECT_MOBILEfc ||
         type == OBJECT_MOBILEfi ||
         type == OBJECT_MOBILEfs ||
         type == OBJECT_MOBILEft )  // fliyng?
    {
        if ( m_bLand )  // on the ground?
        {
            m_motion->SetLinVibration(D3DVECTOR(0.0f, 0.0f, 0.0f));
            m_motion->SetCirVibration(D3DVECTOR(0.0f, 0.0f, 0.0f));
            m_motion->SetInclinaison(D3DVECTOR(0.0f, 0.0f, 0.0f));
        }
        else    // in flight?
        {
            vibLin.y = sinf(aTime*2.00f)*0.5f+
                       sinf(aTime*2.11f)*0.3f;

            vibCir.z = sinf(aTime*PI* 2.01f)*(PI/150.0f)+
                       sinf(aTime*PI* 2.51f)*(PI/200.0f)+
                       sinf(aTime*PI*19.01f)*(PI/400.0f);

            vibCir.x = sinf(aTime*PI* 2.03f)*(PI/150.0f)+
                       sinf(aTime*PI* 2.52f)*(PI/200.0f)+
                       sinf(aTime*PI*19.53f)*(PI/400.0f);

            if ( bOnBoard )  vibCir *= 0.4f;

            speedLin = m_linMotion.realSpeed.x / m_linMotion.advanceSpeed.x;
            speedCir = m_cirMotion.realSpeed.y / m_cirMotion.advanceSpeed.y;
            incl.x = -speedLin*speedCir*0.5f;  // looks if turn

//?         speedLin  = m_linMotion.currentSpeed.x - m_linMotion.motorSpeed.x*1.5f;
            speedLin  = m_linMotion.currentSpeed.x - m_linMotion.motorSpeed.x*1.2f;
            speedLin /= m_linMotion.advanceSpeed.x;
            m_linMotion.finalInclin.z = speedLin*0.8f;
            if ( incl.z < m_linMotion.finalInclin.z )
            {
                incl.z += rTime*0.4f;
                if ( incl.z > m_linMotion.finalInclin.z )
                {
                    incl.z = m_linMotion.finalInclin.z;
                }
            }
            else if ( incl.z > m_linMotion.finalInclin.z )
            {
                incl.z -= rTime*0.4f;
                if ( incl.z < m_linMotion.finalInclin.z )
                {
                    incl.z = m_linMotion.finalInclin.z;
                }
            }
//?         if ( bOnBoard )  incl.z *= 0.5f;

            vibLin *= m_linVibrationFactor;
            vibCir *= m_cirVibrationFactor;
            incl *= m_inclinaisonFactor;

            m_motion->SetLinVibration(vibLin);
            m_motion->SetCirVibration(vibCir);
            m_motion->SetInclinaison(incl);
        }
    }

    if ( type == OBJECT_BEE )  // bee?
    {
        if ( !m_bLand )  // in flight?
        {
            vibLin.y = sinf(aTime*2.00f)*0.5f+
                       sinf(aTime*2.11f)*0.3f;

            vibCir.z = (Rand()-0.5f)*0.1f+
                       sinf(aTime*PI* 2.01f)*(PI/150.0f)+
                       sinf(aTime*PI* 2.51f)*(PI/200.0f)+
                       sinf(aTime*PI*19.01f)*(PI/400.0f);

            vibCir.x = (Rand()-0.5f)*0.1f+
                       sinf(aTime*PI* 2.03f)*(PI/150.0f)+
                       sinf(aTime*PI* 2.52f)*(PI/200.0f)+
                       sinf(aTime*PI*19.53f)*(PI/400.0f);

            speedLin = m_linMotion.realSpeed.x / m_linMotion.advanceSpeed.x;
            speedCir = m_cirMotion.realSpeed.y / m_cirMotion.advanceSpeed.y;
            incl.x = -speedLin*speedCir*1.5f;  // looks if turn

//?         speedLin  = m_linMotion.currentSpeed.x - m_linMotion.motorSpeed.x*1.5f;
            speedLin  = m_linMotion.currentSpeed.x - m_linMotion.motorSpeed.x*1.2f;
            speedLin /= m_linMotion.advanceSpeed.x;
            m_linMotion.finalInclin.z = speedLin*1.4f;
            if ( incl.z < m_linMotion.finalInclin.z )
            {
                incl.z += rTime*1.6f;
                if ( incl.z > m_linMotion.finalInclin.z )
                {
                    incl.z = m_linMotion.finalInclin.z;
                }
            }
            else if ( incl.z > m_linMotion.finalInclin.z )
            {
                incl.z -= rTime*1.6f;
                if ( incl.z < m_linMotion.finalInclin.z )
                {
                    incl.z = m_linMotion.finalInclin.z;
                }
            }

            vibLin *= m_linVibrationFactor;
            vibCir *= m_cirVibrationFactor;
            incl *= m_inclinaisonFactor;

            m_motion->SetLinVibration(vibLin);
            m_motion->SetCirVibration(vibCir);
            m_motion->SetInclinaison(incl);
        }
    }
}


// Updates structure Motion.

void CPhysics::UpdateMotionStruct(float rTime, Motion &motion)
{
    float   speed, motor;

    // Management for the coordinate x.
    speed = motion.currentSpeed.x;
    motor = motion.motorSpeed.x * m_inclinaisonFactor;
    if ( speed < motor )
    {
        speed += rTime*motion.motorAccel.x;  // accelerates
        if ( speed > motor )
        {
            speed = motor;  // does not exceed the speed
        }
    }
    if ( speed > motor )
    {
        speed -= rTime*motion.motorAccel.x;  // decelerates
        if ( speed < motor )
        {
            speed = motor;  // does not exceed the speed
        }
    }
    motion.currentSpeed.x = speed;
    motion.realSpeed.x    = speed;

    if ( Abs(motion.terrainSpeed.x) > motion.terrainSlide.x )
    {
        if ( motion.terrainSpeed.x > 0 )
        {
            speed = motion.terrainSpeed.x - motion.terrainSlide.x;
        }
        else
        {
            speed = motion.terrainSpeed.x + motion.terrainSlide.x;
        }
        motion.realSpeed.x += speed;
    }

    // Management for the coordinate y.
    speed = motion.currentSpeed.y;
    motor = motion.motorSpeed.y;  // unlimited speed!
    if ( speed < motor )
    {
        speed += rTime*motion.motorAccel.y;  // accelerates
        if ( speed > motor )
        {
            speed = motor;  // does not exceed the speed
        }
    }
    if ( speed > motor )
    {
        speed -= rTime*motion.motorAccel.y;  // decelerates
        if ( speed < motor )
        {
            speed = motor;  // does not exceed the speed
        }
    }
    motion.currentSpeed.y = speed;
    motion.realSpeed.y    = speed;

    if ( Abs(motion.terrainSpeed.y) > motion.terrainSlide.y )
    {
        if ( motion.terrainSpeed.y > 0 )
        {
            speed = motion.terrainSpeed.y - motion.terrainSlide.y;
        }
        else
        {
            speed = motion.terrainSpeed.y + motion.terrainSlide.y;
        }
        motion.realSpeed.y += speed;
    }

    // Management for the coordinate z.
    speed = motion.currentSpeed.z;
    motor = motion.motorSpeed.z * m_inclinaisonFactor;
    if ( speed < motor )
    {
        speed += rTime*motion.motorAccel.z;  // accelerates
        if ( speed > motor )
        {
            speed = motor;  // does not exceed the speed
        }
    }
    if ( speed > motor )
    {
        speed -= rTime*motion.motorAccel.z;  // decelerates
        if ( speed < motor )
        {
            speed = motor;  // does not exceed the speed
        }
    }
    motion.currentSpeed.z = speed;
    motion.realSpeed.z    = speed;

    if ( Abs(motion.terrainSpeed.z) > motion.terrainSlide.z )
    {
        if ( motion.terrainSpeed.z > 0 )
        {
            speed = motion.terrainSpeed.z - motion.terrainSlide.z;
        }
        else
        {
            speed = motion.terrainSpeed.z + motion.terrainSlide.z;
        }
        motion.realSpeed.z += speed;
    }
}


// Makes physics evolve as time elapsed.
// Returns FALSE if the object is destroyed.
//
//  a:  acceleration
//  v1: velocity at time t1
//  v2: velocity at time t2
//  dt: time elapsed since t1, then: dt = t2-t1
//  dd: difference in distance (advance)
//
//  v2 = v1 + a*dt
//  dd = v2*dt

BOOL CPhysics::EventFrame(const Event &event)
{
    ObjectType  type;
    D3DMATRIX   objRotate, matRotate;
    D3DVECTOR   iPos, iAngle, tAngle, pos, newpos, angle, newangle, n;
    float       h, w;
    int         i;

    if ( m_engine->RetPause() )  return TRUE;

    m_time += event.rTime;
    m_timeUnderWater += event.rTime;
    m_soundTimeJostle += event.rTime;

    type = m_object->RetType();

    FrameParticule(m_time, event.rTime);
    MotorUpdate(m_time, event.rTime);
    EffectUpdate(m_time, event.rTime);
    WaterFrame(m_time, event.rTime);

    iPos   = pos   = m_object->RetPosition(0);
    iAngle = angle = m_object->RetAngle(0);

    // Accelerate is the descent, brake is the ascent.
    if ( m_bFreeze || m_object->RetDead() )
    {
        m_linMotion.terrainSpeed.x = 0.0f;
        m_linMotion.terrainSpeed.z = 0.0f;
        m_linMotion.terrainSpeed.y = 0.0f;
    }
    else
    {
        tAngle = angle;
        h = m_terrain->RetBuildingFactor(pos);
        if ( type == OBJECT_HUMAN ||
             type == OBJECT_TECH  )
        {
            if ( m_linMotion.currentSpeed.x == 0.0f )
            {
                h *= 0.5f;  // immobile man -> slippage
            }
            FloorAngle(pos, tAngle);  // calculates the angle with the ground
        }
#if 1
        if ( pos.y < m_water->RetLevel(m_object) )  // underwater?
        {
            h *= 0.5f;
        }
#endif
//?     m_linMotion.terrainSpeed.x = -tAngle.z*m_linMotion.terrainForce.x*h;
//?     m_linMotion.terrainSpeed.z =  tAngle.x*m_linMotion.terrainForce.z*h;
//?     m_linMotion.terrainSpeed.x = -sinf(tAngle.z)*PI*0.5f*m_linMotion.terrainForce.x*h;
//?     m_linMotion.terrainSpeed.z =  sinf(tAngle.x)*PI*0.5f*m_linMotion.terrainForce.z*h;
        m_linMotion.terrainSpeed.x = -tanf(tAngle.z)*0.9f*m_linMotion.terrainForce.x*h;
        m_linMotion.terrainSpeed.z =  tanf(tAngle.x)*0.9f*m_linMotion.terrainForce.z*h;
        m_linMotion.terrainSpeed.y = 0.0f;

        // If the terrain is very steep, do not exaggerate!
        if ( m_linMotion.terrainSpeed.x >  50.0f )  m_linMotion.terrainSpeed.x =  20.0f;
        if ( m_linMotion.terrainSpeed.x < -50.0f )  m_linMotion.terrainSpeed.x = -20.0f;
        if ( m_linMotion.terrainSpeed.z >  50.0f )  m_linMotion.terrainSpeed.z =  20.0f;
        if ( m_linMotion.terrainSpeed.z < -50.0f )  m_linMotion.terrainSpeed.z = -20.0f;
    }

    if ( type == OBJECT_BEE && !m_bLand )
    {
        h = m_floorLevel;  // ground level
        w = m_water->RetLevel(m_object);
        if ( h < w )  h = w;
        h = pos.y-h-10.0f;  // maximum height (*)
        if ( h < 0.0f )  h = 0.0f;
        m_linMotion.terrainSpeed.y = -h*2.5f;  // is not above
    }

    // (*)  High enough to pass over the tower defense (OBJECT_TOWER),
    //      but not too much to pass under the cover of the ship (OBJECT_BASE)!

    UpdateMotionStruct(event.rTime, m_linMotion);
    UpdateMotionStruct(event.rTime, m_cirMotion);

    newangle = angle + event.rTime*m_cirMotion.realSpeed;
    MatRotateZXY(matRotate, newangle);
    newpos = event.rTime*m_linMotion.realSpeed;
    newpos = Transform(matRotate, newpos);
    newpos += pos;

    m_terrain->LimitPos(newpos);

    if ( m_type == TYPE_FLYING && !m_bLand )
    {
        h = m_terrain->RetFlyingLimit(newpos, type==OBJECT_BEE);
        h += m_object->RetCharacter()->height;
        if ( newpos.y > h )  newpos.y = h;
    }

    if ( m_bForceUpdate        ||
         newpos.x   != pos.x   ||
         newpos.y   != pos.y   ||
         newpos.z   != pos.z   ||
         newangle.x != angle.x ||
         newangle.y != angle.y ||
         newangle.z != angle.z )
    {
        FloorAdapt(m_time, event.rTime, newpos, newangle);
    }

    if ( m_bForceUpdate    ||
         newpos.x != pos.x ||
         newpos.y != pos.y ||
         newpos.z != pos.z )
    {
        i = ObjectAdapt(newpos, newangle);
        if ( i == 2 )  // object destroyed?
        {
            return FALSE;
        }
        if ( i == 1 )  // immobile object?
        {
            newpos = iPos;  // keeps the initial position, but accepts the rotation
        }
    }

    if ( newangle.x != angle.x ||
         newangle.y != angle.y ||
         newangle.z != angle.z )
    {
        m_object->SetAngle(0, newangle);
    }

    if ( newpos.x != pos.x ||
         newpos.y != pos.y ||
         newpos.z != pos.z )
    {
        m_object->SetPosition(0, newpos);
    }

    MotorParticule(m_time, event.rTime);
    SoundMotor(event.rTime);

    m_bForceUpdate = FALSE;

    return TRUE;
}

// Starts or stops the engine sounds.

void CPhysics::SoundMotor(float rTime)
{
    CObject*    power;
    ObjectType  type;
    float       energy;

    m_lastSoundInsect -= rTime;
    type = m_object->RetType();

    if ( type == OBJECT_MOTHER )
    {
        if ( m_lastSoundInsect <= 0.0f && m_object->RetActif() )
        {
            m_sound->Play(SOUND_INSECTm, m_object->RetPosition(0));
            if ( m_bMotor )  m_lastSoundInsect = 0.4f+Rand()*2.5f;
            else             m_lastSoundInsect = 1.5f+Rand()*4.0f;
        }
    }
    else if ( type == OBJECT_ANT )
    {
        if ( m_object->RetBurn()  ||
             m_object->RetFixed() )
        {
            if ( m_lastSoundInsect <= 0.0f )
            {
                m_sound->Play(SOUND_INSECTa, m_object->RetPosition(0), 1.0f, 1.5f+Rand()*0.5f);
                m_lastSoundInsect = 0.4f+Rand()*0.6f;
            }
        }
        else if ( m_object->RetActif() )
        {
            if ( m_lastSoundInsect <= 0.0f )
            {
                m_sound->Play(SOUND_INSECTa, m_object->RetPosition(0));
                if ( m_bMotor )  m_lastSoundInsect = 0.4f+Rand()*2.5f;
                else             m_lastSoundInsect = 1.5f+Rand()*4.0f;
            }
        }
    }
    else if ( type == OBJECT_BEE )
    {
        if ( m_object->RetActif() )
        {
            if ( m_lastSoundInsect <= 0.0f )
            {
                m_sound->Play(SOUND_INSECTb, m_object->RetPosition(0));
                if ( m_bMotor )  m_lastSoundInsect = 0.4f+Rand()*2.5f;
                else             m_lastSoundInsect = 1.5f+Rand()*4.0f;
            }
        }
        else if ( m_object->RetBurn() )
        {
            if ( m_lastSoundInsect <= 0.0f )
            {
                m_sound->Play(SOUND_INSECTb, m_object->RetPosition(0), 1.0f, 1.5f+Rand()*0.5f);
                m_lastSoundInsect = 0.3f+Rand()*0.5f;
            }
        }
    }
    else if ( type == OBJECT_WORM )
    {
        if ( m_object->RetActif() )
        {
            if ( m_lastSoundInsect <= 0.0f )
            {
                m_sound->Play(SOUND_INSECTw, m_object->RetPosition(0));
                if ( m_bMotor )  m_lastSoundInsect = 0.4f+Rand()*2.5f;
                else             m_lastSoundInsect = 1.5f+Rand()*4.0f;
            }
        }
        else if ( m_object->RetBurn() )
        {
            if ( m_lastSoundInsect <= 0.0f )
            {
                m_sound->Play(SOUND_INSECTw, m_object->RetPosition(0), 1.0f, 1.5f+Rand()*0.5f);
                m_lastSoundInsect = 0.2f+Rand()*0.2f;
            }
        }
    }
    else if ( type == OBJECT_SPIDER )
    {
        if ( m_object->RetBurn()  ||
             m_object->RetFixed() )
        {
            if ( m_lastSoundInsect <= 0.0f )
            {
                m_sound->Play(SOUND_INSECTs, m_object->RetPosition(0), 1.0f, 1.5f+Rand()*0.5f);
                m_lastSoundInsect = 0.4f+Rand()*0.6f;
            }
        }
        else if ( m_object->RetActif() )
        {
            if ( m_lastSoundInsect <= 0.0f )
            {
                m_sound->Play(SOUND_INSECTs, m_object->RetPosition(0));
                if ( m_bMotor )  m_lastSoundInsect = 0.4f+Rand()*2.5f;
                else             m_lastSoundInsect = 1.5f+Rand()*4.0f;
            }
        }
    }
    else    // vehicle?
    {
        if ( m_type == TYPE_ROLLING )
        {
            if ( m_bMotor && m_object->RetActif() )
            {
                SoundMotorFull(rTime, type);  // full diet
            }
            else
            {
                energy = 0.0f;
                power = m_object->RetPower();
                if ( power != 0 )
                {
                    energy = power->RetEnergy();
                }

                if ( m_object->RetSelect() &&
                     energy != 0.0f        )
                {
                    SoundMotorSlow(rTime, type);  // in slow motion
                }
                else
                {
                    SoundMotorStop(rTime, type);  // to the stop
                }
            }
        }

        if ( m_type == TYPE_FLYING )
        {
            if ( m_bMotor && !m_bSwim &&
                 m_object->RetActif() && !m_object->RetDead() )
            {
                SoundReactorFull(rTime, type);  // full diet
            }
            else
            {
                SoundReactorStop(rTime, type);  // to the stop
            }
        }
    }
}

// Detonates the object if it is underwater.

void CPhysics::WaterFrame(float aTime, float rTime)
{
    ObjectType  type;
    D3DVECTOR   pos, speed;
    FPOINT      dim;
    float       level;

    level = m_water->RetLevel();
    if ( level == 0.0f )  return;  // no water?
    if ( m_object->RetTruck() != 0 )  return;  // object transported?

    // Management of flames into the lava.
    pos = m_object->RetPosition(0);
    if ( m_water->RetLava() &&
         pos.y-m_object->RetCharacter()->height <= level )
    {
        if ( m_lastFlameParticule+m_engine->ParticuleAdapt(0.05f) <= aTime )
        {
            m_lastFlameParticule = aTime;

            pos = m_object->RetPosition(0);
            pos.x += (Rand()-0.5f)*3.0f;
            pos.z += (Rand()-0.5f)*3.0f;
            speed.x = 0.0f;
            speed.z = 0.0f;
            speed.y = Rand()*5.0f+3.0f;
            dim.x = Rand()*2.0f+1.0f;
            dim.y = dim.x;
            m_particule->CreateParticule(pos, speed, dim, PARTIFLAME, 2.0f, 0.0f, 0.2f);

            pos = m_object->RetPosition(0);
            pos.y -= 2.0f;
            pos.x += (Rand()-0.5f)*5.0f;
            pos.z += (Rand()-0.5f)*5.0f;
            speed.x = 0.0f;
            speed.z = 0.0f;
            speed.y = 6.0f+Rand()*6.0f+6.0f;
            dim.x = Rand()*1.5f+1.0f+3.0f;
            dim.y = dim.x;
            m_particule->CreateParticule(pos, speed, dim, PARTISMOKE3, 4.0f);
        }
    }

    pos = m_object->RetPosition(0);
    if ( pos.y >= m_water->RetLevel(m_object) )  return;  // out of water?

    type = m_object->RetType();
    if ( type == OBJECT_TOTO )  return;
    if ( type == OBJECT_NULL )  return;

    if ( !m_object->RetActif() )  return;
    if ( m_object->RetResetBusy() )  return;  // reset in progress?

    if ( m_water->RetLava()      ||
         (type == OBJECT_HUMAN   &&
          m_object->RetOption() != 0 ) ||  // human without a helmet?
         type == OBJECT_MOBILEfa ||
         type == OBJECT_MOBILEta ||
         type == OBJECT_MOBILEwa ||
         type == OBJECT_MOBILEia ||
         type == OBJECT_MOBILEfc ||
         type == OBJECT_MOBILEtc ||
         type == OBJECT_MOBILEwc ||
         type == OBJECT_MOBILEic ||
         type == OBJECT_MOBILEfi ||
         type == OBJECT_MOBILEti ||
         type == OBJECT_MOBILEwi ||
         type == OBJECT_MOBILEii ||
         type == OBJECT_MOBILEfs ||
         type == OBJECT_MOBILEts ||
         type == OBJECT_MOBILEws ||
         type == OBJECT_MOBILEis ||
         type == OBJECT_MOBILErt ||
         type == OBJECT_MOBILErc ||
         type == OBJECT_MOBILErr ||
         type == OBJECT_MOBILErs ||
         type == OBJECT_MOBILEft ||
         type == OBJECT_MOBILEtt ||
         type == OBJECT_MOBILEwt ||
         type == OBJECT_MOBILEit ||
         type == OBJECT_MOBILEdr ||
         type == OBJECT_APOLLO2  )  // vehicle not underwater?
    {
        m_object->ExploObject(EXPLO_WATER, 1.0f);  // starts explosion
    }
}

// Sounds the engine at full power.

void CPhysics::SoundMotorFull(float rTime, ObjectType type)
{
    Sound       sound;
    float       amplitude, time, freq;

    if ( type == OBJECT_MOBILEia ||
         type == OBJECT_MOBILEic ||
         type == OBJECT_MOBILEii ||
         type == OBJECT_MOBILEis )
    {
        if ( m_soundChannel == -1 )
        {
            m_soundChannel = m_sound->Play(SOUND_MOTORi, m_object->RetPosition(0), 0.0f, 1.0f, TRUE);
            m_sound->AddEnvelope(m_soundChannel, 1.0f, 1.0f, 0.2f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 1.0f, 1.0f, 1.0f, SOPER_LOOP);
        }
        else
        {
            m_sound->Position(m_soundChannel, m_object->RetPosition(0));
        }

        freq = 1.0f+m_linMotion.terrainSpeed.x/50.0f;
        if ( m_linMotion.realSpeed.x == 0.0f )
        {
            freq -= Abs(m_cirMotion.realSpeed.y/3.0f);
        }
        else
        {
            freq -= Abs(m_cirMotion.realSpeed.y/4.0f);
        }
        m_sound->Frequency(m_soundChannel, freq);

        return;
    }

    if ( type == OBJECT_MOBILEsa )
    {
        sound = SOUND_MOTORs;
        amplitude = 0.6f;
        time = 0.5f;
    }
    else if ( type == OBJECT_MOBILErt ||
              type == OBJECT_MOBILErc ||
              type == OBJECT_MOBILErr ||
              type == OBJECT_MOBILErs )
    {
        sound = SOUND_MOTORr;
        amplitude = 1.0f;
        time = 0.7f;
    }
    else if ( type == OBJECT_MOBILEta ||
              type == OBJECT_MOBILEtc ||
              type == OBJECT_MOBILEti ||
              type == OBJECT_MOBILEts )
    {
        sound = SOUND_MOTORt;
        amplitude = 1.0f;
        time = 0.5f;
    }
    else if ( type == OBJECT_APOLLO2 )
    {
        sound = SOUND_MANIP;
        amplitude = 1.0f;
        time = 0.5f;
    }
    else
    {
        sound = SOUND_MOTORw;
        amplitude = 0.7f;
        time = 0.3f;
    }

    if ( m_object->RetToy() )
    {
        sound = SOUND_MOTORd;
        amplitude = 1.0f;
        time = 0.1f;
    }

    freq = 0.75f+(Abs(m_motorSpeed.x)+Abs(m_motorSpeed.z))*0.25f;
    if ( freq > 1.0f )  freq = 1.0f;
    if ( m_object->RetToy() )  freq = 1.0f;

    if ( m_soundChannel == -1 )
    {
        m_soundChannel = m_sound->Play(sound, m_object->RetPosition(0), 0.0f, 0.5f, TRUE);
        m_sound->AddEnvelope(m_soundChannel, amplitude, freq, time, SOPER_CONTINUE);
        m_sound->AddEnvelope(m_soundChannel, amplitude, freq, 1.0f, SOPER_LOOP);
    }
    else
    {
        m_sound->Position(m_soundChannel, m_object->RetPosition(0));

        if ( m_bSoundSlow )  // in slow motion?
        {
            m_sound->FlushEnvelope(m_soundChannel);
            m_sound->AddEnvelope(m_soundChannel, amplitude, freq, time, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, amplitude, freq, 1.0f, SOPER_LOOP);
            m_bSoundSlow = FALSE;
        }
    }

    freq *= 1.0f + m_linMotion.terrainSpeed.x/100.0f;
    freq *= 1.0f + Abs(m_cirMotion.realSpeed.y/20.0f);
    m_sound->Frequency(m_soundChannel, freq);

    m_soundTimePshhh -= rTime*2.0f;
}

// Sounds the engine idling.

void CPhysics::SoundMotorSlow(float rTime, ObjectType type)
{
    D3DMATRIX*  mat;
    D3DVECTOR   pos, speed;
    FPOINT      dim;
    Sound       sound;
    float       amplitude;
    int         i, max;

    if ( type == OBJECT_MOBILEia ||
         type == OBJECT_MOBILEic ||
         type == OBJECT_MOBILEii ||
         type == OBJECT_MOBILEis )
    {
        if ( m_soundChannel != -1 )  // engine is running?
        {
            m_sound->FlushEnvelope(m_soundChannel);
            m_sound->AddEnvelope(m_soundChannel, 0.0f, 1.0f, 0.3f, SOPER_STOP);
            m_soundChannel = -1;
        }
        return;
    }

    if ( type == OBJECT_MOBILEsa )
    {
        sound = SOUND_MOTORs;
        amplitude = 0.4f;
    }
    else if ( type == OBJECT_MOBILErt ||
              type == OBJECT_MOBILErc ||
              type == OBJECT_MOBILErr ||
              type == OBJECT_MOBILErs )
    {
        sound = SOUND_MOTORr;
        amplitude = 0.9f;
    }
    else if ( type == OBJECT_MOBILEta ||
              type == OBJECT_MOBILEtc ||
              type == OBJECT_MOBILEti ||
              type == OBJECT_MOBILEts )
    {
        sound = SOUND_MOTORt;
        amplitude = 0.7f;
    }
    else if ( type == OBJECT_APOLLO2 )
    {
        if ( m_soundChannel != -1 )  // engine is running?
        {
            m_sound->FlushEnvelope(m_soundChannel);
            m_sound->AddEnvelope(m_soundChannel, 0.0f, 0.5f, 0.3f, SOPER_STOP);
            m_soundChannel = -1;
        }
        return;
    }
    else
    {
        sound = SOUND_MOTORw;
        amplitude = 0.3f;
    }

    if ( m_object->RetToy() )
    {
        sound = SOUND_MOTORd;
        amplitude = 0.0f;
    }

    if ( m_soundChannel == -1 )
    {
        m_soundChannel = m_sound->Play(sound, m_object->RetPosition(0), 0.0f, 0.25f, TRUE);
        m_sound->AddEnvelope(m_soundChannel, amplitude, 0.5f, 0.2f, SOPER_CONTINUE);
        m_sound->AddEnvelope(m_soundChannel, amplitude, 0.5f, 1.0f, SOPER_LOOP);
    }
    else
    {
        m_sound->Position(m_soundChannel, m_object->RetPosition(0));

        if ( !m_bSoundSlow )  // full power?
        {
            m_sound->FlushEnvelope(m_soundChannel);
            m_sound->AddEnvelope(m_soundChannel, amplitude, 0.5f, 0.3f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, amplitude, 0.5f, 1.0f, SOPER_LOOP);
            m_bSoundSlow = TRUE;
        }
    }

    if ( type == OBJECT_MOBILErt ||
         type == OBJECT_MOBILErc ||
         type == OBJECT_MOBILErr ||
         type == OBJECT_MOBILErs )
    {
        m_soundTimePshhh -= rTime;

        if ( m_soundTimePshhh <= 0.0f )
        {
            amplitude = 0.5f-m_soundTimePshhh*0.08f;
            if ( amplitude > 1.0f )  amplitude = 1.0f;
//?         m_sound->Play(SOUND_PSHHH, m_object->RetPosition(0), amplitude);
            m_sound->Play(SOUND_PSHHH, m_object->RetPosition(0), 1.0f);

            m_soundTimePshhh = 4.0f+4.0f*Rand();

            max = (int)(10.0f*m_engine->RetParticuleDensity());
            for ( i=0 ; i<max ; i++ )
            {
                pos = D3DVECTOR(-5.0f, 2.0f, 0.0f);
                pos.x += Rand()*4.0f;
                pos.z += (Rand()-0.5f)*2.0f;

                speed = pos;
                speed.x -= Rand()*4.0f;
                speed.y -= Rand()*3.0f;
                speed.z += (Rand()-0.5f)*6.0f;

                mat = m_object->RetWorldMatrix(0);
                pos   = Transform(*mat, pos);
                speed = Transform(*mat, speed)-pos;

                dim.x = Rand()*1.0f+1.0f;
                dim.y = dim.x;

                m_particule->CreateParticule(pos, speed, dim, PARTIMOTOR, 2.0f);
            }
        }
    }
}

// Sounds the engine not running.

void CPhysics::SoundMotorStop(float rTime, ObjectType type)
{
    if ( type == OBJECT_MOBILEia ||
         type == OBJECT_MOBILEic ||
         type == OBJECT_MOBILEii ||
         type == OBJECT_MOBILEis )
    {
        if ( m_soundChannel != -1 )  // engine is running?
        {
            m_sound->FlushEnvelope(m_soundChannel);
            m_sound->AddEnvelope(m_soundChannel, 0.0f, 1.0f, 0.3f, SOPER_STOP);
            m_soundChannel = -1;
        }
        return;
    }

    if ( m_soundChannel != -1 )  // engine is running?
    {
        m_sound->FlushEnvelope(m_soundChannel);
        m_sound->AddEnvelope(m_soundChannel, 0.0f, 0.5f, 0.3f, SOPER_STOP);
        m_soundChannel = -1;
    }

    m_soundTimePshhh -= rTime*2.0f;
}

// Sounds the reactor at full power.

void CPhysics::SoundReactorFull(float rTime, ObjectType type)
{
    Sound       sound;
    D3DMATRIX*  mat;
    D3DVECTOR   pos, speed;
    FPOINT      dim;
    float       freq;
    int         i;

    if ( m_soundChannelSlide != -1 )  // slides?
    {
        m_sound->FlushEnvelope(m_soundChannelSlide);
        m_sound->AddEnvelope(m_soundChannelSlide, 0.0f, 1.0f, 0.3f, SOPER_STOP);
        m_soundChannelSlide = -1;
    }

    if ( m_reactorRange > 0.0f )
    {
        if ( m_soundChannel == -1 )
        {
            if ( type == OBJECT_HUMAN ||
                 type == OBJECT_TECH  )
            {
                sound = SOUND_FLYh;
            }
            else
            {
                sound = SOUND_FLY;
            }

            m_soundChannel = m_sound->Play(sound, m_object->RetPosition(0), 0.0f, 1.0f, TRUE);
            m_sound->AddEnvelope(m_soundChannel, 1.0f, 1.0f, 0.6f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 1.0f, 1.0f, 1.0f, SOPER_LOOP);
        }
        else
        {
            m_sound->Position(m_soundChannel, m_object->RetPosition(0));
        }

        freq = 1.0f + m_linMotion.realSpeed.y/100.0f;
        freq *= 1.0f + Abs(m_cirMotion.realSpeed.y/5.0f);
        m_sound->Frequency(m_soundChannel, freq);
    }
    else
    {
        if ( m_soundChannel != -1 )  // engine is running?
        {
            m_sound->FlushEnvelope(m_soundChannel);
            m_sound->AddEnvelope(m_soundChannel, 0.0f, 1.0f, 1.0f, SOPER_STOP);
            m_soundChannel = -1;
        }

        if ( m_timeReactorFail <= m_time )
        {
            freq = 1.0f+Rand()*0.5f;
            m_sound->Play(SOUND_FLYf, m_object->RetPosition(0), 1.0f, freq);
            m_camera->StartEffect(CE_PET, m_object->RetPosition(0), 1.0f);

            for ( i=0 ; i<5 ; i++ )
            {
                if ( m_object->RetType() == OBJECT_HUMAN ||
                     m_object->RetType() == OBJECT_TECH  )
                {
                    pos = D3DVECTOR(-1.6f, -0.5f, 0.0f);
                }
                else
                {
                    pos = D3DVECTOR(0.0f, -1.0f, 0.0f);
                }
                pos.x += (Rand()-0.5f)*2.0f;
                pos.z += (Rand()-0.5f)*2.0f;
                mat = m_object->RetWorldMatrix(0);
                pos = Transform(*mat, pos);
                speed.x = (Rand()-0.5f)*5.0f;
                speed.z = (Rand()-0.5f)*5.0f;
                speed.y = -(4.0f+Rand()*4.0f);
                dim.x = (2.0f+Rand()*1.0f);
                dim.y = dim.x;
                m_particule->CreateParticule(pos, speed, dim, PARTISMOKE1, 2.0f, 0.0f, 0.1f);
            }

            m_timeReactorFail = m_time+0.10f+Rand()*0.30f;
        }
        else
        {
            if ( m_object->RetType() == OBJECT_HUMAN ||
                 m_object->RetType() == OBJECT_TECH  )
            {
                pos = D3DVECTOR(-1.6f, -0.5f, 0.0f);
            }
            else
            {
                pos = D3DVECTOR(0.0f, -1.0f, 0.0f);
            }
            pos.x += (Rand()-0.5f)*1.0f;
            pos.z += (Rand()-0.5f)*1.0f;
            mat = m_object->RetWorldMatrix(0);
            pos = Transform(*mat, pos);
            speed.x = (Rand()-0.5f)*2.0f;
            speed.z = (Rand()-0.5f)*2.0f;
            speed.y = -(4.0f+Rand()*4.0f);
            dim.x = (0.7f+Rand()*0.4f);
            dim.y = dim.x;
            m_particule->CreateParticule(pos, speed, dim, PARTISMOKE1, 2.0f, 0.0f, 0.1f);
        }
    }

}

// Sounds the reactor stopped.

void CPhysics::SoundReactorStop(float rTime, ObjectType type)
{
    CObject*    power;
    float       energy;

    energy = 0.0f;
    power = m_object->RetPower();
    if ( power != 0 )
    {
        energy = power->RetEnergy();
    }

    if ( m_soundChannel != -1 )  // engine is running?
    {
        m_sound->FlushEnvelope(m_soundChannel);
        m_sound->AddEnvelope(m_soundChannel, 0.0f, 1.0f, 1.0f, SOPER_STOP);
        m_soundChannel = -1;
    }

    if ( type == OBJECT_HUMAN ||
         type == OBJECT_TECH  )
    {
        if ( m_soundChannelSlide != -1 )  // slides?
        {
            m_sound->FlushEnvelope(m_soundChannelSlide);
            m_sound->AddEnvelope(m_soundChannelSlide, 0.0f, 1.0f, 0.3f, SOPER_STOP);
            m_soundChannelSlide = -1;
        }
    }
    else
    {
        if ( energy != 0.0f &&
             (m_motorSpeed.x != 0.0f ||  // slides with small reactors in skates?
              m_cirMotion.realSpeed.y != 0.0f) )
        {
            if ( m_soundChannelSlide == -1 )
            {
                m_soundChannelSlide = m_sound->Play(SOUND_SLIDE, m_object->RetPosition(0), 0.0f, 1.0f, TRUE);
                m_sound->AddEnvelope(m_soundChannelSlide, 0.5f, 1.0f, 0.3f, SOPER_CONTINUE);
                m_sound->AddEnvelope(m_soundChannelSlide, 0.5f, 1.0f, 1.0f, SOPER_LOOP);
            }
            m_sound->Position(m_soundChannelSlide, m_object->RetPosition(0));
        }
        else
        {
            if ( m_soundChannelSlide != -1 )  // slides?
            {
                m_sound->FlushEnvelope(m_soundChannelSlide);
                m_sound->AddEnvelope(m_soundChannelSlide, 0.0f, 1.0f, 0.3f, SOPER_STOP);
                m_soundChannelSlide = -1;
            }
        }
    }
}


// Adapts the physics of the object based on the ground.

void CPhysics::FloorAdapt(float aTime, float rTime,
                          D3DVECTOR &pos, D3DVECTOR &angle)
{
    Character*  character;
    ObjectType  type;
    D3DVECTOR   norm;
    D3DMATRIX   matRotate;
    float       level, h, f, a1, volume, freq, force;
    BOOL        bOldSwim, bSlopingTerrain;

    type = m_object->RetType();
    character = m_object->RetCharacter();

    level = m_water->RetLevel(m_object);
    bOldSwim = m_bSwim;
    SetSwim( pos.y < level );

    m_floorLevel = m_terrain->RetFloorLevel(pos);  // height above the ground
    h = pos.y-m_floorLevel;
    h -= character->height;
    m_floorHeight = h;

    WaterParticule(aTime, pos, type, m_floorLevel,
                   Abs(m_linMotion.realSpeed.x),
                   Abs(m_cirMotion.realSpeed.y*15.0f));

    if ( m_type == TYPE_ROLLING )
    {
        pos.y -= h;  // plate to the ground immediately
        pos.y += character->height;
        m_floorHeight = 0.0f;
    }

    if ( m_type == TYPE_FLYING )
    {
        bSlopingTerrain = FALSE;  // ground as possible to land

        if ( !m_bLand )  // in flight?
        {
            m_terrain->GetNormal(norm, pos);
            a1 = Abs(RotateAngle(Length(norm.x, norm.z), norm.y));
            if ( a1 < (90.0f-55.0f)*PI/180.0f )  // slope exceeds 55 degrees?
            {
                bSlopingTerrain = TRUE;  // very sloped ground

                if ( h < 4.0f )  // collision with the ground?
                {
                    force = 5.0f+Abs(m_linMotion.realSpeed.x*0.3f)+
                                 Abs(m_linMotion.realSpeed.y*0.3f);
                    m_linMotion.currentSpeed = norm*force;
                    MatRotateXZY(matRotate, -angle);
                    m_linMotion.currentSpeed = Transform(matRotate, m_linMotion.currentSpeed);

                    if ( aTime-m_soundTimeBoum > 0.5f )
                    {
                        volume = Abs(m_linMotion.realSpeed.x*0.02f)+
                                 Abs(m_linMotion.realSpeed.y*0.02f);
                        freq = 0.5f+m_terrain->RetHardness(pos)*2.5f;
                        m_sound->Play(SOUND_BOUM, pos, volume, freq);

                        m_soundTimeBoum = aTime;
                    }

//?                 pos = m_object->RetPosition(0);  // gives position before collision
                }
            }
        }

        if ( (h <= 0.0f || m_bLand) && !bSlopingTerrain )  // on the ground?
        {
            if ( !m_bLand )  // in flight?
            {
                volume = Abs(m_linMotion.realSpeed.y*0.02f);
                freq = 0.5f+m_terrain->RetHardness(pos)*2.5f;
                m_sound->Play(SOUND_BOUM, pos, volume, freq);
            }

            m_bLand = TRUE;  // on the ground?
            SetMotor(FALSE);
            pos.y -= h;  // plate to the ground immediately
            m_floorHeight = 0.0f;

            if ( h < 0.0f )
            {
                f = Abs(m_linMotion.currentSpeed.y/m_linMotion.advanceSpeed.y);
                CrashParticule(f);
            }
            m_linMotion.currentSpeed.y = 0.0f;
            m_inclinaisonFactor  = 1.0f/LANDING_SPEED;  // slips a little to the ground
            m_linVibrationFactor = 0.0f;
            m_cirVibrationFactor = 0.0f;

            if ( type == OBJECT_HUMAN ||
                 type == OBJECT_TECH  )  return;  // always right
        }

        if ( h > 4.0f || bSlopingTerrain )  // meters above the ground?
        {
            if ( m_bSwim )
            {
                m_linVibrationFactor = 1.0f;  // vibrates a max
                m_cirVibrationFactor = 1.0f;
            }
            else
            {
                m_linVibrationFactor = 2.0f;  // vibrates a large max
                m_cirVibrationFactor = 2.0f;
            }
            m_inclinaisonFactor = 1.0f;

            // Gives gently the horizontal.
            if ( angle.x > 0.0f )
            {
                angle.x -= rTime*0.5f;
                if ( angle.x < 0.0f )  angle.x = 0.0f;
            }
            if ( angle.x < 0.0f )
            {
                angle.x += rTime*0.5f;
                if ( angle.x > 0.0f )  angle.x = 0.0f;
            }
            if ( angle.z > 0.0f )
            {
                angle.z -= rTime*0.5f;
                if ( angle.z < 0.0f )  angle.z = 0.0f;
            }
            if ( angle.z < 0.0f )
            {
                angle.z += rTime*0.5f;
                if ( angle.z > 0.0f )  angle.z = 0.0f;
            }
            return;
        }
    }

    if ( m_floorHeight == 0.0f )  // ground plate?
    {
        if ( m_object->RetTraceDown() )
        {
            WheelParticule(m_object->RetTraceColor(), m_object->RetTraceWidth()*g_unit);
        }
        else
        {
            WheelParticule(-1, 0.0f);
        }
    }

    if ( type == OBJECT_HUMAN ||
         type == OBJECT_TECH  ||
         type == OBJECT_WORM  )  return;  // always right

    FloorAngle(pos, angle);  // adjusts the angle at the ground

    if ( m_type == TYPE_FLYING && !m_bLand )  // flying in the air?
    {
        f = h/1.0f;
        if ( f < 0.0f )  f = 0.0f;
        if ( f > 1.0f )  f = 1.0f;
        m_linVibrationFactor = f;
        m_cirVibrationFactor = f;
        angle.z *= 1.0f-f;
        angle.x *= 1.0f-f;

        f = h/1.0f;
        if ( f < 0.0f )  f = 0.0f;
        if ( f > 1.0f )  f = 1.0f;
        m_inclinaisonFactor = f;
    }
}

// Calculates the angle of an object with the field.

void CPhysics::FloorAngle(const D3DVECTOR &pos, D3DVECTOR &angle)
{
    Character*  character;
    D3DVECTOR   pw, norm;
    float       a1, a2;

    character = m_object->RetCharacter();

    pw.x = pos.x+character->wheelFront*cosf(angle.y+PI*0.0f);
    pw.y = pos.y;
    pw.z = pos.z-character->wheelFront*sinf(angle.y+PI*0.0f);
    a1 = atanf(m_terrain->RetFloorHeight(pw)/character->wheelFront);

    pw.x = pos.x+character->wheelBack*cosf(angle.y+PI*1.0f);
    pw.y = pos.y;
    pw.z = pos.z-character->wheelBack*sinf(angle.y+PI*1.0f);
    a2 = atanf(m_terrain->RetFloorHeight(pw)/character->wheelBack);

    angle.z = (a2-a1)/2.0f;

    pw.x = pos.x+character->wheelLeft*cosf(angle.y+PI*0.5f)*cosf(angle.z);
    pw.y = pos.y;
    pw.z = pos.z-character->wheelLeft*sinf(angle.y+PI*0.5f)*cosf(angle.z);
    a1 = atanf(m_terrain->RetFloorHeight(pw)/character->wheelLeft);

    pw.x = pos.x+character->wheelRight*cosf(angle.y+PI*1.5f)*cosf(angle.z);
    pw.y = pos.y;
    pw.z = pos.z-character->wheelRight*sinf(angle.y+PI*1.5f)*cosf(angle.z);
    a2 = atanf(m_terrain->RetFloorHeight(pw)/character->wheelRight);

    angle.x = (a2-a1)/2.0f;
}


// Adapts the physics of the object in relation to other objects.
// Returns 0 -> mobile object
// Returns 1 -> immobile object (because collision)
// Returns 2 -> destroyed object

int CPhysics::ObjectAdapt(const D3DVECTOR &pos, const D3DVECTOR &angle)
{
    CObject*    pObj;
    CPyro*      pyro;
    CPhysics*   ph;
    D3DMATRIX   matRotate;
    D3DVECTOR   iPos, oPos, iiPos, oAngle, oSpeed;
    Sound       sound;
    float       iRad, oRad, distance, force, volume;
    int         i, j, colType;
    ObjectType  iType, oType;

    if ( m_object->RetRuin() )  return 0;  // is burning or exploding?
    if ( !m_object->RetClip() )  return 0;

    // iiPos = sphere center is the old position.
    // iPos  = sphere center has the new position.
    m_object->GetCrashSphere(0, iiPos, iRad);
    iPos = iiPos + (pos - m_object->RetPosition(0));
    iType = m_object->RetType();

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        if ( pObj == m_object )  continue;  // yourself?
        if ( pObj->RetTruck() != 0 )  continue;  // object transported?
        if ( !pObj->RetEnable() )  continue;  // inactive?
        if ( pObj->RetRuin() )  continue;  // is burning or exploding?
        if ( pObj->RetDead() )  continue;  // dead man?

        oType = pObj->RetType();
        if ( oType == OBJECT_NULL                             )  continue;
        if ( oType == OBJECT_TOTO                             )  continue;
//?     if ( iType == OBJECT_BEE    && oType == OBJECT_BEE    )  continue;
        if ( iType == OBJECT_WORM   && oType != OBJECT_WORM   )  continue;
        if ( iType != OBJECT_WORM   && oType == OBJECT_WORM   )  continue;
        if ( iType == OBJECT_MOTHER && oType == OBJECT_ANT    )  continue;
        if ( iType == OBJECT_ANT    && oType == OBJECT_MOTHER )  continue;
        if ( iType == OBJECT_MOTHER && oType == OBJECT_SPIDER )  continue;
        if ( iType == OBJECT_SPIDER && oType == OBJECT_MOTHER )  continue;
        if ( iType == OBJECT_MOTHER && oType == OBJECT_EGG    )  continue;
        if ( iType == OBJECT_EGG    && oType == OBJECT_MOTHER )  continue;

        pObj->GetJotlerSphere(oPos, oRad);
        if ( oRad > 0.0f )
        {
            JostleObject(pObj, iPos, iRad, oPos, oRad);
        }

        if ( iType == OBJECT_MOTHER ||
             iType == OBJECT_ANT    ||
             iType == OBJECT_SPIDER ||
             iType == OBJECT_WORM   ||
             iType == OBJECT_BEE    )  // insect?
        {
            if ( oType == OBJECT_STONE   ||
                 oType == OBJECT_URANIUM ||
                 oType == OBJECT_METAL   ||
                 oType == OBJECT_POWER   ||
                 oType == OBJECT_ATOMIC  ||
                 oType == OBJECT_BULLET  ||
                 oType == OBJECT_BBOX    ||
                 oType == OBJECT_KEYa    ||
                 oType == OBJECT_KEYb    ||
                 oType == OBJECT_KEYc    ||
                 oType == OBJECT_KEYd    ||
                 oType == OBJECT_TNT     ||
                (oType >= OBJECT_PLANT0    && oType <= OBJECT_PLANT19  ) ||
                (oType >= OBJECT_MUSHROOM0 && oType <= OBJECT_MUSHROOM9) )  continue;
        }

#if _TEEN
        if ( oType == OBJECT_WAYPOINT &&
             pObj->RetEnable()        &&
            !m_object->RetResetBusy() )  // driving vehicle?
#else
        if ( oType == OBJECT_WAYPOINT &&
             pObj->RetEnable()        &&
            !m_object->RetResetBusy() &&
             m_object->RetTrainer()   )  // driving vehicle?
#endif
        {
            oPos = pObj->RetPosition(0);
            distance = Length2d(oPos, iPos);
            if ( distance < 4.0f )
            {
                m_sound->Play(SOUND_WAYPOINT, m_object->RetPosition(0));
                pyro = new CPyro(m_iMan);
                pyro->Create(PT_WPCHECK, pObj);
            }
        }

        if ( oType == OBJECT_TARGET2 )
        {
            oPos = pObj->RetPosition(0);
            distance = Length(oPos, iPos);
            if ( distance < 10.0f*1.5f )
            {
                m_sound->Play(SOUND_WAYPOINT, m_object->RetPosition(0));
                pyro = new CPyro(m_iMan);
                pyro->Create(PT_WPCHECK, pObj);
            }
        }

        j = 0;
        while ( pObj->GetCrashSphere(j++, oPos, oRad) )
        {
            if ( iType == OBJECT_MOTHER && oRad <= 1.2f )  continue;
            if ( iType == OBJECT_ANT    && oRad <= 1.2f )  continue;
            if ( iType == OBJECT_SPIDER && oRad <= 1.2f )  continue;
            if ( iType == OBJECT_BEE    && oRad <= 1.2f )  continue;
            if ( iType == OBJECT_WORM   && oRad <= 1.2f )  continue;

            distance = Length(oPos, iPos);
            if ( distance < iRad+oRad )  // collision?
            {
                distance = Length(oPos, iiPos);
                if ( distance >= iRad+oRad )  // view (*)
                {
                    m_bCollision = TRUE;
                    m_bObstacle = TRUE;

                    sound = pObj->RetCrashSphereSound(j-1);
                    if ( sound != SOUND_CLICK )
                    {
                        force = Abs(m_linMotion.realSpeed.x);
                        force *= pObj->RetCrashSphereHardness(j-1)*2.0f;
                        if ( ExploOther(iType, pObj, oType, force) )  continue;
                        colType = ExploHimself(iType, oType, force);
                        if ( colType == 2 )  return 2;  // destroyed?
                        if ( colType == 0 )  continue;  // ignores?
                    }

                    force = Length(m_linMotion.realSpeed);
                    force *= pObj->RetCrashSphereHardness(j-1);
                    volume = Abs(force*0.05f);
                    if ( volume > 1.0f )  volume = 1.0f;
                    if ( sound != SOUND_CLICK )
                    {
                        m_sound->Play(sound, m_object->RetPosition(0), volume);
                    }
                    if ( iType == OBJECT_HUMAN && volume > 0.5f )
                    {
                        m_sound->Play(SOUND_AIE, m_object->RetPosition(0), volume);
                    }

                    if ( m_repeatCollision > 0 )
                    {
                        force *= 0.5f*m_repeatCollision;
                        if ( force > 20.0f )  force = 20.0f;
                    }
                    m_repeatCollision += 2;
                    if ( m_repeatCollision > 10 )
                    {
                        m_repeatCollision = 10;
                    }

                    m_linMotion.currentSpeed = Normalize(iPos-oPos)*force;
                    MatRotateXZY(matRotate, -angle);
                    m_linMotion.currentSpeed = Transform(matRotate, m_linMotion.currentSpeed);
                    if ( m_type == TYPE_ROLLING )
                    {
                        m_linMotion.currentSpeed.y = 0.0f;
                    }

                    ph = pObj->RetPhysics();
                    if ( ph != 0 )
                    {
                        oAngle = pObj->RetAngle(0);
                        oSpeed = Normalize(oPos-iPos)*force;
                        MatRotateXZY(matRotate, -oAngle);
                        oSpeed = Transform(matRotate, oSpeed);
                        if ( ph->RetType() == TYPE_ROLLING )
                        {
                            oSpeed.y = 0.0f;
                        }
                        ph->SetLinMotion(MO_CURSPEED, oSpeed);
                    }
                    return 1;
                }
            }
        }
    }

    if ( m_repeatCollision > 0 )
    {
        m_repeatCollision --;
    }
    return 0;
}

// (*)  Collision has the initial position (iiPos) and the new position (iPos),
//  the obstacle is not known. We can therefore pass through.
//  This is necessary when barriers found "in" a vehicle, not to block it definitely!


// Shakes an object.

BOOL CPhysics::JostleObject(CObject* pObj, D3DVECTOR iPos, float iRad,
                            D3DVECTOR oPos, float oRad)
{
    D3DVECTOR   speed;
    float       distance, force, d, f;

    distance = Length(oPos, iPos);
    if ( distance >= iRad+oRad )  return FALSE;

    d = (iRad+oRad)/2.0f;
    f = (distance-d)/d;  // 0 = off, 1 = near
    if ( f < 0.0f )  f = 0.0f;
    if ( f > 1.0f )  f = 1.0f;

    speed = m_linMotion.realSpeed;
    speed.y = 0.0f;
    force = Length(speed)*f*0.05f;
    if ( force > 1.0f )  force = 1.0f;

    if ( m_soundTimeJostle >= 0.20f )
    {
        m_soundTimeJostle = 0.0f;
        m_sound->Play(SOUND_JOSTLE, iPos, force);
    }

    return pObj->JostleObject(force);
}

// Shakes forcing an object.

BOOL CPhysics::JostleObject(CObject* pObj, float force)
{
    D3DVECTOR   oPos;
    float       oRad;

    pObj->GetJotlerSphere(oPos, oRad);
    if ( oRad <= 0.0f )  return FALSE;

    if ( m_soundTimeJostle >= 0.20f )
    {
        m_soundTimeJostle = 0.0f;
        m_sound->Play(SOUND_JOSTLE, pObj->RetPosition(0), force);
    }

    return pObj->JostleObject(force);
}

// Effects of the explosion on the object buffers.
// Returns TRUE if we ignore this obstacle.

BOOL CPhysics::ExploOther(ObjectType iType,
                          CObject *pObj, ObjectType oType, float force)
{
    CPyro*      pyro;

    if ( !pObj->RetEnable() )  return TRUE;

    JostleObject(pObj, 1.0f);  // shakes the object

    if ( force > 50.0f &&
         (oType == OBJECT_FRET  ||
          oType == OBJECT_METAL ) )
    {
        pyro = new CPyro(m_iMan);
        pyro->Create(PT_EXPLOT, pObj);  // total destruction
    }

    if ( force > 50.0f &&
         (oType == OBJECT_POWER   ||
          oType == OBJECT_ATOMIC  ) )
    {
        pyro = new CPyro(m_iMan);
        pyro->Create(PT_FRAGT, pObj);  // total destruction
    }

    if ( force > 25.0f &&
         (oType == OBJECT_STONE   ||
          oType == OBJECT_URANIUM ) )
    {
        pyro = new CPyro(m_iMan);
        pyro->Create(PT_FRAGT, pObj);  // total destruction
    }

    if ( force > 25.0f &&
         (oType == OBJECT_DERRICK  ||
          oType == OBJECT_FACTORY  ||
          oType == OBJECT_STATION  ||
          oType == OBJECT_CONVERT  ||
          oType == OBJECT_REPAIR   ||
          oType == OBJECT_DESTROYER||
          oType == OBJECT_TOWER    ||
          oType == OBJECT_RESEARCH ||
          oType == OBJECT_RADAR    ||
          oType == OBJECT_INFO     ||
          oType == OBJECT_ENERGY   ||
          oType == OBJECT_LABO     ||
          oType == OBJECT_NUCLEAR  ||
          oType == OBJECT_PARA     ||
          oType == OBJECT_SAFE     ||
          oType == OBJECT_HUSTON   ) )  // building?
    {
        pObj->ExploObject(EXPLO_BOUM, force/400.0f);
    }

    if ( force > 25.0f &&
         (oType == OBJECT_MOBILEwa ||
          oType == OBJECT_MOBILEta ||
          oType == OBJECT_MOBILEfa ||
          oType == OBJECT_MOBILEia ||
          oType == OBJECT_MOBILEwc ||
          oType == OBJECT_MOBILEtc ||
          oType == OBJECT_MOBILEfc ||
          oType == OBJECT_MOBILEic ||
          oType == OBJECT_MOBILEwi ||
          oType == OBJECT_MOBILEti ||
          oType == OBJECT_MOBILEfi ||
          oType == OBJECT_MOBILEii ||
          oType == OBJECT_MOBILEws ||
          oType == OBJECT_MOBILEts ||
          oType == OBJECT_MOBILEfs ||
          oType == OBJECT_MOBILEis ||
          oType == OBJECT_MOBILErt ||
          oType == OBJECT_MOBILErc ||
          oType == OBJECT_MOBILErr ||
          oType == OBJECT_MOBILErs ||
          oType == OBJECT_MOBILEsa ||
          oType == OBJECT_MOBILEwt ||
          oType == OBJECT_MOBILEtt ||
          oType == OBJECT_MOBILEft ||
          oType == OBJECT_MOBILEit ||
          oType == OBJECT_MOBILEdr ||
          oType == OBJECT_APOLLO2  ) )  // vehicle?
    {
        pObj->ExploObject(EXPLO_BOUM, force/200.0f);
    }

    if ( force > 10.0f &&
         (oType == OBJECT_MOBILEtg ||
          oType == OBJECT_TNT      ) )
    {
        pyro = new CPyro(m_iMan);
        pyro->Create(PT_FRAGT, pObj);  // total destruction
    }

    if ( force > 0.0f &&
         oType == OBJECT_BOMB )
    {
        pyro = new CPyro(m_iMan);
        pyro->Create(PT_FRAGT, pObj);  // total destruction
    }

    return FALSE;
}

// Effects of the explosion on the object itself.
// Returns 0 -> mobile object
// Returns 1 -> immobile object
// Returns 2 -> object destroyed

int CPhysics::ExploHimself(ObjectType iType, ObjectType oType, float force)
{
    PyroType    type;
    CPyro*      pyro;

    if ( force > 10.0f &&
         (oType == OBJECT_TNT      ||
          oType == OBJECT_MOBILEtg ) )
    {
        if ( iType == OBJECT_HUMAN )  type = PT_DEADG;
        else                          type = PT_EXPLOT;
        pyro = new CPyro(m_iMan);
        pyro->Create(type, m_object);  // total destruction
        return 2;
    }

    if ( force > 0.0f &&
         oType == OBJECT_BOMB )
    {
        if ( iType == OBJECT_HUMAN )
        {
            type = PT_DEADG;
        }
        else if ( iType == OBJECT_ANT    ||
                  iType == OBJECT_SPIDER ||
                  iType == OBJECT_BEE    )
        {
            type = PT_EXPLOO;
        }
        else
        {
            type = PT_EXPLOT;
        }
        pyro = new CPyro(m_iMan);
        pyro->Create(type, m_object);  // total destruction
        return 2;
    }

    if ( force > 25.0f &&
         (iType == OBJECT_HUMAN    ||
          iType == OBJECT_MOBILEwa ||
          iType == OBJECT_MOBILEta ||
          iType == OBJECT_MOBILEfa ||
          iType == OBJECT_MOBILEia ||
          iType == OBJECT_MOBILEwc ||
          iType == OBJECT_MOBILEtc ||
          iType == OBJECT_MOBILEfc ||
          iType == OBJECT_MOBILEic ||
          iType == OBJECT_MOBILEwi ||
          iType == OBJECT_MOBILEti ||
          iType == OBJECT_MOBILEfi ||
          iType == OBJECT_MOBILEii ||
          iType == OBJECT_MOBILEws ||
          iType == OBJECT_MOBILEts ||
          iType == OBJECT_MOBILEfs ||
          iType == OBJECT_MOBILEis ||
          iType == OBJECT_MOBILErt ||
          iType == OBJECT_MOBILErc ||
          iType == OBJECT_MOBILErr ||
          iType == OBJECT_MOBILErs ||
          iType == OBJECT_MOBILEsa ||
          iType == OBJECT_MOBILEwt ||
          iType == OBJECT_MOBILEtt ||
          iType == OBJECT_MOBILEft ||
          iType == OBJECT_MOBILEit ||
          iType == OBJECT_MOBILEdr ||
          iType == OBJECT_APOLLO2  ) )  // vehicle?
    {
        if ( oType == OBJECT_DERRICK  ||
             oType == OBJECT_FACTORY  ||
             oType == OBJECT_STATION  ||
             oType == OBJECT_CONVERT  ||
             oType == OBJECT_REPAIR   ||
             oType == OBJECT_DESTROYER||
             oType == OBJECT_TOWER    ||
             oType == OBJECT_RESEARCH ||
             oType == OBJECT_RADAR    ||
             oType == OBJECT_INFO     ||
             oType == OBJECT_ENERGY   ||
             oType == OBJECT_LABO     ||
             oType == OBJECT_NUCLEAR  ||
             oType == OBJECT_PARA     ||
             oType == OBJECT_SAFE     ||
             oType == OBJECT_HUSTON   )  // building?
        {
            force /= 200.0f;
        }
        else
        if ( oType == OBJECT_MOTHER ||
             oType == OBJECT_ANT    ||
             oType == OBJECT_SPIDER ||
             oType == OBJECT_BEE    ||
             oType == OBJECT_WORM   )  // insect?
        {
            force /= 400.0f;
        }
        else
        if ( oType == OBJECT_FRET  ||
             oType == OBJECT_STONE ||
             oType == OBJECT_METAL )
        {
            force /= 500.0f;
        }
        else
        if ( oType == OBJECT_URANIUM ||
             oType == OBJECT_POWER   ||
             oType == OBJECT_ATOMIC  )
        {
            force /= 100.0f;
        }
        else
        {
            force /= 200.0f;
        }

        if ( m_object->ExploObject(EXPLO_BOUM, force) )  return 2;
    }

    return 1;
}



// Makes the particles evolve.

void CPhysics::FrameParticule(float aTime, float rTime)
{
    D3DVECTOR   pos;
    CObject*    power;
    float       energy, intensity;
    int         effectLight;
    BOOL        bFlash;

    m_restBreakParticule -= rTime;
    if ( aTime-m_lastPowerParticule < m_engine->ParticuleAdapt(0.05f) )  return;
    m_lastPowerParticule = aTime;

    bFlash = FALSE;

    energy = 0.0f;
    power = m_object->RetPower();
    if ( power != 0 )
    {
        energy = power->RetEnergy();
    }

    if ( energy != m_lastEnergy )  // change the energy level?
    {
        if ( energy > m_lastEnergy )  // recharge?
        {
            PowerParticule(1.0f, FALSE);
            bFlash = TRUE;
        }

        if ( energy == 0.0f || m_lastEnergy == 0.0f )
        {
            m_restBreakParticule = 2.5f;  // particles for 2.5s
        }

        m_lastEnergy = energy;
    }

    if ( m_restBreakParticule > 0.0f )
    {
        PowerParticule(m_restBreakParticule/2.5f, (energy == 0));
        bFlash = TRUE;
    }

    effectLight = m_object->RetEffectLight();
    if ( effectLight != -1 )
    {
        if ( bFlash )
        {
            intensity = 0.0f;
            if ( Rand() < 0.5f )  intensity = 1.0f;
            m_light->SetLightIntensity(effectLight, intensity);
            m_light->SetLightIntensitySpeed(effectLight, 10000.0f);
        }
        else
        {
            m_light->SetLightIntensity(effectLight, 0.0f);
        }
    }
}

// Generates some particles after a recharge.

void CPhysics::PowerParticule(float factor, BOOL bBreak)
{
    Character*  character;
    CObject*    fret;
    D3DMATRIX*  mat;
    D3DVECTOR   pos, ppos, eye, speed;
    FPOINT      dim;
    BOOL        bCarryPower;

    bCarryPower = FALSE;
    fret = m_object->RetFret();
    if ( fret != 0 && fret->RetType() == OBJECT_POWER &&
         m_object->RetAngleZ(1) == ARM_STOCK_ANGLE1 )
    {
        bCarryPower = TRUE;  // carries a battery
    }

    mat = m_object->RetWorldMatrix(0);
    character = m_object->RetCharacter();

    pos = character->posPower;
    pos.x -= 0.3f;
    pos.y += 1.0f;  // battery center position
    pos = Transform(*mat, pos);

    speed.x = (Rand()-0.5f)*12.0f;
    speed.y = (Rand()-0.5f)*12.0f;
    speed.z = (Rand()-0.5f)*12.0f;

    ppos.x = pos.x;
    ppos.y = pos.y+(Rand()-0.5f)*2.0f;
    ppos.z = pos.z;

    dim.x = 1.0f*factor;
    dim.y = 1.0f*factor;

    m_particule->CreateParticule(ppos, speed, dim, PARTIBLITZ, 0.5f, 0.0f, 0.0f);

    if ( bCarryPower )  // carry a battery?
    {
        pos = D3DVECTOR(3.0f, 5.6f, 0.0f);  // position of battery holder
        pos = Transform(*mat, pos);

        speed.x = (Rand()-0.5f)*12.0f;
        speed.y = (Rand()-0.5f)*12.0f;
        speed.z = (Rand()-0.5f)*12.0f;

        ppos.x = pos.x;
        ppos.y = pos.y;
        ppos.z = pos.z+(Rand()-0.5f)*2.0f;

        dim.x = 1.0f*factor;
        dim.y = 1.0f*factor;

        m_particule->CreateParticule(ppos, speed, dim, PARTIBLITZ, 0.5f, 0.0f, 0.0f);
    }
}

// Generates some particles after a fall.
// crash: 0=super soft, 1=big crash

void CPhysics::CrashParticule(float crash)
{
    D3DVECTOR   pos, ppos, speed;
    FPOINT      dim;
    float       len;
    int         i, max;

    if ( crash < 0.2f )  return;

    pos = m_object->RetPosition(0);
    m_camera->StartEffect(CE_CRASH, pos, crash);

//? max = (int)(crash*50.0f);
    max = (int)(crash*10.0f*m_engine->RetParticuleDensity());

    for ( i=0 ; i<max ; i++ )
    {
        ppos.x = pos.x + (Rand()-0.5f)*15.0f*crash;
        ppos.z = pos.z + (Rand()-0.5f)*15.0f*crash;
        ppos.y = pos.y + Rand()*4.0f;
        len = 1.0f-(Length(ppos, pos)/(15.0f+5.0f));
        if ( len <= 0.0f )  continue;
        speed.x = (ppos.x-pos.x)*0.1f;
        speed.z = (ppos.z-pos.z)*0.1f;
        speed.y = -2.0f;
        dim.x = 2.0f+crash*5.0f*len;
        dim.y = dim.x;
        m_particule->CreateParticule(ppos, speed, dim, PARTICRASH, 2.0f);
    }
}

// Generates some exhaust gas particle.

void CPhysics::MotorParticule(float aTime, float rTime)
{
    D3DMATRIX*  mat;
    D3DVECTOR   pos, speed;
    FPOINT      dim;
    ObjectType  type;
    FPOINT      c, p;
    float       h, a, delay, level;
    int         r, i, nb;

    if ( m_object->RetToy() )  return;

    type = m_object->RetType();

    if ( type == OBJECT_MOBILEia ||
         type == OBJECT_MOBILEic ||
         type == OBJECT_MOBILEii ||
         type == OBJECT_MOBILEis ||  // legs?
         type == OBJECT_MOBILEdr ||
         type == OBJECT_MOTHER   ||
         type == OBJECT_ANT      ||
         type == OBJECT_SPIDER   ||
         type == OBJECT_BEE      ||
         type == OBJECT_WORM     ||
         type == OBJECT_APOLLO2  )  return;

    if ( type == OBJECT_HUMAN )  delay = 3.0f;
    else                         delay = 8.0f;
    if ( m_bSwim && m_timeUnderWater < delay )  // bubbles when entering water?
    {
        if ( aTime-m_lastUnderParticule >= m_engine->ParticuleAdapt(0.05f) )
        {
            m_lastUnderParticule = aTime;

            nb = (int)(20.0f-(20.0f/delay)*m_timeUnderWater);
            for ( i=0 ; i<nb ; i++ )
            {
                pos = m_object->RetPosition(0);
                pos.x += (Rand()-0.5f)*4.0f;
                pos.y += (Rand()-0.5f)*4.0f;
                pos.z += (Rand()-0.5f)*4.0f;
                speed.y = (Rand()-0.5f)*8.0f+8.0f;
                speed.x = (Rand()-0.5f)*0.2f;
                speed.z = (Rand()-0.5f)*0.2f;
                dim.x = 0.06f+Rand()*0.10f;
                dim.y = dim.x;
                m_particule->CreateParticule(pos, speed, dim, PARTIBUBBLE, 3.0f, 0.0f, 0.0f);
            }
        }
    }

    level = m_water->RetLevel();
    pos = m_object->RetPosition(0);
    if ( type == OBJECT_HUMAN )  pos.y -= 2.0f;
    if ( pos.y < level )  // underwater?
    {
        m_absorbWater += rTime*(1.0f/2.0f);  // gets wet
        if ( m_absorbWater > 1.0f )  m_absorbWater = 1.0f;
    }
    else    // out of water?
    {
        m_absorbWater -= rTime*(1.0f/3.0f);  // to dry
        if ( m_absorbWater < 0.0f )  m_absorbWater = 0.0f;
    }

    if ( pos.y >= level       &&
         m_absorbWater > 0.0f &&
         !m_water->RetLava()  )  // drops on leaving the water?
    {
        if ( aTime-m_lastUnderParticule >= m_engine->ParticuleAdapt(0.05f) )
        {
            m_lastUnderParticule = aTime;

            nb = (int)(8.0f*m_absorbWater);
            for ( i=0 ; i<nb ; i++ )
            {
                pos = m_object->RetPosition(0);
                if ( type == OBJECT_HUMAN )  pos.y -= Rand()*2.0f;
                else                         pos.y += Rand()*2.0f;
                pos.x += (Rand()-0.5f)*2.0f;
                pos.z += (Rand()-0.5f)*2.0f;
                speed.y = -((Rand()-0.5f)*8.0f+8.0f);
                speed.x = 0.0f;
                speed.z = 0.0f;
                dim.x = 0.2f;
                dim.y = 0.2f;
                m_particule->CreateParticule(pos, speed, dim, PARTIWATER, 2.0f, 0.0f, 1.0f);
            }
        }
    }

    if ( type == OBJECT_HUMAN ||  // human?
         type == OBJECT_TECH  )
    {
        if ( m_bLand &&
             aTime-m_lastSlideParticule >= m_engine->ParticuleAdapt(0.05f) )
        {
            h = Max(Abs(m_linMotion.terrainSpeed.x),
                    Abs(m_linMotion.terrainSpeed.z));
            if ( h > m_linMotion.terrainSlide.x+0.5f &&
                 m_linMotion.motorSpeed.x == 0.0f )  // slides a stop?
            {
                m_lastSlideParticule = aTime;

                mat = m_object->RetWorldMatrix(0);
                pos.x = (Rand()-0.5f)*1.0f;
                pos.y = -m_object->RetCharacter()->height;
                pos.z = Rand()*0.4f+1.0f;
                if ( rand()%2 == 0 )  pos.z = -pos.z;
                pos = Transform(*mat, pos);
                speed = D3DVECTOR(0.0f, 1.0f, 0.0f);
                dim.x = Rand()*(h-5.0f)/2.0f+1.0f;
                if ( dim.x > 2.5f )  dim.x = 2.5f;
                dim.y = dim.x;
                m_particule->CreateParticule(pos, speed, dim, PARTICRASH, 2.0f, 0.0f, 0.2f);
            }
        }
    }

    if ( type == OBJECT_MOBILEta ||
         type == OBJECT_MOBILEtc ||
         type == OBJECT_MOBILEti ||
         type == OBJECT_MOBILEts )  // caterpillars?
    {
        if ( aTime-m_lastSlideParticule >= m_engine->ParticuleAdapt(0.05f) )
        {
            h = Abs(m_linMotion.motorSpeed.x-m_linMotion.realSpeed.x);
            if ( h > 5.0f )
            {
                m_lastSlideParticule = aTime;

                mat = m_object->RetWorldMatrix(0);
                pos.x = (Rand()-0.5f)*8.0f;
                pos.y = 0.0f;
                pos.z = Rand()*2.0f+3.0f;
                if ( rand()%2 == 0 )  pos.z = -pos.z;
                pos = Transform(*mat, pos);
                speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
                dim.x = Rand()*(h-5.0f)/2.0f+1.0f;
                if ( dim.x > 3.0f )  dim.x = 3.0f;
                dim.y = dim.x;
                m_particule->CreateParticule(pos, speed, dim, PARTICRASH, 2.0f, 0.0f, 0.2f);
            }
        }
    }

    if ( type == OBJECT_MOBILErt ||
         type == OBJECT_MOBILErc ||
         type == OBJECT_MOBILErr ||
         type == OBJECT_MOBILErs )  // large caterpillars?
    {
        if ( aTime-m_lastSlideParticule >= m_engine->ParticuleAdapt(0.05f) )
        {
            h = Abs(m_linMotion.motorSpeed.x-m_linMotion.realSpeed.x);
            if ( h > 5.0f )
            {
                m_lastSlideParticule = aTime;

                mat = m_object->RetWorldMatrix(0);
                pos.x = (Rand()-0.5f)*9.0f;
                pos.y = 0.0f;
                pos.z = Rand()*3.0f+3.0f;
                if ( rand()%2 == 0 )  pos.z = -pos.z;
                pos = Transform(*mat, pos);
                speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
                dim.x = Rand()*(h-5.0f)/2.0f+1.0f;
                if ( dim.x > 3.0f )  dim.x = 3.0f;
                dim.y = dim.x;
                m_particule->CreateParticule(pos, speed, dim, PARTICRASH, 2.0f, 0.0f, 0.2f);
            }
        }
    }

    if ( (type == OBJECT_HUMAN || type == OBJECT_TECH) && !m_bSwim )
    {
        if ( m_bLand )  // on the ground?
        {
            if ( m_reactorTemperature > 0.0f )
            {
                m_reactorTemperature -= rTime*(1.0f/10.0f);  // cooling
                if ( m_reactorTemperature < 0.0f )
                {
                    m_reactorTemperature = 0.0f;
                }
            }

            if ( m_reactorTemperature == 0.0f ||
                 aTime-m_lastMotorParticule < m_engine->ParticuleAdapt(0.05f) )  return;
            m_lastMotorParticule = aTime;

            pos = D3DVECTOR(-1.6f, -0.5f, 0.0f);
            mat = m_object->RetWorldMatrix(0);
            pos = Transform(*mat, pos);

            speed.x = (Rand()-0.5f)*0.6f;
            speed.z = (Rand()-0.5f)*0.6f;
            speed.y = -(0.5f+Rand()*0.3f)*(1.0f-m_reactorTemperature);

            dim.x = (1.0f+Rand()*0.5f)*(0.2f+m_reactorTemperature*0.8f);
            dim.y = dim.x;

            m_particule->CreateParticule(pos, speed, dim, PARTISMOKE2, 3.0f, 0.0f, 0.1f);
        }
        else    // in flight?
        {
            if ( !m_bMotor || m_reactorRange == 0.0f )  return;

            if ( m_reactorTemperature < 1.0f )  // not too hot?
            {
                m_reactorTemperature += rTime*(1.0f/4.0f);  // heating
                if ( m_reactorTemperature > 1.0f )
                {
                    m_reactorTemperature = 1.0f;  // but not too much
                }
            }

            if ( aTime-m_lastMotorParticule < m_engine->ParticuleAdapt(0.02f) )  return;
            m_lastMotorParticule = aTime;

            pos = D3DVECTOR(-1.6f, -1.0f, 0.0f);
            pos.x += (Rand()-0.5f)*3.0f;
            pos.y += (Rand()-0.5f)*1.5f;
            pos.z += (Rand()-0.5f)*3.0f;
            mat = m_object->RetWorldMatrix(0);
            pos = Transform(*mat, pos);

            h = m_floorHeight;
            if ( h > 10.0f )  // high enough?
            {
                speed = D3DVECTOR(0.0f, -10.0f, 0.0f);  // against the bottom
            }
            else
            {
                speed.y = 10.0f-2.0f*h - Rand()*(10.0f-h);  //against the top
                speed.x = (Rand()-0.5f)*(5.0f-h)*1.0f;  // horizontal (xz)
                speed.z = (Rand()-0.5f)*(5.0f-h)*1.0f;
            }

            dim.x = 0.12f;
            dim.y = 0.12f;

            m_particule->CreateParticule(pos, speed, dim, PARTISCRAPS, 2.0f, 10.0f);

#if 1
            pos = D3DVECTOR(-1.6f, -0.5f, 0.0f);
            pos = Transform(*mat, pos);

            speed.x = (Rand()-0.5f)*1.0f;
            speed.z = (Rand()-0.5f)*1.0f;
            speed.y = -(4.0f+Rand()*3.0f);
            speed.x += m_linMotion.realSpeed.x*0.8f;
            speed.z -= m_linMotion.realSpeed.x*m_cirMotion.realSpeed.y*0.05f;
            if ( m_linMotion.realSpeed.y > 0.0f )
            {
                speed.y += m_linMotion.realSpeed.y*0.5f;
            }
            else
            {
                speed.y += m_linMotion.realSpeed.y*1.2f;
            }
            a = m_object->RetAngleY(0);
            p.x = speed.x;
            p.y = speed.z;
            p = RotatePoint(-a, p);
            speed.x = p.x;
            speed.z = p.y;

            dim.x = 0.4f+Rand()*0.2f;
            dim.y = dim.x;

            m_particule->CreateParticule(pos, speed, dim, PARTIEJECT, 0.3f, 10.0f);
#endif
        }
    }

    if ( (type == OBJECT_HUMAN || type == OBJECT_TECH) && m_bSwim )
    {
        m_reactorTemperature = 0.0f;  // reactor cold
    }

    if ( m_type == TYPE_FLYING &&
         type != OBJECT_HUMAN &&
         type != OBJECT_TECH  &&
         !m_bSwim )
    {
        if ( m_bLand )  // on the ground?
        {
            if ( m_motorSpeed.x == 0.0f &&  // glide slope due to ground?
                 m_cirMotion.realSpeed.y == 0.0f )
            {
                h = Max(Abs(m_linMotion.realSpeed.x),
                        Abs(m_linMotion.realSpeed.z));

                if ( h < 3.0f )  return;

                if ( aTime-m_lastMotorParticule < m_engine->ParticuleAdapt(0.2f) )  return;
                m_lastMotorParticule = aTime;

                r = rand()%3;
                if ( r == 0 )  pos = D3DVECTOR(-3.0f, 0.0f, -4.0f);
                if ( r == 1 )  pos = D3DVECTOR(-3.0f, 0.0f,  4.0f);
                if ( r == 2 )  pos = D3DVECTOR( 4.0f, 0.0f,  0.0f);

                pos.x += (Rand()-0.5f)*2.0f;
                pos.z += (Rand()-0.5f)*2.0f;
                mat = m_object->RetWorldMatrix(0);
                pos = Transform(*mat, pos);
                speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
                dim.x = Rand()*h/5.0f+2.0f;
                dim.y = dim.x;
                m_particule->CreateParticule(pos, speed, dim, PARTICRASH, 2.0f);
            }
            else    // glide with small reactors in skates?
            {
                if ( m_linMotion.realSpeed.x == 0.0f &&
                     m_cirMotion.realSpeed.y == 0.0f )  return;

                if ( aTime-m_lastMotorParticule < m_engine->ParticuleAdapt(0.02f) )  return;
                m_lastMotorParticule = aTime;

                r = rand()%3;
                if ( r == 0 )  pos = D3DVECTOR(-3.0f, 0.0f, -4.0f);
                if ( r == 1 )  pos = D3DVECTOR(-3.0f, 0.0f,  4.0f);
                if ( r == 2 )  pos = D3DVECTOR( 4.0f, 0.0f,  0.0f);

                pos.x += (Rand()-0.5f)*1.0f;
                pos.z += (Rand()-0.5f)*1.0f;
                mat = m_object->RetWorldMatrix(0);
                pos = Transform(*mat, pos);
                speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
                dim.x = 1.0f;
                dim.y = dim.x;
                m_particule->CreateParticule(pos, speed, dim, PARTIEJECT);
            }
        }
        else    // in flight?
        {
            if ( !m_bMotor || m_reactorRange == 0.0f )  return;

            if ( aTime-m_lastMotorParticule < m_engine->ParticuleAdapt(0.02f) )  return;
            m_lastMotorParticule = aTime;

            pos = D3DVECTOR(0.0f, -1.0f, 0.0f);
            pos.x += (Rand()-0.5f)*6.0f;
            pos.y += (Rand()-0.5f)*3.0f;
            pos.z += (Rand()-0.5f)*6.0f;
            mat = m_object->RetWorldMatrix(0);
            pos = Transform(*mat, pos);

            h = m_floorHeight;
            if ( h > 10.0f )  // high enough?
            {
                speed = D3DVECTOR(0.0f, -10.0f, 0.0f);  // against the bottom
            }
            else
            {
                speed.y = 10.0f-2.0f*h - Rand()*(10.0f-h);  // against the top
                speed.x = (Rand()-0.5f)*(10.0f-h)*2.0f;  // horizontal (xz)
                speed.z = (Rand()-0.5f)*(10.0f-h)*2.0f;
            }

            dim.x = 0.2f;
            dim.y = 0.2f;

            m_particule->CreateParticule(pos, speed, dim, PARTISCRAPS, 2.0f, 10.0f);

#if 1
            pos = D3DVECTOR(0.0f, 1.0f, 0.0f);
            pos = Transform(*mat, pos);

            speed.x = (Rand()-0.5f)*1.0f;
            speed.z = (Rand()-0.5f)*1.0f;
            speed.y = -(6.0f+Rand()*4.5f);
            speed.x += m_linMotion.realSpeed.x*0.8f;
            speed.z -= m_linMotion.realSpeed.x*m_cirMotion.realSpeed.y*0.05f;
            if ( m_linMotion.realSpeed.y > 0.0f )
            {
                speed.y += m_linMotion.realSpeed.y*0.5f;
            }
            else
            {
                speed.y += m_linMotion.realSpeed.y*1.2f;
            }
            a = m_object->RetAngleY(0);
            p.x = speed.x;
            p.y = speed.z;
            p = RotatePoint(-a, p);
            speed.x = p.x;
            speed.z = p.y;

            dim.x = 0.7f+Rand()*0.6f;
            dim.y = dim.x;

            m_particule->CreateParticule(pos, speed, dim, PARTIEJECT, 0.5f, 10.0f);
#endif
        }
    }

    if ( (type == OBJECT_HUMAN || type == OBJECT_TECH) && m_bSwim )
    {
        if ( !m_object->RetDead() )
        {
            h = Mod(aTime, 5.0f);
            if ( h < 3.5f && ( h < 1.5f || h > 1.6f ) )  return;
        }
        if ( aTime-m_lastMotorParticule < m_engine->ParticuleAdapt(0.06f) )  return;
        m_lastMotorParticule = aTime;

        pos = D3DVECTOR(0.0f, 3.0f, 0.0f);
        mat = m_object->RetWorldMatrix(0);
        pos = Transform(*mat, pos);
        pos.x += (Rand()-0.5f)*1.0f;
        pos.z += (Rand()-0.5f)*1.0f;
        speed.y = (Rand()-0.5f)*8.0f+8.0f;
        speed.x = (Rand()-0.5f)*0.2f;
        speed.z = (Rand()-0.5f)*0.2f;
        dim.x = 0.2f;
        dim.y = 0.2f;
        m_particule->CreateParticule(pos, speed, dim, PARTIBUBBLE, 3.0f, 0.0f, 0.0f);

        if ( aTime-m_lastSoundWater > 1.5f )
        {
            m_lastSoundWater = aTime;
            m_sound->Play(SOUND_BLUP, m_object->RetPosition(0), 0.5f+Rand()*0.5f);
        }
    }

    if ( type == OBJECT_MOBILEsa && m_bSwim )
    {
        h = Mod(aTime, 3.0f);
        if ( h < 1.5f && ( h < 0.5f || h > 0.9f ) )  return;
        if ( aTime-m_lastMotorParticule < m_engine->ParticuleAdapt(0.06f) )  return;
        m_lastMotorParticule = aTime;

        pos = D3DVECTOR(0.0f, 3.0f, 0.0f);
        mat = m_object->RetWorldMatrix(0);
        pos = Transform(*mat, pos);
        pos.x += (Rand()-0.5f)*1.0f;
        pos.z += (Rand()-0.5f)*1.0f;
        speed.y = (Rand()-0.5f)*8.0f+8.0f;
        speed.x = (Rand()-0.5f)*0.2f;
        speed.z = (Rand()-0.5f)*0.2f;
        dim.x = 0.2f;
        dim.y = 0.2f;
        m_particule->CreateParticule(pos, speed, dim, PARTIBUBBLE, 3.0f, 0.0f, 0.0f);

        if ( aTime-m_lastSoundWater > 1.5f )
        {
            m_lastSoundWater = aTime;
            m_sound->Play(SOUND_BLUP, m_object->RetPosition(0), 0.5f+Rand()*0.5f);
        }
    }

    if ( m_type == TYPE_ROLLING )
    {
        if ( type == OBJECT_APOLLO2 )  return;  // electric motors!

        if ( type == OBJECT_MOBILErt ||
             type == OBJECT_MOBILErc ||
             type == OBJECT_MOBILErr ||
             type == OBJECT_MOBILErs )
        {
            if ( !m_bMotor )  return;

            if ( aTime-m_lastMotorParticule < m_engine->ParticuleAdapt(0.1f) )  return;
            m_lastMotorParticule = aTime;

            pos = D3DVECTOR(-2.5f, 10.3f, -1.3f);
            pos.x += (Rand()-0.5f)*1.0f;
            pos.z += (Rand()-0.5f)*1.0f;
            mat = m_object->RetWorldMatrix(0);
            pos   = Transform(*mat, pos);

            speed.x = (Rand()-0.5f)*2.0f;
            speed.z = (Rand()-0.5f)*2.0f;
            speed.y = 1.5f+Rand()*1.0f;

            dim.x = Rand()*0.6f+0.4f;
            dim.y = dim.x;

            m_particule->CreateParticule(pos, speed, dim, PARTIMOTOR, 2.0f);
        }
        else
        {
            if ( !m_bMotor )  return;

            if ( aTime-m_lastMotorParticule < m_engine->ParticuleAdapt(0.05f) )  return;
            m_lastMotorParticule = aTime;

            pos = D3DVECTOR(-3.4f, 1.8f, 0.5f);

            speed = pos;
            if ( m_linMotion.currentSpeed.x < 0.0f )
            {
                speed.x += m_linMotion.currentSpeed.x*1.2f;
            }
            else if ( m_linMotion.currentSpeed.x > 0.0f )
            {
                speed.x += 0.0f;
            }
            else
            {
                speed.x -= 3.0f;
            }
            speed.y -= 0.5f+Rand()*2.0f;
            speed.z += (Rand()-0.5f)*3.0f;

            mat = m_object->RetWorldMatrix(0);
            pos   = Transform(*mat, pos);
            speed = Transform(*mat, speed)-pos;

            dim.x = Rand()*0.4f+0.3f;
            dim.y = dim.x;

            m_particule->CreateParticule(pos, speed, dim, PARTIMOTOR, 2.0f);
        }
    }
}

// Generates some particles after falling into the water.

void CPhysics::WaterParticule(float aTime, D3DVECTOR pos, ObjectType type,
                              float floor, float advance, float turn)
{
    D3DVECTOR   ppos, speed;
    FPOINT      dim;
    float       delay, level, min, max, force, volume, diam;
    int         i, nb;

    level = m_water->RetLevel();
    if ( floor >= level )  return;

    if ( type == OBJECT_HUMAN ||
         type == OBJECT_TECH  )
    {
        min = 3.0f;
        max = 3.0f;
    }
    else
    {
        min = 0.0f;
        max = 9.0f;
    }

    if ( pos.y+max < level || pos.y-min > level )  return;

    // Management of the particle "splash".
    if ( m_linMotion.realSpeed.y < -10.0f   &&
         aTime-m_lastPloufParticule >= 1.0f )
    {
        m_lastPloufParticule = aTime;

        force = -m_linMotion.realSpeed.y/20.0f;  // power according to speed drops
        if ( type == OBJECT_HUMAN ||
             type == OBJECT_TECH  )
        {
            diam = 2.5f;
        }
        else
        {
            diam = 5.0f;
            force *= 1.3f;  // a robot is heavier
        }

        pos = m_object->RetPosition(0);
        pos.y = m_water->RetLevel()-1.0f;
        dim.x = 2.0f*force;  // height
        dim.y = diam;  // diameter
        m_particule->CreateParticule(pos, D3DVECTOR(0.0f, 0.0f, 0.0f), dim, PARTIPLOUF0, 1.4f, 0.0f, 0.0f);

        force = (0.5f+force*0.5f);
        nb = (int)(force*50.0f*m_engine->RetParticuleDensity());
        for ( i=0 ; i<nb ; i++ )
        {
            ppos = pos;
            ppos.x += (Rand()-0.5f)*4.0f;
            ppos.z += (Rand()-0.5f)*4.0f;
            ppos.y += 0.6f;
            speed.x = (Rand()-0.5f)*12.0f*force;
            speed.z = (Rand()-0.5f)*12.0f*force;
            speed.y = 6.0f+Rand()*6.0f*force;
            dim.x = 0.5f;
            dim.y = dim.x;
            m_particule->CreateParticule(ppos, speed, dim, PARTIDROP, 2.0f, 20.0f, 0.2f);
        }

        volume = Abs(m_linMotion.realSpeed.y*0.02f);
        if ( volume > 1.0f )  volume = 1.0f;
        m_sound->Play(SOUND_PLOUF, pos, volume);
    }

    // Management particles "cop".
    if ( m_water->RetLava() )  return;

    if ( advance == 0.0f && turn == 0.0f )
    {
        turn = 10.0f;
        delay = 0.50f;
    }
    else if ( advance == 0.0f )
    {
        delay = 0.24f;
    }
    else
    {
        delay = 0.06f;
    }
    m_engine->ParticuleAdapt(delay);

    if ( aTime-m_lastWaterParticule < delay )  return;
    m_lastWaterParticule = aTime;

    force = (advance+turn)*0.16f;
    if ( force < 0.001f )  return;

    pos = m_object->RetPosition(0);
    pos.y = level+0.1f;
    if ( advance == 0 )
    {
        pos.x += (Rand()-0.5f)*10.0f;
        pos.z += (Rand()-0.5f)*10.0f;
    }
    else
    {
        pos.x += (Rand()-0.5f)*4.0f;
        pos.z += (Rand()-0.5f)*4.0f;
    }
    speed.y = 0.0f;
    speed.x = 0.0f;
    speed.z = 0.0f;
    dim.x = Min(Rand()*force+force+1.0f, 10.0f);
    dim.y = dim.x;
    m_particule->CreateParticule(pos, speed, dim, PARTIFLIC, 3.0f, 0.0f, 0.0f);
}

// Creates the trace under the robot.

void CPhysics::WheelParticule(int color, float width)
{
    Character*      character;
    D3DMATRIX*      mat;
    D3DVECTOR       goal1, goal2, wheel1, wheel2;
    ParticuleType   parti;
    float           dist1, dist2, step;

    character = m_object->RetCharacter();
    mat = m_object->RetWorldMatrix(0);

    // Draw a trace on the ground.
    if ( color >= 0 && color <= 17 )
    {
        parti = (ParticuleType)(PARTITRACE0+color);
        step = 2.0f;
        if ( color >= 16 )  step = 4.0f;  // arrow?
        step /= m_engine->RetTracePrecision();

        goal1.x = step/2.0f;
        goal1.y = 0.0f;
        goal1.z = -width/2.0f;
        goal1 = Transform(*mat, goal1);

        goal2.x = step/2.0f;
        goal2.y = 0.0f;
        goal2.z = width/2.0f;
        goal2 = Transform(*mat, goal2);

        if ( !m_bWheelParticuleBrake )
        {
            m_wheelParticulePos[0] = goal1;
            m_wheelParticulePos[1] = goal2;
        }

        while ( TRUE )
        {
            dist1 = Length(m_wheelParticulePos[0], goal1);
            if ( dist1 < step )  break;
            dist2 = Length(m_wheelParticulePos[1], goal2);
            wheel1 = SegmentDist(m_wheelParticulePos[0], goal1, step);
            wheel2 = SegmentDist(m_wheelParticulePos[1], goal2, step*dist2/dist1);
            if ( m_linMotion.realSpeed.x >= 0.0f )
            {
                m_particule->CreateWheelTrace(m_wheelParticulePos[0], m_wheelParticulePos[1], wheel1, wheel2, parti);
            }
            else
            {
                m_particule->CreateWheelTrace(m_wheelParticulePos[1], m_wheelParticulePos[0], wheel2, wheel1, parti);
            }
            m_wheelParticulePos[0] = wheel1;
            m_wheelParticulePos[1] = wheel2;
        }

        m_bWheelParticuleBrake = TRUE;
    }
    else
    {
        m_bWheelParticuleBrake = FALSE;
    }
}


// Creates the interface.

void CPhysics::CreateInterface(BOOL bSelect)
{
    if ( m_brain != 0 )
    {
        m_brain->CreateInterface(bSelect);
    }
}


// Returns an error related to the general state.

Error CPhysics::RetError()
{
    ObjectType  type;
    CObject*    power;

    type = m_object->RetType();
    if ( type == OBJECT_HUMAN    ||
         type == OBJECT_TECH     ||
         type == OBJECT_MOTHER   ||
         type == OBJECT_ANT      ||
         type == OBJECT_SPIDER   ||
         type == OBJECT_BEE      ||
         type == OBJECT_WORM     ||
         type == OBJECT_APOLLO2  ||
         type == OBJECT_MOBILEdr )  return ERR_OK;

    if ( m_brain != 0 && m_brain->RetActiveVirus() )
    {
        return ERR_VEH_VIRUS;
    }

    power = m_object->RetPower();  // searches for the object battery used
    if ( power == 0 )
    {
        return ERR_VEH_POWER;
    }
    else
    {
        if ( power->RetEnergy() == 0.0f )  return ERR_VEH_ENERGY;
    }

    return ERR_OK;
}

