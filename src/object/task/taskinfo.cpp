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

// taskinfo.cpp


#include <stdio.h>

#include "object/task/taskinfo.h"

#include "common/iman.h"
#include "old/particule.h"
#include "object/auto/autoinfo.h"





// Object's constructor.

CTaskInfo::CTaskInfo(CInstanceManager* iMan, CObject* object)
						 : CTask(iMan, object)
{
}

// Object's destructor.

CTaskInfo::~CTaskInfo()
{
}


// Management of an event.

bool CTaskInfo::EventProcess(const Event &event)
{
	if ( m_engine->RetPause() )  return true;
	if ( event.event != EVENT_FRAME )  return true;
	if ( m_bError )  return false;

	m_progress += event.rTime*m_speed;  // other advance
	m_time += event.rTime;

	return true;
}


// Assigns the goal was achieved.

Error CTaskInfo::Start(char *name, float value, float power, bool bSend)
{
	CObject*	pInfo;
	CAutoInfo*	pAuto;
	Math::Vector	pos, goal;
	Info		info;
	int			i, total, op;

	m_bError = true;
	m_object->SetInfoReturn(NAN);

	pInfo = SearchInfo(power);  // seeks terminal
	if ( pInfo == 0 )
	{
		return ERR_INFO_NULL;
	}

	pAuto = (CAutoInfo*)pInfo->RetAuto();
	if ( pAuto == 0 )
	{
		return ERR_INFO_NULL;
	}

	op = 1;  // transmission impossible
	if ( bSend )  // send?
	{
		total = pInfo->RetInfoTotal();
		for ( i=0 ; i<total ; i++ )
		{
			info = pInfo->RetInfo(i);
			if ( strcmp(info.name, name) == 0 )
			{
				info.value = value;
				pInfo->SetInfo(i, info);
				break;
			}
		}
		if ( i == total )
		{
			if ( total < OBJECTMAXINFO )
			{
				strcpy(info.name, name);
				info.value = value;
				pInfo->SetInfo(total, info);
				op = 2;  // start of reception (for terminal)
			}
		}
		else
		{
			op = 2;  // start of reception (for terminal)
		}
	}
	else	// receive?
	{
		total = pInfo->RetInfoTotal();
		for ( i=0 ; i<total ; i++ )
		{
			info = pInfo->RetInfo(i);
			if ( strcmp(info.name, name) == 0 )
			{
				m_object->SetInfoReturn(info.value);
				break;
			}
		}
		if ( i < total )
		{
			op = 0;  // beginning of transmission (for terminal)
		}
	}

	pAuto->Start(op);

	if ( op == 0 )  // transmission?
	{
		pos = pInfo->RetPosition(0);
		pos.y += 9.5f;
		goal = m_object->RetPosition(0);
		goal.y += 4.0f;
		m_particule->CreateRay(pos, goal, PARTIRAY3, Math::Point(2.0f, 2.0f), 1.0f);
	}
	if ( op == 2 )  // reception?
	{
		goal = pInfo->RetPosition(0);
		goal.y += 9.5f;
		pos = m_object->RetPosition(0);
		pos.y += 4.0f;
		m_particule->CreateRay(pos, goal, PARTIRAY3, Math::Point(2.0f, 2.0f), 1.0f);
	}

	m_progress = 0.0f;
	m_speed    = 1.0f/1.0f;
	m_time     = 0.0f;

	m_bError = false;  // ok

	return ERR_OK;
}

// Indicates whether the action is finished.

Error CTaskInfo::IsEnded()
{
	if ( m_engine->RetPause() )  return ERR_CONTINUE;
	if ( m_bError )  return ERR_STOP;

	if ( m_progress < 1.0f )  return ERR_CONTINUE;
	m_progress = 0.0f;

	Abort();
	return ERR_STOP;
}

// Suddenly ends the current action.

bool CTaskInfo::Abort()
{
	return true;
}


// Seeks the nearest information terminal.

CObject* CTaskInfo::SearchInfo(float power)
{
	CObject		*pObj, *pBest;
	Math::Vector	iPos, oPos;
	ObjectType	type;
	float		dist, min;
	int			i;

	iPos = m_object->RetPosition(0);

	min = 100000.0f;
	pBest = 0;
	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		type = pObj->RetType();
		if ( type != OBJECT_INFO )  continue;

		if ( !pObj->RetActif() )  continue;

		oPos = pObj->RetPosition(0);
		dist = Math::Distance(oPos, iPos);
		if ( dist > power )  continue;  // too far?
		if ( dist < min )
		{
			min = dist;
			pBest = pObj;
		}
	}

	return pBest;
}

