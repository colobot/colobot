// tasklist.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "D3DEngine.h"
#include "misc.h"
#include "iman.h"
#include "restext.h"
#include "math3d.h"
#include "event.h"
#include "mainundo.h"
#include "object.h"
#include "motion.h"
#include "motionblupi.h"
#include "motiontrax.h"
#include "motionperfo.h"
#include "auto.h"
#include "robotmain.h"
#include "taskmanager.h"
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
#include "taskdrink.h"
#include "taskgoal.h"
#include "tasklist.h"




// Constructeur de l'objet.

CTaskList::CTaskList(CInstanceManager* iMan, CObject* object)
{
	m_iMan = iMan;
	m_iMan->AddInstance(CLASS_TASKLIST, this, 100);

	m_undo = (CMainUndo*)m_iMan->SearchInstance(CLASS_UNDO);
	m_main = (CRobotMain*)m_iMan->SearchInstance(CLASS_MAIN);

	m_object = object;

	m_taskOrder = TO_NULL;
	m_task = 0;
	m_bStopPending = FALSE;
	Flush();
	FlushStatistic();
}

// Destructeur de l'objet.

CTaskList::~CTaskList()
{
	delete m_task;
}


// Vide toute la liste.

void CTaskList::Flush()
{
	m_total = 0;
	m_head  = 0;
	m_tail  = 0;
	m_object->SetFuturStrong(m_object->RetStrong());
}

// Ajoute un ordre en tête du fifo.

BOOL CTaskList::AddHead(TaskOrder order, D3DVECTOR pos, int id, int part, float param)
{
	int		i;

	if ( m_total == MAXTASKLIST )  return FALSE;

	AddStatistic(order, pos, id, part, param);

	i = m_head;

	m_fifo[i].order = order;
	m_fifo[i].pos   = pos;
	m_fifo[i].id    = id;
	m_fifo[i].part  = part;
	m_fifo[i].param = param;

	i ++;
	if ( i == MAXTASKLIST )  i = 0;
	m_head = i;
	m_total ++;

	return TRUE;
}

// Ajoute un ordre en queue du fifo.

BOOL CTaskList::AddTail(TaskOrder order, D3DVECTOR pos, int id, int part, float param)
{
	int		i;

	if ( m_total == MAXTASKLIST )  return FALSE;

	i = m_tail;
	if ( i == 0 )  i = MAXTASKLIST;
	i --;

	m_fifo[i].order = order;
	m_fifo[i].pos   = pos;
	m_fifo[i].id    = id;
	m_fifo[i].part  = part;
	m_fifo[i].param = param;

	m_tail = i;
	m_total ++;

	return TRUE;
}

// Enlève un ordre à la tête du fifo.

BOOL CTaskList::SubHead(TaskOrder &order, D3DVECTOR &pos, int &id, int &part, float &param)
{
	int		i;

	if ( m_total == 0 )  return FALSE;

	i = m_head;
	if ( i == 0 )  i = MAXTASKLIST;
	i --;

	order = m_fifo[i].order;
	pos   = m_fifo[i].pos;
	id    = m_fifo[i].id;
	part  = m_fifo[i].part;
	param = m_fifo[i].param;

	m_head = i;
	m_total --;

	return TRUE;
}

// Enlève un ordre à la queue du fifo.

BOOL CTaskList::SubTail(TaskOrder &order, D3DVECTOR &pos, int &id, int &part, float &param)
{
	int		i;

	if ( m_total == 0 )  return FALSE;

	i = m_tail;

	order = m_fifo[i].order;
	pos   = m_fifo[i].pos;
	id    = m_fifo[i].id;
	part  = m_fifo[i].part;
	param = m_fifo[i].param;

	i ++;
	if ( i == MAXTASKLIST )  i = 0;
	m_tail = i;
	m_total --;

	return TRUE;
}


// Progression de la tâche en cours.

