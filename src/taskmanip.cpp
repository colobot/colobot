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

// taskmanip.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "d3dengine.h"
#include "d3dmath.h"
#include "math3d.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "terrain.h"
#include "object.h"
#include "pyro.h"
#include "physics.h"
#include "brain.h"
#include "camera.h"
#include "sound.h"
#include "robotmain.h"
#include "task.h"
#include "taskmanip.h"


//?#define MARGIN_FRONT 2.0f
//?#define MARGIN_BACK      2.0f
//?#define MARGIN_FRIEND    2.0f
//?#define MARGIN_BEE       5.0f
#define MARGIN_FRONT    4.0f  //OK 1.9
#define MARGIN_BACK     4.0f  //OK 1.9
#define MARGIN_FRIEND   4.0f  //OK 1.9
#define MARGIN_BEE      5.0f  //OK 1.9




// Object's constructor.

CTaskManip::CTaskManip(CInstanceManager* iMan, CObject* object)
                       : CTask(iMan, object)
{
    CTask::CTask(iMan, object);

    m_arm  = TMA_NEUTRAL;
    m_hand = TMH_OPEN;
}

// Object's destructor.

CTaskManip::~CTaskManip()
{
}


// Management of an event.

BOOL CTaskManip::EventProcess(const Event &event)
{
    D3DVECTOR   pos;
    float       angle, a, g, cirSpeed, progress;
    int         i;

    if ( m_engine->RetPause() )  return TRUE;
    if ( event.event != EVENT_FRAME )  return TRUE;
    if ( m_bError )  return FALSE;

    if ( m_bBee )  // bee?
    {
        return TRUE;
    }

    if ( m_bTurn )  // preliminary rotation?
    {
        a = m_object->RetAngleY(0);
        g = m_angle;
        cirSpeed = Direction(a, g)*1.0f;
        if ( m_physics->RetType() == TYPE_FLYING )  // flying on the ground?
        {
            cirSpeed *= 4.0f;  // more fishing
        }
        if ( cirSpeed >  1.0f )  cirSpeed =  1.0f;
        if ( cirSpeed < -1.0f )  cirSpeed = -1.0f;

        m_physics->SetMotorSpeedZ(cirSpeed);  // turns left / right
        return TRUE;
    }

    if ( m_move != 0 )  // preliminary advance?
    {
        m_timeLimit -= event.rTime;
        m_physics->SetMotorSpeedX(m_move);  // forward/backward
        return TRUE;
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
                pos = m_object->RetPosition(1);
                pos.y = 3.0f-progress*2.0f;
                m_object->SetPosition(1, pos);
            }
            if ( m_step == 1 )  // farm?
            {
                pos = m_object->RetPosition(2);
                pos.z = -1.5f+progress*0.5f;
                m_object->SetPosition(2, pos);

                pos = m_object->RetPosition(3);
                pos.z = 1.5f-progress*0.5f;
                m_object->SetPosition(3, pos);
            }
            if ( m_step == 2 )  // up?
            {
                pos = m_object->RetPosition(1);
                pos.y = 3.0f-(1.0f-progress)*2.0f;
                m_object->SetPosition(1, pos);
            }
        }
        else
        {
            if ( m_step == 0 )  // fall?
            {
                pos = m_object->RetPosition(1);
                pos.y = 3.0f-progress*2.0f;
                m_object->SetPosition(1, pos);
            }
            if ( m_step == 1 )  // farm?
            {
                pos = m_object->RetPosition(2);
                pos.z = -1.5f+(1.0f-progress)*0.5f;
                m_object->SetPosition(2, pos);

                pos = m_object->RetPosition(3);
                pos.z = 1.5f-(1.0f-progress)*0.5f;
                m_object->SetPosition(3, pos);
            }
            if ( m_step == 2 )  // up?
            {
                pos = m_object->RetPosition(1);
                pos.y = 3.0f-(1.0f-progress)*2.0f;
                m_object->SetPosition(1, pos);
            }
        }
    }
    else
    {
        for ( i=0 ; i<5 ; i++ )
        {
            angle = (m_finalAngle[i]-m_initialAngle[i])*progress;
            angle += m_initialAngle[i];
            m_object->SetAngleZ(i+1, angle);
        }
    }

    return TRUE;
}


// Initializes the initial and final angles.

