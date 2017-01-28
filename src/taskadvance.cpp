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
#include "particule.h"
#include "motion.h"
#include "motionbot.h"
#include "motionperfo.h"
#include "motiongun.h"
#include "pyro.h"
#include "sound.h"
#include "mainundo.h"
#include "tasklist.h"
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
	CPyro*		pyro;
	D3DVECTOR	pos, cirVib, boxPos, speed;
	FPOINT		dim;
	float		linSpeed, duration;

	if ( m_engine->RetPause() )  return TRUE;
	if ( event.event != EVENT_FRAME )  return TRUE;
	if ( m_bError )  return TRUE;

	linSpeed = m_motion->RetLinSpeed();
	if ( m_bPerfo )  linSpeed *= 0.5f;
	m_moveDist += event.rTime*linSpeed;
	if ( m_moveDist > m_moveAbs )  m_moveDist = m_moveAbs;

	pos = SegmentDist(m_startPos, m_goalPos, m_moveDist);

	if ( m_bFall && m_moveDist > 5.0f )
	{
		pos.y -= powf((m_moveDist-5.0f), 2.0f);
	}

	if ( m_bPerfo )
	{
		cirVib.x = (Rand()-0.5f)*m_moveDist*0.05f;
		cirVib.y = (Rand()-0.5f)*m_moveDist*0.05f;
		cirVib.z = (Rand()-0.5f)*m_moveDist*0.05f;
		m_pPerfo->SetCirVibration(cirVib);

		boxPos = m_goalPos+(m_goalPos-m_startPos)*(m_moveDist/8.0f)*0.6f;
		m_pPerfo->SetPosition(0, boxPos);

		boxPos.x += (Rand()*0.5f)*4.0f;
		boxPos.z += (Rand()*0.5f)*4.0f;
		boxPos.y += 4.0f;
		speed.x = (Rand()-0.5f)*2.0f;
		speed.z = (Rand()-0.5f)*2.0f;
		speed.y = Rand()*5.0f;
		dim.x = (4.0f+Rand()*4.0f);
		dim.y = dim.x;
		duration = 2.0f+Rand()*2.0f;
		m_particule->CreateParticule(boxPos, speed, dim, PARTISMOKE1, duration);
	}

	m_object->SetPosition(0, pos);

	if ( m_bLostGoal )
	{
		if ( m_moveDist >= 4.0f && m_pGoal != 0 )
		{
			pyro = new CPyro(m_iMan);
			pyro->Create(PT_GOAL, m_pGoal);  // destruction ballon
			m_pGoal = 0;
		}
	}

	return TRUE;
}


// Assigne le but à atteindre.
// Le mode bNoError=TRUE est utilisé lorsque la tâche est créée
// suite à un Undo. Dans ce cas, la position d'arrivée m_goalPos
// peut être occupée selon IsPosFree par le robot lui-même. Et
// il ne faut pas la considérer comme occupée !