BOOL CTaskList::EventFrame(const Event &event)
{
	CObject*	target;
	TaskOrder	order;
	D3DVECTOR	pos, tPos;
	float		param;
	int			id, part;
	Error		err;

	if ( m_task == 0 )  // pas de tâche en cours ?
	{
		if ( SubTail(order, pos, id, part, param) )  // nouvelle tâche à commencer ?
		{
			err = ERR_GENERIC;

			target = IdSearch(id);  // cherche l'objet cible
			if ( order == TO_MOVE )  // avance ?
			{
				err = StartTaskMove(param, FALSE);
			}
			if ( order == TO_TURN )  // tourne ?
			{
				err = StartTaskTurn(param);
			}
			if ( order == TO_GOTO )  // va ?
			{
				UndoRecord();
				err = StartTaskGoto(pos, target, part);
				if ( err != ERR_OK )
				{
					AddTail(order, pos, id, part, param);
					if ( SearchOtherBlupi() )  return TRUE;
				}
			}
			if ( order == TO_GOTOPUSH )  // va puis pousse ?
			{
				if ( target == 0 )
				{
					err = ERR_GENERIC;
				}
				else
				{
					if ( IsTargetRunning(target) )
					{
						AddTail(order, pos, id, part, param);  // recommence + tard
						err = ERR_OK;
					}
					else
					{
						tPos = target->RetPosition(0);
						UndoRecord(tPos);
						err = StartTaskGoto(tPos, target, part);
						if ( err == ERR_OK )
						{
							AddTail(TO_PUSH, D3DVECTOR(0.0f, 0.0f, 0.0f), id, part, param);
						}
						else
						{
							AddTail(order, pos, id, part, param);
							if ( SearchOtherBlupi() )  return TRUE;
						}
					}
				}
			}
			if ( order == TO_GOTODOCK )  // va puis manoeuvre ?
			{
				if ( target == 0 )
				{
					err = ERR_GENERIC;
				}
				else
				{
					if ( IsTargetRunning(target) )
					{
						AddTail(order, pos, id, part, param);  // recommence + tard
						err = ERR_OK;
					}
					else
					{
						tPos = target->RetPosition(0);
						UndoRecord(tPos);
						err = StartTaskGoto(tPos, target, part);
						if ( err == ERR_OK )
						{
							AddTail(TO_DOCK, D3DVECTOR(0.0f, 0.0f, 0.0f), id, part, 0.0f);
						}
						else
						{
							AddTail(order, pos, id, part, param);
							if ( SearchOtherBlupi() )  return TRUE;
						}
					}
				}
			}
			if ( order == TO_GOTOCATA )  // va puis manoeuvre ?
			{
				if ( target == 0 )
				{
					err = ERR_GENERIC;
				}
				else
				{
					if ( IsTargetRunning(target) )
					{
						AddTail(order, pos, id, part, param);  // recommence + tard
						err = ERR_OK;
					}
					else
					{
						tPos = target->RetPosition(0);
						UndoRecord(tPos);
						err = StartTaskGoto(tPos, target, part);
						if ( err == ERR_OK )
						{
							AddTail(TO_CATA, D3DVECTOR(0.0f, 0.0f, 0.0f), id, part, 0.0f);
						}
						else
						{
							AddTail(order, pos, id, part, param);
							if ( SearchOtherBlupi() )  return TRUE;
						}
					}
				}
			}
			if ( order == TO_GOTOTRAX )  // va puis manoeuvre ?
			{
				if ( target == 0 )
				{
					err = ERR_GENERIC;
				}
				else
				{
					if ( IsTargetRunning(target) )
					{
						AddTail(order, pos, id, part, param);  // recommence + tard
						err = ERR_OK;
					}
					else
					{
						tPos = target->RetPosition(0);
						UndoRecord(tPos);
						err = StartTaskGoto(tPos, target, part);
						if ( err == ERR_OK )
						{
							AddTail(TO_TRAX, D3DVECTOR(0.0f, 0.0f, 0.0f), id, part, 0.0f);
						}
						else
						{
							AddTail(order, pos, id, part, param);
							if ( SearchOtherBlupi() )  return TRUE;
						}
					}
				}
			}
			if ( order == TO_GOTOPERFO )  // va puis manoeuvre ?
			{
				if ( target == 0 )
				{
					err = ERR_GENERIC;
				}
				else
				{
					if ( IsTargetRunning(target) )
					{
						AddTail(order, pos, id, part, param);  // recommence + tard
						err = ERR_OK;
					}
					else
					{
						tPos = target->RetPosition(0);
						UndoRecord(tPos);
						err = StartTaskGoto(tPos, target, part);
						if ( err == ERR_OK )
						{
							AddTail(TO_PERFO, D3DVECTOR(0.0f, 0.0f, 0.0f), id, part, 0.0f);
						}
						else
						{
							AddTail(order, pos, id, part, param);
							if ( SearchOtherBlupi() )  return TRUE;
						}
					}
				}
			}
			if ( order == TO_GOTOGUN )  // va puis manoeuvre ?
			{
				if ( target == 0 )
				{
					err = ERR_GENERIC;
				}
				else
				{
					if ( IsTargetRunning(target) )
					{
						AddTail(order, pos, id, part, param);  // recommence + tard
						err = ERR_OK;
					}
					else
					{
						tPos = target->RetPosition(0);
						UndoRecord(tPos);
						err = StartTaskGoto(tPos, target, part);
						if ( err == ERR_OK )
						{
							AddTail(TO_GUN, D3DVECTOR(0.0f, 0.0f, 0.0f), id, part, 0.0f);
						}
						else
						{
							AddTail(order, pos, id, part, param);
							if ( SearchOtherBlupi() )  return TRUE;
						}
					}
				}
			}
			if ( order == TO_GOTODRINK )  // va puis boit ?
			{
				if ( target == 0 )
				{
					err = ERR_GENERIC;
				}
				else
				{
					tPos = target->RetPosition(0);
					UndoRecord(tPos);
					err = StartTaskGoto(tPos, target, part);
					if ( err == ERR_OK )
					{
						AddTail(TO_DRINK, D3DVECTOR(0.0f, 0.0f, 0.0f), id, 0, 0.0f);
					}
					else
					{
						AddTail(order, pos, id, part, param);
						if ( SearchOtherBlupi() )  return TRUE;
					}
				}
			}
			if ( order == TO_GOTOGOAL )  // va puis s'envole ?
			{
				if ( target == 0 )
				{
					err = ERR_GENERIC;
				}
				else
				{
					tPos = target->RetPosition(0);
					UndoRecord(tPos);
					err = StartTaskGoto(tPos, target, part);
					if ( err == ERR_OK )
					{
						AddTail(TO_GOAL, D3DVECTOR(0.0f, 0.0f, 0.0f), id, 0, 0.0f);
					}
					else
					{
						AddTail(order, pos, id, part, param);
						if ( SearchOtherBlupi() )  return TRUE;
					}
				}
			}
			if ( order == TO_GOTODIVE )  // va puis plonge ?
			{
				if ( target == 0 )
				{
					err = ERR_GENERIC;
				}
				else
				{
					if ( IsTargetRunning(target) )
					{
						AddTail(order, pos, id, part, param);  // recommence + tard
						err = ERR_OK;
					}
					else
					{
						tPos = target->RetPosition(0);
						UndoRecord(tPos);
						err = StartTaskGoto(tPos, target, part);
						if ( err == ERR_OK )
						{
							AddTail(TO_DIVE, D3DVECTOR(0.0f, 0.0f, 0.0f), id, part, 0.0f);
						}
						else
						{
							AddTail(order, pos, id, part, param);
							if ( SearchOtherBlupi() )  return TRUE;
						}
					}
				}
			}
			if ( order == TO_PUSH )  // pousse seulement ?
			{
				UndoRecord();
				err = StartTaskPush(part, (int)param);
			}
			if ( order == TO_ROLL )  // roule une sphère ?
			{
				UndoRecord();
				err = StartTaskRoll(pos);
			}
			if ( order == TO_DOCK )  // manoeuvre seulement ?
			{
				UndoRecord();
				target = IdSearch(id);  // cherche l'objet cible
				err = StartTaskDock(target, part);
			}
			if ( order == TO_CATA )  // manoeuvre seulement ?
			{
				UndoRecord();
				target = IdSearch(id);  // cherche l'objet cible
				err = StartTaskCatapult(target, part);
			}
			if ( order == TO_TRAX )  // manoeuvre seulement ?
			{
				UndoRecord();
				target = IdSearch(id);  // cherche l'objet cible
				err = StartTaskTrax(target, part);
			}
			if ( order == TO_PERFO )  // manoeuvre seulement ?
			{
				UndoRecord();
				target = IdSearch(id);  // cherche l'objet cible
				err = StartTaskPerfo(target, part);
			}
			if ( order == TO_GUN )  // manoeuvre seulement ?
			{
				UndoRecord();
				target = IdSearch(id);  // cherche l'objet cible
				err = StartTaskGun(target, part);
			}
			if ( order == TO_DRINK )  // boit seulement ?
			{
				UndoRecord();
				target = IdSearch(id);  // cherche l'objet cible
				err = StartTaskDrink(target);
			}
			if ( order == TO_GOAL )  // s'envole seulement ?
			{
				UndoRecord();
				target = IdSearch(id);  // cherche l'objet cible
				err = StartTaskGoal(target);
			}
			if ( order == TO_DIVE )  // plonge seulement ?
			{
				UndoRecord();
				target = IdSearch(id);  // cherche l'objet cible
				err = StartTaskDive(target);
			}

			if ( err == ERR_OK )
			{
				m_taskOrder = order;
			}
			else
			{
				Flush();  // stoppe tout
				ShowError();
			}
		}
	}
	else	// tâche en cours ?
	{
		m_task->EventProcess(event);

		err = m_task->IsEnded();
		if ( err != ERR_CONTINUE )  // tâche terminée ?
		{
			if ( err != ERR_STOP )
			{
				Flush();  // stoppe tout
				ShowError();
			}
			delete m_task;
			m_task = 0;
			m_taskOrder = TO_NULL;
		}
	}

	return TRUE;
}

