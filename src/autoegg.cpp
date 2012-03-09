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
#include "D3DEngine.h"
#include "D3DMath.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "math3d.h"
#include "particule.h"
#include "terrain.h"
#include "camera.h"
#include "object.h"
#include "pyro.h"
#include "cmdtoken.h"
#include "auto.h"
#include "autoegg.h"



// Constructeur de l'objet.

CAutoEgg::CAutoEgg(CInstanceManager* iMan, CObject* object)
				   : CAuto(iMan, object)
{
	CAuto::CAuto(iMan, object);

	m_type = OBJECT_NULL;
	m_value = 0.0f;
	m_string[0] = 0;

	m_param = 0;
	m_phase = AEP_NULL;
	Init();
}

// Destructeur de l'objet.

CAutoEgg::~CAutoEgg()
{
	CAuto::~CAuto();
}


// D�truit l'objet.

void CAutoEgg::DeleteObject(BOOL bAll)
{
	CObject*	alien;

	CAuto::DeleteObject(bAll);

	if ( !bAll )
	{
		alien = SearchAlien();
		if ( alien != 0 )
		{
			if ( alien->RetZoom(0) == 1.0f )
			{
				alien->SetLock(FALSE);
				alien->SetActivity(TRUE);  // l'insect n� est actif
			}
			else
			{
				alien->DeleteObject();
				delete alien;
			}
		}
	}
}


// Initialise l'objet.

void CAutoEgg::Init()
{
	CObject*	alien;

	alien = SearchAlien();
	if ( alien == 0 )
	{
		m_phase    = AEP_NULL;
		m_progress = 0.0f;
		m_speed    = 1.0f/5.0f;
		m_time     = 0.0f;
		return;
	}

	m_phase    = AEP_INCUB;
	m_progress = 0.0f;
	m_speed    = 1.0f/5.0f;
	m_time     = 0.0f;

	m_type = alien->RetType();

	if ( m_type == OBJECT_ANT    ||
		 m_type == OBJECT_SPIDER ||
		 m_type == OBJECT_BEE    )
	{
		alien->SetZoom(0, 0.2f);
	}
	if ( m_type == OBJECT_WORM )
	{
		alien->SetZoom(0, 0.01f);  // invisible !
	}
	alien->SetLock(TRUE);
	alien->SetActivity(FALSE);
}


// Donne une valeur.

BOOL CAutoEgg::SetType(ObjectType type)
{
	m_type = type;
	return TRUE;
}

// Donne une valeur.

BOOL CAutoEgg::SetValue(int rank, float value)
{
	if ( rank != 0 )  return FALSE;
	m_value = value;
	return TRUE;
}

// Donne la string.

BOOL CAutoEgg::SetString(char *string)
{
	strcpy(m_string, string);
	return TRUE;
}


// D�marre l'objet.

void CAutoEgg::Start(int param)
{
	if ( m_type == OBJECT_NULL )  return;
	if ( m_value == 0.0f )  return;

	m_phase    = AEP_DELAY;
	m_progress = 0.0f;
	m_speed    = 1.0f/m_value;

	m_param = param;
}


// Gestion d'un �v�nement.

BOOL CAutoEgg::EventProcess(const Event &event)
{
	CObject*	alien;

	CAuto::EventProcess(event);

	if ( m_engine->RetPause() )  return TRUE;

	if ( event.event != EVENT_FRAME )  return TRUE;
	if ( m_phase == AEP_NULL )  return TRUE;

	if ( m_phase == AEP_DELAY )
	{
		m_progress += event.rTime*m_speed;
		if ( m_progress < 1.0f )  return TRUE;

		alien = new CObject(m_iMan);
		if ( !alien->CreateInsect(m_object->RetPosition(0), m_object->RetAngleY(0), m_type) )
		{
			delete alien;
			m_phase    = AEP_DELAY;
			m_progress = 0.0f;
			m_speed    = 1.0f/2.0f;
			return TRUE;
		}
		alien->SetActivity(FALSE);
		alien->ReadProgram(0, m_string);
		alien->RunProgram(0);
		Init();
	}

	alien = SearchAlien();
	if ( alien == 0 )  return TRUE;
	alien->SetActivity(FALSE);

	m_progress += event.rTime*m_speed;

	if ( m_phase == AEP_ZOOM )
	{
		if ( m_type == OBJECT_ANT    ||
			 m_type == OBJECT_SPIDER ||
			 m_type == OBJECT_BEE    )
		{
			alien->SetZoom(0, 0.2f+m_progress*0.8f);  // �a pousse
		}
	}

	return TRUE;
}