Error CTaskAdvance::Start(float length, BOOL bNoError)
{
	CObject*	target;
	ObjectType	type;
	FPOINT		rot;
	float		angle, level;

	m_type = m_object->RetType();
	m_startPos = m_object->RetPosition(0);

	angle = m_object->RetAngleY(0);
	angle = Grid(angle, PI/2.0f);
	rot = RotatePoint(-angle, length);
	m_goalPos.x = m_startPos.x+rot.x;
	m_goalPos.z = m_startPos.z+rot.y;
	m_goalPos.y = m_startPos.y;

	m_bMine     = FALSE;
	m_bGoal     = FALSE;
	m_bLostGoal = FALSE;
	m_bFall     = FALSE;
	m_bPerfo    = FALSE;

	if ( m_type == OBJECT_CRAZY )
	{
		target = SearchObject(m_goalPos, 4.0f);
		if ( target != 0 )
		{
			type = target->RetType();
			if ( type == OBJECT_MINE )
			{
				m_bMine = TRUE;
				m_pMine = target;
				m_pMine->SetLock(TRUE);
			}
			if ( type == OBJECT_GOAL )
			{
				m_bGoal = TRUE;
				m_pGoal = target;
//				m_pGoal->SetLock(TRUE);  // fait par CTaskGoal::Start()
			}
		}

		if ( !bNoError && !m_bMine && !IsPosFree(m_goalPos) )
		{
			m_bError = TRUE;
			return ERR_GENERIC;
		}
	}
	else if ( m_type == OBJECT_PERFO )
	{
		target = SearchObject(m_goalPos, 4.0f);
		if ( target != 0 )
		{
			type = target->RetType();
			if ( type >= OBJECT_BOX1  &&
				 type <= OBJECT_BOX20 )
			{
				m_bPerfo = TRUE;
				m_pPerfo = target;
				m_pPerfo->SetLock(TRUE);
				StartAction(m_object, MPERFO_ACTION);
				m_sound->Play(SOUND_PERFO2, m_startPos);
			}
			else if ( type == OBJECT_GOAL )
			{
				m_bLostGoal = TRUE;
				m_pGoal = target;
				m_pGoal->SetLock(TRUE);
			}
			else
			{
				m_bError = TRUE;
				return ERR_GENERIC;
			}
		}
	}
	else if ( m_type == OBJECT_TRAX ||
			  m_type == OBJECT_GUN  )
	{
		target = SearchObject(m_goalPos, 4.0f);
		if ( target != 0 )
		{
			type = target->RetType();
			if ( type == OBJECT_GOAL )
			{
				m_bLostGoal = TRUE;
				m_pGoal = target;
				m_pGoal->SetLock(TRUE);
			}
			else
			{
				m_bError = TRUE;
				return ERR_GENERIC;
			}
		}
	}
	else
	{
		if ( !IsPosFree(m_goalPos) )
		{
			m_bError = TRUE;
			return ERR_GENERIC;
		}
	}

	level = m_terrain->RetFloorLevel(m_goalPos);
	if ( level < 0.0f )
	{
		m_bFall = TRUE;
		m_object->SetLock(TRUE);
	}

	if ( m_type == OBJECT_TRAX  ||
		 m_type == OBJECT_PERFO ||
		 m_type == OBJECT_GUN   )
	{
		// Pas de Lock pour permettre d'actionner les boutons
		// pendant que le trax fonctionne !
//		m_object->SetLock(TRUE);
	}

	// La position de start est déjà lock.
	m_terrain->SetLockZone(m_goalPos, LZ_BOT);

	if ( m_bFall )
	{
		m_object->SetAdjustShadow(FALSE);
	}

	m_moveAbs = Abs(length);
	m_moveDist = 0.0f;

	ProgressLinSpeed(1.0f);
	ProgressCirSpeed(0.0f);

	m_bError = FALSE;
	return ERR_OK;
}

// Indique si l'action est terminée.

Error CTaskAdvance::IsEnded()
{
	CPyro*		pyro;
	PyroType	pt;

	if ( m_engine->RetPause() )  return ERR_CONTINUE;

	if ( m_bError )
	{
		return ERR_STOP;
	}

	if ( m_bMine && m_moveDist > 4.0f )
	{
		m_terrain->SetLockZone(m_startPos, LZ_FREE);
		m_terrain->SetLockZone(m_goalPos, LZ_FREE);

		pyro = new CPyro(m_iMan);
		pyro->Create(PT_FRAGT, m_pMine);  // destruction mine

		pyro = new CPyro(m_iMan);
		pyro->Create(PT_EXPLOT, m_object);  // destruction robot

		m_bError = TRUE;
		return ERR_STOP;
	}

	if ( m_bPerfo && m_moveDist > 4.0f )
	{
		pyro = new CPyro(m_iMan);
		pyro->Create(PT_FRAGT, m_pPerfo, 0.5f);  // destruction caisse

		m_bPerfo = FALSE;
		m_pPerfo = 0;
		StartAction(m_object, MPERFO_WAIT);
	}

	if ( m_moveDist >= m_moveAbs )
	{
		// On laisse lock la position d'arrivée.
		m_terrain->SetLockZone(m_startPos, LZ_FREE);

		if ( m_type != OBJECT_CRAZY )
		{
			ProgressLinSpeed(0.0f);
			ProgressCirSpeed(0.0f);
		}

		if ( m_bFall )
		{
			pt = PT_BREAKT;
			if ( m_type == OBJECT_TRAX  )  pt = PT_FRAGT;
			if ( m_type == OBJECT_PERFO )  pt = PT_FRAGT;
			if ( m_type == OBJECT_GUN   )  pt = PT_FRAGT;
			pyro = new CPyro(m_iMan);
			pyro->Create(pt, m_object);  // casse robot
		}
		else
		{
			if ( m_type == OBJECT_TRAX  ||
				 m_type == OBJECT_PERFO ||
				 m_type == OBJECT_GUN   )
			{
				m_object->SetLock(FALSE);
			}
			if ( m_type == OBJECT_PERFO )
			{
				m_terrain->SetLockZone(m_goalPos, LZ_BOT);
			}
		}

		if ( m_bGoal )
		{
			ProgressLinSpeed(0.0f);
			ProgressCirSpeed(0.0f);

			m_object->SetLock(TRUE);  // l'instruction "islock" le détecte !
			m_object->StartTaskList(TO_GOAL, m_goalPos, m_pGoal, 0, 0.0f);
		}

		if ( m_type == OBJECT_GUN )
		{
			StartAction(m_object, MGUN_FIRE);
		}

		return ERR_STOP;
	}

	return ERR_CONTINUE;
}