// Mémorise la situation actuelle avant d'effectuer un ordre.

void CTaskList::UndoRecord(const D3DVECTOR &goal)
{
	D3DVECTOR	pos;
	float		dist;

	pos = m_object->RetPosition(0);
	dist = Length2d(pos, goal);
	if ( dist < 10.0f )  return;

	if ( m_object->RetType() == OBJECT_BLUPI )
	{
		m_undo->Record();
	}
}

// Mémorise la situation actuelle avant d'effectuer un ordre.

void CTaskList::UndoRecord()
{
	if ( m_object->RetType() == OBJECT_BLUPI )
	{
		m_undo->Record();
	}
}


// Indique si la cible (l'objet sur lequel porte l'ordre)
// est occupée. Si oui, on remet l'ordre dans la file
// avec AddTail(), pour réessayer plus tard.

BOOL CTaskList::IsTargetRunning(CObject *target)
{
	CTaskList*	taskList;
	CAuto*		pAuto;

	if ( target == 0 )  return FALSE;

	taskList = target->RetTaskList();
	if ( taskList != 0 )
	{
		if ( taskList->IsRunning() )  return TRUE;
	}

	pAuto = target->RetAuto();
	if ( pAuto != 0 )
	{
		if ( pAuto->RetBusy() )  return TRUE;
	}

	return FALSE;
}


