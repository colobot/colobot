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

#include "object/task/taskflag.h"

#include "graphics/engine/particle.h"
#include "graphics/engine/pyro_manager.h"
#include "graphics/engine/water.h"

#include "math/geometry.h"

#include "object/object_manager.h"
#include "object/old_object.h"

#include "object/interface/carrier_object.h"

#include "object/motion/motionhuman.h"

#include "physics/physics.h"

#include "sound/sound.h"



// Object's constructor.

CTaskFlag::CTaskFlag(COldObject* object) : CForegroundTask(object)
{
}

// Object's destructor.

CTaskFlag::~CTaskFlag()
{
}


// Management of an event.

bool CTaskFlag::EventProcess(const Event &event)
{
    if ( m_bError )  return true;
    if ( m_engine->GetPause() )  return true;
    if ( event.type != EVENT_FRAME )  return true;

    m_time += event.rTime;

    return true;
}



// Assigns the goal was achieved.

Error CTaskFlag::Start(TaskFlagOrder order, int rank)
{
    Math::Vector    pos, speed;
    Error       err;

    m_order = order;
    m_time = 0.0f;

    m_bError = true;  // operation impossible
    if ( !m_physics->GetLand() )
    {
        pos = m_object->GetPosition();
        if ( pos.y < m_water->GetLevel() )  return ERR_FLAG_WATER;
        return ERR_FLAG_FLY;
    }

    speed = m_physics->GetMotorSpeed();
    if ( speed.x != 0.0f ||
         speed.z != 0.0f )  return ERR_FLAG_MOTOR;

    if (IsObjectCarryingCargo(m_object))  return ERR_FLAG_BUSY;

    if ( order == TFL_CREATE )
    {
        err = CreateFlag(rank);
        if ( err != ERR_OK )  return err;
    }

    if ( order == TFL_DELETE )
    {
        err = DeleteFlag();
        if ( err != ERR_OK )  return err;
    }

    m_bError = false;

    m_motion->SetAction(MHS_FLAG);  // sets/removes flag
    m_camera->StartCentering(m_object, Math::PI*0.3f, 99.9f, 0.0f, 0.5f);

    return ERR_OK;
}

// Indicates whether the action is finished.

Error CTaskFlag::IsEnded()
{
    if ( m_engine->GetPause() )  return ERR_CONTINUE;

    if ( m_bError )  return ERR_STOP;
    if ( m_time < 2.0f )  return ERR_CONTINUE;

    Abort();
    return ERR_STOP;
}

// Suddenly ends the current action.

bool CTaskFlag::Abort()
{
    m_motion->SetAction(-1);
    m_camera->StopCentering(m_object, 2.0f);
    return true;
}



// Returns the closest object to a given position.

CObject* CTaskFlag::SearchNearest(Math::Vector pos, ObjectType type)
{
    std::vector<ObjectType> types;
    if(type == OBJECT_NULL)
    {
        types = {OBJECT_FLAGb, OBJECT_FLAGr, OBJECT_FLAGg, OBJECT_FLAGy, OBJECT_FLAGv};
    }
    else
    {
        types = {type};
    }
    return CObjectManager::GetInstancePointer()->FindNearest(nullptr, pos, types);
}

// Counts the number of existing objects.

int CTaskFlag::CountObject(ObjectType type)
{
    int count = 0;
    for (CObject* obj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        ObjectType  oType = obj->GetType();
        if ( type == OBJECT_NULL )
        {
            if ( oType != OBJECT_FLAGb &&
                 oType != OBJECT_FLAGr &&
                 oType != OBJECT_FLAGg &&
                 oType != OBJECT_FLAGy &&
                 oType != OBJECT_FLAGv )  continue;
        }
        else
        {
            if ( oType != type )  continue;
        }

        count ++;
    }
    return count;
}

// Creates a color indicator.

Error CTaskFlag::CreateFlag(int rank)
{
    ObjectType table[5] =
    {
        OBJECT_FLAGb,
        OBJECT_FLAGr,
        OBJECT_FLAGg,
        OBJECT_FLAGy,
        OBJECT_FLAGv,
    };

    Math::Matrix* mat = m_object->GetWorldMatrix(0);
    Math::Vector pos = Transform(*mat, Math::Vector(4.0f, 0.0f, 0.0f));

    CObject* pObj = SearchNearest(pos, OBJECT_NULL);
    if ( pObj != nullptr )
    {
        float dist = Math::Distance(pos, pObj->GetPosition());
        if ( dist < 10.0f )
        {
            return ERR_FLAG_PROXY;
        }
    }

    ObjectType type = table[rank];
    if ( CountObject(type) >= 5 )
    {
        return ERR_FLAG_CREATE;
    }

    float angle = 0.0f;
    CObject* pNew = CObjectManager::GetInstancePointer()->CreateObject(pos, angle, type);
    //pNew->SetScale(0.0f);

    m_sound->Play(SOUND_WAYPOINT, pos);
    m_engine->GetPyroManager()->Create(Gfx::PT_FLCREATE, pNew);

    return ERR_OK;
}

// Deletes a color indicator.

Error CTaskFlag::DeleteFlag()
{
    CObject*     pObj;
    Math::Vector iPos, oPos;
    float        iAngle, angle, aLimit, dist;

    iPos = m_object->GetPosition();
    iAngle = m_object->GetRotationY();
    iAngle = Math::NormAngle(iAngle);  // 0..2*Math::PI

    pObj = SearchNearest(iPos, OBJECT_NULL);
    if ( pObj == nullptr )
    {
        return ERR_FLAG_DELETE;
    }
    dist = Math::Distance(iPos, pObj->GetPosition());
    if ( dist > 10.0f )
    {
        return ERR_FLAG_DELETE;
    }

    oPos = pObj->GetPosition();
    angle = Math::RotateAngle(oPos.x-iPos.x, iPos.z-oPos.z);  // CW !
    aLimit = 45.0f*Math::PI/180.0f;
    if ( !Math::TestAngle(angle, iAngle-aLimit, iAngle+aLimit) )
    {
        return ERR_FLAG_DELETE;
    }

    m_sound->Play(SOUND_WAYPOINT, iPos);

    m_engine->GetPyroManager()->Create(Gfx::PT_FLDELETE, pObj);

    return ERR_OK;
}
