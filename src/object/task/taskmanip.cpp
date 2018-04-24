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


#include "object/task/taskmanip.h"

#include "graphics/engine/pyro_manager.h"
#include "graphics/engine/terrain.h"

#include "level/robotmain.h"

#include "math/geometry.h"

#include "object/object_manager.h"
#include "object/old_object.h"

#include "object/interface/carrier_object.h"
#include "object/interface/powered_object.h"
#include "object/interface/transportable_object.h"

#include "physics/physics.h"

#include "sound/sound.h"


//?const float MARGIN_FRONT     = 2.0f;
//?const float MARGIN_BACK      = 2.0f;
//?const float MARGIN_FRIEND    = 2.0f;
//?const float MARGIN_BEE       = 5.0f;
const float MARGIN_FRONT    = 4.0f;  //OK 1.9
const float MARGIN_BACK     = 4.0f;  //OK 1.9
const float MARGIN_FRIEND   = 4.0f;  //OK 1.9
const float MARGIN_BEE      = 5.0f;  //OK 1.9




// Object's constructor.

CTaskManip::CTaskManip(COldObject* object) : CForegroundTask(object)
{
    m_arm  = TMA_NEUTRAL;
    m_hand = TMH_OPEN;

    assert(m_object->Implements(ObjectInterfaceType::Carrier));
}

// Object's destructor.

CTaskManip::~CTaskManip()
{
}


// Management of an event.

bool CTaskManip::EventProcess(const Event &event)
{
    Math::Vector    pos;
    float       angle, a, g, cirSpeed, progress;
    int         i;

    if ( m_engine->GetPause() )  return true;
    if ( event.type != EVENT_FRAME )  return true;
    if ( m_bError )  return false;

    if ( m_bBee )  // bee?
    {
        return true;
    }

    if ( m_bTurn )  // preliminary rotation?
    {
        a = m_object->GetRotationY();
        g = m_angle;
        cirSpeed = Math::Direction(a, g)*1.0f;
        if ( m_object->Implements(ObjectInterfaceType::Flying) )  // flying on the ground?
        {
            cirSpeed *= 4.0f;  // more fishing
        }
        if ( cirSpeed >  1.0f )  cirSpeed =  1.0f;
        if ( cirSpeed < -1.0f )  cirSpeed = -1.0f;

        m_physics->SetMotorSpeedZ(cirSpeed);  // turns left / right
        return true;
    }

    if ( m_move != 0 )  // preliminary advance?
    {
        m_timeLimit -= event.rTime;
        m_physics->SetMotorSpeedX(m_move);  // forward/backward
        return true;
    }

    m_progress += event.rTime*m_speed;  // others advance
    progress = m_progress;
    if ( progress > 1.0f )  progress = 1.0f;

    if ( m_bSubm )  // submarine?
    {
        if ( m_order == TMO_GRAB )
        {
            if ( m_step == 0 )  // fall?
            {
                pos = m_object->GetPartPosition(1);
                pos.y = 3.0f-progress*2.0f;
                m_object->SetPartPosition(1, pos);
            }
            if ( m_step == 1 )  // farm?
            {
                pos = m_object->GetPartPosition(2);
                pos.z = -1.5f+progress*0.5f;
                m_object->SetPartPosition(2, pos);

                pos = m_object->GetPartPosition(3);
                pos.z = 1.5f-progress*0.5f;
                m_object->SetPartPosition(3, pos);
            }
            if ( m_step == 2 )  // up?
            {
                pos = m_object->GetPartPosition(1);
                pos.y = 3.0f-(1.0f-progress)*2.0f;
                m_object->SetPartPosition(1, pos);
            }
        }
        else
        {
            if ( m_step == 0 )  // fall?
            {
                pos = m_object->GetPartPosition(1);
                pos.y = 3.0f-progress*2.0f;
                m_object->SetPartPosition(1, pos);
            }
            if ( m_step == 1 )  // farm?
            {
                pos = m_object->GetPartPosition(2);
                pos.z = -1.5f+(1.0f-progress)*0.5f;
                m_object->SetPartPosition(2, pos);

                pos = m_object->GetPartPosition(3);
                pos.z = 1.5f-(1.0f-progress)*0.5f;
                m_object->SetPartPosition(3, pos);
            }
            if ( m_step == 2 )  // up?
            {
                pos = m_object->GetPartPosition(1);
                pos.y = 3.0f-(1.0f-progress)*2.0f;
                m_object->SetPartPosition(1, pos);
            }
        }
    }
    else
    {
        for ( i=0 ; i<5 ; i++ )
        {
            angle = (m_finalAngle[i]-m_initialAngle[i])*progress;
            angle += m_initialAngle[i];
            m_object->SetPartRotationZ(i+1, angle);
        }
    }

    return true;
}


// Initializes the initial and final angles.

