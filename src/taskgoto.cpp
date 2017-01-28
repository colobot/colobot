// taskgoto.cpp

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
#include "water.h"
#include "object.h"
#include "particule.h"
#include "motion.h"
#include "motionblupi.h"
#include "motionbot.h"
#include "task.h"
#include "taskgoto.h"



#define BM_DIM_STEP		8.0f




// Constructeur de l'objet.

CTaskGoto::CTaskGoto(CInstanceManager* iMan, CObject* object)
					 : CTask(iMan, object)
{
	CTask::CTask(iMan, object);

	m_bStopPending = FALSE;

	m_dimTile = m_terrain->RetDimTile();
	m_nbTiles = m_terrain->RetNbTiles();
	m_nbTiles2 = m_nbTiles/2;

	m_listTable = 0;
	m_listFlags = 0;
	m_listTotal = 0;
}

// Destructeur de l'objet.

CTaskGoto::~CTaskGoto()
{
	ListFlush();
}


// Gestion d'un événement.

BOOL CTaskGoto::EventProcess(const Event &event)
{
	D3DVECTOR	pos, goal;
	FPOINT		rot, repulse;
	float		a, g, dist, speed, move, dir, factor;
	float		linSpeed, cirSpeed;

	if ( m_engine->RetPause() )  return TRUE;
	if ( event.event != EVENT_FRAME )  return TRUE;

	if ( m_error != ERR_OK )  return FALSE;

	m_totalTime += event.rTime;

	if ( m_phase == TGP_GOTO && m_pathIndex >= 0 )  // goto dot list ?
	{
		pos = m_object->RetPosition(0);  // position actuelle
		goal = m_pathPos[m_pathIndex];  // position à atteindre
		dist = Length2d(pos, goal);  // longueur à avancer
		rot.x = (goal.x-pos.x)/dist;
		rot.y = (goal.z-pos.z)/dist;

		a = m_object->RetAngleY(0);
		g = RotateAngle(rot.x, -rot.y);  // CW !
		speed = m_motion->RetCirSpeed();
		if ( m_object->RetStrong() > 0.0f )  speed *= 1.5f;
		if ( m_object->RetStrong() < 0.0f )  speed *= 0.7f;
		dir = Direction(a, g);
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

		if ( Abs(dir) < 50.0f*PI/180.0f )  // presque fini de tourner ?
		{
			linSpeed = 1.0f;
			cirSpeed = 0.0f;
			speed = m_motion->RetLinSpeed();
			if ( m_object->RetStrong() > 0.0f )  speed *= 1.5f;
			if ( m_object->RetStrong() < 0.0f )  speed *= 0.7f;
			factor = 1.0f;
			if ( m_totalAdvance < m_linStopLength )  // première droite ?
			{
				factor = 1.0f-(m_linStopLength-m_totalAdvance)/m_linStopLength;
				factor = 0.2f+factor*0.8f;
			}
			if ( m_pathIndex == 0 &&  // dernière droite ?
				 dist < m_linStopLength )
			{
				factor = 1.0f-(m_linStopLength-dist)/m_linStopLength;
				factor = 0.2f+factor*0.8f;
			}
			speed *= factor;
			linSpeed *= factor;
			move = speed*event.rTime;
			if ( move > dist )  move = dist;
			pos = pos + (goal-pos)*move/dist;
			m_totalAdvance += move;
		}
		else
		{
			linSpeed = 0.0f;
			cirSpeed = 1.0f;
		}
		ProgressLinSpeed(linSpeed);
		ProgressCirSpeed(cirSpeed);
		m_object->SetPosition(0, pos);

		return TRUE;
	}

	if ( m_phase == TGP_TURN )  // tourne vers l'objet ?
	{
		a = m_object->RetAngleY(0);
		g = m_angle;
		speed = m_motion->RetCirSpeed();
		if ( m_object->RetStrong() > 0.0f )  speed *= 1.5f;
		if ( m_object->RetStrong() < 0.0f )  speed *= 0.7f;
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
		ProgressLinSpeed(0.0f);
		ProgressCirSpeed(1.0f);
	}

	if ( m_phase == TGP_RUSH )  // élan initial ?
	{
		m_delay -= event.rTime;
	}

	return TRUE;
}

