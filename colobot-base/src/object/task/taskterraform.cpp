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

#include "object/task/taskterraform.h"

#include "graphics/engine/engine.h"
#include "graphics/engine/particle.h"
#include "graphics/engine/pyro_manager.h"
#include "graphics/engine/terrain.h"

#include "level/robotmain.h"

#include "math/geometry.h"

#include "object/object_manager.h"
#include "object/old_object.h"

#include "object/interface/slotted_object.h"

#include "object/motion/motionant.h"
#include "object/motion/motionspider.h"

#include "object/subclass/base_alien.h"

#include "physics/physics.h"

#include "sound/sound.h"


const float ENERGY_TERRA    = 0.40f;        // energy consumed by blow
const float ACTION_RADIUS   = 400.0f;



// Object's constructor.

CTaskTerraform::CTaskTerraform(COldObject* object) : CForegroundTask(object)
{
    m_lastParticle = 0.0f;
    m_soundChannel = -1;

    assert(m_object->GetNumSlots() == 1);
}

// Object's destructor.

CTaskTerraform::~CTaskTerraform()
{
}


// Management of an event.

bool CTaskTerraform::EventProcess(const Event &event)
{
    CObject*    power;
    glm::vec3    pos, dir, speed;
    glm::vec2       dim;
    float       energy;

    if ( m_engine->GetPause() )  return true;
    if ( event.type != EVENT_FRAME )  return true;
    if ( m_error != ERR_OK )  return false;

    m_progress += event.rTime*m_speed;  // others advance
    m_time += event.rTime;

    if ( m_phase == TTP_CHARGE )
    {
        if ( m_soundChannel == -1 )
        {
            m_soundChannel = m_sound->Play(SOUND_GGG, m_object->GetPosition(), 1.0f, 0.5f, true);
            m_sound->AddEnvelope(m_soundChannel, 1.0f, 2.0f, 4.0f, SOPER_CONTINUE);
            m_sound->AddEnvelope(m_soundChannel, 0.0f, 0.5f, 0.5f, SOPER_STOP);
        }

        dir.x = 0.0f;
        dir.y = (Math::Rand()-0.5f)*0.2f*m_progress;
        dir.z = 0.0f;
        m_object->SetCirVibration(dir);

        m_object->SetScale(1.0f+m_progress*0.2f);

        power = m_object->GetSlotContainedObjectReq(CSlottedObject::Pseudoslot::POWER);
        if (power == nullptr)
        {
            m_error = ERR_TERRA_ENERGY;
            Abort();
            return false;
        }
        else
        {
            power->SetScale(1.0f+m_progress*1.0f);

            if (power->Implements(ObjectInterfaceType::PowerContainer))
            {
                CPowerContainerObject* powerContainer = dynamic_cast<CPowerContainerObject*>(power);
                energy = powerContainer->GetEnergy();
                energy -= event.rTime*ENERGY_TERRA/4.0f;
                if ( energy < 0.0f )  energy = 0.0f;
                powerContainer->SetEnergy(energy);
            }
        }
    }

    if ( m_phase == TTP_DOWN )
    {
        pos.x = 9.0f;
        pos.y = 4.0f-m_progress*5.8f;
        pos.z = 0.0f;
        m_object->SetPartPosition(2, pos);
    }

    if ( m_phase == TTP_UP )
    {
        pos.x = 9.0f;
        pos.y = 4.0f-(1.0f-m_progress)*5.8f;
        pos.z = 0.0f;
        m_object->SetPartPosition(2, pos);
    }

    dir.x = 0.0f;
    dir.y = 0.0f;
    dir.z = 0.0f;
    pos = m_object->GetPartPosition(2);
    if ( pos.y < 0.0f )
    {
        dir.z = -atanf((pos.y/2.0f)/9.0f);
    }
    m_object->SetTilt(dir);

    if ( m_time-m_lastParticle >= m_engine->ParticleAdapt(0.05f) )
    {
        m_lastParticle = m_time;

        glm::mat4 mat = m_object->GetWorldMatrix(0);

        if ( m_phase == TTP_CHARGE )
        {
            // Battery.
            pos = glm::vec3(-6.0f, 5.5f+2.0f*m_progress, 0.0f);
            pos.x += (Math::Rand()-0.5f)*1.0f;
            pos.z += (Math::Rand()-0.5f)*1.0f;
            pos   = Math::Transform(mat, pos);
            speed.x = (Math::Rand()-0.5f)*6.0f*(1.0f+m_progress*4.0f);
            speed.z = (Math::Rand()-0.5f)*6.0f*(1.0f+m_progress*4.0f);
            speed.y = 6.0f+Math::Rand()*4.0f*(1.0f+m_progress*2.0f);
            dim.x = 0.5f+1.5f*m_progress;
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTIBLITZ, 2.0f, 20.0f);
        }

        if ( m_phase != TTP_CHARGE )
        {
            // Left grid.
            pos = glm::vec3(-1.0f, 5.8f, 3.5f);
            pos.x += (Math::Rand()-0.5f)*1.0f;
            pos.z += (Math::Rand()-0.5f)*1.0f;
            pos   = Math::Transform(mat, pos);
            speed.x = Math::Rand()*4.0f;
            speed.z = Math::Rand()*2.0f;
            speed.y = 2.5f+Math::Rand()*1.0f;
            speed = Math::Transform(mat, speed);
            speed -= m_object->GetPosition();
            dim.x = Math::Rand()*1.0f+1.0f;
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISMOKE1, 3.0f);

            // Right grid.
            pos = glm::vec3(-1.0f, 5.8f, -3.5f);
            pos.x += (Math::Rand()-0.5f)*1.0f;
            pos.z += (Math::Rand()-0.5f)*1.0f;
            pos   = Math::Transform(mat, pos);
            speed.x =  Math::Rand()*4.0f;
            speed.z = -Math::Rand()*2.0f;
            speed.y = 2.5f+Math::Rand()*1.0f;
            speed = Math::Transform(mat, speed);
            speed -= m_object->GetPosition();
            dim.x = Math::Rand()*1.0f+1.0f;
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTISMOKE1, 3.0f);
        }
    }

    return true;
}