void CTaskManip::InitAngle()
{
    if ( m_bSubm || m_bBee )  return;

    if ( m_arm == TMA_NEUTRAL ||
         m_arm == TMA_GRAB    )
    {
        m_finalAngle[0] = ARM_NEUTRAL_ANGLE1;  // arm
        m_finalAngle[1] = ARM_NEUTRAL_ANGLE2;  // forearm
        m_finalAngle[2] = ARM_NEUTRAL_ANGLE3;  // hand
    }
    if ( m_arm == TMA_STOCK )
    {
        m_finalAngle[0] = ARM_STOCK_ANGLE1;  // arm
        m_finalAngle[1] = ARM_STOCK_ANGLE2;  // forearm
        m_finalAngle[2] = ARM_STOCK_ANGLE3;  // hand
    }
    if ( m_arm == TMA_FFRONT )
    {
        m_finalAngle[0] =   35.0f*Math::PI/180.0f;  // arm
        m_finalAngle[1] =  -95.0f*Math::PI/180.0f;  // forearm
        m_finalAngle[2] =  -27.0f*Math::PI/180.0f;  // hand
    }
    if ( m_arm == TMA_FBACK )
    {
        m_finalAngle[0] =  145.0f*Math::PI/180.0f;  // arm
        m_finalAngle[1] =   95.0f*Math::PI/180.0f;  // forearm
        m_finalAngle[2] =   27.0f*Math::PI/180.0f;  // hand
    }
    if ( m_arm == TMA_POWER )
    {
        m_finalAngle[0] =   95.0f*Math::PI/180.0f;  // arm
        m_finalAngle[1] =  125.0f*Math::PI/180.0f;  // forearm
        m_finalAngle[2] =   50.0f*Math::PI/180.0f;  // hand
    }
    if ( m_arm == TMA_OTHER )
    {
        if ( m_height <= 3.0f )
        {
            m_finalAngle[0] =  55.0f*Math::PI/180.0f;  // arm
            m_finalAngle[1] = -90.0f*Math::PI/180.0f;  // forearm
            m_finalAngle[2] = -35.0f*Math::PI/180.0f;  // hand
        }
        else
        {
            m_finalAngle[0] =  70.0f*Math::PI/180.0f;  // arm
            m_finalAngle[1] = -90.0f*Math::PI/180.0f;  // forearm
            m_finalAngle[2] = -50.0f*Math::PI/180.0f;  // hand
        }
    }

    if ( m_hand == TMH_OPEN )  // open clamp?
    {
        m_finalAngle[3] = -Math::PI*0.10f;  // clamp close
        m_finalAngle[4] =  Math::PI*0.10f;  // clamp remote
    }
    if ( m_hand == TMH_CLOSE )  // clamp closed?
    {
        m_finalAngle[3] =  Math::PI*0.05f;  // clamp close
        m_finalAngle[4] = -Math::PI*0.05f;  // clamp remote
    }

    for (int i = 0; i < 5; i++)
    {
        m_initialAngle[i] = m_object->GetPartRotationZ(i+1);
    }

    float max = 0.0f;
    for (int i = 0;  i < 5; i++)
    {
        max = Math::Max(max, fabs(m_initialAngle[i] - m_finalAngle[i]));
    }
    m_speed = (Math::PI*1.0f)/max;
    if ( m_speed > 3.0f )  m_speed = 3.0f;  // piano, ma non troppo (?)

    float energy = GetObjectEnergy(m_object);
    if ( energy == 0.0f )
    {
        m_speed *= 0.7f;  // slower if more energy!
    }
}

// Assigns the goal was achieved.