void CTaskManip::InitAngle()
{
    CObject*    power;
    float       max, energy;
    int         i;

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
        m_finalAngle[0] =   35.0f*PI/180.0f;  // arm
        m_finalAngle[1] =  -95.0f*PI/180.0f;  // forearm
        m_finalAngle[2] =  -27.0f*PI/180.0f;  // hand
    }
    if ( m_arm == TMA_FBACK )
    {
        m_finalAngle[0] =  145.0f*PI/180.0f;  // arm
        m_finalAngle[1] =   95.0f*PI/180.0f;  // forearm
        m_finalAngle[2] =   27.0f*PI/180.0f;  // hand
    }
    if ( m_arm == TMA_POWER )
    {
        m_finalAngle[0] =   95.0f*PI/180.0f;  // arm
        m_finalAngle[1] =  125.0f*PI/180.0f;  // forearm
        m_finalAngle[2] =   50.0f*PI/180.0f;  // hand
    }
    if ( m_arm == TMA_OTHER )
    {
        if ( m_height <= 3.0f )
        {
            m_finalAngle[0] =  55.0f*PI/180.0f;  // arm
            m_finalAngle[1] = -90.0f*PI/180.0f;  // forearm
            m_finalAngle[2] = -35.0f*PI/180.0f;  // hand
        }
        else
        {
            m_finalAngle[0] =  70.0f*PI/180.0f;  // arm
            m_finalAngle[1] = -90.0f*PI/180.0f;  // forearm
            m_finalAngle[2] = -50.0f*PI/180.0f;  // hand
        }
    }

    if ( m_hand == TMH_OPEN )  // open clamp?
    {
        m_finalAngle[3] = -PI*0.10f;  // clamp close
        m_finalAngle[4] =  PI*0.10f;  // clamp remote
    }
    if ( m_hand == TMH_CLOSE )  // clamp closed?
    {
        m_finalAngle[3] =  PI*0.05f;  // clamp close
        m_finalAngle[4] = -PI*0.05f;  // clamp remote
    }

    for ( i=0 ; i<5 ; i++ )
    {
        m_initialAngle[i] = m_object->RetAngleZ(i+1);
    }

    max = 0.0f;
    for ( i=0 ; i<5 ; i++ )
    {
        max = Max(max, Abs(m_initialAngle[i] - m_finalAngle[i]));
    }
    m_speed = (PI*1.0f)/max;
    if ( m_speed > 3.0f )  m_speed = 3.0f;  // piano, ma non troppo (?)

    energy = 0.0f;
    power = m_object->RetPower();
    if ( power != 0 )
    {
        energy = power->RetEnergy();
    }

    if ( energy == 0.0f )
    {
        m_speed *= 0.7f;  // slower if more energy!
    }
}


// Tests whether an object is compatible with the operation TMA_OTHER.

BOOL TestFriend(ObjectType oType, ObjectType fType)
{
    if ( oType == OBJECT_ENERGY )
    {
        return ( fType == OBJECT_METAL );
    }
    if ( oType == OBJECT_LABO )
    {
        return ( fType == OBJECT_BULLET );
    }
    if ( oType == OBJECT_NUCLEAR )
    {
        return ( fType == OBJECT_URANIUM );
    }

    return ( fType == OBJECT_POWER  ||
             fType == OBJECT_ATOMIC );
}

// Assigns the goal was achieved.