// Assigne le but à atteindre.

Error CTaskGoto::Start(D3DVECTOR goal, CObject *target, int part)
{
	D3DVECTOR	pos;

	goal.y = 0.0f+m_object->RetCharacter()->height;

	m_type = m_object->RetType();
	m_typeTarget = OBJECT_NULL;
	m_goalObject = goal;
	m_goal       = goal;
	m_phase      = TGP_GOTO;
	m_error      = ERR_OK;
	m_bFinalTurn = FALSE;
	m_bFinalRush = FALSE;
	m_finalAngle = 0.0f;
	m_linStopLength = 1.5f;
	m_totalAdvance = 0.0f;
	m_finalMargin = 0.0f;
	m_totalTime = 0.0f;

	if ( target != 0 )
	{
		m_goal = target->RetPosition(0);
		m_goal.y += m_object->RetCharacter()->height;
		AdjustTarget(target, part);
	}

	m_goalx = (int)(m_goal.x/m_dimTile) + m_nbTiles2;
	m_goaly = (int)(m_goal.z/m_dimTile) + m_nbTiles2;

	pos = m_object->RetPosition(0);
	pos = Grid(pos, 8.0f);

	if ( m_type == OBJECT_BLUPI &&
		 target != 0            &&
		 m_bFinalTurn           &&
		 ((m_typeTarget >= OBJECT_BOX1  && m_typeTarget <= OBJECT_BOX6 ) ||
		  (m_typeTarget >= OBJECT_BOX8  && m_typeTarget <= OBJECT_BOX9 ) ||
		  (m_typeTarget >= OBJECT_BOX11 && m_typeTarget <= OBJECT_BOX20) ||
		  (m_typeTarget >= OBJECT_KEY1  && m_typeTarget <= OBJECT_KEY5 ) ) &&
		 pos.x == m_goal.x      &&
		 pos.z == m_goal.z      )
	{
		m_bFinalRush = TRUE;
	}

	if ( !PathFinder(pos, m_goal) )
	{
		m_error = ERR_GOTO_BUSY;
		return m_error;
	}
	return ERR_OK;
}

// Indique si l'action est terminée.

Error CTaskGoto::IsEnded()
{
	D3DVECTOR	pos, goal;
	float		dist, angle, limit;

	if ( m_engine->RetPause() )  return ERR_CONTINUE;
	if ( m_error != ERR_OK )  return m_error;

	if ( m_phase == TGP_GOTO )  // goto dot list ?
	{
		pos = m_object->RetPosition(0);  // position actuelle

		if ( m_pathIndex >= 0 )
		{
			if ( m_pathIndex == 0 )
			{
				if ( m_finalMargin == 0.0f )
				{
					limit = 0.2f;  // précis si dernier point
				}
				else
				{
					limit = m_finalMargin;
				}
			}
			else
			{
				limit = 1.0f;  // moins précis si pas dernier point
			}
			goal = m_pathPos[m_pathIndex];  // position à atteindre
			dist = Length2d(pos, goal);  // longueur à avancer
		}
		else
		{
			limit = 1.0f;
			dist = 0.0f;
		}

		if ( dist < limit )
		{
			JostleObject(pos);

			m_pathIndex --;  // index point suivant à atteindre

			if ( m_pathIndex >= 0 &&  // pas encore arrivé ?
				 !m_bStopPending )
			{
				// Nouvel obstacle survenu entre-temps ?
				if ( IsLockZone(m_pathPos[m_pathIndex]) )
				{
					pos = m_object->RetPosition(0);
					pos = Grid(pos, 8.0f);
					if ( !PathFinder(pos, m_goal) )  // nouveau chemin
					{
						ProgressLinSpeed(0.0f);
						ProgressCirSpeed(0.0f);
						m_error = ERR_GOTO_BUSY;
						return m_error;
					}
					return ERR_CONTINUE;
				}
			}
			else	// est-on arrivé ?
			{
				if ( m_bFinalTurn )
				{
					m_angle = RotateAngle(m_goalObject.x-pos.x, pos.z-m_goalObject.z);
					m_angle += m_finalAngle;
					m_angle = Grid(m_angle, PI/2.0f);
					m_angle = NormAngle(m_angle);
					m_phase = TGP_TURN;
				}
				else
				{
					ProgressLinSpeed(0.0f);
					ProgressCirSpeed(0.0f);
					FreeLockZone();
					return ERR_STOP;
				}
			}
		}
	}

	if ( m_phase == TGP_TURN )  // tourne vers l'objet ?
	{
		angle = NormAngle(m_object->RetAngleY(0));
		if ( Abs(angle-m_angle) < 0.02f )
		{
			m_object->SetAngleY(0, m_angle);
			ProgressLinSpeed(0.0f);
			ProgressCirSpeed(0.0f);
			FreeLockZone();

			if ( m_bFinalRush && m_totalTime < 0.8f )
			{
				StartAction(MBLUPI_RUSH);
				m_phase = TGP_RUSH;
				m_delay = 0.8f-m_totalTime;
				return ERR_CONTINUE;
			}
			return ERR_STOP;
		}
	}

	if ( m_phase == TGP_RUSH )  // élan initial ?
	{
		if ( m_delay <= 0.0f )
		{
			StartAction(MBLUPI_WAIT);
			return ERR_STOP;
		}
	}

	return ERR_CONTINUE;
}