// Indique si une tâche est en cours.

BOOL CTaskList::IsRunning()
{
	return (m_task != 0);
}

// Retourne la tâche en cours.

TaskOrder CTaskList::RetRunningOrder()
{
	return m_taskOrder;
}

// Retourne la tâche en cours.

CTask* CTaskList::RetRunningTask()
{
	if ( m_task == 0 )  return 0;
	return m_task->RetRunningTask();
}

// Indique s'il existe une autre tâche à exécuter.

BOOL CTaskList::IsOtherTask()
{
	return (m_total != 0);
}

// Indique si la tâche en cours est annulable.

BOOL CTaskList::IsUndoable()
{
	if ( m_task == 0 )  return TRUE;
	return m_task->IsUndoable();
}

// Indique si la tâche en cours est stoppable.

BOOL CTaskList::IsStopable()
{
#if 1
	if ( m_task == 0 )  return FALSE;
	return !m_bStopPending;
#else
	return ( m_task != 0 );
#endif
}

// Stoppe proprement la tâche en cours.

BOOL CTaskList::Stop()
{
	Flush();
	if ( m_task == 0)  return FALSE;
	m_bStopPending = TRUE;
	return m_task->Stop();
}

// Stoppe brutalement la tâche en cours.

BOOL CTaskList::Abort()
{
	Flush();
	if ( m_task == 0)  return FALSE;
	m_task->Abort();
	delete m_task;  // stoppe la tâche en cours
	m_task = 0;
	return TRUE;
}


// Avancer.

Error CTaskList::StartTaskMove(float length, BOOL bNoError)
{
	Error	err;

	if ( m_task != 0 )
	{
		delete m_task;  // stoppe la tâche en cours
		m_task = 0;
	}

	m_task = new CTaskManager(m_iMan, m_object);
	err = m_task->StartTaskMove(length, bNoError);
	m_bStopPending = FALSE;
	return err;
}

// Tourner.