// Assigns the goal was achieved.

Error CTaskTerraform::Start()
{
    CObject*    power;
    glm::vec3    pos, speed;
    float       energy;

    ObjectType  type;

    m_error = ERR_STOP;  // operation impossible
    if ( !m_physics->GetLand() )  return ERR_WRONG_BOT;

    type = m_object->GetType();
    if ( type != OBJECT_MOBILErt )  return ERR_WRONG_BOT;

    power = m_object->GetSlotContainedObjectReq(CSlottedObject::Pseudoslot::POWER);
    if ( power == nullptr || !power->Implements(ObjectInterfaceType::PowerContainer) )  return ERR_TERRA_ENERGY;
    energy = dynamic_cast<CPowerContainerObject&>(*power).GetEnergy();
    if ( energy < ENERGY_TERRA+0.05f )  return ERR_TERRA_ENERGY;

    speed = m_physics->GetMotorSpeed();
    if ( speed.x != 0.0f ||
         speed.z != 0.0f )  return ERR_MANIP_MOTOR;

    glm::mat4 mat = m_object->GetWorldMatrix(0);
    pos = glm::vec3(9.0f, 0.0f, 0.0f);
    pos = Math::Transform(mat, pos);  // battery position
    m_terraPos = pos;

    m_phase    = TTP_CHARGE;
    m_progress = 0.0f;
    m_speed    = 1.0f/4.0f;
    m_time     = 0.0f;

    m_error = ERR_OK;

    m_camera->StartCentering(m_object, Math::PI*0.35f, 99.9f, 20.0f, 2.0f);
    return ERR_OK;
}

// Indicates whether the action is finished.