// Termine brutalement l'action en cours.

BOOL CTaskGoto::Abort()
{
	if ( m_pathIndex >= 0 )
	{
		m_lastPos = m_pathPos[m_pathIndex];
		FreeLockZone();
	}
	return TRUE;
}

// Indique si la tache en cours est stoppable.

BOOL CTaskGoto::IsStopable()
{
	return !m_bStopPending;
}

// Stoppe proprement la tache en cours.

BOOL CTaskGoto::Stop()
{
	if ( m_bStopPending )  return FALSE;

	if ( m_phase == TGP_GOTO )  // goto dot list ?
	{
		m_bStopPending = TRUE;
		return TRUE;
	}

	return FALSE;
}


// Fait bouger les ballons et les plantes.

void CTaskGoto::JostleObject(D3DVECTOR center)
{
	CObject*	pObj;
	D3DVECTOR	pos;
	ObjectType	type;
	float		dist;
	int			i;

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( pObj->RetExplo() )  continue;
		if ( pObj->RetLock() )  continue;  // déjà stoppé ?

		type = pObj->RetType();

		if ( type == OBJECT_GOAL )
		{
			pos = pObj->RetPosition(0);
			dist = Length2d(pos, center);

			if ( dist <= 5.0f )
			{
				pObj->JostleObject(1.0f);
			}
		}

		if ( (type >= OBJECT_PLANT0  &&
			  type <= OBJECT_PLANT4  ) ||
			 (type >= OBJECT_PLANT10 &&
			  type <= OBJECT_PLANT19 ) )
		{
			pos = pObj->RetPosition(0);
			if ( pos.y < -2.0f )  continue;
			dist = Length2d(pos, center);

			if ( dist <= 10.0f )
			{
				pObj->JostleObject(1.0f);
			}
		}
	}
}


// Si on arrive sur une caisse, modifie la position pour arriver par
// le bon côté de la caisse.
// Retourne TRUE s'il faut ignorer l'objet goal.