Error CTaskManip::Start(TaskManipOrder order, TaskManipArm arm)
{
    ObjectType   type;
    CObject      *front, *other;
    float        iAngle, dist, len;
    float        fDist, fAngle, oDist, oAngle, oHeight;
    Math::Vector pos, fPos, oPos;

    m_arm      = arm;
    m_height   = 0.0f;
    m_step     = 0;
    m_progress = 0.0f;
    m_speed    = 1.0f/1.5f;

    iAngle = m_object->GetRotationY();
    iAngle = Math::NormAngle(iAngle);  // 0..2*Math::PI
    oAngle = iAngle;

    m_bError = true;  // operation impossible

    if ( m_arm != TMA_FFRONT &&
         m_arm != TMA_FBACK  &&
         m_arm != TMA_POWER  &&
         m_arm != TMA_GRAB   )  return ERR_WRONG_BOT;

    m_physics->SetMotorSpeed(Math::Vector(0.0f, 0.0f, 0.0f));

    type = m_object->GetType();
    if ( type == OBJECT_BEE )  // bee?
    {
        if (m_object->GetCargo() == nullptr)
        {
            if ( !m_physics->GetLand() )  return ERR_MANIP_FLY;

            other = SearchTakeUnderObject(m_targetPos, MARGIN_BEE);
            if (other == nullptr)  return ERR_MANIP_NIL;
            assert(other->Implements(ObjectInterfaceType::Transportable));

            m_object->SetCargo(other);  // takes the ball
            dynamic_cast<CTransportableObject*>(other)->SetTransporter(m_object);
            dynamic_cast<CTransportableObject*>(other)->SetTransporterPart(0);  // taken with the base
            other->SetPosition(Math::Vector(0.0f, -3.0f, 0.0f));
        }
        else
        {
            other = m_object->GetCargo();  // other = ball
            assert(other->Implements(ObjectInterfaceType::Transportable));

            m_object->SetCargo(nullptr);  // lick the ball
            dynamic_cast<CTransportableObject*>(other)->SetTransporter(nullptr);
            pos = m_object->GetPosition();
            pos.y -= 3.0f;
            other->SetPosition(pos);

            pos = m_object->GetPosition();
            pos.y += 2.0f;
            m_object->SetPosition(pos);  // against the top of jump

            m_engine->GetPyroManager()->Create(Gfx::PT_FALL, other);  // the ball falls
        }

        m_bBee = true;
        m_bError = false;  // ok
        return ERR_OK;
    }
    m_bBee = false;

    m_bSubm = ( type == OBJECT_MOBILEsa );  // submarine?

    if ( m_arm == TMA_GRAB )  // takes immediately?
    {
        TransporterTakeObject();
        Abort();
        return ERR_OK;
    }

    m_energy = GetObjectEnergy(m_object);

    if ( !m_physics->GetLand() )  return ERR_MANIP_FLY;

    if ( type != OBJECT_MOBILEfa &&
         type != OBJECT_MOBILEta &&
         type != OBJECT_MOBILEwa &&
         type != OBJECT_MOBILEia &&
         type != OBJECT_MOBILEsa )  return ERR_WRONG_BOT;

    if ( m_bSubm )  // submarine?
    {
        m_arm = TMA_FFRONT;  // only possible in front!
    }

    m_move = 0.0f;  // advance not necessary
    m_angle = iAngle;

    if ( order == TMO_AUTO )
    {
        if (m_object->GetCargo() == nullptr)
        {
            m_order = TMO_GRAB;
        }
        else
        {
            m_order = TMO_DROP;
        }
    }
    else
    {
        m_order = order;
    }

    if (m_order == TMO_GRAB && m_object->GetCargo() != nullptr)
    {
        return ERR_MANIP_BUSY;
    }
    if (m_order == TMO_DROP && m_object->GetCargo() == nullptr)
    {
        return ERR_MANIP_EMPTY;
    }

//? speed = m_physics->GetMotorSpeed();
//? if ( speed.x != 0.0f ||
//?      speed.z != 0.0f )  return ERR_MANIP_MOTOR;

    if ( m_order == TMO_GRAB )
    {
        if ( m_arm == TMA_FFRONT )
        {
            front = SearchTakeFrontObject(true, fPos, fDist, fAngle);
            other = SearchOtherObject(true, oPos, oDist, oAngle, oHeight);

            if ( front != nullptr && fDist < oDist )
            {
                m_targetPos = fPos;
                m_angle = fAngle;
                m_move = 1.0f;  // advance required
            }
            else if ( other != nullptr && oDist < fDist )
            {
                if (! ObjectHasPowerCell(other)) return ERR_MANIP_NIL;
                m_targetPos = oPos;
                m_angle = oAngle;
                m_height = oHeight;
                m_move = 1.0f;  // advance required
                m_arm = TMA_OTHER;
            }
            else
            {
                return ERR_MANIP_NIL;
            }
            m_main->HideDropZone(front);  // hides buildable area
        }
        if ( m_arm == TMA_FBACK )
        {
            if ( SearchTakeBackObject(true, m_targetPos, fDist, m_angle) == nullptr )
            {
                return ERR_MANIP_NIL;
            }
            m_angle += Math::PI;
            m_move = -1.0f;  // back necessary
        }
        if ( m_arm == TMA_POWER )
        {
            assert(m_object->Implements(ObjectInterfaceType::Powered));
            if (m_object->GetPower() == nullptr)  return ERR_MANIP_NIL;
        }
    }

    if ( m_order == TMO_DROP )
    {
        if ( m_arm == TMA_FFRONT )
        {
            other = SearchOtherObject(true, oPos, oDist, oAngle, oHeight);
            if (other != nullptr && !ObjectHasPowerCell(other))
            {
                m_targetPos = oPos;
                m_angle = oAngle;
                m_height = oHeight;
                m_move = 1.0f;  // advance required
                m_arm = TMA_OTHER;
            }
            else
            {
                if ( !IsFreeDeposeObject(Math::Vector(TAKE_DIST, 0.0f, 0.0f)) )  return ERR_MANIP_OCC;
            }
        }
        if ( m_arm == TMA_FBACK )
        {
            if ( !IsFreeDeposeObject(Math::Vector(-TAKE_DIST, 0.0f, 0.0f)) )  return ERR_MANIP_OCC;
        }
        if ( m_arm == TMA_POWER )
        {
            assert(m_object->Implements(ObjectInterfaceType::Powered));
            if (m_object->GetPower() != nullptr)  return ERR_MANIP_OCC;
        }
    }

    dist = Math::Distance(m_object->GetPosition(), m_targetPos);
    len = dist-TAKE_DIST;
    if ( m_arm == TMA_OTHER ) len -= TAKE_DIST_OTHER;
    if ( len < 0.0f )  len = 0.0f;
    if ( m_arm == TMA_FBACK ) len = -len;
    m_advanceLength = dist-m_physics->GetLinLength(len);
    if ( dist <= m_advanceLength+0.2f )  m_move = 0.0f;  // not necessary to advance

    if ( m_energy == 0.0f )  m_move = 0.0f;

    if ( m_move != 0.0f )  // forward or backward?
    {
        m_timeLimit = m_physics->GetLinTimeLength(fabs(len))*1.5f;
        if ( m_timeLimit < 0.5f )  m_timeLimit = 0.5f;
    }

    if (m_object->GetCargo() == nullptr)  // not carrying anything?
    {
        m_hand = TMH_OPEN;  // open clamp
    }
    else
    {
        m_hand = TMH_CLOSE;  // closed clamp
    }

    InitAngle();

    if ( iAngle == m_angle || m_energy == 0.0f )
    {
        m_bTurn = false;  // preliminary rotation unnecessary
        SoundManip(1.0f/m_speed);
    }
    else
    {
        m_bTurn = true;  // preliminary rotation necessary
    }

    if ( m_bSubm )
    {
        m_camera->StartCentering(m_object, Math::PI*0.8f, 99.9f, 0.0f, 0.5f);
    }

    m_physics->SetFreeze(true);  // it does not move

    m_bError = false;  // ok
    return ERR_OK;
}