Error CTaskList::StartTaskTurn(float angle)
{
	Error	err;

	if ( m_task != 0 )
	{
		delete m_task;  // stoppe la tâche en cours
		m_task = 0;
	}

	m_task = new CTaskManager(m_iMan, m_object);
	err = m_task->StartTaskTurn(angle);
	m_bStopPending = FALSE;
	return err;
}

// Déplacer.

Error CTaskList::StartTaskGoto(D3DVECTOR pos, CObject *target, int part)
{
	Error	err;

	if ( m_task != 0 )
	{
		delete m_task;  // stoppe la tâche en cours
		m_task = 0;
	}

	m_task = new CTaskManager(m_iMan, m_object);
	err = m_task->StartTaskGoto(pos, target, part);
	m_bStopPending = FALSE;
	return err;
}

// Pousser.

Error CTaskList::StartTaskPush(int part, int nbTiles)
{
	Error	err;

	if ( m_task != 0 )
	{
		delete m_task;  // stoppe la tâche en cours
		m_task = 0;
	}

	m_task = new CTaskManager(m_iMan, m_object);
	err = m_task->StartTaskPush(part, nbTiles);
	m_bStopPending = FALSE;
	return err;
}

// Rouler.

Error CTaskList::StartTaskRoll(D3DVECTOR dir)
{
	Error	err;

	if ( m_task != 0 )
	{
		delete m_task;  // stoppe la tâche en cours
		m_task = 0;
	}

	m_task = new CTaskManager(m_iMan, m_object);
	err = m_task->StartTaskRoll(dir);
	m_bStopPending = FALSE;
	return err;
}

// Manoeuvrer.

Error CTaskList::StartTaskDock(CObject *dock, int part)
{
	Error	err;

	if ( m_task != 0 )
	{
		delete m_task;  // stoppe la tâche en cours
		m_task = 0;
	}

	m_task = new CTaskManager(m_iMan, m_object);
	err = m_task->StartTaskDock(dock, part);
	m_bStopPending = FALSE;
	return err;
}

// Manoeuvrer.

Error CTaskList::StartTaskCatapult(CObject *catapult, int part)
{
	Error	err;

	if ( m_task != 0 )
	{
		delete m_task;  // stoppe la tâche en cours
		m_task = 0;
	}

	m_task = new CTaskManager(m_iMan, m_object);
	err = m_task->StartTaskCatapult(catapult, part);
	m_bStopPending = FALSE;
	return err;
}

// Manoeuvrer.

Error CTaskList::StartTaskTrax(CObject *trax, int part)
{
	Error	err;

	if ( m_task != 0 )
	{
		delete m_task;  // stoppe la tâche en cours
		m_task = 0;
	}

	m_task = new CTaskManager(m_iMan, m_object);
	err = m_task->StartTaskTrax(trax, part);
	m_bStopPending = FALSE;
	return err;
}

// Manoeuvrer.

Error CTaskList::StartTaskPerfo(CObject *perfo, int part)
{
	Error	err;

	if ( m_task != 0 )
	{
		delete m_task;  // stoppe la tâche en cours
		m_task = 0;
	}

	m_task = new CTaskManager(m_iMan, m_object);
	err = m_task->StartTaskPerfo(perfo, part);
	m_bStopPending = FALSE;
	return err;
}

// Manoeuvrer.

Error CTaskList::StartTaskGun(CObject *gun, int part)
{
	Error	err;

	if ( m_task != 0 )
	{
		delete m_task;  // stoppe la tâche en cours
		m_task = 0;
	}

	m_task = new CTaskManager(m_iMan, m_object);
	err = m_task->StartTaskGun(gun, part);
	m_bStopPending = FALSE;
	return err;
}

// Boire.

Error CTaskList::StartTaskDrink(CObject *fiole)
{
	Error	err;

	if ( m_task != 0 )
	{
		delete m_task;  // stoppe la tâche en cours
		m_task = 0;
	}

	m_task = new CTaskManager(m_iMan, m_object);
	err = m_task->StartTaskDrink(fiole);
	m_bStopPending = FALSE;
	return err;
}

// S'envoler.

Error CTaskList::StartTaskGoal(CObject *goal)
{
	Error	err;

	if ( m_task != 0 )
	{
		delete m_task;  // stoppe la tâche en cours
		m_task = 0;
	}

	m_task = new CTaskManager(m_iMan, m_object);
	err = m_task->StartTaskGoal(goal);
	m_bStopPending = FALSE;
	return err;
}

// Plonger.

