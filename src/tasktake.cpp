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

// tasktake.cpp

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
#include "physics.h"
#include "brain.h"
#include "water.h"
#include "camera.h"
#include "motion.h"
#include "motionhuman.h"
#include "sound.h"
#include "robotmain.h"
#include "task.h"
#include "tasktake.h"




// Object's constructor.

CTaskTake::CTaskTake(CInstanceManager* iMan, CObject* object)
                     : CTask(iMan, object)
{
    m_terrain = (CTerrain*)m_iMan->SearchInstance(CLASS_TERRAIN);

    m_arm  = TTA_NEUTRAL;
}

// Object's destructor.

CTaskTake::~CTaskTake()
{
}


// Management of an event.

BOOL CTaskTake::EventProcess(const Event &event)
{
    float       a, g, cirSpeed;

    if ( m_engine->RetPause() )  return TRUE;
    if ( event.event != EVENT_FRAME )  return TRUE;
    if ( m_bError )  return FALSE;

    if ( m_bTurn )  // preliminary rotation?
    {
        a = m_object->RetAngleY(0);
        g = m_angle;
        cirSpeed = Direction(a, g)*2.0f;
        if ( cirSpeed >  1.0f )  cirSpeed =  1.0f;
        if ( cirSpeed < -1.0f )  cirSpeed = -1.0f;

        m_physics->SetMotorSpeedZ(cirSpeed);  // turns left / right
        return TRUE;
    }

    m_progress += event.rTime*m_speed;  // others advance

    m_physics->SetMotorSpeed(D3DVECTOR(0.0f, 0.0f, 0.0f));  // immobile!

    return TRUE;
}


// Assigns the goal was achieved.

Error CTaskTake::Start()
{
    ObjectType  type;
    CObject*    other;
    float       iAngle, oAngle, h;
    D3DVECTOR   pos;

    m_height   = 0.0f;
    m_step     = 0;
    m_progress = 0.0f;

    iAngle = m_object->RetAngleY(0);
    iAngle = NormAngle(iAngle);  // 0..2*PI
    oAngle = iAngle;

    m_bError = TRUE;  // operation impossible
    if ( !m_physics->RetLand() )
    {
        pos = m_object->RetPosition(0);
        h = m_water->RetLevel(m_object);
        if ( pos.y < h )  return ERR_MANIP_WATER;  // impossible under water
        return ERR_MANIP_FLY;
    }

    type = m_object->RetType();
    if ( type != OBJECT_HUMAN &&
         type != OBJECT_TECH  )  return ERR_MANIP_VEH;

    m_physics->SetMotorSpeed(D3DVECTOR(0.0f, 0.0f, 0.0f));

    if ( m_object->RetFret() == 0 )
    {
        m_order = TTO_TAKE;
    }
    else
    {
        m_order = TTO_DEPOSE;
    }

    if ( m_order == TTO_TAKE )
    {
        pos = m_object->RetPosition(0);
        h = m_water->RetLevel(m_object);
        if ( pos.y < h )  return ERR_MANIP_WATER;  // impossible under water

        other = SearchFriendObject(oAngle, 1.5f, PI*0.50f);
        if ( other != 0 && other->RetPower() != 0 )
        {
            type = other->RetPower()->RetType();
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
//?         m_camera->StartCentering(m_object, PI*0.3f, -PI*0.1f, 0.0f, 0.8f);
            m_arm = TTA_FRIEND;
        }
        else
        {
            other = SearchTakeObject(oAngle, 1.5f, PI*0.45f);
            if ( other == 0 )  return ERR_MANIP_NIL;
            type = other->RetType();
            if ( type == OBJECT_URANIUM )  return ERR_MANIP_RADIO;
//?         m_camera->StartCentering(m_object, PI*0.3f, 99.9f, 0.0f, 0.8f);
            m_arm = TTA_FFRONT;
            m_main->HideDropZone(other);  // hides buildable area
        }
    }

    if ( m_order == TTO_DEPOSE )
    {
//?     speed = m_physics->RetMotorSpeed();
//?     if ( speed.x != 0.0f ||
//?          speed.z != 0.0f )  return ERR_MANIP_MOTOR;

        other = SearchFriendObject(oAngle, 1.5f, PI*0.50f);
        if ( other != 0 && other->RetPower() == 0 )
        {
//?         m_camera->StartCentering(m_object, PI*0.3f, -PI*0.1f, 0.0f, 0.8f);
            m_arm = TTA_FRIEND;
        }
        else
        {
            if ( !IsFreeDeposeObject(D3DVECTOR(2.5f, 0.0f, 0.0f)) )  return ERR_MANIP_OCC;
//?         m_camera->StartCentering(m_object, PI*0.3f, 99.9f, 0.0f, 0.8f);
            m_arm = TTA_FFRONT;
        }
    }

    m_bTurn = TRUE;  // preliminary rotation necessary
    m_angle = oAngle;  // angle was reached

    m_physics->SetFreeze(TRUE);  // it does not move

    m_bError = FALSE;  // ok
    return ERR_OK;
}

