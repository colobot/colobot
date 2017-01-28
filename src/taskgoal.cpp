// taskgoal.cpp

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
#include "motionbot.h"
#include "auto.h"
#include "autogoal.h"
#include "pyro.h"
#include "robotmain.h"
#include "sound.h"
#include "task.h"
#include "tasklist.h"
#include "taskgoal.h"




// Constructeur de l'objet.

CTaskGoal::CTaskGoal(CInstanceManager* iMan, CObject* object)
					 : CTask(iMan, object)
{
	CTask::CTask(iMan, object);

	m_time = 0.0f;
	m_bError = TRUE;
}

// Destructeur de l'objet.

CTaskGoal::~CTaskGoal()
{
}


// Gestion d'un événement.

BOOL CTaskGoal::EventProcess(const Event &event)
{
	D3DVECTOR	pos;
	float		progress;

	if ( m_engine->RetPause() )  return TRUE;
	if ( event.event != EVENT_FRAME )  return TRUE;
	if ( m_bError )  return FALSE;

	m_time += event.rTime;
	m_progress += event.rTime*m_speed;
//?	progress = Norm(m_progress);
	progress = m_progress;

	if ( m_phase == TGO_GOAL )
	{
	}

	if ( m_phase == TGO_FLY )
	{
		pos = m_startPos;
		pos.y += m_object->RetCharacter()->height;
//?		pos.y += powf(progress, 1.5f)*40.0f;
//?		pos.y += powf(progress, 1.5f)*200.0f;
		pos.y += powf(progress, 1.5f)*300.0f;
		m_object->SetPosition(0, pos);
	}

	return TRUE;
}


// Assigne le but à atteindre.

Error CTaskGoal::Start(CObject *goal)
{
	ObjectType	type;

	m_main->IncTotalManip();

	if ( goal == 0 || goal->RetLock() )
	{
		m_bError = TRUE;
		return ERR_GENERIC;
	}

	type = m_object->RetType();

	m_goal = goal;
	m_startPos = m_goal->RetPosition(0);

	m_time = 0.0f;
	m_phase = TGO_GOAL;
	m_progress = 0.0f;
	if ( type == OBJECT_BLUPI )
	{
		m_speed = 1.0f/3.0f;
	}
	else
	{
		m_speed = 1.0f/1.0f;
	}
	m_bError = FALSE;

	m_goal->SetLock(TRUE);
	m_goal->SetDead(TRUE);

	m_object->SetLock(TRUE);
	m_object->SetDead(TRUE);
//?	m_object->SetEnable(FALSE);

	if ( m_object->RetSelect() )
	{
		m_main->SelectOther(m_object);
	}

	StartAction(MBLUPI_YOUPIE);  // attrape le ballon
	return ERR_OK;
}

// Indique si l'action est terminée.

Error CTaskGoal::IsEnded()
{
	CAuto*		pa;
	CTaskList*	taskList;
	D3DVECTOR	pos;
	float		max;
	int			objRank;

	if ( m_engine->RetPause() )  return ERR_CONTINUE;

	if ( m_bError )
	{
		return ERR_STOP;
	}

	max = 1.0f;
	if ( m_phase == TGO_FLY )
	{
//?		if ( IsLastGoal() )  max = 1.5f;  // monte plus haut
		max = 1.5f;
	}

	if ( m_progress < max )
	{
		return ERR_CONTINUE;
	}

	if ( m_phase == TGO_GOAL )
	{
		pa = m_goal->RetAuto();
		if ( pa != 0 )
		{
			pa->Start(IsLastGoal()?AGO_LASTFLY:AGO_FLY);
		}

		m_object->SetFret(m_goal);
		m_goal->SetTruck(m_object);
		pos.x = 0.0f;
		pos.y = -m_object->RetCharacter()->height;
		pos.z = 0.0f;
		m_goal->SetPosition(0, pos);
		m_goal->SetAngleY(0, m_goal->RetAngleY(0)-m_object->RetAngleY(0));

		StartAction(MBLUPI_GOAL2);  // suspendu au ballon

		pos = m_object->RetPosition(0);
		m_terrain->SetLockZone(pos, LZ_FREE);

		m_phase = TGO_FLY;
		m_progress = 0.0f;
		m_speed = 1.0f/6.0f;
		return ERR_CONTINUE;
	}

	if ( m_phase == TGO_FLY )
	{
		m_goal->DeleteObject();  // supprime le ballon
		delete m_goal;
		m_goal = 0;

		objRank = m_object->RetObjectRank(0);
		m_engine->ShadowDelete(objRank);  // supprime l'ombre

		taskList = m_object->RetTaskList();
		if ( taskList != 0 )
		{
			taskList->Flush();  // supprime les ordres suivants
		}

		m_object->SetHide(-1, TRUE);  // cache tout l'objet

		pos = m_object->RetPosition(0);
		pos.y = 10000.0f;
		m_object->SetPosition(0, pos);

		return ERR_STOP;
	}

	return ERR_STOP;
}


// Démarre une action pour Blupi.

void CTaskGoal::StartAction(int action, float speed)
{
	CMotion*	motion;
	ObjectType	type;
	Sound		sound;
	int			a;

	motion = m_object->RetMotion();
	if ( motion == 0 )  return;

	type = m_object->RetType();

	if ( type == OBJECT_BLUPI )
	{
		sound = SOUND_BLUPIohhh;
		if ( action == MBLUPI_YOUPIE )  sound = SOUND_BLUPIouaaa;
		if ( action == MBLUPI_GOAL2  )  sound = SOUND_BLUPIohhh;
		m_sound->Play(sound, m_object->RetPosition(0), 1.0f);
		motion->SetAction(action, speed);
	}

	if ( type == OBJECT_CRAZY )
	{
		a = -1;
		if ( action == MBLUPI_YOUPIE )  a = MBOT_YOUPIE;
		if ( action == MBLUPI_GOAL2  )  a = MBOT_GOAL;
		motion->SetAction(a, speed);
		m_sound->Play(SOUND_CRAZY, m_object->RetPosition(0), 1.0f, 1.5f);
	}
}

// Indique s'il s'agit du dernier ballon à partir.

BOOL CTaskGoal::IsLastGoal()
{
	CObject*	pObj;
	ObjectType	type;
	int			i, total;

	total = 0;
	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( pObj->RetDead() )  continue;

		type = pObj->RetType();

		if ( type == OBJECT_BLUPI ||
			 type == OBJECT_CRAZY )  total ++;
	}

	return ( total == 0 );
}