Error CTaskList::StartTaskDive(CObject *dive)
{
	Error	err;

	if ( m_task != 0 )
	{
		delete m_task;  // stoppe la tâche en cours
		m_task = 0;
	}

	m_task = new CTaskManager(m_iMan, m_object);
	err = m_task->StartTaskDive(dive);
	m_bStopPending = FALSE;
	return err;
}


// Cherche un objet d'après son identificateur.

CObject* CTaskList::IdSearch(int id)
{
	CObject*	pObj;
	int			i;

	if ( id == 0 )  return 0;

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( pObj->RetID() == id )  return pObj;
	}
	return 0;
}


// Blupi proteste car il y a une erreur.

void CTaskList::ShowError()
{
	ObjectType	type;
	CMotion*	motion;

	motion = m_object->RetMotion();
	if ( motion == 0 )  return;

	type = m_object->RetType();

	if ( type == OBJECT_BLUPI )
	{
		motion->SetAction(MBLUPI_ERROR);
	}
	if ( type == OBJECT_TRAX  ||
		 type == OBJECT_PERFO ||
		 type == OBJECT_GUN   )
	{
		motion->SetAction(MTRAX_ERROR);
	}
}


// Vide les statistiques.

void CTaskList::FlushStatistic()
{
	m_statisticOrder = TO_NULL;
	m_statisticLittleGoto = 0;
	m_statisticGotoPush = 0;
}

// Ajoute un ordre dans les statistiques.

void CTaskList::AddStatistic(TaskOrder order, D3DVECTOR pos, int id, int part, float param)
{
	D3DVECTOR	goal;
	CObject*	target;
	float		dist;

	if ( m_object->RetType() != OBJECT_BLUPI )  return;

	if ( order == m_statisticOrder &&
		 order == TO_GOTO )
	{
		dist = Length2d(pos, m_statisticPos);
		if ( dist < 10.0f )
		{
			m_statisticLittleGoto ++;
		}
		else
		{
			m_statisticLittleGoto = 0;
		}
	}
	else
	{
		m_statisticLittleGoto = 0;
	}

	if ( m_statisticOrder == TO_GOTO &&
		 order == TO_GOTOPUSH )
	{
		target = IdSearch(id);
		if ( target != 0 )
		{
			goal = target->RetPosition(0);

			if ( part == 1 || part == 5 )  // flèche ouest ?
			{
				goal.x -= PUSH_DIST;
			}
			if ( part == 2 || part == 6 )  // flèche nord ?
			{
				goal.z += PUSH_DIST;
			}
			if ( part == 3 || part == 7 )  // flèche est ?
			{
				goal.x += PUSH_DIST;
			}
			if ( part == 4 || part == 8 )  // flèche sud ?
			{
				goal.z -= PUSH_DIST;
			}

			dist = Length2d(goal, m_object->RetPosition(0));
			if ( dist < 2.0f )
			{
				m_statisticGotoPush ++;
			}
			else
			{
				m_statisticGotoPush = 0;
			}
		}
	}
	else
	{
		m_statisticGotoPush = 0;
	}

	m_statisticOrder = order;
	m_statisticPos = pos;
}

// Retourne un conseil éventuel en fonction des statistiques.

int CTaskList::RetStatisticAdvise()
{
	if ( m_statisticLittleGoto >= 3 )  return RT_ADVISE_LITGOTO;
	if ( m_statisticGotoPush   >= 1 )  return RT_ADVISE_GOTOPUSH;
	return 0;
}


// Vérifie si un blupi peut effectuer une tâche donnée.