// Indique si l'automate a termin� son activit�.

Error CAutoEgg::IsEnded()
{
	CObject*	alien;
	CPyro*		pyro;

	if ( m_phase == AEP_DELAY )
	{
		return ERR_CONTINUE;
	}

	alien = SearchAlien();
	if ( alien == 0 )  return ERR_STOP;

	if ( m_phase == AEP_INCUB )
	{
		if ( m_progress < 1.0f )  return ERR_CONTINUE;

		m_phase    = AEP_ZOOM;
		m_progress = 0.0f;
		m_speed    = 1.0f/5.0f;
	}

	if ( m_phase == AEP_ZOOM )
	{
		if ( m_progress < 1.0f )  return ERR_CONTINUE;

		pyro = new CPyro(m_iMan);
		pyro->Create(PT_EGG, m_object);  // explosion de l'oeuf

		alien->SetZoom(0, 1.0f);  // c'est un grand gar�on, maintenant

		m_phase    = AEP_WAIT;
		m_progress = 0.0f;
		m_speed    = 1.0f/3.0f;
	}

	if ( m_phase == AEP_WAIT )
	{
		if ( m_progress < 1.0f )  return ERR_CONTINUE;

		alien->SetLock(FALSE);
		alien->SetActivity(TRUE);  // l'insect n� est actif
	}

	return ERR_STOP;
}


// Retourne une erreur li�e � l'�tat de l'automate.

Error CAutoEgg::RetError()
{
	return ERR_OK;
}


// Cherche l'insect qui prend naissance dans l'oeuf.

CObject* CAutoEgg::SearchAlien()
{
	CObject*	pObj;
	CObject*	pBest;
	D3DVECTOR	cPos, oPos;
	ObjectType	type;
	float		dist, min;
	int			i;

	cPos = m_object->RetPosition(0);
	min = 100000.0f;
	pBest = 0;
	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( pObj->RetTruck() != 0 )  continue;

		type = pObj->RetType();
		if ( type != OBJECT_ANT    &&
			 type != OBJECT_BEE    &&
			 type != OBJECT_SPIDER &&
			 type != OBJECT_WORM   )  continue;

		oPos = pObj->RetPosition(0);
		dist = Length2d(oPos, cPos);
		if ( dist < 8.0f && dist < min )
		{
			min = dist;
			pBest = pObj;
		}
	}
	return pBest;
}


// Sauve tous les param�tres de l'automate.

BOOL CAutoEgg::Write(char *line)
{
	char	name[100];

	if ( m_phase == AEP_NULL )  return FALSE;

	sprintf(name, " aExist=%d", 1);
	strcat(line, name);

	CAuto::Write(line);

	sprintf(name, " aPhase=%d", m_phase);
	strcat(line, name);

	sprintf(name, " aProgress=%.2f", m_progress);
	strcat(line, name);

	sprintf(name, " aSpeed=%.5f", m_speed);
	strcat(line, name);

	sprintf(name, " aParamType=%s", GetTypeObject(m_type));
	strcat(line, name);

	sprintf(name, " aParamValue1=%.2f", m_value);
	strcat(line, name);

	sprintf(name, " aParamString=\"%s\"", m_string);
	strcat(line, name);

	return TRUE;
}

// Restitue tous les param�tres de l'automate.

BOOL CAutoEgg::Read(char *line)
{
	if ( OpInt(line, "aExist", 0) == 0 )  return FALSE;

	CAuto::Read(line);

	m_phase = (AutoEggPhase)OpInt(line, "aPhase", AEP_NULL);
	m_progress = OpFloat(line, "aProgress", 0.0f);
	m_speed = OpFloat(line, "aSpeed", 1.0f);
	m_type = OpTypeObject(line, "aParamType", OBJECT_NULL);
	m_value = OpFloat(line, "aParamValue1", 0.0f);
	OpString(line, "aParamString", m_string);

	return TRUE;
}

