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

// taskflag.cpp


#include <stdio.h>

#include "object/task/taskflag.h"

#include "math/geometry.h"
#include "common/iman.h"
#include "graphics/common/water.h"
#include "graphics/common/pyro.h"
#include "physics/physics.h"
#include "object/motion/motionhuman.h"





// Object's constructor.

CTaskFlag::CTaskFlag(CInstanceManager* iMan, CObject* object)
					   : CTask(iMan, object)
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
	if ( m_engine->RetPause() )  return true;
	if ( event.event != EVENT_FRAME )  return true;

	m_time += event.rTime;

	return true;
}



// Assigns the goal was achieved.

Error CTaskFlag::Start(TaskFlagOrder order, int rank)
{
	Math::Vector	pos, speed;
	Error		err;

	m_order = order;
	m_time = 0.0f;

	m_bError = true;  // operation impossible
	if ( !m_physics->RetLand() )
	{
		pos = m_object->RetPosition(0);
		if ( pos.y < m_water->RetLevel() )  return ERR_FLAG_WATER;
		return ERR_FLAG_FLY;
	}

	speed = m_physics->RetMotorSpeed();
	if ( speed.x != 0.0f ||
		 speed.z != 0.0f )  return ERR_FLAG_MOTOR;

	if ( m_object->RetFret() != 0 )  return ERR_FLAG_BUSY;

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
	if ( m_engine->RetPause() )  return ERR_CONTINUE;

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
	ObjectType	oType;
	CObject		*pObj, *pBest;
	Math::Vector	oPos;
	float		min, dist;
	int			i;

	min = 100000.0f;
	pBest = 0;
	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( !pObj->RetEnable() )  continue;

		oType = pObj->RetType();
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

		oPos = pObj->RetPosition(0);
		dist = Math::DistanceProjected(oPos, pos);
		if ( dist < min )
		{
			min = dist;
			pBest = pObj;
		}
	}
	return pBest;
}

// Counts the number of existing objects.

int CTaskFlag::CountObject(ObjectType type)
{
	ObjectType	oType;
	CObject		*pObj;
	Math::Vector	oPos;
	int			i, count;

	count = 0;
	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( !pObj->RetEnable() )  continue;

		oType = pObj->RetType();
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
	CObject*	pObj;
	CObject*	pNew;
	CPyro*		pyro;
	Math::Matrix*	mat;
	Math::Vector	pos;
	float		dist;
	int			i;

	ObjectType	table[5] =
	{
		OBJECT_FLAGb,
		OBJECT_FLAGr,
		OBJECT_FLAGg,
		OBJECT_FLAGy,
		OBJECT_FLAGv,
	};

	mat = m_object->RetWorldMatrix(0);
	pos = Transform(*mat, Math::Vector(4.0f, 0.0f, 0.0f));

	pObj = SearchNearest(pos, OBJECT_NULL);
	if ( pObj != 0 )
	{
		dist = Math::Distance(pos, pObj->RetPosition(0));
		if ( dist < 10.0f )
		{
			return ERR_FLAG_PROXY;
		}
	}

	i = rank;
	if ( CountObject(table[i]) >= 5 )
	{
		return ERR_FLAG_CREATE;
	}

	pNew = new CObject(m_iMan);
	if ( !pNew->CreateFlag(pos, 0.0f, table[i]) )
	{
		delete pNew;
		return ERR_TOOMANY;
	}
	pNew->SetZoom(0, 0.0f);

	m_sound->Play(SOUND_WAYPOINT, pos);
	pyro = new CPyro(m_iMan);
	pyro->Create(PT_FLCREATE, pNew);

	return ERR_OK;
}

// Deletes a color indicator.

Error CTaskFlag::DeleteFlag()
{
	CObject*	pObj;
	CPyro*		pyro;
	Math::Vector	iPos, oPos;
	float		iAngle, angle, aLimit, dist;

	iPos = m_object->RetPosition(0);
	iAngle = m_object->RetAngleY(0);
	iAngle = Math::NormAngle(iAngle);  // 0..2*Math::PI

	pObj = SearchNearest(iPos, OBJECT_NULL);
	if ( pObj == 0 )
	{
		return ERR_FLAG_DELETE;
	}
	dist = Math::Distance(iPos, pObj->RetPosition(0));
	if ( dist > 10.0f )
	{
		return ERR_FLAG_DELETE;
	}

	oPos = pObj->RetPosition(0);
	angle = Math::RotateAngle(oPos.x-iPos.x, iPos.z-oPos.z);  // CW !
	aLimit = 45.0f*Math::PI/180.0f;
	if ( !Math::TestAngle(angle, iAngle-aLimit, iAngle+aLimit) )
	{
		return ERR_FLAG_DELETE;
	}

	m_sound->Play(SOUND_WAYPOINT, iPos);
	pyro = new CPyro(m_iMan);
	pyro->Create(PT_FLDELETE, pObj);

	return ERR_OK;
}

