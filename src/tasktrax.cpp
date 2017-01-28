// tasktrax.cpp

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
#include "motion.h"
#include "motionblupi.h"
#include "motiontrax.h"
#include "auto.h"
#include "sound.h"
#include "robotmain.h"
#include "task.h"
#include "tasklist.h"
#include "tasktrax.h"




// Constructeur de l'objet.

CTaskTrax::CTaskTrax(CInstanceManager* iMan, CObject* object)
					 : CTask(iMan, object)
{
	CTask::CTask(iMan, object);

	m_time = 0.0f;
	m_bError= TRUE;
}

// Destructeur de l'objet.

CTaskTrax::~CTaskTrax()
{
}


// Gestion d'un événement.

BOOL CTaskTrax::EventProcess(const Event &event)
{
	D3DMATRIX*	mat;
	CObject*	pBox;
	D3DVECTOR	pos, dir;
	float		progress;
	int			part;

	if ( m_engine->RetPause() )  return TRUE;
	if ( event.event != EVENT_FRAME )  return TRUE;
	if ( m_bError )  return FALSE;

	m_time += event.rTime;
	m_progress += event.rTime*m_speed;
	progress = Norm(m_progress);

	if ( progress >= 0.67f && !m_bAuto )
	{
		m_bAuto = TRUE;

		if ( m_part == 3 )  // avance ?
		{
			mat = m_trax->RetWorldMatrix(0);
			pos = Transform(*mat, D3DVECTOR(8.0f, 0.0f, 0.0f));
			pBox = SearchBox(pos, 1.0f);
			if ( pBox != 0 )
			{
				dir = pBox->RetPosition(0)-m_object->RetPosition(0);
				part = 0;
				if ( dir.x >  7.0f )  part = 1;  // ouest
				if ( dir.z < -7.0f )  part = 2;  // nord
				if ( dir.x < -7.0f )  part = 3;  // est
				if ( dir.z >  7.0f )  part = 4;  // sud
				if ( part != 0 )
				{
					m_trax->StartTaskList(TO_PUSH, D3DVECTOR(0.0f, 0.0f, 0.0f), 0, part, 0.0f);
				}
			}
			else
			{
				m_trax->StartTaskList(TO_MOVE, D3DVECTOR(0.0f, 0.0f, 0.0f), 0, 0, 8.0f);
			}
		}

		if ( m_part == 4 )  // bouton gauche (tourne à droite) ?
		{
			m_trax->StartTaskList(TO_TURN, D3DVECTOR(0.0f, 0.0f, 0.0f), 0, 0, PI/2.0f);
		}

		if ( m_part == 5 )  // bouton droite (tourne à gauche) ?
		{
			m_trax->StartTaskList(TO_TURN, D3DVECTOR(0.0f, 0.0f, 0.0f), 0, 0, -PI/2.0f);
		}
	}

	return TRUE;
}


// Assigne le but à atteindre.

Error CTaskTrax::Start(CObject *trax, int part)
{
	m_main->IncTotalManip();

	if ( trax == 0 )
	{
		m_bError = TRUE;
		return ERR_GENERIC;
	}

	m_trax = trax;
	m_part = part;
	m_time = 0.0f;
	m_progress = 0.0f;
	m_speed = 1.0f/1.5f;
	m_bAuto = FALSE;
	m_bError = FALSE;

//?	m_sound->Play(SOUND_BLUPIohhh, m_object->RetPosition(0), 1.0f);
	StartAction(MBLUPI_TRAX);
	return ERR_OK;
}

// Indique si l'action est terminée.

Error CTaskTrax::IsEnded()
{
	if ( m_engine->RetPause() )  return ERR_CONTINUE;

	if ( m_bError )
	{
		return ERR_STOP;
	}

	if ( m_progress >= 1.0f )
	{
		return ERR_STOP;
	}

	return ERR_CONTINUE;
}


// Cherche un objet quelconque.

CObject* CTaskTrax::SearchBox(D3DVECTOR center, float radius)
{
	CObject		*pObj, *pBest;
	D3DVECTOR	pos;
	ObjectType	type;
	float		min, dist;
	int			i;

	pBest = 0;
	min = 100000.0f;
	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;
		if ( pObj == m_object )  continue;  // soi-même ?
		if ( pObj->RetExplo() )  continue;
		if ( pObj->RetLock() )  continue;
		if ( !pObj->RetEnable() )  continue;

		type = pObj->RetType();
		if ( (type < OBJECT_BOX1 || type > OBJECT_BOX20) &&
			 (type < OBJECT_KEY1 || type > OBJECT_KEY5 ) )  continue;

		pos = pObj->RetPosition(0);
		dist = Length2d(pos, center);

		if ( dist <= radius && dist < min )
		{
			min = dist;
			pBest = pObj;
		}
	}
	return pBest;
}


// Démarre une action pour un objet.

void CTaskTrax::StartAction(int action, float speed)
{
	CMotion*	motion;

	motion = m_object->RetMotion();
	if ( motion == 0 )  return;

	motion->SetAction(action, speed);
}