Error CTaskManip::Start(TaskManipOrder order, TaskManipArm arm)
{
    ObjectType  type;
    CObject     *front, *other, *power;
    CPyro       *pyro;
    float       iAngle, dist, len;
    float       fDist, fAngle, oDist, oAngle, oHeight;
    D3DVECTOR   pos, fPos, oPos;

    m_arm      = arm;
    m_height   = 0.0f;
    m_step     = 0;
    m_progress = 0.0f;
    m_speed    = 1.0f/1.5f;

    iAngle = m_object->RetAngleY(0);
    iAngle = NormAngle(iAngle);  // 0..2*PI
    oAngle = iAngle;

    m_bError = TRUE;  // operation impossible

    if ( m_arm != TMA_FFRONT &&
         m_arm != TMA_FBACK  &&
         m_arm != TMA_POWER  &&
         m_arm != TMA_GRAB   )  return ERR_MANIP_VEH;

    m_physics->SetMotorSpeed(D3DVECTOR(0.0f, 0.0f, 0.0f));

    type = m_object->RetType();
    if ( type == OBJECT_BEE )  // bee?
    {
        if ( m_object->RetFret() == 0 )
        {
            if ( !m_physics->RetLand() )  return ERR_MANIP_FLY;

            other = SearchTakeUnderObject(m_targetPos, MARGIN_BEE);
            if ( other == 0 )  return ERR_MANIP_NIL;
            m_object->SetFret(other);  // takes the ball
            other->SetTruck(m_object);
            other->SetTruckPart(0);  // taken with the base
            other->SetPosition(0, D3DVECTOR(0.0f, -3.0f, 0.0f));
        }
        else
        {
            other = m_object->RetFret();  // other = ball
            m_object->SetFret(0);  // lick the ball
            other->SetTruck(0);
            pos = m_object->RetPosition(0);
            pos.y -= 3.0f;
            other->SetPosition(0, pos);

            pos = m_object->RetPosition(0);
            pos.y += 2.0f;
            m_object->SetPosition(0, pos);  // against the top of jump

            pyro = new CPyro(m_iMan);
            pyro->Create(PT_FALL, other);  // the ball falls
        }

        m_bBee = TRUE;
        m_bError = FALSE;  // ok
        return ERR_OK;
    }
    m_bBee = FALSE;

    m_bSubm = ( type == OBJECT_MOBILEsa );  // submarine?

    if ( m_arm == TMA_GRAB )  // takes immediately?
    {
        TruckTakeObject();
        Abort();
        return ERR_OK;
    }

    m_energy = 0.0f;
    power = m_object->RetPower();
    if ( power != 0 )
    {
        m_energy = power->RetEnergy();
    }

    if ( !m_physics->RetLand() )  return ERR_MANIP_FLY;

    if ( type != OBJECT_MOBILEfa &&
         type != OBJECT_MOBILEta &&
         type != OBJECT_MOBILEwa &&
         type != OBJECT_MOBILEia &&
         type != OBJECT_MOBILEsa )  return ERR_MANIP_VEH;

    if ( m_bSubm )  // submarine?
    {
        m_arm = TMA_FFRONT;  // only possible in front!
    }

    m_move = 0.0f;  // advance not necessary
    m_angle = iAngle;

    if ( order == TMO_AUTO )
    {
        if ( m_object->RetFret() == 0 )
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

    if ( m_order == TMO_GRAB && m_object->RetFret() != 0 )
    {
        return ERR_MANIP_BUSY;
    }
    if ( m_order == TMO_DROP && m_object->RetFret() == 0 )
    {
        return ERR_MANIP_EMPTY;
    }

//? speed = m_physics->RetMotorSpeed();
//? if ( speed.x != 0.0f ||
//?      speed.z != 0.0f )  return ERR_MANIP_MOTOR;

    if ( m_order == TMO_GRAB )
    {
        if ( m_arm == TMA_FFRONT )
        {
            front = SearchTakeFrontObject(TRUE, fPos, fDist, fAngle);
            other = SearchOtherObject(TRUE, oPos, oDist, oAngle, oHeight);

            if ( front != 0 && fDist < oDist )
            {
                m_targetPos = fPos;
                m_angle = fAngle;
                m_move = 1.0f;  // advance required
            }
            else if ( other != 0 && oDist < fDist )
            {
                if ( other->RetPower() == 0 )  return ERR_MANIP_NIL;
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
            if ( SearchTakeBackObject(TRUE, m_targetPos, fDist, m_angle) == 0 )
            {
                return ERR_MANIP_NIL;
            }
            m_angle += PI;
            m_move = -1.0f;  // back necessary
        }
        if ( m_arm == TMA_POWER )
        {
            if ( m_object->RetPower() == 0 )  return ERR_MANIP_NIL;
        }
    }

    if ( m_order == TMO_DROP )
    {
        if ( m_arm == TMA_FFRONT )
        {
            other = SearchOtherObject(TRUE, oPos, oDist, oAngle, oHeight);
            if ( other != 0 && other->RetPower() == 0 )
            {
                m_targetPos = oPos;
                m_angle = oAngle;
                m_height = oHeight;
                m_move = 1.0f;  // advance required
                m_arm = TMA_OTHER;
            }
            else
            {
                if ( !IsFreeDeposeObject(D3DVECTOR(TAKE_DIST, 0.0f, 0.0f)) )  return ERR_MANIP_OCC;
            }
        }
        if ( m_arm == TMA_FBACK )
        {
            if ( !IsFreeDeposeObject(D3DVECTOR(-TAKE_DIST, 0.0f, 0.0f)) )  return ERR_MANIP_OCC;
        }
        if ( m_arm == TMA_POWER )
        {
            if ( m_object->RetPower() != 0 )  return ERR_MANIP_OCC;
        }
    }

    dist = Length(m_object->RetPosition(0), m_targetPos);
    len = dist-TAKE_DIST;
    if ( m_arm == TMA_OTHER ) len -= TAKE_DIST_OTHER;
    if ( len < 0.0f )  len = 0.0f;
    if ( m_arm == TMA_FBACK ) len = -len;
    m_advanceLength = dist-m_physics->RetLinLength(len);
    if ( dist <= m_advanceLength+0.2f )  m_move = 0.0f;  // not necessary to advance

    if ( m_energy == 0.0f )  m_move = 0.0f;

    if ( m_move != 0.0f )  // forward or backward?
    {
        m_timeLimit = m_physics->RetLinTimeLength(Abs(len))*1.5f;
        if ( m_timeLimit < 0.5f )  m_timeLimit = 0.5f;
    }

    if ( m_object->RetFret() == 0 )  // not carrying anything?
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
        m_bTurn = FALSE;  // preliminary rotation unnecessary
        SoundManip(1.0f/m_speed);
    }
    else
    {
        m_bTurn = TRUE;  // preliminary rotation necessary
    }

    if ( m_bSubm )
    {
        m_camera->StartCentering(m_object, PI*0.8f, 99.9f, 0.0f, 0.5f);
    }

    m_physics->SetFreeze(TRUE);  // it does not move

    m_bError = FALSE;  // ok
    return ERR_OK;
}

// Indicates whether the action is complete.

Error CTaskManip::IsEnded()
{
    CObject*    fret;
    D3DVECTOR   pos;
    float       angle, dist;
    int         i;

    if ( m_engine->RetPause() )  return ERR_CONTINUE;
    if ( m_bError )  return ERR_STOP;

    if ( m_bBee )  // bee?
    {
        return ERR_STOP;
    }

    if ( m_bTurn )  // preliminary rotation?
    {
        angle = m_object->RetAngleY(0);
        angle = NormAngle(angle);  // 0..2*PI

        if ( TestAngle(angle, m_angle-PI*0.01f, m_angle+PI*0.01f) )
        {
            m_bTurn = FALSE;  // rotation ended
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
            dist = Length(m_object->RetPosition(0), m_targetPos);
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

        dist = Length(m_object->RetPosition(0), m_targetPos);
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
            m_object->SetAngleZ(i+1, m_finalAngle[i]);
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
            if ( !TruckTakeObject() &&
                 m_object->RetFret() == 0 )
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
                     (m_fretType == OBJECT_POWER  ||
                      m_fretType == OBJECT_ATOMIC ) )
                {
                    m_sound->Play(SOUND_POWEROFF, m_object->RetPosition(0));
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
            fret = m_object->RetFret();
            if ( TruckDeposeObject() )
            {
                if ( (m_arm == TMA_OTHER ||
                      m_arm == TMA_POWER ) &&
                     (m_fretType == OBJECT_POWER  ||
                      m_fretType == OBJECT_ATOMIC ) )
                {
                    m_sound->Play(SOUND_POWERON, m_object->RetPosition(0));
                }
                if ( fret != 0 && m_fretType == OBJECT_METAL && m_arm == TMA_FFRONT )
                {
                    m_main->ShowDropZone(fret, m_object);  // shows buildable area
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

BOOL CTaskManip::Abort()
{
    int     i;

    if ( m_object->RetFret() == 0 )  // not carrying anything?
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
        for ( i=0 ; i<5 ; i++ )
        {
            m_object->SetAngleZ(i+1, m_finalAngle[i]);
        }
    }

    m_camera->StopCentering(m_object, 2.0f);
    m_physics->SetFreeze(FALSE);  // is moving again
    return TRUE;
}


// Seeks the object below to take (for bees).

CObject* CTaskManip::SearchTakeUnderObject(D3DVECTOR &pos, float dLimit)
{
    CObject     *pObj, *pBest;
    D3DVECTOR   iPos, oPos;
    ObjectType  type;
    float       min, distance;
    int         i;

    iPos   = m_object->RetPosition(0);

    min = 1000000.0f;
    pBest = 0;
    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        type = pObj->RetType();

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

        if ( pObj->RetTruck() != 0 )  continue;  // object transported?
        if ( pObj->RetLock() )  continue;
        if ( pObj->RetZoomY(0) != 1.0f )  continue;

        oPos = pObj->RetPosition(0);
        distance = Length(oPos, iPos);
        if ( distance <= dLimit &&
             distance < min     )
        {
            min = distance;
            pBest = pObj;
        }
    }
    if ( pBest != 0 )
    {
        pos = pBest->RetPosition(0);
    }
    return pBest;
}

// Seeks the object in front to take.

CObject* CTaskManip::SearchTakeFrontObject(BOOL bAdvance, D3DVECTOR &pos,
                                           float &distance, float &angle)
{
    CObject     *pObj, *pBest;
    D3DVECTOR   iPos, oPos;
    ObjectType  type;
    float       min, iAngle, bAngle, aLimit, dLimit, f;
    int         i;

    iPos   = m_object->RetPosition(0);
    iAngle = m_object->RetAngleY(0);
    iAngle = NormAngle(iAngle);  // 0..2*PI

    if ( bAdvance && m_energy > 0.0f )
    {
        aLimit = 60.0f*PI/180.0f;
        dLimit = MARGIN_FRONT+10.0f;
    }
    else
    {
//?     aLimit = 7.0f*PI/180.0f;
        aLimit = 15.0f*PI/180.0f;  //OK 1.9
        dLimit = MARGIN_FRONT;
    }

    min = 1000000.0f;
    pBest = 0;
    bAngle = 0.0f;
    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        type = pObj->RetType();

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
             type != OBJECT_TNT     &&
             type != OBJECT_SCRAP1  &&
             type != OBJECT_SCRAP2  &&
             type != OBJECT_SCRAP3  &&
             type != OBJECT_SCRAP4  &&
             type != OBJECT_SCRAP5  )  continue;

        if ( pObj->RetTruck() != 0 )  continue;  // object transported?
        if ( pObj->RetLock() )  continue;
        if ( pObj->RetZoomY(0) != 1.0f )  continue;

        oPos = pObj->RetPosition(0);
        distance = Abs(Length(oPos, iPos)-TAKE_DIST);
        f = 1.0f-distance/50.0f;
        if ( f < 0.5f )  f = 0.5f;

        angle = RotateAngle(oPos.x-iPos.x, iPos.z-oPos.z);  // CW !
        if ( !TestAngle(angle, iAngle-aLimit*f, iAngle+aLimit*f) )  continue;

        if ( distance < -dLimit ||
             distance >  dLimit )  continue;

        if ( distance < min )
        {
            min = distance;
            pBest = pObj;
            bAngle = angle;
        }
    }
    if ( pBest == 0 )
    {
        distance = 1000000.0f;
        angle = 0.0f;
    }
    else
    {
        pos = pBest->RetPosition(0);
        distance = min;
        angle = bAngle;
    }
    return pBest;
}

// Seeks the object back to take.

CObject* CTaskManip::SearchTakeBackObject(BOOL bAdvance, D3DVECTOR &pos,
                                          float &distance, float &angle)
{
    CObject     *pObj, *pBest;
    D3DVECTOR   iPos, oPos;
    ObjectType  type;
    float       min, iAngle, bAngle, aLimit, dLimit, f;
    int         i;

    iPos   = m_object->RetPosition(0);
    iAngle = m_object->RetAngleY(0)+PI;
    iAngle = NormAngle(iAngle);  // 0..2*PI

    if ( bAdvance && m_energy > 0.0f )
    {
        aLimit = 60.0f*PI/180.0f;
        dLimit = MARGIN_BACK+5.0f;
    }
    else
    {
        aLimit = 7.0f*PI/180.0f;
        dLimit = MARGIN_BACK;
    }

    min = 1000000.0f;
    pBest = 0;
    bAngle = 0.0f;
    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        type = pObj->RetType();

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
             type != OBJECT_TNT     &&
             type != OBJECT_SCRAP1  &&
             type != OBJECT_SCRAP2  &&
             type != OBJECT_SCRAP3  &&
             type != OBJECT_SCRAP4  &&
             type != OBJECT_SCRAP5  )  continue;

        if ( pObj->RetTruck() != 0 )  continue;  // object transported?
        if ( pObj->RetLock() )  continue;
        if ( pObj->RetZoomY(0) != 1.0f )  continue;

        oPos = pObj->RetPosition(0);
        distance = Abs(Length(oPos, iPos)-TAKE_DIST);
        f = 1.0f-distance/50.0f;
        if ( f < 0.5f )  f = 0.5f;

        angle = RotateAngle(oPos.x-iPos.x, iPos.z-oPos.z);  // CW !
        if ( !TestAngle(angle, iAngle-aLimit*f, iAngle+aLimit*f) )  continue;

        if ( distance < -dLimit ||
             distance >  dLimit )  continue;

        if ( distance < min )
        {
            min = distance;
            pBest = pObj;
            bAngle = angle;
        }
    }
    if ( pBest == 0 )
    {
        distance = 1000000.0f;
        angle = 0.0f;
    }
    else
    {
        pos = pBest->RetPosition(0);
        distance = min;
        angle = bAngle;
    }
    return pBest;
}