void CTaskGoto::AdjustTarget(CObject *target, int part)
{
	D3DVECTOR	spos;
	float		angle;

	m_typeTarget = target->RetType();

	if ( (m_typeTarget >= OBJECT_BOX1 && m_typeTarget <= OBJECT_BOX20) ||
		 (m_typeTarget >= OBJECT_KEY1 && m_typeTarget <= OBJECT_KEY5 ) )
	{
		if ( part == 1 || part == 5 )  // flèche ouest ?
		{
			m_goal = target->RetPosition(0);
			m_goal.x -= PUSH_DIST;
			m_goal.y += m_object->RetCharacter()->height;
			m_bFinalTurn = TRUE;
			m_finalAngle = 0.0f;
		}
		if ( part == 2 || part == 6 )  // flèche nord ?
		{
			m_goal = target->RetPosition(0);
			m_goal.z += PUSH_DIST;
			m_goal.y += m_object->RetCharacter()->height;
			m_bFinalTurn = TRUE;
			m_finalAngle = 0.0f;
		}
		if ( part == 3 || part == 7 )  // flèche est ?
		{
			m_goal = target->RetPosition(0);
			m_goal.x += PUSH_DIST;
			m_goal.y += m_object->RetCharacter()->height;
			m_bFinalTurn = TRUE;
			m_finalAngle = 0.0f;
		}
		if ( part == 4 || part == 8 )  // flèche sud ?
		{
			m_goal = target->RetPosition(0);
			m_goal.z -= PUSH_DIST;
			m_goal.y += m_object->RetCharacter()->height;
			m_bFinalTurn = TRUE;
			m_finalAngle = 0.0f;
		}
	}

	if ( m_typeTarget == OBJECT_DOCK )
	{
		if ( part == 4 || part == 5 )   // ^/v ?
		{
			spos = target->RetPosition(1);
			m_goal = target->RetPosition(0);
			m_goal.x += -24.0f;
			m_goal.z +=  -8.0f+spos.z;
			m_goal.y += m_object->RetCharacter()->height;
			m_bFinalTurn = TRUE;
			m_finalAngle = PI;
		}
		if ( part == 6 )  // piston ?
		{
			spos = target->RetPosition(1);
			m_goal = target->RetPosition(0);
			m_goal.x += -24.0f;
			m_goal.z += spos.z;
			m_goal.y += m_object->RetCharacter()->height;
			m_bFinalTurn = TRUE;
			m_finalAngle = PI;
		}
		if ( part == 7 || part == 8 )   // </> ?
		{
			spos = target->RetPosition(1);
			m_goal = target->RetPosition(0);
			m_goal.x += -24.0f;
			m_goal.z +=   8.0f+spos.z;
			m_goal.y += m_object->RetCharacter()->height;
			m_bFinalTurn = TRUE;
			m_finalAngle = PI;
		}
	}

	if ( m_typeTarget == OBJECT_CATAPULT )
	{
		if ( part == 1 )   // action ?
		{
			m_goal = D3DVECTOR(0.0f, 0.0f, -16.0f);
			angle = target->RetAngleY(0);
			RotatePoint(D3DVECTOR(0.0f, 0.0f, 0.0f), -angle, 0.0f, m_goal);
			m_goal.y += m_object->RetCharacter()->height;
			m_goal += target->RetPosition(0);
			m_goal = Grid(m_goal, 8.0f);
			m_bFinalTurn = TRUE;
			m_finalAngle = 0.0f;
		}
	}

	if ( m_typeTarget == OBJECT_BARRIER69 )  // plongeoir ?
	{
		m_goal = D3DVECTOR(-8.0f, 0.0f, 0.0f);
		angle = target->RetAngleY(0);
		RotatePoint(D3DVECTOR(0.0f, 0.0f, 0.0f), -angle, 0.0f, m_goal);
		m_goal.y += m_object->RetCharacter()->height;
		m_goal += target->RetPosition(0);
		m_goal = Grid(m_goal, 8.0f);
		m_bFinalTurn = TRUE;
		m_finalAngle = 0.0f;
	}

	if ( m_typeTarget == OBJECT_TRAX  ||
		 m_typeTarget == OBJECT_PERFO ||
		 m_typeTarget == OBJECT_GUN   )
	{
		if ( part == 3 )   // avance ?
		{
			m_goal = D3DVECTOR(-8.0f, 0.0f, 0.0f);
			angle = target->RetAngleY(0);
			RotatePoint(D3DVECTOR(0.0f, 0.0f, 0.0f), -angle, 0.0f, m_goal);
			m_goal.y += m_object->RetCharacter()->height;
			m_goal += target->RetPosition(0);
			m_goal = Grid(m_goal, 8.0f);
			m_bFinalTurn = TRUE;
			m_finalAngle = 0.0f;
		}
		if ( part == 4 )  // gauche ?
		{
			m_goal = D3DVECTOR(0.0f, 0.0f, 8.0f);
			angle = target->RetAngleY(0);
			RotatePoint(D3DVECTOR(0.0f, 0.0f, 0.0f), -angle, 0.0f, m_goal);
			m_goal.y += m_object->RetCharacter()->height;
			m_goal += target->RetPosition(0);
			m_goal = Grid(m_goal, 8.0f);
			m_bFinalTurn = TRUE;
			m_finalAngle = 0.0f;
		}
		if ( part == 5 )   // droite ?
		{
			m_goal = D3DVECTOR(0.0f, 0.0f, -8.0f);
			angle = target->RetAngleY(0);
			RotatePoint(D3DVECTOR(0.0f, 0.0f, 0.0f), -angle, 0.0f, m_goal);
			m_goal.y += m_object->RetCharacter()->height;
			m_goal += target->RetPosition(0);
			m_goal = Grid(m_goal, 8.0f);
			m_bFinalTurn = TRUE;
			m_finalAngle = 0.0f;
		}
	}

	if ( m_typeTarget == OBJECT_FIOLE ||
		 m_typeTarget == OBJECT_GLU   )
	{
		m_finalMargin = 8.0f;
		m_bFinalTurn = TRUE;
		m_finalAngle = 0.0f;
	}
}


