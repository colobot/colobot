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


#include "physics/physics.h"

#include "app/app.h"

#include "common/event.h"
#include "common/global.h"

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

#include "object/interface/jostleable_object.h"
#include "object/interface/slotted_object.h"
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




// Object's constructor.

CPhysics::CPhysics(COldObject* object)
{
    m_object    = object;
    m_engine    = Gfx::CEngine::GetInstancePointer();
    m_lightMan  = m_engine->GetLightManager();
    m_particle  = m_engine->GetParticle();
    m_water     = m_engine->GetWater();
    m_terrain   = CRobotMain::GetInstancePointer()->GetTerrain();
    m_camera    = CRobotMain::GetInstancePointer()->GetCamera();
    m_sound     = CApplication::GetInstancePointer()->GetSound();
    m_motion    = nullptr;

    m_gravity = 9.81f;  // default gravity
    m_time = 0.0f;
    m_timeUnderWater = 0.0f;
    m_motorSpeed = glm::vec3(0.0f, 0.0f, 0.0f);
    m_bMotor = false;
    m_bLand = true;  // ground
    m_bSwim = false;  // in air
    m_bCollision = false;
    m_bObstacle = false;
    m_repeatCollision = 0;
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
    m_bWheelParticleBrake = false;
    m_absorbWater        = 0.0f;
    m_reactorTemperature = 0.0f;
    m_timeReactorFail    = 0.0f;
    m_lastEnergy = 0.0f;
    m_lastSoundWater = 0.0f;
    m_lastSoundInsect = 0.0f;
    m_restBreakParticle = 0.0f;
    m_floorHeight = 0.0f;
    m_soundChannel = -1;
    m_soundChannelSlide = -1;
    m_soundTimePshhh = 0.0f;
    m_soundTimeJostle = 0.0f;
    m_soundTimeBoum = 0.0f;
    m_bSoundSlow = true;
    m_bFreeze = false;
    m_bForceUpdate = true;
    m_bLowLevel = false;
    m_fallingHeight = 0.0f;
    m_minFallingHeight = 20.0f;
    m_fallDamageFraction = 0.007f;
    m_floorLevel = 0.0f;
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
    if ( m_soundChannelSlide != -1 )
    {
        m_sound->FlushEnvelope(m_soundChannelSlide);
        m_sound->AddEnvelope(m_soundChannelSlide, 0.0f, 1.0f, 0.3f, SOPER_STOP);
        m_soundChannelSlide = -1;
    }
}




void CPhysics::SetMotion(CMotion* motion)
{
    m_motion = motion;
}


// Saves all parameters of the object.

bool CPhysics::Write(CLevelParserLine* line)
{
    line->AddParam("motor", std::make_unique<CLevelParserParam>(m_motorSpeed));

    if ( m_object->Implements(ObjectInterfaceType::Flying) )
    {
        if ( m_object->Implements(ObjectInterfaceType::JetFlying) )
        {
            line->AddParam("reactorRange", std::make_unique<CLevelParserParam>(m_object->GetReactorRange()));
        }
        line->AddParam("land", std::make_unique<CLevelParserParam>(GetLand()));
    }

    return true;
}

// Restores all parameters of the object.

bool CPhysics::Read(CLevelParserLine* line)
{
    m_motorSpeed = line->GetParam("motor")->AsPoint(glm::vec3(0.0f, 0.0f, 0.0f));

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


// Specifies the engine speed.
// x = forward/backward
// y = up/down
// z = turn

void CPhysics::SetMotorSpeed(glm::vec3 speed)
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

glm::vec3 CPhysics::GetMotorSpeed()
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


// Management of linear and angular velocities.
// Specifies the speed parallel to the direction of travel.

void CPhysics::SetLinMotion(PhysicsMode mode, glm::vec3 value)
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

glm::vec3 CPhysics::GetLinMotion(PhysicsMode mode)
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
    return glm::vec3(0.0f, 0.0f, 0.0f);
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
    return 0.0f;
}

// Specifies the rotation around the axis of walk.

void CPhysics::SetCirMotion(PhysicsMode mode, glm::vec3 value)
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

glm::vec3 CPhysics::GetCirMotion(PhysicsMode mode)
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
    return glm::vec3(0.0f, 0.0f, 0.0f);
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


// Management of an event.
// Returns false if the object is destroyed.

bool CPhysics::EventProcess(const Event &event)
{
    if ( event.type == EVENT_FRAME )
    {
        return EventFrame(event);
    }
    return true;
}


// Updates instructions for the motor speed.

void CPhysics::MotorUpdate(float aTime, float rTime)
{
    ObjectType  type;
    CPowerContainerObject* power = nullptr;
    glm::vec3    pos{ 0, 0, 0 }, motorSpeed{ 0, 0, 0 };
    float       energy, speed, factor, h;

    type = m_object->GetType();

    if(std::isnan(m_motorSpeed.x)) m_motorSpeed.x = 0.f;
    if(std::isnan(m_motorSpeed.y)) m_motorSpeed.y = 0.f;
    if(std::isnan(m_motorSpeed.z)) m_motorSpeed.z = 0.f;

    motorSpeed = m_motorSpeed;

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

    if (HasPowerCellSlot(m_object))
    {
        power = GetObjectPowerCell(m_object); // searches for the object battery uses
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

    if ( m_object->GetType() == OBJECT_HUMAN && dynamic_cast<CDestroyableObject&>(*m_object).GetDying() == DeathType::Dead )  // dead man?
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
         dynamic_cast<CJetFlyingObject&>(*m_object).GetRange() > 0.0f )  // limited flight range?
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

//? MotorParticle(aTime);

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
            reactorCool = dynamic_cast<CJetFlyingObject&>(*m_object).GetReactorRange() > 0.1f;
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
             type == OBJECT_MOBILEib ||
             type == OBJECT_MOBILEis ||
             type == OBJECT_MOBILEic ||
             type == OBJECT_MOBILEii ||
             type == OBJECT_MOBILEit )  factor = 0.5f;

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
}


// Updates the effects of vibration and tilt.

