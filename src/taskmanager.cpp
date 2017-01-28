// taskmanager.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "D3DEngine.h"
#include "misc.h"
#include "iman.h"
#include "event.h"
#include "object.h"
#include "task.h"
#include "taskwait.h"
#include "taskadvance.h"
#include "taskturn.h"
#include "taskgoto.h"
#include "taskfire.h"
#include "taskmanager.h"




// Constructeur de l'objet.

CTaskManager::CTaskManager(CInstanceManager* iMan, CObject* object)
{
	m_iMan = iMan;
	m_iMan->AddInstance(CLASS_TASKMANAGER, this, 100);

	m_task = 0;
	m_object = object;
	m_bPilot = FALSE;
}

// Destructeur de l'objet.

CTaskManager::~CTaskManager()
{
	delete m_task;
}



// Attend un certain temps.

Error CTaskManager::StartTaskWait(float time)
{
	m_task = new CTaskWait(m_iMan, m_object);
	return ((CTaskWait*)m_task)->Start(time);
}

// Avance droit devant d'une certaine distance.

Error CTaskManager::StartTaskAdvance(float length)
{
	m_task = new CTaskAdvance(m_iMan, m_object);
	return ((CTaskAdvance*)m_task)->Start(length);
}

// Tourne d'un certain angle.

Error CTaskManager::StartTaskTurn(float angle)
{
	m_task = new CTaskTurn(m_iMan, m_object);
	return ((CTaskTurn*)m_task)->Start(angle);
}

// Atteint une position donnée.

Error CTaskManager::StartTaskGoto(D3DVECTOR pos, float altitude, TaskGotoGoal goalMode, TaskGotoCrash crashMode)
{
	m_task = new CTaskGoto(m_iMan, m_object);
	return ((CTaskGoto*)m_task)->Start(pos, altitude, goalMode, crashMode);
}

// Tire.

Error CTaskManager::StartTaskFire(float delay)
{
	m_bPilot = TRUE;
	m_task = new CTaskFire(m_iMan, m_object);
	return ((CTaskFire*)m_task)->Start(delay);
}





// Gestion d'un événement.

BOOL CTaskManager::EventProcess(const Event &event)
{
	if ( m_task == 0 )  return FALSE;
	return m_task->EventProcess(event);
}


// Indique si l'action est terminée.

Error CTaskManager::IsEnded()
{
	if ( m_task == 0 )  return ERR_GENERIC;
	return m_task->IsEnded();
}


// Indique s'il est possible de piloter le robot pendant l'exécution
// de la tâche en cours.

BOOL CTaskManager::IsPilot()
{
	return m_bPilot;
}


// Termine brutalement l'action en cours.

BOOL CTaskManager::Abort()
{
	if ( m_task == 0 )  return FALSE;
	return m_task->Abort();
}