// Démarre une action pour Blupi.

void CTaskGoto::StartAction(int action, float speed)
{
	CMotion*	motion;

	motion = m_object->RetMotion();
	if ( motion == 0 )  return;

	if ( m_type == OBJECT_BLUPI )
	{
		motion->SetAction(action, speed);
	}
}

// Spécifie la progression pour une action pour Blupi.

void CTaskGoto::ProgressAction(float progress)
{
	CMotion*	motion;

	motion = m_object->RetMotion();
	if ( motion == 0 )  return;

	motion->SetActionProgress(progress);
}

// Spécifie la vitesse linéaire pour une action pour Blupi.

void CTaskGoto::ProgressLinSpeed(float speed)
{
	CMotion*	motion;

	motion = m_object->RetMotion();
	if ( motion == 0 )  return;

	motion->SetActionLinSpeed(speed);
}

// Spécifie la vitesse circulaire pour une action pour Blupi.

void CTaskGoto::ProgressCirSpeed(float speed)
{
	CMotion*	motion;

	motion = m_object->RetMotion();
	if ( motion == 0 )  return;

	motion->SetActionCirSpeed(speed);
}


// Teste si un déplacement dans la case "goal" suivante est possible..
// Retourne TRUE si on ne peut pas atteindre la position.
// Retourne FALSE et bloque la position si le déplacement est possible.

BOOL CTaskGoto::IsLockZone(const D3DVECTOR &goal)
{
	D3DVECTOR	pos, corner;
	LockZone	lz;

	FreeLockZone();

	if ( m_finalMargin != 0.0f )
	{
		if ( goal.x == m_goal.x && goal.z == m_goal.z )
		{
			return FALSE;
		}
	}

	lz = m_terrain->RetLockZone(goal);
	if ( lz != LZ_FREE    &&
		 lz != LZ_MAX1X   &&
		 lz != LZ_TUNNELh &&
		 lz != LZ_TUNNELv )  return TRUE;

	pos = Grid(m_object->RetPosition(0), 8.0f);
	if ( Abs(Abs(goal.x-pos.x)-Abs(goal.z-pos.z)) < 1.0f )  // diagonale ?
	{
		corner.x = pos.x;
		corner.z = goal.z;
		corner.y = 0.0f;
		if ( m_terrain->IsLockZoneSquare(corner) )  return TRUE;

		corner.x = goal.x;
		corner.z = pos.z;
		corner.y = 0.0f;
		if ( m_terrain->IsLockZoneSquare(corner) )  return TRUE;
	}

	m_lastPos = pos;
	m_terrain->SetLockZone(goal, LZ_BLUPI);

	return FALSE;
}

