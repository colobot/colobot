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


#include "object/task/tasktake.h"

#include "graphics/engine/terrain.h"
#include "graphics/engine/water.h"

#include "level/robotmain.h"

#include "math/geometry.h"

#include "object/object_manager.h"
#include "object/old_object.h"

#include "object/interface/slotted_object.h"
#include "object/interface/transportable_object.h"

#include "object/motion/motionhuman.h"

#include "physics/physics.h"

#include "sound/sound.h"




// Object's constructor.

CTaskTake::CTaskTake(COldObject* object) : CForegroundTask(object)
{
    m_arm  = TTA_NEUTRAL;

    assert(m_object->MapPseudoSlot(CSlottedObject::Pseudoslot::CARRYING) >= 0);
}

// Object's destructor.

CTaskTake::~CTaskTake()
{
}


// Management of an event.

bool CTaskTake::EventProcess(const Event &event)
{
    float       a, g, cirSpeed;

    if ( m_engine->GetPause() )  return true;
    if ( event.type != EVENT_FRAME )  return true;
    if ( m_bError )  return false;

    if ( m_bTurn )  // preliminary rotation?
    {
        a = m_object->GetRotationY();
        g = m_angle;
        cirSpeed = Math::Direction(a, g)*2.0f;
        if ( cirSpeed >  1.0f )  cirSpeed =  1.0f;
        if ( cirSpeed < -1.0f )  cirSpeed = -1.0f;

        m_physics->SetMotorSpeedZ(cirSpeed);  // turns left / right
        return true;
    }

    m_progress += event.rTime*m_speed;  // others advance

    m_physics->SetMotorSpeed(Math::Vector(0.0f, 0.0f, 0.0f));  // immobile!

    return true;
}


// Assigns the goal was achieved.

Error CTaskTake::Start()
{
    m_height   = 0.0f;
    m_step     = 0;
    m_progress = 0.0f;

    float iAngle = m_object->GetRotationY();
    iAngle = Math::NormAngle(iAngle);  // 0..2*Math::PI
    float oAngle = iAngle;

    m_bError = true;  // operation impossible
    if ( !m_physics->GetLand() )
    {
        Math::Vector pos = m_object->GetPosition();
        float h = m_water->GetLevel(m_object);
        if ( pos.y < h )  return ERR_MANIP_WATER;  // impossible under water
        return ERR_MANIP_FLY;
    }

    ObjectType type = m_object->GetType();
    if ( type != OBJECT_HUMAN &&
         type != OBJECT_TECH  )  return ERR_WRONG_BOT;

    m_physics->SetMotorSpeed(Math::Vector(0.0f, 0.0f, 0.0f));

    if (m_object->GetSlotContainedObjectOpt(CSlottedObject::Pseudoslot::CARRYING) != nullptr)
        m_order = TTO_DEPOSE;
    else
        m_order = TTO_TAKE;


    if ( m_order == TTO_TAKE )
    {
        Math::Vector pos = m_object->GetPosition();
        float h = m_water->GetLevel(m_object);
        if ( pos.y < h )  return ERR_MANIP_WATER;  // impossible under water

        int otherSlotNum = -1;
        CObject* other = SearchFriendObject(oAngle, 1.5f, Math::PI*0.50f, otherSlotNum);
        CSlottedObject* otherAsSlotted = dynamic_cast<CSlottedObject*>(other);
        assert(other == nullptr || otherSlotNum >= 0);

        if (other != nullptr && otherAsSlotted->GetSlotContainedObject(otherSlotNum) != nullptr)
        {
            CObject* power = otherAsSlotted->GetSlotContainedObject(otherSlotNum);
            type = power->GetType();
            if ( type == OBJECT_URANIUM )  return ERR_MANIP_RADIO;
            assert(power->Implements(ObjectInterfaceType::Transportable));
//?         m_camera->StartCentering(m_object, Math::PI*0.3f, -Math::PI*0.1f, 0.0f, 0.8f);
            m_arm = TTA_FRIEND;
        }
        else
        {
            other = SearchTakeObject(oAngle, 1.5f, Math::PI*0.45f);
            if ( other == nullptr )  return ERR_MANIP_NIL;
            type = other->GetType();
            if ( type == OBJECT_URANIUM )  return ERR_MANIP_RADIO;
//?         m_camera->StartCentering(m_object, Math::PI*0.3f, 99.9f, 0.0f, 0.8f);
            m_arm = TTA_FFRONT;
            m_main->HideDropZone(other);  // hides buildable area
        }
    }

    if ( m_order == TTO_DEPOSE )
    {
//?     speed = m_physics->GetMotorSpeed();
//?     if ( speed.x != 0.0f ||
//?          speed.z != 0.0f )  return ERR_MANIP_MOTOR;

        int otherSlotNum = -1;
        CObject* other = SearchFriendObject(oAngle, 1.5f, Math::PI*0.50f, otherSlotNum);
        CSlottedObject* otherAsSlotted = dynamic_cast<CSlottedObject*>(other);
        assert(other == nullptr || otherSlotNum >= 0);

        if (other != nullptr && otherAsSlotted->GetSlotContainedObject(otherSlotNum) == nullptr)
        {
//?         m_camera->StartCentering(m_object, Math::PI*0.3f, -Math::PI*0.1f, 0.0f, 0.8f);
            m_arm = TTA_FRIEND;
        }
        else
        {
            if ( !IsFreeDeposeObject(Math::Vector(2.5f, 0.0f, 0.0f)) )  return ERR_MANIP_OCC;
//?         m_camera->StartCentering(m_object, Math::PI*0.3f, 99.9f, 0.0f, 0.8f);
            m_arm = TTA_FFRONT;
        }
    }

    m_bTurn = true;  // preliminary rotation necessary
    m_angle = oAngle;  // angle was reached

    m_physics->SetFreeze(true);  // it does not move

    m_bError = false;  // ok
    return ERR_OK;
}

