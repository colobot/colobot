// taskspiderexplo.cpp

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
#include "physics.h"
#include "pyro.h"
#include "motion.h"
#include "motionspider.h"
#include "task.h"
#include "taskspiderexplo.h"




// Constructeur de l'objet.

CTaskSpiderExplo::CTaskSpiderExplo(CInstanceManager* iMan, CObject* object)
						  : CTask(iMan, object)
{
	CTask::CTask(iMan, object);

	m_time = 0.0f;
	m_bError = FALSE;
}

// Destructeur de l'objet.

CTaskSpiderExplo::~CTaskSpiderExplo()
{
}


// Gestion d'un événement.

BOOL CTaskSpiderExplo::EventProcess(const Event &event)
{
	if ( m_engine->RetPause() )  return TRUE;
	if ( event.event != EVENT_FRAME )  return TRUE;

	// Objet momentanément immobile (fourmi sur le dos) ?
	if ( m_object->RetFixed() )
	{
		m_bError = TRUE;
		return TRUE;
	}

	m_time += event.rTime;

	return TRUE;
}


// Assigne le but à atteindre.

Error CTaskSpiderExplo::Start()
{
	m_motion->SetAction(MSS_EXPLO, 1.0f);  // l'abdomen gonfle
	m_time = 0.0f;

	m_physics->SetMotorSpeedX(0.0f);  // stoppe l'avance
	m_physics->SetMotorSpeedZ(0.0f);  // stoppe la rotation

	m_bError = FALSE;
	return ERR_OK;
}

// Indique si l'action est terminée.

Error CTaskSpiderExplo::IsEnded()
{
	CPyro*		pyro;

	if ( m_engine->RetPause() )  return ERR_CONTINUE;

	if ( m_bError )
	{
		Abort();
		return ERR_STOP;
	}

	if ( m_time < 1.0f )  return ERR_CONTINUE;

	pyro = new CPyro(m_iMan);
	pyro->Create(PT_SPIDER, m_object);  // l'araignée explose (suicide)

	Abort();
	return ERR_STOP;
}

// Termine brutalement l'action en cours.

BOOL CTaskSpiderExplo::Abort()
{
	return TRUE;
}

