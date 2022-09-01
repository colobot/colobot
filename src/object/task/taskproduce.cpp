/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2021, Daniel Roux, EPSITEC SA & TerranovaTeam
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


#include "object/task/taskproduce.h"

#include "object/object_manager.h"

#include "graphics/engine/engine.h"

#include "math/geometry.h"

#include "physics/physics.h"

// Object's constructor.

CTaskProduce::CTaskProduce(COldObject* object) : CForegroundTask(object)
{
}

// Object's destructor.

CTaskProduce::~CTaskProduce()
{
}


// Management of an event.

bool CTaskProduce::EventProcess(const Event &event)
{
    if ( m_engine->GetPause() )  return true;
    if ( event.type != EVENT_FRAME )  return true;
    if ( m_bEnded )  return true;

    m_passTime += event.rTime;

    if (m_passTime >= m_waitTime)
    {
        m_physics->SetMotorSpeedX(0.0f);
        m_physics->SetMotorSpeedZ(0.0f);
        m_bEnded = true;
    }

    return true;
}


// Assigns the goal was achieved.

Error CTaskProduce::Start(ObjectType type)
{
    Math::Vector speed = m_physics->GetMotorSpeed();
    if ( speed.x != 0.0f || speed.z != 0.0f )
    {
        m_bEnded = true;
        return ERR_BUILD_MOTOR;
    }

    if ( !IsFreeDeposeObject(Math::Vector(0.0f, 0.0f, 0.0f)) )
    {
        m_bEnded = true;
        return ERR_MANIP_OCC;
    }

    ObjectCreateParams params;
    params.pos = m_object->GetPosition();
    params.angle = 0.0f;
    params.team = m_object->GetTeam();
    params.type = type;

    CObject* object = CObjectManager::GetInstancePointer()->Produce(params);
    if (object == nullptr)
    {
        m_bEnded = true;
        return ERR_UNKNOWN;
    }

    bool isOldObjectSelectable = m_object->GetSelectable();
    assert(object->Implements(ObjectInterfaceType::Old));
    dynamic_cast<COldObject&>(*object).SetSelectable(isOldObjectSelectable);

    m_physics->SetMotorSpeedX(1.0f);
    m_physics->SetMotorSpeedZ(0.0f);
    m_waitTime = 2.5f;  // duration to wait for Alien Queen to move away
    m_passTime = 0.0f;  // time elapsed
    m_bEnded = false;

    return ERR_OK;
}

// Indicates whether the action is finished.

Error CTaskProduce::IsEnded()
{
    if ( m_engine->GetPause() )  return ERR_CONTINUE;
    if ( m_bEnded )  return ERR_STOP;
    return ERR_CONTINUE;
}

// Seeks if a location allows to deposit an object.

bool CTaskProduce::IsFreeDeposeObject(Math::Vector pos)
{
    Math::Matrix* mat = m_object->GetWorldMatrix(0);
    Math::Vector iPos = Transform(*mat, pos);

    for (CObject* obj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        if ( obj == m_object )  continue;
        if ( !obj->GetDetectable() )  continue;  // inactive?
        if (IsObjectBeingTransported(obj))  continue;

        for (const auto& crashSphere : obj->GetAllCrashSpheres())
        {
            if ( Math::Distance(m_object->GetPosition(), crashSphere.sphere.pos)-(crashSphere.sphere.radius+1.0f) < 6.0f )
            {
                return false;  // location occupied
            }
        }
    }
    return true;  // location free
}