BOOL CTaskList::IsTaskPossible(CObject *pObj, TaskOrder order,
							   D3DVECTOR pos, int id, int part, float param)
{
	CObject*	target;
	CTaskList*	pObjTL;
	D3DVECTOR	tPos;
	Error		err;

	target = IdSearch(id);  // cherche l'objet cible

	pObjTL = pObj->RetTaskList();
	if ( pObjTL == 0 )  return FALSE;

	err = ERR_OK;

	if ( order == TO_GOTO )  // va ?
	{
		err = pObjTL->StartTaskGoto(pos, target, part);
	}
	if ( order == TO_GOTOPUSH )  // va puis pousse ?
	{
		if ( target == 0 )
		{
			err = ERR_GENERIC;
		}
		else
		{
			if ( IsTargetRunning(target) )
			{
				err = ERR_OK;
			}
			else
			{
				tPos = target->RetPosition(0);
				err = pObjTL->StartTaskGoto(tPos, target, part);
			}
		}
	}
	if ( order == TO_GOTODOCK )  // va puis manoeuvre ?
	{
		if ( target == 0 )
		{
			err = ERR_GENERIC;
		}
		else
		{
			if ( IsTargetRunning(target) )
			{
				err = ERR_OK;
			}
			else
			{
				tPos = target->RetPosition(0);
				err = pObjTL->StartTaskGoto(tPos, target, part);
			}
		}
	}
	if ( order == TO_GOTOCATA )  // va puis manoeuvre ?
	{
		if ( target == 0 )
		{
			err = ERR_GENERIC;
		}
		else
		{
			if ( IsTargetRunning(target) )
			{
				err = ERR_OK;
			}
			else
			{
				tPos = target->RetPosition(0);
				err = pObjTL->StartTaskGoto(tPos, target, part);
			}
		}
	}
	if ( order == TO_GOTOTRAX )  // va puis manoeuvre ?
	{
		if ( target == 0 )
		{
			err = ERR_GENERIC;
		}
		else
		{
			if ( IsTargetRunning(target) )
			{
				err = ERR_OK;
			}
			else
			{
				tPos = target->RetPosition(0);
				err = pObjTL->StartTaskGoto(tPos, target, part);
			}
		}
	}
	if ( order == TO_GOTOPERFO )  // va puis manoeuvre ?
	{
		if ( target == 0 )
		{
			err = ERR_GENERIC;
		}
		else
		{
			if ( IsTargetRunning(target) )
			{
				err = ERR_OK;
			}
			else
			{
				tPos = target->RetPosition(0);
				err = pObjTL->StartTaskGoto(tPos, target, part);
			}
		}
	}
	if ( order == TO_GOTOGUN )  // va puis manoeuvre ?
	{
		if ( target == 0 )
		{
			err = ERR_GENERIC;
		}
		else
		{
			if ( IsTargetRunning(target) )
			{
				err = ERR_OK;
			}
			else
			{
				tPos = target->RetPosition(0);
				err = pObjTL->StartTaskGoto(tPos, target, part);
			}
		}
	}
	if ( order == TO_GOTODRINK )  // va puis boit ?
	{
		if ( target == 0 )
		{
			err = ERR_GENERIC;
		}
		else
		{
			tPos = target->RetPosition(0);
			err = pObjTL->StartTaskGoto(tPos, target, part);
		}
	}
	if ( order == TO_GOTOGOAL )  // va puis s'envole ?
	{
		if ( target == 0 )
		{
			err = ERR_GENERIC;
		}
		else
		{
			tPos = target->RetPosition(0);
			err = pObjTL->StartTaskGoto(tPos, target, part);
		}
	}
	if ( order == TO_GOTODIVE )  // va puis plonge ?
	{
		if ( target == 0 )
		{
			err = ERR_GENERIC;
		}
		else
		{
			if ( IsTargetRunning(target) )
			{
				err = ERR_OK;
			}
			else
			{
				tPos = target->RetPosition(0);
				err = pObjTL->StartTaskGoto(tPos, target, part);
			}
		}
	}

	pObjTL->Abort();  // avorte la tâche crée "pour voir"

	return ( err == ERR_OK );
}

// Cherche un autre blupi pour exécuter l'ordre.

BOOL CTaskList::SearchOtherBlupi()
{
	CObject*	pObj;
	CTaskList*	pObjTL;
	TaskOrder	order;
	D3DVECTOR	pos;
	float		param;
	int			i, id, part;

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;
		if ( pObj == m_object )  continue;  // soi-même ?
		if ( pObj->RetLock() )  continue;
		if ( !pObj->RetEnable() )  continue;
		if ( pObj->RetType() != OBJECT_BLUPI )  continue;

		pObjTL = pObj->RetTaskList();
		if ( pObjTL == 0 )  continue;
		if ( pObjTL->IsRunning() )  continue;

		SubTail(order, pos, id, part, param);
		if ( IsTaskPossible(pObj, order, pos, id, part, param) )
		{
			pObjTL->AddHead(order, pos, id, part, param);

			while ( SubTail(order, pos, id, part, param) )
			{
				pObjTL->AddHead(order, pos, id, part, param);
			}

			m_main->SetSelect(pObj);  // sélectionne ce blupi
			return TRUE;
		}
		AddTail(order, pos, id, part, param);
	}

	return FALSE;
}

