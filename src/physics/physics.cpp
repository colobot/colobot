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


#include "physics/physics.h"

#include "app/app.h"

#include "common/event.h"
#include "common/global.h"
#include "common/make_unique.h"

#include "graphics/engine/camera.h"
#include "graphics/engine/engine.h"
#include "graphics/engine/lightman.h"
#include "graphics/engine/pyro_manager.h"
#include "graphics/engine/terrain.h"
#include "graphics/engine/water.h"

#include "level/robotmain.h"

#include "level/parser/parserline.h"
#include "level/parser/parserparam.h"

#include "math/geometry.h"

#include "object/object_manager.h"
#include "object/old_object.h"

#include "object/interface/carrier_object.h"
#include "object/interface/jostleable_object.h"
#include "object/interface/powered_object.h"
#include "object/interface/transportable_object.h"

#include "object/motion/motion.h"
#include "object/motion/motionhuman.h"
#include "object/motion/motionvehicle.h"

#include "object/subclass/base_alien.h"

#include "object/task/task.h"

#include "sound/sound.h"



const float LANDING_SPEED   = 3.0f;
const float LANDING_ACCEL   = 5.0f;
const float LANDING_ACCELh  = 1.5f;

const float STARTDELAY = 0.6f; // temps de mise en marche du moteur



// Object's constructor.

CPhysics::CPhysics(COldObject* object)
{
    m_object    = object;
    m_main      = CRobotMain::GetInstancePointer();
    m_engine    = Gfx::CEngine::GetInstancePointer();
    m_lightMan  = m_engine->GetLightManager();
    m_particle  = m_engine->GetParticle();
    m_water     = m_engine->GetWater();
    m_terrain   = CRobotMain::GetInstancePointer()->GetTerrain();
    m_camera    = CRobotMain::GetInstancePointer()->GetCamera();
    m_sound     = CApplication::GetInstancePointer()->GetSound();
    m_motion    = nullptr;

    m_type = TYPE_NORMAL;
    m_gravity = 9.81f;  // default gravity
    m_time = 0.0f;
    m_timeUnderWater = 0.0f;
    m_timeWheelBurn = 0.0f;
    m_timeMotorBurn = 0.0f;
    m_timeLock = 0.0f;
    m_motorSpeed = Math::Vector(0.0f, 0.0f, 0.0f);
    m_motorSpeedW = 0.0f;
    m_motorSpeedWk = 0.0f;
    m_forceSpeed = Math::Vector(0.0f, 0.0f, 0.0f);
    m_imprecisionA = Math::Vector(0.0f, 0.0f, 0.0f);
    m_imprecisionH = Math::Vector(0.0f, 0.0f, 0.0f);
    m_imprecisionT = Math::Vector(0.0f, 0.0f, 0.0f);
    m_bMotor = false;
    m_bSilent = false;
    m_bForceSlow = false;
    m_bLand = true;  // ground
    m_bSwim = false;  // in air
    m_bCollision = false;
    m_bObstacle = false;
    m_repeatCollision = 0;
    m_doorRank = 0;
    m_linVibrationFactor = 1.0f;
    m_cirVibrationFactor = 1.0f;
    m_inclinaisonFactor  = 1.0f;
    m_lastPowerParticle = 0.0f;
    m_lastSlideParticle = 0.0f;
    m_lastMotorParticle = 0.0f;
    m_lastWaterParticle = 0.0f;
    m_lastUnderParticle = 0.0f;
    m_lastPloufParticle = 0.0f;
    m_lastFlameParticle = 0.0f;
    m_lastCrashParticle = 0.0f;
    m_lastWheelParticle = 0.0f;
    m_lastSuspParticle  = 0.0f;
    m_bWheelParticleBrake = false;
    m_absorbWater        = 0.0f;
    m_reactorTemperature = 0.0f;
    m_timeReactorFail    = 0.0f;
    m_lastSoundCollision = 0.0f;
    m_lastSoundCrash     = 0.0f;
    m_lastSoundWater     = 0.0f;
    m_lastSoundInsect    = 0.0f;
    m_lastEnergy = 0.0f;
    m_lastSoundWater = 0.0f;
    m_lastSoundInsect = 0.0f;
    m_restBreakParticle = 0.0f;
    m_floorHeight = 0.0f;
    m_soundChannelMotor1 = -1;
    m_soundChannelMotor2 = -1;
    m_soundChannelSlide = -1;
    m_soundChannelBrake = -1;
    m_soundChannelBoost = -1;
    m_soundChannelHorn  = -1;
    m_soundChannelGlide = -1;
    m_soundTimePshhh = 0.0f;
    m_soundTimeJostle = 0.0f;
    m_soundTimeBoum = 0.0f;
    m_soundAmplitudeMotor1 = 0.0f;
    m_soundFrequencyMotor1 = 0.3f;
    m_soundAmplitudeMotor2 = 0.0f;
    m_soundFrequencyMotor2 = 0.3f;
    m_bSoundSlow = true;
    m_bFreeze = false;
    m_bBrake = false;
    m_bHandbrake = false;
    m_bForceUpdate = true;
    m_bLowLevel = false;
    m_bWater = false;
    m_bOldWater = false;
    m_bHornPress = false;
    m_lastDoorCounter = 0;
    m_FFBamplitude = 0.0f;
    m_FFBspeed = 0.0f;
    m_FFBfrequency = 1.0f;
    m_FFBprogress = 0.0f;
    m_FFBforce = 0.0f;
    m_FFBbrake = 0.0f;
    m_fallingHeight = 0.0f;
    m_minFallingHeight = 20.0f;
    m_fallDamageFraction = 0.007f;
    m_floorLevel = 0.0f;

    m_graviSpeed  = 0.0f;
    m_graviGlu    = 0.0f;
    m_centriSpeed = 0.0f;
    m_wheelSlide  = 0.0f;
    m_overTurn    = 0.0f;
    m_overTurnCur = 0.0f;
    m_overBrake   = 0.0f;
    m_chocSpin    = 0.0f;
    m_chocAngle   = 0.0f;
    m_motorAngle  = 0.0f;
    m_motorRPM    = 0.0f;
    m_motorState  = 0;

    m_suspTime[0]    = 0.0f;
    m_suspTime[1]    = 0.0f;
    m_suspHeight[0]  = 0.0f;
    m_suspHeight[1]  = 0.0f;
    m_suspDelayed[0] = 0.0f;
    m_suspDelayed[1] = 0.0f;

    for ( int i=0 ; i<5 ; i++ )
    {
        m_suspEnergy[0][i]  = 0.0f;
        m_suspEnergy[1][i]  = 0.0f;
    }
}

// Object's destructor.

CPhysics::~CPhysics()
{
}


// Destroys the object.

void CPhysics::DeleteObject(bool bAll)
{
    if ( m_soundChannel != -1 )
    {
        m_sound->FlushEnvelope(m_soundChannel);
        m_sound->AddEnvelope(m_soundChannel, 0.0f, 1.0f, 0.3f, SOPER_STOP);
        m_soundChannel = -1;
    }
    if ( m_soundChannelMotor1 != -1 )
    {
        m_sound->FlushEnvelope(m_soundChannelMotor1);
        m_sound->AddEnvelope(m_soundChannelMotor1, 0.0f, 1.0f, 0.3f, SOPER_STOP);
        m_soundChannelMotor1 = -1;
    }
    if ( m_soundChannelMotor2 != -1 )
    {
        m_sound->FlushEnvelope(m_soundChannelMotor2);
        m_sound->AddEnvelope(m_soundChannelMotor2, 0.0f, 1.0f, 0.3f, SOPER_STOP);
        m_soundChannelMotor2 = -1;
    }
    if ( m_soundChannelSlide != -1 )
    {
        m_sound->FlushEnvelope(m_soundChannelSlide);
        m_sound->AddEnvelope(m_soundChannelSlide, 0.0f, 1.0f, 0.3f, SOPER_STOP);
        m_soundChannelSlide = -1;
    }
    if ( m_soundChannelBrake != -1 )
    {
        m_sound->FlushEnvelope(m_soundChannelBrake);
        m_sound->AddEnvelope(m_soundChannelBrake, 0.0f, 1.0f, 0.3f, SOPER_STOP);
        m_soundChannelBrake = -1;
    }
    if ( m_soundChannelBoost != -1 )
    {
        m_sound->FlushEnvelope(m_soundChannelBoost);
        m_sound->AddEnvelope(m_soundChannelBoost, 0.0f, 1.0f, 0.3f, SOPER_STOP);
        m_soundChannelBoost = -1;
    }
    if ( m_soundChannelHorn != -1 )
    {
        m_sound->FlushEnvelope(m_soundChannelHorn);
        m_sound->AddEnvelope(m_soundChannelHorn, 0.0f, 1.0f, 0.3f, SOPER_STOP);
        m_soundChannelHorn = -1;
    }
    if ( m_soundChannelGlide != -1 )
    {
        m_sound->FlushEnvelope(m_soundChannelGlide);
        m_sound->AddEnvelope(m_soundChannelGlide, 0.0f, 1.0f, 0.3f, SOPER_STOP);
        m_soundChannelGlide = -1;
    }
}




void CPhysics::SetMotion(CMotion* motion)
{
    m_motion = motion;
}

void CPhysics::SetType(PhysicsType type)
{
    m_type = type;
}

PhysicsType CPhysics::GetType()
{
    return m_type;
}


// Saves all parameters of the object.

bool CPhysics::Write(CLevelParserLine* line)
{
    line->AddParam("motor", MakeUnique<CLevelParserParam>(m_motorSpeed));

    if ( m_object->Implements(ObjectInterfaceType::Flying) )
    {
        if ( m_object->Implements(ObjectInterfaceType::JetFlying) )
        {
            line->AddParam("reactorRange", MakeUnique<CLevelParserParam>(m_object->GetReactorRange()));
        }
        line->AddParam("land", MakeUnique<CLevelParserParam>(GetLand()));
    }

    return true;
}

// Restores all parameters of the object.

bool CPhysics::Read(CLevelParserLine* line)
{
    m_motorSpeed = line->GetParam("motor")->AsPoint(Math::Vector(0.0f, 0.0f, 0.0f));

    if ( m_object->Implements(ObjectInterfaceType::Flying) )
    {
        if ( m_object->Implements(ObjectInterfaceType::JetFlying) )
        {
            m_object->SetReactorRange(line->GetParam("reactorRange")->AsFloat(1.0f));
        }
        SetLand(line->GetParam("land")->AsBool(true));
    }

    return true;
}



// Management of the force of gravity.

void CPhysics::SetGravity(float value)
{
    m_gravity = value;
}

float CPhysics::GetGravity()
{
    return m_gravity;
}


// Returns the height above the ground.

float CPhysics::GetFloorHeight()
{
    return m_floorHeight;
}


// Managing the state of the engine.

void CPhysics::SetMotor(bool bState)
{
    int         light;

    m_bMotor = bState;

    light = m_object->GetShadowLight();
    if ( light != -1 )
    {
        m_lightMan->SetLightIntensity(light, m_bMotor?1.0f:0.0f);
        m_lightMan->SetLightIntensitySpeed(light, 3.0f);
    }
}

bool CPhysics::GetMotor()
{
    return m_bMotor;
}

void CPhysics::SetSilent(bool bState)
{
    m_bSilent = bState;
}

bool CPhysics::GetSilent()
{
    return m_bSilent;
}

void CPhysics::SetForceSlow(bool bState)
{
    m_bForceSlow = bState;
    m_timeLock = 0.0f;
}

bool CPhysics::GetForceSlow()
{
    return m_bForceSlow;
}


// Management of the state in flight/ground.

void CPhysics::SetLand(bool bState)
{
    m_bLand = bState;
    SetMotor(!bState);  // lights if you leave the reactor in flight
}

bool CPhysics::GetLand()
{
    return m_bLand;
}


// Management of the state in air/water.

void CPhysics::SetSwim(bool bState)
{
    if ( !m_bSwim && bState )  // enters the water?
    {
        m_timeUnderWater = 0.0f;
    }
    m_bSwim = bState;
}

bool CPhysics::GetSwim()
{
    return m_bSwim;
}


// Indicates whether a collision occurred.

void CPhysics::SetCollision(bool bCollision)
{
    m_bCollision = bCollision;
}

bool CPhysics::GetCollision()
{
    return m_bCollision;
}


// Indicates whether the influence of soil is activated or not.

void CPhysics::SetFreeze(bool bFreeze)
{
    m_bFreeze = bFreeze;
}

bool CPhysics::GetFreeze()
{
    return m_bFreeze;
}

// Indique si les phares sont allumés.

bool CPhysics::GetLight(int rank)
{
    if ( m_object->IsDying() )  return false;
    if ( !m_motion->GetLight(rank) )  return false;

    if ( rank == 0 || rank == 1 )  // phares avant ?
    {
        return true;
    }
    if ( rank == 2 || rank == 3 )  // feu de stop ?
    {
        return m_bBrake;
    }
    if ( rank == 4 || rank == 5 )  // feu de recule ?
    {
        return (m_motorSpeed.x < 0.0f && !m_bBrake);
    }

    return false;
}

// Specifies the engine speed.
// x = forward/backward
// y = up/down
// z = turn

void CPhysics::SetMotorSpeed(Math::Vector speed)
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

// Spécifie la force du frain ŕ main.
//  0 = relâché
// +1 = tiré au maximun

void CPhysics::SetMotorSpeedW(float speed)
{
    m_motorSpeedW = speed;
}

Math::Vector CPhysics::GetMotorSpeed()
{
    return m_motorSpeed;
}

float CPhysics::GetMotorSpeedX()
{
    return m_motorSpeed.x;
}

float CPhysics::GetMotorSpeedY()
{
    return m_motorSpeed.y;
}

float CPhysics::GetMotorSpeedZ()
{
    return m_motorSpeed.z;
}

float CPhysics::GetMotorSpeedW()
{
    return m_motorSpeedW;
}

void CPhysics::ForceMotorSpeedX(float speed)
{
    m_forceSpeed.x = speed;
}

void CPhysics::ForceMotorSpeedY(float speed)
{
    m_forceSpeed.y = speed;
}

void CPhysics::ForceMotorSpeedZ(float speed)
{
    m_forceSpeed.z = speed;
}

// Retourne la valeur du compte tour (0..2).

float CPhysics::GetMotorRPM()
{
    return m_motorRPM;
}

// Retourne le rapport de la boîte de vitesses.

int CPhysics::GetMotorState()
{
    return m_motorState;
}


// Management of linear and angular velocities.
// Specifies the speed parallel to the direction of travel.

void CPhysics::SetLinMotion(PhysicsMode mode, Math::Vector value)
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
    if ( mode == MO_REAACCEL )  m_linMotion.realAccel     = value;
}

Math::Vector CPhysics::GetLinMotion(PhysicsMode mode)
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
    if ( mode == MO_REAACCEL )  return m_linMotion.realAccel;
    assert(false);
    return Math::Vector(0.0f, 0.0f, 0.0f);
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
    if ( mode == MO_REAACCEL )  m_linMotion.realAccel.x     = value;
}

float CPhysics::GetLinMotionX(PhysicsMode mode)
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
    if ( mode == MO_REAACCEL )  return m_linMotion.realAccel.x;
    assert(false);
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
    if ( mode == MO_REAACCEL )  m_linMotion.realAccel.y     = value;
}

float CPhysics::GetLinMotionY(PhysicsMode mode)
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
    if ( mode == MO_REAACCEL )  return m_linMotion.realAccel.y;
    assert(false);
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
    if ( mode == MO_REAACCEL )  m_linMotion.realAccel.z     = value;
}

float CPhysics::GetLinMotionZ(PhysicsMode mode)
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
    if ( mode == MO_REAACCEL )  return m_linMotion.realAccel.z;
    assert(false);
    return 0.0f;
}

// Specifies the rotation around the axis of walk.

void CPhysics::SetCirMotion(PhysicsMode mode, Math::Vector value)
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
    if ( mode == MO_REAACCEL )  m_cirMotion.realAccel     = value;
}

Math::Vector CPhysics::GetCirMotion(PhysicsMode mode)
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
    if ( mode == MO_REAACCEL )  return m_cirMotion.realAccel;
    assert(false);
    return Math::Vector(0.0f, 0.0f, 0.0f);
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
    if ( mode == MO_REAACCEL )  m_cirMotion.realAccel.x     = value;
}

float CPhysics::GetCirMotionX(PhysicsMode mode)
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
    if ( mode == MO_REAACCEL )  return m_cirMotion.realAccel.x;
    assert(false);
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
    if ( mode == MO_REAACCEL )  m_cirMotion.realAccel.y     = value;
}

float CPhysics::GetCirMotionY(PhysicsMode mode)
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
    if ( mode == MO_REAACCEL )  return m_cirMotion.realAccel.y;
    assert(false);
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
    if ( mode == MO_REAACCEL )  m_cirMotion.realAccel.z     = value;
}

float CPhysics::GetCirMotionZ(PhysicsMode mode)
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
    if ( mode == MO_REAACCEL )  return m_cirMotion.realAccel.z;
    assert(false);
    return 0.0f;
}


// Returns the linear distance braking.
//
//           v*v
//  d = -----
//       2a

float CPhysics::GetLinStopLength(PhysicsMode sMode, PhysicsMode aMode)
{
    float       speed, accel;

    speed = GetLinMotionX(sMode);  // MO_ADVSPEED/MO_RECSPEED
    accel = GetLinMotionX(aMode);  // MO_ADVACCEL/MO_RECACCEL/MO_STOACCEL

    if ( m_object->Implements(ObjectInterfaceType::Flying) && m_bLand )  // flying on the ground?
    {
        speed /= LANDING_SPEED;
        accel *= LANDING_ACCEL;
    }

    return (speed*speed) / (accel*2.0f);
}

// Returns the angle of circular braking.

float CPhysics::GetCirStopLength()
{
    return m_cirMotion.advanceSpeed.y * m_cirMotion.advanceSpeed.y /
           m_cirMotion.stopAccel.y / 2.0f;
}

// Returns the length advanced into a second, on the ground, maximum speed.

float CPhysics::GetLinMaxLength(float dir)
{
    float       dist;

    if ( dir > 0.0f )  dist = m_linMotion.advanceSpeed.x;
    else               dist = m_linMotion.recedeSpeed.x;

    if ( m_object->Implements(ObjectInterfaceType::Flying) )
    {
        dist /= 5.0f;
    }

    return dist;
}

// Returns the time needed to travel some distance.

float CPhysics::GetLinTimeLength(float dist, float dir)
{
    float       accel, decel, dps;

    if ( dir > 0.0f )
    {
        accel = GetLinStopLength(MO_ADVSPEED, MO_ADVACCEL);
        decel = GetLinStopLength(MO_ADVSPEED, MO_STOACCEL);
    }
    else
    {
        accel = GetLinStopLength(MO_RECSPEED, MO_RECACCEL);
        decel = GetLinStopLength(MO_RECSPEED, MO_STOACCEL);
    }

    dps = GetLinMaxLength(dir);

    return (dist+accel+decel)/dps;
}

// Returns the length for a forward travel some distance, taking into account the accelerations / decelerations.

float CPhysics::GetLinLength(float dist)
{
    float   accDist, desDist;

    if ( dist > 0.0f )
    {
        accDist = GetLinStopLength(MO_ADVSPEED, MO_ADVACCEL);
        desDist = GetLinStopLength(MO_ADVSPEED, MO_STOACCEL);

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
        accDist = GetLinStopLength(MO_RECSPEED, MO_RECACCEL);
        desDist = GetLinStopLength(MO_RECSPEED, MO_STOACCEL);

        if ( dist > accDist+desDist )
        {
            return dist-desDist;
        }

        return dist*m_linMotion.stopAccel.x /
               (m_linMotion.recedeAccel.x+m_linMotion.stopAccel.x);
    }
}

// Retourne la vitesse de rotation des roues.

float CPhysics::GetWheelSpeed(bool bFront)
{
    float	speed;

    if ( m_bBrake && !(m_bHandbrake && bFront) && fabs(m_motorSpeed.x) > 0.5f )
    {
        speed = 0.0f;  // roues bloquées (pas d'ABS !)
    }
    else
    {
        speed = m_linMotion.realSpeed.x / m_linMotion.advanceSpeed.x * 50.0f;

        if ( !bFront )  // roues arričres (motrices) ?
        {
            speed += m_wheelSlide*40.0f;  // ajoute le patinage ŕ l'accélération
        }
    }

    return speed;
}


// Management of an event.
// Returns false if the object is destroyed.

bool CPhysics::EventProcess(const Event &event)
{
// TODO: ? ~krzys_h    if ( !m_object->GetEnable() )  return true;

    if ( event.type == EVENT_KEY_DOWN || event.type == EVENT_KEY_UP )
    {
        if ( event.GetData<KeyEventData>()->slot == INPUT_SLOT_ACTION )
        {
            HornEvent(event.type == EVENT_KEY_DOWN);
        }

        // Original code did this by adding additional input axis. I think we can do without that, at least for now. ~krzys_h
        if ( event.GetData<KeyEventData>()->key == KEY(SPACE) ) // TODO: keybinding?
        {
            m_motorSpeedWk = event.type == EVENT_KEY_DOWN ? 1.0f : 0.0f;
        }
    }

    /*if ( event.type == EVENT_BUTTON_HORN )
    {
        HornEvent(true);
    }
    if ( event.type == EVENT_BUTTON_BRAKE )
    {
        m_motorSpeedWk = 1.0f;
    }
    if ( event.type == EVENT_LBUTTONUP ||
         event.type == EVENT_RBUTTONUP )
    {
        HornEvent(false);
        m_motorSpeedWk = 0.0f;
    }*/

    if ( event.type == EVENT_FRAME )
    {
        if ( m_soundChannelHorn != -1 )
        {
            m_sound->Position(m_soundChannelHorn, m_object->GetPosition());
        }
        return EventFrame(event);
    }
    return true;
}


// Action du claxon.

void CPhysics::HornEvent(bool bPress)
{
    if ( bPress != m_bHornPress )  // changement d'état ?
    {
        m_bHornPress = bPress;

        if ( !m_bHornPress )  // bouton relâché ?
        {
            HornAction();
        }
    }

    if ( bPress )
    {
        if ( m_soundChannelHorn == -1 )
        {
            m_soundChannelHorn = m_sound->Play(SOUND_HORN, m_object->GetPosition(), 0.0f, 0.95f, true);
            m_sound->AddEnvelope(m_soundChannelHorn, 1.0f, 1.0f, 0.05f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannelHorn, 1.0f, 1.0f, 1.00f, SOPER_LOOP);
        }
    }
    else
    {
        if ( m_soundChannelHorn != -1 )
        {
            m_sound->FlushEnvelope(m_soundChannelHorn);
            m_sound->AddEnvelope(m_soundChannelHorn, 0.0f, 1.0f, 0.1f, SOPER_STOP);
            m_soundChannelHorn = -1;
        }
    }
}

// Updates instructions for the motor speed.

void CPhysics::MotorUpdate(float aTime, float rTime)
{
    Character* character;
    ObjectType  type;
    CPowerContainerObject* power = nullptr;
    Math::Vector    pos, motorSpeed;
    float energy, speed, factor, terrain, h, booster;

    if ( m_type == TYPE_MASS )
    {
        m_motorSpeed = Math::Vector(0.0f, 0.0f, 0.0f);
        m_motorSpeedW = 0.0f;
        m_motorSpeedWk = 0.0f;
        m_linMotion.motorAccel.x = m_linMotion.stopAccel.x;  // avancer/reculer
        m_linMotion.motorAccel.y = m_linMotion.stopAccel.y;  // monter/descendre
        m_linMotion.motorAccel.z = m_linMotion.stopAccel.z;  // translation latérale

        if ( m_floorHeight > 0.0f )  // en l'air ?
        {
            m_linMotion.motorSpeed.y = -200.0f;  // vitesse max de chute
        }
        else    // au sol ?
        {
            m_linMotion.motorSpeed.y = 0.0f;
        }
        return;
    }

    type = m_object->GetType();
    character = m_object->GetCharacter();

    if(std::isnan(m_motorSpeed.x)) m_motorSpeed.x = 0.f;
    if(std::isnan(m_motorSpeed.y)) m_motorSpeed.y = 0.f;
    if(std::isnan(m_motorSpeed.z)) m_motorSpeed.z = 0.f;

    if ( m_object->IsDying() || GetLock() || m_chocSpin != 0.0f )
    {
        m_motorSpeed = Math::Vector(0.0f, 0.0f, 0.0f);
        m_motorSpeedW = 0.0f;
        m_motorSpeedWk = 0.0f;
    }

    // Les consignes forcées sont utiles lorsque le véhicule est SetLock.
    if ( m_forceSpeed.x != 0.0f )
    {
        m_motorSpeed.x = m_forceSpeed.x;
        m_motorSpeedW = 0.0f;
        m_motorSpeedWk = 0.0f;
    }
    if ( m_forceSpeed.y != 0.0f )
    {
        m_motorSpeed.y = m_forceSpeed.y;
    }
    if ( m_forceSpeed.z != 0.0f )
    {
        m_motorSpeed.z = m_forceSpeed.z;
    }

    m_bHandbrake = false;

    if ( m_type == TYPE_RACE )
    {
        // Avance moins vite sur les zones lentes.
//TODO (krzys_h):        factor = m_terrain->GetSlowerZone(m_object->GetPosition());
        factor = 1.0f;
        if ( m_motorSpeed.x > 0.0f )
        {
            m_motorSpeed.x *= factor;
        }
        if ( m_motorSpeed.x < 0.0f )
        {
            m_motorSpeed.x *= Math::Norm(factor*3.0f);
        }

        // Avance moins vite sur les sols mous.
        if ( m_terrainHard < 0.5f )
        {
            m_motorSpeed.x *= 0.5f+m_terrainHard;
        }

        // Gestion du frein ŕ main.
        if ( m_motorSpeedW > 0.0f &&  // frein ŕ main ?
             m_motorSpeed.x >= 0.0f )
        {
            if ( m_linMotion.realSpeed.x > 0.0f )  // on avance ?
            {
                m_motorSpeed.x = -m_motorSpeedW*0.6f;  // met du frein normal
            }
            m_bHandbrake = true;

            if ( fabs(m_linMotion.realSpeed.x) < 1.0f )  // presque arręté ?
            {
                m_motorSpeed.x = 0.0f;  // ne met pas de gaz
            }
        }
        if ( m_motorSpeedWk > 0.0f &&  // frein ŕ main ?
             m_motorSpeed.x >= 0.0f )
        {
            if ( m_linMotion.realSpeed.x > 0.0f )  // on avance ?
            {
                m_motorSpeed.x = -m_motorSpeedWk*0.6f;  // met du frein normal
            }
            m_bHandbrake = true;

            if ( fabs(m_linMotion.realSpeed.x) < 1.0f )  // presque arręté ?
            {
                m_motorSpeed.x = 0.0f;  // ne met pas de gaz
            }
        }
    }

    motorSpeed = m_motorSpeed;

    booster = 1.0f;
    /* TODO (krzys_h)
    if ( m_object->GetStarting() )  // bloqué pendant 3,2,1,go ?
    {
        if ( m_main->IsStarter() &&   // starter existe ?
             m_main->GetMotorBlast() )
        {
            if ( m_forceSpeed.x == 0.0f )
            {
                if ( motorSpeed.x > 0.0f )
                {
                    m_timeMotorBurn += motorSpeed.x*rTime;
                }
                else
                {
                    m_timeMotorBurn -= rTime;
                    if ( m_timeMotorBurn < 0.0f )  m_timeMotorBurn = 0.0f;
                }
            }
        }
        motorSpeed.x = 0.0f;
    }
    else
    {
        if ( m_timeMotorBurn > 1.0f )
        {
            booster = m_timeMotorBurn*2.0f;  // super-méga accélération
            m_timeMotorBurn -= rTime;
        }
    }
     */

    if ( type == OBJECT_HUMAN ||
         type == OBJECT_TECH  )
    {
        if (IsObjectCarryingCargo(m_object) &&  // carries something?
             !m_bFreeze )
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

                if (IsObjectCarryingCargo(m_object))  // carries something?
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

    if (m_object->Implements(ObjectInterfaceType::Powered))
    {
        power = dynamic_cast<CPowerContainerObject*>(dynamic_cast<CPoweredObject*>(m_object)->GetPower());  // searches for the object battery uses
        if ( GetObjectEnergy(m_object) == 0.0f )  // no battery or flat?
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
                SetFalling();
            }
            SetMotor(false);
        }
    }

    if ( m_object->GetType() == OBJECT_HUMAN && dynamic_cast<CDestroyableObject*>(m_object)->GetDying() == DeathType::Dead )  // dead man?
    {
        motorSpeed.x = 0.0f;
        motorSpeed.z = 0.0f;
        if ( m_motion->GetAction() == MHS_DEADw )  // drowned?
        {
            motorSpeed.y = 0.0f;  // this is MHS_DEADw going back
        }
        else
        {
            motorSpeed.y = -1.0f;  // grave
        }
        SetMotor(false);
    }

    if ( m_object->Implements(ObjectInterfaceType::Flying) && !m_bLand && motorSpeed.y > 0.0f )
    {
        pos = m_object->GetPosition();
        h = m_terrain->GetFlyingLimit(pos, type==OBJECT_BEE);
        h += m_object->GetCharacter()->height;
        if ( pos.y > h-40.0f )  // almost at the top?
        {
            factor = 1.0f-(pos.y-(h-40.0f))/40.0f;
            if ( factor < -1.0f )  factor = -1.0f;
            if ( factor >  1.0f )  factor =  1.0f;
            motorSpeed.y *= factor;  // limit the rate of rise
        }
    }

    if ( m_object->Implements(ObjectInterfaceType::JetFlying) &&
         dynamic_cast<CJetFlyingObject*>(m_object)->GetRange() > 0.0f )  // limited flight range?
    {
        CJetFlyingObject* jetFlying = dynamic_cast<CJetFlyingObject*>(m_object);
        if ( m_bLand || m_bSwim || m_bObstacle )  // on the ground or in the water?
        {
            factor = 1.0f;
            if ( m_bObstacle )  factor = 3.0f;  // in order to leave!
            if ( m_bSwim )  factor = 3.0f;  // cools faster in water
            jetFlying->SetReactorRange(jetFlying->GetReactorRange() + rTime*(1.0f/5.0f)*factor);
            if ( jetFlying->GetReactorRange() == 1.0f )
            {
                if ( m_bLowLevel && m_object->GetSelect() )  // beep cool?
                {
                    m_sound->Play(SOUND_INFO, m_object->GetPosition(), 1.0f, 2.0f);
                    m_bLowLevel = false;
                }
            }
            m_bObstacle = false;
        }
        else    // in flight?
        {
            jetFlying->SetReactorRange(jetFlying->GetReactorRange() - rTime*(1.0f/jetFlying->GetRange()));
            if ( jetFlying->GetReactorRange() < 0.5f )  m_bLowLevel = true;
        }

        if ( jetFlying->GetReactorRange() == 0.0f )  // reactor tilt?
        {
            motorSpeed.y = -1.0f;  // grave
            SetFalling();
        }
    }

    if ( m_type == TYPE_RACE )
    {
        // Avance moins vite si dérape.
        motorSpeed.x *= 1.0f-Math::Norm(fabs(m_centriSpeed/character->gripSlide))*0.3f;

        // Tourne proportionnellement ŕ la vitesse.
        speed = Math::Norm(fabs(m_linMotion.realSpeed.x/m_linMotion.advanceSpeed.x));
        if ( motorSpeed.x >= 0.0f )  // avance ?
        {
            // Tourne moins sur un sol lisse.
            motorSpeed.z *= powf(speed, 0.5f)*(0.25f+0.5f*m_terrainHard);
            motorSpeed.z *= 1.0f-Math::Norm(fabs(m_centriSpeed/character->gripSlide)*character->turnSlide);
        }
        else    // recule ?
        {
            motorSpeed.z *= speed*2.0f;  // tourne + en marche arričre
        }

        // On freine si on avance avec la marche arričre.
        m_bBrake = ( motorSpeed.x < 0.0f && m_linMotion.realSpeed.x > 0.0f );
        m_bBrake |= m_bHandbrake;

        if ( m_bBrake && !m_bHandbrake )
        {
            // Tourne moins bien lors d'un freinage.
            motorSpeed.z *= 1.0f-(1.0f-character->brakeDir)*fabs(motorSpeed.x);
        }
        if ( m_bHandbrake )  // frein ŕ main ?
        {
            speed = Math::Norm(fabs(m_linMotion.realSpeed.x)/m_linMotion.advanceSpeed.x);
            motorSpeed.z *= 1.0f+powf(speed, 5.0f);  // tourne plus
        }

        if ( motorSpeed.x > 0.0f )  // en avant ?
        {
            speed = Math::Norm(fabs(m_linMotion.realSpeed.x)/m_linMotion.advanceSpeed.x);
//?            speed = Math::Norm(speed*1.2f);
//?            m_wheelSlide = m_motorSpeed.x*(1.0f-speed);
            if ( m_motorSpeed.x <= 0.5f )
            {
                m_wheelSlide = 0.0f;
            }
            else
            {
                speed = Math::Norm(speed*1.2f);
                m_wheelSlide = (m_motorSpeed.x-0.5f)/0.5f*(1.0f-speed);
            }
        }
        else
        {
            m_wheelSlide = 0.0f;
        }

        if ( m_wheelSlide > 0.0f )  // en avant ?
        {
            // Survire si gaz ŕ fond et vitesse faible.
            speed = Math::Norm(fabs(m_linMotion.realSpeed.x)/m_linMotion.advanceSpeed.x);
            speed = Math::Norm(speed*1.5f);
            motorSpeed.z *= 1.0f+m_wheelSlide*(1.0f-speed)*character->overProp;
        }

        if ( m_floorHeight > 0.0f )
        {
            h = m_floorHeight/character->suspDetect;
            if ( h > 1.0f )  h = 1.0f;
            motorSpeed.x *= 1.0f-h;  // on ne peut plus accélérer en l'air
            motorSpeed.z *= 1.0f-h;  // on ne peut plus tourner en l'air
        }

        // Ajoute une imprécision mécanique.
        if ( m_imprecisionT.x <= m_time )
        {
            m_imprecisionT.x = m_time+0.1f+Math::Rand()*0.2f;
            m_imprecisionH.x = Math::Rand()-0.5f;
        }
        m_imprecisionA.x = Math::Smooth(m_imprecisionA.x, m_imprecisionH.x, rTime*20.0f);

        if ( m_imprecisionT.z <= m_time )
        {
            m_imprecisionT.z = m_time+0.1f+Math::Rand()*0.2f;
            m_imprecisionH.z = Math::Rand()-0.5f;
        }
        m_imprecisionA.z = Math::Smooth(m_imprecisionA.z, m_imprecisionH.z, rTime*20.0f);

        factor = 2.0f-(m_terrainHard-0.2f)/0.6f;
        if ( motorSpeed.x != 0.0f )  motorSpeed.x += m_imprecisionA.x*factor*0.2f;
        motorSpeed.z += fabs(motorSpeed.x)*m_imprecisionA.z*factor*0.02f;
        motorSpeed.z *= 1.0f+m_imprecisionA.z*factor*0.5f;
    }

    // Avancer/reculer.
    if ( m_type == TYPE_RACE )
    {
        // Accélčre moins plus on va vite.
        factor = Math::Norm(fabs(m_linMotion.realSpeed.x/m_linMotion.advanceSpeed.x));
        factor = 1.0f-powf(1.0f-factor, character->accelSmooth);
        factor = character->accelLow - (character->accelLow-character->accelHigh)*factor;

        terrain = (0.6f+m_terrainHard*0.5f);
    }
    else
    {
        factor  = 1.0f;
        terrain = 1.0f;
    }