Error CTaskTerraform::IsEnded()
{
    CObject*    power;
    glm::vec3    pos, speed;
    glm::vec2       dim;
    float       dist, duration;
    int         i, max;

    if ( m_engine->GetPause() )  return ERR_CONTINUE;
    if ( m_error != ERR_OK )  return m_error;

    if ( m_progress < 1.0f )  return ERR_CONTINUE;
    m_progress = 0.0f;

    if ( m_phase == TTP_CHARGE )
    {

        m_phase = TTP_DOWN;
        m_speed = 1.0f/0.2f;
        return ERR_CONTINUE;
    }

    if ( m_phase == TTP_DOWN )
    {
        Terraform();  // changes the terrain.

        m_object->SetCirVibration(glm::vec3(0.0f, 0.0f, 0.0f));
        m_object->SetScale(1.0f);

        power = m_object->GetSlotContainedObjectReq(CSlottedObject::Pseudoslot::POWER);
        if (power != nullptr)
        {
            power->SetScale(1.0f);
        }

        max= static_cast<int>(50.0f*m_engine->GetParticleDensity());
        for ( i=0 ; i<max ; i++ )
        {
            pos.x = m_terraPos.x+(Math::Rand()-0.5f)*80.0f;
            pos.z = m_terraPos.z+(Math::Rand()-0.5f)*80.0f;
            pos.y = m_terraPos.y;
            m_terrain->AdjustToFloor(pos);
            dist = glm::distance(pos, m_terraPos);
            speed = glm::vec3(0.0f, 0.0f, 0.0f);
            dim.x = 2.0f+(40.0f-dist)/(1.0f+Math::Rand()*4.0f);
            dim.y = dim.x;
            m_particle->CreateParticle(pos, speed, dim, Gfx::PARTICRASH, 2.0f);

            pos = m_terraPos;
            speed.x = (Math::Rand()-0.5f)*40.0f;
            speed.z = (Math::Rand()-0.5f)*40.0f;
            speed.y = Math::Rand()*15.0f+15.0f;
            dim.x = 0.6f;
            dim.y = dim.x;
            pos.y += dim.y;
            duration = Math::Rand()*3.0f+3.0f;
            m_particle->CreateTrack(pos, speed, dim, Gfx::PARTITRACK5,
                                     duration, Math::Rand()*10.0f+15.0f,
                                     duration*0.2f, 1.0f);
        }

        m_phase = TTP_TERRA;
        m_speed = 1.0f/2.0f;
        return ERR_CONTINUE;
    }

    if ( m_phase == TTP_TERRA )
    {
        m_phase = TTP_UP;
        m_speed = 1.0f/1.0f;
        return ERR_CONTINUE;
    }

    Abort();
    return ERR_STOP;
}

// Suddenly ends the current action.

bool CTaskTerraform::Abort()
{
    CObject*    power;

    if ( m_soundChannel != -1 )
    {
        m_sound->FlushEnvelope(m_soundChannel);
        m_sound->AddEnvelope(m_soundChannel, 0.0f, 0.5f, 0.3f, SOPER_STOP);
        m_soundChannel = -1;
    }

    m_object->SetPartPosition(2, glm::vec3(9.0f, 4.0f, 0.0f));
    m_object->SetTilt(glm::vec3(0.0f, 0.0f, 0.0f));
    m_object->SetCirVibration(glm::vec3(0.0f, 0.0f, 0.0f));
    m_object->SetScale(1.0f);

    power = m_object->GetSlotContainedObjectReq(CSlottedObject::Pseudoslot::POWER);
    if (power != nullptr)
    {
        power->SetScale(1.0f);
    }

    m_camera->StopCentering(m_object, 2.0f);
    return true;
}


// Returns all the close ants and spiders.