// Indicates whether the action is complete.

Error CTaskManip::IsEnded()
{
    CObject*    cargo;
    Math::Vector    pos;
    float       angle, dist;
    int         i;

    if ( m_engine->GetPause() )  return ERR_CONTINUE;
    if ( m_bError )  return ERR_STOP;

    if ( m_bBee )  // bee?
    {
        return ERR_STOP;
    }

    if ( m_bTurn )  // preliminary rotation?
    {
        angle = m_object->GetRotationY();
        angle = Math::NormAngle(angle);  // 0..2*Math::PI

        if ( Math::TestAngle(angle, m_angle-Math::PI*0.01f, m_angle+Math::PI*0.01f) )
        {
            m_bTurn = false;  // rotation ended
            m_physics->SetMotorSpeedZ(0.0f);
            if ( m_move == 0.0f )
            {
                SoundManip(1.0f/m_speed);
            }
        }
        return ERR_CONTINUE;
    }

    if ( m_move != 0.0f )  // preliminary advance?
    {
        if ( m_timeLimit <= 0.0f )
        {
//OK 1.9
            dist = Math::Distance(m_object->GetPosition(), m_targetPos);
            if ( dist <= m_advanceLength + 2.0f )
            {
                m_move = 0.0f;  // advance ended
                m_physics->SetMotorSpeedX(0.0f);
                SoundManip(1.0f/m_speed);
                return ERR_CONTINUE;
            }
            else
            {
//EOK 1.9
                m_move = 0.0f;  // advance ended
                m_physics->SetMotorSpeedX(0.0f);  // stops
                Abort();
                return ERR_STOP;
            }
        }

        dist = Math::Distance(m_object->GetPosition(), m_targetPos);
        if ( dist <= m_advanceLength )
        {
            m_move = 0.0f;  // advance ended
            m_physics->SetMotorSpeedX(0.0f);
            SoundManip(1.0f/m_speed);
        }
        return ERR_CONTINUE;
    }

    if ( m_progress < 1.0f )  return ERR_CONTINUE;
    m_progress = 0.0f;

    if ( !m_bSubm )
    {
        for ( i=0 ; i<5 ; i++ )
        {
            m_object->SetPartRotationZ(i+1, m_finalAngle[i]);
        }
    }
    m_step ++;

    if ( m_order == TMO_GRAB )
    {
        if ( m_step == 1 )
        {
            if ( m_bSubm )  m_speed = 1.0f/0.7f;
            m_hand = TMH_CLOSE;  // closes the clamp to take
            InitAngle();
            SoundManip(1.0f/m_speed, 0.8f, 1.5f);
            return ERR_CONTINUE;
        }
        if ( m_step == 2 )
        {
            if ( m_bSubm )  m_speed = 1.0f/1.5f;
            if ( !TransporterTakeObject() &&
                 m_object->GetCargo() == nullptr)
            {
                m_hand = TMH_OPEN;  // reopens the clamp
                m_arm = TMA_NEUTRAL;
                InitAngle();
                SoundManip(1.0f/m_speed, 0.8f, 1.5f);
            }
            else
            {
                if ( (m_arm == TMA_OTHER ||
                      m_arm == TMA_POWER ) &&
                     m_object->GetCargo()->Implements(ObjectInterfaceType::PowerContainer) )
                {
                    m_sound->Play(SOUND_POWEROFF, m_object->GetPosition());
                }
                m_arm = TMA_STOCK;
                InitAngle();
                SoundManip(1.0f/m_speed);
            }
            return ERR_CONTINUE;
        }
    }

    if ( m_order == TMO_DROP )
    {
        if ( m_step == 1 )
        {
            if ( m_bSubm )  m_speed = 1.0f/0.7f;
            cargo = m_object->GetCargo();
            if (TransporterDeposeObject())
            {
                if ( (m_arm == TMA_OTHER ||
                      m_arm == TMA_POWER ) &&
                     cargo->Implements(ObjectInterfaceType::PowerContainer) )
                {
                    m_sound->Play(SOUND_POWERON, m_object->GetPosition());
                }
                if (cargo != nullptr && m_cargoType == OBJECT_METAL && m_arm == TMA_FFRONT)
                {
                    m_main->ShowDropZone(cargo, m_object);  // shows buildable area
                }
                m_hand = TMH_OPEN;  // opens the clamp to deposit
                SoundManip(1.0f/m_speed, 0.8f, 1.5f);
            }
            InitAngle();
            return ERR_CONTINUE;
        }
        if ( m_step == 2 )
        {
            if ( m_bSubm )  m_speed = 1.0f/1.5f;
            m_arm = TMA_NEUTRAL;
            InitAngle();
            SoundManip(1.0f/m_speed);
            return ERR_CONTINUE;
        }
    }

    Abort();
    return ERR_STOP;
}