// Indique si l'action est annulable.

BOOL CTaskAdvance::IsUndoable()
{
	return ( m_type == OBJECT_CRAZY );
}


// Cherche un objet quelconque.

CObject* CTaskAdvance::SearchObject(D3DVECTOR center, float radius)
{
	CObject		*pObj, *pBest;
	D3DVECTOR	pos;
	ObjectType	type;
	float		min, dist;
	int			i;

	pBest = 0;
	min = 100000.0f;
	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( pObj->RetExplo() )  continue;
		if ( pObj->RetLock() )  continue;  // déjà stoppé ?
		if ( !pObj->RetEnable() )  continue;

		type = pObj->RetType();
		if ( type == OBJECT_LIFT )  continue;

		pos = pObj->RetPosition(0);
		dist = Length2d(pos, center);

		if ( dist <= radius && dist < min )
		{
			min = dist;
			pBest = pObj;
		}
	}
	return pBest;
}

// Cherche si une position est libre.

BOOL CTaskAdvance::IsPosFree(D3DVECTOR center)
{
	LockZone	lz;

	lz = m_terrain->RetLockZone(center);

	if ( m_type == OBJECT_CRAZY )
	{
		if ( lz == LZ_MINE )  return TRUE;
	}

	if ( lz == LZ_FREE  ||
		 lz == LZ_MAX1X )  return TRUE;

	return FALSE;
}


// Spécifie la vitesse linéaire pour une action pour Blupi.

void CTaskAdvance::ProgressLinSpeed(float speed)
{
	CMotion*	motion;

	motion = m_object->RetMotion();
	if ( motion == 0 )  return;

	motion->SetActionLinSpeed(speed);
}

// Spécifie la vitesse circulaire pour une action pour Blupi.

void CTaskAdvance::ProgressCirSpeed(float speed)
{
	CMotion*	motion;

	motion = m_object->RetMotion();
	if ( motion == 0 )  return;

	motion->SetActionCirSpeed(speed);
}


// Ecrit la situation de l'objet.

void CTaskAdvance::WriteSituation()
{
	m_undo->WriteTokenFloat("abs", m_moveAbs);
	m_undo->WriteTokenFloat("dist", m_moveDist);
	m_undo->WriteTokenPos("start", m_startPos);
	m_undo->WriteTokenPos("goal", m_goalPos);
}

// lit la situation de l'objet.

void CTaskAdvance::ReadSituation()
{
	m_undo->ReadTokenFloat("abs", m_moveAbs);
	m_undo->ReadTokenFloat("dist", m_moveDist);
	m_undo->ReadTokenPos("start", m_startPos);
	m_undo->ReadTokenPos("goal", m_goalPos);
}

// Démarre une action pour un objet.

void CTaskAdvance::StartAction(CObject* pObj, int action)
{
	CMotion*	motion;

	motion = pObj->RetMotion();
	if ( motion == 0 )  return;

	motion->SetAction(action);
}

