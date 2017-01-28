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
#include "taskpush.h"
#include "taskroll.h"
#include "taskdock.h"
#include "taskcatapult.h"
#include "tasktrax.h"
#include "taskperfo.h"
#include "taskgun.h"
#include "taskdrink.h"
#include "taskgoal.h"
#include "taskdive.h"
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

Error CTaskManager::StartTaskMove(float length, BOOL bNoError)
{
	m_task = new CTaskAdvance(m_iMan, m_object);
	return ((CTaskAdvance*)m_task)->Start(length, bNoError);
}

// Tourne d'un certain angle.

Error CTaskManager::StartTaskTurn(float angle)
{
	m_task = new CTaskTurn(m_iMan, m_object);
	return ((CTaskTurn*)m_task)->Start(angle);
}

// Atteint une position donnée.

Error CTaskManager::StartTaskGoto(D3DVECTOR pos, CObject *target, int part)
{
	m_task = new CTaskGoto(m_iMan, m_object);
	return ((CTaskGoto*)m_task)->Start(pos, target, part);
}

// Pousse une caisse.

Error CTaskManager::StartTaskPush(int part, int nbTiles)
{
	m_task = new CTaskPush(m_iMan, m_object);
	return ((CTaskPush*)m_task)->Start(part, nbTiles);
}

// Roule une sphère.

Error CTaskManager::StartTaskRoll(D3DVECTOR dir)
{
	m_task = new CTaskRoll(m_iMan, m_object);
	return ((CTaskRoll*)m_task)->Start(dir);
}

// Manoeuvre un dock.

Error CTaskManager::StartTaskDock(CObject *dock, int part)
{
	m_task = new CTaskDock(m_iMan, m_object);
	return ((CTaskDock*)m_task)->Start(dock, part);
}

// Manoeuvre une catapulte.

Error CTaskManager::StartTaskCatapult(CObject *catapult, int part)
{
	m_task = new CTaskCatapult(m_iMan, m_object);
	return ((CTaskCatapult*)m_task)->Start(catapult, part);
}

// Manoeuvre un trax.

Error CTaskManager::StartTaskTrax(CObject *trax, int part)
{
	m_task = new CTaskTrax(m_iMan, m_object);
	return ((CTaskTrax*)m_task)->Start(trax, part);
}

// Manoeuvre une perforatrice.

Error CTaskManager::StartTaskPerfo(CObject *perfo, int part)
{
	m_task = new CTaskPerfo(m_iMan, m_object);
	return ((CTaskPerfo*)m_task)->Start(perfo, part);
}

// Manoeuvre un canon.

Error CTaskManager::StartTaskGun(CObject *gun, int part)
{
	m_task = new CTaskGun(m_iMan, m_object);
	return ((CTaskGun*)m_task)->Start(gun, part);
}

// Boit une fiole.

Error CTaskManager::StartTaskDrink(CObject *fiole)
{
	m_task = new CTaskDrink(m_iMan, m_object);
	return ((CTaskDrink*)m_task)->Start(fiole);
}

// S'envole avec un ballon.

Error CTaskManager::StartTaskGoal(CObject *goal)
{
	m_task = new CTaskGoal(m_iMan, m_object);
	return ((CTaskGoal*)m_task)->Start(goal);
}

// Plonge.

Error CTaskManager::StartTaskDive(CObject *dive)
{
	m_task = new CTaskDive(m_iMan, m_object);
	return ((CTaskDive*)m_task)->Start(dive);
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


// Indique si la tâche en cours est annulable.

BOOL CTaskManager::IsUndoable()
{
	if ( m_task == 0 )  return FALSE;
	return m_task->IsUndoable();
}

// Indique si la tâche en cours est stoppable.

BOOL CTaskManager::IsStopable()
{
	if ( m_task == 0 )  return FALSE;
	return m_task->IsStopable();
}

// Stoppe proprement la tâche en cours.

BOOL CTaskManager::Stop()
{
	if ( m_task == 0 )  return FALSE;
	return m_task->Stop();
}


// Termine brutalement l'action en cours.

BOOL CTaskManager::Abort()
{
	if ( m_task == 0 )  return FALSE;
	return m_task->Abort();
}


// Retourne la tache en cours.

CTask* CTaskManager::RetRunningTask()
{
	return m_task;
}