// Suddenly ends the current action.

bool CTaskManip::Abort()
{
    if (m_object->GetCargo() == nullptr)  // not carrying anything?
    {
        m_hand = TMH_OPEN;  // open clamp
        m_arm = TMA_NEUTRAL;
    }
    else
    {
        m_hand = TMH_CLOSE;  // closed clamp
        m_arm = TMA_STOCK;
    }
    InitAngle();

    if ( !m_bSubm )
    {
        for (int i = 0; i < 5; i++)
        {
            m_object->SetPartRotationZ(i+1, m_finalAngle[i]);
        }
    }

    m_camera->StopCentering(m_object, 2.0f);
    m_physics->SetFreeze(false);  // is moving again
    return true;
}


// Seeks the object below to take (for bees).

CObject* CTaskManip::SearchTakeUnderObject(Math::Vector &pos, float dLimit)
{
    CObject    *pBest;
    Math::Vector    iPos, oPos;
    float       min, distance;

    iPos   = m_object->GetPosition();

    min = 1000000.0f;
    pBest = nullptr;
    for (CObject* pObj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        if ( !pObj->Implements(ObjectInterfaceType::Transportable) )  continue;

        if (IsObjectBeingTransported(pObj))  continue;
        if ( pObj->GetLock() )  continue;
        if ( pObj->GetScaleY() != 1.0f )  continue;

        oPos = pObj->GetPosition();
        distance = Math::Distance(oPos, iPos);
        if ( distance <= dLimit &&
             distance < min     )
        {
            min = distance;
            pBest = pObj;
        }
    }
    if ( pBest != nullptr )
    {
        pos = pBest->GetPosition();
    }
    return pBest;
}

// Seeks the object in front to take.

CObject* CTaskManip::SearchTakeFrontObject(bool bAdvance, Math::Vector &pos,
                                           float &distance, float &angle)
{
    CObject     *pBest;
    Math::Vector    iPos, oPos;
    float       min, iAngle, bAngle, aLimit, dLimit, f;

    iPos   = m_object->GetPosition();
    iAngle = m_object->GetRotationY();
    iAngle = Math::NormAngle(iAngle);  // 0..2*Math::PI

    if ( bAdvance && m_energy > 0.0f )
    {
        aLimit = 60.0f*Math::PI/180.0f;
        dLimit = MARGIN_FRONT+10.0f;
    }
    else
    {
//?     aLimit = 7.0f*Math::PI/180.0f;
        aLimit = 15.0f*Math::PI/180.0f;  //OK 1.9
        dLimit = MARGIN_FRONT;
    }

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
        distance = fabs(Math::Distance(oPos, iPos)-TAKE_DIST);
        f = 1.0f-distance/50.0f;
        if ( f < 0.5f )  f = 0.5f;

        angle = Math::RotateAngle(oPos.x-iPos.x, iPos.z-oPos.z);  // CW !
        if ( !Math::TestAngle(angle, iAngle-aLimit*f, iAngle+aLimit*f) )  continue;

        if ( distance < -dLimit ||
             distance >  dLimit )  continue;

        if ( distance < min )
        {
            min = distance;
            pBest = pObj;
            bAngle = angle;
        }
    }
    if ( pBest == nullptr )
    {
        distance = 1000000.0f;
        angle = 0.0f;
    }
    else
    {
        pos = pBest->GetPosition();
        distance = min;
        angle = bAngle;
    }
    return pBest;
}

// Seeks the object back to take.

CObject* CTaskManip::SearchTakeBackObject(bool bAdvance, Math::Vector &pos,
                                          float &distance, float &angle)
{
    CObject     *pBest;
    Math::Vector    iPos, oPos;
    float       min, iAngle, bAngle, aLimit, dLimit, f;

    iPos   = m_object->GetPosition();
    iAngle = m_object->GetRotationY()+Math::PI;
    iAngle = Math::NormAngle(iAngle);  // 0..2*Math::PI

    if ( bAdvance && m_energy > 0.0f )
    {
        aLimit = 60.0f*Math::PI/180.0f;
        dLimit = MARGIN_BACK+5.0f;
    }
    else
    {
        aLimit = 7.0f*Math::PI/180.0f;
        dLimit = MARGIN_BACK;
    }

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
        distance = fabs(Math::Distance(oPos, iPos)-TAKE_DIST);
        f = 1.0f-distance/50.0f;
        if ( f < 0.5f )  f = 0.5f;

        angle = Math::RotateAngle(oPos.x-iPos.x, iPos.z-oPos.z);  // CW !
        if ( !Math::TestAngle(angle, iAngle-aLimit*f, iAngle+aLimit*f) )  continue;

        if ( distance < -dLimit ||
             distance >  dLimit )  continue;

        if ( distance < min )
        {
            min = distance;
            pBest = pObj;
            bAngle = angle;
        }
    }
    if ( pBest == nullptr )
    {
        distance = 1000000.0f;
        angle = 0.0f;
    }
    else
    {
        pos = pBest->GetPosition();
        distance = min;
        angle = bAngle;
    }
    return pBest;
}