// Seeks the robot or building on which it wants to put a battery or or other object.

CObject* CTaskManip::SearchOtherObject(BOOL bAdvance, D3DVECTOR &pos,
                                       float &distance, float &angle,
                                       float &height)
{
    Character*  character;
    CObject*    pObj;
    CObject*    pPower;
    D3DMATRIX*  mat;
    D3DVECTOR   iPos, oPos;
    ObjectType  type, powerType;
    float       iAngle, iRad, oAngle, oLimit, aLimit, dLimit;
    int         i;

    distance = 1000000.0f;
    angle = 0.0f;

    if ( m_bSubm )  return 0;  // impossible with the submarine

    if ( !m_object->GetCrashSphere(0, iPos, iRad) )  return 0;
    iAngle = m_object->RetAngleY(0);
    iAngle = NormAngle(iAngle);  // 0..2*PI

    if ( bAdvance && m_energy > 0.0f )
    {
        aLimit = 60.0f*PI/180.0f;
        dLimit = MARGIN_FRIEND+10.0f;
    }
    else
    {
        aLimit = 7.0f*PI/180.0f;
        dLimit = MARGIN_FRIEND;
    }

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        if ( pObj == m_object )  continue;  // yourself?

        type = pObj->RetType();
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

        pPower = pObj->RetPower();
        if ( pPower != 0 )
        {
            if ( pPower->RetLock() )  continue;
            if ( pPower->RetZoomY(0) != 1.0f )  continue;

            powerType = pPower->RetType();
            if ( powerType == OBJECT_NULL ||
                 powerType == OBJECT_FIX  )  continue;
        }

        mat = pObj->RetWorldMatrix(0);
        character = pObj->RetCharacter();
        oPos = Transform(*mat, character->posPower);

        oAngle = pObj->RetAngleY(0);
        if ( type == OBJECT_TOWER    ||
             type == OBJECT_RESEARCH )
        {
            oLimit = 45.0f*PI/180.0f;
        }
        else if ( type == OBJECT_ENERGY )
        {
            oLimit = 90.0f*PI/180.0f;
        }
        else if ( type == OBJECT_LABO )
        {
            oLimit = 120.0f*PI/180.0f;
        }
        else if ( type == OBJECT_NUCLEAR )
        {
            oLimit = 45.0f*PI/180.0f;
        }
        else
        {
            oLimit = 45.0f*PI/180.0f;
            oAngle += PI;  // is behind
        }
        oAngle = NormAngle(oAngle);  // 0..2*PI
        angle = RotateAngle(iPos.x-oPos.x, oPos.z-iPos.z);  // CW !
        if ( !TestAngle(angle, oAngle-oLimit, oAngle+oLimit) )  continue;

        distance = Abs(Length(oPos, iPos)-TAKE_DIST);
        if ( distance <= dLimit )
        {
            angle = RotateAngle(oPos.x-iPos.x, iPos.z-oPos.z);  // CW !
            if ( TestAngle(angle, iAngle-aLimit, iAngle+aLimit) )
            {
                character = pObj->RetCharacter();
                height = character->posPower.y;
                pos = oPos;
                return pObj;
            }
        }
    }

    distance = 1000000.0f;
    angle = 0.0f;
    return 0;
}