// Libère la dernière position.

void CTaskGoto::FreeLockZone()
{
	if ( m_lastPos.x != NAN )
	{
		m_terrain->SetLockZone(m_lastPos, LZ_FREE);
		m_lastPos.x = NAN;
	}
}


// Essaye plusieurs fois de trouver le chemin.
// Dans une première passe, essaye en évitant les max1x fermés.
// Dans une deuxième passe, essaye même les max1x fermés.

BOOL CTaskGoto::PathFinder(D3DVECTOR start, D3DVECTOR goal)
{
	m_pass = 0;  // première passe
	m_secondNeed = 0;
	if ( PathFinderPass(start, goal) )  return TRUE;

	if ( m_secondNeed == 0 )  return FALSE;
	m_pass = 1;  // deuxième passe
	return PathFinderPass(start, goal);
}

// Cherche le chemin le plus court, selon une idée géniale de Denis.
// Soit le labyrinthe suivant ("S"=start, "G"=goal, "X"=obstacle) :
//		S . . X . . . G
//		. . . X . X . .
//		. . . . . X . .
//		. X X . . X . .
//		. . X . . . . .
//		. . . . . . . .
// On cherche d'abord les cases possibles "1" autour de "S".
// On cherche ensuite les cases possibles "2" autour des "1".
// On cherche ensuite les cases possibles "n+1" autour des "n", etc.
// Toutes ces cases visitées sont ajoutées dans une liste (m_listTable),
// avec l'index de la case père.
//		S 1 2 X 6 7 8 G
//		1 1 2 X 5 X . .
//		2 2 2 3 4 X 8 8
//		3 X X 3 4 X 7 7
//		4 5 X 4 4 5 6 7
//		5 5 6 5 5 5 6 7
// Dès qu'on atteint "G", on remonte "8", "7" ... "1" en arrière
// dans la liste (m_listTable) grace à l'index de la case père,
// ce qui permet d'obtenir ce chemin (dans m_pathPos), depuis la fin :
//		S . . X 6 7 8 G
//		. 1 . X 5 X . .
//		. . 2 3 4 X . .
//		. X X . . X . .
//		. . X . . . . .
//		. . . . . . . .
// On obtient ainsi le chemin le plus court dans tous les cas !
// On teste les 8 cases "1" (n+1) autour de la case courante (n) :
//		1 1 1
//		1 S 1
//		1 1 1
// Celles des coins ont la restriction suivante :
//		. a 1
//		. S b
//		. . .
// On accepte d'aller de "S" à "1" si "a" ET "b" sont des obstacles
// arrondis (blupi, mine, sphère, etc.).