bool CTaskTerraform::Terraform()
{
    CMotion*    motion;
    ObjectType  type;
    float       dist;

    m_camera->StartEffect(Gfx::CAM_EFFECT_TERRAFORM, m_terraPos, 1.0f);

    m_sound->Play(SOUND_THUMP, m_terraPos);

    for (CObject* pObj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        type = pObj->GetType();
        if ( type == OBJECT_NULL )  continue;

        if ( type == OBJECT_TEEN0        ||
             type == OBJECT_TEEN1        ||
             type == OBJECT_TEEN2        ||
             type == OBJECT_TEEN4        ||
             type == OBJECT_TEEN5        ||
             type == OBJECT_TEEN34       ||
             type == OBJECT_POWER        ||
             type == OBJECT_ATOMIC       ||
             type == OBJECT_STONE        ||
             type == OBJECT_URANIUM      ||
             type == OBJECT_METAL        ||
             type == OBJECT_BULLET       ||
             type == OBJECT_BBOX         ||
             type == OBJECT_KEYa         ||
             type == OBJECT_KEYb         ||
             type == OBJECT_KEYc         ||
             type == OBJECT_KEYd         ||
             type == OBJECT_TNT          ||
             type == OBJECT_NEST         ||
             type == OBJECT_BOMB         ||
             type == OBJECT_WINFIRE      ||
             type == OBJECT_BAG          ||
             type == OBJECT_PLANT0       ||
             type == OBJECT_PLANT1       ||
             type == OBJECT_PLANT2       ||
             type == OBJECT_PLANT3       ||
             type == OBJECT_PLANT4       ||
             type == OBJECT_PLANT5       ||
             type == OBJECT_PLANT6       ||
             type == OBJECT_PLANT7       ||
             type == OBJECT_PLANT15      ||
             type == OBJECT_PLANT16      ||
             type == OBJECT_PLANT17      ||
             type == OBJECT_PLANT18      ||
             type == OBJECT_PLANT19      ||
             type == OBJECT_EGG          ||
             type == OBJECT_RUINmobilew1 ||
             type == OBJECT_RUINmobilew2 ||
             type == OBJECT_RUINmobilet1 ||
             type == OBJECT_RUINmobilet2 ||
             type == OBJECT_RUINdoor     ||
             type == OBJECT_RUINsupport  ||
             type == OBJECT_RUINradar    ||
             type == OBJECT_BARRIER0     ||
             type == OBJECT_BARRIER1     ||
             type == OBJECT_BARRIER2     ||
             type == OBJECT_BARRIER3     ||
             type == OBJECT_APOLLO4      )  // everything what fits?
        {
            dist = glm::distance(m_terraPos, pObj->GetPosition());

            if ( type == OBJECT_BULLET ||
                 type == OBJECT_NEST   ||
                 type == OBJECT_EGG    ) // Alien Organic?
            {
                if ( dist > 5.0f )  continue;
                m_engine->GetPyroManager()->Create(Gfx::PT_FRAGO, pObj);
            }
            else if ( type == OBJECT_TNT  ||
                      type == OBJECT_BOMB ) // Explosives?
            {
                if ( dist > 5.0f )  continue;
                m_engine->GetPyroManager()->Create(Gfx::PT_EXPLOT, pObj);
                dynamic_cast<CDamageableObject&>(*m_object).DamageObject(DamageType::Explosive, 0.9f);
            }
            else if ( type == OBJECT_PLANT0    ||
                      type == OBJECT_PLANT1    ||
                      type == OBJECT_PLANT2    ||
                      type == OBJECT_PLANT3    ||
                      type == OBJECT_PLANT4    ||
                      type == OBJECT_PLANT5    ||
                      type == OBJECT_PLANT6    ||
                      type == OBJECT_PLANT7    ||
                      type == OBJECT_PLANT15   ||
                      type == OBJECT_PLANT16   ||
                      type == OBJECT_PLANT17   ||
                      type == OBJECT_PLANT18   ||
                      type == OBJECT_PLANT19   ) // Plants?
            {
                if ( dist > 7.5f )  continue;
                m_engine->GetPyroManager()->Create(Gfx::PT_FRAGV, pObj);

            }
            else // Other?
            {
                if ( dist > 5.0f )  continue;
                m_engine->GetPyroManager()->Create(Gfx::PT_FRAGT, pObj);
            }
        }
        else
        {
            if ( !pObj->Implements(ObjectInterfaceType::Movable) )  continue;
            motion = dynamic_cast<CMovableObject&>(*pObj).GetMotion();

            dist = glm::distance(m_terraPos, pObj->GetPosition());
            if ( dist > ACTION_RADIUS )  continue;

            if ( type == OBJECT_ANT || type == OBJECT_SPIDER )
            {
                assert(pObj->Implements(ObjectInterfaceType::TaskExecutor));
                dynamic_cast<CTaskExecutorObject&>(*pObj).StopForegroundTask();

                int actionType = -1;
                if (type == OBJECT_ANT)    actionType = MAS_BACK1;
                if (type == OBJECT_SPIDER) actionType = MSS_BACK1;
                motion->SetAction(actionType, 0.8f+Math::Rand()*0.3f);
                dynamic_cast<CBaseAlien&>(*pObj).SetFixed(true);  // not moving

                if ( dist > 5.0f ) continue;
                m_engine->GetPyroManager()->Create(Gfx::PT_EXPLOO, pObj);
            }
            else if ( type == OBJECT_BEE || type == OBJECT_WORM )
            {
                if ( dist > 5.0f ) continue;
                m_engine->GetPyroManager()->Create(Gfx::PT_EXPLOO, pObj);
            }
        }
    }

    return true;
}