// Takes the object placed in front.

BOOL CTaskManip::TruckTakeObject()
{
    CObject*    fret;
    CObject*    other;
    D3DMATRIX   matRotate;
    D3DVECTOR   pos;
    float       angle, dist;

    if ( m_arm == TMA_GRAB )  // takes immediately?
    {
        fret = m_object->RetFret();
        if ( fret == 0 )  return FALSE;  // nothing to take?
        m_fretType = fret->RetType();

        if ( m_object->RetType() == OBJECT_HUMAN ||
             m_object->RetType() == OBJECT_TECH  )
        {
            fret->SetTruck(m_object);
            fret->SetTruckPart(4);  // takes with the hand

            fret->SetPosition(0, D3DVECTOR(1.7f, -0.5f, 1.1f));
            fret->SetAngleY(0, 0.1f);
            fret->SetAngleX(0, 0.0f);
            fret->SetAngleZ(0, 0.8f);
        }
        else if ( m_bSubm )
        {
            fret->SetTruck(m_object);
            fret->SetTruckPart(2);  // takes with the right claw

            pos = D3DVECTOR(1.1f, -1.0f, 1.0f);  // relative
            fret->SetPosition(0, pos);
            fret->SetAngleX(0, 0.0f);
            fret->SetAngleY(0, 0.0f);
            fret->SetAngleZ(0, 0.0f);
        }
        else
        {
            fret->SetTruck(m_object);
            fret->SetTruckPart(3);  // takes with the hand

            pos = D3DVECTOR(4.7f, 0.0f, 0.0f);  // relative to the hand (lem4)
            fret->SetPosition(0, pos);
            fret->SetAngleX(0, 0.0f);
            fret->SetAngleZ(0, PI/2.0f);
            fret->SetAngleY(0, 0.0f);
        }

        m_object->SetFret(fret);  // takes
    }

    if ( m_arm == TMA_FFRONT )  // takes on the ground in front?
    {
        fret = SearchTakeFrontObject(FALSE, pos, dist, angle);
        if ( fret == 0 )  return FALSE;  // nothing to take?
        m_fretType = fret->RetType();

        if ( m_bSubm )
        {
            fret->SetTruck(m_object);
            fret->SetTruckPart(2);  // takes with the right claw

            pos = D3DVECTOR(1.1f, -1.0f, 1.0f);  // relative
            fret->SetPosition(0, pos);
            fret->SetAngleX(0, 0.0f);
            fret->SetAngleY(0, 0.0f);
            fret->SetAngleZ(0, 0.0f);
        }
        else
        {
            fret->SetTruck(m_object);
            fret->SetTruckPart(3);  // takes with the hand

            pos = D3DVECTOR(4.7f, 0.0f, 0.0f);  // relative to the hand (lem4)
            fret->SetPosition(0, pos);
            fret->SetAngleX(0, 0.0f);
            fret->SetAngleZ(0, PI/2.0f);
            fret->SetAngleY(0, 0.0f);
        }

        m_object->SetFret(fret);  // takes
    }

    if ( m_arm == TMA_FBACK )  // takes on the ground behind?
    {
        fret = SearchTakeBackObject(FALSE, pos, dist, angle);
        if ( fret == 0 )  return FALSE;  // nothing to take?
        m_fretType = fret->RetType();

        fret->SetTruck(m_object);
        fret->SetTruckPart(3);  // takes with the hand

        pos = D3DVECTOR(4.7f, 0.0f, 0.0f);  // relative to the hand (lem4)
        fret->SetPosition(0, pos);
        fret->SetAngleX(0, 0.0f);
        fret->SetAngleZ(0, PI/2.0f);
        fret->SetAngleY(0, 0.0f);

        m_object->SetFret(fret);  // takes
    }

    if ( m_arm == TMA_POWER )  // takes battery in the back?
    {
        fret = m_object->RetPower();
        if ( fret == 0 )  return FALSE;  // no battery?
        m_fretType = fret->RetType();

        pos = D3DVECTOR(4.7f, 0.0f, 0.0f);  // relative to the hand (lem4)
        fret->SetPosition(0, pos);
        fret->SetAngleX(0, 0.0f);
        fret->SetAngleZ(0, PI/2.0f);
        fret->SetAngleY(0, 0.0f);
        fret->SetTruckPart(3);  // takes with the hand

        m_object->SetPower(0);
        m_object->SetFret(fret);  // takes
    }

    if ( m_arm == TMA_OTHER )  // battery takes from friend?
    {
        other = SearchOtherObject(FALSE, pos, dist, angle, m_height);
        if ( other == 0 )  return FALSE;

        fret = other->RetPower();
        if ( fret == 0 )  return FALSE;  // the other does not have a battery?
        m_fretType = fret->RetType();

        other->SetPower(0);
        fret->SetTruck(m_object);
        fret->SetTruckPart(3);  // takes with the hand

        pos = D3DVECTOR(4.7f, 0.0f, 0.0f);  // relative to the hand (lem4)
        fret->SetPosition(0, pos);
        fret->SetAngleX(0, 0.0f);
        fret->SetAngleZ(0, PI/2.0f);
        fret->SetAngleY(0, 0.0f);

        m_object->SetFret(fret);  // takes
    }

    return TRUE;
}