void CPhysics::EffectUpdate(float aTime, float rTime)
{
    Character*  character;
    glm::vec3    vibLin{ 0, 0, 0 }, vibCir{ 0, 0, 0 }, incl{ 0, 0, 0 };
    float       speedLin, speedCir, accel;
    ObjectType  type;
    bool        bOnBoard;

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
            m_motion->SetLinVibration(glm::vec3(0.0f, 0.0f, 0.0f));

//?         m_motion->SetCirVibration(glm::vec3(0.0f, 0.0f, 0.0f));
//?         m_motion->SetTilt(glm::vec3(0.0f, 0.0f, 0.0f));
        }
    }

    if ( type == OBJECT_MOBILEwa ||
         type == OBJECT_MOBILEwb ||
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
         type == OBJECT_MOBILEfb ||
         type == OBJECT_MOBILEfc ||
         type == OBJECT_MOBILEfi ||
         type == OBJECT_MOBILEfs ||
         type == OBJECT_MOBILEft )  // flying?
    {
        if ( m_bLand )  // on the ground?
        {
            m_motion->SetLinVibration(glm::vec3(0.0f, 0.0f, 0.0f));
            m_motion->SetCirVibration(glm::vec3(0.0f, 0.0f, 0.0f));
            m_motion->SetTilt(glm::vec3(0.0f, 0.0f, 0.0f));
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
    ObjectType  type;
    glm::mat4   matRotate;
    glm::vec3    iPos{ 0, 0, 0 }, iAngle{ 0, 0, 0 }, tAngle{ 0, 0, 0 }, pos{ 0, 0, 0 }, newpos{ 0, 0, 0 }, angle{ 0, 0, 0 }, newangle{ 0, 0, 0 }, n{ 0, 0, 0 };
    float       h, w;
    int         i;

    if ( m_engine->GetPause() )  return true;

    m_time += event.rTime;
    m_timeUnderWater += event.rTime;
    m_soundTimeJostle += event.rTime;

    type = m_object->GetType();

    FrameParticle(m_time, event.rTime);
    MotorUpdate(m_time, event.rTime);
    EffectUpdate(m_time, event.rTime);
    WaterFrame(m_time, event.rTime);

    iPos   = pos   = m_object->GetPosition();
    iAngle = angle = m_object->GetRotation();

    // Accelerate is the descent, brake is the ascent.
    if ( m_bFreeze || (m_object->Implements(ObjectInterfaceType::Destroyable) && dynamic_cast<CDestroyableObject&>(*m_object).IsDying()) )
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

    newangle = angle + event.rTime*m_cirMotion.realSpeed;
    Math::LoadRotationZXYMatrix(matRotate, newangle);
    newpos = event.rTime*m_linMotion.realSpeed;
    newpos = Math::Transform(matRotate, newpos);
    newpos += pos;

    m_terrain->AdjustToStandardBounds(newpos);

    if ( m_object->Implements(ObjectInterfaceType::Flying) && !m_bLand )
    {
        h = m_terrain->GetFlyingLimit(newpos, type==OBJECT_BEE);
        h += m_object->GetCharacter()->height;
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
            return false;
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

// Starts or stops the engine sounds.

void CPhysics::SoundMotor(float rTime)
{
    ObjectType  type;

    m_lastSoundInsect -= rTime;
    type = m_object->GetType();

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
        if ( dynamic_cast<CDestroyableObject&>(*m_object).GetDying() == DeathType::Burning ||
             dynamic_cast<CBaseAlien&>(*m_object).GetFixed() )
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
        else if ( dynamic_cast<CDestroyableObject&>(*m_object).GetDying() == DeathType::Burning )
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
        else if ( dynamic_cast<CDestroyableObject&>(*m_object).GetDying() == DeathType::Burning )
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
        if ( dynamic_cast<CDestroyableObject&>(*m_object).GetDying() == DeathType::Burning ||
             dynamic_cast<CBaseAlien&>(*m_object).GetFixed() )
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
    ObjectType  type;
    glm::vec3    pos{ 0, 0, 0 }, speed{ 0, 0, 0 };
    glm::vec2   dim;
    float       level;

    level = m_water->GetLevel();
    if ( level == 0.0f )  return;  // no water?
    if (IsObjectBeingTransported(m_object))  return;

    // Management of flames into the lava.
    pos = m_object->GetPosition();
    if ( m_water->GetLava() &&
         pos.y-m_object->GetCharacter()->height <= level )
    {
        if ( m_lastFlameParticle+m_engine->ParticleAdapt(0.05f) <= aTime )
        {
            m_lastFlameParticle = aTime;

            pos = m_object->GetPosition();
            pos.x += (Math::Rand()-0.5f)*3.0f;
            pos.z += (Math::Rand()-0.5f)*3.0f;
            speed.x = 0.0f;
            speed.z = 0.0f;
            speed.y = Math::Rand()*5.0f+3.0f;
            dim.x = Math::Rand()*2.0f+1.0f;
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIFLAME, 2.0f, 0.0f, 0.2f);

            pos = m_object->GetPosition();
            pos.y -= 2.0f;
            pos.x += (Math::Rand()-0.5f)*5.0f;
            pos.z += (Math::Rand()-0.5f)*5.0f;
            speed.x = 0.0f;
            speed.z = 0.0f;
            speed.y = 6.0f+Math::Rand()*6.0f+6.0f;
            dim.x = Math::Rand()*1.5f+1.0f+3.0f;
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISMOKE3, 4.0f);
        }
    }

    pos = m_object->GetPosition();
    if ( pos.y >= m_water->GetLevel(m_object) )  return;  // out of water?

    type = m_object->GetType();
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
         type == OBJECT_MOBILEfb ||
         type == OBJECT_MOBILEtb ||
         type == OBJECT_MOBILEwb ||
         type == OBJECT_MOBILEib ||
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
         type == OBJECT_MOBILErp ||
         type == OBJECT_MOBILEtg ||
         type == OBJECT_MOBILEdr ||
         type == OBJECT_APOLLO2  )
    {
        if (m_object->Implements(ObjectInterfaceType::Destroyable))
        {
            dynamic_cast<CDestroyableObject*>(m_object)->DestroyObject(DestructionType::ExplosionWater);
        }
    }
}

