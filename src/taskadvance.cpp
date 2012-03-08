// taskadvance.cpp

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
#include "taskadvance.h"




// Constructeur de l'objet.

CTaskAdvance::CTaskAdvance(CInstanceManager* iMan, CObject* object)
						   : CTask(iMan, object)
{
	CTask::CTask(iMan, object);
}

// Destructeur de l'objet.

CTaskAdvance::~CTaskAdvance()
{
}


// Gestion d'un événement.

BOOL CTaskAdvance::EventProcess(const Event &event)
{
	if ( m_engine->RetPause() )  return TRUE;
	if ( event.event != EVENT_FRAME )  return TRUE;

	m_fixTime += event.rTime;

	// Objet momentanément immobile (fourmi sur le dos) ?
	if ( m_object->RetFixed() )
	{
		m_physics->SetMotorSpeedX(0.0f);  // stoppe l'avance
		m_physics->SetMotorSpeedZ(0.0f);  // stoppe la rotation
		m_bError = TRUE;
		return TRUE;
	}

	m_timeLimit -= event.rTime;
	return TRUE;
}


// Assigne le but à atteindre.

Error CTaskAdvance::Start(float length)
{
	m_direction = (length>=0.0f)?1.0f:-1.0f;
	m_totalLength = Abs(length);
	m_advanceLength = m_physics->RetLinLength(length);
	m_startPos = m_object->RetPosition(0);
	m_lastDist = 0.0f;
	m_fixTime = 0.0f;

	m_timeLimit = m_physics->RetLinTimeLength(m_totalLength, m_direction)*3.0f;
	if ( m_timeLimit < 2.0f )  m_timeLimit = 2.0f;

	m_physics->SetMotorSpeedX(m_direction*1.0f);  // avance/recule
	m_physics->SetMotorSpeedY(0.0f);
	m_physics->SetMotorSpeedZ(0.0f);

	m_bError = FALSE;
	return ERR_OK;
}

// Indique si l'action est terminée.

Error CTaskAdvance::IsEnded()
{
	D3DVECTOR	pos;
	float		length;

	if ( m_engine->RetPause() )  return ERR_CONTINUE;

	if ( m_bError )
	{
		return ERR_STOP;
	}

	if ( m_timeLimit < 0.0f )
	{
		m_physics->SetMotorSpeedX(0.0f);
		return ERR_MOVE_IMPOSSIBLE;
	}

	pos = m_object->RetPosition(0);
	length = Length2d(pos, m_startPos);

	if ( length > m_lastDist )  // avance ?
	{
		m_fixTime = 0.0f;
	}
	else	// n'avance plus ?
	{
		if ( m_fixTime > 1.0f )  // depuis plus d'une seconde ?
		{
			m_physics->SetMotorSpeedX(0.0f);
			return ERR_MOVE_IMPOSSIBLE;
		}
	}
	m_lastDist = length;

	if ( length >= m_totalLength )
	{
		m_physics->SetMotorSpeedX(0.0f);
		m_physics->SetLinMotionX(MO_CURSPEED, 0.0f);

		if ( length != 0.0f )
		{
			pos = m_startPos+((pos-m_startPos)*m_totalLength/length);
			m_object->SetPosition(0, pos);
		}
		return ERR_STOP;
	}

	if ( length >= m_advanceLength )
	{
		m_physics->SetMotorSpeedX(m_direction*0.1f);
	}
	return ERR_CONTINUE;
}