//? MotorParticle(aTime);

    // Forward/backward.
    if ( motorSpeed.x > 0.0f )
    {
        m_linMotion.motorAccel.x = m_linMotion.advanceAccel.x * terrain * factor * booster;
        m_linMotion.motorSpeed.x = m_linMotion.advanceSpeed.x * motorSpeed.x;
    }
    if ( motorSpeed.x < 0.0f )
    {
        m_linMotion.motorAccel.x = m_linMotion.recedeAccel.x * terrain;
        m_linMotion.motorSpeed.x = m_linMotion.recedeSpeed.x * motorSpeed.x;
    }
    if ( motorSpeed.x == 0.0f )
    {
        if ( m_type == TYPE_RACE && m_floorHeight > 0.0f )
        {
            if ( m_main->GetSelectedDifficulty() == 1 )  // niveau CN ?
            {
                m_linMotion.motorAccel.x = 5.0f;  // déccélération en l'air
                m_linMotion.motorSpeed.x = 10.0f * motorSpeed.x;
            }
            else
            {
                m_linMotion.motorAccel.x = 10.0f;  // déccélération en l'air
                m_linMotion.motorSpeed.x = 10.0f * motorSpeed.x;
            }
            m_centriSpeed = Math::SmoothP(m_centriSpeed, 0.0f, rTime);
        }
        else
        {
            m_linMotion.motorAccel.x = m_linMotion.stopAccel.x;
            m_linMotion.motorSpeed.x = 0.0f;
        }
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
    if ( m_type == TYPE_RACE )
    {
        speed = motorSpeed.z;

        // reverse if running back
        if ( m_linMotion.realSpeed.x < 0.0f && motorSpeed.x <= 0.0f )
        {
            speed = -speed;
        }
    }
    else
    {
        speed = motorSpeed.z;
    }

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

    if ( m_object->Implements(ObjectInterfaceType::Flying) && m_bLand )  // flying on the ground?
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

        pos = m_object->GetPosition();
        h = m_terrain->GetFlyingLimit(pos, type==OBJECT_BEE);
        h += m_object->GetCharacter()->height;
        bool reactorCool = true;
        if ( m_object->Implements(ObjectInterfaceType::JetFlying) )
        {
            reactorCool = dynamic_cast<CJetFlyingObject*>(m_object)->GetReactorRange() > 0.1f;
        }
        if ( motorSpeed.y > 0.0f && reactorCool && pos.y < h )
        {
            m_bLand = false;  // take off
            SetMotor(true);
            pos.y += 0.05f;  // small initial height (startup)
            m_object->SetPosition(pos);
        }
    }

    if ( !m_object->Implements(ObjectInterfaceType::Flying) )
    {
        if ( motorSpeed.x == 0.0f &&
             motorSpeed.z == 0.0f )
        {
            SetMotor(false);
        }
        else
        {
            SetMotor(true);
        }
    }

    if ( power != nullptr )  // battery transported?
    {
        factor = 1.0f;
        if ( type == OBJECT_MOBILEia ||
             type == OBJECT_MOBILEis ||
             type == OBJECT_MOBILEic ||
             type == OBJECT_MOBILEii )  factor = 0.5f;

        energy = power->GetEnergy();
        energy -= fabs(motorSpeed.x)*rTime*factor*0.005f;
        energy -= fabs(motorSpeed.z)*rTime*factor*0.005f;

        if ( m_object->Implements(ObjectInterfaceType::Flying) && motorSpeed.y > 0.0f )
        {
            energy -= motorSpeed.y*rTime*factor*0.01f;
        }
        if ( energy < 0.0f )  energy = 0.0f;
        power->SetEnergy(energy);
    }

    if ( m_type == TYPE_RACE )
    {
        if ( m_motorSpeed.x == 0.0f )
        {
            SetMotor(false);
        }
        else
        {
            SetMotor(true);
        }

        if ( m_object->GetLock() || !m_object->GetSelect() )
        {
            SetSilent(!m_bForceSlow);
        }
        else
        {
            SetSilent(false);
        }
    }

    if ( m_type == TYPE_TANK )
    {
        if ( motorSpeed.x == 0.0f &&
             motorSpeed.z == 0.0f )
        {
            SetMotor(false);
        }
        else
        {
            SetMotor(true);
        }
    }
}


// Action éventuelle lorsque le claxon est actionné.

void CPhysics::HornAction()
{
    Math::Vector zoom;

    if ( !m_object->GetSelect() )  return;
    if ( GetLock() )  return;
    if ( m_motion->GetAction() != -1 )  return;

    zoom = m_object->GetScale();
    if ( zoom.x != 1.0f ||
         zoom.y != 1.0f ||
         zoom.z != 1.0f )  return;

    /* TODO (krzys_h):
    if ( m_main->GetHornAction() == 1 )
    {
        m_motion->SetAction(MV_LOADBOT, 2.5f);
    }
    */
}

// Updates the effects of vibration and tilt.

void CPhysics::EffectUpdate(float aTime, float rTime)
{
    Character*  character;
    Math::Vector    vibLin, vibCir, incl, zoom;
    float       speedLin, speedCir, accel, factor, over;
    ObjectType  type;
    bool        bOnBoard;

    if ( m_type == TYPE_MASS )  return;
    if ( !m_engine->IsVisiblePoint(m_object->GetPosition()) )  return;

    type = m_object->GetType();
    character = m_object->GetCharacter();

    bOnBoard = false;
    if ( m_object->GetSelect() &&
         m_camera->GetType() == Gfx::CAM_TYPE_ONBOARD )
    {
        bOnBoard = true;
    }

    vibLin = m_motion->GetLinVibration();
    vibCir = m_motion->GetCirVibration();
    incl   = m_motion->GetTilt();

    if ( type == OBJECT_HUMAN ||  // human?
         type == OBJECT_TECH  )
    {
        if ( !m_bLand && !m_bSwim )  // in flight?
        {
            vibLin.y = sinf(aTime*2.00f)*0.5f+
                       sinf(aTime*2.11f)*0.3f;

            vibCir.z = sinf(aTime*Math::PI* 2.01f)*(Math::PI/150.0f)+
                       sinf(aTime*Math::PI* 2.51f)*(Math::PI/200.0f)+
                       sinf(aTime*Math::PI*19.01f)*(Math::PI/400.0f);

            vibCir.x = sinf(aTime*Math::PI* 2.03f)*(Math::PI/150.0f)+
                       sinf(aTime*Math::PI* 2.52f)*(Math::PI/200.0f)+
                       sinf(aTime*Math::PI*19.53f)*(Math::PI/400.0f);

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
            m_motion->SetTilt(incl);
        }
        else if ( m_bSwim )  // swimming?
        {
            vibLin.y = sinf(aTime*2.00f)*0.5f+
                       sinf(aTime*2.11f)*0.3f;

            vibCir.z = sinf(aTime*Math::PI* 2.01f)*(Math::PI/150.0f)+
                       sinf(aTime*Math::PI* 2.51f)*(Math::PI/200.0f)+
//?                    sinf(aTime*Math::PI*19.01f)*(Math::PI/400.0f)-Math::PI/2.0f;
                       sinf(aTime*Math::PI*19.01f)*(Math::PI/400.0f);

            vibCir.x = sinf(aTime*Math::PI* 2.03f)*(Math::PI/150.0f)+
                       sinf(aTime*Math::PI* 2.52f)*(Math::PI/200.0f)+
                       sinf(aTime*Math::PI*19.53f)*(Math::PI/400.0f);

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
            vibCir.z -= Math::PI*0.4f;

            vibLin *= m_linVibrationFactor;
            vibCir *= m_cirVibrationFactor;
            incl *= m_inclinaisonFactor;

            m_motion->SetLinVibration(vibLin);
            m_motion->SetCirVibration(vibCir);
            m_motion->SetTilt(incl);
        }
        else
        {
            m_motion->SetLinVibration(Math::Vector(0.0f, 0.0f, 0.0f));

//?         m_motion->SetCirVibration(Math::Vector(0.0f, 0.0f, 0.0f));
//?         m_motion->SetTilt(Math::Vector(0.0f, 0.0f, 0.0f));
        }
    }

    if ( type == OBJECT_CAR )  // roues ?
    {
        zoom = m_object->GetScale();

        speedLin = m_linMotion.realSpeed.x / m_linMotion.advanceSpeed.x;
        speedCir = m_cirMotion.realSpeed.y / m_cirMotion.advanceSpeed.y;
        incl.x = speedLin*speedCir*character->rolling;  // penche si virage
        if ( IsObjectCarryingCargo(m_object) )  incl.x *= 1.4f;
        if ( zoom.y < 1.0f )  incl.x = 0.0f;  // tout plat ?
        if ( zoom.z < 1.0f )  incl.x *= 3.0f;  // tout mince ?
        if ( incl.x >  0.9f )  incl.x =  0.9f;
        if ( incl.x < -0.9f )  incl.x = -0.9f;

        speedLin  = m_linMotion.currentSpeed.x - m_linMotion.motorSpeed.x;
        speedLin /= m_linMotion.advanceSpeed.x;
        if ( speedLin > 1.0f )  speedLin = 1.0f;
        factor = 1.0f;
        if ( IsObjectCarryingCargo(m_object) )  factor = 1.4f;
        m_linMotion.finalInclin.z = -speedLin*character->nicking*factor;
        accel = (0.40f-fabs(incl.z))*4.0f;
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

        // Tient compte de l'angle de survirage.
//?        incl.y = m_overTurn;
        if ( fabs(m_overTurn) > fabs(m_overTurnCur) )
        {
            m_overTurnCur = Math::SmoothP(m_overTurnCur, m_overTurn, rTime*1.0f);
        }
        else
        {
            m_overTurnCur = m_overTurn;
        }
        incl.y = m_overTurnCur;
        if ( m_motion->GetWheelType() != WT_BURN   &&
             m_camera->GetType() != Gfx::CAM_TYPE_ONBOARD )
        {
            incl.y *= character->overMul;
        }

        vibCir.x = 0.0f;
        vibCir.y = 0.0f;
        vibCir.z = 0.0f;

        vibLin.x = 0.0f;
        vibLin.z = 0.0f;
        vibLin.y = fabs(character->wheelFrontPos.x*sinf(incl.z))*0.8f +
                   fabs(character->wheelFrontPos.z*sinf(incl.x))*0.5f;
        if ( m_object->IsDying() )
        {
            vibLin.y -= character->wheelFrontDim*0.5f;
        }
        else
        {
            vibLin.y += (m_suspHeight[0]+m_suspHeight[1])/2.0f;
            vibCir.z += atanf((m_suspHeight[0]-vibLin.y)/character->wheelFrontPos.x);
        }

        // Fait vibrer la voiture ŕ l'arręt.
        factor = 1.0f-Math::Norm(fabs(m_linMotion.realSpeed.x)/20.0f);
        if ( m_object->IsDying() || m_bSilent )  factor = 0.0f;
        if ( bOnBoard )  factor *= 0.2f;
        vibCir.x += sinf(aTime*30.0f)*0.02f*factor;
        if ( zoom.y < 1.0f )  vibCir.x = 0.0f;  // tout plat ?

        if ( incl.x+vibCir.x > character->maxRolling )
        {
            over = incl.x+vibCir.x - character->maxRolling;
            incl.x   -= over/2.0f;
            vibCir.x -= over/2.0f;
        }
        if ( incl.x+vibCir.x < -character->maxRolling )
        {
            over = incl.x+vibCir.x + character->maxRolling;
            incl.x   -= over/2.0f;
            vibCir.x -= over/2.0f;
        }
        if ( bOnBoard )  vibCir.x *= 0.5f;
        if ( bOnBoard )  incl.x   *= 0.5f;

        if ( incl.z+vibCir.z > character->maxNicking )
        {
            over = incl.z+vibCir.z - character->maxNicking;
            incl.z   -= over/2.0f;
            vibCir.z -= over/2.0f;
        }
        if ( incl.z+vibCir.z < -character->maxNicking )
        {
            over = incl.z+vibCir.z + character->maxNicking;
            incl.z   -= over/2.0f;
            vibCir.z -= over/2.0f;
        }
        if ( bOnBoard )  vibCir.z *= 0.5f;

        m_motion->SetTilt(incl);
        m_motion->SetLinVibration(vibLin);
        m_object->SetCirVibration(vibCir);

        // Fait vibrer le moteur.
//?        factor = 1.0f-Math::Norm(fabs(m_linMotion.realSpeed.x)/50.0f);
//?        if ( m_object->GetDead() || m_bSilent )  factor = 0.0f;
//?        m_object->SetAngleX(7, sinf(aTime*41.0f)*0.03f*factor);
        factor = Math::Norm(fabs(m_linMotion.realSpeed.x)/50.0f);
        m_motorAngle += rTime*37.0f*(1.0f+factor);
        if ( m_object->IsDying() || m_bSilent )  m_motorAngle = 0.0f;
        m_object->SetPartRotationX(7, sinf(m_motorAngle)*0.03f*(1.0f-factor*0.5f));
    }

    if ( type == OBJECT_UFO )
    {
        speedLin = m_linMotion.realSpeed.x / m_linMotion.advanceSpeed.x;
        speedCir = m_cirMotion.realSpeed.y / m_cirMotion.advanceSpeed.y;

//?        vibLin.y = fabs(speedLin)*24.0f;
//?        if ( vibLin.y > 12.0f )  vibLin.y = 12.0f;
        vibLin.y = fabs(speedLin)*4.0f;

        vibCir.x = -speedCir*0.8f + sinf(aTime*8.0f)*0.12f + sinf(aTime*5.1f)*0.08f;
        vibCir.z = -speedLin*0.4f;

        m_motion->SetLinVibration(vibLin);
        m_motion->SetCirVibration(vibCir);
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
        accel = (0.40f-fabs(incl.z))*4.0f;
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
        m_object->SetTilt(incl);

        vibLin.x = 0.0f;
        vibLin.z = 0.0f;
        vibLin.y = fabs(character->wheelFront*sinf(incl.z))*0.8f +
                   fabs(character->wheelRight*sinf(incl.x))*0.5f;
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
            m_motion->SetLinVibration(Math::Vector(0.0f, 0.0f, 0.0f));
            m_motion->SetCirVibration(Math::Vector(0.0f, 0.0f, 0.0f));
            m_motion->SetTilt(Math::Vector(0.0f, 0.0f, 0.0f));
        }
        else    // in flight?
        {
            vibLin.y = sinf(aTime*2.00f)*0.5f+
                       sinf(aTime*2.11f)*0.3f;

            vibCir.z = sinf(aTime*Math::PI* 2.01f)*(Math::PI/150.0f)+
                       sinf(aTime*Math::PI* 2.51f)*(Math::PI/200.0f)+
                       sinf(aTime*Math::PI*19.01f)*(Math::PI/400.0f);

            vibCir.x = sinf(aTime*Math::PI* 2.03f)*(Math::PI/150.0f)+
                       sinf(aTime*Math::PI* 2.52f)*(Math::PI/200.0f)+
                       sinf(aTime*Math::PI*19.53f)*(Math::PI/400.0f);

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
            m_motion->SetTilt(incl);
        }
    }

    if ( type == OBJECT_BEE )  // bee?
    {
        if ( !m_bLand )  // in flight?
        {
            vibLin.y = sinf(aTime*2.00f)*0.5f+
                       sinf(aTime*2.11f)*0.3f;

            vibCir.z = (Math::Rand()-0.5f)*0.1f+
                       sinf(aTime*Math::PI* 2.01f)*(Math::PI/150.0f)+
                       sinf(aTime*Math::PI* 2.51f)*(Math::PI/200.0f)+
                       sinf(aTime*Math::PI*19.01f)*(Math::PI/400.0f);

            vibCir.x = (Math::Rand()-0.5f)*0.1f+
                       sinf(aTime*Math::PI* 2.03f)*(Math::PI/150.0f)+
                       sinf(aTime*Math::PI* 2.52f)*(Math::PI/200.0f)+
                       sinf(aTime*Math::PI*19.53f)*(Math::PI/400.0f);

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
            m_motion->SetTilt(incl);
        }
    }
}


// Updates structure Motion.

void CPhysics::UpdateMotionStruct(float rTime, Motion &motion)
{
    float    speed, motor, init;

    // Management for the coordinate x.
    init  = motion.realSpeed.x;
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

    if ( fabs(motion.terrainSpeed.x) > motion.terrainSlide.x )
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
    motion.realAccel.x = (motion.realSpeed.x-init)/rTime;

    // Management for the coordinate y.
    init  = motion.realSpeed.y;
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

    if ( fabs(motion.terrainSpeed.y) > motion.terrainSlide.y )
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
    motion.realAccel.y = (motion.realSpeed.y-init)/rTime;

    // Management for the coordinate z.
    init  = motion.realSpeed.z;
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

    if ( fabs(motion.terrainSpeed.z) > motion.terrainSlide.z )
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
    motion.realAccel.z = (motion.realSpeed.z-init)/rTime;
}


// Makes physics evolve as time elapsed.
// Returns false if the object is destroyed.
//
//  a:  acceleration
//  v1: velocity at time t1
//  v2: velocity at time t2
//  dt: time elapsed since t1, then: dt = t2-t1
//  dd: difference in distance (advance)
//
//  v2 = v1 + a*dt
//  dd = v2*dt

bool CPhysics::EventFrame(const Event &event)
{
    Character*    character;
    ObjectType  type;
    Math::Matrix    objRotate, matRotate, *mat;
    Math::Vector    iPos, iAngle, tAngle, pos, newpos, angle, newangle, n, cirVib;
    float       h, w, factor;
    int         i;

    if ( m_engine->GetPause() )
    {
        if ( m_object->GetType() == OBJECT_CAR )
        {
//TODO (krzys_h):            m_engine->SetJoyForces(0.0f, 0.0f);
        }
        return true;
    }

    m_time += event.rTime;
    m_timeUnderWater += event.rTime;
    m_soundTimeJostle += event.rTime;
    m_timeLock        += event.rTime;

    character = m_object->GetCharacter();
    type = m_object->GetType();
    iPos      = m_object->GetPosition();
    iAngle    = m_object->GetRotation();

    if ( type == OBJECT_CARROT  ||
         type == OBJECT_STARTER ||
         type == OBJECT_WALKER  ||
         type == OBJECT_CRAZY   ||
         type == OBJECT_GUIDE   )
    {
        if ( IsObjectBeingTransported(m_object) ||
             m_object->GetLock()  )  return true;
    }

    FFBForce(0.0f);

    m_terrainHard = m_terrain->GetHardness(iPos);
    m_bOldWater = m_bWater;
    m_bWater = ( iPos.y+1.0f < m_water->GetLevel() && !m_water->GetLava() );

    if ( type == OBJECT_CAR )
    {
//TODO (krzys_h):        m_sound->SetWater(m_bWater);  // bruit feutré si sous l'eau
    }

    FrameParticle(m_time, event.rTime);
    MotorUpdate(m_time, event.rTime);
    EffectUpdate(m_time, event.rTime);
    WaterFrame(m_time, event.rTime);

    pos   = m_object->GetPosition();
    angle = m_object->GetRotation();

    if ( m_timeMotorBurn >= 1.9f )  // moteur explose ?
    {
        m_main->FlushStarterType();  // recommence avec un starter lent
        m_engine->GetPyroManager()->Create(Gfx::PT_EXPLOT, m_object);  // destruction totale
        return false;
    }

    // Accelerate is the descent, brake is the ascent.
    if ( m_bFreeze || (m_object->Implements(ObjectInterfaceType::Destroyable) && dynamic_cast<CDestroyableObject*>(m_object)->IsDying()) )
    {
        m_linMotion.terrainSpeed.x = 0.0f;
        m_linMotion.terrainSpeed.z = 0.0f;
        m_linMotion.terrainSpeed.y = 0.0f;
    }
    else
    {
        tAngle = angle;
        h = m_terrain->GetBuildingFactor(pos);
        if ( type == OBJECT_HUMAN ||
             type == OBJECT_TECH  )
        {
            if ( m_linMotion.currentSpeed.x == 0.0f )
            {
                h *= 0.5f;  // immobile man -> slippage
            }
            FloorAngle(pos, tAngle);  // calculates the angle with the ground
        }

        if ( pos.y < m_water->GetLevel(m_object) )  // underwater?
        {
            h *= 0.5f;
            m_fallingHeight = 0.0f; // can't fall underwater
        }

//?     m_linMotion.terrainSpeed.x = -tAngle.z*m_linMotion.terrainForce.x*h;
//?     m_linMotion.terrainSpeed.z =  tAngle.x*m_linMotion.terrainForce.z*h;
//?     m_linMotion.terrainSpeed.x = -sinf(tAngle.z)*Math::PI*0.5f*m_linMotion.terrainForce.x*h;
//?     m_linMotion.terrainSpeed.z =  sinf(tAngle.x)*Math::PI*0.5f*m_linMotion.terrainForce.z*h;
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
        w = m_water->GetLevel(m_object);
        if ( h < w )  h = w;
        h = pos.y-h-10.0f;  // maximum height (*)
        if ( h < 0.0f )  h = 0.0f;
        m_linMotion.terrainSpeed.y = -h*2.5f;  // is not above
    }

    // (*)  High enough to pass over the tower defense (OBJECT_TOWER),
    //      but not too much to pass under the cover of the ship (OBJECT_BASE)!

    UpdateMotionStruct(event.rTime, m_linMotion);
    UpdateMotionStruct(event.rTime, m_cirMotion);
    m_linMotion.realSpeed.z += m_centriSpeed;

    if ( m_type == TYPE_MASS )
    {
        cirVib = m_object->GetCirVibration();
        cirVib += event.rTime*m_cirMotion.realSpeed;
        m_object->SetCirVibration(cirVib);
        newangle = iAngle;
        newpos = iPos + event.rTime*m_linMotion.realSpeed;
    }
    else
    {
        newangle = angle + event.rTime * m_cirMotion.realSpeed;
        Math::LoadRotationZXYMatrix(matRotate, newangle);
        newpos = event.rTime * m_linMotion.realSpeed;
        newpos = Transform(matRotate, newpos);
        newpos += pos;
    }

    if ( m_type == TYPE_RACE &&
         !m_bBrake &&
         m_linMotion.realSpeed.x < 0.0f &&  // marche arričre ?
         m_chocSpin == 0.0f )  // pas de rotation due ŕ un choc ?
    {
        // Déplace l'axe de rotation sur l'essieu arričre (ouaaah !).
        mat = m_object->GetRotateMatrix(0);
        newpos += Transform(*mat, Math::Vector(character->wheelBackPos.x, 0.0f, 0.0f));
        newpos -= Transform(matRotate, Math::Vector(character->wheelBackPos.x, 0.0f, 0.0f));
    }

    // Transfčre l'énergie de survirage en orientation réelle
    // et définitive.
    if ( m_type == TYPE_RACE && m_floorHeight < character->suspDetect )
    {
        factor = 1.0f-(m_floorHeight/character->suspDetect);
        factor *= m_overTurn*event.rTime*1.0f;
//?        if ( m_bBrake )  factor *= 0.0f;
//?        if ( m_bHandbrake )  factor *= 0.0f;

        if ( m_overTurn > 0.0f )
        {
            m_overTurn -= factor;
            if ( m_overTurn < 0.0f )  m_overTurn = 0.0f;
        }
        if ( m_overTurn < 0.0f )
        {
            m_overTurn -= factor;
            if ( m_overTurn > 0.0f )  m_overTurn = 0.0f;
        }

        newangle.y += factor;
    }

    m_terrain->AdjustToStandardBounds(newpos);

    if ( m_object->Implements(ObjectInterfaceType::Flying) && !m_bLand )
    {
        h = m_terrain->GetFlyingLimit(newpos, type==OBJECT_BEE);
        h += m_object->GetCharacter()->height;
        if ( newpos.y > h )  newpos.y = h;
    }

    FloorAdapt(m_time, event.rTime, newpos, newangle);
    m_glideAmplitude = 0.0f;

    if ( m_bForceUpdate         ||
         newpos.x   != iPos.x   ||
         newpos.y   != iPos.y   ||
         newpos.z   != iPos.z   ||
         newangle.x != iAngle.x ||
         newangle.y != iAngle.y ||
         newangle.z != iAngle.z )
    {
        i = ObjectAdapt(newpos, newangle, m_time, event.rTime);
        if ( i == 2 )  // object destroyed?
        {
            return false;
        }
        if ( i == 1 )  // immobile object?
        {
            newpos = iPos;  // keeps the initial position, but accepts the rotation
        }
    }

    UpdateGlideSound(event.rTime);

    if ( newangle.x != angle.x ||
         newangle.y != angle.y ||
         newangle.z != angle.z )
    {
        m_object->SetRotation(newangle);
    }

    if ( newpos.x != pos.x ||
         newpos.y != pos.y ||
         newpos.z != pos.z )
    {
        m_object->SetPosition(newpos);
    }

    MotorParticle(m_time, event.rTime);
    SoundMotor(event.rTime);

    if ( type == OBJECT_CAR )
    {
        FFBFrame(m_time, event.rTime);
    }

    if ( m_object->GetSelect() &&
         m_type == TYPE_RACE   &&
         m_camera->GetControllingObject() == m_object )
    {
        m_camera->SetMotorSpeed(m_linMotion.realSpeed.x);

        /* TODO (krzys_h): There is soething wrong with this code
        if ( !GetLock() && !m_object->IsDying() )
        {
            // Eloigne la caméra en marche arričre.
            float hope, current;
            if ( m_motorSpeed.x < 0.0f && !m_bBrake )
            {
                hope = 50.0f;
                factor = 1.0f;
            }
            else
            {
                hope = 20.0f;
                factor = 0.6f;
            }
            current = m_camera->GetBackDist();
            m_camera->SetBackDist(Math::Smooth(current, hope, event.rTime*factor));

            if ( m_motorSpeed.x < 0.0f && !m_bBrake )
            {
                hope = -Math::PI*0.10f;
                factor = 1.0f;
            }
            else
            {
                hope = -Math::PI*0.05f;
                factor = 0.6f;
            }
            current = m_camera->GetBackVerti();
            m_camera->SetBackVerti(Math::Smooth(current, hope, event.rTime*factor));
        }
        */
    }

    if ( m_bLand && m_fallingHeight != 0.0f ) // if fell
    {
        if (m_object->Implements(ObjectInterfaceType::Damageable))
        {
            float force = (m_fallingHeight - m_object->GetPosition().y) * m_fallDamageFraction;
            if (m_object->DamageObject(DamageType::Fall, force))
            {
                return false; // ugly hack, but works for 0.1.6 release :/
            }
        }
        m_fallingHeight = 0.0f;
    }

    m_bForceUpdate = false;

    return true;
}