// Sounds the engine at full power.

void CPhysics::SoundMotorFull(float rTime, ObjectType type)
{
    SoundType   sound;
    float       amplitude, time, freq;

    if ( type == OBJECT_MOBILEia ||
         type == OBJECT_MOBILEib ||
         type == OBJECT_MOBILEic ||
         type == OBJECT_MOBILEii ||
         type == OBJECT_MOBILEis ||
         type == OBJECT_MOBILEit )
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

    if ( type == OBJECT_MOBILEsa ||
         type == OBJECT_MOBILEst )
    {
        sound = SOUND_MOTORs;
        amplitude = 0.6f;
        time = 0.5f;
    }
    else if ( type == OBJECT_MOBILErt ||
              type == OBJECT_MOBILErc ||
              type == OBJECT_MOBILErr ||
              type == OBJECT_MOBILErs ||
              type == OBJECT_MOBILErp )
    {
        sound = SOUND_MOTORr;
        amplitude = 1.0f;
        time = 0.7f;
    }
    else if ( type == OBJECT_MOBILEta ||
              type == OBJECT_MOBILEtc ||
              type == OBJECT_MOBILEti ||
              type == OBJECT_MOBILEts ||
              type == OBJECT_MOBILEtt )
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
    glm::vec3    pos{ 0, 0, 0 }, speed{ 0, 0, 0 };
    glm::vec2       dim;
    SoundType       sound;
    float       amplitude;
    int         i, max;

    if ( type == OBJECT_MOBILEia ||
         type == OBJECT_MOBILEib ||
         type == OBJECT_MOBILEic ||
         type == OBJECT_MOBILEii ||
         type == OBJECT_MOBILEis ||
         type == OBJECT_MOBILEit )
    {
        if ( m_soundChannel != -1 )  // engine is running?
        {
            m_sound->FlushEnvelope(m_soundChannel);
            m_sound->AddEnvelope(m_soundChannel, 0.0f, 1.0f, 0.3f, SOPER_STOP);
            m_soundChannel = -1;
        }
        return;
    }

    if ( type == OBJECT_MOBILEsa ||
         type == OBJECT_MOBILEst )
    {
        sound = SOUND_MOTORs;
        amplitude = 0.4f;
    }
    else if ( type == OBJECT_MOBILErt ||
              type == OBJECT_MOBILErc ||
              type == OBJECT_MOBILErr ||
              type == OBJECT_MOBILErs ||
              type == OBJECT_MOBILErp )
    {
        sound = SOUND_MOTORr;
        amplitude = 0.9f;
    }
    else if ( type == OBJECT_MOBILEta ||
              type == OBJECT_MOBILEtb ||
              type == OBJECT_MOBILEtc ||
              type == OBJECT_MOBILEti ||
              type == OBJECT_MOBILEts ||
              type == OBJECT_MOBILEtt )
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
         type == OBJECT_MOBILErs ||
         type == OBJECT_MOBILErp )
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
                pos = glm::vec3(-5.0f, 2.0f, 0.0f);
                pos.x += Math::Rand()*4.0f;
                pos.z += (Math::Rand()-0.5f)*2.0f;

                speed = pos;
                speed.x -= Math::Rand()*4.0f;
                speed.y -= Math::Rand()*3.0f;
                speed.z += (Math::Rand()-0.5f)*6.0f;

                glm::mat4 mat = m_object->GetWorldMatrix(0);
                pos   = Math::Transform(mat, pos);
                speed = Math::Transform(mat, speed)-pos;

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
    if ( type == OBJECT_MOBILEia ||
         type == OBJECT_MOBILEib ||
         type == OBJECT_MOBILEic ||
         type == OBJECT_MOBILEii ||
         type == OBJECT_MOBILEis ||
         type == OBJECT_MOBILEit )
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
    glm::vec3    pos{ 0, 0, 0 }, speed{ 0, 0, 0 };
    glm::vec2       dim;
    float       freq;
    int         i;

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
                    pos = glm::vec3(-1.6f, -0.5f, 0.0f);
                }
                else
                {
                    pos = glm::vec3(0.0f, -1.0f, 0.0f);
                }
                pos.x += (Math::Rand()-0.5f)*2.0f;
                pos.z += (Math::Rand()-0.5f)*2.0f;
                glm::mat4 mat = m_object->GetWorldMatrix(0);
                pos = Math::Transform(mat, pos);
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
                pos = glm::vec3(-1.6f, -0.5f, 0.0f);
            }
            else
            {
                pos = glm::vec3(0.0f, -1.0f, 0.0f);
            }
            pos.x += (Math::Rand()-0.5f)*1.0f;
            pos.z += (Math::Rand()-0.5f)*1.0f;
            glm::mat4 mat = m_object->GetWorldMatrix(0);
            pos = Math::Transform(mat, pos);
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
                          glm::vec3 &pos, glm::vec3 &angle)
{
    Character*  character;
    ObjectType  type;
    glm::vec3    norm{ 0, 0, 0 };
    glm::mat4    matRotate;
    float       level, h, f, a1, volume, freq, force;
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
            a1 = fabs(Math::RotateAngle(glm::length(glm::vec2(norm.x, norm.z)), norm.y));
            if ( a1 < (90.0f-55.0f)*Math::PI/180.0f )  // slope exceeds 55 degrees?
            {
                bSlopingTerrain = true;  // very sloped ground

                if ( h < 4.0f )  // collision with the ground?
                {
                    force = 5.0f+fabs(m_linMotion.realSpeed.x*0.3f)+
                                 fabs(m_linMotion.realSpeed.y*0.3f);
                    m_linMotion.currentSpeed = norm*force;
                    Math::LoadRotationXZYMatrix(matRotate, -angle);
                    m_linMotion.currentSpeed = Math::Transform(matRotate, m_linMotion.currentSpeed);

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
                CrashParticle(f);
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
        CTraceDrawingObject* traceDrawing = nullptr;
        if (m_object->Implements(ObjectInterfaceType::TraceDrawing))
            traceDrawing = dynamic_cast<CTraceDrawingObject*>(m_object);

        if (traceDrawing != nullptr && traceDrawing->GetTraceDown())
        {
            WheelParticle(traceDrawing->GetTraceColor(), traceDrawing->GetTraceWidth()*g_unit);
        }
        else
        {
            WheelParticle(TraceColor::Default, 0.0f);
        }
    }

    if ( type == OBJECT_HUMAN ||
         type == OBJECT_TECH  ||
         type == OBJECT_WORM  )  return;  // always right

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

// Calculates the angle of an object with the field.

void CPhysics::FloorAngle(const glm::vec3 &pos, glm::vec3 &angle)
{
    Character*  character;
    glm::vec3    pw{ 0, 0, 0 }, norm{ 0, 0, 0 };
    float       a1, a2;

    character = m_object->GetCharacter();

    pw.x = pos.x+character->wheelFront*cosf(angle.y+Math::PI*0.0f);
    pw.y = pos.y;
    pw.z = pos.z-character->wheelFront*sinf(angle.y+Math::PI*0.0f);
    a1 = atanf(m_terrain->GetHeightToFloor(pw)/character->wheelFront);

    pw.x = pos.x+character->wheelBack*cosf(angle.y+Math::PI*1.0f);
    pw.y = pos.y;
    pw.z = pos.z-character->wheelBack*sinf(angle.y+Math::PI*1.0f);
    a2 = atanf(m_terrain->GetHeightToFloor(pw)/character->wheelBack);

    angle.z = (a2-a1)/2.0f;

    pw.x = pos.x+character->wheelLeft*cosf(angle.y+Math::PI*0.5f)*cosf(angle.z);
    pw.y = pos.y;
    pw.z = pos.z-character->wheelLeft*sinf(angle.y+Math::PI*0.5f)*cosf(angle.z);
    a1 = atanf(m_terrain->GetHeightToFloor(pw)/character->wheelLeft);

    pw.x = pos.x+character->wheelRight*cosf(angle.y+Math::PI*1.5f)*cosf(angle.z);
    pw.y = pos.y;
    pw.z = pos.z-character->wheelRight*sinf(angle.y+Math::PI*1.5f)*cosf(angle.z);
    a2 = atanf(m_terrain->GetHeightToFloor(pw)/character->wheelRight);

    angle.x = (a2-a1)/2.0f;
}


// Adapts the physics of the object in relation to other objects.
// Returns 0 -> mobile object
// Returns 1 -> immobile object (because collision)
// Returns 2 -> destroyed object

int CPhysics::ObjectAdapt(const glm::vec3 &pos, const glm::vec3 &angle)
{
    glm::mat4    matRotate;
    glm::vec3    iPos{ 0, 0, 0 }, oAngle{ 0, 0, 0 }, oSpeed{ 0, 0, 0 };
    float           distance, force, volume;
    int             colType;
    ObjectType      iType, oType;

    if ( m_object->Implements(ObjectInterfaceType::Destroyable) && dynamic_cast<CDestroyableObject&>(*m_object).IsDying() )  return 0;  // is burning or exploding?
    if ( !m_object->GetCollisions() )  return 0;

    // iiPos = sphere center is the old position.
    // iPos  = sphere center has the new position.
    if (m_object->GetCrashSphereCount() < 1)
        return 0;

    auto firstCrashSphere = m_object->GetFirstCrashSphere();
    glm::vec3 iiPos = firstCrashSphere.sphere.pos;
    float iRad = firstCrashSphere.sphere.radius;

    iPos = iiPos + (pos - m_object->GetPosition());
    iType = m_object->GetType();

    for (CObject* pObj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        if ( pObj == m_object )  continue;  // yourself?
        if (IsObjectBeingTransported(pObj))  continue;
        if ( pObj->Implements(ObjectInterfaceType::Destroyable) && dynamic_cast<CDestroyableObject&>(*pObj).GetDying() == DeathType::Exploding )  continue;  // is exploding?

        oType = pObj->GetType();
        if ( oType == OBJECT_TOTO            )  continue;
        if ( !m_object->CanCollideWith(pObj) )  continue;

        if (pObj->Implements(ObjectInterfaceType::Jostleable))
        {
            JostleObject(dynamic_cast<CJostleableObject*>(pObj), iPos, iRad);
        }

        if ( oType == OBJECT_WAYPOINT &&
             !pObj->GetLock()         &&
             m_object->GetTrainer()   )  // driving vehicle?
        {
            glm::vec3 oPos = pObj->GetPosition();
            distance = Math::DistanceProjected(oPos, iPos);
            if ( distance < 4.0f )
            {
                m_sound->Play(SOUND_WAYPOINT, m_object->GetPosition());
                m_engine->GetPyroManager()->Create(Gfx::PT_WPCHECK, pObj);
            }
        }

        if ( oType == OBJECT_TARGET2 && !pObj->GetLock() )
        {
            glm::vec3 oPos = pObj->GetPosition();
            distance = glm::distance(oPos, iPos);
            if ( distance < 10.0f*1.5f )
            {
                m_sound->Play(SOUND_WAYPOINT, m_object->GetPosition());
                m_engine->GetPyroManager()->Create(Gfx::PT_WPCHECK, pObj);
            }
        }

        for (const auto& crashSphere : pObj->GetAllCrashSpheres())
        {
            glm::vec3 oPos = crashSphere.sphere.pos;
            float oRad = crashSphere.sphere.radius;

            // Aliens ignore small objects
            // TODO: But why? :/
            if ( iType == OBJECT_MOTHER && oRad <= 1.2f )  continue;
            if ( iType == OBJECT_ANT    && oRad <= 1.2f )  continue;
            if ( iType == OBJECT_SPIDER && oRad <= 1.2f )  continue;
            if ( iType == OBJECT_BEE    && oRad <= 1.2f )  continue;
            if ( iType == OBJECT_WORM   && oRad <= 1.2f )  continue;

            distance = glm::distance(oPos, iPos);
            if ( distance < iRad+oRad )  // collision?
            {
                distance = glm::distance(oPos, iiPos);
                if ( distance >= iRad+oRad )  // view (*)
                {
                    m_bCollision = true;
                    m_bObstacle = true;

                    if (crashSphere.sound != SOUND_NONE)
                    {
                        force = glm::length(m_linMotion.realSpeed);
                        force *= crashSphere.hardness*2.0f;
                        if ( ExploOther(iType, pObj, oType, force) )  continue;
                        colType = ExploHimself(iType, oType, force);
                        if ( colType == 2 )  return 2;  // destroyed?
                        if ( colType == 0 )  continue;  // ignores?
                    }

                    force = glm::length(m_linMotion.realSpeed);
                    force *= crashSphere.hardness;
                    volume = fabs(force*0.05f);
                    if ( volume > 1.0f )  volume = 1.0f;
                    if ( crashSphere.sound != SOUND_NONE )
                    {
                        m_sound->Play(crashSphere.sound, m_object->GetPosition(), volume);
                    }
                    if ( iType == OBJECT_HUMAN && volume > 0.5f )
                    {
                        m_sound->Play(SOUND_AIE, m_object->GetPosition(), volume);
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

                    m_linMotion.currentSpeed = glm::normalize(iPos-oPos)*force;
                    Math::LoadRotationXZYMatrix(matRotate, -angle);
                    m_linMotion.currentSpeed = Math::Transform(matRotate, m_linMotion.currentSpeed);
                    if ( !m_object->Implements(ObjectInterfaceType::Flying) )
                    {
                        m_linMotion.currentSpeed.y = 0.0f;
                    }


                    CPhysics* ph = nullptr;
                    if (pObj->Implements(ObjectInterfaceType::Movable))
                        ph = dynamic_cast<CMovableObject&>(*pObj).GetPhysics();
                    if ( ph != nullptr )
                    {
                        oAngle = pObj->GetRotation();
                        oSpeed = glm::normalize(oPos-iPos)*force;
                        Math::LoadRotationXZYMatrix(matRotate, -oAngle);
                        oSpeed = Math::Transform(matRotate, oSpeed);
                        if ( !pObj->Implements(ObjectInterfaceType::Flying) )
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

bool CPhysics::JostleObject(CJostleableObject* pObj, glm::vec3 iPos, float iRad)
{
    Math::Sphere jostlingSphere = pObj->GetJostlingSphere();

    float distance = glm::distance(jostlingSphere.pos, iPos);
    if ( distance >= iRad+jostlingSphere.radius)  return false;

    float d = (iRad+jostlingSphere.radius)/2.0f;
    float f = (distance-d)/d;  // 0 = off, 1 = near
    if ( f < 0.0f )  f = 0.0f;
    if ( f > 1.0f )  f = 1.0f;

    glm::vec3 speed = m_linMotion.realSpeed;
    speed.y = 0.0f;
    float force = glm::length(speed)*f*0.05f;
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
                          CObject *pObj, ObjectType oType, float force)
{
    JostleObject(pObj, 1.0f);  // shakes the object

    if (pObj->Implements(ObjectInterfaceType::Damageable))
    {
        // TODO: CFragileObject::GetDestructionForce (I can't do this now because you can't inherit both in COldObject ~krzys_h)
        DamageType damageType = DamageType::Collision;
        float destructionForce = pObj->Implements(ObjectInterfaceType::Fragile) ? 50.0f : -1.0f; // Titanium, PowerCell, NuclearCell, default
        if (pObj->GetType() == OBJECT_STONE   ) { destructionForce = 25.0f; } // TitaniumOre
        if (pObj->GetType() == OBJECT_URANIUM ) { destructionForce = 25.0f; } // UraniumOre
        if (pObj->GetType() == OBJECT_MOBILEtg) { destructionForce = 10.0f; damageType = DamageType::Explosive; } // TargetBot (something running into it)
        if (iType           == OBJECT_MOBILEtg) { destructionForce =  0.0f; damageType = DamageType::Explosive; } // TargetBot (it running into something)
        if (pObj->GetType() == OBJECT_TNT     ) { destructionForce = 10.0f; damageType = DamageType::Explosive; } // TNT
        if (pObj->GetType() == OBJECT_BOMB    ) { destructionForce =  0.0f; damageType = DamageType::Explosive; } // Mine

        if ( force > destructionForce && destructionForce >= 0.0f )
        {
            // TODO: implement "killer"?
            dynamic_cast<CDamageableObject&>(*pObj).DamageObject(damageType);
        }

        float damage = -1.0f;
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
            damage = force/400.0f;
        }
        if (oType == OBJECT_MOBILEwa ||
            oType == OBJECT_MOBILEta ||
            oType == OBJECT_MOBILEfa ||
            oType == OBJECT_MOBILEia ||
            oType == OBJECT_MOBILEwb ||
            oType == OBJECT_MOBILEtb ||
            oType == OBJECT_MOBILEfb ||
            oType == OBJECT_MOBILEib ||
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
            oType == OBJECT_MOBILErp ||
            oType == OBJECT_MOBILEst  )  // vehicle?
        {
            damage = force/200.0f;
        }
        if ( force > 25.0f && damage > 0.0f )
        {
            // TODO: implement "killer"?
            dynamic_cast<CDamageableObject&>(*pObj).DamageObject(DamageType::Collision, damage);
        }
    }

    if((oType == OBJECT_PLANT0  ||
        oType == OBJECT_PLANT1  ||
        oType == OBJECT_PLANT2  ||
        oType == OBJECT_PLANT3  ||
        oType == OBJECT_PLANT4  ||
        oType == OBJECT_PLANT15 ||
        oType == OBJECT_PLANT16 ||
        oType == OBJECT_PLANT17 ||
        oType == OBJECT_PLANT18)&&
        GetDriveFromObject(iType)==DriveType::Heavy)
    {
        dynamic_cast<CDestroyableObject*>(pObj)->DestroyObject(DestructionType::Squash);
    }

    return false;
}

// Effects of the explosion on the object itself.
// Returns 0 -> mobile object
// Returns 1 -> immobile object
// Returns 2 -> object destroyed

int CPhysics::ExploHimself(ObjectType iType, ObjectType oType, float force)
{
    if (!m_object->Implements(ObjectInterfaceType::Damageable)) return 1;

    // TODO: CExplosiveObject? derrives from CFragileObject
    float destructionForce = -1.0f; // minimal force required to destroy an object using this explosive, default: not explosive
    if ( oType == OBJECT_TNT      ) destructionForce = 10.0f; // TNT
    if ( oType == OBJECT_MOBILEtg ) destructionForce = 10.0f; // TargetBot (something running into it)
    if ( iType == OBJECT_MOBILEtg ) destructionForce =  0.0f; // TargetBot (it running into something)
    if ( oType == OBJECT_BOMB     ) destructionForce =  0.0f; // Mine

    if ( force > destructionForce && destructionForce >= 0.0f )
    {
        // TODO: implement "killer"?
        dynamic_cast<CDamageableObject&>(*m_object).DamageObject(DamageType::Explosive);
        return 2;
    }

    if ( force > 25.0f )
    {
        if ( iType == OBJECT_HUMAN    ||
             iType == OBJECT_MOBILEwa ||
             iType == OBJECT_MOBILEta ||
             iType == OBJECT_MOBILEfa ||
             iType == OBJECT_MOBILEia ||
             iType == OBJECT_MOBILEwb ||
             iType == OBJECT_MOBILEtb ||
             iType == OBJECT_MOBILEfb ||
             iType == OBJECT_MOBILEib ||
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
             iType == OBJECT_MOBILErp ||
             iType == OBJECT_MOBILEst ||
             iType == OBJECT_MOBILEdr ||
             iType == OBJECT_APOLLO2  )  // vehicle?
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
            if ( dynamic_cast<CDamageableObject&>(*m_object).DamageObject(DamageType::Collision, force) )  return 2;
        }
    }

    return 1;
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
    glm::vec3    pos{ 0, 0, 0 }, ppos{ 0, 0, 0 }, eye{ 0, 0, 0 }, speed{ 0, 0, 0 };
    glm::vec2       dim;
    bool        bCarryPower;

    // TODO: it should be all slots that contain a power cell that can get recharged. Not just the carrying slot.
    bCarryPower = false;
    if (m_object->Implements(ObjectInterfaceType::Slotted))
    {
        CObject* cargo = dynamic_cast<CSlottedObject&>(*m_object).GetSlotContainedObjectOpt(CSlottedObject::Pseudoslot::CARRYING);
        if ( cargo != nullptr && cargo->Implements(ObjectInterfaceType::PowerContainer) &&
            dynamic_cast<CPowerContainerObject&>(*cargo).IsRechargeable() &&
            m_object->GetPartRotationZ(1) == ARM_STOCK_ANGLE1 )
        {
            bCarryPower = true;  // carries a battery
        }
    }

    glm::mat4 mat = m_object->GetWorldMatrix(0);

    pos = m_object->GetSlotPosition(m_object->MapPseudoSlot(CSlottedObject::Pseudoslot::POWER));
    pos.x -= 0.3f;
    pos.y += 1.0f;  // battery center position
    pos = Math::Transform(mat, pos);

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
        pos = glm::vec3(3.0f, 5.6f, 0.0f);  // position of battery holder // TODO: Move to CTransportableObject
        pos = Math::Transform(mat, pos);

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

void CPhysics::CrashParticle(float crash)
{
    glm::vec3    pos{ 0, 0, 0 }, ppos{ 0, 0, 0 }, speed{ 0, 0, 0 };
    glm::vec2       dim;
    float           len;
    int             i, max;

    if ( crash < 0.2f )  return;

    pos = m_object->GetPosition();
    m_camera->StartEffect(Gfx::CAM_EFFECT_CRASH, pos, crash);

//? max = (int)(crash*50.0f);
    max = static_cast<int>(crash*10.0f*m_engine->GetParticleDensity());

    for ( i=0 ; i<max ; i++ )
    {
        ppos.x = pos.x + (Math::Rand()-0.5f)*15.0f*crash;
        ppos.z = pos.z + (Math::Rand()-0.5f)*15.0f*crash;
        ppos.y = pos.y + Math::Rand()*4.0f;
        len = 1.0f-(glm::distance(ppos, pos)/(15.0f+5.0f));
        if ( len <= 0.0f )  continue;
        speed.x = (ppos.x-pos.x)*0.1f;
        speed.z = (ppos.z-pos.z)*0.1f;
        speed.y = -2.0f;
        dim.x = 2.0f+crash*5.0f*len;
        dim.y = dim.x;
        m_particle->CreateParticle(ppos, speed, dim, Gfx::PARTICRASH, 2.0f);
    }
}

// Generates some exhaust gas particle.

void CPhysics::MotorParticle(float aTime, float rTime)
{
    glm::vec3    pos{ 0, 0, 0 }, speed{ 0, 0, 0 };
    glm::vec2       dim;
    ObjectType  type;
    glm::vec2   p;
    float       h, a, delay, level;
    int         r, i, nb;

    if ( m_object->GetToy() )  return;

    type = m_object->GetType();

    if ( type == OBJECT_MOBILEia ||
         type == OBJECT_MOBILEib ||
         type == OBJECT_MOBILEic ||
         type == OBJECT_MOBILEii ||
         type == OBJECT_MOBILEis ||
         type == OBJECT_MOBILEit ||  // legs?
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

                glm::mat4 mat = m_object->GetWorldMatrix(0);
                pos.x = (Math::Rand()-0.5f)*1.0f;
                pos.y = -m_object->GetCharacter()->height;
                pos.z = Math::Rand()*0.4f+1.0f;
                if ( rand()%2 == 0 )  pos.z = -pos.z;
                pos = Math::Transform(mat, pos);
                speed = glm::vec3(0.0f, 1.0f, 0.0f);
                dim.x = Math::Rand()*(h-5.0f)/2.0f+1.0f;
                if ( dim.x > 2.5f )  dim.x = 2.5f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTICRASH, 2.0f, 0.0f, 0.2f);
            }
        }
    }

    if ( type == OBJECT_MOBILEta ||
         type == OBJECT_MOBILEtb ||
         type == OBJECT_MOBILEtc ||
         type == OBJECT_MOBILEti ||
         type == OBJECT_MOBILEts ||
         type == OBJECT_MOBILEtt )  // caterpillars?
    {
        if ( aTime-m_lastSlideParticle >= m_engine->ParticleAdapt(0.05f) )
        {
            h = fabs(m_linMotion.motorSpeed.x-m_linMotion.realSpeed.x);
            if ( h > 5.0f )
            {
                m_lastSlideParticle = aTime;

                glm::mat4 mat = m_object->GetWorldMatrix(0);
                pos.x = (Math::Rand()-0.5f)*8.0f;
                pos.y = 0.0f;
                pos.z = Math::Rand()*2.0f+3.0f;
                if ( rand()%2 == 0 )  pos.z = -pos.z;
                pos = Math::Transform(mat, pos);
                speed = glm::vec3(0.0f, 0.0f, 0.0f);
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
         type == OBJECT_MOBILErs ||
         type == OBJECT_MOBILErp )  // large caterpillars?
    {
        if ( aTime-m_lastSlideParticle >= m_engine->ParticleAdapt(0.05f) )
        {
            h = fabs(m_linMotion.motorSpeed.x-m_linMotion.realSpeed.x);
            if ( h > 5.0f )
            {
                m_lastSlideParticle = aTime;

                glm::mat4 mat = m_object->GetWorldMatrix(0);
                pos.x = (Math::Rand()-0.5f)*9.0f;
                pos.y = 0.0f;
                pos.z = Math::Rand()*3.0f+3.0f;
                if ( rand()%2 == 0 )  pos.z = -pos.z;
                pos = Math::Transform(mat, pos);
                speed = glm::vec3(0.0f, 0.0f, 0.0f);
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

            pos = glm::vec3(-1.6f, -0.5f, 0.0f);
            glm::mat4 mat = m_object->GetWorldMatrix(0);
            pos = Math::Transform(mat, pos);

            speed.x = (Math::Rand()-0.5f)*0.6f;
            speed.z = (Math::Rand()-0.5f)*0.6f;
            speed.y = -(0.5f+Math::Rand()*0.3f)*(1.0f-m_reactorTemperature);

            dim.x = (1.0f+Math::Rand()*0.5f)*(0.2f+m_reactorTemperature*0.8f);
            dim.y = dim.x;

            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISMOKE2, 3.0f, 0.0f, 0.1f);
        }
        else    // in flight?
        {
            if ( !m_bMotor || (m_object->Implements(ObjectInterfaceType::JetFlying) && dynamic_cast<CJetFlyingObject&>(*m_object).GetReactorRange() == 0.0f) )  return;

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

            pos = glm::vec3(-1.6f, -1.0f, 0.0f);
            pos.x += (Math::Rand()-0.5f)*3.0f;
            pos.y += (Math::Rand()-0.5f)*1.5f;
            pos.z += (Math::Rand()-0.5f)*3.0f;
            glm::mat4 mat = m_object->GetWorldMatrix(0);
            pos = Math::Transform(mat, pos);

            h = m_floorHeight;
            if ( h > 10.0f )  // high enough?
            {
                speed = glm::vec3(0.0f, -10.0f, 0.0f);  // against the bottom
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

            pos = glm::vec3(-1.6f, -0.5f, 0.0f);
            pos = Math::Transform(mat, pos);

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
                if ( r == 0 )  pos = glm::vec3(-3.0f, 0.0f, -4.0f);
                if ( r == 1 )  pos = glm::vec3(-3.0f, 0.0f,  4.0f);
                if ( r == 2 )  pos = glm::vec3( 4.0f, 0.0f,  0.0f);

                pos.x += (Math::Rand()-0.5f)*2.0f;
                pos.z += (Math::Rand()-0.5f)*2.0f;
                glm::mat4 mat = m_object->GetWorldMatrix(0);
                pos = Math::Transform(mat, pos);
                speed = glm::vec3(0.0f, 0.0f, 0.0f);
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
                if ( r == 0 )  pos = glm::vec3(-3.0f, 0.0f, -4.0f);
                if ( r == 1 )  pos = glm::vec3(-3.0f, 0.0f,  4.0f);
                if ( r == 2 )  pos = glm::vec3( 4.0f, 0.0f,  0.0f);

                pos.x += (Math::Rand()-0.5f)*1.0f;
                pos.z += (Math::Rand()-0.5f)*1.0f;
                glm::mat4 mat = m_object->GetWorldMatrix(0);
                pos = Math::Transform(mat, pos);
                speed = glm::vec3(0.0f, 0.0f, 0.0f);
                dim.x = 1.0f;
                dim.y = dim.x;
                m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIEJECT);
            }
        }
        else    // in flight?
        {
            if ( !m_bMotor || (m_object->Implements(ObjectInterfaceType::JetFlying) && dynamic_cast<CJetFlyingObject&>(*m_object).GetReactorRange() == 0.0f) )  return;

            if ( aTime-m_lastMotorParticle < m_engine->ParticleAdapt(0.02f) )  return;
            m_lastMotorParticle = aTime;

            pos = glm::vec3(0.0f, -1.0f, 0.0f);
            pos.x += (Math::Rand()-0.5f)*6.0f;
            pos.y += (Math::Rand()-0.5f)*3.0f;
            pos.z += (Math::Rand()-0.5f)*6.0f;
            glm::mat4 mat = m_object->GetWorldMatrix(0);
            pos = Math::Transform(mat, pos);

            h = m_floorHeight;
            if ( h > 10.0f )  // high enough?
            {
                speed = glm::vec3(0.0f, -10.0f, 0.0f);  // against the bottom
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

            pos = glm::vec3(0.0f, 1.0f, 0.0f);
            pos = Math::Transform(mat, pos);

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
        if ( !m_object->Implements(ObjectInterfaceType::Destroyable) || dynamic_cast<CDestroyableObject&>(*m_object).GetDying() != DeathType::Dead )
        {
            h = Math::Mod(aTime, 5.0f);
            if ( h < 3.5f && ( h < 1.5f || h > 1.6f ) )  return;
        }
        if ( aTime-m_lastMotorParticle < m_engine->ParticleAdapt(0.06f) )  return;
        m_lastMotorParticle = aTime;

        pos = glm::vec3(0.0f, 3.0f, 0.0f);
        glm::mat4 mat = m_object->GetWorldMatrix(0);
        pos = Math::Transform(mat, pos);
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

    if ( (type == OBJECT_MOBILEsa || type == OBJECT_MOBILEst) && m_bSwim )
    {
        h = Math::Mod(aTime, 3.0f);
        if ( h < 1.5f && ( h < 0.5f || h > 0.9f ) )  return;
        if ( aTime-m_lastMotorParticle < m_engine->ParticleAdapt(0.06f) )  return;
        m_lastMotorParticle = aTime;

        pos = glm::vec3(0.0f, 3.0f, 0.0f);
        glm::mat4 mat = m_object->GetWorldMatrix(0);
        pos = Math::Transform(mat, pos);
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
             type == OBJECT_MOBILErp )
        {
            if ( !m_bMotor )  return;

            if ( aTime-m_lastMotorParticle < m_engine->ParticleAdapt(0.1f) )  return;
            m_lastMotorParticle = aTime;

            pos = glm::vec3(-2.5f, 10.3f, -1.3f);
            pos.x += (Math::Rand()-0.5f)*1.0f;
            pos.z += (Math::Rand()-0.5f)*1.0f;
            glm::mat4 mat = m_object->GetWorldMatrix(0);
            pos   = Math::Transform(mat, pos);

            speed.x = (Math::Rand()-0.5f)*2.0f;
            speed.z = (Math::Rand()-0.5f)*2.0f;
            speed.y = 1.5f+Math::Rand()*1.0f;

            dim.x = Math::Rand()*0.6f+0.4f;
            dim.y = dim.x;

            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIMOTOR, 2.0f);
        }
        else
        {
            if ( !m_bMotor )  return;

            if ( aTime-m_lastMotorParticle < m_engine->ParticleAdapt(0.05f) )  return;
            m_lastMotorParticle = aTime;

            pos = glm::vec3(-3.4f, 1.8f, 0.5f);

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

            glm::mat4 mat = m_object->GetWorldMatrix(0);
            pos   = Math::Transform(mat, pos);
            speed = Math::Transform(mat, speed)-pos;

            dim.x = Math::Rand()*0.4f+0.3f;
            dim.y = dim.x;

            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIMOTOR, 2.0f);
        }
    }
}

// Generates some particles after falling into the water.

void CPhysics::WaterParticle(float aTime, glm::vec3 pos, ObjectType type,
                              float floor, float advance, float turn)
{
    glm::vec3    ppos{ 0, 0, 0 }, speed{ 0, 0, 0 };
    glm::vec2       dim;
    float       delay, level, min, max, force, volume, diam;
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

    // Management of the particle "splash".
    if ( m_linMotion.realSpeed.y < -10.0f   &&
         aTime-m_lastPloufParticle >= 1.0f )
    {
        m_lastPloufParticle = aTime;

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

        pos = m_object->GetPosition();
        pos.y = m_water->GetLevel()-1.0f;
        dim.x = 2.0f*force;  // height
        dim.y = diam;  // diameter
        m_particle->CreateParticle(pos, glm::vec3(0.0f, 0.0f, 0.0f), dim, Gfx::PARTIPLOUF0, 1.4f, 0.0f, 0.0f);

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

        volume = fabs(m_linMotion.realSpeed.y*0.02f);
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

void CPhysics::WheelParticle(TraceColor color, float width)
{
    glm::vec3        goal1{ 0, 0, 0 }, goal2{ 0, 0, 0 }, wheel1{ 0, 0, 0 }, wheel2{ 0, 0, 0 };
    float               dist1, dist2, step;

    glm::mat4 mat = m_object->GetWorldMatrix(0);

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
        goal1 = Math::Transform(mat, goal1);

        goal2.x = step/2.0f;
        goal2.y = 0.0f;
        goal2.z = width/2.0f;
        goal2 = Math::Transform(mat, goal2);

        if ( !m_bWheelParticleBrake )
        {
            m_wheelParticlePos[0] = goal1;
            m_wheelParticlePos[1] = goal2;
        }

        while ( true )
        {
            dist1 = glm::distance(m_wheelParticlePos[0], goal1);
            if ( dist1 < step )  break;

            dist2 = glm::distance(m_wheelParticlePos[1], goal2);

            wheel1 = Math::SegmentPoint(m_wheelParticlePos[0], goal1, step);
            wheel2 = Math::SegmentPoint(m_wheelParticlePos[1], goal2, step * dist2 / dist1);

            if ( m_linMotion.realSpeed.x >= 0.0f )
            {
                m_particle->CreateWheelTrace(m_wheelParticlePos[0], m_wheelParticlePos[1], wheel1, wheel2, color);
            }
            else
            {
                m_particle->CreateWheelTrace(m_wheelParticlePos[1], m_wheelParticlePos[0], wheel2, wheel1, color);
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
        if ( dynamic_cast<CProgramStorageObject&>(*m_object).GetActiveVirus() )
        {
            return ERR_VEH_VIRUS;
        }
    }

    if (HasPowerCellSlot(m_object))
    {
        CPowerContainerObject* power = GetObjectPowerCell(m_object);  // searches for the object battery used
        if (power == nullptr)
        {
            return ERR_VEH_POWER;
        }
        else
        {
            if ( power->GetEnergy() == 0.0f )  return ERR_VEH_ENERGY;
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