// Seeks the robot or building on which it wants to put a battery or or other object.

CObject* CTaskManip::SearchOtherObject(bool bAdvance, Math::Vector &pos,
                                       float &distance, float &angle,
                                       float &height)
{
    Math::Matrix*   mat;
    float       iAngle, oAngle, oLimit, aLimit, dLimit;

    distance = 1000000.0f;
    angle = 0.0f;

    if ( m_bSubm )  return nullptr;  // impossible with the submarine

    if (m_object->GetCrashSphereCount() == 0) return nullptr;

    Math::Vector iPos = m_object->GetFirstCrashSphere().sphere.pos;

    iAngle = m_object->GetRotationY();
    iAngle = Math::NormAngle(iAngle);  // 0..2*Math::PI

    if ( bAdvance && m_energy > 0.0f )
    {
        aLimit = 60.0f*Math::PI/180.0f;
        dLimit = MARGIN_FRIEND+10.0f;
    }
    else
    {
        aLimit = 7.0f*Math::PI/180.0f;
        dLimit = MARGIN_FRIEND;
    }

    for (CObject* pObj : CObjectManager::GetInstancePointer()->GetAllObjects())
    {
        if ( pObj == m_object )  continue;  // yourself?

        ObjectType type = pObj->GetType();
        if ( !pObj->Implements(ObjectInterfaceType::Powered) )  continue;

        CObject* power = dynamic_cast<CPoweredObject*>(pObj)->GetPower();
        if (power != nullptr)
        {
            if (power->GetLock())  continue;
            if (power->GetScaleY() != 1.0f)  continue;
        }

        mat = pObj->GetWorldMatrix(0);
        Math::Vector oPos = Transform(*mat, dynamic_cast<CPoweredObject*>(pObj)->GetPowerPosition());

        oAngle = pObj->GetRotationY();
        if ( type == OBJECT_TOWER    ||
             type == OBJECT_RESEARCH )
        {
            oLimit = 45.0f*Math::PI/180.0f;
        }
        else if ( type == OBJECT_ENERGY )
        {
            oLimit = 90.0f*Math::PI/180.0f;
        }
        else if ( type == OBJECT_LABO )
        {
            oLimit = 120.0f*Math::PI/180.0f;
        }
        else if ( type == OBJECT_NUCLEAR )
        {
            oLimit = 45.0f*Math::PI/180.0f;
        }
        else
        {
            oLimit = 45.0f*Math::PI/180.0f;
            oAngle += Math::PI;  // is behind
        }
        oAngle = Math::NormAngle(oAngle);  // 0..2*Math::PI
        angle = Math::RotateAngle(iPos.x-oPos.x, oPos.z-iPos.z);  // CW !
        if ( !Math::TestAngle(angle, oAngle-oLimit, oAngle+oLimit) )  continue;

        distance = fabs(Math::Distance(oPos, iPos)-TAKE_DIST);
        if ( distance <= dLimit )
        {
            angle = Math::RotateAngle(oPos.x-iPos.x, iPos.z-oPos.z);  // CW !
            if ( Math::TestAngle(angle, iAngle-aLimit, iAngle+aLimit) )
            {
                Math::Vector powerPos = dynamic_cast<CPoweredObject*>(pObj)->GetPowerPosition();
                height = powerPos.y;
                pos = oPos;
                return pObj;
            }
        }
    }

    distance = 1000000.0f;
    angle = 0.0f;
    return nullptr;
}

// Takes the object placed in front.

