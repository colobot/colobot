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


#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "common/struct.h"
#include "math/geometry.h"
#include "graphics/d3d/d3dengine.h"
#include "math/old/d3dmath.h"
#include "common/event.h"
#include "common/misc.h"
#include "common/iman.h"
#include "math/old/math3d.h"
#include "graphics/common/particule.h"
#include "graphics/common/terrain.h"
#include "graphics/common/camera.h"
#include "object/object.h"
#include "graphics/common/pyro.h"
#include "script/cmdtoken.h"
#include "object/auto/auto.h"
#include "object/auto/autoegg.h"



// Object's constructor.

CAutoEgg::CAutoEgg(CInstanceManager* iMan, CObject* object)
				   : CAuto(iMan, object)
{
	m_type = OBJECT_NULL;
	m_value = 0.0f;
	m_string[0] = 0;

	m_param = 0;
	m_phase = AEP_NULL;
	Init();
}

// Object's destructor.

CAutoEgg::~CAutoEgg()
{
}


// Destroys the object.

void CAutoEgg::DeleteObject(bool bAll)
{
	CObject*	alien;

	CAuto::DeleteObject(bAll);

	if ( !bAll )
	{
		alien = SearchAlien();
		if ( alien != 0 )
		{
			// Probably the intended action
			// Original code: ( alien->RetZoom(0) == 1.0f )
			if ( alien->RetZoomY(0) == 1.0f )
			{
				alien->SetLock(false);
				alien->SetActivity(true);  // the insect is active
			}
			else
			{
				alien->DeleteObject();
				delete alien;
			}
		}
	}
}


// Initialize the object.

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
	alien->SetLock(true);
	alien->SetActivity(false);
}


// Gives a value.

bool CAutoEgg::SetType(ObjectType type)
{
	m_type = type;
	return true;
}

// Gives a value.

bool CAutoEgg::SetValue(int rank, float value)
{
	if ( rank != 0 )  return false;
	m_value = value;
	return true;
}

// Gives the string.

bool CAutoEgg::SetString(char *string)
{
	strcpy(m_string, string);
	return true;
}


// Start object.

void CAutoEgg::Start(int param)
{
	if ( m_type == OBJECT_NULL )  return;
	if ( m_value == 0.0f )  return;

	m_phase    = AEP_DELAY;
	m_progress = 0.0f;
	m_speed    = 1.0f/m_value;

	m_param = param;
}


// Management of an event.

bool CAutoEgg::EventProcess(const Event &event)
{
	CObject*	alien;

	CAuto::EventProcess(event);

	if ( m_engine->RetPause() )  return true;

	if ( event.event != EVENT_FRAME )  return true;
	if ( m_phase == AEP_NULL )  return true;

	if ( m_phase == AEP_DELAY )
	{
		m_progress += event.rTime*m_speed;
		if ( m_progress < 1.0f )  return true;

		alien = new CObject(m_iMan);
		if ( !alien->CreateInsect(m_object->RetPosition(0), m_object->RetAngleY(0), m_type) )
		{
			delete alien;
			m_phase    = AEP_DELAY;
			m_progress = 0.0f;
			m_speed    = 1.0f/2.0f;
			return true;
		}
		alien->SetActivity(false);
		alien->ReadProgram(0, m_string);
		alien->RunProgram(0);
		Init();
	}

	alien = SearchAlien();
	if ( alien == 0 )  return true;
	alien->SetActivity(false);

	m_progress += event.rTime*m_speed;

	if ( m_phase == AEP_ZOOM )
	{
		if ( m_type == OBJECT_ANT    ||
			 m_type == OBJECT_SPIDER ||
			 m_type == OBJECT_BEE    )
		{
			alien->SetZoom(0, 0.2f+m_progress*0.8f);  // Others push
		}
	}

	return true;
}

// Indicates whether the controller has completed its activity.

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
		pyro->Create(PT_EGG, m_object);  // exploding egg

		alien->SetZoom(0, 1.0f);  // this is a big boy now

		m_phase    = AEP_WAIT;
		m_progress = 0.0f;
		m_speed    = 1.0f/3.0f;
	}

	if ( m_phase == AEP_WAIT )
	{
		if ( m_progress < 1.0f )  return ERR_CONTINUE;

		alien->SetLock(false);
		alien->SetActivity(true);  // the insect is active
	}

	return ERR_STOP;
}


// Returns an error due the state of the automation.

Error CAutoEgg::RetError()
{
	return ERR_OK;
}


// Seeking the insect that starts in the egg.

CObject* CAutoEgg::SearchAlien()
{
	CObject*	pObj;
	CObject*	pBest;
	Math::Vector	cPos, oPos;
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
		dist = Math::DistanceProjected(oPos, cPos);
		if ( dist < 8.0f && dist < min )
		{
			min = dist;
			pBest = pObj;
		}
	}
	return pBest;
}


// Saves all parameters of the controller.

bool CAutoEgg::Write(char *line)
{
	char	name[100];

	if ( m_phase == AEP_NULL )  return false;

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

	return true;
}

// Restores all parameters of the controller.

bool CAutoEgg::Read(char *line)
{
	if ( OpInt(line, "aExist", 0) == 0 )  return false;

	CAuto::Read(line);

	m_phase = (AutoEggPhase)OpInt(line, "aPhase", AEP_NULL);
	m_progress = OpFloat(line, "aProgress", 0.0f);
	m_speed = OpFloat(line, "aSpeed", 1.0f);
	m_type = OpTypeObject(line, "aParamType", OBJECT_NULL);
	m_value = OpFloat(line, "aParamValue1", 0.0f);
	OpString(line, "aParamString", m_string);

	return true;
}