// Indique si la voiture est utilisable. Il y a un léger retard aprčs
// que l'objet soit Unlock, pour simuler le démarrage du moteur.

bool CPhysics::GetLock()
{
    if ( m_type == TYPE_RACE &&
         !m_main->IsStarter() )   // pas de starter ?
    {
        if ( m_object->GetLock() )
        {
            m_timeLock = 0.0f;
            return true;
        }
        else
        {
            return (m_timeLock < STARTDELAY);
        }
    }
    else
    {
        return m_object->GetLock();
    }
}


// Starts or stops the engine sounds.

void CPhysics::SoundMotor(float rTime)
{
    ObjectType  type;

    if ( m_type == TYPE_MASS )  return;

    m_lastSoundInsect -= rTime;
    type = m_object->GetType();

    if ( m_type == TYPE_RACE )
    {
        if ( m_bMotor && !m_bSilent && !m_bBrake && m_object->GetActive() )
        {
            SoundMotorFull(rTime, type);  // plein régime
        }
        else
        {
            if ( (m_object->GetSelect() || m_bForceSlow) &&
                 !m_object->IsDying() &&
                 !m_bSilent )
            {
                SoundMotorSlow(rTime, type);  // au ralenti
            }
            else
            {
                SoundMotorStop(rTime, type);  // ŕ l'arręt
            }
        }
        return;
    }

    if ( type == OBJECT_TRAX )
    {
        if ( m_bMotor && m_object->GetActive() )
        {
            SoundMotorFull(rTime, type);  // plein régime
        }
        else
        {
            SoundMotorSlow(rTime, type);  // au ralenti
        }
        return;
    }

    if ( type == OBJECT_UFO )
    {
        if ( m_bMotor && !m_bSwim &&
             m_object->GetActive() && !m_object->IsDying() )
        {
            SoundReactorFull(rTime, type);  // plein régime
        }
        else
        {
            SoundReactorStop(rTime, type);  // ŕ l'arręt
        }
        return;
    }

    // m_type == TYPE_DEFAULT

    if ( type == OBJECT_MOTHER )
    {
        if ( m_lastSoundInsect <= 0.0f && m_object->GetDetectable() )
        {
            m_sound->Play(SOUND_INSECTm, m_object->GetPosition());
            if ( m_bMotor )  m_lastSoundInsect = 0.4f+Math::Rand()*2.5f;
            else             m_lastSoundInsect = 1.5f+Math::Rand()*4.0f;
        }
    }
    else if ( type == OBJECT_ANT )
    {
        assert(m_object->Implements(ObjectInterfaceType::Destroyable));
        if ( dynamic_cast<CDestroyableObject*>(m_object)->GetDying() == DeathType::Burning ||
             dynamic_cast<CBaseAlien*>(m_object)->GetFixed() )
        {
            if ( m_lastSoundInsect <= 0.0f )
            {
                m_sound->Play(SOUND_INSECTa, m_object->GetPosition(), 1.0f, 1.5f+Math::Rand()*0.5f);
                m_lastSoundInsect = 0.4f+Math::Rand()*0.6f;
            }
        }
        else if ( m_object->GetDetectable() )
        {
            if ( m_lastSoundInsect <= 0.0f )
            {
                m_sound->Play(SOUND_INSECTa, m_object->GetPosition());
                if ( m_bMotor )  m_lastSoundInsect = 0.4f+Math::Rand()*2.5f;
                else             m_lastSoundInsect = 1.5f+Math::Rand()*4.0f;
            }
        }
    }
    else if ( type == OBJECT_BEE )
    {
        assert(m_object->Implements(ObjectInterfaceType::Destroyable));
        if ( m_object->GetDetectable() )
        {
            if ( m_lastSoundInsect <= 0.0f )
            {
                m_sound->Play(SOUND_INSECTb, m_object->GetPosition());
                if ( m_bMotor )  m_lastSoundInsect = 0.4f+Math::Rand()*2.5f;
                else             m_lastSoundInsect = 1.5f+Math::Rand()*4.0f;
            }
        }
        else if ( dynamic_cast<CDestroyableObject*>(m_object)->GetDying() == DeathType::Burning )
        {
            if ( m_lastSoundInsect <= 0.0f )
            {
                m_sound->Play(SOUND_INSECTb, m_object->GetPosition(), 1.0f, 1.5f+Math::Rand()*0.5f);
                m_lastSoundInsect = 0.3f+Math::Rand()*0.5f;
            }
        }
    }
    else if ( type == OBJECT_WORM )
    {
        assert(m_object->Implements(ObjectInterfaceType::Destroyable));
        if ( m_object->GetDetectable() )
        {
            if ( m_lastSoundInsect <= 0.0f )
            {
                m_sound->Play(SOUND_INSECTw, m_object->GetPosition());
                if ( m_bMotor )  m_lastSoundInsect = 0.4f+Math::Rand()*2.5f;
                else             m_lastSoundInsect = 1.5f+Math::Rand()*4.0f;
            }
        }
        else if ( dynamic_cast<CDestroyableObject*>(m_object)->GetDying() == DeathType::Burning )
        {
            if ( m_lastSoundInsect <= 0.0f )
            {
                m_sound->Play(SOUND_INSECTw, m_object->GetPosition(), 1.0f, 1.5f+Math::Rand()*0.5f);
                m_lastSoundInsect = 0.2f+Math::Rand()*0.2f;
            }
        }
    }
    else if ( type == OBJECT_SPIDER )
    {
        assert(m_object->Implements(ObjectInterfaceType::Destroyable));
        if ( dynamic_cast<CDestroyableObject*>(m_object)->GetDying() == DeathType::Burning ||
             dynamic_cast<CBaseAlien*>(m_object)->GetFixed() )
        {
            if ( m_lastSoundInsect <= 0.0f )
            {
                m_sound->Play(SOUND_INSECTs, m_object->GetPosition(), 1.0f, 1.5f+Math::Rand()*0.5f);
                m_lastSoundInsect = 0.4f+Math::Rand()*0.6f;
            }
        }
        else if ( m_object->GetDetectable() )
        {
            if ( m_lastSoundInsect <= 0.0f )
            {
                m_sound->Play(SOUND_INSECTs, m_object->GetPosition());
                if ( m_bMotor )  m_lastSoundInsect = 0.4f+Math::Rand()*2.5f;
                else             m_lastSoundInsect = 1.5f+Math::Rand()*4.0f;
            }
        }
    }
    else    // vehicle?
    {
        if ( !m_object->Implements(ObjectInterfaceType::Flying) )
        {
            if ( m_bMotor && m_object->GetDetectable() )
            {
                SoundMotorFull(rTime, type);  // full diet
            }
            else
            {
                float energy = GetObjectEnergy(m_object);

                if ( m_object->GetSelect() &&
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

        if ( m_object->Implements(ObjectInterfaceType::Flying) )
        {
            if ( m_bMotor && !m_bSwim &&
                 m_object->GetDetectable() )
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
    Math::Matrix*    mat;
    Character*    character;
    ObjectType  type;
    Math::Vector    pos, speed;
    Math::Point     dim;
    float        level, duration, height, factor;

    level = m_water->GetLevel();
    if ( level == 0.0f )  return;  // no water?
    if (IsObjectBeingTransported(m_object))  return;

    type = m_object->GetType();

    // Management of flames into the lava.
    pos = m_object->GetPosition();
    height = pos.y - m_water->GetLevel(m_object);  // hauteur sur l'eau
    if ( m_water->GetLava() &&
         pos.y-m_object->GetCharacter()->height <= level )
    {
        if ( m_lastFlameParticle+m_engine->ParticleAdapt(0.05f) <= aTime )
        {
            m_lastFlameParticle = aTime;

            pos = m_object->GetPosition();
            pos.x += (Math::Rand() - 0.5f) * 3.0f;
            pos.z += (Math::Rand() - 0.5f) * 3.0f;
            speed.x = 0.0f;
            speed.z = 0.0f;
            speed.y = Math::Rand() * 5.0f + 3.0f;
            dim.x = Math::Rand() * 2.0f + 1.0f;
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIFLAME, 2.0f, 0.0f, 0.2f);

            pos = m_object->GetPosition();
            pos.y -= 2.0f;
            pos.x += (Math::Rand() - 0.5f) * 5.0f;
            pos.z += (Math::Rand() - 0.5f) * 5.0f;
            speed.x = 0.0f;
            speed.z = 0.0f;
            speed.y = 6.0f + Math::Rand() * 6.0f + 6.0f;
            dim.x = Math::Rand() * 1.5f + 1.0f + 3.0f;
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISMOKE3, 4.0f);

            if (type == OBJECT_CAR)  // voiture ?
            {
                character = m_object->GetCharacter();
                mat = m_object->GetWorldMatrix(0);

                if (height < 0.0f &&
                    m_motion->GetWheelType() != WT_BURN)  // roues pas cramées ?
                {
                    for (int i = 0; i < 4 * 5; i++)
                    {
                        if (i % 4 < 2) pos = character->wheelFrontPos;
                        else pos = character->wheelBackPos;
                        pos.z += 1.0f;
                        if (i % 2 == 0) pos.z = -pos.z;
                        pos = Transform(*mat, pos);
                        speed.x = 0.0f;
                        speed.z = 0.0f;
                        speed.y = Math::Rand() * 6.0f + 6.0f;
                        dim.x = Math::Rand() * 1.0f + 1.0f;
                        dim.y = dim.x;
                        duration = 0.5f + Math::Rand() * 0.5f;
                        m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIWHEEL, duration);
                    }
                }

                if (m_timeWheelBurn > 0.0f)  // jantes crament encore ?
                {
                    if (m_timeWheelBurn > 0.0f)
                    {
                        m_timeWheelBurn -= rTime;
                        if (m_timeWheelBurn < 0.0f) m_timeWheelBurn = 0.0f;
                    }

                    for (int i = 0; i < 4 * 2; i++)
                    {
                        if (i % 4 < 2) pos = character->wheelFrontPos;
                        else pos = character->wheelBackPos;
                        pos.z += 1.0f;
                        if (i % 2 == 0) pos.z = -pos.z;
                        pos = Transform(*mat, pos);
                        speed.x = 0.0f;
                        speed.z = 0.0f;
                        speed.y = Math::Rand() * 3.0f + 3.0f;
                        dim.x = Math::Rand() * 0.6f + 0.6f;
                        dim.y = dim.x;
                        duration = 1.5f + Math::Rand() * 1.5f;
                        m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIWHEEL, duration);
                    }
                }
            }
        }
    }

    if ( height >= 0.0f )  return;  // out of water?

    if ( type == OBJECT_TOTO )  return;
    if ( type == OBJECT_NULL )  return;

    if ( !m_object->GetDetectable() )  return;

    if (type == OBJECT_HUMAN && m_object->GetOption() != 0 )  // human without a helmet?)
    {
        assert(m_object->Implements(ObjectInterfaceType::Destroyable));
        dynamic_cast<CDestroyableObject*>(m_object)->DestroyObject(DestructionType::Drowned);
    }
    else if ( m_water->GetLava() ||
         type == OBJECT_MOBILEfa || // TODO: A function in CObject to check if object is waterproof or not
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
         type == OBJECT_APOLLO2  )
    {
        if (m_object->Implements(ObjectInterfaceType::Destroyable) && !m_object->IsDying())
        {
            if ( type == OBJECT_CAR )  // voiture ?
            {
                level = m_object->GetBurnShield();
                factor = 5.0f+height*4.0f/5.0f;  // si profond -> meurt vite
                if ( factor < 0.5f )  factor = 0.5f;
                level -= rTime/factor;
                if ( level < 0.0f )  level = 0.0f;
                m_object->SetBurnShield(level);

                if ( level < 0.5f )
                {
                    if ( m_motion->GetWheelType() != WT_BURN )
                    {
                        m_motion->SetWheelType(WT_BURN);  // roule sur les jantes
                        m_timeWheelBurn = 5.0f;  // crame encore qq temps
                    }
                }

                if ( level == 0.0f )
                {
                    dynamic_cast<CDestroyableObject*>(m_object)->DestroyObject(DestructionType::Explosion);
                }
            }
            else
            {
                dynamic_cast<CDestroyableObject*>(m_object)->DestroyObject(DestructionType::ExplosionWater);
            }
        }
    }
}

// Choc avec vibration.

void CPhysics::FFBCrash(float amplitude, float speed, float frequency)
{
    m_FFBamplitude = amplitude;
    m_FFBspeed = speed;
    m_FFBfrequency = frequency;
    m_FFBprogress = 1.0f;
}

// Force constante.

void CPhysics::FFBForce(float amplitude)
{
    m_FFBforce = amplitude;
}

// Gestion de l'effet force feedback.

void CPhysics::FFBFrame(float aTime, float rTime)
{
    /* TODO (krzys_h)
    float    forceX, forceY, effect, speed;

    if ( m_main->GetPhase() != PHASE_SIMUL )  return;

    forceX = 0.0f;
    forceY = 0.0f;
    speed = Abs(m_linMotion.realSpeed.x/m_linMotion.advanceSpeed.x);

    if ( m_FFBprogress > 0.0f )
    {
        m_FFBprogress -= rTime*(1.0f/m_FFBspeed);
        if ( m_FFBprogress < 0.0f )  m_FFBprogress = 0.0f;
//?        effect = (Rand()-0.5f)*2.0f;
        effect = sinf(0.2f+m_FFBprogress* 7.0f*m_FFBfrequency)+
                 cosf(0.1f+m_FFBprogress*10.0f*m_FFBfrequency)+
                 sinf(0.7f+m_FFBprogress*19.0f*m_FFBfrequency);
        effect *= m_FFBprogress*m_FFBamplitude;
        forceX += effect;
        forceY += effect;
    }

    forceX += m_FFBforce;

    if ( m_terrainHard < 0.5f && !m_bWater )  // sol mou ?
    {
        effect = (Rand()-0.5f)*0.7f*speed;
        forceX += effect;
        forceY += effect;
    }

    if ( m_motion->GetWheelType() == WT_BURN )  // sur les jantes ?
    {
        effect = sinf(m_time*15.0f*speed)*0.6f*speed;
        forceX += effect;
        forceY += effect;
    }

    if ( forceX == 0.0f && m_engine->GetJoystick() == 1 )
    {
        if ( m_bWater )  // sous l'eau ?
        {
            forceX = -m_motorSpeed.z*speed;  // accentue
        }
        else
        {
            if ( m_bBrake )
            {
                m_FFBbrake += rTime*3.0f;
                m_FFBbrake = Norm(m_FFBbrake);
                forceX = -m_motorSpeed.z*speed*0.5f*m_FFBbrake;  // accentue
            }
            else
            {
                m_FFBbrake = 0.0f;
                forceX = m_motorSpeed.z*0.7f;  // rappel au centre
            }
        }
    }

    if ( m_floorHeight > 0.0f )
    {
        forceX = 0.0f;
    }

    m_engine->SetJoyForces(NormSign(forceX), NormSign(forceY));
    */
}

// Sounds the engine at full power.

void CPhysics::SoundMotorFull(float rTime, ObjectType type)
{
    // TODO (krzys_h): check this code
    if (m_type != TYPE_NORMAL)
    {
        Character*    character;
        SoundType        sound;
        Math::Matrix*    mat;
        Math::Vector    pos, speed;
        Math::Point        dim;
        float        freq, dir, factor, RPM;
        int            i, max;

        if ( type == OBJECT_HUMAN   ||
             type == OBJECT_TECH    ||
             type == OBJECT_UFO     ||
             type == OBJECT_CARROT  ||
             type == OBJECT_STARTER ||
             type == OBJECT_WALKER  ||
             type == OBJECT_CRAZY   ||
             type == OBJECT_GUIDE   ||
             type == OBJECT_EVIL1   ||
             type == OBJECT_EVIL2   ||
             type == OBJECT_EVIL3   ||
             type == OBJECT_EVIL4   ||
             type == OBJECT_EVIL5   )  return;

        character = m_object->GetCharacter();
        sound = SOUND_MOTOR1;
        if ( character->motorSound == 1 )  sound = SOUND_MOTOR1;
        if ( character->motorSound == 2 )  sound = SOUND_MOTOR2;
        if ( character->motorSound == 3 )  sound = SOUND_MOTOR3;
        if ( character->motorSound == 4 )  sound = SOUND_MOTOR4;
        if ( character->motorSound == 5 )  sound = SOUND_MOTOR5;
        if ( character->motorSound == 6 )  sound = SOUND_MOTOR6;

        freq = fabs(m_linMotion.realSpeed.x/m_linMotion.advanceSpeed.x);

        if ( freq < 0.5f )  // 1čre
        {
            freq = freq/0.5f;
            m_motorState = 1;
        }
        else if ( freq < 0.8f )  // 2čme
        {
            freq = (freq-0.5f)/0.3f;
            freq = 0.2f+freq*0.8f;
            m_motorState = 2;
        }
        else  // 3čme
        {
            freq = (freq-0.8f)/0.2f;
            freq = 0.2f+freq*0.8f;
            m_motorState = 3;
        }

        if ( m_motorSpeed.x < 0.0f && !m_bBrake )
        {
            m_motorState = -1;
        }

        freq = 0.5f+0.5f*freq;
        freq *= 1.3f;
        if ( m_floorHeight > 0.0f )  freq *= 1.3f;
        if ( freq > 2.0f )  freq = 2.0f;  // rupteur
//?    freq *= 1.0f+m_wheelSlide*1.5f;
        if ( false /* TODO (krzys_h): m_object->GetStarting() */ )  // bloqué pendant 3,2,1,go ?
        {
            freq *= 3.0f;
            if ( m_timeMotorBurn > 1.0f )  // explosion moteur imminente ?
            {
                freq *= m_timeMotorBurn;  // augmente la fréquence
            }
            factor = 10.0f;
            m_motorState = 0;
        }
        else
        {
            factor = 10.0f;
        }
        m_soundFrequencyMotor1 = Math::Smooth(m_soundFrequencyMotor1, freq, rTime*factor);
        m_soundAmplitudeMotor1 = Math::Linear(m_soundAmplitudeMotor1, 1.0f, rTime*factor);

        m_soundFrequencyMotor2 = Math::Smooth(m_soundFrequencyMotor2, freq, rTime*factor);
        m_soundAmplitudeMotor2 = Math::Linear(m_soundAmplitudeMotor2, 1.0f, rTime*factor);

        RPM = (m_soundFrequencyMotor1-0.5f)/0.5f;
        RPM = 0.2f+RPM*0.8f;
        if ( false /* TODO (krzys_h): m_object->GetStarting() */ )  // bloqué pendant 3,2,1,go ?
        {
            RPM = log10f((RPM+1.0f))*2.5f;
        }
        factor = Math::Norm(fabs(m_linMotion.realSpeed.x)/50.0f);
        RPM += sinf(m_motorAngle)*0.03f*(1.0f-factor*0.5f);
        m_motorRPM = Math::Smooth(m_motorRPM, RPM, rTime*10.0f);

        factor = 1.0f;

#if 1
        if ( m_soundAmplitudeMotor1 > 0.0f )
        {
            if ( m_soundChannelMotor1 == -1 )
            {
                if ( !m_bWater )  m_sound->Play(SOUND_MOTORs, m_object->GetPosition());
                m_soundChannelMotor1 = m_sound->Play(sound, m_object->GetPosition(), 1.0f, 1.0f, true);
            }
            m_sound->Position(m_soundChannelMotor1, m_object->GetPosition());
            m_sound->Frequency(m_soundChannelMotor1, m_soundFrequencyMotor1*factor);
            m_sound->Amplitude(m_soundChannelMotor1, m_soundAmplitudeMotor1*factor);
        }
        else
        {
            if ( m_soundChannelMotor1 != -1 )
            {
                m_sound->Stop(m_soundChannelMotor1);
                m_soundChannelMotor1 = -1;
            }
        }
#endif

#if 0
        if ( m_soundAmplitudeMotor2 > 0.0f )
    {
        if ( m_soundChannelMotor2 == -1 )
        {
            m_soundChannelMotor2 = m_sound->Play(SOUND_MOTOR2, m_object->GetPosition(), 1.0f, 1.0f, true);
        }
        m_sound->Position(m_soundChannelMotor2, m_object->GetPosition());
        m_sound->Frequency(m_soundChannelMotor2, m_soundFrequencyMotor2*factor);
        m_sound->Amplitude(m_soundChannelMotor2, m_soundAmplitudeMotor2*factor);
    }
    else
    {
        if ( m_soundChannelMotor2 != -1 )
        {
            m_sound->Stop(m_soundChannelMotor2);
            m_soundChannelMotor2 = -1;
        }
    }
#endif

        if ( type == OBJECT_TRAX )
        {
            m_soundTimePshhh -= rTime;

            if ( m_soundTimePshhh <= 0.0f )
            {
                m_sound->Play(SOUND_PSHHH, m_object->GetPosition(), 1.0f);
                m_soundTimePshhh = 4.0f+4.0f*Math::Rand();

                max = static_cast<int>(10.0f*m_engine->GetParticleDensity());
                mat = m_object->GetWorldMatrix(3);  // pelle
                for ( i=0 ; i<max ; i++ )
                {
                    if ( Math::Rand() < 0.5f )  dir =  1.0f;
                    else                  dir = -1.0f;
                    pos = Math::Vector(6.0f, 1.0f, 10.0f*dir);
                    pos.x += (Math::Rand()-0.5f)*4.0f;
                    pos.z += (Math::Rand()-0.5f)*2.0f;
                    speed = pos;
                    speed.x += (Math::Rand()-0.5f)*6.0f;
                    speed.z += Math::Rand()*12.0f*dir;
                    speed.y += Math::Rand()*6.0f;
                    pos   = Transform(*mat, pos);
                    speed = Transform(*mat, speed)-pos;
                    dim.x = Math::Rand()*3.0f+3.0f;
                    dim.y = dim.x;
                    m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIMOTOR, 2.0f);
                }
            }
        }
        return;
    }

    SoundType   sound;
    float       amplitude, time, freq;

    if ( type == OBJECT_MOBILEia ||
         type == OBJECT_MOBILEic ||
         type == OBJECT_MOBILEii ||
         type == OBJECT_MOBILEis )
    {
        if ( m_soundChannel == -1 )
        {
            m_soundChannel = m_sound->Play(SOUND_MOTORi, m_object->GetPosition(), 0.0f, 1.0f, true);
            m_sound->AddEnvelope(m_soundChannel, 1.0f, 1.0f, 0.2f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 1.0f, 1.0f, 1.0f, SOPER_LOOP);
        }
        else
        {
            m_sound->Position(m_soundChannel, m_object->GetPosition());
        }

        freq = 1.0f+m_linMotion.terrainSpeed.x/50.0f;
        if ( m_linMotion.realSpeed.x == 0.0f )
        {
            freq -= fabs(m_cirMotion.realSpeed.y/3.0f);
        }
        else
        {
            freq -= fabs(m_cirMotion.realSpeed.y/4.0f);
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

    if ( m_object->GetToy() )
    {
        sound = SOUND_MOTORd;
        amplitude = 1.0f;
        time = 0.1f;
    }

    freq = 0.75f+(fabs(m_motorSpeed.x)+fabs(m_motorSpeed.z))*0.25f;
    if ( freq > 1.0f )  freq = 1.0f;
    if ( m_object->GetToy() )  freq = 1.0f;

    if ( m_soundChannel == -1 )
    {
        m_soundChannel = m_sound->Play(sound, m_object->GetPosition(), 0.0f, 0.5f, true);
        m_sound->AddEnvelope(m_soundChannel, amplitude, freq, time, SOPER_CONTINUE);
        m_sound->AddEnvelope(m_soundChannel, amplitude, freq, 1.0f, SOPER_LOOP);
    }
    else
    {
        m_sound->Position(m_soundChannel, m_object->GetPosition());

        if ( m_bSoundSlow )  // in slow motion?
        {
            m_sound->FlushEnvelope(m_soundChannel);
            m_sound->AddEnvelope(m_soundChannel, amplitude, freq, time, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, amplitude, freq, 1.0f, SOPER_LOOP);
            m_bSoundSlow = false;
        }
    }

    freq *= 1.0f + m_linMotion.terrainSpeed.x/100.0f;
    freq *= 1.0f + fabs(m_cirMotion.realSpeed.y/20.0f);
    m_sound->Frequency(m_soundChannel, freq);

    m_soundTimePshhh -= rTime*2.0f;
}

// Sounds the engine idling.

void CPhysics::SoundMotorSlow(float rTime, ObjectType type)
{
    // TODO (krzys_h): check this code
    if (m_type != TYPE_NORMAL)
    {
        Character*    character;
        SoundType        sound;
        float        factor, RPM;

        if ( type == OBJECT_HUMAN   ||
             type == OBJECT_TECH    ||
             type == OBJECT_UFO     ||
             type == OBJECT_CARROT  ||
             type == OBJECT_STARTER ||
             type == OBJECT_WALKER  ||
             type == OBJECT_CRAZY   ||
             type == OBJECT_GUIDE   ||
             type == OBJECT_EVIL1   ||
             type == OBJECT_EVIL2   ||
             type == OBJECT_EVIL3   ||
             type == OBJECT_EVIL4   ||
             type == OBJECT_EVIL5   )  return;

        character = m_object->GetCharacter();
        sound = SOUND_MOTOR1;
        if ( character->motorSound == 1 )  sound = SOUND_MOTOR1;
        if ( character->motorSound == 2 )  sound = SOUND_MOTOR2;
        if ( character->motorSound == 3 )  sound = SOUND_MOTOR3;
        if ( character->motorSound == 4 )  sound = SOUND_MOTOR4;
        if ( character->motorSound == 5 )  sound = SOUND_MOTOR5;
        if ( character->motorSound == 6 )  sound = SOUND_MOTOR6;

        if ( false /* TODO (krzys_h): m_object->GetStarting() */ )  // bloqué pendant 3,2,1,go ?
        {
            factor = 1.5f;
        }
        else
        {
            factor = 0.5f;
        }

        m_soundFrequencyMotor1 = Math::Linear(m_soundFrequencyMotor1, 0.5f, rTime*factor);
        m_soundAmplitudeMotor1 = Math::Linear(m_soundAmplitudeMotor1, 0.8f, rTime*factor);

        m_soundFrequencyMotor2 = Math::Linear(m_soundFrequencyMotor2, 0.5f, rTime*factor);
        m_soundAmplitudeMotor2 = Math::Linear(m_soundAmplitudeMotor2, 0.8f, rTime*factor);

        m_motorState = 0;

        RPM = (m_soundFrequencyMotor1-0.5f)/0.5f;
        RPM = 0.2f+RPM*0.8f;
        if ( false /* TODO (krzys_h): m_object->GetStarting() */ )  // bloqué pendant 3,2,1,go ?
        {
            RPM = log10f((RPM+1.0f))*2.5f;
        }
        factor = Math::Norm(fabs(m_linMotion.realSpeed.x)/50.0f);
        RPM += sinf(m_motorAngle)*0.03f*(1.0f-factor*0.5f);
        m_motorRPM = Math::Smooth(m_motorRPM, RPM, rTime*10.0f);

        factor = 1.0f;

#if 1
        if ( m_soundAmplitudeMotor1 > 0.0f )
        {
            if ( m_soundChannelMotor1 == -1 )
            {
                if ( !m_bWater )  m_sound->Play(SOUND_MOTORs, m_object->GetPosition());
                m_soundChannelMotor1 = m_sound->Play(sound, m_object->GetPosition(), 1.0f, 1.0f, true);
            }
            m_sound->Position(m_soundChannelMotor1, m_object->GetPosition());
            m_sound->Frequency(m_soundChannelMotor1, m_soundFrequencyMotor1*factor);
            m_sound->Amplitude(m_soundChannelMotor1, m_soundAmplitudeMotor1*factor);
        }
        else
        {
            if ( m_soundChannelMotor1 != -1 )
            {
                m_sound->Stop(m_soundChannelMotor1);
                m_soundChannelMotor1 = -1;
            }
        }
#endif

#if 0
        if ( m_soundAmplitudeMotor2 > 0.0f )
        {
            if ( m_soundChannelMotor2 == -1 )
            {
                m_soundChannelMotor2 = m_sound->Play(SOUND_MOTOR2, m_object->GetPosition(), 1.0f, 1.0f, true);
            }
            m_sound->Position(m_soundChannelMotor2, m_object->GetPosition());
            m_sound->Frequency(m_soundChannelMotor2, m_soundFrequencyMotor2*factor);
            m_sound->Amplitude(m_soundChannelMotor2, m_soundAmplitudeMotor2*factor);
        }
        else
        {
            if ( m_soundChannelMotor2 != -1 )
            {
                m_sound->Stop(m_soundChannelMotor2);
                m_soundChannelMotor2 = -1;
            }
        }
#endif
        return;
    }

    Math::Matrix*   mat;
    Math::Vector    pos, speed;
    Math::Point     dim;
    SoundType       sound;
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

    if ( m_object->GetToy() )
    {
        sound = SOUND_MOTORd;
        amplitude = 0.0f;
    }

    if ( m_soundChannel == -1 )
    {
        m_soundChannel = m_sound->Play(sound, m_object->GetPosition(), 0.0f, 0.25f, true);
        m_sound->AddEnvelope(m_soundChannel, amplitude, 0.5f, 0.2f, SOPER_CONTINUE);
        m_sound->AddEnvelope(m_soundChannel, amplitude, 0.5f, 1.0f, SOPER_LOOP);
    }
    else
    {
        m_sound->Position(m_soundChannel, m_object->GetPosition());

        if ( !m_bSoundSlow )  // full power?
        {
            m_sound->FlushEnvelope(m_soundChannel);
            m_sound->AddEnvelope(m_soundChannel, amplitude, 0.5f, 0.3f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, amplitude, 0.5f, 1.0f, SOPER_LOOP);
            m_bSoundSlow = true;
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
//?         m_sound->Play(SOUND_PSHHH, m_object->GetPosition(), amplitude);
            m_sound->Play(SOUND_PSHHH, m_object->GetPosition(), 1.0f);

            m_soundTimePshhh = 4.0f+4.0f*Math::Rand();

            max = static_cast<int>(10.0f*m_engine->GetParticleDensity());
            for ( i=0 ; i<max ; i++ )
            {
                pos = Math::Vector(-5.0f, 2.0f, 0.0f);
                pos.x += Math::Rand()*4.0f;
                pos.z += (Math::Rand()-0.5f)*2.0f;

                speed = pos;
                speed.x -= Math::Rand()*4.0f;
                speed.y -= Math::Rand()*3.0f;
                speed.z += (Math::Rand()-0.5f)*6.0f;

                mat = m_object->GetWorldMatrix(0);
                pos   = Transform(*mat, pos);
                speed = Transform(*mat, speed)-pos;

                dim.x = Math::Rand()*1.0f+1.0f;
                dim.y = dim.x;

                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIMOTOR, 2.0f);
            }
        }
    }
}

// Sounds the engine not running.

void CPhysics::SoundMotorStop(float rTime, ObjectType type)
{
    // TODO (krzys_h): check this code
    if (m_type != TYPE_NORMAL)
    {
        Character*    character;
        SoundType        sound;
        float        factor, RPM;

        if ( type == OBJECT_HUMAN   ||
             type == OBJECT_TECH    ||
             type == OBJECT_UFO     ||
             type == OBJECT_CARROT  ||
             type == OBJECT_STARTER ||
             type == OBJECT_WALKER  ||
             type == OBJECT_CRAZY   ||
             type == OBJECT_GUIDE   ||
             type == OBJECT_EVIL1   ||
             type == OBJECT_EVIL2   ||
             type == OBJECT_EVIL3   ||
             type == OBJECT_EVIL4   ||
             type == OBJECT_EVIL5   )  return;

        character = m_object->GetCharacter();
        sound = SOUND_MOTOR1;
        if ( character->motorSound == 1 )  sound = SOUND_MOTOR1;
        if ( character->motorSound == 2 )  sound = SOUND_MOTOR2;
        if ( character->motorSound == 3 )  sound = SOUND_MOTOR3;
        if ( character->motorSound == 4 )  sound = SOUND_MOTOR4;
        if ( character->motorSound == 5 )  sound = SOUND_MOTOR5;
        if ( character->motorSound == 6 )  sound = SOUND_MOTOR6;

        m_soundFrequencyMotor1 = Math::Linear(m_soundFrequencyMotor1, 0.3f, rTime*0.3f);
        m_soundAmplitudeMotor1 = Math::Linear(m_soundAmplitudeMotor1, 0.0f, rTime*1.0f);

        m_soundFrequencyMotor2 = Math::Linear(m_soundFrequencyMotor2, 0.5f, rTime*0.3f);
        m_soundAmplitudeMotor2 = Math::Linear(m_soundAmplitudeMotor2, 0.0f, rTime*1.0f);

        m_motorState = 0;

        RPM = (m_soundFrequencyMotor1-0.5f)/0.5f;
        RPM = 0.2f+RPM*0.8f;
        factor = Math::Norm(fabs(m_linMotion.realSpeed.x)/50.0f);
        RPM += sinf(m_motorAngle)*0.03f*(1.0f-factor*0.5f);
        m_motorRPM = Math::Smooth(m_motorRPM, RPM, rTime*10.0f);

        factor = 1.0f;

#if 1
        if ( m_soundAmplitudeMotor1 > 0.0f )
        {
            if ( m_soundChannelMotor1 == -1 )
            {
                m_soundChannelMotor1 = m_sound->Play(sound, m_object->GetPosition(), 1.0f, 1.0f, true);
            }
            m_sound->Position(m_soundChannelMotor1, m_object->GetPosition());
            m_sound->Frequency(m_soundChannelMotor1, m_soundFrequencyMotor1*factor);
            m_sound->Amplitude(m_soundChannelMotor1, m_soundAmplitudeMotor1*factor);
        }
        else
        {
            if ( m_soundChannelMotor1 != -1 )
            {
                m_sound->Stop(m_soundChannelMotor1);
                m_soundChannelMotor1 = -1;
            }
        }
#endif

#if 0
        if ( m_soundAmplitudeMotor2 > 0.0f )
    {
        if ( m_soundChannelMotor2 == -1 )
        {
            m_soundChannelMotor2 = m_sound->Play(SOUND_MOTOR2, m_object->GetPosition(), 1.0f, 1.0f, true);
        }
        m_sound->Position(m_soundChannelMotor2, m_object->GetPosition());
        m_sound->Frequency(m_soundChannelMotor2, m_soundFrequencyMotor2*factor);
        m_sound->Amplitude(m_soundChannelMotor2, m_soundAmplitudeMotor2*factor);
    }
    else
    {
        if ( m_soundChannelMotor2 != -1 )
        {
            m_sound->Stop(m_soundChannelMotor2);
            m_soundChannelMotor2 = -1;
        }
    }
#endif
        return;
    }

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
    SoundType      sound;
    Math::Matrix*   mat;
    Math::Vector    pos, speed;
    Math::Point     dim;
    float       freq;
    int         i;

    // TODO (krzys_h): check this code
    if (m_type != TYPE_NORMAL)
    {
        if ( m_soundChannelMotor1 == -1 )
        {
            sound = SOUND_FLY;
            m_soundChannelMotor1 = m_sound->Play(sound, m_object->GetPosition(), 0.0f, 1.0f, true);
            m_sound->AddEnvelope(m_soundChannelMotor1, 1.0f, 1.0f, 0.6f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannelMotor1, 1.0f, 1.0f, 1.0f, SOPER_LOOP);
        }
        else
        {
            m_sound->Position(m_soundChannelMotor1, m_object->GetPosition());
        }

        freq = 1.0f + m_linMotion.realSpeed.y/100.0f;
        freq *= 1.0f + fabs(m_cirMotion.realSpeed.y/5.0f);
        m_sound->Frequency(m_soundChannelMotor1, freq);
        return;
    }

    if ( m_soundChannelSlide != -1 )  // slides?
    {
        m_sound->FlushEnvelope(m_soundChannelSlide);
        m_sound->AddEnvelope(m_soundChannelSlide, 0.0f, 1.0f, 0.3f, SOPER_STOP);
        m_soundChannelSlide = -1;
    }

    if ( !m_object->Implements(ObjectInterfaceType::JetFlying) || dynamic_cast<CJetFlyingObject*>(m_object)->GetReactorRange() > 0.0f )
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

            m_soundChannel = m_sound->Play(sound, m_object->GetPosition(), 0.0f, 1.0f, true);
            m_sound->AddEnvelope(m_soundChannel, 1.0f, 1.0f, 0.6f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 1.0f, 1.0f, 1.0f, SOPER_LOOP);
        }
        else
        {
            m_sound->Position(m_soundChannel, m_object->GetPosition());
        }

        freq = 1.0f + m_linMotion.realSpeed.y/100.0f;
        freq *= 1.0f + fabs(m_cirMotion.realSpeed.y/5.0f);
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
            freq = 1.0f+Math::Rand()*0.5f;
            m_sound->Play(SOUND_FLYf, m_object->GetPosition(), 1.0f, freq);
            m_camera->StartEffect(Gfx::CAM_EFFECT_PET, m_object->GetPosition(), 1.0f);

            for ( i=0 ; i<5 ; i++ )
            {
                if ( m_object->GetType() == OBJECT_HUMAN ||
                     m_object->GetType() == OBJECT_TECH  )
                {
                    pos = Math::Vector(-1.6f, -0.5f, 0.0f);
                }
                else
                {
                    pos = Math::Vector(0.0f, -1.0f, 0.0f);
                }
                pos.x += (Math::Rand()-0.5f)*2.0f;
                pos.z += (Math::Rand()-0.5f)*2.0f;
                mat = m_object->GetWorldMatrix(0);
                pos = Transform(*mat, pos);
                speed.x = (Math::Rand()-0.5f)*5.0f;
                speed.z = (Math::Rand()-0.5f)*5.0f;
                speed.y = -(4.0f+Math::Rand()*4.0f);
                dim.x = (2.0f+Math::Rand()*1.0f);
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISMOKE1, 2.0f, 0.0f, 0.1f);
            }

            m_timeReactorFail = m_time+0.10f+Math::Rand()*0.30f;
        }
        else
        {
            if ( m_object->GetType() == OBJECT_HUMAN ||
                 m_object->GetType() == OBJECT_TECH  )
            {
                pos = Math::Vector(-1.6f, -0.5f, 0.0f);
            }
            else
            {
                pos = Math::Vector(0.0f, -1.0f, 0.0f);
            }
            pos.x += (Math::Rand()-0.5f)*1.0f;
            pos.z += (Math::Rand()-0.5f)*1.0f;
            mat = m_object->GetWorldMatrix(0);
            pos = Transform(*mat, pos);
            speed.x = (Math::Rand()-0.5f)*2.0f;
            speed.z = (Math::Rand()-0.5f)*2.0f;
            speed.y = -(4.0f+Math::Rand()*4.0f);
            dim.x = (0.7f+Math::Rand()*0.4f);
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISMOKE1, 2.0f, 0.0f, 0.1f);
        }
    }

}

