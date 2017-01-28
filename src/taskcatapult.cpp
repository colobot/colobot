// taskcatapult.cpp

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
#include "auto.h"
#include "autodock.h"
#include "sound.h"
#include "robotmain.h"
#include "task.h"
#include "taskcatapult.h"




// Constructeur de l'objet.

CTaskCatapult::CTaskCatapult(CInstanceManager* iMan, CObject* object)
					 : CTask(iMan, object)
{
	CTask::CTask(iMan, object);

	m_time = 0.0f;
	m_bError= TRUE;
}

// Destructeur de l'objet.

CTaskCatapult::~CTaskCatapult()
{
}


// Gestion d'un événement.

BOOL CTaskCatapult::EventProcess(const Event &event)
{
	CAutoDock*	ad;
	float		progress;

	if ( m_engine->RetPause() )  return TRUE;
	if ( event.event != EVENT_FRAME )  return TRUE;
	if ( m_bError )  return FALSE;

	m_time += event.rTime;
	m_progress += event.rTime*m_speed;
	progress = Norm(m_progress);

	if ( progress >= 0.5f && !m_bAuto )
	{
		m_bAuto = TRUE;

		ad = (CAutoDock*)m_dock->RetAuto();
		if ( ad != 0 )
		{
			if ( !ad->Start(m_part) )  // démarre la catapulte
			{
				StartAction(MBLUPI_ERROR);
			}
		}
	}

	return TRUE;
}


// Assigne le but à atteindre.

Error CTaskCatapult::Start(CObject *dock, int part)
{
	m_main->IncTotalManip();

	if ( dock == 0 )
	{
		m_bError = TRUE;
		return ERR_GENERIC;
	}

	m_dock = dock;
	m_part = part;
	m_time = 0.0f;
	m_progress = 0.0f;
	m_speed = 1.0f/2.0f;
	m_bAuto = FALSE;
	m_bError = FALSE;

//?	m_sound->Play(SOUND_BLUPIohhh, m_object->RetPosition(0), 1.0f);
	StartAction(MBLUPI_ROLL);
	return ERR_OK;
}

// Indique si l'action est terminée.

Error CTaskCatapult::IsEnded()
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


// Démarre une action pour Blupi.

void CTaskCatapult::StartAction(int action, float speed)
{
	CMotion*	motion;

	motion = m_object->RetMotion();
	if ( motion == 0 )  return;

	motion->SetAction(action, speed);
}