BOOL CTaskGoto::PathFinderPass(D3DVECTOR start, D3DVECTOR goal)
{
	D3DVECTOR	p;
	FPOINT		current, pos;
	int			i, i1,i2, ii1,ii2, x,y;

	m_lastPos = D3DVECTOR(NAN, NAN, NAN);

	m_listGoalx = (int)(goal.x/m_dimTile) + m_nbTiles2;
	m_listGoaly = (int)(goal.z/m_dimTile) + m_nbTiles2;

	ListCreate();  // crée la liste et le bitmap
	m_pathIndex = -1;  // corrige un bug cherché longtemps avec Denis !!!

	// Met le premier point (start) dans la liste.
	m_bListFound = FALSE;
	i1 = m_listTotal;
	x = (int)(start.x/m_dimTile) + m_nbTiles2;
	y = (int)(start.z/m_dimTile) + m_nbTiles2;
	ListAdd(-1, x,y);
	i2 = m_listTotal;

	// Met tous les points "n+1" autour des précédents "n".
	while ( !m_bListFound )  // tant que goal pas atteint
	{
		ii1 = m_listTotal;

		for ( i=i1 ; i<i2 ; i++ )  // parcourt toutes les cases "n"
		{
			x = m_listTable[i].x;
			y = m_listTable[i].y;  // case centrale "n"

			if ( IsTerrainFree(x,y, x+1,y) )
			{
				if ( ListAdd(i, x+1,y) )  break;
			}

			if ( IsTerrainFree(x,y, x-1,y) )
			{
				if ( ListAdd(i, x-1,y) )  break;
			}

			if ( IsTerrainFree(x,y, x,y+1) )
			{
				if ( ListAdd(i, x,y+1) )  break;
			}

			if ( IsTerrainFree(x,y, x,y-1) )
			{
				if ( ListAdd(i, x,y-1) )  break;
			}

			if ( IsTerrainFreeR(x,y, x+1,y+1) &&
				 IsTerrainRound(x,y, x+0,y+1) &&
				 IsTerrainRound(x,y, x+1,y+0) )
			{
				if ( ListAdd(i, x+1,y+1) )  break;
			}

			if ( IsTerrainFreeR(x,y, x-1,y+1) &&
				 IsTerrainRound(x,y, x+0,y+1) &&
				 IsTerrainRound(x,y, x-1,y+0) )
			{
				if ( ListAdd(i, x-1,y+1) )  break;
			}

			if ( IsTerrainFreeR(x,y, x+1,y-1) &&
				 IsTerrainRound(x,y, x+0,y-1) &&
				 IsTerrainRound(x,y, x+1,y+0) )
			{
				if ( ListAdd(i, x+1,y-1) )  break;
			}

			if ( IsTerrainFreeR(x,y, x-1,y-1) &&
				 IsTerrainRound(x,y, x+0,y-1) &&
				 IsTerrainRound(x,y, x-1,y+0) )
			{
				if ( ListAdd(i, x-1,y-1) )  break;
			}
		}

		ii2 = m_listTotal;
		if ( ii1 == ii2 )  return FALSE;

		i1 = ii1;
		i2 = ii2;  // les cases "n+1" deviendront "n" au prochain while
	}

	// Récupère le chemin le plus court depuis la fin.
	m_pathIndex = 0;
	i = m_listTotal-1;  // index dernière case (c'est toujours le goal)
	while ( i != 0 )
	{
		x = m_listTable[i].x;
		y = m_listTable[i].y;

		p.x = (x-m_nbTiles2)*m_dimTile;
		p.z = (y-m_nbTiles2)*m_dimTile;
		p.y = m_object->RetCharacter()->height;
		m_pathPos[m_pathIndex++] = p;

		if ( m_pathIndex >= MAXPATH )  return FALSE;

		i = m_listTable[i].father;  // index case "n-1" père
	}
	m_pathIndex --;

	// Lock le premier segment qui sera avancé.
	if ( m_pathIndex >= 0 )
	{
		if ( IsLockZone(m_pathPos[m_pathIndex]) )  return FALSE;
	}

	return TRUE;
}

// Détermine si une position du terrain est libre.
// Si on va sur le goal est qu'il s'agit d'une fiole, le mouvement
// (diagonale) est refusé.

BOOL CTaskGoto::IsTerrainFreeR(int ix, int iy, int x, int y)
{
	if ( x == m_goalx && y == m_goaly && m_finalMargin != 0.0f )
	{
		return FALSE;
	}

	return IsTerrainFree(ix, iy, x, y);
}

// Détermine si une position du terrain est libre.