// Sounds the reactor stopped.

void CPhysics::SoundReactorStop(float rTime, ObjectType type)
{
    // TODO (krzys_h): check this code
    if (m_type != TYPE_NORMAL)
    {
        if ( m_soundChannelMotor1 != -1 )  // moteur tourne ?
        {
            m_sound->FlushEnvelope(m_soundChannelMotor1);
            m_sound->AddEnvelope(m_soundChannelMotor1, 0.0f, 1.0f, 1.0f, SOPER_STOP);
            m_soundChannelMotor1 = -1;
        }
        return;
    }

    float energy = GetObjectEnergy(m_object);

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
                m_soundChannelSlide = m_sound->Play(SOUND_SLIDE, m_object->GetPosition(), 0.0f, 1.0f, true);
                m_sound->AddEnvelope(m_soundChannelSlide, 0.5f, 1.0f, 0.3f, SOPER_CONTINUE);
                m_sound->AddEnvelope(m_soundChannelSlide, 0.5f, 1.0f, 1.0f, SOPER_LOOP);
            }
            m_sound->Position(m_soundChannelSlide, m_object->GetPosition());
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
                          Math::Vector &pos, Math::Vector &angle)
{
    Character*  character;
    ObjectType  type;
    Math::Vector    norm, iPos, nAngle;
    Math::Matrix    matRotate;
    Math::Point        v;
    float        level, h, f, a1, volume, freq, force=0, centri, speed, accel, mass;
    bool        bSlopingTerrain;

    type = m_object->GetType();
    character = m_object->GetCharacter();

    level = m_water->GetLevel(m_object);
    SetSwim( pos.y < level );

    m_floorLevel = m_terrain->GetFloorLevel(pos);  // height above the ground
    h = pos.y-m_floorLevel;
    h -= character->height;
    m_floorHeight = h;

    WaterParticle(aTime, pos, type, m_floorLevel,
                   fabs(m_linMotion.realSpeed.x),
                   fabs(m_cirMotion.realSpeed.y*15.0f));

    if ( !m_object->Implements(ObjectInterfaceType::Flying) )
    {
        pos.y -= h;  // plate to the ground immediately
        pos.y += character->height;
        m_floorHeight = 0.0f;
    }

    if ( m_object->Implements(ObjectInterfaceType::Flying) )
    {
        bSlopingTerrain = false;  // ground as possible to land

        if ( !m_bLand )  // in flight?
        {
            m_terrain->GetNormal(norm, pos);
            a1 = fabs(Math::RotateAngle(Math::Point(norm.x, norm.z).Length(), norm.y));
            if ( a1 < (90.0f-55.0f)*Math::PI/180.0f )  // slope exceeds 55 degrees?
            {
                bSlopingTerrain = true;  // very sloped ground

                if ( h < 4.0f )  // collision with the ground?
                {
                    force = 5.0f+fabs(m_linMotion.realSpeed.x*0.3f)+
                                 fabs(m_linMotion.realSpeed.y*0.3f);
                    m_linMotion.currentSpeed = norm*force;
                    Math::LoadRotationXZYMatrix(matRotate, -angle);
                    m_linMotion.currentSpeed = Transform(matRotate, m_linMotion.currentSpeed);

                    if ( aTime-m_soundTimeBoum > 0.5f )
                    {
                        volume = fabs(m_linMotion.realSpeed.x*0.02f)+
                                 fabs(m_linMotion.realSpeed.y*0.02f);
                        freq = 0.5f+m_terrain->GetHardness(pos)*2.5f;
                        m_sound->Play(SOUND_BOUM, pos, volume, freq);

                        m_soundTimeBoum = aTime;
                    }

//?                 pos = m_object->GetPosition();  // gives position before collision
                }
            }
        }

        if ( (h <= 0.0f || m_bLand) && !bSlopingTerrain )  // on the ground?
        {
            if ( !m_bLand )  // in flight?
            {
                volume = fabs(m_linMotion.realSpeed.y*0.02f);
                freq = 0.5f+m_terrain->GetHardness(pos)*2.5f;
                m_sound->Play(SOUND_BOUM, pos, volume, freq);
            }

            m_bLand = true;  // on the ground?
            SetMotor(false);
            pos.y -= h;  // plate to the ground immediately
            m_floorHeight = 0.0f;

            if ( h < 0.0f )
            {
                f = fabs(m_linMotion.currentSpeed.y/m_linMotion.advanceSpeed.y);
                CrashParticle(f, false);
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


    if ( m_type == TYPE_RACE )
    {
        iPos = m_object->GetPosition();
        mass = character->mass;
        if ( m_bWater )  mass *= 0.1f;
//?        pos.y = iPos.y + (m_graviSpeed-mass/500.0f)*rTime;
        pos.y = iPos.y + (m_graviSpeed-mass/15.0f*rTime)*rTime;
        if ( pos.y < m_floorLevel )  pos.y = m_floorLevel;  // pas sous le sol
        m_floorHeight = h = pos.y-m_floorLevel;

        speed = (pos.y-iPos.y)/rTime;
        accel = (speed-m_graviSpeed)/rTime;  // positif si tombe
        m_graviSpeed = speed;
        if ( m_graviSpeed > 0.0f )  // monte ?
        {
            m_graviSpeed *= 1.0f-m_graviGlu;  // colle au sol si choc récent
        }

        m_graviGlu -= rTime*10.0f;
        if ( m_graviGlu < 0.0f )  m_graviGlu = 0.0f;

        if ( m_floorHeight == 0.0f )  // au sol ?
        {
            if ( accel > 100.0f )
            {
//?                force = Math::Norm((accel-100.0f)/100.0f);
                force = Math::Norm((accel-100.0f)/1000.0f);
                CrashParticle(force, true);  // poussičre si tombe
            }
            if ( accel > 200.0f && !m_bWater )
            {
                volume = Math::Norm((accel-200.0f)/200.0f);
                m_sound->Play(SOUND_BOUMv, pos, volume);  // bruit si tombe

                if ( m_main->GetPhase() == PHASE_SIMUL )
                {
                    if ( m_terrainHard >= 0.5f )  // sol dur ?
                    {
                        if ( accel > 2000.0f )
                        {
                            m_sound->Play(SOUND_FALLg, pos, 0.8f);
                        }
                    }
                    else
                    {
                        volume = Math::Norm((accel-200.0f)/2000.0f);
                        if ( volume > 0.6f )  volume = 0.6f;
                        m_sound->Play(SOUND_FALLg, pos, volume);
                    }
                }
            }

            SuspForce(0, -accel, 0.0f, rTime);
        }
        else    // en l'air ?
        {
            SuspForce(0, 0.0f, 0.0f, rTime);
        }

        // Calcule la force centripčte qui déporte le véhicule
        // vers l'extérieur lors d'un virage.
        if ( m_floorHeight == 0.0f )
        {
            if ( m_linMotion.realSpeed.x >= 0.0f )  // avance ?
            {
                speed = Math::Norm(fabs(m_linMotion.realSpeed.x)/m_linMotion.advanceSpeed.x);
                force = speed*m_cirMotion.realSpeed.y;
                f = 1.0f-Math::Norm(speed);  // gripLimit -> 0 (si vitesse élevée)
                if ( fabs(force) > character->gripLimit*f )
                {
                    if ( force > 0.0f )  force -= character->gripLimit*f;
                    else                 force += character->gripLimit*f;
                    force *= character->gripSlide;
                    centri = force*(1.0f-m_terrainHard*0.5f);
#if 0
                    if ( centri > 0.0f )
                    {
                        if ( centri > m_centriSpeed )  f = 2.0f;
                        else                           f = 4.0f;
                    }
                    else
                    {
                        if ( centri < m_centriSpeed )  f = 2.0f;
                        else                           f = 4.0f;
                    }
#else
//?                    f = 2.0f+2.0f*speed;
                    f = 2.0f+m_terrainHard*speed;  // brusque si vitesse haute
#endif
                    if ( m_bBrake )
                    {
                        f *= 1.0f-Math::Norm(fabs(centri/character->gripSlide));
                    }
                }
                else
                {
                    centri = 0.0f;
                    f = 4.0f;
                }
                if ( m_bHandbrake )  // frein ŕ main?
                {
                    centri *= 2.0f;
//?                    centri = 0.0f;
//?                    f = 3.0f;
                }
                if ( centri >  40.0f )  centri =  40.0f;
                if ( centri < -40.0f )  centri = -40.0f;
                m_centriSpeed = Math::SmoothP(m_centriSpeed, centri, rTime*f);

                if ( speed < 0.1f )  // presque arręté ?
                {
                    m_centriSpeed *= speed/0.1f;  // plus de force centripčte
                }
            }
            else    // recule ?
            {
                m_centriSpeed = 0.0f;
            }

            m_overTurn = m_centriSpeed*character->overFactor/character->gripSlide;
            if ( m_overTurn >  character->overAngle )  m_overTurn =  character->overAngle;
            if ( m_overTurn < -character->overAngle )  m_overTurn = -character->overAngle;
        }

        if ( m_floorHeight == 0.0f && force != 0.0f )
        {
            force = Math::Norm(fabs(m_centriSpeed/20.0f));
            SlideParticle(aTime, rTime, force*(1.0f-m_terrainHard), force*m_terrainHard);
        }
        else
        {
            SlideParticle(aTime, rTime, 0.0f, 0.0f);
        }

        if ( m_floorHeight == 0.0f )  // plaqué au sol ?
        {
            if ( m_bBrake )
            {
                // traces de frein
//?                force = fabs(m_linMotion.realSpeed.x)/m_linMotion.advanceSpeed.x;
                force = fabs(m_motorSpeed.x);
                SuspForce(1, -force, 1.0f, rTime);
                if ( force <= 0.5f )
                {
                    WheelParticle(aTime, rTime, 0.0f, true);
                }
                else
                {
                    force = (force-0.5f)/0.5f;
                    WheelParticle(aTime, rTime, -m_motorSpeed.x*force, true);
                }
            }
            else if ( m_wheelSlide > 0.0f )
            {
                // patine au démarrage
//?                WheelParticle(aTime, rTime, 1.0f+m_wheelSlide*0.5f, false);
                WheelParticle(aTime, rTime, m_wheelSlide*2.0f, false);
            }
            else
            {
                WheelParticle(aTime, rTime, 0.0f, false);
            }
        }
        else
        {
            WheelParticle(aTime, rTime, 0.0f, false);
        }

        SuspParticle(aTime, rTime);

        // Calcule la suspension.
        for ( int j=0 ; j<2 ; j++ )
        {
            force = 0.0f;
            for ( int i=0 ; i<5 ; i++ )
            {
                force += m_suspEnergy[j][i];  // somme des forces
            }
            force = Math::NormSign(force);

            if ( fabs(force) >= fabs(m_suspDelayed[j]) )
            {
                m_suspDelayed[j] = force;
                m_suspHeight[j] = m_suspDelayed[j];
                m_suspTime[j] = 0.0f;
            }
            else
            {
                m_suspDelayed[j] = Math::Smooth(m_suspDelayed[j], force, rTime*character->suspAbsorber*2.0f);
                m_suspHeight[j] = cosf(m_suspTime[j]*character->suspFrequency)*Math::NormSign(m_suspDelayed[j]);
                m_suspTime[j] += rTime;
            }

            for ( int i=0 ; i<5 ; i++ )
            {
                if ( i == 0)  continue;  // force du terrain ?
                m_suspEnergy[j][i] = Math::Smooth(m_suspEnergy[j][i], 0.0f, rTime*character->suspAbsorber);
            }

            m_suspHeight[j] *= character->suspHeight;
            if ( m_camera->GetType() == Gfx::CAM_TYPE_ONBOARD )  m_suspHeight[j] *= 0.5f;
            if ( m_object->GetScaleY() < 1.0f )  // tout plat ?
            {
                m_suspHeight[j] = 0.0f;
            }
        }

        // Calcule la rotation aprčs un choc.
        if ( m_chocSpin != 0.0f )  // rotation en cours ?
        {
            m_chocAngle += m_chocSpin*rTime;
            m_object->SetCirChoc(Math::Vector(0.0f, m_chocAngle, 0.0f));

            if ( m_chocSpin > 0.0f )
            {
                m_chocSpin -= rTime*(8.0f+m_terrainHard*5.0f);
                if ( m_chocSpin < 0.0f )  m_chocSpin = 0.0f;
            }
            if ( m_chocSpin < 0.0f )
            {
                m_chocSpin += rTime*(8.0f+m_terrainHard*5.0f);
                if ( m_chocSpin > 0.0f )  m_chocSpin = 0.0f;
            }

            if ( m_chocSpin == 0.0f )  // rotation terminée ?
            {
                angle.y += m_chocAngle;
                m_object->SetCirChoc(Math::Vector(0.0f, 0.0f, 0.0f));
            }
        }
    }

    if ( m_type == TYPE_TANK )
    {
        if ( type == OBJECT_WALKER ||
             type == OBJECT_CRAZY  )
        {
            if ( m_linMotion.realSpeed.x != 0.0f &&  // marche ?
                 !IsObjectBeingTransported(m_object) )
            {
                pos.y -= h;  // plaque immédiatement au sol
                m_floorHeight = 0.0f;
            }
        }
        else
        {
            pos.y -= h;  // plaque immédiatement au sol
            m_floorHeight = 0.0f;
        }
    }

    if ( m_floorHeight == 0.0f )  // ground plate?
    {
        CTraceDrawingObject* traceDrawing = nullptr;
        if (m_object->Implements(ObjectInterfaceType::TraceDrawing))
            traceDrawing = dynamic_cast<CTraceDrawingObject*>(m_object);

        if (traceDrawing != nullptr && traceDrawing->GetTraceDown())
        {
            DrawingPenParticle(traceDrawing->GetTraceColor(), traceDrawing->GetTraceWidth()*g_unit);
        }
        else
        {
            DrawingPenParticle(TraceColor::Default, 0.0f);
        }
    }

    if ( type == OBJECT_HUMAN ||
         type == OBJECT_TECH  ||
         type == OBJECT_WORM  )  return;  // always right

    if ( m_type == TYPE_RACE )
    {
        if ( h > 0.0f )  // en l'air ?
        {
            f = h/character->suspDetect;
            if ( f < 1.0f )  // presque au sol ?
            {
                nAngle = angle;
                FloorAngle(pos, nAngle);  // adapte l'angle au terrain
                angle.x = angle.x*f + nAngle.x*(1.0f-f);
                angle.z = angle.z*f + nAngle.z*(1.0f-f);
            }
            return;
        }
    }

    if ( m_type == TYPE_MASS )
    {
        iPos = m_object->GetPosition();
        if ( pos.y < m_floorLevel )  // collision avec le sol ?
        {
            pos.y = m_floorLevel;  // pas sous le sol
//?            m_linMotion.realSpeed.y = -m_linMotion.realSpeed.y;
        }
        m_floorHeight = pos.y-m_floorLevel;
    }

    FloorAngle(pos, angle);  // adjusts the angle at the ground

    if ( m_object->Implements(ObjectInterfaceType::Flying) && !m_bLand )  // flying in the air?
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

// Force actionnée sur la suspension. Une force négative effectue une
// pression vers le bas (écrasement).
//    rank=0 -> force du terrain
//    rank=1 -> force de freinage
//    rank=2 -> force des effets (objet posé dans véhicule)

void CPhysics::SuspForce(int rank, float force, float front, float rTime)
{
    float    factor;

    if ( rank == 0 )  // force du terrain ?
    {
        factor = Math::NormSign(1.0f+front*2.0f);  // force sur l'avant
        m_suspEnergy[0][rank] = Math::NormSign(Math::Smooth(m_suspEnergy[0][rank], force*factor/20.0f, rTime));

        factor = Math::NormSign(1.0f-front*2.0f);  // force sur l'arričre
        m_suspEnergy[1][rank] = Math::NormSign(Math::Smooth(m_suspEnergy[1][rank], force*factor/20.0f, rTime));
    }
    else    // autre force ?
    {
        factor = Math::NormSign(1.0f+front*2.0f);  // force sur l'avant
        m_suspEnergy[0][rank] = Math::NormSign(m_suspEnergy[0][rank]+force*factor);

        factor = Math::NormSign(1.0f-front*2.0f);  // force sur l'arričre
        m_suspEnergy[1][rank] = Math::NormSign(m_suspEnergy[1][rank]+force*factor);
    }
}

// Calculates the angle of an object with the field.

/*
void CPhysics::FloorAngle(const Math::Vector &pos, Math::Vector &angle)
{
    Character*  character;
    Math::Vector    pw, norm, choc;
    Math::Point nn;
    float       a, a1, a2;

    a = angle.y;

    choc = m_object->GetCirChoc();  // tient compte rotation choc
    a += choc.y;

    if ( m_type == TYPE_MASS )
    {
        if ( m_floorHeight == 0.0f &&  // au sol ?
             m_terrain->GetNormal(norm, pos) )
        {
            nn = RotatePoint(-a, Math::Point(norm.z, norm.x));
            angle.x =  sinf(nn.x);
            angle.z = -sinf(nn.y);
        }

        return;
    }

    character = m_object->GetCharacter();

    // TODO: Convert old wheelXXX to new format? ~krzys_h
    float wheelFront = character->wheelFront;
    float wheelBack = character->wheelBack;
    float wheelLeft = character->wheelLeft;
    float wheelRight = character->wheelRight;
    if (m_type != TYPE_NORMAL)
    {
        wheelFront = character->wheelFrontPos.x;
        wheelBack = character->wheelBackPos.z;
        wheelLeft = character->wheelBackPos.z;
        wheelRight = character->wheelBackPos.z;
    }

    pw.x = pos.x+wheelFront*cosf(a+Math::PI*0.0f);
    pw.y = pos.y;
    pw.z = pos.z-wheelFront*sinf(a+Math::PI*0.0f);
    a1 = atanf(m_terrain->GetHeightToFloor(pw)/wheelFront);

    pw.x = pos.x-wheelBack*cosf(a+Math::PI*1.0f);
    pw.y = pos.y;
    pw.z = pos.z+wheelBack*sinf(a+Math::PI*1.0f);
    a2 = atanf(m_terrain->GetHeightToFloor(pw)/wheelBack);

    angle.z = (a2-a1)/2.0f;

    pw.x = pos.x+wheelLeft*cosf(a+Math::PI*0.5f)*cosf(angle.z);
    pw.y = pos.y;
    pw.z = pos.z-wheelLeft*sinf(a+Math::PI*0.5f)*cosf(angle.z);
    a1 = atanf(m_terrain->GetHeightToFloor(pw)/wheelLeft);

    pw.x = pos.x+wheelRight*cosf(a+Math::PI*1.5f)*cosf(angle.z);
    pw.y = pos.y;
    pw.z = pos.z-wheelRight*sinf(a+Math::PI*1.5f)*cosf(angle.z);
    a2 = atanf(m_terrain->GetHeightToFloor(pw)/wheelRight);

    angle.x = (a2-a1)/2.0f;
}
*/

void CPhysics::FloorAngle(const Math::Vector &pos, Math::Vector &angle)
{
    Character*  character;
    Math::Vector    pw, norm, choc;
    Math::Point nn;
    float       a, a1, a2;

    a = angle.y;

    choc = m_object->GetCirChoc();  // tient compte rotation choc
    a += choc.y;

    if ( m_type == TYPE_MASS )
    {
        if ( m_floorHeight == 0.0f &&  // au sol ?
             m_terrain->GetNormal(norm, pos) )
        {
            nn = RotatePoint(-a, Math::Point(norm.z, norm.x));
            angle.x =  sinf(nn.x);
            angle.z = -sinf(nn.y);
        }

        return;
    }

    character = m_object->GetCharacter();

    // TODO: Convert old wheelXXX to new format? ~krzys_h
    float wheelFront = character->wheelFront;
    float wheelBack = character->wheelBack;
    float wheelLeft = character->wheelLeft;
    float wheelRight = character->wheelRight;
    if (m_type != TYPE_NORMAL)
    {
        wheelFront = character->wheelFrontPos.x;
        wheelBack = character->wheelBackPos.z;
        wheelLeft = character->wheelBackPos.z;
        wheelRight = character->wheelBackPos.z;
    }

    pw.x = pos.x+wheelFront*cosf(a+Math::PI*0.0f);
    pw.y = pos.y;
    pw.z = pos.z-wheelFront*sinf(a+Math::PI*0.0f);
    a1 = atanf(m_terrain->GetHeightToFloor(pw)/wheelFront);

    pw.x = pos.x+wheelBack*cosf(a+Math::PI*1.0f);
    pw.y = pos.y;
    pw.z = pos.z-wheelBack*sinf(a+Math::PI*1.0f);
    a2 = atanf(m_terrain->GetHeightToFloor(pw)/wheelBack);

    angle.z = (a2-a1)/2.0f;

    pw.x = pos.x+wheelLeft*cosf(a+Math::PI*0.5f)*cosf(angle.z);
    pw.y = pos.y;
    pw.z = pos.z-wheelLeft*sinf(a+Math::PI*0.5f)*cosf(angle.z);
    a1 = atanf(m_terrain->GetHeightToFloor(pw)/wheelLeft);

    pw.x = pos.x+wheelRight*cosf(a+Math::PI*1.5f)*cosf(angle.z);
    pw.y = pos.y;
    pw.z = pos.z-wheelRight*sinf(a+Math::PI*1.5f)*cosf(angle.z);
    a2 = atanf(m_terrain->GetHeightToFloor(pw)/wheelRight);

    angle.x = (a2-a1)/2.0f;
}

// Adapts the physics of the object in relation to other objects.
// Returns 0 -> mobile object
// Returns 1 -> immobile object (because collision)
// Returns 2 -> destroyed object

int CPhysics::ObjectAdapt(Math::Vector &pos, Math::Vector &angle,
                          float aTime, float rTime)
{
    CObject        *maxObj=nullptr;
    Math::Matrix    matRotate;
    Math::Vector    iPos, oAngle, oSpeed, repulse;
    Math::Point        adjust, inter, maxAdjust, maxInter;
    SoundType        sound, maxSound=SOUND_NONE;
    float        distance, force, volume, hardness, maxHardness=0;
    float        iMass, oMass, factor, chocAngle, len, maxLen, maxAngle=0;
    float        priority, maxPriority=0;
    int           colType=0;
    ObjectType    iType, oType;

    if ( m_object->Implements(ObjectInterfaceType::Destroyable) && dynamic_cast<CDestroyableObject*>(m_object)->IsDying() )  return 0;  // is burning or exploding?
    if ( !m_object->GetCollisions() )  return 0;
// TODO (krzys_h):    if ( m_object->GetGhost() )  return 0;  // voiture fantome ?

    // iiPos = sphere center is the old position.
    // iPos  = sphere center has the new position.
    if (m_object->GetCrashSphereCount() < 1)
        return 0;

    auto firstCrashSphere = m_object->GetFirstCrashSphere();
    Math::Vector iiPos = firstCrashSphere.sphere.pos;
    float iRad = firstCrashSphere.sphere.radius;

    iPos = iiPos + (pos - m_object->GetPosition());
    iType = m_object->GetType();

    UpdateCorner(pos, angle);  // m_newCorner <- nouveaux coins

    maxLen = 0.0f;
    for (CObject* pObj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        if ( pObj == m_object )  continue;  // yourself?
        if (IsObjectBeingTransported(pObj))  continue;
        if ( pObj->Implements(ObjectInterfaceType::Destroyable) && dynamic_cast<CDestroyableObject*>(pObj)->IsDying() )  continue;  // is burning or exploding?
// TODO (krzys_h):    if ( pObj->GetGhost() )  continue;  // voiture fantome ?

        oType = pObj->GetType();
        if ( oType == OBJECT_TOTO            )  continue;
        if ( !m_object->CanCollideWith(pObj) )  continue;

        // TODO: Move to CanCollideWith ~krzys_h
        if ( iType == OBJECT_UFO   && oType != OBJECT_CAR  )  continue;
        if ( iType == OBJECT_PIECE && oType != OBJECT_CAR  )  continue;

        if (pObj->Implements(ObjectInterfaceType::Jostleable))
        {
            JostleObject(dynamic_cast<CJostleableObject*>(pObj), iPos, iRad);
        }

        if ( oType == OBJECT_WAYPOINT &&
             !pObj->GetLock()         &&
             m_object->GetTrainer()   )  // driving vehicle?
        {
            Math::Vector oPos = pObj->GetPosition();
            distance = Math::DistanceProjected(oPos, iPos);
            if ( distance < 4.0f )
            {
                m_sound->Play(SOUND_WAYPOINT, m_object->GetPosition());
                m_engine->GetPyroManager()->Create(Gfx::PT_WPCHECK, pObj);
            }
        }

        if ( oType == OBJECT_TARGET2 && !pObj->GetLock() )
        {
            Math::Vector oPos = pObj->GetPosition();
            distance = Math::Distance(oPos, iPos);
            if ( distance < 10.0f*1.5f )
            {
                m_sound->Play(SOUND_WAYPOINT, m_object->GetPosition());
                m_engine->GetPyroManager()->Create(Gfx::PT_WPCHECK, pObj);
            }
        }

        if ( (oType == OBJECT_WAYPOINT ||
              oType == OBJECT_TARGET   ) &&
             !pObj->GetLock()   && // TODO: I don't remember, does GetEnable really == !GetLock() ? ~krzys_h
             iType == OBJECT_CAR )
        {
            Math::Vector oPos = pObj->GetPosition();
            distance = Math::DistanceProjected(oPos, iPos);
            if ( distance < 40.0f )
            {
                assert(pObj->Implements(ObjectInterfaceType::Old));
                DoorCounter(dynamic_cast<COldObject*>(pObj), oType);
            }
        }

        if ( iType == OBJECT_CAR  ||
             iType == OBJECT_TRAX ) // TODO: check other types
        {
            if ( pObj->GetCrashLineCount() > 1 )
            {
                if ( CrashCornerRect(pObj, pos, angle, adjust, inter, chocAngle, priority, hardness, sound, aTime, rTime) != 0 )
                {
                    len = adjust.Length();
                    if ( len > maxLen )
                    {
                        maxLen = len;
                        maxObj = pObj;
                        maxAdjust = adjust;
                        maxInter = inter;
                        maxAngle = chocAngle;
                        maxPriority = priority;
                        maxHardness = hardness;
                        maxSound = sound;
                    }
                }
            }

            if ( pObj->GetCrashSphereCount() >= 1 )
            {
                if ( CrashCornerCircle(pObj, pos, angle, adjust, inter, chocAngle, priority, hardness, sound, aTime, rTime) != 0 )
                {
                    len = adjust.Length();
                    if ( len > maxLen )
                    {
                        maxLen = len;
                        maxObj = pObj;
                        maxAdjust = adjust;
                        maxInter = inter;
                        maxAngle = chocAngle;
                        maxPriority = priority;
                        maxHardness = hardness;
                        maxSound = sound;
                    }
                }
            }

            continue;
        }


        for (const auto& crashSphere : pObj->GetAllCrashSpheres())
        {
            Math::Vector oPos = crashSphere.sphere.pos;
            float oRad = crashSphere.sphere.radius;

            // Aliens ignore small objects
            // TODO: But why? :/
            if ( iType == OBJECT_MOTHER && oRad <= 1.2f )  continue;
            if ( iType == OBJECT_ANT    && oRad <= 1.2f )  continue;
            if ( iType == OBJECT_SPIDER && oRad <= 1.2f )  continue;
            if ( iType == OBJECT_BEE    && oRad <= 1.2f )  continue;
            if ( iType == OBJECT_WORM   && oRad <= 1.2f )  continue;

            distance = Math::Distance(oPos, iPos);
            if ( distance < iRad+oRad )  // collision?
            {
                distance = Math::Distance(oPos, iiPos);
                if ( distance >= iRad+oRad )  // view (*)
                {
                    m_bCollision = true;
                    m_bObstacle = true;

                    if (crashSphere.sound != SOUND_NONE)
                    {
                        force = fabs(m_linMotion.realSpeed.x);
                        if ( iType == OBJECT_TRAX )  force *= 10.0f;
                        hardness = crashSphere.hardness;
                        force *= crashSphere.hardness*2.0f;
                        if ( ExploOther(iType, pObj, oType, force, hardness) )  continue;
                        colType = ExploHimself(iType, oType, force, hardness, Math::Vector(NAN, NAN, NAN));
                        if ( colType == 2 )  return 2;  // destroyed?
                        if ( colType == 0 )  continue;  // ignores?
                    }

                    force = m_linMotion.realSpeed.Length();
                    force *= crashSphere.hardness;
                    volume = fabs(force*0.05f);
                    if ( volume > 1.0f )  volume = 1.0f;
                    if ( crashSphere.sound != SOUND_NONE &&
                         iType != OBJECT_HUMAN &&
                         oType != OBJECT_BOT3  &&
                         oType != OBJECT_EVIL1 &&
                         oType != OBJECT_EVIL3 &&
                         aTime-m_lastSoundCollision >= 0.1f )
                    {
                        m_lastSoundCollision = aTime;
                        m_sound->Play(sound, m_object->GetPosition(), volume);
                    }
                    if ( iType == OBJECT_HUMAN && volume > 0.5f )
                    {
                        m_sound->Play(SOUND_AIE, m_object->GetPosition(), volume);
                    }

                    // TODO (krzys_h): review this code
                    if (m_type == TYPE_NORMAL)
                    {
                        if (m_repeatCollision > 0)
                        {
                            force *= 0.5f * m_repeatCollision;
                            if (force > 20.0f) force = 20.0f;
                        }
                        m_repeatCollision += 2;
                        if (m_repeatCollision > 10)
                        {
                            m_repeatCollision = 10;
                        }

                        m_linMotion.currentSpeed = Normalize(iPos - oPos) * force;
                        Math::LoadRotationXZYMatrix(matRotate, -angle);
                        m_linMotion.currentSpeed = Transform(matRotate, m_linMotion.currentSpeed);
                        if (!m_object->Implements(ObjectInterfaceType::Flying))
                        {
                            m_linMotion.currentSpeed.y = 0.0f;
                        }


                        CPhysics* ph = nullptr;
                        if (pObj->Implements(ObjectInterfaceType::Movable))
                            ph = dynamic_cast<CMovableObject*>(pObj)->GetPhysics();
                        if (ph != nullptr)
                        {
                            oAngle = pObj->GetRotation();
                            oSpeed = Normalize(oPos - iPos) * force;
                            Math::LoadRotationXZYMatrix(matRotate, -oAngle);
                            oSpeed = Transform(matRotate, oSpeed);
                            if (!pObj->Implements(ObjectInterfaceType::Flying))
                            {
                                oSpeed.y = 0.0f;
                            }
                            ph->SetLinMotion(MO_CURSPEED, oSpeed);
                        }
                    } else {

                        if ( colType == 3 )  // passe au travers ?
                        {
                            iMass = m_object->GetCharacter()->mass;
                            oMass = pObj->GetCharacter()->mass;
                            if ( iMass == 0.0f || oMass == 0.0f )
                            {
                                factor = 0.6f;
                            }
                            else
                            {
                                factor = Math::Norm(0.25f+(iMass/oMass)*0.25f);
                            }
                            m_linMotion.currentSpeed *= factor;  // on est freiné
                            continue;  // passe outre
                        }

                        m_centriSpeed = 0.0f;
                        m_overTurn = 0.0f;
                        m_overBrake = 0.0f;

                        if ( m_chocSpin == 0.0f )
                        {
                            m_chocSpin = ChocSpin(oPos, force);
                        }

//?                    repulse = Normalize(iPos-oPos)*force;
                        repulse = ChocRepulse(m_object, pObj, iPos, oPos);
                        repulse *= 0.5f;  // amortissement
//?                    repulse.x *= 1.0f+(m_repeatCollision-2)*0.1f;
                        ChocObject(m_object, -repulse);
                        ChocObject(pObj, repulse);
                    }
                    return 1;
                }
            }
        }
    }

    if ( maxLen > 0.0f )
    {
        colType = CrashCornerDo(maxObj, pos, maxAdjust, maxInter, maxAngle,
                                maxPriority, maxHardness, maxSound,
                                aTime, rTime);
        if ( colType == 2 )  return 2;  // détruit ?
        if ( colType != 0 )  return 0;
    }

    if (m_type == TYPE_NORMAL)
    {
        if (m_repeatCollision > 0)
        {
            m_repeatCollision--;
        }
    }
    return 0;
}

// (*)  Collision has the initial position (iiPos) and the new position (iPos),
//  the obstacle is not known. We can therefore pass through.
//  This is necessary when barriers found "in" a vehicle, not to block it definitely!

// Calcule la force de répulsion ŕ appliquer aprčs une collision.

Math::Vector CPhysics::ChocRepulse(CObject *pObj1, CObject *pObj2,
                                   Math::Vector p1, Math::Vector p2)
{
    CPhysics*    physics;
    Math::Matrix    matRotate;
    Math::Vector    v1, v2, a1, a2, c;

    c = Math::Vector(0.0f, 0.0f, 0.0f);

    if ( !pObj1->Implements(ObjectInterfaceType::Movable) )  return c;
    physics = dynamic_cast<CMovableObject*>(pObj1)->GetPhysics();
    v1 = physics->GetLinMotion(MO_CURSPEED);
    Math::LoadRotationXZYMatrix(matRotate, pObj1->GetRotation());
    v1 = Transform(matRotate, v1);

    if ( !pObj2->Implements(ObjectInterfaceType::Movable) )
    {
        a1 = Math::Projection(p1, p2, p1+v1);
//?        c = (a1-p1)*2.0f;
        c = (a1-p1)*3.0f;
    }
    else
    {
        physics = dynamic_cast<CMovableObject*>(pObj2)->GetPhysics();
        v2 = physics->GetLinMotion(MO_CURSPEED);
        Math::LoadRotationXZYMatrix(matRotate, pObj2->GetRotation());
        v2 = Transform(matRotate, v2);

        a1 = Math::Projection(p1, p2, p1+v1);
        a2 = Math::Projection(p1, p2, p2+v2);
        c = (a1-p1) - (a2-p2);
    }

    if ( c.x == 0.0f && c.y == 0.0f && c.z == 0.0f )
    {
        c = Math::Normalize(p1-p2);
    }

    return c;
}

// Effectue un choc sur un objet suite ŕ une collision.

void CPhysics::ChocObject(CObject *pObj, Math::Vector repulse)
{
    CPhysics*    physics;
    PhysicsType    type;
    Math::Matrix    matRotate;
    Math::Vector    angle, speed, current;

    if ( !pObj->Implements(ObjectInterfaceType::Movable) )  return;
    physics = dynamic_cast<CMovableObject*>(pObj)->GetPhysics();
    type = physics->GetType();

    angle = pObj->GetRotation();
    Math::LoadRotationXZYMatrix(matRotate, -angle);
    speed = Transform(matRotate, repulse);

    if ( type == TYPE_RACE ||
         type == TYPE_TANK )
    {
        speed.y = 0.0f;  // reste toujours au sol
        speed.z *= 0.4f;  // glisse peu latéralement
    }
    if ( type == TYPE_MASS )
    {
//?        speed.y = 50.0f;  //????
        physics->SetCirMotionY(MO_CURSPEED, 3.0f);
    }

    current = physics->GetLinMotion(MO_CURSPEED);
    physics->SetLinMotion(MO_CURSPEED, current+speed);
}

// Calcule l'angle de rotation suite ŕ un choc.

float CPhysics::ChocSpin(Math::Vector obstacle, float force)
{
    Math::Matrix*    mat;
    Math::Vector    pos, p;
    Math::Point        center, p1, p2;
    float        a;

    if ( m_type != TYPE_RACE )  return 0.0f;
    if ( force < 40.0f )  return 0.0f;  // petit/moyen choc ?

    pos = m_object->GetPosition();
    center.x = pos.x;
    center.y = pos.z;
    mat = m_object->GetWorldMatrix(0);
    p = Transform(*mat, Math::Vector(2.0f, 0.0f, 0.0f));
    p1.x = p.x;
    p1.y = p.z;
    p2.x = obstacle.x;
    p2.y = obstacle.z;
    a = RotateAngle(center, p1, p2);

    if ( force > 80.0f )  force = 80.0f;
    force *= 0.3f;

    a /= Math::PI*0.25f;  // 0..8 (octan)
    if ( a >= 2.0f && a <= 4.0f )  a -= 2.0f;  // 0..2
    if ( a >= 4.0f && a <= 6.0f )  a += 2.0f;  // 6..8

    if ( a <= 1.0f )  return -force*a;         // choc côté droite, devant
    if ( a <= 2.0f )  return -force*(2.0f-a);  // choc côté droite, derričre
    if ( a <= 7.0f )  return  force*(a-6.0f);  // choc côté gauche, derričre
    return  force*(8.0f-a);  // choc côté gauche, devant
}


// Shakes an object.

bool CPhysics::JostleObject(CJostleableObject* pObj, Math::Vector iPos, float iRad)
{
    Math::Sphere jostlingSphere = pObj->GetJostlingSphere();

    float distance = Math::Distance(jostlingSphere.pos, iPos);
    if ( distance >= iRad+jostlingSphere.radius)  return false;

    float d = (iRad+jostlingSphere.radius)/2.0f;
    float f = (distance-d)/d;  // 0 = off, 1 = near
    if ( f < 0.0f )  f = 0.0f;
    if ( f > 1.0f )  f = 1.0f;

    Math::Vector speed = m_linMotion.realSpeed;
    speed.y = 0.0f;
    float force = speed.Length()*f*0.05f;
    if ( force > 1.0f )  force = 1.0f;

    if ( m_soundTimeJostle >= 0.20f )
    {
        m_soundTimeJostle = 0.0f;
        m_sound->Play(SOUND_JOSTLE, iPos, force);
    }

    return pObj->JostleObject(force);
}

// Shakes forcing an object.

bool CPhysics::JostleObject(CObject* pObj, float force)
{
    if (! pObj->Implements(ObjectInterfaceType::Jostleable))
        return false;

    CJostleableObject* jostleableObject = dynamic_cast<CJostleableObject*>(pObj);

    if ( m_soundTimeJostle >= 0.20f )
    {
        m_soundTimeJostle = 0.0f;
        m_sound->Play(SOUND_JOSTLE, pObj->GetPosition(), force);
    }

    return jostleableObject->JostleObject(force);
}

// Effects of the explosion on the object buffers.
// Returns true if we ignore this obstacle.

bool CPhysics::ExploOther(ObjectType iType,
                          CObject *pObj, ObjectType oType,
                          float force, float hardness)
{
    JostleObject(pObj, 1.0f);  // shakes the object


    // Activation of objects that are hit

    CAuto*        automatic;
    if ( force > 0.0f &&
         (oType == OBJECT_DOOR2 ||
          oType == OBJECT_DOOR3 ) &&
         hardness == 0.44f )
    {
        automatic = pObj->GetAuto();
        if ( automatic != 0 )
        {
//TODO            automatic->Start(2);  // casse le mécanisme
        }
    }
    if ( force > 0.0f &&
         oType == OBJECT_ALIEN8 &&
         hardness == 0.44f )
    {
        automatic = pObj->GetAuto();
        if ( automatic != 0 )
        {
//TODO            automatic->Start(2);  // casse le mécanisme
        }
    }
    if ( force > 0.0f &&
         oType == OBJECT_COMPUTER &&
         hardness >= 0.42f &&
         hardness <= 0.44f )
    {
        automatic = pObj->GetAuto();
        if ( automatic != 0 )
        {
            if ( hardness == 0.44f )
            {
//TODO                automatic->Start(2);  // casse le mécanisme
            }
            if ( hardness == 0.43f )
            {
//TODO                automatic->Start(3);  // casse le mécanisme
            }
            if ( hardness == 0.42f )
            {
//TODO                automatic->Start(4);  // casse le mécanisme
            }
        }
    }
    if ( force > 0.0f &&
         oType == OBJECT_INCA7 &&
         hardness == 0.44f )
    {
        automatic = pObj->GetAuto();
        if ( automatic != 0 )
        {
//TODO            automatic->Start(2);  // casse le mécanisme
        }
    }

    if (pObj->Implements(ObjectInterfaceType::Damageable))
    {
        // TODO: CFragileObject::GetDestructionForce (I can't do this now because you can't inherit both in COldObject ~krzys_h)
        DamageType damageType = DamageType::Collision;
        float destructionForce = pObj->Implements(ObjectInterfaceType::Fragile) ? 50.0f : -1.0f; // Titanium, PowerCell, NuclearCell, default
        if (pObj->GetType() == OBJECT_STONE   ) { destructionForce = 25.0f; } // TitaniumOre
        if (pObj->GetType() == OBJECT_URANIUM ) { destructionForce = 25.0f; } // UraniumOre
        if (pObj->GetType() == OBJECT_MOBILEtg) { destructionForce = 10.0f; damageType = DamageType::Explosive; } // TargetBot (something running into it)
        if (iType           == OBJECT_MOBILEtg) { destructionForce = 10.0f; damageType = DamageType::Explosive; } // TargetBot (it running into something)
        if (pObj->GetType() == OBJECT_TNT     ) { destructionForce = 10.0f; damageType = DamageType::Explosive; } // TNT
        if (pObj->GetType() == OBJECT_BOMB    ) { destructionForce =  0.0f; damageType = DamageType::Explosive; } // Mine

        if ( force > destructionForce && destructionForce >= 0.0f )
        {
            // TODO: implement "killer"?
            dynamic_cast<CDamageableObject*>(pObj)->DamageObject(damageType);
        }
    }

    if ( force > 25.0f )
    {
        // TODO: Some function in CShieldedObject. GetCollisionResistance()?
        if (oType == OBJECT_DERRICK  ||
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
            oType == OBJECT_HUSTON    )  // building?
        {
            assert(pObj->Implements(ObjectInterfaceType::Damageable));
            // TODO: implement "killer"?
            dynamic_cast<CDamageableObject*>(pObj)->DamageObject(DamageType::Collision, force/400.0f);
        }

        if (oType == OBJECT_MOBILEwa ||
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
            oType == OBJECT_MOBILEit  )  // vehicle?
        {
            assert(pObj->Implements(ObjectInterfaceType::Damageable));
            // TODO: implement "killer"?
            dynamic_cast<CDamageableObject*>(pObj)->DamageObject(DamageType::Collision, force/200.0f);
        }
    }

    /* TODO (krzys_h)
    if ( force > 50.0f &&
         (oType == OBJECT_FRET  ||
          oType == OBJECT_METAL ) )
    {
        m_engine->GetPyroManager()->Create(Gfx::PT_EXPLOT, pObj);  // destruction totale
    }

    if ( force > 0.0f &&
         oType == OBJECT_BARREL )
    {
        m_engine->GetPyroManager()->Create(Gfx::PT_FRAGT, pObj);  // destruction totale
    }
    if ( force > 0.0f &&
         (oType == OBJECT_BARRELa ||
          oType == OBJECT_ATOMIC  ) )
    {
        m_engine->GetPyroManager()->Create(Gfx::PT_FRAGA, pObj);  // destruction totale
    }

    if ( force > 25.0f &&
         (oType >= OBJECT_CARCASS1 && oType <= OBJECT_CARCASS10) )
    {
        pyro = new CPyro(m_iMan);
//-        pyro->Create(PT_EJECT, pObj);  // destruction totale
        pyro->Create(PT_ACROBATIC, pObj, f);  // voltige
    }

    if ( force > 25.0f &&
         (oType == OBJECT_STONE   ||
          oType == OBJECT_URANIUM ) )
    {
        pyro = new CPyro(m_iMan);
//-        pyro->Create(PT_FRAGT, pObj);  // destruction totale
        pyro->Create(PT_ACROBATIC, pObj, f);  // voltige
    }

    if ( force > 25.0f &&
         ((oType >= OBJECT_BARRIER4 && oType <= OBJECT_BARRIER5) ||
          oType == OBJECT_BARRIER19 ) )
    {
        pyro = new CPyro(m_iMan);
//-        pyro->Create(PT_EJECT, pObj);  // destruction totale
        pyro->Create(PT_ACROBATIC, pObj, f);  // voltige
    }

    if ( force > 25.0f &&
         ((oType >= OBJECT_ROADSIGN1  && oType <= OBJECT_ROADSIGN21) ||
          (oType >= OBJECT_ROADSIGN26 && oType <= OBJECT_ROADSIGN30) ) )
    {
        pyro = new CPyro(m_iMan);
//-        pyro->Create(PT_EJECT, pObj);  // destruction totale
        pyro->Create(PT_ACROBATIC, pObj, f);  // voltige
    }

    if ( force > 25.0f &&
//?         (oType >= OBJECT_BOX1 && oType <= OBJECT_BOX10) )
         (oType >= OBJECT_BOX1 && oType <= OBJECT_BOX3) )
    {
        pyro = new CPyro(m_iMan);
//?        pyro->Create(PT_EJECT, pObj);  // destruction totale
        pyro->Create(PT_FRAGT, pObj);  // destruction totale
    }

    if ( force > 25.0f &&
         (oType >= OBJECT_BOX5 && oType <= OBJECT_BOX6) )
    {
        m_engine->GetPyroManager()->Create(Gfx::PT_ACROBATIC, pObj, f);  // voltige
    }

    if ( force > 25.0f &&
         (oType >= OBJECT_TOYS1 && oType <= OBJECT_TOYS5) )
    {
        m_engine->GetPyroManager()->Create(Gfx::PT_ACROBATIC, pObj, f);  // voltige
    }

    if ( oType == OBJECT_PIECE ||
         oType == OBJECT_CONE  )
    {
        pyro = new CPyro(m_iMan);
//-        pyro->Create(PT_EJECT, pObj);  // destruction totale
        pyro->Create(PT_ACROBATIC, pObj, f);  // voltige
    }

    if ( force > 0.0f &&
         (oType == OBJECT_EVIL1 ||
          oType == OBJECT_EVIL3 ) )
    {
        m_engine->GetPyroManager()->Create(Gfx::PT_EXPLOO, pObj);  // destruction totale
    }

    if ( force > 10.0f &&
         (oType == OBJECT_BOT1    ||
          oType == OBJECT_BOT2    ||
          oType == OBJECT_BOT3    ||
          oType == OBJECT_BOT4    ||
          oType == OBJECT_BOT5    ||
          oType == OBJECT_CARROT  ||
          oType == OBJECT_STARTER ||
          oType == OBJECT_WALKER  ||
          oType == OBJECT_CRAZY   ||
          oType == OBJECT_GUIDE   ) )
    {
        m_engine->GetPyroManager()->Create(Gfx::PT_ACROBATIC, pObj, f);  // voltige
    }

    if ( force > 25.0f &&
         oType == OBJECT_CAR )  // véhicule ?
    {
        pObj->ExploObject(EXPLO_BOUM, force/200.0f);
    }

    if ( force > 25.0f &&
         oType == OBJECT_UFO )  // soucoupe ?
    {
        pObj->ExploObject(EXPLO_BOUM, force/200.0f);
    }

    if ( force > 25.0f &&
         iType != OBJECT_TRAX &&
         oType == OBJECT_TRAX )  // trax ?
    {
        if ( hardness > 0.5f )  // bouton arričre touché ?
        {
            m_engine->GetPyroManager()->Create(Gfx::PT_EXPLOT, pObj);  // destruction totale
        }
    }

    if ( force > 10.0f &&
         (oType == OBJECT_MOBILEtg ||
          oType == OBJECT_TNT      ) )
    {
        m_engine->GetPyroManager()->Create(Gfx::PT_FRAGT, pObj);  // destruction totale
    }

    if ( force > 0.0f &&
         oType == OBJECT_MINE )
    {
        m_engine->GetPyroManager()->Create(Gfx::PT_FRAGT, pObj);  // destruction totale
    }
    */

    return false;
}

// Effects of the explosion on the object itself.
// Returns 0 -> mobile object
// Returns 1 -> immobile object
// Returns 2 -> object destroyed
// Returns 3 -> passe au travers

int CPhysics::ExploHimself(ObjectType iType, ObjectType oType,
                           float force, float hardness, Math::Vector impact)
{
    if (!m_object->Implements(ObjectInterfaceType::Damageable)) return 1;

    /* TODO (krzys_h)

    PyroType    type;
    ObjectType    fType;
    CObject*    fret;
    CMotion*    motion;
    CPyro*        pyro;
    int            colli;

    m_main->InfoCollision(oType);

    FFBCrash(force/50.0f, 0.3f, 2.0f);

    fret = m_object->GetFret();
    if ( fret != 0 )
    {
        fType = fret->GetType();
        if ( fType == OBJECT_BOT1   ||
             fType == OBJECT_BOT2   ||
             fType == OBJECT_BOT3   ||
             fType == OBJECT_BOT4   ||
             fType == OBJECT_BOT5   ||
             fType == OBJECT_WALKER ||
             fType == OBJECT_CRAZY  )
        {
            motion = fret->GetMotion();
            if ( motion != 0 )
            {
                motion->SetAction(MB_FEAR, 10.0f);  // le robot a peur !
            }
        }
    }

    if ( force > 0.0f &&
         (oType == OBJECT_BOT3  ||
          oType == OBJECT_EVIL1 ||
          oType == OBJECT_EVIL3 ) )
    {
        return 3;  // passe au travers sans aucun dégat
    }

    if ( force > 10.0f &&
         (oType == OBJECT_TNT      ||
          oType == OBJECT_MOBILEtg ) )
    {
        if ( iType == OBJECT_HUMAN )  type = PT_DEADG;
        else                          type = PT_EXPLOT;
        pyro = new CPyro(m_iMan);
        pyro->Create(type, m_object);  // destruction totale
        return 2;
    }

    if ( force > 0.0f &&
         oType == OBJECT_MINE )
    {
        if ( iType == OBJECT_HUMAN )
        {
            type = PT_DEADG;
        }
        else
        {
            type = PT_EXPLOT;
        }
        pyro = new CPyro(m_iMan);
        pyro->Create(type, m_object);  // destruction totale
        return 2;
    }

    if ( force > 0.0f &&
         iType != OBJECT_TRAX &&
         oType == OBJECT_TRAX &&
         hardness < 0.5f )  // pas le bouton arričre ?
    {
        force /= 10.0f;
        if ( m_object->ExploObject(EXPLO_BOUM, force) )  return 2;
        return 1;
    }

    if ( oType == OBJECT_CAR &&
         (iType == OBJECT_BOT1    ||
          iType == OBJECT_BOT2    ||
          iType == OBJECT_BOT3    ||
          iType == OBJECT_BOT4    ||
          iType == OBJECT_BOT5    ||
          iType == OBJECT_CARROT  ||
          iType == OBJECT_STARTER ||
          iType == OBJECT_WALKER  ||
          iType == OBJECT_CRAZY   ||
          iType == OBJECT_GUIDE   ) )
    {
//?        pyro = new CPyro(m_iMan);
//?        pyro->Create(PT_ACROBATIC, m_object, f);  // voltige
        return 3;  // passe au travers sans aucun dégat
    }

    colli = 1;  // immobile

    if ( force > 25.0f &&
         (iType == OBJECT_HUMAN ||
          iType == OBJECT_CAR   ||
          iType == OBJECT_UFO   ) )  // véhicule ?
    {
        if ( oType == OBJECT_TOWER    ||
             oType == OBJECT_NUCLEAR  ||
             oType == OBJECT_PARA     ||
             oType == OBJECT_COMPUTER ||
             oType == OBJECT_DOCK     ||
             oType == OBJECT_REMOTE   ||
             oType == OBJECT_STAND    )  // bâtiment ?
        {
            force /= 200.0f;
        }
        else
        if ( (oType == OBJECT_DOOR2 ||
              oType == OBJECT_DOOR3 ) &&
             hardness == 0.44f )  // casse le mécanisme ?
        {
            return 1;
        }
        if ( oType == OBJECT_ALIEN8 &&
             hardness == 0.44f )  // casse le mécanisme ?
        {
            return 1;
        }
        else
        if ( oType == OBJECT_FRET     ||
             oType == OBJECT_STONE    ||
             oType == OBJECT_METAL    ||
             (oType >= OBJECT_BARRIER4 && oType <= OBJECT_BARRIER5) ||
             oType == OBJECT_BARRIER19 ||
             (oType >= OBJECT_ROADSIGN1  && oType <= OBJECT_ROADSIGN21) ||
             (oType >= OBJECT_ROADSIGN26 && oType <= OBJECT_ROADSIGN30) ||
//?             (oType >= OBJECT_CARCASS1 && oType <= OBJECT_CARCASS10) ||
//?             (oType >= OBJECT_BOX1 && oType <= OBJECT_BOX10) )
             (oType >= OBJECT_BOX1 && oType <= OBJECT_BOX3) )
        {
            force /= 500.0f;
            colli = 3;  // passe au travers
        }
        else
        if ( oType == OBJECT_BOX4 )
        {
            return 1;
        }
        else
        if ( oType == OBJECT_PIECE ||
             oType == OBJECT_CONE  )
        {
            return 3;  // passe au travers sans aucun dégat
        }
        else
        if ( oType == OBJECT_URANIUM ||
             oType == OBJECT_BARREL  ||
             oType == OBJECT_BARRELa ||
             oType == OBJECT_ATOMIC  )
        {
            force /= 500.0f;
            colli = 3;  // passe au travers
        }
        else
        {
            force /= 200.0f;
        }

        if ( m_object->ExploObject(EXPLO_BOUM, force, impact) )  return 2;
    }

    */

    // TODO: CExplosiveObject? derrives from CFragileObject
    float destructionForce = -1.0f; // minimal force required to destroy an object using this explosive, default: not explosive
    if ( oType == OBJECT_TNT      ) destructionForce = 10.0f; // TNT
    if ( oType == OBJECT_MOBILEtg ) destructionForce = 10.0f; // TargetBot (something running into it)
    if ( iType == OBJECT_MOBILEtg ) destructionForce = 10.0f; // TargetBot (it running into something)
    if ( oType == OBJECT_BOMB     ) destructionForce =  0.0f; // Mine

    if ( force > destructionForce && destructionForce >= 0.0f )
    {
        // TODO: implement "killer"?
        dynamic_cast<CDamageableObject*>(m_object)->DamageObject(DamageType::Explosive, std::numeric_limits<float>::infinity(), nullptr, impact);
        return 2;
    }

    if ( force > 25.0f )
    {
        if ( iType == OBJECT_HUMAN    ||
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
             iType == OBJECT_MOBILEdr ||  // vehicle?
             iType == OBJECT_APOLLO2  ||
             iType == OBJECT_CAR      ) // TODO (krzys_h): just added this manually to test, find the full list in sources
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
            else if ( oType == OBJECT_MOTHER ||
                      oType == OBJECT_ANT    ||
                      oType == OBJECT_SPIDER ||
                      oType == OBJECT_BEE    ||
                      oType == OBJECT_WORM   )  // insect?
            {
                force /= 400.0f;
            }
            else
            if ( oType == OBJECT_STONE ||
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

            // TODO: implement "killer"?
            if ( dynamic_cast<CDamageableObject*>(m_object)->DamageObject(DamageType::Collision, force, nullptr, impact) )  return 2;
        }
    }

    return 1;
}



// Gestion du passage multiple sur un objet, dans les circuits
// ŕ plusieurs tours.

void CPhysics::DoorCounter(COldObject* pObj, ObjectType oType)
{
    float        freq;
    int            rank, counter, lap;
    bool        bSound;

    if ( pObj == m_lastDoorCounter )  return;
    m_lastDoorCounter = pObj;

    rank = pObj->GetRankCounter();
    lap = m_main->GetLapProgress();
    if ( lap == 0 )  lap = 1000;
    if ( rank != -1 )
    {
        if ( m_doorRank%lap != rank )
        {
            GetLogger()->Info("Passed through INCORRECT door %d, should be %d (%d/%d on lap %d)\n", (m_doorRank/lap)*lap + rank, m_doorRank, m_doorRank%lap+1, lap, m_doorRank/lap+1);
            m_sound->Play(SOUND_ERROR, m_object->GetPosition(), 1.0f, 1.0f);
            return;
        }
        GetLogger()->Info("Passed through CORRECT door %d (%d/%d on lap %d)\n", m_doorRank, m_doorRank%lap+1, lap, m_doorRank/lap+1);
        m_doorRank ++;
    }

    counter = pObj->GetPassCounter();
    if ( counter == 0 )
    {
//TODO (krzys_h):        m_main->InfoCollision(oType);

        if ( oType == OBJECT_TARGET )  freq = 1.0f;
        else                           freq = 1.5f;
        m_sound->Play(SOUND_WAYPOINT, m_object->GetPosition(), 1.0f, freq);

        pObj->SetLock(true);
        m_engine->GetPyroManager()->Create(Gfx::PT_WPCHECK, pObj);
    }
    else
    {
        bSound = m_main->IncProgress();
        counter --;
        pObj->SetPassCounter(counter);

//TODO (krzys_h):        m_main->InfoCollision(oType);

        if ( !bSound )
        {
            if ( oType == OBJECT_TARGET )  freq = 1.0f;
            else                           freq = 1.5f;
            if ( counter == 0 )            freq *= 1.5f;
            m_sound->Play(SOUND_WAYPOINT, m_object->GetPosition(), 1.0f, freq);
        }

        if ( counter == 0 )
        {
            pObj->SetLock(true);
            m_engine->GetPyroManager()->Create(Gfx::PT_WPCHECK, pObj);
        }
        else
        {
            m_engine->GetPyroManager()->Create(Gfx::PT_WPVIBRA, pObj);
        }
    }
}


// Gestion de la collision d'un véhicule rectangulaire avec un obstacle
// rectangulaire.

int CPhysics::CrashCornerRect(CObject *pObj, const Math::Vector &pos, const Math::Vector &angle,
                              Math::Point &adjust, Math::Point &inter,
                              float &chocAngle, float &priority,
                              float &hardness, SoundType &sound,
                              float aTime, float rTime)
{
    Math::Vector    oPos;
    Math::Point        bbMin, bbMax;
    Math::Point        t1,t2, p, pp, oneP, oneT1,oneT2;
    Math::Point        maxAdjust, minAdjust, maxImpact, minImpact;
    Math::Point        maxP1,minP1, maxP2,minP2;
    SoundType        maxSound=SOUND_NONE, minSound=SOUND_NONE, oneSound=SOUND_NONE;
    float        maxHardness=0, minHardness=0, oneHardness=0;
    float        maxPriority=0, minPriority=0, onePriority=0;
    float        maxLen, minLen;
    float        len, a=0, h;
    int            max, nbInter, onePart;

    max = pObj->GetCrashLineCount();
    if ( max < 2 )  return 0;

    h = pObj->GetCrashLineHeight();
    if ( !std::isnan(h) )
    {
        oPos = pObj->GetPosition();
        if ( pos.y > oPos.y+h )  return 0;
    }

    pObj->GetCrashLineBBox(bbMin, bbMax);
    if ( m_newCorner.min.x > bbMax.x )  return 0;
    if ( m_newCorner.max.x < bbMin.x )  return 0;
    if ( m_newCorner.min.y > bbMax.y )  return 0;
    if ( m_newCorner.max.y < bbMin.y )  return 0;

    minLen = 100000.0f;
    nbInter = 0;  // nb d'intersections
    onePart = 0;  // unknow segment

    std::vector<CrashLine> crashLines = pObj->GetAllCrashLines();
    for ( std::vector<CrashLine>::iterator P1 = crashLines.begin(), P2 = P1 + 1; P2 != crashLines.end(); P1++, P2++ )
    {
        if ( P2->bNew )  continue;

        // En cas d'intersection du segment de l'obstacle avec un
        // segment du véhicule, il faut vérifier si tous les segments
        // d'obstacle interrompent le męme segment de véhicule.
        // Si oui, on est dans le cas particulier "one" oů une pointe
        // d'obstacle entre dans un flan du véhicule.

        for ( int i=0 ; i<4 ; i++ )
        {
//?            if ( m_linMotion.realSpeed.x > 0.0f && i == 2 )  continue;
//?            if ( m_linMotion.realSpeed.x < 0.0f && i == 0 )  continue;

            t1.x = m_newCorner.p[i].x;
            t1.y = m_newCorner.p[i].z;
            t2.x = m_newCorner.p[(i+1)%4].x;
            t2.y = m_newCorner.p[(i+1)%4].z;
            if ( Math::IntersectSegment(P1->pos,P2->pos, t1,t2, pp) )
            {
                nbInter ++;
                if ( onePart == 0 )  // unknow segment ?
                {
                    onePart = i+1;  // 1..4 = quel segment du véhicule
                }
                if ( onePart >= 1 && onePart <= 4 && onePart != i+1 )
                {
                    onePart = -1;  // plusieurs segments
                }
                if ( onePart >= 1 && onePart <= 4 )
                {
                    oneP = pp;
                    if ( Math::IsInside(t1,t2, P1->pos) )  oneP = P1->pos;
                    if ( Math::IsInside(t1,t2, P2->pos) )  oneP = P2->pos;
                }
                oneT1 = t1;
                oneT2 = t2;
                oneHardness = P2->hardness;
                oneSound = P2->sound;
                onePriority = 1.0f;
                if ( m_linMotion.realSpeed.x > 0.0f && i == 2 )  onePriority = 0.0f;
                if ( m_linMotion.realSpeed.x < 0.0f && i == 0 )  onePriority = 0.0f;
            }
        }

        // Cherche pour les 4 coins du véhicule celui qui nécessite le
        // plus grand déplacement (maxAdjust) pour éviter l'obstacle par
        // un déplacement perpendiculaire au segment de l'obstacle.

        maxLen = -1.0f;
        for ( int i=0 ; i<4 ; i++ )
        {
//?            if ( m_linMotion.realSpeed.x > 0.0f && i/2 != 0 )  continue;
//?            if ( m_linMotion.realSpeed.x < 0.0f && i/2 == 0 )  continue;

            p.x = m_newCorner.p[i].x;
            p.y = m_newCorner.p[i].z;

            if ( Math::IsInside(P1->pos,P2->pos, p) )
            {
                pp = Math::Projection(P1->pos,P2->pos, p);
                len = Math::Distance(p, pp);
                if ( len > maxLen )
                {
                    adjust.x = pp.x-p.x;
                    adjust.y = pp.y-p.y;
                    if ( CrashValidity(pObj, adjust) )
                    {
                        maxLen = len;
                        maxAdjust = adjust;
                        maxImpact = pp;
                        maxP1 = P1->pos;
                        maxP2 = P2->pos;
                        maxHardness = P2->hardness;
                        maxSound = P2->sound;
                        maxPriority = 1.0f;
                        if ( m_linMotion.realSpeed.x > 0.0f && i/2 != 0 )  maxPriority = 0.0f;
                        if ( m_linMotion.realSpeed.x < 0.0f && i/2 == 0 )  maxPriority = 0.0f;
                    }
                }
            }
        }

        // Le plus petit déplacement pour chaque segment d'obstacle
        // sera le meilleur.

        if ( maxLen > -1.0f )
        {
            if ( maxLen < minLen )
            {
                minLen = maxLen;
                minAdjust = maxAdjust;
                minImpact = maxImpact;
                minP1 = maxP1;
                minP2 = maxP2;
                minHardness = maxHardness;
                minSound = maxSound;
                minPriority = maxPriority;
            }
        }
    }

    if ( nbInter > 0 && minLen < 100000.0f )
    {
        if ( nbInter == 2 && onePart >= 1 && onePart <= 4 )  // cas particulier "one" ?
        {
            pp = Math::Projection(oneT1,oneT2, oneP);
            minAdjust.x = oneP.x-pp.x;
            minAdjust.y = oneP.y-pp.y;
            minImpact = oneP;
            minP1 = oneT1;
            minP2 = oneT2;
            minHardness = oneHardness;
            minSound = oneSound;
            minPriority = onePriority;

            if ( onePart%2 == 0 )  return 0;  //?

//?            if ( onePart%2 == 0 )  a = PI/8.0f;  // flan ?
//?            else                   a = PI/2.0f;  // av/ar ?
            if ( onePart == 1 )  a = Math::PI*0.5f;  // avant ?
            if ( onePart == 2 )  a = Math::PI*0.4f;  // flan gauche ?
            if ( onePart == 3 )  a = Math::PI*0.5f;  // arričre ?
            if ( onePart == 4 )  a = Math::PI*0.6f;  // flan droite ?
        }
        else
        {
            t1.x = m_newCorner.p[1].x;
            t1.y = m_newCorner.p[1].z;  // fl
            t2.x = m_newCorner.p[2].x;
            t2.y = m_newCorner.p[2].z;  // rl
            if ( Math::Intersect(t1,t2, minP1,minP2, p) )
            {
                a = RotateAngle(p, minP2, t1);
                if ( a > Math::PI )  a -= Math::PI;
//?                if ( a > PI/2.0f )  a = PI-a;
            }
            else
            {
                a = Math::PI/2.0f;
            }
        }

        len = minAdjust.Length();
//?        char s[100];
//?        sprintf(s, "CrashCornerRect: i=%d op=%d adj=%.2f;%.2f a=%.2f len=%.2f\n", nbInter, onePart, minAdjust.x,minAdjust.y, a*180.0f/PI, len);
//?        GetLogger()->Trace(s);
        if ( len < 0.1f )  return 0;

        adjust    = minAdjust;
        inter     = minImpact;
        chocAngle = a;
        hardness  = minHardness;
        sound     = minSound;
        priority  = minPriority;
        return 1;
    }

    return 0;
}

// Valide si un ajustement produit une position du véhicule qui
// n'est pas dans l'obstacle.

bool CPhysics::CrashValidity(CObject *pObj, Math::Point adjust)
{
    Math::Point    t1,t2;
    bool    bOut;

    if ( adjust.x > 0.0f )  adjust.x += 0.001f;
    if ( adjust.x < 0.0f )  adjust.x -= 0.001f;
    if ( adjust.y > 0.0f )  adjust.y += 0.001f;
    if ( adjust.y < 0.0f )  adjust.y -= 0.001f;

    std::vector<CrashLine> crashLines = pObj->GetAllCrashLines();

    for ( int i=0 ; i<4 ; i++ )
    {
        t1.x = adjust.x+m_newCorner.p[i].x;
        t1.y = adjust.y+m_newCorner.p[i].z;

        bOut = false;
        for ( std::vector<CrashLine>::iterator P1 = crashLines.begin(), P2 = P1 + 1; P2 != crashLines.end(); P1++, P2++ )
        {
            if ( P2->bNew )  continue;

            if ( !Math::IsInside(P1->pos,P2->pos, t1) )  bOut = true;
        }
        if ( !bOut )
        {
//?            GetLogger()->Trace("CrashValidity exclure A\n");
            return false;
        }
    }

    for ( std::vector<CrashLine>::iterator P1 = crashLines.begin()+1; P1 != crashLines.end(); P1++ )
    {
        if ( P1->bNew )  continue;

        bOut = false;
        for ( int i=0 ; i<4 ; i++ )
        {
            t1.x = adjust.x+m_newCorner.p[i].x;
            t1.y = adjust.y+m_newCorner.p[i].z;
            t2.x = adjust.x+m_newCorner.p[(i+1)%4].x;
            t2.y = adjust.y+m_newCorner.p[(i+1)%4].z;

            if ( !Math::IsInside(t1,t2, P1->pos) )  bOut = true;
        }
        if ( !bOut )
        {
//?            GetLogger()->Trace("CrashValidity exclure B\n");
            return false;
        }
    }

    return true;
}

// Gestion de la collision d'un véhicule rectangulaire avec un obstacle
// circulaire.

int CPhysics::CrashCornerCircle(CObject *pObj, const Math::Vector &pos, const Math::Vector &angle,
                                Math::Point &adjust, Math::Point &inter,
                                float &chocAngle, float &priority,
                                float &hardness, SoundType &sound,
                                float aTime, float rTime)
{
    Character*    character;
    Math::Point        cc, t1,t2, c, pp;
    float        iRad, distance, min, a=0, max;
    int            i;

    character = m_object->GetCharacter();

    cc.x = (m_newCorner.p[0].x+m_newCorner.p[1].x+m_newCorner.p[2].x+m_newCorner.p[3].x)/4.0f;
    cc.y = (m_newCorner.p[0].z+m_newCorner.p[1].z+m_newCorner.p[2].z+m_newCorner.p[3].z)/4.0f;
    iRad = Math::Point(cc.x-m_newCorner.p[0].x, cc.y-m_newCorner.p[0].z).Length();

    for (const auto& crashSphere : pObj->GetAllCrashSpheres())
    {
        if ( pos.y+5.0f < crashSphere.sphere.pos.y-crashSphere.sphere.radius )  continue;
        if ( pos.y+1.0f > crashSphere.sphere.pos.y+crashSphere.sphere.radius )  continue;

        c.x = crashSphere.sphere.pos.x;
        c.y = crashSphere.sphere.pos.z;

        distance = Math::Distance(c, cc);
        if ( distance >= iRad+crashSphere.sphere.radius )  continue;  // pas de collision ?

        min = 10000.0f;
        for ( i=0 ; i<4 ; i++ )
        {
            t1.x = m_newCorner.p[i].x;
            t1.y = m_newCorner.p[i].z;
            t2.x = m_newCorner.p[(i+1)%4].x;
            t2.y = m_newCorner.p[(i+1)%4].z;

            pp = Math::Projection(t1,t2, c);
            if ( !Math::IsInSegment(t1,t2, pp) )  continue;

            distance = Math::Distance(pp, c);
            if ( Math::IsInside(t1,t2, c) )
            {
                distance += crashSphere.sphere.radius;
                if ( i%2 == 0 )  max = character->crashFront-character->crashBack;
                else             max = character->crashWidth*2.0f;
                if ( distance >= crashSphere.sphere.radius+max )  continue;
                if ( distance >= min )  continue;
                min = distance;
                inter.x = pp.x+(c.x-pp.x)*(crashSphere.sphere.radius*2.0f/min);
                inter.y = pp.y+(c.y-pp.y)*(crashSphere.sphere.radius*2.0f/min);
                adjust.x = inter.x-pp.x;
                adjust.y = inter.y-pp.y;
                if ( i%2 == 0 )  a = Math::PI/2.0f;  // collision frontale
                else             a = 0.0f;     // collision rasante
            }
            else
            {
                if ( distance >= crashSphere.sphere.radius )  continue;
                if ( distance >= min )  continue;
                min = distance;
                inter.x = c.x+(pp.x-c.x)*(crashSphere.sphere.radius/min);
                inter.y = c.y+(pp.y-c.y)*(crashSphere.sphere.radius/min);
                adjust.x = inter.x-pp.x;
                adjust.y = inter.y-pp.y;
                if ( i%2 == 0 )  a = Math::PI/2.0f;  // collision frontale
                else             a = 0.0f;     // collision rasante
            }
        }

        for ( i=0 ; i<4 ; i++ )
        {
            t1.x = m_newCorner.p[i].x;
            t1.y = m_newCorner.p[i].z;

            distance = Math::Distance(t1, c);
            if ( distance >= crashSphere.sphere.radius )  continue;
            if ( distance >= min )  continue;
            min = distance;
            inter.x = c.x+(t1.x-c.x)*(crashSphere.sphere.radius/min);
            inter.y = c.y+(t1.y-c.y)*(crashSphere.sphere.radius/min);
            adjust.x = inter.x-t1.x;
            adjust.y = inter.y-t1.y;

            t2.x = m_newCorner.p[i^1].x;
            t2.y = m_newCorner.p[i^1].z;
            a = RotateAngle(t1, t2, c);
            if ( a > Math::PI )  a -= Math::PI;
        }

        if ( min < 10000.0f )
        {
//?            char s[100];
//?            sprintf(s, "CrashCornerCircle: a=%.2f\n", a*180.0f/PI);
//?            OutputDebugString(s);

            chocAngle = a;
            priority = 1.0f;
            hardness = crashSphere.hardness;
            sound = crashSphere.sound;
            return 1;
        }
    }

    return 0;
}

// Effectue la collision.

int CPhysics::CrashCornerDo(CObject *pObj, Math::Vector &pos,
                            Math::Point adjust, Math::Point inter,
                            float angle, float priority,
                            float hardness, SoundType sound,
                            float aTime, float rTime)
{
    Math::Vector    impact, ppos, speed, iSpeed;
    Math::Point        dim;
    ObjectType    iType, oType;
    float        force, volume, mass, iMass, oMass, factor, amplitude, iAngle;
    int            colType;

    iType = m_object->GetType();
    oType = pObj->GetType();

    impact.x = inter.x;
    impact.z = inter.y;
    impact.y = pos.y+1.0f;

    // Ajuste la position.
    pos.x += adjust.x;
    pos.z += adjust.y;

    force = fabs(m_linMotion.realSpeed.x);
    iSpeed = m_linMotion.currentSpeed;
    iAngle = angle;

#if 0
    // Vz' = -Vx*sin(a)*cos(a)*2
    // Vx' = -Vz'*tan(PI/2-2a)
    if ( Abs(angle-PI/2.0f) < 0.01f )
    {
        m_linMotion.currentSpeed.x = -m_linMotion.currentSpeed.x;
        m_linMotion.currentSpeed.z = 0.0f;
    }
    else
    {
        m_linMotion.currentSpeed.z = -m_linMotion.currentSpeed.x*sinf(angle)*cosf(angle)*2.0f;
        m_linMotion.currentSpeed.x = -m_linMotion.currentSpeed.z*tanf(PI/2.0f-2.0f*angle);
    }

    // Plus le choc est rasant (angle -> 0 ou 180), moins la vitesse
    // est amortie (par exemple pour glisser le long d'une barričre).
    m_linMotion.currentSpeed.x *= (1.0f-sinf(angle)*0.8f);
    m_linMotion.currentSpeed.z *= (1.0f-sinf(angle)*0.8f)*0.7f;

    m_linMotion.realSpeed = m_linMotion.currentSpeed;
#endif
#if 0
    //?    m_linMotion.currentSpeed.x = 0.0f;
//?    m_linMotion.realSpeed = m_linMotion.currentSpeed;
    m_linMotion.currentSpeed.x *= 0.8f;
    m_linMotion.realSpeed = m_linMotion.currentSpeed;
#endif
#if 0
    if ( angle < PI/4.0f )  // choc rasant ?
    {
        m_linMotion.currentSpeed.x *= cosf(angle);
    }
    else    // choc frontal ?
    {
        m_linMotion.currentSpeed.x = -m_linMotion.currentSpeed.x*0.5f;
    }

    m_linMotion.realSpeed = m_linMotion.currentSpeed;
#endif
#if 1
    if ( priority == 0.0f )
    {
        force = 0.0f;
    }
    else
    {
        if ( angle < Math::PI*0.25f || angle > Math::PI*0.75f )  // choc rasant ?
        {
            if ( angle > Math::PI/2.0f )  angle = Math::PI-angle;
            m_linMotion.currentSpeed.x *= cosf(angle);
            m_linMotion.currentSpeed.z = 0.0f;
            force *= powf(angle/(Math::PI*0.25f), 2.0f);
        }
        else    // choc frontal ?
        {
            // Vz' = -Vx*sin(a)*cos(a)*2
            // Vx' = -Vz'*tan(PI/2-2a)
            if ( fabs(angle-Math::PI/2.0f) < 0.01f )
            {
                m_linMotion.currentSpeed.x = -m_linMotion.currentSpeed.x;
                m_linMotion.currentSpeed.z = 0.0f;
            }
            else
            {
                m_linMotion.currentSpeed.z = -m_linMotion.currentSpeed.x*sinf(angle)*cosf(angle)*2.0f;
                m_linMotion.currentSpeed.x = -m_linMotion.currentSpeed.z*tanf(Math::PI/2.0f-2.0f*angle);
            }
//?            m_linMotion.currentSpeed.x *= 0.5f;
//?            m_linMotion.currentSpeed.z *= 0.3f;
            m_linMotion.currentSpeed.x *= 0.2f;
            m_linMotion.currentSpeed.z *= 0.1f;
            m_centriSpeed *= 0.1f;
        }
    }

    m_linMotion.realSpeed = m_linMotion.currentSpeed;
    m_graviGlu = 1.0f;
#endif

    if ( iType == OBJECT_TRAX )  force *= 10.0f;
    force *= hardness*2.0f;
    ExploOther(iType, pObj, oType, force, hardness);
    colType = ExploHimself(iType, oType, force, hardness, impact);
    if ( colType == 2 )  return 2;  // détruit ?

    volume = Math::Norm(fabs(force/40.0f));
    if ( sound != SOUND_CLICK &&
         iType != OBJECT_HUMAN &&
         oType != OBJECT_BOT3  &&
         oType != OBJECT_EVIL1 &&
         oType != OBJECT_EVIL3 &&
         aTime-m_lastSoundCrash >= 0.1f )
    {
        m_lastSoundCrash = aTime;
        m_sound->Play(sound, impact, volume);

        if ( sound != SOUND_BOUMm &&
             oType != OBJECT_CONE )
        {
            m_sound->Play(SOUND_BOUMm, impact, volume*0.8f);
        }
    }

    if ( colType == 3 )  // passe au travers ?
    {
        iMass = m_object->GetCharacter()->mass;
        oMass = pObj->GetCharacter()->mass;
        if ( iMass == 0.0f || oMass == 0.0f )
        {
            factor = 0.6f;
        }
        else
        {
            factor = Math::Norm(0.25f+(iMass/oMass)*0.25f);
        }
        m_linMotion.currentSpeed = iSpeed*factor;  // on est freiné
        m_linMotion.realSpeed = m_linMotion.currentSpeed;
        return 1;  // passe outre
    }

    if ( angle < Math::PI*0.25f || angle > Math::PI*0.75f )  // choc rasant ?
    {
//?        m_glideAmplitude = Norm(Abs(force/5.0f))*0.6f;
        m_glideAmplitude = Math::Norm(fabs(force/3.0f));
        if ( m_glideAmplitude < 0.8f )  m_glideAmplitude = 0.8f;
        m_glideImpact = impact;

        ppos = impact;
        ppos.y += Math::Rand()*3.0f;
        speed.x = (Math::Rand()-0.5f)*5.0f;
        speed.z = (Math::Rand()-0.5f)*5.0f;
        speed.y = 5.0f+Math::Rand()*5.0f;
        mass    = 20.0f+Math::Rand()*20.0f;
        dim.x = 0.4f+Math::Rand()*0.4f;
        dim.y = dim.x;
        m_particle->CreateParticle(ppos, speed, dim, Gfx::PARTIGLINT, 0.5f, mass);

        amplitude = fabs(iSpeed.x/m_linMotion.advanceSpeed.x)*5.0f;
        if ( iAngle < Math::PI*0.25f )
        {
            amplitude *= iAngle/(Math::PI*0.25f);  // tire ŕ gauche contre l'obstacle
        }
        else
        {
            amplitude *= -(Math::PI-iAngle)/(Math::PI*0.25f);  // tire ŕ droite contre l'obstacle
        }
        FFBForce(amplitude*0.5f);
    }
    else
    {
        amplitude = fabs(iSpeed.x/m_linMotion.advanceSpeed.x)*5.0f;
        FFBCrash(amplitude, 0.3f, 1.0f);
    }

    return 1;
}

// Calcule les coins courants du véhicule.

void CPhysics::UpdateCorner()
{
    Character*    character;
    Math::Matrix*    mat;
    ObjectType    type;

    type = m_object->GetType();
    if ( type != OBJECT_CAR  &&
         type != OBJECT_TRAX )  return;

    character = m_object->GetCharacter();
    mat = m_object->GetWorldMatrix(0);
    m_curCorner.p[0] = Transform(*mat, Math::Vector(character->crashFront, 0.0f, -character->crashWidth));  // fr
    m_curCorner.p[1] = Transform(*mat, Math::Vector(character->crashFront, 0.0f,  character->crashWidth));  // fl
    m_curCorner.p[2] = Transform(*mat, Math::Vector(character->crashBack,  0.0f,  character->crashWidth));  // rl
    m_curCorner.p[3] = Transform(*mat, Math::Vector(character->crashBack,  0.0f, -character->crashWidth));  // rr

    m_curCorner.min.x = Math::Min(m_curCorner.p[0].x, m_curCorner.p[1].x, m_curCorner.p[2].x, m_curCorner.p[3].x);
    m_curCorner.min.y = Math::Min(m_curCorner.p[0].z, m_curCorner.p[1].z, m_curCorner.p[2].z, m_curCorner.p[3].z);

    m_curCorner.max.x = Math::Max(m_curCorner.p[0].x, m_curCorner.p[1].x, m_curCorner.p[2].x, m_curCorner.p[3].x);
    m_curCorner.max.y = Math::Max(m_curCorner.p[0].z, m_curCorner.p[1].z, m_curCorner.p[2].z, m_curCorner.p[3].z);
}

// Calcule les nouveaux coins du véhicule.

void CPhysics::UpdateCorner(const Math::Vector &pos, const Math::Vector &angle)
{
    Character*    character;
    Math::Matrix    matRotate;
    ObjectType    type;

    type = m_object->GetType();
    if ( type != OBJECT_CAR  &&
         type != OBJECT_TRAX )  return;

    character = m_object->GetCharacter();
    Math::LoadRotationZXYMatrix(matRotate, angle);  // calcule la nouvelle matrice de rotation
    m_newCorner.p[0] = Transform(matRotate, Math::Vector(character->crashFront, 0.0f, -character->crashWidth))+pos;  // fr
    m_newCorner.p[1] = Transform(matRotate, Math::Vector(character->crashFront, 0.0f,  character->crashWidth))+pos;  // fl
    m_newCorner.p[2] = Transform(matRotate, Math::Vector(character->crashBack,  0.0f,  character->crashWidth))+pos;  // rl
    m_newCorner.p[3] = Transform(matRotate, Math::Vector(character->crashBack,  0.0f, -character->crashWidth))+pos;  // rr

    m_newCorner.min.x = Math::Min(m_newCorner.p[0].x, m_newCorner.p[1].x, m_newCorner.p[2].x, m_newCorner.p[3].x);
    m_newCorner.min.y = Math::Min(m_newCorner.p[0].z, m_newCorner.p[1].z, m_newCorner.p[2].z, m_newCorner.p[3].z);

    m_newCorner.max.x = Math::Max(m_newCorner.p[0].x, m_newCorner.p[1].x, m_newCorner.p[2].x, m_newCorner.p[3].x);
    m_newCorner.max.y = Math::Max(m_newCorner.p[0].z, m_newCorner.p[1].z, m_newCorner.p[2].z, m_newCorner.p[3].z);
}

// Met ŕ jour le grincement lorsque la voiture glisse le long d'un obstacle.

void CPhysics::UpdateGlideSound(float rTime)
{
    if ( m_glideAmplitude > 0.0f )
    {
        m_glideVolume = m_glideAmplitude;

        if ( m_soundChannelGlide == -1 )
        {
            m_soundChannelGlide = m_sound->Play(SOUND_WHEELb, m_glideImpact, m_glideVolume, 1.2f, true);
            m_sound->AddEnvelope(m_soundChannelGlide, m_glideVolume, 1.2f, 1.0f, SOPER_LOOP);
        }
        if ( m_soundChannelGlide != -1 )
        {
            m_sound->Position(m_soundChannelGlide, m_glideImpact);
            m_sound->Amplitude(m_soundChannelGlide, m_glideVolume);
            m_sound->Frequency(m_soundChannelGlide, 1.0f);
        }
        m_glideTime = 0.2f;
    }
    else
    {
        m_glideTime -= rTime;
        if ( m_glideTime < 0.0f )  m_glideTime = 0.0f;

        if ( m_glideTime == 0.0f )
        {
            if ( m_soundChannelGlide != -1 )
            {
                m_sound->FlushEnvelope(m_soundChannelGlide);
                m_sound->AddEnvelope(m_soundChannelGlide, 0.0f, 1.2f, 0.1f, SOPER_STOP);
                m_soundChannelGlide = -1;
            }
        }
        else
        {
            if ( m_soundChannelGlide != -1 )
            {
                m_glideVolume -= rTime/0.4f;
                if ( m_glideVolume < 0.0f )  m_glideVolume = 0.0f;
                m_sound->Position(m_soundChannelGlide, m_glideImpact);
                m_sound->Amplitude(m_soundChannelGlide, m_glideVolume);
                m_sound->Frequency(m_soundChannelGlide, 1.0f);
            }
        }
    }
}


// Makes the particles evolve.

void CPhysics::FrameParticle(float aTime, float rTime)
{
    m_restBreakParticle -= rTime;
    if ( aTime-m_lastPowerParticle < m_engine->ParticleAdapt(0.05f) )  return;
    m_lastPowerParticle = aTime;

    float energy = GetObjectEnergyLevel(m_object);

    if ( energy != m_lastEnergy )  // change the energy level?
    {
        if ( energy > m_lastEnergy )  // recharge?
        {
            PowerParticle(1.0f, false);
        }

        if ( energy == 0.0f || m_lastEnergy == 0.0f )
        {
            m_restBreakParticle = 2.5f;  // particles for 2.5s
        }

        m_lastEnergy = energy;
    }

    if ( m_restBreakParticle > 0.0f )
    {
        PowerParticle(m_restBreakParticle/2.5f, (energy == 0));
    }
}

// Generates some particles after a recharge.

void CPhysics::PowerParticle(float factor, bool bBreak)
{
    Math::Matrix*   mat;
    Math::Vector    pos, ppos, eye, speed;
    Math::Point     dim;
    bool        bCarryPower;

    bCarryPower = false;
    if (m_object->Implements(ObjectInterfaceType::Carrier))
    {
        CObject* cargo = dynamic_cast<CCarrierObject*>(m_object)->GetCargo();
        if ( cargo != nullptr && cargo->Implements(ObjectInterfaceType::PowerContainer) &&
            dynamic_cast<CPowerContainerObject*>(cargo)->IsRechargeable() &&
            m_object->GetPartRotationZ(1) == ARM_STOCK_ANGLE1 )
        {
            bCarryPower = true;  // carries a battery
        }
    }

    mat = m_object->GetWorldMatrix(0);

    pos = m_object->GetPowerPosition();
    pos.x -= 0.3f;
    pos.y += 1.0f;  // battery center position
    pos = Transform(*mat, pos);

    speed.x = (Math::Rand()-0.5f)*12.0f;
    speed.y = (Math::Rand()-0.5f)*12.0f;
    speed.z = (Math::Rand()-0.5f)*12.0f;

    ppos.x = pos.x;
    ppos.y = pos.y+(Math::Rand()-0.5f)*2.0f;
    ppos.z = pos.z;

    dim.x = 1.0f*factor;
    dim.y = 1.0f*factor;

    m_particle->CreateParticle(ppos, speed, dim, Gfx::PARTIBLITZ, 0.5f, 0.0f, 0.0f);

    if ( bCarryPower )  // carry a battery?
    {
        pos = Math::Vector(3.0f, 5.6f, 0.0f);  // position of battery holder // TODO: Move to CTransportableObject
        pos = Transform(*mat, pos);

        speed.x = (Math::Rand()-0.5f)*12.0f;
        speed.y = (Math::Rand()-0.5f)*12.0f;
        speed.z = (Math::Rand()-0.5f)*12.0f;

        ppos.x = pos.x;
        ppos.y = pos.y;
        ppos.z = pos.z+(Math::Rand()-0.5f)*2.0f;

        dim.x = 1.0f*factor;
        dim.y = 1.0f*factor;

        m_particle->CreateParticle(ppos, speed, dim, Gfx::PARTIBLITZ, 0.5f, 0.0f, 0.0f);
    }
}

// Generates some particles after a fall.
// crash: 0=super soft, 1=big crash

void CPhysics::CrashParticle(float crash, bool bCrash)
{
    Math::Vector    pos, ppos, speed;
    Math::Point     dim;
    float           len;
    int             i, max;

    if ( crash < 0.2f || m_bWater )  return;

    pos = m_object->GetPosition();
    m_camera->StartEffect(Gfx::CAM_EFFECT_CRASH, pos, crash);

    if ( bCrash )
    {
        m_camera->StartEffect(Gfx::CAM_EFFECT_CRASH, pos, crash);
        FFBCrash(crash, 0.4f, 5.0f);
//?char s[100];
//?sprintf(s, "crash=%.2f\n", crash);
//?OutputDebugString(s);
    }

//? max = (int)(crash*50.0f);
    max = static_cast<int>(crash*10.0f*m_engine->GetParticleDensity());

    for ( i=0 ; i<max ; i++ )
    {
        ppos.x = pos.x + (Math::Rand()-0.5f)*15.0f*crash;
        ppos.z = pos.z + (Math::Rand()-0.5f)*15.0f*crash;
        ppos.y = pos.y + Math::Rand()*4.0f;
        len = 1.0f-(Math::Distance(ppos, pos)/(15.0f+5.0f));
        if ( len <= 0.0f )  continue;
        speed.x = (ppos.x-pos.x)*0.1f;
        speed.z = (ppos.z-pos.z)*0.1f;
        speed.y = -2.0f;
        dim.x = 2.0f+crash*5.0f*len;
        dim.y = dim.x;
        m_particle->CreateParticle(ppos, speed, dim, Gfx::PARTICRASH, 2.0f);
    }
}

// Génčre qq particules suite ŕ un dérappage.

void CPhysics::SlideParticle(float aTime, float rTime,
                             float fDusty, float fSound)
{
    SoundType        sound;
    WheelType    wType;

    wType = m_motion->GetWheelType();

    if ( wType != WT_BURN &&
         m_lastCrashParticle+m_engine->ParticleAdapt(0.05f) <= aTime )
    {
        m_lastCrashParticle = aTime;
        CrashParticle(fDusty*1.5f, false);
    }

    if ( fSound <= 0.4f )
    {
        if ( m_soundChannelSlide != -1 )
        {
            m_sound->FlushEnvelope(m_soundChannelSlide);
            m_sound->AddEnvelope(m_soundChannelSlide, 0.0f, 1.0f, 0.1f, SOPER_STOP);
            m_soundChannelSlide = -1;
        }
        return;
    }

    if ( m_soundChannelSlide == -1 && !m_bWater )
    {
        sound = SOUND_WHEEL;
        if ( wType == WT_BURN )  sound = SOUND_WHEELb;
        m_soundChannelSlide = m_sound->Play(sound, m_object->GetPosition(), 0.0f, 1.0f, true);
        m_sound->AddEnvelope(m_soundChannelSlide, 0.7f, 1.0f, 0.1f, SOPER_CONTINUE);
        m_sound->AddEnvelope(m_soundChannelSlide, 0.7f, 1.0f, 1.0f, SOPER_LOOP);
    }
    if ( m_soundChannelSlide != -1 )
    {
        if ( wType == WT_BURN )  fSound *= 2.0f;
        m_sound->Position(m_soundChannelSlide, m_object->GetPosition());
        m_sound->Amplitude(m_soundChannelSlide, Math::Norm(fSound*1.0f));
        m_sound->Frequency(m_soundChannelSlide, 1.0f+fSound*0.3f);
    }
}

// Génčre qq particules ŕ l'emplacement des roues, soit lors
// d'une force accélération, soit lors d'un freinage.

void CPhysics::WheelParticle(float aTime, float rTime, float force, bool bBrake)
{
    SoundType            sound;
    Character*        character;
    Math::Matrix*        mat;
    Math::Vector        pos, speed, goal1, goal2, wheel1, wheel2;
    Math::Point            dim;
    Gfx::ParticleType    parti;
    WheelType        wType;
    float            delay, factor, volume, mass, dist1, dist2, step;
    bool            bSilent;

    character = m_object->GetCharacter();
    mat = m_object->GetWorldMatrix(0);
    wType = m_motion->GetWheelType();

    if ( force <= 0.01f )
    {
        if ( m_soundChannelBoost != -1 )
        {
            m_sound->FlushEnvelope(m_soundChannelBoost);
            m_sound->AddEnvelope(m_soundChannelBoost, 0.0f, 1.0f, 0.3f, SOPER_STOP);
            m_soundChannelBoost = -1;
        }
        if ( m_soundChannelBrake != -1 )
        {
            m_sound->FlushEnvelope(m_soundChannelBrake);
            m_sound->AddEnvelope(m_soundChannelBrake, 0.0f, 1.0f, 0.3f, SOPER_STOP);
            m_soundChannelBrake = -1;
        }
        if ( wType != WT_BURN )
        {
            m_bWheelParticleBrake = false;
            return;
        }
        bSilent = true;  // seulement pour dessiner les traces au sol
    }
    else
    {
        bSilent = false;
    }

    if ( bBrake )
    {
        if ( m_soundChannelBoost != -1 )
        {
            m_sound->FlushEnvelope(m_soundChannelBoost);
            m_sound->AddEnvelope(m_soundChannelBoost, 0.0f, 1.0f, 0.3f, SOPER_STOP);
            m_soundChannelBoost = -1;
        }
    }
    else
    {
        if ( m_soundChannelBrake != -1 )
        {
            m_sound->FlushEnvelope(m_soundChannelBrake);
            m_sound->AddEnvelope(m_soundChannelBrake, 0.0f, 1.0f, 0.3f, SOPER_STOP);
            m_soundChannelBrake = -1;
        }
    }

    if ( m_terrainHard >= 0.5f )  // sol dur ?
    {
        sound = SOUND_WHEEL;
        if ( wType == WT_BURN )  sound = SOUND_WHEELb;
        volume = 0.5f+Math::Norm(force)*0.5f;
        if ( m_bHandbrake )  volume = Math::Norm(volume*2.0f);
    }
    else    // sol mou ?
    {
        sound = SOUND_WHEELg;
        if ( bBrake )
        {
            force = fabs(m_linMotion.realSpeed.x)/m_linMotion.advanceSpeed.x;
            volume = Math::Norm(force*4.0f);
            if ( m_bHandbrake )  volume = Math::Norm(volume*2.0f);
        }
        else
        {
            volume = 0.0f;
        }
    }

    if ( m_soundChannelBoost == -1 && !bBrake && !m_bWater && !bSilent )
    {
        m_soundChannelBoost = m_sound->Play(sound, m_object->GetPosition(), 0.0f, 1.5f, true);
        m_sound->AddEnvelope(m_soundChannelBoost, volume, 1.5f, 0.3f, SOPER_CONTINUE);
        m_sound->AddEnvelope(m_soundChannelBoost, volume, 1.5f, 4.0f, SOPER_LOOP);
    }
    if ( m_soundChannelBoost != -1 )
    {
        if ( wType == WT_BURN )
        {
            volume *= 2.0f;
        }
        else
        {
            volume *= fabs(m_cirMotion.realSpeed.y)/m_cirMotion.advanceSpeed.y*2.0f;
        }
        m_sound->Position(m_soundChannelBoost, m_object->GetPosition());
        m_sound->Amplitude(m_soundChannelBoost, volume);
        m_sound->Frequency(m_soundChannelBoost, 1.0f);
    }

    if ( m_soundChannelBrake == -1 && bBrake && !m_bWater && !bSilent )
    {
        m_soundChannelBrake = m_sound->Play(sound, m_object->GetPosition(), 0.0f, 1.0f, true);
        if ( sound == SOUND_WHEELg )
        {
            m_sound->AddEnvelope(m_soundChannelBrake, volume, 1.0f, 0.3f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannelBrake, volume, 1.0f, 4.0f, SOPER_LOOP);
        }
        else
        {
            m_sound->AddEnvelope(m_soundChannelBrake, volume, 1.0f, 0.3f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannelBrake, volume, 0.5f, 4.0f, SOPER_LOOP);
        }
    }
    if ( m_soundChannelBrake != -1 )
    {
        m_sound->Position(m_soundChannelBrake, m_object->GetPosition());
        m_sound->Amplitude(m_soundChannelBrake, volume);
        m_sound->Frequency(m_soundChannelBrake, 1.0f);
    }

    // Dessine les traces de frein sur le sol.
    if ( bBrake || force > 0.5f || wType == WT_BURN )
    {
        if ( wType == WT_BURN )
        {
            parti = Gfx::PARTITRACE7;
            step = 2.0f;
        }
        else
        {
            if ( m_terrainHard < 0.5f )  // sol mou ?
            {
                parti = Gfx::PARTITRACE6;
            }
            else    // sol dur (route) ?
            {
                if ( bBrake )  parti = Gfx::PARTITRACE1;
                else           parti = Gfx::PARTITRACE2;
            }
            step = 8.0f;
        }

        goal1.x = character->wheelBackPos.x+step/2.0f;
        goal1.y = 0.0f;
        goal1.z = character->wheelBackPos.z;
        goal1 = Transform(*mat, goal1);

        goal2.x = character->wheelBackPos.x+step/2.0f;
        goal2.y = 0.0f;
        goal2.z = character->wheelBackPos.z+character->wheelBackWidth;
        goal2 = Transform(*mat, goal2);

        if ( !m_bWheelParticleBrake )
        {
            m_wheelParticlePos[0] = goal1;
            m_wheelParticlePos[1] = goal2;
        }

        while ( true )
        {
            dist1 = Math::Distance(m_wheelParticlePos[0], goal1);
            if ( dist1 < step )  break;
            dist2 = Math::Distance(m_wheelParticlePos[1], goal2);
            wheel1 = Math::SegmentPoint(m_wheelParticlePos[0], goal1, step);
            wheel2 = Math::SegmentPoint(m_wheelParticlePos[1], goal2, step*dist2/dist1);
            if ( m_linMotion.realSpeed.x >= 0.0f )
            {
                m_particle->CreateWheelTrace(m_wheelParticlePos[0], m_wheelParticlePos[1], wheel1, wheel2, parti);
            }
            else
            {
                m_particle->CreateWheelTrace(m_wheelParticlePos[1], m_wheelParticlePos[0], wheel2, wheel1, parti);
            }
            m_wheelParticlePos[0] = wheel1;
            m_wheelParticlePos[1] = wheel2;
        }

        goal1.x = character->wheelBackPos.x+step/2.0f;
        goal1.y = 0.0f;
        goal1.z = -character->wheelBackPos.z;
        goal1 = Transform(*mat, goal1);

        goal2.x = character->wheelBackPos.x+step/2.0f;
        goal2.y = 0.0f;
        goal2.z = -(character->wheelBackPos.z+character->wheelBackWidth);
        goal2 = Transform(*mat, goal2);

        if ( !m_bWheelParticleBrake )
        {
            m_wheelParticlePos[2] = goal1;
            m_wheelParticlePos[3] = goal2;
        }

        while ( true )
        {
            dist1 = Math::Distance(m_wheelParticlePos[2], goal1);
            if ( dist1 < step )  break;
            dist2 = Math::Distance(m_wheelParticlePos[3], goal2);
            wheel1 = Math::SegmentPoint(m_wheelParticlePos[2], goal1, step);
            wheel2 = Math::SegmentPoint(m_wheelParticlePos[3], goal2, step*dist2/dist1);
            if ( m_linMotion.realSpeed.x >= 0.0f )
            {
                m_particle->CreateWheelTrace(m_wheelParticlePos[3], m_wheelParticlePos[2], wheel2, wheel1, parti);
            }
            else
            {
                m_particle->CreateWheelTrace(m_wheelParticlePos[2], m_wheelParticlePos[3], wheel1, wheel2, parti);
            }
            m_wheelParticlePos[2] = wheel1;
            m_wheelParticlePos[3] = wheel2;
        }

        m_bWheelParticleBrake = true;
    }
    else
    {
        m_bWheelParticleBrake = false;
    }

    if ( m_lastWheelParticle+m_engine->ParticleAdapt(0.05f) > aTime )  return;
    m_lastWheelParticle = aTime;

    if ( wType == WT_BURN )
    {
        if ( force == 0.0f )  return;
        force += 1.0f;
    }

    if ( m_terrainHard == 0.0f )  // neige ?
    {
        parti  = Gfx::PARTICRASH;
        delay  = 2.0f;
        factor = 1.0f;
        speed  = Math::Vector(0.0f, 0.0f, 0.0f);
        mass   = 0.0f;
    }
    else if ( m_terrainHard < 0.6f )  // sol mou ?
    {
        if ( bBrake )  parti = Gfx::PARTIDUST1;
        else           parti = Gfx::PARTIDUST2;
        delay  = 2.0f;
        factor = 2.0f;
        speed  = Math::Vector(0.0f, 0.0f, 0.0f);
        mass   = 0.0f;
    }
    else    // sol dur ?
    {
        parti  = Gfx::PARTIWHEEL;
        delay  = 5.0f;
        factor = 1.0f;
        speed  = Math::Vector(0.0f, 0.0f, 0.0f);
        mass   = 0.0f;
    }
    if ( wType == WT_BURN )  // roules sur les jantes ?
    {
        parti   = Gfx::PARTIGLINT;  // étincelles
        delay   = 0.5f;
        factor  = 0.2f;
        speed.x = (Math::Rand()-0.5f)*5.0f;
        speed.z = (Math::Rand()-0.5f)*5.0f;
        speed.y = 5.0f+Math::Rand()*5.0f;
        mass    = 20.0f+Math::Rand()*20.0f;
    }
    if ( m_bWater )
    {
        parti   = Gfx::PARTIBUBBLE;
        delay   = 2.0f;
        factor  = 0.1f;
        speed.x = (Math::Rand()-0.5f)*5.0f;
        speed.z = (Math::Rand()-0.5f)*5.0f;
        speed.y = 5.0f+Math::Rand()*5.0f;
        mass    = 0.0f;
    }

    if ( bBrake )
    {
        pos.x = character->wheelFrontPos.x+(Math::Rand()-0.5f)*2.0f*character->wheelFrontDim;
        pos.y = 0.0f;
        pos.z = character->wheelFrontPos.z+Math::Rand()*character->wheelFrontDim;
        pos = Transform(*mat, pos);
        dim.x = (0.5f+force*0.8f+Math::Rand()*1.0f)*factor;
        dim.y = dim.x;
        m_particle->CreateParticle(pos, speed, dim, parti, delay, mass);

        pos.x = character->wheelFrontPos.x+(Math::Rand()-0.5f)*2.0f*character->wheelFrontDim;
        pos.y = 0.0f;
        pos.z = -character->wheelFrontPos.z-Math::Rand()*character->wheelFrontDim;
        pos = Transform(*mat, pos);
        dim.x = (0.5f+force*0.8f+Math::Rand()*1.0f)*factor;
        dim.y = dim.x;
        m_particle->CreateParticle(pos, speed, dim, parti, delay, mass);
    }

    pos.x = character->wheelBackPos.x+(Math::Rand()-0.5f)*2.0f*character->wheelBackDim;
    pos.y = 0.0f;
    pos.z = character->wheelBackPos.z+Math::Rand()*character->wheelBackDim;
    pos = Transform(*mat, pos);
    dim.x = (0.5f+force*0.8f+Math::Rand()*1.0f)*factor;
    dim.y = dim.x;
    m_particle->CreateParticle(pos, speed, dim, parti, delay, mass);

    pos.x = character->wheelBackPos.x+(Math::Rand()-0.5f)*2.0f*character->wheelBackDim;
    pos.y = 0.0f;
    pos.z = -character->wheelBackPos.z-Math::Rand()*character->wheelBackDim;
    pos = Transform(*mat, pos);
    dim.x = (0.5f+force*0.8f+Math::Rand()*1.0f)*factor;
    dim.y = dim.x;
    m_particle->CreateParticle(pos, speed, dim, parti, delay, mass);

#if 0
    // Gommes qui brűlent tant le démarrage est rapide.
    if ( !bBrake               &&  // dragster ?
         m_terrainHard >= 0.6f &&  // sol dur ?
         wType != WT_BURN      &&  // roues pas cramées ?
         !m_bWater             )   // pas sous l'eau ?
    {
        pos.x = character->wheelBackPos.x+(Math::Rand()-0.5f)*2.0f*character->wheelBackDim;
        pos.y = 0.0f;
        pos.z = character->wheelBackPos.z+Math::Rand()*character->wheelBackDim;
        pos = Transform(*mat, pos);
        speed = Transform(*mat, Math::Vector(m_linMotion.realSpeed.x/2.0f, 0.0f, 0.0f));
        speed.y += 4.0f+Math::Rand()*4.0f;
        speed -= m_object->GetPosition(0);
        dim.x = (0.5f+force*0.8f+Math::Rand()*1.0f)*factor;
        dim.y = dim.x;
        delay = 1.0f+Math::Rand()*2.0f;
        m_particle->CreateParticle(pos, speed, dim, PARTIWHEEL, delay, 0.0f);

        pos.x = character->wheelBackPos.x+(Math::Rand()-0.5f)*2.0f*character->wheelBackDim;
        pos.y = 0.0f;
        pos.z = -character->wheelBackPos.z-Math::Rand()*character->wheelBackDim;
        pos = Transform(*mat, pos);
        speed = Transform(*mat, Math::Vector(m_linMotion.realSpeed.x/2.0f, 0.0f, 0.0f));
        speed.y += 4.0f+Math::Rand()*4.0f;
        speed -= m_object->GetPosition(0);
        dim.x = (0.5f+force*0.8f+Math::Rand()*1.0f)*factor;
        dim.y = dim.x;
        delay = 1.0f+Math::Rand()*2.0f;
        m_particle->CreateParticle(pos, speed, dim, PARTIWHEEL, delay, 0.0f);
    }
#endif

    // Ejection de petites pierres.
    if ( !bBrake              &&  // dragster ?
         m_terrainHard < 0.6f &&  // sol mou ?
         wType != WT_BURN     )   // roues pas cramées ?
    {
        pos.x = character->wheelBackPos.x;
        pos.y = 0.0f;
        pos.z = character->wheelBackPos.z+Math::Rand()*character->wheelBackDim;
        pos = Transform(*mat, pos);
        speed.x = (Math::Rand()-0.5f)*15.0f;
        speed.z = (Math::Rand()-0.5f)*15.0f;
        speed.y = 12.0f+Math::Rand()*12.0f;
        dim.x = (0.5f+force*0.8f+Math::Rand()*1.0f)*0.05f;
        dim.y = dim.x;
        mass = 20.0f+Math::Rand()*20.0f;
        parti = static_cast<Gfx::ParticleType>(Gfx::PARTISTONE1+rand()%4);
        if ( m_bWater )  parti = Gfx::PARTIBUBBLE;
        m_particle->CreateParticle(pos, speed, dim, parti, 3.0f, mass);

        pos.x = character->wheelBackPos.x;
        pos.y = 0.0f;
        pos.z = -character->wheelBackPos.z-Math::Rand()*character->wheelBackDim;
        pos = Transform(*mat, pos);
        speed.x = (Math::Rand()-0.5f)*15.0f;
        speed.z = (Math::Rand()-0.5f)*15.0f;
        speed.y = 12.0f+Math::Rand()*12.0f;
        dim.x = (0.5f+force*0.8f+Math::Rand()*1.0f)*0.05f;
        dim.y = dim.x;
        mass = 20.0f+Math::Rand()*20.0f;
        parti = static_cast<Gfx::ParticleType>(Gfx::PARTISTONE1+rand()%4);
        if ( m_bWater )  parti = Gfx::PARTIBUBBLE;
        m_particle->CreateParticle(pos, speed, dim, parti, 3.0f, mass);
    }
}

// Génčre qq particules selon la suspension.

void CPhysics::SuspParticle(float aTime, float rTime)
{
    Math::Matrix*    mat;
    Character*    character;
    Math::Vector    pos, speed;
    Math::Point        dim;
    float        factor, speedx;
    int            i;

    return;
    if ( m_floorHeight > 0.0f )  return;
    if ( m_terrainHard < 0.5f )  return;

    character = m_object->GetCharacter();

    factor = m_linMotion.realSpeed.z;
    factor = fabs(factor/20.0f);
    if ( factor <= 0.0f )  return;

    speedx = fabs(m_linMotion.realSpeed.x/m_linMotion.advanceSpeed.x);
    speedx = (speedx-0.5f)/0.5f;
//?    if ( speedx <= 0.0f )  return;
//?    factor *= speedx;

    if ( m_lastSuspParticle+m_engine->ParticleAdapt(0.05f) <= aTime )
    {
        m_lastSuspParticle = aTime;

        mat = m_object->GetWorldMatrix(0);
        for ( i=0 ; i<static_cast<int>(factor*10.0f) ; i++ )
        {
            pos.x = character->wheelBackPos.x*Math::Rand();
            pos.y = 0.0f;
            pos.z = character->wheelBackPos.z*(Math::Rand()-0.5f)*1.5f;
            pos = Transform(*mat, pos);
            speed.x = -m_linMotion.realSpeed.x/5.0f;
            speed.y =  m_linMotion.realSpeed.x/20.0f;
            speed.z = (Math::Rand()-0.5f)*6.0f;
            speed = Transform(*mat, speed);
            speed -= m_object->GetPosition();
            dim.x = 0.2f+Math::Rand()*0.2f;
            dim.y = dim.x;
//            duration = 1.0f+Math::Rand()*1.0f;
//?            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIGLINT, duration, 20.0f);
            m_particle->CreateTrack(pos, speed, dim, Gfx::PARTITRACK1,
                                     2.0f, 20.0f, 0.5f, 1.0f);
        }
    }
}

// Calcule position et vitesse pour les gaz d'échappement.

void CPhysics::GazCompute(int model, Math::Matrix *mat, int i, float dir,
                          float factor,
                          Math::Vector &pos, Math::Vector &speed)
{
    if ( model == 1 )  // tijuana taxi ?
    {
        pos = Math::Vector(2.7f+0.5f*i, 3.1f, 3.1f*dir);
        speed = pos;
        speed.x += ((Math::Rand()-0.5f)*2.0f)*factor;
        speed.y += (1.0f+Math::Rand()*2.0f)*factor;
        speed.z += (1.0f+Math::Rand()*2.0f)*factor*dir;
        pos   = Transform(*mat, pos);
        speed = Transform(*mat, speed)-pos;
    }

    if ( model == 2 )  // ford 32?
    {
        pos = Math::Vector(1.8f+0.5f*i, 2.2f, 3.3f*dir);
        speed = pos;
        speed.x += ((Math::Rand()-0.5f)*2.0f)*factor;
        speed.y += (1.0f+Math::Rand()*2.0f)*factor;
        speed.z += (1.0f+Math::Rand()*2.0f)*factor*dir;
        pos   = Transform(*mat, pos);
        speed = Transform(*mat, speed)-pos;
    }

    if ( model == 3 )  // pickup ?
    {
        if ( i == 0 )  pos = Math::Vector(2.10f, 1.2f, 3.3f*dir*1.1f);
        if ( i == 1 )  pos = Math::Vector(2.65f, 1.4f, 3.3f*dir*1.1f);
        if ( i == 2 )  pos = Math::Vector(3.20f, 1.6f, 3.3f*dir*1.1f);
        speed = pos;
        speed.x += ((Math::Rand()-0.5f)*2.0f)*factor;
        speed.y += ((Math::Rand()-0.5f)*2.0f)*factor;
        speed.z += (1.0f+Math::Rand()*2.0f)*factor*dir;
        pos   = Transform(*mat, pos);
        speed = Transform(*mat, speed)-pos;
    }

    if ( model == 4 )  // firecraker ?
    {
        pos = Math::Vector(2.6f, 2.0f, 2.7f*dir);
        speed = pos;
        speed.x += ((Math::Rand()-0.8f)*2.0f)*factor;
        speed.y += ((Math::Rand()-0.2f)*2.0f)*factor;
        speed.z += (1.0f+Math::Rand()*2.0f)*factor*dir;
        pos   = Transform(*mat, pos);
        speed = Transform(*mat, speed)-pos;
    }

    if ( model == 5 )  // hooligan ?
    {
        pos = Math::Vector(2.9f, 1.5f, 3.4f*dir);
        speed = pos;
        speed.x += ((Math::Rand()-0.8f)*2.0f)*factor;
        speed.y += ((Math::Rand()-0.8f)*2.0f)*factor;
        speed.z += (1.0f+Math::Rand()*2.0f)*factor*dir;
        pos   = Transform(*mat, pos);
        speed = Transform(*mat, speed)-pos;
    }

    if ( model == 6 )  // chevy ?
    {
        pos = Math::Vector(-1.8f, 1.0f, 3.4f*dir);
        speed = pos;
        speed.x += ((Math::Rand()-0.8f)*2.0f)*factor;
        speed.y += ((Math::Rand()-0.5f)*2.0f)*factor;
        speed.z += (1.0f+Math::Rand()*2.0f)*factor*dir;
        pos   = Transform(*mat, pos);
        speed = Transform(*mat, speed)-pos;
    }

    if ( model == 7 )  // reo ?
    {
        pos = Math::Vector(2.2f, 2.3f, 3.0f*dir);
        speed = pos;
        speed.x += ((Math::Rand()-0.8f)*2.0f)*factor;
        speed.y += ((Math::Rand()-0.2f)*2.0f)*factor;
        speed.z += (1.0f+Math::Rand()*2.0f)*factor*dir;
        pos   = Transform(*mat, pos);
        speed = Transform(*mat, speed)-pos;
    }

    if ( model == 8 )  // torpedo ?
    {
        pos = Math::Vector(-6.0f, 2.85f, -2.15f)*1.1f;
        speed = pos;
        speed.x += -(1.0f+Math::Rand()*4.0f)*factor;
        speed.y += ((Math::Rand()-0.5f)*2.0f)*factor;
        speed.z += ((Math::Rand()-0.5f)*2.0f)*factor;
        pos   = Transform(*mat, pos);
        speed = Transform(*mat, speed)-pos;
    }
}

// Generates some exhaust gas particle.

void CPhysics::MotorParticle(float aTime, float rTime)
{
    Gfx::ParticleType parti;
    Math::Matrix*   mat;
    Math::Vector    pos, speed;
    Math::Point     dim;
    ObjectType  type;
    Math::Point     c, p;
    float       h, a, delay, level, factor, boost, duration, size;
    int         r, i, nb, model;

    if ( m_type == TYPE_MASS )  return;
    if ( m_object->GetToy() )  return;

    type = m_object->GetType();
    model = m_object->GetModel();

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

    // BuzzingCars
    if ( type == OBJECT_CARROT  ||
         type == OBJECT_STARTER ||
         type == OBJECT_WALKER  ||
         type == OBJECT_CRAZY   ||
         type == OBJECT_GUIDE   ||
         type == OBJECT_EVIL1   ||
         type == OBJECT_EVIL2   ||
         type == OBJECT_EVIL3   ||
         type == OBJECT_EVIL4   ||
         type == OBJECT_EVIL5   )  return;

    if ( type == OBJECT_HUMAN )  delay = 3.0f;
    else                         delay = 8.0f;
    if ( m_bSwim && m_timeUnderWater < delay )  // bubbles when entering water?
    {
        if ( aTime-m_lastUnderParticle >= m_engine->ParticleAdapt(0.05f) )
        {
            m_lastUnderParticle = aTime;

            nb = static_cast<int>(20.0f-(20.0f/delay)*m_timeUnderWater);
            for ( i=0 ; i<nb ; i++ )
            {
                pos = m_object->GetPosition();
                pos.x += (Math::Rand()-0.5f)*4.0f;
                pos.y += (Math::Rand()-0.5f)*4.0f;
                pos.z += (Math::Rand()-0.5f)*4.0f;
                speed.y = (Math::Rand()-0.5f)*8.0f+8.0f;
                speed.x = (Math::Rand()-0.5f)*0.2f;
                speed.z = (Math::Rand()-0.5f)*0.2f;
                dim.x = 0.06f+Math::Rand()*0.10f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIBUBBLE, 3.0f, 0.0f, 0.0f);
            }
        }
    }

    level = m_water->GetLevel();
    pos = m_object->GetPosition();
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
         !m_water->GetLava()  )  // drops on leaving the water?
    {
        if ( aTime-m_lastUnderParticle >= m_engine->ParticleAdapt(0.05f) )
        {
            m_lastUnderParticle = aTime;

            nb = static_cast<int>(8.0f*m_absorbWater);
            for ( i=0 ; i<nb ; i++ )
            {
                pos = m_object->GetPosition();
                if ( type == OBJECT_HUMAN )  pos.y -= Math::Rand()*2.0f;
                else                         pos.y += Math::Rand()*2.0f;
                pos.x += (Math::Rand()-0.5f)*2.0f;
                pos.z += (Math::Rand()-0.5f)*2.0f;
                speed.y = -((Math::Rand()-0.5f)*8.0f+8.0f);
                speed.x = 0.0f;
                speed.z = 0.0f;
                dim.x = 0.2f;
                dim.y = 0.2f;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIWATER, 2.0f, 0.0f, 1.0f);
            }
        }
    }

    if ( type == OBJECT_HUMAN ||  // human?
         type == OBJECT_TECH  )
    {
        if ( m_bLand &&
             aTime-m_lastSlideParticle >= m_engine->ParticleAdapt(0.05f) )
        {
            h = Math::Max(fabs(m_linMotion.terrainSpeed.x),
                    fabs(m_linMotion.terrainSpeed.z));
            if ( h > m_linMotion.terrainSlide.x+0.5f &&
                 m_linMotion.motorSpeed.x == 0.0f )  // slides a stop?
            {
                m_lastSlideParticle = aTime;

                mat = m_object->GetWorldMatrix(0);
                pos.x = (Math::Rand()-0.5f)*1.0f;
                pos.y = -m_object->GetCharacter()->height;
                pos.z = Math::Rand()*0.4f+1.0f;
                if ( rand()%2 == 0 )  pos.z = -pos.z;
                pos = Transform(*mat, pos);
                speed = Math::Vector(0.0f, 1.0f, 0.0f);
                dim.x = Math::Rand()*(h-5.0f)/2.0f+1.0f;
                if ( dim.x > 2.5f )  dim.x = 2.5f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTICRASH, 2.0f, 0.0f, 0.2f);
            }
        }
    }

    if ( type == OBJECT_MOBILEta ||
         type == OBJECT_MOBILEtc ||
         type == OBJECT_MOBILEti ||
         type == OBJECT_MOBILEts || // caterpillars?
         type == OBJECT_MOBILEfb || // BC
         type == OBJECT_MOBILEob )  // BC
    {
        if ( aTime-m_lastSlideParticle >= m_engine->ParticleAdapt(0.05f) )
        {
            h = fabs(m_linMotion.motorSpeed.x-m_linMotion.realSpeed.x);
            if ( h > 5.0f )
            {
                m_lastSlideParticle = aTime;

                mat = m_object->GetWorldMatrix(0);
                pos.x = (Math::Rand()-0.5f)*8.0f;
                pos.y = 0.0f;
                pos.z = Math::Rand()*2.0f+3.0f;
                if ( rand()%2 == 0 )  pos.z = -pos.z;
                pos = Transform(*mat, pos);
                speed = Math::Vector(0.0f, 0.0f, 0.0f);
                dim.x = Math::Rand()*(h-5.0f)/2.0f+1.0f;
                if ( dim.x > 3.0f )  dim.x = 3.0f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTICRASH, 2.0f, 0.0f, 0.2f);
            }
        }
    }

    if ( type == OBJECT_MOBILErt ||
         type == OBJECT_MOBILErc ||
         type == OBJECT_MOBILErr ||
         type == OBJECT_MOBILErs )  // large caterpillars?
    {
        if ( aTime-m_lastSlideParticle >= m_engine->ParticleAdapt(0.05f) )
        {
            h = fabs(m_linMotion.motorSpeed.x-m_linMotion.realSpeed.x);
            if ( h > 5.0f )
            {
                m_lastSlideParticle = aTime;

                mat = m_object->GetWorldMatrix(0);
                pos.x = (Math::Rand()-0.5f)*9.0f;
                pos.y = 0.0f;
                pos.z = Math::Rand()*3.0f+3.0f;
                if ( rand()%2 == 0 )  pos.z = -pos.z;
                pos = Transform(*mat, pos);
                speed = Math::Vector(0.0f, 0.0f, 0.0f);
                dim.x = Math::Rand()*(h-5.0f)/2.0f+1.0f;
                if ( dim.x > 3.0f )  dim.x = 3.0f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTICRASH, 2.0f, 0.0f, 0.2f);
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
                 aTime-m_lastMotorParticle < m_engine->ParticleAdapt(0.05f) )  return;
            m_lastMotorParticle = aTime;

            pos = Math::Vector(-1.6f, -0.5f, 0.0f);
            mat = m_object->GetWorldMatrix(0);
            pos = Transform(*mat, pos);

            speed.x = (Math::Rand()-0.5f)*0.6f;
            speed.z = (Math::Rand()-0.5f)*0.6f;
            speed.y = -(0.5f+Math::Rand()*0.3f)*(1.0f-m_reactorTemperature);

            dim.x = (1.0f+Math::Rand()*0.5f)*(0.2f+m_reactorTemperature*0.8f);
            dim.y = dim.x;

            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISMOKE2, 3.0f, 0.0f, 0.1f);
        }
        else    // in flight?
        {
            if ( !m_bMotor || (m_object->Implements(ObjectInterfaceType::JetFlying) && dynamic_cast<CJetFlyingObject*>(m_object)->GetReactorRange() == 0.0f) )  return;

            if ( m_reactorTemperature < 1.0f )  // not too hot?
            {
                m_reactorTemperature += rTime*(1.0f/4.0f);  // heating
                if ( m_reactorTemperature > 1.0f )
                {
                    m_reactorTemperature = 1.0f;  // but not too much
                }
            }

            if ( aTime-m_lastMotorParticle < m_engine->ParticleAdapt(0.02f) )  return;
            m_lastMotorParticle = aTime;

            pos = Math::Vector(-1.6f, -1.0f, 0.0f);
            pos.x += (Math::Rand()-0.5f)*3.0f;
            pos.y += (Math::Rand()-0.5f)*1.5f;
            pos.z += (Math::Rand()-0.5f)*3.0f;
            mat = m_object->GetWorldMatrix(0);
            pos = Transform(*mat, pos);

            h = m_floorHeight;
            if ( h > 10.0f )  // high enough?
            {
                speed = Math::Vector(0.0f, -10.0f, 0.0f);  // against the bottom
            }
            else
            {
                speed.y = 10.0f-2.0f*h - Math::Rand()*(10.0f-h);  //against the top
                speed.x = (Math::Rand()-0.5f)*(5.0f-h)*1.0f;  // horizontal (xz)
                speed.z = (Math::Rand()-0.5f)*(5.0f-h)*1.0f;
            }

            dim.x = 0.12f;
            dim.y = 0.12f;

            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISCRAPS, 2.0f, 10.0f);

            pos = Math::Vector(-1.6f, -0.5f, 0.0f);
            pos = Transform(*mat, pos);

            speed.x = (Math::Rand()-0.5f)*1.0f;
            speed.z = (Math::Rand()-0.5f)*1.0f;
            speed.y = -(4.0f+Math::Rand()*3.0f);
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
            a = m_object->GetRotationY();
            p.x = speed.x;
            p.y = speed.z;
            p = Math::RotatePoint(-a, p);
            speed.x = p.x;
            speed.z = p.y;

            dim.x = 0.4f+Math::Rand()*0.2f;
            dim.y = dim.x;

            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIEJECT, 0.3f, 10.0f);
        }
    }

    if ( (type == OBJECT_HUMAN || type == OBJECT_TECH) && m_bSwim )
    {
        m_reactorTemperature = 0.0f;  // reactor cold
    }

    if ( m_object->Implements(ObjectInterfaceType::Flying) &&
         type != OBJECT_HUMAN &&
         type != OBJECT_TECH  &&
         !m_bSwim )
    {
        if ( m_bLand )  // on the ground?
        {
            if ( m_motorSpeed.x == 0.0f &&  // glide slope due to ground?
                 m_cirMotion.realSpeed.y == 0.0f )
            {
                h = Math::Max(fabs(m_linMotion.realSpeed.x),
                        fabs(m_linMotion.realSpeed.z));

                if ( h < 3.0f )  return;

                if ( aTime-m_lastMotorParticle < m_engine->ParticleAdapt(0.2f) )  return;
                m_lastMotorParticle = aTime;

                r = rand()%3;
                if ( r == 0 )  pos = Math::Vector(-3.0f, 0.0f, -4.0f);
                if ( r == 1 )  pos = Math::Vector(-3.0f, 0.0f,  4.0f);
                if ( r == 2 )  pos = Math::Vector( 4.0f, 0.0f,  0.0f);

                pos.x += (Math::Rand()-0.5f)*2.0f;
                pos.z += (Math::Rand()-0.5f)*2.0f;
                mat = m_object->GetWorldMatrix(0);
                pos = Transform(*mat, pos);
                speed = Math::Vector(0.0f, 0.0f, 0.0f);
                dim.x = Math::Rand()*h/5.0f+2.0f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTICRASH, 2.0f);
            }
            else    // glide with small reactors in skates?
            {
                if ( m_linMotion.realSpeed.x == 0.0f &&
                     m_cirMotion.realSpeed.y == 0.0f )  return;

                if ( aTime-m_lastMotorParticle < m_engine->ParticleAdapt(0.02f) )  return;
                m_lastMotorParticle = aTime;

                r = rand()%3;
                if ( r == 0 )  pos = Math::Vector(-3.0f, 0.0f, -4.0f);
                if ( r == 1 )  pos = Math::Vector(-3.0f, 0.0f,  4.0f);
                if ( r == 2 )  pos = Math::Vector( 4.0f, 0.0f,  0.0f);

                pos.x += (Math::Rand()-0.5f)*1.0f;
                pos.z += (Math::Rand()-0.5f)*1.0f;
                mat = m_object->GetWorldMatrix(0);
                pos = Transform(*mat, pos);
                speed = Math::Vector(0.0f, 0.0f, 0.0f);
                dim.x = 1.0f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIEJECT);
            }
        }
        else    // in flight?
        {
            if ( !m_bMotor || (m_object->Implements(ObjectInterfaceType::JetFlying) && dynamic_cast<CJetFlyingObject*>(m_object)->GetReactorRange() == 0.0f) )  return;

            if ( aTime-m_lastMotorParticle < m_engine->ParticleAdapt(0.02f) )  return;
            m_lastMotorParticle = aTime;

            pos = Math::Vector(0.0f, -1.0f, 0.0f);
            pos.x += (Math::Rand()-0.5f)*6.0f;
            pos.y += (Math::Rand()-0.5f)*3.0f;
            pos.z += (Math::Rand()-0.5f)*6.0f;
            mat = m_object->GetWorldMatrix(0);
            pos = Transform(*mat, pos);

            h = m_floorHeight;
            if ( h > 10.0f )  // high enough?
            {
                speed = Math::Vector(0.0f, -10.0f, 0.0f);  // against the bottom
            }
            else
            {
                speed.y = 10.0f-2.0f*h - Math::Rand()*(10.0f-h);  // against the top
                speed.x = (Math::Rand()-0.5f)*(10.0f-h)*2.0f;  // horizontal (xz)
                speed.z = (Math::Rand()-0.5f)*(10.0f-h)*2.0f;
            }

            dim.x = 0.2f;
            dim.y = 0.2f;

            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISCRAPS, 2.0f, 10.0f);

            pos = Math::Vector(0.0f, 1.0f, 0.0f);
            pos = Transform(*mat, pos);

            speed.x = (Math::Rand()-0.5f)*1.0f;
            speed.z = (Math::Rand()-0.5f)*1.0f;
            speed.y = -(6.0f+Math::Rand()*4.5f);
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
            a = m_object->GetRotationY();
            p.x = speed.x;
            p.y = speed.z;
            p = Math::RotatePoint(-a, p);
            speed.x = p.x;
            speed.z = p.y;

            dim.x = 0.7f+Math::Rand()*0.6f;
            dim.y = dim.x;

            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIEJECT, 0.5f, 10.0f);
        }
    }

    if ( (type == OBJECT_HUMAN || type == OBJECT_TECH) && m_bSwim )
    {
        if ( !m_object->Implements(ObjectInterfaceType::Destroyable) || dynamic_cast<CDestroyableObject*>(m_object)->GetDying() != DeathType::Dead )
        {
            h = Math::Mod(aTime, 5.0f);
            if ( h < 3.5f && ( h < 1.5f || h > 1.6f ) )  return;
        }
        if ( aTime-m_lastMotorParticle < m_engine->ParticleAdapt(0.06f) )  return;
        m_lastMotorParticle = aTime;

        pos = Math::Vector(0.0f, 3.0f, 0.0f);
        mat = m_object->GetWorldMatrix(0);
        pos = Transform(*mat, pos);
        pos.x += (Math::Rand()-0.5f)*1.0f;
        pos.z += (Math::Rand()-0.5f)*1.0f;
        speed.y = (Math::Rand()-0.5f)*8.0f+8.0f;
        speed.x = (Math::Rand()-0.5f)*0.2f;
        speed.z = (Math::Rand()-0.5f)*0.2f;
        dim.x = 0.2f;
        dim.y = 0.2f;
        m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIBUBBLE, 3.0f, 0.0f, 0.0f);

        if ( aTime-m_lastSoundWater > 1.5f )
        {
            m_lastSoundWater = aTime;
            m_sound->Play(SOUND_BLUP, m_object->GetPosition(), 0.5f+Math::Rand()*0.5f);
        }
    }

    if ( type == OBJECT_MOBILEsa && m_bSwim )
    {
        h = Math::Mod(aTime, 3.0f);
        if ( h < 1.5f && ( h < 0.5f || h > 0.9f ) )  return;
        if ( aTime-m_lastMotorParticle < m_engine->ParticleAdapt(0.06f) )  return;
        m_lastMotorParticle = aTime;

        pos = Math::Vector(0.0f, 3.0f, 0.0f);
        mat = m_object->GetWorldMatrix(0);
        pos = Transform(*mat, pos);
        pos.x += (Math::Rand()-0.5f)*1.0f;
        pos.z += (Math::Rand()-0.5f)*1.0f;
        speed.y = (Math::Rand()-0.5f)*8.0f+8.0f;
        speed.x = (Math::Rand()-0.5f)*0.2f;
        speed.z = (Math::Rand()-0.5f)*0.2f;
        dim.x = 0.2f;
        dim.y = 0.2f;
        m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIBUBBLE, 3.0f, 0.0f, 0.0f);

        if ( aTime-m_lastSoundWater > 1.5f )
        {
            m_lastSoundWater = aTime;
            m_sound->Play(SOUND_BLUP, m_object->GetPosition(), 0.5f+Math::Rand()*0.5f);
        }
    }

    if ( !m_object->Implements(ObjectInterfaceType::Flying) )
    {
        if ( type == OBJECT_APOLLO2 )  return;  // electric motors!

        // Create engine smoke
        if ( type == OBJECT_MOBILErt ||
             type == OBJECT_MOBILErc ||
             type == OBJECT_MOBILErr ||
             type == OBJECT_MOBILErs ||
             type == OBJECT_MOBILEfb || // BC
             type == OBJECT_MOBILEob )  // BC
        {
            if ( !m_bMotor )  return;

            if ( aTime-m_lastMotorParticle < m_engine->ParticleAdapt(0.1f) )  return;
            m_lastMotorParticle = aTime;

            pos = Math::Vector(-2.5f, 10.3f, -1.3f);
            pos.x += (Math::Rand()-0.5f)*1.0f;
            pos.z += (Math::Rand()-0.5f)*1.0f;
            mat = m_object->GetWorldMatrix(0);
            pos   = Transform(*mat, pos);

            speed.x = (Math::Rand()-0.5f)*2.0f;
            speed.z = (Math::Rand()-0.5f)*2.0f;
            speed.y = 1.5f+Math::Rand()*1.0f;

            dim.x = Math::Rand()*0.6f+0.4f;
            dim.y = dim.x;

            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIMOTOR, 2.0f);
        }
        else if ( type == OBJECT_TRAX )
        {
//?            if ( !m_bMotor )  return;
//?            if ( m_linMotion.motorSpeed.x == 0.0f )  return;

            if ( aTime-m_lastMotorParticle < m_engine->ParticleAdapt(0.1f) )  return;
            m_lastMotorParticle = aTime;

            mat = m_object->GetWorldMatrix(0);
            for ( i=0 ; i<5 ; i++ )
            {
                pos = Math::Vector(2.0f, 16.0f, -2.0f);
                pos = Transform(*mat, pos);
                speed.x = (Math::Rand()-0.5f)*10.0f;
                speed.z = (Math::Rand()-0.5f)*10.0f;
                speed.y = 10.0f+Math::Rand()*10.0f;
                dim.x = Math::Rand()*2.0f+1.0f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISMOKE3, 2.0f);
            }
        }
        else if ( type == OBJECT_UFO )
        {
            if ( !m_bMotor )  return;
            if ( m_linMotion.motorSpeed.x == 0.0f )  return;

            if ( aTime-m_lastMotorParticle < m_engine->ParticleAdapt(0.05f) )  return;
            m_lastMotorParticle = aTime;

            mat = m_object->GetWorldMatrix(0);
            for ( i=0 ; i<10 ; i++ )
            {
                pos = Math::Vector(-6.0f, 3.0f, 0.0f);
                pos = Transform(*mat, pos);
                speed.x = (Math::Rand()-0.5f)*3.0f;
                speed.y = (Math::Rand()-0.5f)*3.0f;
                speed.z = (Math::Rand()-0.5f)*3.0f;
                dim.x = (Math::Rand()*1.5f+1.5f);
                dim.y = dim.x;
                duration = 0.6f+Math::Rand()*0.6f;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIFLAME, duration);

                pos.x = (Math::Rand()-0.5f)*5.0f;
                pos.z = (Math::Rand()-0.5f)*5.0f;
                pos.y = -1.0f;
                pos = Transform(*mat, pos);
                speed.x = (Math::Rand()-0.5f)*5.0f;
                speed.z = (Math::Rand()-0.5f)*5.0f;
                speed.y = -(15.0f+Math::Rand()*15.0f);
                dim.x = (Math::Rand()*0.2f+0.2f);
                dim.y = dim.x;
                duration = 0.6f+Math::Rand()*0.6f;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIEJECT, duration, 20.0f);
            }

            // Fume si soucoupe abimée.
            factor = m_object->GetShield();
            if ( factor < 0.7f )
            {
                pos = Math::Vector(-6.0f, 3.0f, 0.0f);
                pos = Transform(*mat, pos);
                speed.x = (Math::Rand()-0.5f)*5.0f;
                speed.z = (Math::Rand()-0.5f)*5.0f;
                speed.y = (12.0f+Math::Rand()*12.0f);
                dim.x = (Math::Rand()*8.0f+8.0f)*(1.0f-factor);
                dim.y = dim.x;
                duration = 1.0f+Math::Rand()*1.0f;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISMOKE1, duration);
            }
        }
        else if ( m_type == TYPE_RACE )
        {
            if ( m_object->IsDying() || GetLock() || m_bSilent )  return;

            if ( m_bWater && aTime-m_lastSoundWater > 0.0f )
            {
                m_lastSoundWater = aTime+Math::Rand()*1.0f+1.0f;
                m_sound->Play(Math::Rand()>0.5f?SOUND_BLUP:SOUND_SWIM, m_object->GetPosition(), 0.5f+Math::Rand()*0.5f);
            }

            if ( aTime-m_lastMotorParticle < m_engine->ParticleAdapt(0.05f) )  return;
            m_lastMotorParticle = aTime;

            mat = m_object->GetWorldMatrix(0);
            factor = fabs(m_linMotion.realSpeed.x)/m_linMotion.advanceSpeed.x;
            factor = 1.0f-2.0f*factor;
            if ( m_timeLock < STARTDELAY+0.5f )  // démarrage moteur ?
            {
                size = 4.0f-((m_timeLock-STARTDELAY)/0.5f)*3.0f;
            }
            else
            {
                size = 1.0f;
            }
            if ( factor > 0.2f && m_linMotion.realSpeed.x < 80.0f )
            {
                parti = m_bWater?Gfx::PARTIBUBBLE:Gfx::PARTISMOKE3;
                for ( i=0 ; i<3 ; i++ )
                {
                    GazCompute(model, mat, i, 1.0f, factor, pos, speed);
                    dim.x = (Math::Rand()*0.4f+0.3f)*factor*size;
                    dim.y = dim.x;
                    m_particle->CreateParticle(pos, speed, dim, parti, 2.0f);

                    GazCompute(model, mat, i, -1.0f, factor, pos, speed);
                    dim.x = (Math::Rand()*0.4f+0.3f)*factor*size;
                    dim.y = dim.x;
                    m_particle->CreateParticle(pos, speed, dim, parti, 2.0f);
                }
            }
            if ( m_wheelSlide > 0.0f && m_linMotion.realSpeed.x < 80.0f )
            {
                boost = 1.0f+m_wheelSlide*2.0f;
                parti = m_bWater?Gfx::PARTIBUBBLE:Gfx::PARTIFLAME;
                for ( i=0 ; i<3 ; i++ )
                {
                    GazCompute(model, mat, i, 1.0f, boost, pos, speed);
                    dim.x = (Math::Rand()*0.4f+0.3f)*boost;
                    dim.y = dim.x;
                    m_particle->CreateParticle(pos, speed, dim, parti, 0.3f, 20.0f);

                    GazCompute(model, mat, i, -1.0f, boost, pos, speed);
                    dim.x = (Math::Rand()*0.4f+0.3f)*boost;
                    dim.y = dim.x;
                    m_particle->CreateParticle(pos, speed, dim, parti, 0.3f, 20.0f);
                }
            }
            if ( factor <= 0.2f && m_wheelSlide <= 0.0f && m_linMotion.realSpeed.x < 80.0f )
            {
                parti = m_bWater?Gfx::PARTIBUBBLE:Gfx::PARTIMOTOR;
                for ( i=0 ; i<3 ; i++ )
                {
                    GazCompute(model, mat, i, 1.0f, 2.0f, pos, speed);
                    dim.x = (Math::Rand()*0.4f+0.3f);
                    dim.y = dim.x;
                    m_particle->CreateParticle(pos, speed, dim, parti, 0.7f, 10.0f+Math::Rand()*20.0f);

                    GazCompute(model, mat, i, -1.0f, 2.0f, pos, speed);
                    dim.x = (Math::Rand()*0.4f+0.3f);
                    dim.y = dim.x;
                    m_particle->CreateParticle(pos, speed, dim, parti, 0.7f, 10.0f+Math::Rand()*20.0f);
                }
            }
            if ( m_linMotion.realSpeed.x < 50.0f )
            {
                if ( model == 8 )  // torpedo ?
                {
                    parti = m_bWater?Gfx::PARTIBUBBLE:Gfx::PARTICRASH;
                    pos.x = (1.5f+(Math::Rand()-0.5f)*5.0f)*1.1f;
                    pos.y = (2.5f+(Math::Rand()-0.5f)*2.0f)*1.1f;
                    pos.z = (0.0f+(Math::Rand()-0.5f)*3.0f)*1.1f;
                    speed.x = (Math::Rand()-0.5f)*2.0f;
                    speed.y = Math::Rand()*4.0f+4.0f;
                    speed.z = Math::Rand()*3.0f+3.0f;
                    if ( Math::Rand() > 0.5f )  speed.z = -speed.z;
                    pos   = Transform(*mat, pos);
                    speed = Transform(*mat, speed)-pos;
                    dim.x = (Math::Rand()*1.0f+1.0f);
                    dim.y = dim.x;
                    m_particle->CreateParticle(pos, speed, dim, parti, Math::Rand()*0.7f+0.5f);
                }
            }
        }
        else
        {
            if ( !m_bMotor )  return;

            if ( aTime-m_lastMotorParticle < m_engine->ParticleAdapt(0.05f) )  return;
            m_lastMotorParticle = aTime;

            pos = Math::Vector(-3.4f, 1.8f, 0.5f);

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
            speed.y -= 0.5f+Math::Rand()*2.0f;
            speed.z += (Math::Rand()-0.5f)*3.0f;

            mat = m_object->GetWorldMatrix(0);
            pos   = Transform(*mat, pos);
            speed = Transform(*mat, speed)-pos;

            dim.x = Math::Rand()*0.4f+0.3f;
            dim.y = dim.x;

            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIMOTOR, 2.0f);
        }
    }
}

