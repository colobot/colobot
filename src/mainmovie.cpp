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
#include "global.h"
#include "event.h"
#include "iman.h"
#include "math3d.h"
#include "camera.h"
#include "object.h"
#include "motion.h"
#include "motionhuman.h"
#include "interface.h"
#include "robotmain.h"
#include "sound.h"
#include "mainmovie.h"




// Constructeur de l'application carte.

CMainMovie::CMainMovie(CInstanceManager* iMan)
{
	m_iMan = iMan;
	m_iMan->AddInstance(CLASS_SHORT, this);

	m_interface = (CInterface*)m_iMan->SearchInstance(CLASS_INTERFACE);
	m_event     = (CEvent*)m_iMan->SearchInstance(CLASS_EVENT);
	m_engine    = (CD3DEngine*)m_iMan->SearchInstance(CLASS_ENGINE);
	m_main      = (CRobotMain*)m_iMan->SearchInstance(CLASS_MAIN);
	m_camera    = (CCamera*)m_iMan->SearchInstance(CLASS_CAMERA);
	m_sound     = (CSound*)m_iMan->SearchInstance(CLASS_SOUND);

	Flush();
}

// Destructeur de l'application robot.

CMainMovie::~CMainMovie()
{
}


// Stoppe le film en cours.

void CMainMovie::Flush()
{
	m_type = MM_NONE;
}


// D�but d'un film.

BOOL CMainMovie::Start(MainMovieType type, float time)
{
	D3DMATRIX*	mat;
	D3DVECTOR	pos;
	CObject*	pObj;
	CMotion*	motion;

	m_type = type;
	m_speed = 1.0f/time;
	m_progress = 0.0f;

	if ( m_type == MM_SATCOMopen )
	{
		pObj = m_main->SearchHuman();
		if ( pObj == 0 )
		{
			m_type = MM_NONE;  // c'est d�j� fini !
			return TRUE;
		}

		motion = pObj->RetMotion();
		if ( motion != 0 )
		{
			motion->SetAction(MHS_SATCOM, 0.5f);  // lit le SatCom
		}

		m_camera->RetCamera(m_initialEye, m_initialLookat);
		m_camera->SetType(CAMERA_SCRIPT);
		m_camera->SetSmooth(CS_HARD);
		m_camera->SetScriptEye(m_initialEye);
		m_camera->SetScriptLookat(m_initialLookat);
		m_camera->FixCamera();

		mat = pObj->RetWorldMatrix(0);
		m_finalLookat[0] = Transform(*mat, D3DVECTOR( 1.6f, 1.0f, 1.2f));
		m_finalEye[0]    = Transform(*mat, D3DVECTOR(-1.5f, 5.0f, 3.0f));
		m_finalLookat[1] = Transform(*mat, D3DVECTOR( 1.6f, 1.0f, 1.2f));
		m_finalEye[1]    = Transform(*mat, D3DVECTOR( 0.8f, 3.0f, 0.8f));
	}

	if ( m_type == MM_SATCOMclose )
	{
		pObj = m_main->SearchHuman();
		if ( pObj != 0 )
		{
			motion = pObj->RetMotion();
			if ( motion != 0 )
			{
				motion->SetAction(-1);  // termine lecture du SatCom
			}
		}

		m_camera->SetType(CAMERA_BACK);
		m_type = MM_NONE;  // c'est d�j� fini !
	}

	return TRUE;
}

// Stoppe un film en cours.

BOOL CMainMovie::Stop()
{
	CObject*	pObj;
	CMotion*	motion;

	if ( m_type == MM_SATCOMopen )
	{
		pObj = m_main->SearchHuman();
		if ( pObj != 0 )
		{
			motion = pObj->RetMotion();
			if ( motion != 0 )
			{
				motion->SetAction(-1);  // termine lecture du SatCom
			}
		}
	}

	m_type = MM_NONE;
	return TRUE;
}

// Indique si un film est en cours.

BOOL CMainMovie::IsExist()
{
	return (m_type != MM_NONE);
}


// Traite un �v�nement.

BOOL CMainMovie::EventProcess(const Event &event)
{
	D3DVECTOR	initialEye, initialLookat, finalEye, finalLookat, eye, lookat;
	float		progress;

	if ( m_type == MM_NONE )  return TRUE;

	m_progress += event.rTime*m_speed;

	if ( m_type == MM_SATCOMopen )
	{
		if ( m_progress < 1.0f )
		{
			progress = 1.0f-powf(1.0f-m_progress, 3.0f);

			if ( progress < 0.6f )
			{
				progress = progress/0.6f;
				initialEye    = m_initialEye;
				initialLookat = m_initialLookat;
				finalEye      = m_finalEye[0];
				finalLookat   = m_finalLookat[0];
			}
			else
			{
				progress = (progress-0.6f)/0.3f;
				initialEye    = m_finalEye[0];
				initialLookat = m_finalLookat[0];
				finalEye      = m_finalEye[1];
				finalLookat   = m_finalLookat[1];
			}
			if ( progress > 1.0f )  progress = 1.0f;

			eye = (finalEye-initialEye)*progress+initialEye;
			lookat = (finalLookat-initialLookat)*progress+initialLookat;
			m_camera->SetScriptEye(eye);
			m_camera->SetScriptLookat(lookat);
//			m_camera->FixCamera();
		}
		else
		{
			m_stopType = m_type;
			Flush();
			return FALSE;
		}
	}

	if ( m_type == MM_SATCOMclose )
	{
		if ( m_progress < 1.0f )
		{
		}
		else
		{
			m_stopType = m_type;
			Flush();
			return FALSE;
		}
	}

	return TRUE;
}


// Retourne le type du film en cours.

MainMovieType CMainMovie::RetType()
{
	return m_type;
}

// Retourne le type du film stopp�.

MainMovieType CMainMovie::RetStopType()
{
	return m_stopType;
}


