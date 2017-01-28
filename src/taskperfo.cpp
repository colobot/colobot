// taskperfo.cpp

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
#include "motionperfo.h"
#include "auto.h"
#include "sound.h"
#include "robotmain.h"
#include "task.h"
#include "tasklist.h"
#include "taskperfo.h"




// Constructeur de l'objet.

CTaskPerfo::CTaskPerfo(CInstanceManager* iMan, CObject* object)
					 : CTask(iMan, object)
{
	CTask::CTask(iMan, object);

	m_time = 0.0f;
	m_bError= TRUE;
}

// Destructeur de l'objet.

CTaskPerfo::~CTaskPerfo()
{
}


// Gestion d'un événement.

BOOL CTaskPerfo::EventProcess(const Event &event)
{
	float		progress;

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
			m_perfo->StartTaskList(TO_MOVE, D3DVECTOR(0.0f, 0.0f, 0.0f), 0, 0, 8.0f);
		}

		if ( m_part == 4 )  // bouton gauche (tourne à droite) ?
		{
			m_perfo->StartTaskList(TO_TURN, D3DVECTOR(0.0f, 0.0f, 0.0f), 0, 0, PI/2.0f);
		}

		if ( m_part == 5 )  // bouton droite (tourne à gauche) ?
		{
			m_perfo->StartTaskList(TO_TURN, D3DVECTOR(0.0f, 0.0f, 0.0f), 0, 0, -PI/2.0f);
		}
	}

	return TRUE;
}


// Assigne le but à atteindre.

Error CTaskPerfo::Start(CObject *perfo, int part)
{
	m_main->IncTotalManip();

	if ( perfo == 0 )
	{
		m_bError = TRUE;
		return ERR_GENERIC;
	}

	m_perfo = perfo;
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

Error CTaskPerfo::IsEnded()
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


// Démarre une action pour un objet.

void CTaskPerfo::StartAction(int action, float speed)
{
	CMotion*	motion;

	motion = m_object->RetMotion();
	if ( motion == 0 )  return;

	motion->SetAction(action, speed);
}