// Indicates whether the action is finished.

Error CTaskTake::IsEnded()
{
    if ( m_engine->GetPause() )  return ERR_CONTINUE;
    if ( m_bError )  return ERR_STOP;

    if ( m_bTurn )  // preliminary rotation?
    {
        float angle = m_object->GetRotationY();
        angle = Math::NormAngle(angle);  // 0..2*Math::PI

        if ( Math::TestAngle(angle, m_angle-Math::PI*0.01f, m_angle+Math::PI*0.01f) )
        {
            m_bTurn = false;  // rotation ended
            m_physics->SetMotorSpeedZ(0.0f);

            if ( m_arm == TTA_FFRONT )
            {
                m_motion->SetAction(MHS_TAKE, 0.2f);  // will decrease
            }
            if ( m_arm == TTA_FRIEND )
            {
                if ( m_height <= 3.0f )
                {
                    m_motion->SetAction(MHS_TAKEOTHER, 0.2f);  // will decrease
                }
                else
                {
                    m_motion->SetAction(MHS_TAKEHIGH, 0.2f);  // will decrease
                }
            }
            m_progress = 0.0f;
            m_speed = 1.0f/0.6f;
        }
        return ERR_CONTINUE;
    }

    if ( m_progress < 1.0f )  return ERR_CONTINUE;
    m_progress = 0.0f;

    m_step ++;

    if ( m_order == TTO_TAKE )
    {
        if ( m_step == 1 )
        {
            if ( TransporterTakeObject() )
            {
                if ( m_arm == TTA_FRIEND &&
                     m_object->GetSlotContainedObjectReq(CSlottedObject::Pseudoslot::CARRYING)->Implements(ObjectInterfaceType::PowerContainer) )
                {
                    m_sound->Play(SOUND_POWEROFF, m_object->GetPosition());
                }
            }
            m_motion->SetAction(MHS_UPRIGHT, 0.4f);  // gets up
            m_progress = 0.0f;
            m_speed = 1.0f/0.8f;
            m_camera->StopCentering(m_object, 0.8f);
            return ERR_CONTINUE;
        }
    }

    if ( m_order == TTO_DEPOSE )
    {
        if ( m_step == 1 )
        {
            CObject* cargo = m_object->GetSlotContainedObjectReq(CSlottedObject::Pseudoslot::CARRYING);
            TransporterDeposeObject();
            if ( m_arm == TTA_FRIEND &&
                 cargo->Implements(ObjectInterfaceType::PowerContainer) )
            {
                m_sound->Play(SOUND_POWERON, m_object->GetPosition());
            }
            if ( cargo != nullptr && m_cargoType == OBJECT_METAL && m_arm == TTA_FFRONT )
            {
                m_main->ShowDropZone(cargo, m_object);  // shows buildable area
            }
            m_motion->SetAction(-1);  // gets up
            m_progress = 0.0f;
            m_speed = 1.0f/0.4f;
            m_camera->StopCentering(m_object, 0.8f);
            return ERR_CONTINUE;
        }
    }

    Abort();
    return ERR_STOP;
}