// Deposes the object taken.

BOOL CTaskManip::TruckDeposeObject()
{
    Character*  character;
    CObject*    fret;
    CObject*    other;
    D3DMATRIX*  mat;
    D3DVECTOR   pos;
    float       angle, dist;

    if ( m_arm == TMA_FFRONT )  // deposits on the ground in front?
    {
        fret = m_object->RetFret();
        if ( fret == 0 )  return FALSE;  // nothing transported?
        m_fretType = fret->RetType();

        mat = fret->RetWorldMatrix(0);
        pos = Transform(*mat, D3DVECTOR(0.0f, 1.0f, 0.0f));
        m_terrain->MoveOnFloor(pos);
        fret->SetPosition(0, pos);
        fret->SetAngleY(0, m_object->RetAngleY(0)+PI/2.0f);
        fret->SetAngleX(0, 0.0f);
        fret->SetAngleZ(0, 0.0f);
        fret->FloorAdjust();  // plate well on the ground

        fret->SetTruck(0);
        m_object->SetFret(0);  // deposit
    }

    if ( m_arm == TMA_FBACK )  // deposited on the ground behind?
    {
        fret = m_object->RetFret();
        if ( fret == 0 )  return FALSE;  // nothing transported?
        m_fretType = fret->RetType();

        mat = fret->RetWorldMatrix(0);
        pos = Transform(*mat, D3DVECTOR(0.0f, 1.0f, 0.0f));
        m_terrain->MoveOnFloor(pos);
        fret->SetPosition(0, pos);
        fret->SetAngleY(0, m_object->RetAngleY(0)+PI/2.0f);
        fret->SetAngleX(0, 0.0f);
        fret->SetAngleZ(0, 0.0f);

        fret->SetTruck(0);
        m_object->SetFret(0);  // deposit
    }

    if ( m_arm == TMA_POWER )  // deposits battery in the back?
    {
        fret = m_object->RetFret();
        if ( fret == 0 )  return FALSE;  // nothing transported?
        m_fretType = fret->RetType();

        if ( m_object->RetPower() != 0 )  return FALSE;

        fret->SetTruck(m_object);
        fret->SetTruckPart(0);  // carried by the base

        character = m_object->RetCharacter();
        fret->SetPosition(0, character->posPower);
        fret->SetAngleY(0, 0.0f);
        fret->SetAngleX(0, 0.0f);
        fret->SetAngleZ(0, 0.0f);

        m_object->SetPower(fret);  // uses
        m_object->SetFret(0);
    }

    if ( m_arm == TMA_OTHER )  // deposits battery on friend?
    {
        other = SearchOtherObject(FALSE, pos, dist, angle, m_height);
        if ( other == 0 )  return FALSE;

        fret = other->RetPower();
        if ( fret != 0 )  return FALSE;  // the other already has a battery?

        fret = m_object->RetFret();
        if ( fret == 0 )  return FALSE;
        m_fretType = fret->RetType();

        other->SetPower(fret);
        fret->SetTruck(other);

        character = other->RetCharacter();
        fret->SetPosition(0, character->posPower);
        fret->SetAngleY(0, 0.0f);
        fret->SetAngleX(0, 0.0f);
        fret->SetAngleZ(0, 0.0f);
        fret->SetTruckPart(0);  // carried by the base

        m_object->SetFret(0);  // deposit
    }

    return TRUE;
}