// Generates some particles after falling into the water.

void CPhysics::WaterParticle(float aTime, Math::Vector pos, ObjectType type,
                              float floor, float advance, float turn)
{
    Math::Vector    ppos, speed;
    Math::Point     dim;
    float       delay, level, min, max, force, volume, diam, speedy;
    int         i, nb;

    level = m_water->GetLevel();
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

//?    speedy = m_linMotion.realSpeed.y;
    speedy = m_graviSpeed;

    // Management of the particle "splash".
    if ( speedy < -5.0f   &&
         aTime-m_lastPloufParticle >= 1.0f )
    {
        m_lastPloufParticle = aTime;

        force = -speedy/20.0f;  // power according to speed drops
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

        pos = m_object->GetPosition();
        pos.y = m_water->GetLevel()-1.0f;
        dim.x = 2.0f*force;  // height
        dim.y = diam;  // diameter
        m_particle->CreateParticle(pos, Math::Vector(0.0f, 0.0f, 0.0f), dim, Gfx::PARTIPLOUF0, 1.4f, 0.0f, 0.0f);

        force = (0.5f+force*0.5f);
        nb = static_cast<int>(force*50.0f*m_engine->GetParticleDensity());
        for ( i=0 ; i<nb ; i++ )
        {
            ppos = pos;
            ppos.x += (Math::Rand()-0.5f)*4.0f;
            ppos.z += (Math::Rand()-0.5f)*4.0f;
            ppos.y += 0.6f;
            speed.x = (Math::Rand()-0.5f)*12.0f*force;
            speed.z = (Math::Rand()-0.5f)*12.0f*force;
            speed.y = 6.0f+Math::Rand()*6.0f*force;
            dim.x = 0.5f;
            dim.y = dim.x;
            m_particle->CreateParticle(ppos, speed, dim, Gfx::PARTIDROP, 2.0f, 20.0f, 0.2f);
        }

        volume = fabs(speedy*0.02f);
        if ( volume > 1.0f )  volume = 1.0f;
        m_sound->Play(SOUND_PLOUF, pos, volume);
    }

    // Management particles "cop".
    if ( m_water->GetLava() )  return;

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
    m_engine->ParticleAdapt(delay);

    if ( aTime-m_lastWaterParticle < delay )  return;
    m_lastWaterParticle = aTime;

    force = (advance+turn)*0.16f;
    if ( force < 0.001f )  return;

    pos = m_object->GetPosition();
    pos.y = level+0.1f;
    if ( advance == 0 )
    {
        pos.x += (Math::Rand()-0.5f)*10.0f;
        pos.z += (Math::Rand()-0.5f)*10.0f;
    }
    else
    {
        pos.x += (Math::Rand()-0.5f)*4.0f;
        pos.z += (Math::Rand()-0.5f)*4.0f;
    }
    speed.y = 0.0f;
    speed.x = 0.0f;
    speed.z = 0.0f;
    dim.x = Math::Min(Math::Rand()*force+force+1.0f, 10.0f);
    dim.y = dim.x;
    m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIFLIC, 3.0f, 0.0f, 0.0f);
}