// Suddenly ends the current action.

bool CTaskTake::Abort()
{
    m_motion->SetAction(-1);
    m_camera->StopCentering(m_object, 0.8f);
    m_physics->SetFreeze(false);  // is moving again
    return true;
}


// Seeks the object to take in front.

CObject* CTaskTake::SearchTakeObject(float &angle,
                                     float dLimit, float aLimit)
{
    CObject     *pBest;
    Math::Vector    iPos, oPos;
    float       min, iAngle, bAngle, a, distance;

    iPos   = m_object->GetPosition();
    iAngle = m_object->GetRotationY();
    iAngle = Math::NormAngle(iAngle);  // 0..2*Math::PI

    min = 1000000.0f;
    pBest = nullptr;
    bAngle = 0.0f;
    for (CObject* pObj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        if ( !pObj->Implements(ObjectInterfaceType::Transportable) )  continue;

        if (IsObjectBeingTransported(pObj))  continue;
        if ( pObj->GetLock() )  continue;
        if ( pObj->GetScaleY() != 1.0f )  continue;

        oPos = pObj->GetPosition();
        distance = Math::Distance(oPos, iPos);
        if ( distance >= 4.0f-dLimit &&
             distance <= 4.0f+dLimit )
        {
            angle = Math::RotateAngle(oPos.x-iPos.x, iPos.z-oPos.z);  // CW !
            if ( Math::TestAngle(angle, iAngle-aLimit, iAngle+aLimit) )
            {
                a = fabs(angle-iAngle);
                if ( a > Math::PI )  a = Math::PI*2.0f-a;
                if ( a < min )
                {
                    min = a;
                    pBest = pObj;
                    bAngle = angle;
                }
            }
        }
    }
    angle = bAngle;
    return pBest;
}

// Seeks the robot on which you want take or put a battery.

