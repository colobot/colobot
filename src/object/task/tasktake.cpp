/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2014, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsiteс.ch; http://colobot.info; http://github.com/colobot
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

#include "math/geometry.h"

#include "object/motion/motionhuman.h"
#include "object/object_manager.h"
#include "object/robotmain.h"
#include "object/interface/carrier_object.h"
#include "object/interface/transportable_object.h"

#include "physics/physics.h"




// Object's constructor.

CTaskTake::CTaskTake(CObject* object) : CTask(object)
{
    m_arm  = TTA_NEUTRAL;

    assert(m_object->Implements(ObjectInterfaceType::Carrier));
    m_carrier = dynamic_cast<CCarrierObject*>(m_object);
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
        a = m_object->GetAngleY(0);
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
    ObjectType  type;
    CObject*    other;
    float       iAngle, oAngle, h;
    Math::Vector    pos;

    m_height   = 0.0f;
    m_step     = 0;
    m_progress = 0.0f;

    iAngle = m_object->GetAngleY(0);
    iAngle = Math::NormAngle(iAngle);  // 0..2*Math::PI
    oAngle = iAngle;

    m_bError = true;  // operation impossible
    if ( !m_physics->GetLand() )
    {
        pos = m_object->GetPosition(0);
        h = m_water->GetLevel(m_object);
        if ( pos.y < h )  return ERR_MANIP_WATER;  // impossible under water
        return ERR_MANIP_FLY;
    }

    type = m_object->GetType();
    if ( type != OBJECT_HUMAN &&
         type != OBJECT_TECH  )  return ERR_MANIP_VEH;

    m_physics->SetMotorSpeed(Math::Vector(0.0f, 0.0f, 0.0f));

    if (m_carrier->IsCarryingCargo())
        m_order = TTO_DEPOSE;
    else
        m_order = TTO_TAKE;


    if ( m_order == TTO_TAKE )
    {
        pos = m_object->GetPosition(0);
        h = m_water->GetLevel(m_object);
        if ( pos.y < h )  return ERR_MANIP_WATER;  // impossible under water

        other = SearchFriendObject(oAngle, 1.5f, Math::PI*0.50f);
        if ( other != 0 && other->GetPower() != 0 )
        {
            type = other->GetPower()->GetType();
            if ( type == OBJECT_URANIUM )  return ERR_MANIP_RADIO;
            if ( type != OBJECT_FRET    &&
                 type != OBJECT_STONE   &&
                 type != OBJECT_BULLET  &&
                 type != OBJECT_METAL   &&
                 type != OBJECT_POWER   &&
                 type != OBJECT_ATOMIC  &&
                 type != OBJECT_BBOX    &&
                 type != OBJECT_KEYa    &&
                 type != OBJECT_KEYb    &&
                 type != OBJECT_KEYc    &&
                 type != OBJECT_KEYd    &&
                 type != OBJECT_TNT     )  return ERR_MANIP_FRIEND;
//?         m_camera->StartCentering(m_object, Math::PI*0.3f, -Math::PI*0.1f, 0.0f, 0.8f);
            m_arm = TTA_FRIEND;
        }
        else
        {
            other = SearchTakeObject(oAngle, 1.5f, Math::PI*0.45f);
            if ( other == 0 )  return ERR_MANIP_NIL;
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

        other = SearchFriendObject(oAngle, 1.5f, Math::PI*0.50f);
        if ( other != 0 && other->GetPower() == 0 )
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
    CObject*    cargo;
    float       angle;

    if ( m_engine->GetPause() )  return ERR_CONTINUE;
    if ( m_bError )  return ERR_STOP;

    if ( m_bTurn )  // preliminary rotation?
    {
        angle = m_object->GetAngleY(0);
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
                     (m_cargoType == OBJECT_POWER  ||
                      m_cargoType == OBJECT_ATOMIC ) )
                {
                    m_sound->Play(SOUND_POWEROFF, m_object->GetPosition(0));
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
            cargo = m_carrier->GetCargo();
            TransporterDeposeObject();
            if ( m_arm == TTA_FRIEND &&
                 (m_cargoType == OBJECT_POWER  ||
                  m_cargoType == OBJECT_ATOMIC ) )
            {
                m_sound->Play(SOUND_POWERON, m_object->GetPosition(0));
            }
            if ( cargo != 0 && m_cargoType == OBJECT_METAL && m_arm == TTA_FFRONT )
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
    ObjectType  type;
    float       min, iAngle, bAngle, a, distance;

    iPos   = m_object->GetPosition(0);
    iAngle = m_object->GetAngleY(0);
    iAngle = Math::NormAngle(iAngle);  // 0..2*Math::PI

    min = 1000000.0f;
    pBest = 0;
    bAngle = 0.0f;
    for (CObject* pObj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        type = pObj->GetType();

        if ( type != OBJECT_FRET    &&
             type != OBJECT_STONE   &&
             type != OBJECT_URANIUM &&
             type != OBJECT_BULLET  &&
             type != OBJECT_METAL   &&
             type != OBJECT_POWER   &&
             type != OBJECT_ATOMIC  &&
             type != OBJECT_BBOX    &&
             type != OBJECT_KEYa    &&
             type != OBJECT_KEYb    &&
             type != OBJECT_KEYc    &&
             type != OBJECT_KEYd    &&
             type != OBJECT_TNT     )  continue;

        if (IsObjectBeingTransported(pObj))  continue;
        if ( pObj->GetLock() )  continue;
        if ( pObj->GetZoomY(0) != 1.0f )  continue;

        oPos = pObj->GetPosition(0);
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
                                       float dLimit, float aLimit)
{
    Character*  character;
    CObject*    pPower;
    Math::Matrix*   mat;
    ObjectType  type, powerType;
    float       iAngle, distance;

    if (m_object->GetCrashSphereCount() == 0) return 0;

    auto crashSphere = m_object->GetFirstCrashSphere();
    Math::Vector iPos = crashSphere.sphere.pos;
    float iRad = crashSphere.sphere.radius;

    iAngle = m_object->GetAngleY(0);
    iAngle = Math::NormAngle(iAngle);  // 0..2*Math::PI

    for (CObject* pObj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        if ( pObj == m_object )  continue;  // yourself?

        type = pObj->GetType();
        if ( type != OBJECT_MOBILEfa &&
             type != OBJECT_MOBILEta &&
             type != OBJECT_MOBILEwa &&
             type != OBJECT_MOBILEia &&
             type != OBJECT_MOBILEfc &&
             type != OBJECT_MOBILEtc &&
             type != OBJECT_MOBILEwc &&
             type != OBJECT_MOBILEic &&
             type != OBJECT_MOBILEfi &&
             type != OBJECT_MOBILEti &&
             type != OBJECT_MOBILEwi &&
             type != OBJECT_MOBILEii &&
             type != OBJECT_MOBILEfs &&
             type != OBJECT_MOBILEts &&
             type != OBJECT_MOBILEws &&
             type != OBJECT_MOBILEis &&
             type != OBJECT_MOBILErt &&
             type != OBJECT_MOBILErc &&
             type != OBJECT_MOBILErr &&
             type != OBJECT_MOBILErs &&
             type != OBJECT_MOBILEsa &&
             type != OBJECT_MOBILEtg &&
             type != OBJECT_MOBILEft &&
             type != OBJECT_MOBILEtt &&
             type != OBJECT_MOBILEwt &&
             type != OBJECT_MOBILEit &&
             type != OBJECT_TOWER    &&
             type != OBJECT_RESEARCH &&
             type != OBJECT_ENERGY   &&
             type != OBJECT_LABO     &&
             type != OBJECT_NUCLEAR  )  continue;

        pPower = pObj->GetPower();
        if ( pPower != 0 )
        {
            if ( pPower->GetLock() )  continue;
            if ( pPower->GetZoomY(0) != 1.0f )  continue;

            powerType = pPower->GetType();
            if ( powerType == OBJECT_NULL ||
                 powerType == OBJECT_FIX  )  continue;
        }

        mat = pObj->GetWorldMatrix(0);
        character = pObj->GetCharacter();
        Math::Vector oPos = Transform(*mat, character->posPower);

        distance = fabs(Math::Distance(oPos, iPos) - (iRad+1.0f));
        if ( distance <= dLimit )
        {
            angle = Math::RotateAngle(oPos.x-iPos.x, iPos.z-oPos.z);  // CW !
            if ( Math::TestAngle(angle, iAngle-aLimit, iAngle+aLimit) )
            {
                character = pObj->GetCharacter();
                m_height = character->posPower.y;
                return pObj;
            }
        }
    }

    return 0;
}

// Takes the object in front.

bool CTaskTake::TransporterTakeObject()
{
    CObject*     cargo;
    CObject*     other;
    Math::Matrix matRotate;
    float        angle;

    if ( m_arm == TTA_FFRONT )  // takes on the ground in front?
    {
//?     cargo = SearchTakeObject(angle, 1.5f, Math::PI*0.04f);
        cargo = SearchTakeObject(angle, 1.5f, Math::PI*0.15f);  //OK 1.9
        if ( cargo == 0 )  return false;  // rien � prendre ?
        m_cargoType = cargo->GetType();

        dynamic_cast<CTransportableObject*>(cargo)->SetTransporter(m_object);
        dynamic_cast<CTransportableObject*>(cargo)->SetTransporterPart(4);  // takes with the hand

//?     cargo->SetPosition(0, Math::Vector(2.2f, -1.0f, 1.1f));
        cargo->SetPosition(0, Math::Vector(1.7f, -0.5f, 1.1f));
        cargo->SetAngleY(0, 0.1f);
        cargo->SetAngleX(0, 0.0f);
        cargo->SetAngleZ(0, 0.8f);

        m_carrier->SetCargo(cargo);  // takes
    }

    if ( m_arm == TTA_FRIEND )  // takes friend's battery?
    {
        other = SearchFriendObject(angle, 1.5f, Math::PI*0.04f);
        if ( other == 0 )  return false;

        cargo = other->GetPower();
        if ( cargo == 0 )  return false;  // the other does not have a battery?
        m_cargoType = cargo->GetType();

        other->SetPower(0);
        dynamic_cast<CTransportableObject*>(cargo)->SetTransporter(m_object);
        dynamic_cast<CTransportableObject*>(cargo)->SetTransporterPart(4);  // takes with the hand

//?     cargo->SetPosition(0, Math::Vector(2.2f, -1.0f, 1.1f));
        cargo->SetPosition(0, Math::Vector(1.7f, -0.5f, 1.1f));
        cargo->SetAngleY(0, 0.1f);
        cargo->SetAngleX(0, 0.0f);
        cargo->SetAngleZ(0, 0.8f);

        m_carrier->SetCargo(cargo);  // takes
    }

    return true;
}

// Deposes the object taken.

bool CTaskTake::TransporterDeposeObject()
{
    Character*  character;
    CObject*    cargo;
    CObject*    other;
    Math::Matrix*   mat;
    Math::Vector    pos;
    float       angle;

    if ( m_arm == TTA_FFRONT )  // deposes on the ground in front?
    {
        cargo = m_carrier->GetCargo();
        if ( cargo == 0 )  return false;  // does nothing?
        m_cargoType = cargo->GetType();

        mat = cargo->GetWorldMatrix(0);
        pos = Transform(*mat, Math::Vector(-0.5f, 1.0f, 0.0f));
        m_terrain->AdjustToFloor(pos);
        cargo->SetPosition(0, pos);
        cargo->SetAngleY(0, m_object->GetAngleY(0)+Math::PI/2.0f);
        cargo->SetAngleX(0, 0.0f);
        cargo->SetAngleZ(0, 0.0f);
        cargo->FloorAdjust();  // plate well on the ground

        dynamic_cast<CTransportableObject*>(cargo)->SetTransporter(0);
        m_carrier->SetCargo(0);  // deposit
    }

    if ( m_arm == TTA_FRIEND )  // deposes battery on friends?
    {
        other = SearchFriendObject(angle, 1.5f, Math::PI*0.04f);
        if ( other == 0 )  return false;

        cargo = other->GetPower();
        if ( cargo != 0 )  return false;  // the other already has a battery?

        cargo = m_carrier->GetCargo();
        if ( cargo == 0 )  return false;
        m_cargoType = cargo->GetType();

        other->SetPower(cargo);
        dynamic_cast<CTransportableObject*>(cargo)->SetTransporter(other);

        character = other->GetCharacter();
        cargo->SetPosition(0, character->posPower);
        cargo->SetAngleY(0, 0.0f);
        cargo->SetAngleX(0, 0.0f);
        cargo->SetAngleZ(0, 0.0f);
        dynamic_cast<CTransportableObject*>(cargo)->SetTransporterPart(0);  // carried by the base

        m_carrier->SetCargo(0);  // deposit
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
        if ( !pObj->GetActive() )  continue;  // inactive?
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