bool CTaskManip::TransporterTakeObject()
{
    if (m_arm == TMA_GRAB)  // takes immediately?
    {
        CObject* cargo = m_object->GetCargo();
        if (cargo == nullptr)  return false;  // nothing to take?
        assert(cargo->Implements(ObjectInterfaceType::Transportable));

        m_cargoType = cargo->GetType();

        if ( m_object->GetType() == OBJECT_HUMAN ||
             m_object->GetType() == OBJECT_TECH  )
        {
            dynamic_cast<CTransportableObject*>(cargo)->SetTransporter(m_object);
            dynamic_cast<CTransportableObject*>(cargo)->SetTransporterPart(4);  // takes with the hand

            cargo->SetPosition(Math::Vector(1.7f, -0.5f, 1.1f));
            cargo->SetRotationY(0.1f);
            cargo->SetRotationX(0.0f);
            cargo->SetRotationZ(0.8f);
        }
        else if ( m_bSubm )
        {
            dynamic_cast<CTransportableObject*>(cargo)->SetTransporter(m_object);
            dynamic_cast<CTransportableObject*>(cargo)->SetTransporterPart(2);  // takes with the right claw

            Math::Vector pos = Math::Vector(1.1f, -1.0f, 1.0f);  // relative
            cargo->SetPosition(pos);
            cargo->SetRotationX(0.0f);
            cargo->SetRotationY(0.0f);
            cargo->SetRotationZ(0.0f);
        }
        else
        {
            dynamic_cast<CTransportableObject*>(cargo)->SetTransporter(m_object);
            dynamic_cast<CTransportableObject*>(cargo)->SetTransporterPart(3);  // takes with the hand

            Math::Vector pos = Math::Vector(4.7f, 0.0f, 0.0f);  // relative to the hand (lem4)
            cargo->SetPosition(pos);
            cargo->SetRotationX(0.0f);
            cargo->SetRotationZ(Math::PI/2.0f);
            cargo->SetRotationY(0.0f);
        }

        m_object->SetCargo(cargo);  // takes
    }

    if (m_arm == TMA_FFRONT)  // takes on the ground in front?
    {
        Math::Vector pos;
        float dist = 0.0f, angle = 0.0f;
        CObject* cargo = SearchTakeFrontObject(false, pos, dist, angle);
        if (cargo == nullptr)  return false;  // nothing to take?
        assert(cargo->Implements(ObjectInterfaceType::Transportable));

        m_cargoType = cargo->GetType();

        if ( m_bSubm )
        {
            dynamic_cast<CTransportableObject*>(cargo)->SetTransporter(m_object);
            dynamic_cast<CTransportableObject*>(cargo)->SetTransporterPart(2);  // takes with the right claw

            pos = Math::Vector(1.1f, -1.0f, 1.0f);  // relative
            cargo->SetPosition(pos);
            cargo->SetRotationX(0.0f);
            cargo->SetRotationY(0.0f);
            cargo->SetRotationZ(0.0f);
        }
        else
        {
            dynamic_cast<CTransportableObject*>(cargo)->SetTransporter(m_object);
            dynamic_cast<CTransportableObject*>(cargo)->SetTransporterPart(3);  // takes with the hand

            pos = Math::Vector(4.7f, 0.0f, 0.0f);  // relative to the hand (lem4)
            cargo->SetPosition(pos);
            cargo->SetRotationX(0.0f);
            cargo->SetRotationZ(Math::PI/2.0f);
            cargo->SetRotationY(0.0f);
        }

        m_object->SetCargo(cargo);  // takes
    }

    if (m_arm == TMA_FBACK)  // takes on the ground behind?
    {
        Math::Vector pos;
        float dist = 0.0f, angle = 0.0f;
        CObject* cargo = SearchTakeBackObject(false, pos, dist, angle);
        if (cargo == nullptr) return false;  // nothing to take?
        assert(cargo->Implements(ObjectInterfaceType::Transportable));

        m_cargoType = cargo->GetType();

        dynamic_cast<CTransportableObject*>(cargo)->SetTransporter(m_object);
        dynamic_cast<CTransportableObject*>(cargo)->SetTransporterPart(3);  // takes with the hand

        pos = Math::Vector(4.7f, 0.0f, 0.0f);  // relative to the hand (lem4)
        cargo->SetPosition(pos);
        cargo->SetRotationX(0.0f);
        cargo->SetRotationZ(Math::PI/2.0f);
        cargo->SetRotationY(0.0f);

        m_object->SetCargo(cargo);  // takes
    }

    if (m_arm == TMA_POWER)  // takes battery in the back?
    {
        assert(m_object->Implements(ObjectInterfaceType::Powered));
        CObject* cargo = m_object->GetPower();
        if (cargo == nullptr)  return false;  // no battery?
        assert(cargo->Implements(ObjectInterfaceType::Transportable));

        m_cargoType = cargo->GetType();

        Math::Vector pos = Math::Vector(4.7f, 0.0f, 0.0f);  // relative to the hand (lem4)
        cargo->SetPosition(pos);
        cargo->SetRotationX(0.0f);
        cargo->SetRotationZ(Math::PI/2.0f);
        cargo->SetRotationY(0.0f);
        dynamic_cast<CTransportableObject*>(cargo)->SetTransporterPart(3);  // takes with the hand

        m_object->SetPower(nullptr);
        m_object->SetCargo(cargo);  // takes
    }

    if (m_arm == TMA_OTHER)  // battery takes from friend?
    {
        Math::Vector pos;
        float dist = 0.0f, angle = 0.0f;
        CObject* other = SearchOtherObject(false, pos, dist, angle, m_height);
        if (other == nullptr)  return false;
        assert(other->Implements(ObjectInterfaceType::Powered));

        CObject* cargo = dynamic_cast<CPoweredObject*>(other)->GetPower();
        if (cargo == nullptr)  return false;  // the other does not have a battery?
        assert(cargo->Implements(ObjectInterfaceType::Transportable));

        m_cargoType = cargo->GetType();

        dynamic_cast<CPoweredObject*>(other)->SetPower(nullptr);
        dynamic_cast<CTransportableObject*>(cargo)->SetTransporter(m_object);
        dynamic_cast<CTransportableObject*>(cargo)->SetTransporterPart(3);  // takes with the hand

        pos = Math::Vector(4.7f, 0.0f, 0.0f);  // relative to the hand (lem4)
        cargo->SetPosition(pos);
        cargo->SetRotationX(0.0f);
        cargo->SetRotationZ(Math::PI/2.0f);
        cargo->SetRotationY(0.0f);

        m_object->SetCargo(cargo);  // takes
    }

    return true;
}