CObject* CTaskTake::SearchFriendObject(float &angle,
                                       float dLimit, float aLimit,
                                       int &slotNumOut)
{
    if (m_object->GetCrashSphereCount() == 0) return nullptr;

    auto crashSphere = m_object->GetFirstCrashSphere();
    Math::Vector iPos = crashSphere.sphere.pos;
    float iRad = crashSphere.sphere.radius;

    float iAngle = m_object->GetRotationY();
    iAngle = Math::NormAngle(iAngle);  // 0..2*Math::PI

    for (CObject* pObj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        if ( pObj == m_object )  continue;  // yourself?
        if (!pObj->Implements(ObjectInterfaceType::Slotted)) continue;

        CSlottedObject *obj = dynamic_cast<CSlottedObject*>(pObj);

        int slotNum = obj->GetNumSlots();
        for (int slot = 0; slot < slotNum; slot++)
        {
            CObject *objectInSlot = obj->GetSlotContainedObject(slot);
            if (objectInSlot != nullptr && (objectInSlot->GetLock() || objectInSlot->GetScaleY() != 1.0f))
                continue;

            float objectAngleOffsetLimit = obj->GetSlotAcceptanceAngle(slot);
            if (objectAngleOffsetLimit == 0)
                continue; // slot isn't take-able

            Math::Matrix* mat = pObj->GetWorldMatrix(0);
            Math::Vector worldSlotPos = Transform(*mat, obj->GetSlotPosition(slot));

            // The robot must be in the correct angle relative to the slot (it can't be on the other side of the object)
            float angleFromObjectToRobot = Math::RotateAngle(iPos.x-worldSlotPos.x, worldSlotPos.z-iPos.z);  // CW !
            float objectIdealAngle = Math::NormAngle(pObj->GetRotationY() + obj->GetSlotAngle(slot));

            if ( Math::TestAngle(angleFromObjectToRobot, objectIdealAngle - objectAngleOffsetLimit, objectIdealAngle + objectAngleOffsetLimit) )
            {
                float distance = fabs(Math::Distance(worldSlotPos, iPos)-(iRad + 1.0f));
                // The robot must be close enough to the slot
                if ( distance <= dLimit )
                {
                    // The slot must be in the correct position relative to the robot (the robot must be facing towards the slot, not sideways or away)
                    angle = Math::RotateAngle(worldSlotPos.x-iPos.x, iPos.z-worldSlotPos.z);  // CW !
                    if ( Math::TestAngle(angle, iAngle-aLimit, iAngle+aLimit) )
                    {
                        Math::Vector powerPos = obj->GetSlotPosition(slot);
                        m_height = powerPos.y;
                        slotNumOut = slot;
                        return pObj;
                    }
                }
            }
        }
    }

    return nullptr;
}

// Takes the object in front.

bool CTaskTake::TransporterTakeObject()
{
    if (m_arm == TTA_FFRONT)  // takes on the ground in front?
    {
//?     cargo = SearchTakeObject(angle, 1.5f, Math::PI*0.04f);
        float angle = 0.0f;
        CObject* cargo = SearchTakeObject(angle, 1.5f, Math::PI*0.15f);  //OK 1.9
        if (cargo == nullptr)  return false;  // nothing to take ?
        assert(cargo->Implements(ObjectInterfaceType::Transportable));

        m_cargoType = cargo->GetType();

        dynamic_cast<CTransportableObject&>(*cargo).SetTransporter(m_object);
        dynamic_cast<CTransportableObject&>(*cargo).SetTransporterPart(4);  // takes with the hand

//?     cargo->SetPosition(Math::Vector(2.2f, -1.0f, 1.1f));
        cargo->SetPosition(Math::Vector(1.7f, -0.5f, 1.1f));
        cargo->SetRotationY(0.1f);
        cargo->SetRotationX(0.0f);
        cargo->SetRotationZ(0.8f);

        m_object->SetSlotContainedObjectReq(CSlottedObject::Pseudoslot::CARRYING, cargo);  // takes
    }

    if (m_arm == TTA_FRIEND)  // takes friend's battery?
    {
        float angle = 0.0f;
        int otherSlotNum = -1;
        CObject* other = SearchFriendObject(angle, 1.5f, Math::PI*0.04f, otherSlotNum);
        if (other == nullptr)  return false;
        CSlottedObject* otherAsSlotted = dynamic_cast<CSlottedObject*>(other);
        assert(otherSlotNum >= -1);

        CObject* cargo = otherAsSlotted->GetSlotContainedObject(otherSlotNum);
        if (cargo == nullptr)  return false;  // the other does not have a battery?
        assert(cargo->Implements(ObjectInterfaceType::Transportable));

        m_cargoType = cargo->GetType();

        otherAsSlotted->SetSlotContainedObject(otherSlotNum, nullptr);
        dynamic_cast<CTransportableObject&>(*cargo).SetTransporter(m_object);
        dynamic_cast<CTransportableObject&>(*cargo).SetTransporterPart(4);  // takes with the hand

//?     cargo->SetPosition(Math::Vector(2.2f, -1.0f, 1.1f));
        cargo->SetPosition(Math::Vector(1.7f, -0.5f, 1.1f));
        cargo->SetRotationY(0.1f);
        cargo->SetRotationX(0.0f);
        cargo->SetRotationZ(0.8f);

        m_object->SetSlotContainedObjectReq(CSlottedObject::Pseudoslot::CARRYING, cargo);  // takes
    }

    return true;
}