// Indicates whether the action is finished.

Error CTaskTake::IsEnded()
{
    CObject*    fret;
    float       angle;

    if ( m_engine->RetPause() )  return ERR_CONTINUE;
    if ( m_bError )  return ERR_STOP;

    if ( m_bTurn )  // preliminary rotation?
    {
        angle = m_object->RetAngleY(0);
        angle = NormAngle(angle);  // 0..2*PI

        if ( TestAngle(angle, m_angle-PI*0.01f, m_angle+PI*0.01f) )
        {
            m_bTurn = FALSE;  // rotation ended
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
            if ( TruckTakeObject() )
            {
                if ( m_arm == TTA_FRIEND &&
                     (m_fretType == OBJECT_POWER  ||
                      m_fretType == OBJECT_ATOMIC ) )
                {
                    m_sound->Play(SOUND_POWEROFF, m_object->RetPosition(0));
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
            fret = m_object->RetFret();
            TruckDeposeObject();
            if ( m_arm == TTA_FRIEND &&
                 (m_fretType == OBJECT_POWER  ||
                  m_fretType == OBJECT_ATOMIC ) )
            {
                m_sound->Play(SOUND_POWERON, m_object->RetPosition(0));
            }
            if ( fret != 0 && m_fretType == OBJECT_METAL && m_arm == TTA_FFRONT )
            {
                m_main->ShowDropZone(fret, m_object);  // shows buildable area
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

BOOL CTaskTake::Abort()
{
    m_motion->SetAction(-1);
    m_camera->StopCentering(m_object, 0.8f);
    m_physics->SetFreeze(FALSE);  // is moving again
    return TRUE;
}


// Seeks the object to take in front.

CObject* CTaskTake::SearchTakeObject(float &angle,
                                     float dLimit, float aLimit)
{
    CObject     *pObj, *pBest;
    D3DVECTOR   iPos, oPos;
    ObjectType  type;
    float       min, iAngle, bAngle, a, distance;
    int         i;

    iPos   = m_object->RetPosition(0);
    iAngle = m_object->RetAngleY(0);
    iAngle = NormAngle(iAngle);  // 0..2*PI

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
             type != OBJECT_TNT     )  continue;

        if ( pObj->RetTruck() != 0 )  continue;  // object transported?
        if ( pObj->RetLock() )  continue;
        if ( pObj->RetZoomY(0) != 1.0f )  continue;

        oPos = pObj->RetPosition(0);
        distance = Length(oPos, iPos);
        if ( distance >= 4.0f-dLimit &&
             distance <= 4.0f+dLimit )
        {
            angle = RotateAngle(oPos.x-iPos.x, iPos.z-oPos.z);  // CW !
            if ( TestAngle(angle, iAngle-aLimit, iAngle+aLimit) )
            {
                a = Abs(angle-iAngle);
                if ( a > PI )  a = PI*2.0f-a;
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
    CObject*    pObj;
    CObject*    pPower;
    D3DMATRIX*  mat;
    D3DVECTOR   iPos, oPos;
    ObjectType  type, powerType;
    float       iAngle, iRad, distance;
    int         i;

    if ( !m_object->GetCrashSphere(0, iPos, iRad) )  return 0;
    iAngle = m_object->RetAngleY(0);
    iAngle = NormAngle(iAngle);  // 0..2*PI

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

        distance = Abs(Length(oPos, iPos) - (iRad+1.0f));
        if ( distance <= dLimit )
        {
            angle = RotateAngle(oPos.x-iPos.x, iPos.z-oPos.z);  // CW !
            if ( TestAngle(angle, iAngle-aLimit, iAngle+aLimit) )
            {
                character = pObj->RetCharacter();
                m_height = character->posPower.y;
                return pObj;
            }
        }
    }

    return 0;
}

// Takes the object in front.

BOOL CTaskTake::TruckTakeObject()
{
    CObject*    fret;
    CObject*    other;
    D3DMATRIX   matRotate;
    float       angle;

    if ( m_arm == TTA_FFRONT )  // takes on the ground in front?
    {
//?     fret = SearchTakeObject(angle, 1.5f, PI*0.04f);
        fret = SearchTakeObject(angle, 1.5f, PI*0.15f);  //OK 1.9
        if ( fret == 0 )  return FALSE;  // rien � prendre ?
        m_fretType = fret->RetType();

        fret->SetTruck(m_object);
        fret->SetTruckPart(4);  // takes with the hand

//?     fret->SetPosition(0, D3DVECTOR(2.2f, -1.0f, 1.1f));
        fret->SetPosition(0, D3DVECTOR(1.7f, -0.5f, 1.1f));
        fret->SetAngleY(0, 0.1f);
        fret->SetAngleX(0, 0.0f);
        fret->SetAngleZ(0, 0.8f);

        m_object->SetFret(fret);  // takes
    }

    if ( m_arm == TTA_FRIEND )  // takes friend's battery?
    {
        other = SearchFriendObject(angle, 1.5f, PI*0.04f);
        if ( other == 0 )  return FALSE;

        fret = other->RetPower();
        if ( fret == 0 )  return FALSE;  // the other does not have a battery?
        m_fretType = fret->RetType();

        other->SetPower(0);
        fret->SetTruck(m_object);
        fret->SetTruckPart(4);  // takes with the hand

//?     fret->SetPosition(0, D3DVECTOR(2.2f, -1.0f, 1.1f));
        fret->SetPosition(0, D3DVECTOR(1.7f, -0.5f, 1.1f));
        fret->SetAngleY(0, 0.1f);
        fret->SetAngleX(0, 0.0f);
        fret->SetAngleZ(0, 0.8f);

        m_object->SetFret(fret);  // takes
    }

    return TRUE;
}

// Deposes the object taken.

BOOL CTaskTake::TruckDeposeObject()
{
    Character*  character;
    CObject*    fret;
    CObject*    other;
    D3DMATRIX*  mat;
    D3DVECTOR   pos;
    float       angle;

    if ( m_arm == TTA_FFRONT )  // deposes on the ground in front?
    {
        fret = m_object->RetFret();
        if ( fret == 0 )  return FALSE;  // does nothing?
        m_fretType = fret->RetType();

        mat = fret->RetWorldMatrix(0);
        pos = Transform(*mat, D3DVECTOR(-0.5f, 1.0f, 0.0f));
        m_terrain->MoveOnFloor(pos);
        fret->SetPosition(0, pos);
        fret->SetAngleY(0, m_object->RetAngleY(0)+PI/2.0f);
        fret->SetAngleX(0, 0.0f);
        fret->SetAngleZ(0, 0.0f);
        fret->FloorAdjust();  // plate well on the ground

        fret->SetTruck(0);
        m_object->SetFret(0);  // deposit
    }

    if ( m_arm == TTA_FRIEND )  // deposes battery on friends?
    {
        other = SearchFriendObject(angle, 1.5f, PI*0.04f);
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

BOOL CTaskTake::IsFreeDeposeObject(D3DVECTOR pos)
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
            if ( Length(iPos, oPos)-(oRadius+1.0f) < 1.0f )
            {
                return FALSE;  // location occupied
            }
        }
    }
    return TRUE;  // location free
}


