// taskgungoal.cpp

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
#include "object.h"
#include "sound.h"
#include "task.h"
#include "taskgungoal.h"




// Constructeur de l'objet.

CTaskGunGoal::CTaskGunGoal(CInstanceManager* iMan, CObject* object)
						  : CTask(iMan, object)
{
	CTask::CTask(iMan, object);
}

// Destructeur de l'objet.

CTaskGunGoal::~CTaskGunGoal()
{
}


// Gestion d'un événement.

BOOL CTaskGunGoal::EventProcess(const Event &event)
{
	float		dir;

	if ( m_engine->RetPause() )  return TRUE;
	if ( event.event != EVENT_FRAME )  return TRUE;

	m_progress += event.rTime*m_speed;

	if ( m_progress < 1.0f )
	{
		dir = m_initialDirV + (m_finalDirV-m_initialDirV)*m_progress;
	}
	else
	{
		dir = m_finalDirV;
	}
	m_object->SetGunGoalV(dir);

	if ( m_progress < 1.0f )
	{
		dir = m_initialDirH + (m_finalDirH-m_initialDirH)*m_progress;
	}
	else
	{
		dir = m_finalDirH;
	}
	m_object->SetGunGoalH(dir);

	return TRUE;
}


// Assigne le but à atteindre.

Error CTaskGunGoal::Start(float dirV, float dirH)
{
	float	speedV, speedH;
	int		i;

	m_initialDirV = m_object->RetGunGoalV();
	m_object->SetGunGoalV(dirV);
	m_finalDirV = m_object->RetGunGoalV();  // direction possible
	m_object->SetGunGoalV(m_initialDirV);  // remet direction initiale

	if ( m_finalDirV == m_initialDirV )
	{
		speedV = 100.0f;
	}
	else
	{
		speedV = 1.0f/(Abs(m_finalDirV-m_initialDirV)*1.0f);
	}

	m_initialDirH = m_object->RetGunGoalH();
	m_object->SetGunGoalH(dirH);
	m_finalDirH = m_object->RetGunGoalH();  // direction possible
	m_object->SetGunGoalH(m_initialDirH);  // remet direction initiale

	if ( m_finalDirH == m_initialDirH )
	{
		speedH = 100.0f;
	}
	else
	{
		speedH = 1.0f/(Abs(m_finalDirH-m_initialDirH)*1.0f);
	}

	m_speed = Min(speedV, speedH);

	if ( m_finalDirV != m_initialDirV ||
		 m_finalDirH != m_initialDirH )
	{
		i = m_sound->Play(SOUND_MANIP, m_object->RetPosition(0), 0.3f, 1.5f, TRUE);
		m_sound->AddEnvelope(i, 0.3f, 1.5f, 1.0f/m_speed, SOPER_STOP);
	}

	m_progress = 0.0f;

	return ERR_OK;
}

// Indique si l'action est terminée.

Error CTaskGunGoal::IsEnded()
{
	if ( m_engine->RetPause() )  return ERR_CONTINUE;

	if ( m_initialDirV == m_finalDirV &&
		 m_initialDirH == m_finalDirH )  return ERR_STOP;
	if ( m_progress < 1.0f )  return ERR_CONTINUE;

	m_object->SetGunGoalV(m_finalDirV);
	m_object->SetGunGoalH(m_finalDirH);
	Abort();
	return ERR_STOP;
}

// Termine brutalement l'action en cours.

BOOL CTaskGunGoal::Abort()
{
	return TRUE;
}

