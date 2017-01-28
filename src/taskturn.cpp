// taskturn.cpp

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
#include "motionbot.h"
#include "motiongun.h"
#include "sound.h"
#include "mainundo.h"
#include "task.h"
#include "taskturn.h"




// Constructeur de l'objet.

CTaskTurn::CTaskTurn(CInstanceManager* iMan, CObject* object)
					 : CTask(iMan, object)
{
	CTask::CTask(iMan, object);
}

// Destructeur de l'objet.

CTaskTurn::~CTaskTurn()
{
}


// Gestion d'un événement.

BOOL CTaskTurn::EventProcess(const Event &event)
{
	float	a, g, speed;

	if ( m_engine->RetPause() )  return TRUE;
	if ( event.event != EVENT_FRAME )  return TRUE;

	if ( m_initialWait > 0.0f )
	{
		m_initialWait -= event.rTime;
		if ( m_initialWait <= 0.0f )
		{
			m_sound->Play(SOUND_CRAZY, m_object->RetPosition(0), 1.0f);
		}
		return TRUE;
	}

//?	ProgressCirSpeed(1.0f);

	a = m_object->RetAngleY(0);
	g = m_finalAngle;
	speed = m_motion->RetCirSpeed();
	if ( g-a > PI )  g -= PI*2.0f;
	if ( a-g > PI )  a -= PI*2.0f;
	if ( a < g )
	{
		a += speed*event.rTime;  // rotation CCW
		if ( a > g )  a = g;
	}
	else
	{
		a -= speed*event.rTime;  // rotation CW
		if ( a < g )  a = g;
	}
	m_object->SetAngleY(0, NormAngle(a));

	return TRUE;
}


// Assigne le but à atteindre.
// Un angle positif effectue un virage à droite.

Error CTaskTurn::Start(float angle)
{
	m_type = m_object->RetType();

	m_startAngle = m_object->RetAngleY(0);
	m_finalAngle = NormAngle(m_startAngle+angle);

	ProgressLinSpeed(0.0f);
	ProgressCirSpeed(angle>0.0f?0.5f:-0.5f);

	m_initialWait = 0.0f;
	if ( m_object->RetType() == OBJECT_CRAZY )
	{
		m_initialWait = 1.0f;
	}

	if ( m_type == OBJECT_TRAX  ||
		 m_type == OBJECT_PERFO ||
		 m_type == OBJECT_GUN   )
	{
		m_object->SetLock(TRUE);
	}

	m_bError = FALSE;
	return ERR_OK;
}

// Indique si l'action est terminée.

Error CTaskTurn::IsEnded()
{
	float	angle;

	if ( m_engine->RetPause() )  return ERR_CONTINUE;

	if ( m_bError )
	{
		return ERR_STOP;
	}

	angle = m_object->RetAngleY(0);
	if ( angle == m_finalAngle )
	{
		ProgressLinSpeed(0.0f);
		ProgressCirSpeed(0.0f);

		if ( m_type == OBJECT_GUN )
		{
			StartAction(m_object, MGUN_FIRE);
		}

		if ( m_type == OBJECT_TRAX  ||
			 m_type == OBJECT_PERFO ||
			 m_type == OBJECT_GUN   )
		{
			m_object->SetLock(FALSE);
		}
		return ERR_STOP;
	}

	return ERR_CONTINUE;
}


// Indique si l'action est annulable.

BOOL CTaskTurn::IsUndoable()
{
	return ( m_type == OBJECT_CRAZY );
}


// Spécifie la vitesse linéaire pour une action pour Blupi.

void CTaskTurn::ProgressLinSpeed(float speed)
{
	CMotion*	motion;

	motion = m_object->RetMotion();
	if ( motion == 0 )  return;

	motion->SetActionLinSpeed(speed);
}

// Spécifie la vitesse circulaire pour une action pour Blupi.

void CTaskTurn::ProgressCirSpeed(float speed)
{
	CMotion*	motion;

	motion = m_object->RetMotion();
	if ( motion == 0 )  return;

	motion->SetActionCirSpeed(speed);
}


// Démarre une action pour un objet.

void CTaskTurn::StartAction(CObject* pObj, int action)
{
	CMotion*	motion;

	motion = pObj->RetMotion();
	if ( motion == 0 )  return;

	motion->SetAction(action);
}


// Ecrit la situation de l'objet.

void CTaskTurn::WriteSituation()
{
	m_undo->WriteTokenFloat("wait", m_initialWait);
	m_undo->WriteTokenFloat("start", m_startAngle);
	m_undo->WriteTokenFloat("final", m_finalAngle);
}

// lit la situation de l'objet.

void CTaskTurn::ReadSituation()
{
	m_undo->ReadTokenFloat("wait", m_initialWait);
	m_undo->ReadTokenFloat("start", m_startAngle);
	m_undo->ReadTokenFloat("final", m_finalAngle);
}