// Creates the trace under the robot.

void CPhysics::DrawingPenParticle(TraceColor color, float width)
{
    Math::Matrix*       mat;
    Math::Vector        goal1, goal2, wheel1, wheel2;
    float               dist1, dist2, step;

    mat = m_object->GetWorldMatrix(0);

    // Draw a trace on the ground.
    if ( color != TraceColor::Default )
    {
        step = 2.0f;
        if ( color == TraceColor::BlackArrow ||
             color == TraceColor::RedArrow   )
        {
            step = 4.0f;  // arrow?
        }
        step /= m_engine->GetTracePrecision();

        goal1.x = step/2.0f;
        goal1.y = 0.0f;
        goal1.z = -width/2.0f;
        goal1 = Transform(*mat, goal1);

        goal2.x = step/2.0f;
        goal2.y = 0.0f;
        goal2.z = width/2.0f;
        goal2 = Transform(*mat, goal2);

        if ( !m_bWheelParticleBrake )
        {
            m_wheelParticlePos[0] = goal1;
            m_wheelParticlePos[1] = goal2;
        }

        while ( true )
        {
            dist1 = Math::Distance(m_wheelParticlePos[0], goal1);
            if ( dist1 < step )  break;

            dist2 = Math::Distance(m_wheelParticlePos[1], goal2);

            wheel1 = Math::SegmentPoint(m_wheelParticlePos[0], goal1, step);
            wheel2 = Math::SegmentPoint(m_wheelParticlePos[1], goal2, step * dist2 / dist1);

            if ( m_linMotion.realSpeed.x >= 0.0f )
            {
                m_particle->CreateDrawingPenTrace(m_wheelParticlePos[0], m_wheelParticlePos[1], wheel1, wheel2, color);
            }
            else
            {
                m_particle->CreateDrawingPenTrace(m_wheelParticlePos[1], m_wheelParticlePos[0], wheel2, wheel1, color);
            }

            m_wheelParticlePos[0] = wheel1;
            m_wheelParticlePos[1] = wheel2;
        }

        m_bWheelParticleBrake = true;
    }
    else
    {
        m_bWheelParticleBrake = false;
    }
}