BOOL CTaskGoto::IsTerrainFree(int ix, int iy, int x, int y)
{
	LockZone	lz;

	if ( !m_terrain->IsSolid(x,y) )  return FALSE;

	lz = m_terrain->RetLockZone(ix,iy, TRUE);
	if ( lz == LZ_TUNNELh && ix == x )  return FALSE;
	if ( lz == LZ_TUNNELv && iy == y )  return FALSE;

	lz = m_terrain->RetLockZone(x,y, TRUE);

	if ( x == m_goalx && y == m_goaly )  // est-on sur l'arrivée ?
	{
		if ( lz == LZ_MAX1X )
		{
			lz = LZ_FREE;  // si arrivée = trappe -> comme si libre
		}
		if ( m_finalMargin != 0.0f )
		{
			lz = LZ_FREE;  // si arrivée = potion -> comme si libre
		}
	}

	if ( lz == LZ_FREE )  return TRUE;

	if ( lz == LZ_TUNNELh && iy == y )  return TRUE;
	if ( lz == LZ_TUNNELv && ix == x )  return TRUE;

	if ( m_pass == 0 )  // première passe ?
	{
		if ( lz == LZ_MAX1X )
		{
			m_secondNeed ++;  // une deuxième passe serait utile
			return FALSE;
		}
	}
	else	// deuxième passe ?
	{
		if ( lz == LZ_MAX1X )  return TRUE;
	}

	return FALSE;
}

// Détermine si une position du terrain est libre ou occupée
// par un objet rond.

BOOL CTaskGoto::IsTerrainRound(int ix, int iy, int x, int y)
{
	LockZone	lz;

	if ( !m_terrain->IsSolid(x,y) )  return FALSE;

	lz = m_terrain->RetLockZone(ix,iy, TRUE);
	if ( lz == LZ_TUNNELh ||
		 lz == LZ_TUNNELv )  return FALSE;

	lz = m_terrain->RetLockZone(x,y, TRUE);

	if ( x == m_goalx && y == m_goaly )  // est-on sur l'arrivée ?
	{
		if ( lz == LZ_MAX1X )
		{
			lz = LZ_FREE;  // si arrivée = trappe -> comme si libre
		}
		if ( m_finalMargin != 0.0f )
		{
			lz = LZ_FREE;  // si arrivée = potion -> comme si libre
		}
	}

	if ( lz == LZ_FREE  ||
		 lz == LZ_FIXo  ||
		 lz == LZ_MINE  ||
		 lz == LZ_FIOLE ||
		 lz == LZ_BLUPI ||
		 lz == LZ_BOXo  )  return TRUE;

	if ( m_pass == 0 )  // première passe ?
	{
		if ( lz == LZ_MAX1X )
		{
			m_secondNeed ++;  // une deuxième passe serait utile
			return FALSE;
		}
	}
	else	// deuxième passe ?
	{
		if ( lz == LZ_MAX1X )  return TRUE;
	}

	return FALSE;
}

// Libère la liste et le bitmap.

void CTaskGoto::ListFlush()
{
	free(m_listTable);
	m_listTable = 0;

	free(m_listFlags);
	m_listFlags = 0;
}

// Crée une liste et un bitmap vide.

void CTaskGoto::ListCreate()
{
	ListFlush();  // libère si nécessaire

	m_listTable = (ListItem*)malloc(sizeof(ListItem)*m_nbTiles*m_nbTiles);
	m_listTotal = 0;

	m_listFlags = (char*)malloc(sizeof(char)*m_nbTiles*m_nbTiles);
	ZeroMemory(m_listFlags, sizeof(char)*m_nbTiles*m_nbTiles);
}

// Ajoute un élément à la fin de la liste, seulement s'il n'existe
// encore aucun élément à cette coordonnée.
// Retourne TRUE si on a atteint le goal et qu'il faut arrêter.

BOOL CTaskGoto::ListAdd(int father, int x, int y)
{
	if ( m_listFlags[y*m_nbTiles+x] != 0 )  return FALSE;
	m_listFlags[y*m_nbTiles+x] = 1;

	if ( x == m_listGoalx && y == m_listGoaly )  // goal atteint ?
	{
		m_bListFound = TRUE;
	}

	m_listTable[m_listTotal].father = father;
	m_listTable[m_listTotal].x = x;
	m_listTable[m_listTotal].y = y;
	m_listTotal ++;

	return m_bListFound;
}


