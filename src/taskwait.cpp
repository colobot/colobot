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
// * along with this program. If not, see  http://www.gnu.org/licenses/.// taskwait.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "D3DEngine.h"
#include "math3d.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "terrain.h"
#include "object.h"
#include "physics.h"
#include "brain.h"
#include "task.h"
#include "taskwait.h"




// Constructeur de l'objet.

CTaskWait::CTaskWait(CInstanceManager* iMan, CObject* object)
					 : CTask(iMan, object)
{
	CTask::CTask(iMan, object);
}

// Destructeur de l'objet.

CTaskWait::~CTaskWait()
{
}


// Gestion d'un événement.

BOOL CTaskWait::EventProcess(const Event &event)
{
	if ( m_engine->RetPause() )  return TRUE;
	if ( event.event != EVENT_FRAME )  return TRUE;

	m_passTime += event.rTime;
	m_bEnded = (m_passTime >= m_waitTime);
	return TRUE;
}


// Assigne le but à atteindre.

Error CTaskWait::Start(float time)
{
	m_waitTime = time;  // durée à attendre
	m_passTime = 0.0f;  // durée écoulée
	m_bEnded = FALSE;
	return ERR_OK;
}

// Indique si l'action est terminée.

Error CTaskWait::IsEnded()
{
	if ( m_engine->RetPause() )  return ERR_CONTINUE;
	if ( m_bEnded )  return ERR_STOP;
	return ERR_CONTINUE;
}