// Deposes the object taken.

bool CTaskManip::TransporterDeposeObject()
{
    if (m_arm == TMA_FFRONT)  // deposits on the ground in front?
    {
        CObject* cargo = m_object->GetCargo();
        if (cargo == nullptr)  return false;  // nothing transported?
        assert(cargo->Implements(ObjectInterfaceType::Transportable));

        m_cargoType = cargo->GetType();

        Math::Matrix* mat = cargo->GetWorldMatrix(0);
        Math::Vector pos = Transform(*mat, Math::Vector(0.0f, 1.0f, 0.0f));
        m_terrain->AdjustToFloor(pos);
        cargo->SetPosition(pos);
        cargo->SetRotationY(m_object->GetRotationY()+Math::PI/2.0f);
        cargo->SetRotationX(0.0f);
        cargo->SetRotationZ(0.0f);
        cargo->FloorAdjust();  // plate well on the ground

        dynamic_cast<CTransportableObject*>(cargo)->SetTransporter(nullptr);
        m_object->SetCargo(nullptr);  // deposit
    }

    if (m_arm == TMA_FBACK)  // deposited on the ground behind?
    {
        CObject* cargo = m_object->GetCargo();
        if (cargo == nullptr)  return false;  // nothing transported?
        assert(cargo->Implements(ObjectInterfaceType::Transportable));

        m_cargoType = cargo->GetType();

        Math::Matrix* mat = cargo->GetWorldMatrix(0);
        Math::Vector pos = Transform(*mat, Math::Vector(0.0f, 1.0f, 0.0f));
        m_terrain->AdjustToFloor(pos);
        cargo->SetPosition(pos);
        cargo->SetRotationY(m_object->GetRotationY()+Math::PI/2.0f);
        cargo->SetRotationX(0.0f);
        cargo->SetRotationZ(0.0f);

        dynamic_cast<CTransportableObject*>(cargo)->SetTransporter(nullptr);
        m_object->SetCargo(nullptr);  // deposit
    }

    if (m_arm == TMA_POWER)  // deposits battery in the back?
    {
        assert(m_object->Implements(ObjectInterfaceType::Powered));
        CObject* cargo = m_object->GetCargo();
        if (cargo == nullptr)  return false;  // nothing transported?
        assert(cargo->Implements(ObjectInterfaceType::Transportable));

        m_cargoType = cargo->GetType();

        if (m_object->GetPower() != nullptr)  return false;

        dynamic_cast<CTransportableObject*>(cargo)->SetTransporter(m_object);
        dynamic_cast<CTransportableObject*>(cargo)->SetTransporterPart(0);  // carried by the base

        cargo->SetPosition(m_object->GetPowerPosition());
        cargo->SetRotationY(0.0f);
        cargo->SetRotationX(0.0f);
        cargo->SetRotationZ(0.0f);

        m_object->SetPower(cargo);  // uses
        m_object->SetCargo(nullptr);
    }

    if (m_arm == TMA_OTHER)  // deposits battery on friend?
    {
        Math::Vector pos;
        float angle = 0.0f, dist = 0.0f;

        CObject* other = SearchOtherObject(false, pos, dist, angle, m_height);
        if (other == nullptr)  return false;
        assert(other->Implements(ObjectInterfaceType::Powered));

        CObject* cargo = dynamic_cast<CPoweredObject*>(other)->GetPower();
        if (cargo != nullptr)  return false;  // the other already has a battery?

        cargo = m_object->GetCargo();
        if (cargo == nullptr)  return false;
        assert(cargo->Implements(ObjectInterfaceType::Transportable));

        m_cargoType = cargo->GetType();

        dynamic_cast<CPoweredObject*>(other)->SetPower(cargo);
        dynamic_cast<CTransportableObject*>(cargo)->SetTransporter(other);

        cargo->SetPosition(dynamic_cast<CPoweredObject*>(other)->GetPowerPosition());
        cargo->SetRotationY(0.0f);
        cargo->SetRotationX(0.0f);
        cargo->SetRotationZ(0.0f);
        dynamic_cast<CTransportableObject*>(cargo)->SetTransporterPart(0);  // carried by the base

        m_object->SetCargo(nullptr);  // deposit
    }

    return true;
}

// Seeks if a location allows to deposit an object.

bool CTaskManip::IsFreeDeposeObject(Math::Vector pos)
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
            if ( Math::Distance(iPos, crashSphere.sphere.pos)-(crashSphere.sphere.radius+1.0f) < 2.0f )
            {
                return false;  // location occupied
            }
        }
    }
    return true;  // location free
}

// Plays the sound of the manipulator arm.

void CTaskManip::SoundManip(float time, float amplitude, float frequency)
{
    int i = m_sound->Play(SOUND_MANIP, m_object->GetPosition(), 0.0f, 0.3f*frequency, true);
    m_sound->AddEnvelope(i, 0.5f*amplitude, 1.0f*frequency, 0.1f, SOPER_CONTINUE);
    m_sound->AddEnvelope(i, 0.5f*amplitude, 1.0f*frequency, time-0.1f, SOPER_CONTINUE);
    m_sound->AddEnvelope(i, 0.0f, 0.3f*frequency, 0.1f, SOPER_STOP);
}
