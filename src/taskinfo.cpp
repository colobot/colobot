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
// * along with this program. If not, see .

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "math3d.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "particule.h"
#include "terrain.h"
#include "object.h"
#include "physics.h"
#include "brain.h"
#include "sound.h"
#include "auto.h"
#include "autoinfo.h"
#include "task.h"
#include "taskinfo.h"




// Constructeur de l'objet.

CTaskInfo::CTaskInfo(CInstanceManager* iMan, CObject* object)
						 : CTask(iMan, object)
{
	CTask::CTask(iMan, object);
}

// Destructeur de l'objet.

CTaskInfo::~CTaskInfo()
{
}


// Gestion d'un �v�nement.

BOOL CTaskInfo::EventProcess(const Event &event)
{
	if ( m_engine->RetPause() )  return TRUE;
	if ( event.event != EVENT_FRAME )  return TRUE;
	if ( m_bError )  return FALSE;

	m_progress += event.rTime*m_speed;  // �a avance
	m_time += event.rTime;

	return TRUE;
}


// Assigne le but � atteindre.

Error CTaskInfo::Start(char *name, float value, float power, BOOL bSend)
{
	CObject*	pInfo;
	CAutoInfo*	pAuto;
	D3DVECTOR	pos, goal;
	Info		info;
	int			i, total, op;

	m_bError = TRUE;
	m_object->SetInfoReturn(NAN);

	pInfo = SearchInfo(power);  // cherche borne
	if ( pInfo == 0 )
	{
		return ERR_INFO_NULL;
	}

	pAuto = (CAutoInfo*)pInfo->RetAuto();
	if ( pAuto == 0 )
	{
		return ERR_INFO_NULL;
	}

	op = 1;  // �mission impossible
	if ( bSend )  // send ?
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
				op = 2;  // d�but de r�ception (pour la borne)
			}
		}
		else
		{
			op = 2;  // d�but de r�ception (pour la borne)
		}
	}
	else	// receive ?
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
			op = 0;  // d�but d'�mission (pour la borne)
		}
	}

	pAuto->Start(op);

	if ( op == 0 )  // �mission ?
	{
		pos = pInfo->RetPosition(0);
		pos.y += 9.5f;
		goal = m_object->RetPosition(0);
		goal.y += 4.0f;
		m_particule->CreateRay(pos, goal, PARTIRAY3, FPOINT(2.0f, 2.0f), 1.0f);
	}
	if ( op == 2 )  // r�ception ?
	{
		goal = pInfo->RetPosition(0);
		goal.y += 9.5f;
		pos = m_object->RetPosition(0);
		pos.y += 4.0f;
		m_particule->CreateRay(pos, goal, PARTIRAY3, FPOINT(2.0f, 2.0f), 1.0f);
	}

	m_progress = 0.0f;
	m_speed    = 1.0f/1.0f;
	m_time     = 0.0f;

	m_bError = FALSE;  // ok

	return ERR_OK;
}

// Indique si l'action est termin�e.

Error CTaskInfo::IsEnded()
{
	if ( m_engine->RetPause() )  return ERR_CONTINUE;
	if ( m_bError )  return ERR_STOP;

	if ( m_progress < 1.0f )  return ERR_CONTINUE;
	m_progress = 0.0f;

	Abort();
	return ERR_STOP;
}

// Termine brutalement l'action en cours.

BOOL CTaskInfo::Abort()
{
	return TRUE;
}


// Cherche la borne d'information la plus proche.

CObject* CTaskInfo::SearchInfo(float power)
{
	CObject		*pObj, *pBest;
	D3DVECTOR	iPos, oPos;
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
		dist = Length(oPos, iPos);
		if ( dist > power )  continue;  // trop loin ?
		if ( dist < min )
		{
			min = dist;
			pBest = pObj;
		}
	}

	return pBest;
}