// Deposes the object taken.

bool CTaskTake::TransporterDeposeObject()
{
    if ( m_arm == TTA_FFRONT )  // deposes on the ground in front?
    {
        CObject* cargo = m_object->GetSlotContainedObjectReq(CSlottedObject::Pseudoslot::CARRYING);
        if (cargo == nullptr)  return false;  // does nothing?
        assert(cargo->Implements(ObjectInterfaceType::Transportable));

        m_cargoType = cargo->GetType();

        Math::Matrix* mat = cargo->GetWorldMatrix(0);
        Math::Vector pos = Transform(*mat, Math::Vector(-0.5f, 1.0f, 0.0f));
        m_terrain->AdjustToFloor(pos);
        cargo->SetPosition(pos);
        cargo->SetRotationY(m_object->GetRotationY()+Math::PI/2.0f);
        cargo->SetRotationX(0.0f);
        cargo->SetRotationZ(0.0f);
        cargo->FloorAdjust();  // plate well on the ground

        dynamic_cast<CTransportableObject&>(*cargo).SetTransporter(nullptr);
        m_object->SetSlotContainedObjectReq(CSlottedObject::Pseudoslot::CARRYING, nullptr);  // deposit
    }

    if ( m_arm == TTA_FRIEND )  // deposes battery on friends?
    {
        float angle = 0.0f;
        int otherSlotNum = -1;
        CObject* other = SearchFriendObject(angle, 1.5f, Math::PI*0.04f, otherSlotNum);
        if (other == nullptr)  return false;
        CSlottedObject* otherAsSlotted = dynamic_cast<CSlottedObject*>(other);
        assert(otherSlotNum >= 0);

        CObject* cargo = otherAsSlotted->GetSlotContainedObject(otherSlotNum);
        if (cargo != nullptr)  return false;  // the other already has a battery?

        cargo = m_object->GetSlotContainedObjectReq(CSlottedObject::Pseudoslot::CARRYING);
        if (cargo == nullptr)  return false;
        assert(cargo->Implements(ObjectInterfaceType::Transportable));
        m_cargoType = cargo->GetType();

        otherAsSlotted->SetSlotContainedObject(otherSlotNum, cargo);
        dynamic_cast<CTransportableObject&>(*cargo).SetTransporter(other);

        cargo->SetPosition(otherAsSlotted->GetSlotPosition(otherSlotNum));
        cargo->SetRotationY(0.0f);
        cargo->SetRotationX(0.0f);
        cargo->SetRotationZ(0.0f);
        dynamic_cast<CTransportableObject&>(*cargo).SetTransporterPart(0);  // carried by the base

        m_object->SetSlotContainedObjectReq(CSlottedObject::Pseudoslot::CARRYING, nullptr);  // deposit
    }

    return true;
}

// Seeks if a location allows to deposit an object.

bool CTaskTake::IsFreeDeposeObject(Math::Vector pos)
{
    Math::Matrix* mat = m_object->GetWorldMatrix(0);
    Math::Vector iPos = Transform(*mat, pos);

    for (CObject* pObj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        if ( pObj == m_object )  continue;
        if ( !pObj->GetDetectable() )  continue;  // inactive?
        if (IsObjectBeingTransported(pObj))  continue;

        for (const auto& crashSphere : pObj->GetAllCrashSpheres())
        {
            if ( Math::Distance(iPos, crashSphere.sphere.pos)-(crashSphere.sphere.radius+1.0f) < 1.0f )
            {
                return false;  // location occupied
            }
        }
    }
    return true;  // location free
}