// Seeks if a location allows to deposit an object.

BOOL CTaskManip::IsFreeDeposeObject(D3DVECTOR pos)
{
    CObject*    pObj;
    D3DMATRIX*  mat;
    D3DVECTOR   iPos, oPos;
    float       oRadius;
    int         i, j;

    mat = m_object->RetWorldMatrix(0);
    iPos = Transform(*mat, pos);

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        if ( pObj == m_object )  continue;
        if ( !pObj->RetActif() )  continue;  // inactive?
        if ( pObj->RetTruck() != 0 )  continue;  // object transported?

        j = 0;
        while ( pObj->GetCrashSphere(j++, oPos, oRadius) )
        {
            if ( Length(iPos, oPos)-(oRadius+1.0f) < 2.0f )
            {
                return FALSE;  // location occupied
            }
        }
    }
    return TRUE;  // location free
}

// Plays the sound of the manipulator arm.

void CTaskManip::SoundManip(float time, float amplitude, float frequency)
{
    int     i;

    i = m_sound->Play(SOUND_MANIP, m_object->RetPosition(0), 0.0f, 0.3f*frequency, TRUE);
    m_sound->AddEnvelope(i, 0.5f*amplitude, 1.0f*frequency, 0.1f, SOPER_CONTINUE);
    m_sound->AddEnvelope(i, 0.5f*amplitude, 1.0f*frequency, time-0.1f, SOPER_CONTINUE);
    m_sound->AddEnvelope(i, 0.0f, 0.3f*frequency, 0.1f, SOPER_STOP);
}

