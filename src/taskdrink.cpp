// taskdrink.cpp

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
#include "pyro.h"
#include "sound.h"
#include "robotmain.h"
#include "task.h"
#include "taskdrink.h"




// Constructeur de l'objet.

CTaskDrink::CTaskDrink(CInstanceManager* iMan, CObject* object)
					 : CTask(iMan, object)
{
	CTask::CTask(iMan, object);

	m_time = 0.0f;
	m_bError= TRUE;
}

// Destructeur de l'objet.

CTaskDrink::~CTaskDrink()
{
}


// Gestion d'un événement.

BOOL CTaskDrink::EventProcess(const Event &event)
{
	D3DVECTOR	pos;
	float		progress;

	if ( m_engine->RetPause() )  return TRUE;
	if ( event.event != EVENT_FRAME )  return TRUE;
	if ( m_bError )  return FALSE;

	m_time += event.rTime;
	m_progress += event.rTime*m_speed;
	progress = Norm(m_progress);

	if ( m_phase == TDR_TAKE )
	{
		pos = LineProgress(m_startPos, m_goalPos, progress*0.4f);
		pos.y = m_startPos.y;
		m_object->SetPosition(0, pos);
	}

	if ( m_phase == TDR_DRINK )
	{
		pos = LineProgress(m_startPos, m_goalPos, 0.4f+progress*0.6f);
		pos.y = m_startPos.y;
		m_object->SetPosition(0, pos);
	}

	return TRUE;
}


// Assigne le but à atteindre.

Error CTaskDrink::Start(CObject *fiole)
{
	m_main->IncTotalManip();

	if ( fiole == 0 )
	{
		m_bError = TRUE;
		return ERR_GENERIC;
	}

	m_fiole = fiole;
	m_goalPos = m_fiole->RetPosition(0);
	m_startPos = m_object->RetPosition(0);

	if ( m_object->RetStrong() != 0.0f )  // déjà costaud/glu ?
	{
		StartAction(MBLUPI_ERROR);
		m_bError = TRUE;
		return ERR_GENERIC;
	}

	m_fiole->SetLock(TRUE);

	m_time = 0.0f;
	m_phase = TDR_TAKE;
	m_progress = 0.0f;
	m_speed = 1.0f/0.5f;
	m_bError = FALSE;

	StartAction(MBLUPI_DRINK1);
	return ERR_OK;
}

// Indique si l'action est terminée.

Error CTaskDrink::IsEnded()
{
	CPyro*		pyro;
	D3DMATRIX*	mat;
	D3DVECTOR	pos;

	if ( m_engine->RetPause() )  return ERR_CONTINUE;

	if ( m_bError )
	{
		return ERR_STOP;
	}

	if ( m_progress < 1.0f )
	{
		return ERR_CONTINUE;
	}

	if ( m_phase == TDR_TAKE )
	{
		m_object->SetFret(m_fiole);
		m_fiole->SetTruck(m_object);
		m_fiole->SetTruckPart(9);  // main droite
		m_fiole->SetPosition(0, D3DVECTOR(0.0f, -1.6f, -1.9f));
		m_fiole->SetAngleX(0,  80.0f*PI/180.0f);
		m_fiole->SetAngleY(0, -20.0f*PI/180.0f);

		if ( m_fiole->RetType() == OBJECT_FIOLE )
		{
			m_object->SetStrong(1.0f);
		}
		else
		{
			m_object->SetStrong(-1.0f);
		}
		m_sound->Play(SOUND_BLUPIslurp, m_object->RetPosition(0), 1.0f);
		StartAction(MBLUPI_DRINK2);

		m_phase = TDR_DRINK;
		m_progress = 0.0f;
		m_speed = 1.0f/3.0f;
		return ERR_CONTINUE;
	}

	if ( m_phase == TDR_DRINK )
	{
		mat = m_object->RetWorldMatrix(0);
		pos = Transform(*mat, D3DVECTOR(5.5f, 3.0f, 0.0f));
		m_object->SetFret(0);
		m_fiole->SetTruck(0);
		m_fiole->SetPosition(0, pos);

		m_fiole->DeletePart(1);  // supprime le bouton-action
		pyro = new CPyro(m_iMan);
		pyro->Create(PT_FRAGG, m_fiole);  // destruction fiole
		m_fiole = 0;

		m_terrain->SetLockZone(m_startPos, LZ_FREE);
		m_terrain->SetLockZone(m_goalPos, LZ_BLUPI);

		return ERR_STOP;
	}

	return ERR_STOP;
}


// Démarre une action pour Blupi.

void CTaskDrink::StartAction(int action, float speed)
{
	CMotion*	motion;

	motion = m_object->RetMotion();
	if ( motion == 0 )  return;

	motion->SetAction(action, speed);
}

