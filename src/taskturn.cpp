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
#include "physics.h"
#include "brain.h"
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
	if ( m_engine->RetPause() )  return TRUE;
	if ( event.event != EVENT_FRAME )  return TRUE;

	// Objet momentanément immobile (fourmi sur le dos) ?
	if ( m_object->RetFixed() )
	{
		m_physics->SetMotorSpeedX(0.0f);  // stoppe l'avance
		m_physics->SetMotorSpeedZ(0.0f);  // stoppe la rotation
		m_bError = TRUE;
		return TRUE;
	}

	return TRUE;
}


// Assigne le but à atteindre.
// Un angle positif effectue un virage à droite.

Error CTaskTurn::Start(float angle)
{
	m_startAngle = m_object->RetAngleY(0);
	m_finalAngle = m_startAngle+angle;

	if ( angle < 0.0f )
	{
		m_angle = angle+m_physics->RetCirStopLength();
		m_physics->SetMotorSpeedZ(-1.0f);  // tourne à gauche
		m_bLeft = TRUE;
	}
	else
	{
		m_angle = angle-m_physics->RetCirStopLength();
		m_physics->SetMotorSpeedZ(1.0f);  // tourne à droite
		m_bLeft = FALSE;
	}
	m_physics->SetMotorSpeedX(0.0f);
	m_physics->SetMotorSpeedY(0.0f);

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

	if ( m_bLeft )
	{
		if ( angle <= m_startAngle+m_angle )
		{
			m_physics->SetMotorSpeedZ(0.0f);
//?			m_physics->SetCirMotionY(MO_MOTSPEED, 0.0f);
			m_physics->SetCirMotionY(MO_CURSPEED, 0.0f);
//?			m_physics->SetCirMotionY(MO_REASPEED, 0.0f);
			m_object->SetAngleY(0, m_finalAngle);
			return ERR_STOP;
		}
	}
	else
	{
		if ( angle >= m_startAngle+m_angle )
		{
			m_physics->SetMotorSpeedZ(0.0f);
//?			m_physics->SetCirMotionY(MO_MOTSPEED, 0.0f);
			m_physics->SetCirMotionY(MO_CURSPEED, 0.0f);
//?			m_physics->SetCirMotionY(MO_REASPEED, 0.0f);
			m_object->SetAngleY(0, m_finalAngle);
			return ERR_STOP;
		}
	}

	return ERR_CONTINUE;
}