// Returns an error related to the general state.

Error CPhysics::GetError()
{
    ObjectType type = m_object->GetType();
    if ( type == OBJECT_HUMAN    ||
         type == OBJECT_TECH     ||
         type == OBJECT_MOTHER   ||
         type == OBJECT_ANT      ||
         type == OBJECT_SPIDER   ||
         type == OBJECT_BEE      ||
         type == OBJECT_WORM     ||
         type == OBJECT_APOLLO2  ||
         type == OBJECT_MOBILEdr )  return ERR_OK;

    if (m_object->Implements(ObjectInterfaceType::ProgramStorage))
    {
        if ( dynamic_cast<CProgramStorageObject*>(m_object)->GetActiveVirus() )
        {
            return ERR_VEH_VIRUS;
        }
    }

    if (m_object->Implements(ObjectInterfaceType::Powered))
    {
        CObject* power = dynamic_cast<CPoweredObject*>(m_object)->GetPower();  // searches for the object battery used
        if (power == nullptr || !power->Implements(ObjectInterfaceType::PowerContainer))
        {
            return ERR_VEH_POWER;
        }
        else
        {
            if ( dynamic_cast<CPowerContainerObject*>(power)->GetEnergy() == 0.0f )  return ERR_VEH_ENERGY;
        }
    }

    return ERR_OK;
}

void CPhysics::SetFalling()
{
    if (m_fallingHeight == 0.0f && m_floorHeight >= m_minFallingHeight)
        m_fallingHeight = m_object->GetPosition().y;
}

float CPhysics::GetFallingHeight()
{
    return m_fallingHeight;
}

void CPhysics::SetMinFallingHeight(float value)
{
    if (value < 0.0f) return;
    m_minFallingHeight = value;
}

float CPhysics::GetMinFallingHeight()
{
    return m_minFallingHeight;
}

void CPhysics::SetFallDamageFraction(float value)
{
    if (value < 0.0f) return;
    m_fallDamageFraction = value;
}

float CPhysics::GetFallDamageFraction()
{
    return m_fallDamageFraction;
}
