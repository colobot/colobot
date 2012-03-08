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
#include "physics.h"
#include "brain.h"
#include "task.h"
#include "taskgoto.h"



#define FLY_DIST_GROUND	80.0f		// distance minimale pour rester au sol
#define FLY_DEF_HEIGHT	50.0f		// hauteur de vol par défaut
#define BM_DIM_STEP		5.0f




// Constructeur de l'objet.

CTaskGoto::CTaskGoto(CInstanceManager* iMan, CObject* object)
					 : CTask(iMan, object)
{
	CTask::CTask(iMan, object);

	m_bmArray = 0;
}

// Destructeur de l'objet.

CTaskGoto::~CTaskGoto()
{
	BitmapClose();
}


// Gestion d'un événement.

BOOL CTaskGoto::EventProcess(const Event &event)
{
	D3DVECTOR	pos, goal;
	FPOINT		rot, repulse;
	float		a, g, dist, linSpeed, cirSpeed, h, hh, factor, dir;
	Error		ret;

	if ( m_engine->RetPause() )  return TRUE;
	if ( event.event != EVENT_FRAME )  return TRUE;

	// Objet momentanément immobile (fourmi sur le dos) ?
	if ( m_object->RetFixed() )
	{
		m_physics->SetMotorSpeedX(0.0f);  // stoppe l'avance
		m_physics->SetMotorSpeedZ(0.0f);  // stoppe la rotation
		return TRUE;
	}

	if ( m_error != ERR_OK )  return FALSE;

	if ( m_bWorm )
	{
		WormFrame(event.rTime);
	}

	if ( m_phase == TGP_BEAMLEAK )  // fuite ?
	{
		m_leakTime += event.rTime;

		pos = m_object->RetPosition(0);

		rot.x = m_leakPos.x-pos.x;
		rot.y = m_leakPos.z-pos.z;
		dist = Length(rot.x, rot.y);
		rot.x /= dist;
		rot.y /= dist;

		a = m_object->RetAngleY(0);
		g = RotateAngle(rot.x, -rot.y);  // CW !
		a = Direction(a, g)*1.0f;
		cirSpeed = a;
		if ( cirSpeed >  1.0f )  cirSpeed =  1.0f;
		if ( cirSpeed < -1.0f )  cirSpeed = -1.0f;

		a = NormAngle(a);
		if ( a > PI*0.5f && a < PI*1.5f )
		{
			linSpeed = 1.0f;  // obstacle derrière -> avance
			cirSpeed = -cirSpeed;
		}
		else
		{
			linSpeed = -1.0f;  // obstacle devant -> recule
		}

		if ( m_bLeakRecede )
		{
			linSpeed = -1.0f;
			cirSpeed = 0.0f;
		}

		m_physics->SetMotorSpeedZ(cirSpeed);  // tourne à gauche/droite
		m_physics->SetMotorSpeedX(linSpeed);  // avance
		return TRUE;
	}

	if ( m_phase == TGP_BEAMSEARCH )  // recherche chemin ?
	{
		if ( m_bmStep == 0 )
		{
			// Libère la zone autour du départ.
			BitmapClearCircle(m_object->RetPosition(0), BM_DIM_STEP*1.8f);
		}

		pos = m_object->RetPosition(0);

		if ( m_bmFretObject == 0 )
		{
			goal = m_goal;
			dist = 0.0f;
		}
		else
		{
			goal = m_goalObject;
			dist = TAKE_DIST+2.0f;
			if ( m_bmFretObject->RetType() == OBJECT_BASE )  dist = 12.0f;
		}

		ret = BeamSearch(pos, goal, dist);
		if ( ret == ERR_OK )
		{
#if 0
			D3DVECTOR	min, max;
			min = pos;
			max = m_goal;
			if ( min.x > max.x )  Swap(min.x, max.x);
			if ( min.z > max.z )  Swap(min.z, max.z);
			min.x -= 50.0f;
			min.z -= 50.0f;
			max.x += 50.0f;
			max.z += 50.0f;
			BitmapDebug(min, max, m_object->RetPosition(0), m_goal);
#endif
			if ( m_physics->RetLand() )  m_phase = TGP_BEAMWCOLD;
			else                         m_phase = TGP_BEAMGOTO;
			m_bmIndex = 0;
			m_bmWatchDogPos = m_object->RetPosition(0);
			m_bmWatchDogTime = 0.0f;
		}
		if ( ret == ERR_GOTO_IMPOSSIBLE || ret == ERR_GOTO_ITER )
		{
#if 0
			D3DVECTOR	min, max;
			min = pos;
			max = m_goal;
			if ( min.x > max.x )  Swap(min.x, max.x);
			if ( min.z > max.z )  Swap(min.z, max.z);
			min.x -= 50.0f;
			min.z -= 50.0f;
			max.x += 50.0f;
			max.z += 50.0f;
			BitmapDebug(min, max, m_object->RetPosition(0), m_goal);
#endif
			m_error = ret;
			return FALSE;
		}
		return TRUE;
	}

	if ( m_phase == TGP_BEAMWCOLD )  // attend refroidissement réacteur ?
	{
		return TRUE;
	}

	if ( m_phase == TGP_BEAMUP )  // décolle ?
	{
		m_physics->SetMotorSpeedY(1.0f);  // monte
		return TRUE;
	}

	if ( m_phase == TGP_BEAMGOTO )  // goto dot list ?
	{
		if ( m_physics->RetCollision() )  // collision ?
		{
			m_physics->SetCollision(FALSE);  // y'a plus
		}

		pos = m_object->RetPosition(0);

		if ( m_physics->RetType() == TYPE_FLYING && m_altitude == 0.0f )
		{
			if ( m_physics->RetLand() )
			{
				m_physics->SetMotorSpeedY(0.0f);
			}
			else
			{
				m_physics->SetMotorSpeedY(-1.0f);
			}
		}

		if ( m_physics->RetType() == TYPE_FLYING && m_altitude > 0.0f )
		{
			goal = m_bmPoints[m_bmIndex];
			goal.y = pos.y;
			h = m_terrain->RetFloorHeight(goal, TRUE, TRUE);
			dist = Length2d(pos, goal);
			if ( dist != 0.0f )  // anticipe ?
			{
				linSpeed = m_physics->RetLinMotionX(MO_REASPEED);
				linSpeed /= m_physics->RetLinMotionX(MO_ADVSPEED);
				goal.x = pos.x + (goal.x-pos.x)*linSpeed*20.0f/dist;
				goal.z = pos.z + (goal.z-pos.z)*linSpeed*20.0f/dist;
			}
			goal.y = pos.y;
			hh = m_terrain->RetFloorHeight(goal, TRUE, TRUE);
			h = Min(h, hh);
			linSpeed = 0.0f;
			if ( h < m_altitude-1.0f )
			{
				linSpeed = 0.2f+((m_altitude-1.0f)-h)*0.1f;  // monte
				if ( linSpeed > 1.0f )  linSpeed = 1.0f;
			}
			if ( h > m_altitude+1.0f )
			{
				linSpeed = -0.2f;  // descend
			}
			m_physics->SetMotorSpeedY(linSpeed);
		}

		rot.x = m_bmPoints[m_bmIndex].x-pos.x;
		rot.y = m_bmPoints[m_bmIndex].z-pos.z;
		dist = Length(rot.x, rot.y);
		rot.x /= dist;
		rot.y /= dist;

		a = m_object->RetAngleY(0);
		g = RotateAngle(rot.x, -rot.y);  // CW !
		cirSpeed = Direction(a, g)*2.0f;
		if ( cirSpeed >  1.0f )  cirSpeed =  1.0f;
		if ( cirSpeed < -1.0f )  cirSpeed = -1.0f;
		if ( dist < 4.0f )  cirSpeed *= dist/4.0f;  // si proche -> tourne moins

		if ( m_bmIndex == m_bmTotal )  // dernier point ?
		{
			linSpeed = dist/(m_physics->RetLinStopLength()*1.5f);
			if ( linSpeed > 1.0f )  linSpeed = 1.0f;
		}
		else
		{
			linSpeed = 1.0f;  // fonce sans s'arrêter
		}

		linSpeed *= 1.0f-(1.0f-0.3f)*Abs(cirSpeed);

//?		if ( dist < 20.0f && Abs(cirSpeed) >= 0.5f )
		if ( Abs(cirSpeed) >= 0.2f )
		{
			linSpeed = 0.0f;  // tourne d'abord, puis avance
		}

		dist = Length2d(pos, m_bmWatchDogPos);
		if ( dist < 1.0f && linSpeed != 0.0f )
		{
			m_bmWatchDogTime += event.rTime;
		}
		else
		{
			m_bmWatchDogTime = 0.0f;
			m_bmWatchDogPos = pos;
		}

		if ( m_bmWatchDogTime >= 1.0f )  // immobile depuis longtemps ?
		{
			m_physics->SetMotorSpeedX(0.0f);  // stoppe l'avance
			m_physics->SetMotorSpeedZ(0.0f);  // stoppe la rotation
			BeamStart();  // on recommence tout
			return TRUE;
		}

		m_physics->SetMotorSpeedZ(cirSpeed);  // tourne à gauche/droite
		m_physics->SetMotorSpeedX(linSpeed);  // avance
		return TRUE;
	}

	if ( m_phase == TGP_BEAMDOWN )  // atterri ?
	{
		m_physics->SetMotorSpeedY(-0.5f);  // tombe
		return TRUE;
	}

	if ( m_phase == TGP_LAND )  // atterri ?
	{
		m_physics->SetMotorSpeedY(-0.5f);  // tombe
		return TRUE;
	}

	if ( m_goalMode == TGG_EXPRESS )
	{
		if ( m_crashMode == TGC_HALT )
		{
			if ( m_physics->RetCollision() )  // collision ?
			{
				m_physics->SetCollision(FALSE);  // y'a plus
				m_error = ERR_STOP;
				return TRUE;
			}
		}

		pos = m_object->RetPosition(0);

		if ( m_altitude > 0.0f )
		{
			h = m_terrain->RetFloorHeight(pos, TRUE, TRUE);
			linSpeed = 0.0f;
			if ( h < m_altitude )
			{
				linSpeed = 0.1f;  // monte
			}
			if ( h > m_altitude )
			{
				linSpeed = -0.2f;  // descend
			}
			m_physics->SetMotorSpeedY(linSpeed);
		}

		rot.x = m_goal.x-pos.x;
		rot.y = m_goal.z-pos.z;
		a = m_object->RetAngleY(0);
		g = RotateAngle(rot.x, -rot.y);  // CW !
		cirSpeed = Direction(a, g)*1.0f;
		if ( cirSpeed >  1.0f )  cirSpeed =  1.0f;
		if ( cirSpeed < -1.0f )  cirSpeed = -1.0f;

		m_physics->SetMotorSpeedZ(cirSpeed);  // tourne à gauche/droite
		m_physics->SetMotorSpeedX(1.0f);  // avance
		return TRUE;
	}

	if ( m_phase != TGP_TURN                 &&
		 m_physics->RetType() == TYPE_FLYING &&
		 m_altitude > 0.0f                   )
	{
		pos = m_object->RetPosition(0);
		dist = Length2d(m_goal, pos);
		factor = (dist-20.0f)/20.0f;
		if ( factor < 0.0f )  factor = 0.0f;
		if ( factor > 1.0f )  factor = 1.0f;

		h = m_terrain->RetFloorHeight(m_object->RetPosition(0), TRUE, TRUE);
		linSpeed = 0.0f;
		if ( h < (m_altitude-0.5f)*factor && factor == 1.0f )
		{
			linSpeed = 0.1f;  // monte
		}
		if ( h > m_altitude*factor )
		{
			linSpeed = -0.2f;  // descend
		}
		ComputeFlyingRepulse(dir);
		linSpeed += dir*0.2f;

		m_physics->SetMotorSpeedY(linSpeed);
	}

	if ( m_phase == TGP_ADVANCE )  // va vers l'objectif ?
	{
		if ( m_physics->RetCollision() )  // collision ?
		{
			m_physics->SetCollision(FALSE);  // y'a plus
			m_time = 0.0f;
			m_phase = TGP_CRWAIT;
			return TRUE;
		}

#if 0
		pos = m_object->RetPosition(0);
		a = m_object->RetAngleY(0);
		g = RotateAngle(m_goal.x-pos.x, pos.z-m_goal.z);  // CW !
		cirSpeed = Direction(a, g)*1.0f;
		if ( cirSpeed >  1.0f )  cirSpeed =  1.0f;
		if ( cirSpeed < -1.0f )  cirSpeed = -1.0f;

		dist = Length2d(m_goal, pos);
		linSpeed = dist/(m_physics->RetLinStopLength()*1.5f);
		if ( linSpeed >  1.0f )  linSpeed =  1.0f;

		if ( dist < 20.0f && Abs(cirSpeed) >= 0.5f )
		{
			linSpeed = 0.0f;  // tourne d'abord, puis avance
		}
#else
		pos = m_object->RetPosition(0);

		rot.x = m_goal.x-pos.x;
		rot.y = m_goal.z-pos.z;
		dist = Length(rot.x, rot.y);
		rot.x /= dist;
		rot.y /= dist;

		ComputeRepulse(repulse);
		rot.x += repulse.x*2.0f;
		rot.y += repulse.y*2.0f;

		a = m_object->RetAngleY(0);
		g = RotateAngle(rot.x, -rot.y);  // CW !
		cirSpeed = Direction(a, g)*1.0f;
//?		if ( m_physics->RetType() == TYPE_FLYING &&
//?			 m_physics->RetLand()                )  // volant au sol ?
//?		{
//?			cirSpeed *= 4.0f;  // plus de pèche
//?		}
		if ( cirSpeed >  1.0f )  cirSpeed =  1.0f;
		if ( cirSpeed < -1.0f )  cirSpeed = -1.0f;

		dist = Length2d(m_goal, pos);
		linSpeed = dist/(m_physics->RetLinStopLength()*1.5f);
//?		if ( m_physics->RetType() == TYPE_FLYING &&
//?			 m_physics->RetLand()                )  // volant au sol ?
//?		{
//?			linSpeed *= 8.0f;  // plus de pèche
//?		}
		if ( linSpeed > 1.0f )  linSpeed =  1.0f;

		linSpeed *= 1.0f-(1.0f-0.3f)*Abs(cirSpeed);

		if ( dist < 20.0f && Abs(cirSpeed) >= 0.5f )
		{
			linSpeed = 0.0f;  // tourne d'abord, puis avance
		}
#endif

		m_physics->SetMotorSpeedZ(cirSpeed);  // tourne à gauche/droite
		m_physics->SetMotorSpeedX(linSpeed);  // avance
	}

	if ( m_phase == TGP_TURN   ||  // tourne vers l'objet ?
		 m_phase == TGP_CRTURN ||  // tourne après collision ?
		 m_phase == TGP_CLTURN )   // tourne après collision ?
	{
		a = m_object->RetAngleY(0);
		g = m_angle;
		cirSpeed = Direction(a, g)*1.0f;
		if ( cirSpeed >  1.0f )  cirSpeed =  1.0f;
		if ( cirSpeed < -1.0f )  cirSpeed = -1.0f;

		m_physics->SetMotorSpeedZ(cirSpeed);  // tourne à gauche/droite
	}

	if ( m_phase == TGP_CRWAIT ||  // attend après collision ?
		 m_phase == TGP_CLWAIT )   // attend après collision ?
	{
		m_time += event.rTime;
		m_physics->SetMotorSpeedX(0.0f);  // stoppe l'avance
		m_physics->SetMotorSpeedZ(0.0f);  // stoppe la rotation
	}

	if ( m_phase == TGP_CRADVANCE )  // avance après collision ?
	{
		if ( m_physics->RetCollision() )  // collision ?
		{
			m_physics->SetCollision(FALSE);  // y'a plus
			m_time = 0.0f;
			m_phase = TGP_CLWAIT;
			return TRUE;
		}
		m_physics->SetMotorSpeedX(0.5f);  // avance mollo
	}

	if ( m_phase == TGP_CLADVANCE )  // avance après collision ?
	{
		if ( m_physics->RetCollision() )  // collision ?
		{
			m_physics->SetCollision(FALSE);  // y'a plus
			m_time = 0.0f;
			m_phase = TGP_CRWAIT;
			return TRUE;
		}
		m_physics->SetMotorSpeedX(0.5f);  // avance mollo
	}

	if ( m_phase == TGP_MOVE )  // avance finale ?
	{
		m_bmTimeLimit -= event.rTime;
		m_physics->SetMotorSpeedX(1.0f);
	}

	return TRUE;
}


// Cherche une cible pour le ver.

CObject* CTaskGoto::WormSearch(D3DVECTOR &impact)
{
	CObject*	pObj;
	CObject*	pBest = 0;
	D3DVECTOR	iPos, oPos;
	ObjectType	oType;
	float		distance, min, radius;
	int			i;

	iPos = m_object->RetPosition(0);
	min = 1000000.0f;

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		oType = pObj->RetType();
		if ( oType != OBJECT_MOBILEfa &&
			 oType != OBJECT_MOBILEta &&
			 oType != OBJECT_MOBILEwa &&
			 oType != OBJECT_MOBILEia &&
			 oType != OBJECT_MOBILEfc &&
			 oType != OBJECT_MOBILEtc &&
			 oType != OBJECT_MOBILEwc &&
			 oType != OBJECT_MOBILEic &&
			 oType != OBJECT_MOBILEfi &&
			 oType != OBJECT_MOBILEti &&
			 oType != OBJECT_MOBILEwi &&
			 oType != OBJECT_MOBILEii &&
			 oType != OBJECT_MOBILEfs &&
			 oType != OBJECT_MOBILEts &&
			 oType != OBJECT_MOBILEws &&
			 oType != OBJECT_MOBILEis &&
			 oType != OBJECT_MOBILErt &&
			 oType != OBJECT_MOBILErc &&
			 oType != OBJECT_MOBILErr &&
			 oType != OBJECT_MOBILErs &&
			 oType != OBJECT_MOBILEsa &&
			 oType != OBJECT_MOBILEtg &&
			 oType != OBJECT_MOBILEft &&
			 oType != OBJECT_MOBILEtt &&
			 oType != OBJECT_MOBILEwt &&
			 oType != OBJECT_MOBILEit &&
			 oType != OBJECT_MOBILEdr &&
			 oType != OBJECT_DERRICK  &&
			 oType != OBJECT_STATION  &&
			 oType != OBJECT_FACTORY  &&
			 oType != OBJECT_REPAIR   &&
			 oType != OBJECT_DESTROYER &&
			 oType != OBJECT_CONVERT  &&
			 oType != OBJECT_TOWER    &&
			 oType != OBJECT_RESEARCH &&
			 oType != OBJECT_RADAR    &&
			 oType != OBJECT_INFO     &&
			 oType != OBJECT_ENERGY   &&
			 oType != OBJECT_LABO     &&
			 oType != OBJECT_NUCLEAR  &&
			 oType != OBJECT_PARA     &&
			 oType != OBJECT_SAFE     &&
			 oType != OBJECT_HUSTON   )  continue;

		if ( pObj->RetVirusMode() )  continue;  // objet infecté ?

		if ( !pObj->GetCrashSphere(0, oPos, radius) )  continue;
		distance = Length2d(oPos, iPos);
		if ( distance < min )
		{
			min = distance;
			pBest = pObj;
		}
	}
	if ( pBest == 0 )  return 0;

	impact = pBest->RetPosition(0);
	return pBest;
}

// Contamine les objets proches du ver.

void CTaskGoto::WormFrame(float rTime)
{
	CObject*	pObj;
	D3DVECTOR	impact, pos;
	float		dist;

	m_wormLastTime += rTime;

	if ( m_wormLastTime >= 0.5f )
	{
		m_wormLastTime = 0.0f;

		pObj = WormSearch(impact);
		if ( pObj != 0 )
		{
			pos = m_object->RetPosition(0);
			dist = Length(pos, impact);
			if ( dist <= 15.0f )
			{
				pObj->SetVirusMode(TRUE);  // paf, infecté !
			}
		}
	}
}



// Assigne le but à atteindre.
// "dist" est la distance de laquelle il faut s'éloigner pour
// prendre ou déposer un objet.

Error CTaskGoto::Start(D3DVECTOR goal, float altitude,
					   TaskGotoGoal goalMode, TaskGotoCrash crashMode)
{
	D3DVECTOR	pos;
	CObject*	target;
	ObjectType	type;
	float		dist;
	int			x, y;

	type = m_object->RetType();

	if ( goalMode == TGG_DEFAULT )
	{
		goalMode = TGG_STOP;
		if ( type == OBJECT_MOTHER ||
			 type == OBJECT_ANT    ||
			 type == OBJECT_SPIDER ||
			 type == OBJECT_WORM   )
		{
			goalMode = TGG_EXPRESS;
		}
	}

	if ( crashMode == TGC_DEFAULT )
	{
//?		crashMode = TGC_RIGHTLEFT;
		crashMode = TGC_BEAM;
		if ( type == OBJECT_MOTHER ||
			 type == OBJECT_ANT    ||
			 type == OBJECT_SPIDER ||
			 type == OBJECT_WORM   ||
			 type == OBJECT_BEE    )
		{
			crashMode = TGC_HALT;
		}
	}

	m_altitude   = altitude;
	m_goalMode   = goalMode;
	m_crashMode  = crashMode;
	m_goalObject = goal;
	m_goal       = goal;

	m_bTake = FALSE;
	m_phase = TGP_ADVANCE;
	m_error = ERR_OK;
	m_try = 0;
	m_bmFretObject = 0;
	m_bmFinalMove = 0.0f;

	pos = m_object->RetPosition(0);
	dist = Length2d(pos, m_goal);
	if ( dist < 10.0f && m_crashMode == TGC_BEAM )
	{
		m_crashMode = TGC_RIGHTLEFT;
	}

	m_bWorm = FALSE;
	if ( type == OBJECT_WORM )
	{
		m_bWorm = TRUE;
		m_wormLastTime = 0.0f;
	}

	m_bApprox = FALSE;
	if ( type == OBJECT_HUMAN    ||
		 type == OBJECT_TECH     ||
		 type == OBJECT_MOTHER   ||
		 type == OBJECT_ANT      ||
		 type == OBJECT_SPIDER   ||
		 type == OBJECT_BEE      ||
		 type == OBJECT_WORM     ||
		 type == OBJECT_MOBILErt ||
		 type == OBJECT_MOBILErc ||
		 type == OBJECT_MOBILErr ||
		 type == OBJECT_MOBILErs )
	{
		m_bApprox = TRUE;
	}

	if ( !m_bApprox && m_crashMode != TGC_BEAM )
	{
		target = SearchTarget(goal, 1.0f);
		if ( target != 0 )
		{
			m_goal = target->RetPosition(0);
			dist = 0.0f;
			if ( !AdjustBuilding(m_goal, 1.0f, dist) )
			{
				dist = 0.0f;
				AdjustTarget(target, m_goal, dist);
			}
			m_bTake = TRUE;  // objet à prendre à l'arrivée (rotation finale)
		}
	}

	m_lastDistance = 1000.0f;
	m_physics->SetCollision(FALSE);

	if ( m_crashMode == TGC_BEAM )  // avec l'algorithme des rayons ?
	{
		target = SearchTarget(goal, 1.0f);
		if ( target != 0 )
		{
			m_goal = target->RetPosition(0);
			dist = 4.0f;
			if ( AdjustBuilding(m_goal, 1.0f, dist) )
			{
				m_bmFinalMove = dist;
			}
			else
			{
				dist = 4.0f;
				if ( AdjustTarget(target, m_goal, dist) )
				{
					m_bmFretObject = target;  // fret posé au sol
				}
				else
				{
					m_bmFinalMove = dist;
				}
			}
			m_bTake = TRUE;  // objet à prendre à l'arrivée (rotation finale)
		}

		if ( m_physics->RetType() == TYPE_FLYING && m_altitude == 0.0f )
		{
			pos = m_object->RetPosition(0);
			dist = Length2d(pos, m_goal);
			if ( dist > FLY_DIST_GROUND )  // plus de 20 mètres ?
			{
				m_altitude = FLY_DEF_HEIGHT;  // altitude par défaut
			}
		}

		BeamStart();

		if ( m_bmFretObject == 0 )
		{
			x = (int)((m_goal.x+1600.0f)/BM_DIM_STEP);
			y = (int)((m_goal.z+1600.0f)/BM_DIM_STEP);
			if ( BitmapTestDot(0, x, y) )  // arrivée occupée ?
			{
#if 0
				D3DVECTOR	min, max;
				min = m_object->RetPosition(0);
				max = m_goal;
				if ( min.x > max.x )  Swap(min.x, max.x);
				if ( min.z > max.z )  Swap(min.z, max.z);
				min.x -= 50.0f;
				min.z -= 50.0f;
				max.x += 50.0f;
				max.z += 50.0f;
				BitmapDebug(min, max, m_object->RetPosition(0), m_goal);
#endif
				m_error = ERR_GOTO_BUSY;
				return m_error;
			}
		}
	}

	return ERR_OK;
}

// Indique si l'action est terminée.

Error CTaskGoto::IsEnded()
{
	D3DVECTOR	pos;
	float		limit, angle, dist, h, level;

	if ( m_engine->RetPause() )  return ERR_CONTINUE;
	if ( m_error != ERR_OK )  return m_error;

	pos = m_object->RetPosition(0);

	if ( m_phase == TGP_BEAMLEAK )  // fuite ?
	{
		if ( m_leakTime >= m_leakDelay )
		{
			m_physics->SetMotorSpeedX(0.0f);  // stoppe l'avance
			m_physics->SetMotorSpeedZ(0.0f);  // stoppe la rotation
			BeamInit();
			m_phase = TGP_BEAMSEARCH;  // faudra chercher le chemin
		}
		return ERR_CONTINUE;
	}

	if ( m_phase == TGP_BEAMSEARCH )  // recherche du chemin ?
	{
		return ERR_CONTINUE;
	}

	if ( m_phase == TGP_BEAMWCOLD )  // attend refroidissement réacteur ?
	{
		if ( m_altitude != 0.0f &&
			 m_physics->RetReactorRange() < 1.0f )  return ERR_CONTINUE;
		m_phase = TGP_BEAMUP;
	}

	if ( m_phase == TGP_BEAMUP )  // décolle ?
	{
		if ( m_physics->RetType() == TYPE_FLYING && m_altitude > 0.0f )
		{
			level = m_terrain->RetFloorLevel(pos, TRUE, TRUE);
			h = level+m_altitude-20.0f;
			limit = m_terrain->RetFlyingMaxHeight();
			if ( h > limit )  h = limit;
			if ( pos.y < h-1.0f )  return ERR_CONTINUE;

			m_physics->SetMotorSpeedY(0.0f);  // stoppe la montée
		}
		m_phase = TGP_BEAMGOTO;
	}

	if ( m_phase == TGP_BEAMGOTO )  // goto dot list ?
	{
		if ( m_altitude != 0.0f &&
			 m_physics->RetReactorRange() < 0.1f )  // surchauffe ?
		{
			m_physics->SetMotorSpeedX(0.0f);  // stoppe l'avance
			m_physics->SetMotorSpeedZ(0.0f);  // stoppe la rotation
			m_physics->SetMotorSpeedY(-1.0f);  // tombe
			m_phase = TGP_BEAMWCOLD;
			return ERR_CONTINUE;
		}

		if ( m_physics->RetLand() )  // au sol ?
		{
			limit = 1.0f;
		}
		else	// en vol ?
		{
			limit = 2.0f;
			if ( m_bmIndex < m_bmTotal )  limit *= 2.0f;  // point intermédiaire
		}
		if ( m_bApprox )  limit = 2.0f;

		if ( Abs(pos.x - m_bmPoints[m_bmIndex].x) < limit &&
			 Abs(pos.z - m_bmPoints[m_bmIndex].z) < limit )
		{
			m_physics->SetMotorSpeedX(0.0f);  // stoppe l'avance
			m_physics->SetMotorSpeedZ(0.0f);  // stoppe la rotation

			m_bmIndex = BeamShortcut();

			if ( m_bmIndex > m_bmTotal )
			{
				m_phase = TGP_BEAMDOWN;
			}
		}
	}

	if ( m_phase == TGP_BEAMDOWN )  // atteri ?
	{
		if ( m_physics->RetType() == TYPE_FLYING && m_altitude > 0.0f )
		{
			if ( !m_physics->RetLand() )  return ERR_CONTINUE;
			m_physics->SetMotorSpeedY(0.0f);  // stoppe la descente

			m_altitude = 0.0f;
			m_phase = TGP_BEAMGOTO;  // avance finement au sol pour finir
			m_bmIndex = m_bmTotal;
			return ERR_CONTINUE;
		}

		if ( m_bTake )
		{
			m_angle = RotateAngle(m_goalObject.x-pos.x, pos.z-m_goalObject.z);
			m_phase = TGP_TURN;
		}
		else
		{
			m_physics->SetMotorSpeedX(0.0f);  // stoppe l'avance
			m_physics->SetMotorSpeedZ(0.0f);  // stoppe la rotation
			return ERR_STOP;
		}
	}

	if ( m_goalMode == TGG_EXPRESS )
	{
		dist = Length2d(m_goal, pos);
		if ( dist < 10.0f && dist > m_lastDistance )
		{
			return ERR_STOP;
		}
		m_lastDistance = dist;
	}

	if ( m_phase == TGP_ADVANCE )  // va vers l'objectif ?
	{
		if ( m_physics->RetLand() )  limit = 0.1f;  // au sol
		else                         limit = 1.0f;  // en vol
		if ( m_bApprox )  limit = 2.0f;

		if ( Abs(pos.x - m_goal.x) < limit &&
			 Abs(pos.z - m_goal.z) < limit )
		{
			m_physics->SetMotorSpeedX(0.0f);  // stoppe l'avance
			m_physics->SetMotorSpeedZ(0.0f);  // stoppe la rotation
			m_phase = TGP_LAND;
		}
	}

	if ( m_phase == TGP_LAND )  // atterri ?
	{
		if ( m_physics->RetType() == TYPE_FLYING && m_altitude > 0.0f )
		{
			if ( !m_physics->RetLand() )  return ERR_CONTINUE;
			m_physics->SetMotorSpeedY(0.0f);
		}

		if ( m_bTake )
		{
			m_angle = RotateAngle(m_goalObject.x-pos.x, pos.z-m_goalObject.z);
			m_phase = TGP_TURN;
		}
		else
		{
			return ERR_STOP;
		}
	}

	if ( m_phase == TGP_TURN )  // tourne vers l'objet ?
	{
		angle = NormAngle(m_object->RetAngleY(0));
		limit = 0.02f;
		if ( m_bApprox )  limit = 0.10f;
		if ( Abs(angle-m_angle) < limit )
		{
			m_physics->SetMotorSpeedZ(0.0f);  // stoppe la rotation
			if ( m_bmFinalMove == 0.0f )  return ERR_STOP;

			m_bmFinalPos = m_object->RetPosition(0);
			m_bmFinalDist = m_physics->RetLinLength(m_bmFinalMove);
			m_bmTimeLimit = m_physics->RetLinTimeLength(Abs(m_bmFinalMove))*1.5f;
			if ( m_bmTimeLimit < 0.5f )  m_bmTimeLimit = 0.5f;
			m_phase = TGP_MOVE;
		}
	}

	if ( m_phase == TGP_CRWAIT )  // attend après collision ?
	{
		if ( m_crashMode == TGC_HALT )
		{
			m_physics->SetMotorSpeedX(0.0f);  // stoppe l'avance
			m_physics->SetMotorSpeedZ(0.0f);  // stoppe la rotation
			m_error = ERR_GENERIC;
			return m_error;
		}
		if ( m_time >= 1.0f )
		{
			if ( m_crashMode == TGC_RIGHTLEFT ||
				 m_crashMode == TGC_RIGHT     )  angle =  PI/2.0f;  // 90 à droite
			else                            angle = -PI/2.0f;  // 90 à gauche
			m_angle = NormAngle(m_object->RetAngleY(0)+angle);
			m_phase = TGP_CRTURN;
//?			m_phase = TGP_ADVANCE;
		}
	}

	if ( m_phase == TGP_CRTURN )  // tourne après collision ?
	{
		angle = NormAngle(m_object->RetAngleY(0));
		limit = 0.1f;
		if ( Abs(angle-m_angle) < limit )
		{
			m_physics->SetMotorSpeedZ(0.0f);  // stoppe la rotation
			m_pos = pos;
			m_phase = TGP_CRADVANCE;
		}
	}

	if ( m_phase == TGP_CRADVANCE )  // avance après collision ?
	{
		if ( Length(pos, m_pos) >= 5.0f )
		{
			m_phase = TGP_ADVANCE;
		}
	}

	if ( m_phase == TGP_CLWAIT )  // attend après collision ?
	{
		if ( m_time >= 1.0f )
		{
			if ( m_crashMode == TGC_RIGHTLEFT )  angle = -PI;
			if ( m_crashMode == TGC_LEFTRIGHT )  angle =  PI;
			if ( m_crashMode == TGC_RIGHT     )  angle =  PI/2.0f;
			if ( m_crashMode == TGC_LEFT      )  angle = -PI/2.0f;
			m_angle = NormAngle(m_object->RetAngleY(0)+angle);
			m_phase = TGP_CLTURN;
		}
	}

	if ( m_phase == TGP_CLTURN )  // tourne après collision ?
	{
		angle = NormAngle(m_object->RetAngleY(0));
		limit = 0.1f;
		if ( Abs(angle-m_angle) < limit )
		{
			m_physics->SetMotorSpeedZ(0.0f);  // stoppe la rotation
			m_pos = pos;
			m_phase = TGP_CLADVANCE;
		}
	}

	if ( m_phase == TGP_CLADVANCE )  // avance après collision ?
	{
		if ( Length(pos, m_pos) >= 10.0f )
		{
			m_phase = TGP_ADVANCE;
			m_try ++;
		}
	}

	if ( m_phase == TGP_MOVE )  // avance finale ?
	{
		if ( m_bmTimeLimit <= 0.0f )
		{
			m_physics->SetMotorSpeedX(0.0f);  // stoppe
			Abort();
			return ERR_STOP;
		}

		dist = Length(m_bmFinalPos, m_object->RetPosition(0));
		if ( dist < m_bmFinalDist )  return ERR_CONTINUE;
		m_physics->SetMotorSpeedX(0.0f);  // stoppe l'avance
		return ERR_STOP;
	}

	return ERR_CONTINUE;
}


// Cherche l'objet à la position cible.

CObject* CTaskGoto::SearchTarget(D3DVECTOR pos, float margin)
{
	CObject		*pObj, *pBest;
	D3DVECTOR	oPos;
	float		dist, min;
	int			i;

	pBest = 0;
	min = 1000000.0f;
	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( !pObj->RetActif() )  continue;
		if ( pObj->RetTruck() != 0 )  continue;  // objet porté ?

		oPos = pObj->RetPosition(0);
		dist = Length2d(pos, oPos);

		if ( dist <= margin && dist <= min )
		{
			min = dist;
			pBest = pObj;
		}
	}

	return pBest;
}

// Ajuste la cible en fonction de l'objet.
// Retourne TRUE s'il s'agit de fret posé au sol, dont on peut
// s'approcher par n'importe quel côté.

BOOL CTaskGoto::AdjustTarget(CObject* pObj, D3DVECTOR &pos, float &distance)
{
	ObjectType	type;
	Character*	character;
	D3DMATRIX*	mat;
	D3DVECTOR	goal;
	float		dist, suppl;

	type = m_object->RetType();
	if ( type == OBJECT_BEE  ||
		 type == OBJECT_WORM )
	{
		pos = pObj->RetPosition(0);
		return FALSE;  // approche unique
	}

	type = pObj->RetType();

	if ( type == OBJECT_FRET         ||
		 type == OBJECT_STONE        ||
		 type == OBJECT_URANIUM      ||
		 type == OBJECT_METAL        ||
		 type == OBJECT_POWER        ||
		 type == OBJECT_ATOMIC       ||
		 type == OBJECT_BULLET       ||
		 type == OBJECT_BBOX         ||
		 type == OBJECT_KEYa         ||
		 type == OBJECT_KEYb         ||
		 type == OBJECT_KEYc         ||
		 type == OBJECT_KEYd         ||
		 type == OBJECT_TNT          ||
		 type == OBJECT_SCRAP1       ||
		 type == OBJECT_SCRAP2       ||
		 type == OBJECT_SCRAP3       ||
		 type == OBJECT_SCRAP4       ||
		 type == OBJECT_SCRAP5       ||
		 type == OBJECT_BOMB         ||
		 type == OBJECT_RUINmobilew1 ||
		 type == OBJECT_RUINmobilew2 ||
		 type == OBJECT_RUINmobilet1 ||
		 type == OBJECT_RUINmobilet2 ||
		 type == OBJECT_RUINmobiler1 ||
		 type == OBJECT_RUINmobiler2 )
	{
		pos = m_object->RetPosition(0);
		goal = pObj->RetPosition(0);
		dist = Length(goal, pos);
		pos = (pos-goal)*(TAKE_DIST+distance)/dist + goal;
		return TRUE;  // approche par tous les côtés
	}

	if ( type == OBJECT_BASE )
	{
		pos = m_object->RetPosition(0);
		goal = pObj->RetPosition(0);
		dist = Length(goal, pos);
		pos = (pos-goal)*(TAKE_DIST+distance)/dist + goal;
		return TRUE;  // approche par tous les côtés
	}

	if ( type == OBJECT_MOBILEfa ||
		 type == OBJECT_MOBILEta ||
		 type == OBJECT_MOBILEwa ||
		 type == OBJECT_MOBILEia ||
		 type == OBJECT_MOBILEfs ||
		 type == OBJECT_MOBILEts ||
		 type == OBJECT_MOBILEws ||
		 type == OBJECT_MOBILEis ||
		 type == OBJECT_MOBILEfc ||
		 type == OBJECT_MOBILEtc ||
		 type == OBJECT_MOBILEwc ||
		 type == OBJECT_MOBILEic ||
		 type == OBJECT_MOBILEfi ||
		 type == OBJECT_MOBILEti ||
		 type == OBJECT_MOBILEwi ||
		 type == OBJECT_MOBILEii ||
		 type == OBJECT_MOBILErt ||
		 type == OBJECT_MOBILErc ||
		 type == OBJECT_MOBILErr ||
		 type == OBJECT_MOBILErs ||
		 type == OBJECT_MOBILEsa ||
		 type == OBJECT_MOBILEtg ||
		 type == OBJECT_MOBILEft ||
		 type == OBJECT_MOBILEtt ||
		 type == OBJECT_MOBILEwt ||
		 type == OBJECT_MOBILEit ||
		 type == OBJECT_MOBILEdr )
	{
		character = pObj->RetCharacter();
		pos = character->posPower;
		pos.x -= TAKE_DIST+TAKE_DIST_OTHER+distance;
		mat = pObj->RetWorldMatrix(0);
		pos = Transform(*mat, pos);
		return FALSE;  // approche unique
	}

	if ( GetHotPoint(pObj, goal, TRUE, distance, suppl) )
	{
		pos = goal;
		distance += suppl;
		return FALSE;  // approche unique
	}

	pos = pObj->RetPosition(0);
	distance = 0.0f;
	return FALSE;  // approche unique
}

// S'il on est sur un objet produit par un bâtiment (minerai produit
// par derrick), modifie la position par-rapport au bâtiment.

BOOL CTaskGoto::AdjustBuilding(D3DVECTOR &pos, float margin, float &distance)
{
	CObject*	pObj;
	D3DVECTOR	oPos;
	float		dist, suppl;
	int			i;

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( !pObj->RetActif() )  continue;
		if ( pObj->RetTruck() != 0 )  continue;  // objet porté ?

		if ( !GetHotPoint(pObj, oPos, FALSE, 0.0f, suppl) )  continue;
		dist = Length2d(pos, oPos);
		if ( dist <= margin )
		{
			GetHotPoint(pObj, pos, TRUE, distance, suppl);
			distance += suppl;
			return TRUE;
		}
	}
	return FALSE;
}

// Retourne le point où est produit ou posé qq chose sur un bâtiment.

BOOL CTaskGoto::GetHotPoint(CObject *pObj, D3DVECTOR &pos,
							BOOL bTake, float distance, float &suppl)
{
	ObjectType	type;
	D3DMATRIX*	mat;

	pos = D3DVECTOR(0.0f, 0.0f, 0.0f);
	suppl = 0.0f;
	type = pObj->RetType();

	if ( type == OBJECT_DERRICK )
	{
		mat = pObj->RetWorldMatrix(0);
		pos.x += 8.0f;
		if ( bTake && distance != 0.0f )  suppl = 4.0f;
		if ( bTake )  pos.x += TAKE_DIST+distance+suppl;
		pos = Transform(*mat, pos);
		return TRUE;
	}

	if ( type == OBJECT_CONVERT )
	{
		mat = pObj->RetWorldMatrix(0);
		pos.x += 0.0f;
		if ( bTake && distance != 0.0f )  suppl = 4.0f;
		if ( bTake )  pos.x += TAKE_DIST+distance+suppl;
		pos = Transform(*mat, pos);
		return TRUE;
	}

	if ( type == OBJECT_RESEARCH )
	{
		mat = pObj->RetWorldMatrix(0);
		pos.x += 10.0f;
		if ( bTake && distance != 0.0f )  suppl = 2.5f;
		if ( bTake )  pos.x += TAKE_DIST+TAKE_DIST_OTHER+distance+suppl;
		pos = Transform(*mat, pos);
		return TRUE;
	}

	if ( type == OBJECT_ENERGY )
	{
		mat = pObj->RetWorldMatrix(0);
		pos.x += 6.0f;
		if ( bTake && distance != 0.0f )  suppl = 6.0f;
		if ( bTake )  pos.x += TAKE_DIST+TAKE_DIST_OTHER+distance;
		pos = Transform(*mat, pos);
		return TRUE;
	}

	if ( type == OBJECT_TOWER )
	{
		mat = pObj->RetWorldMatrix(0);
		pos.x += 5.0f;
		if ( bTake && distance != 0.0f )  suppl = 4.0f;
		if ( bTake )  pos.x += TAKE_DIST+TAKE_DIST_OTHER+distance+suppl;
		pos = Transform(*mat, pos);
		return TRUE;
	}

	if ( type == OBJECT_LABO )
	{
		mat = pObj->RetWorldMatrix(0);
		pos.x += 6.0f;
		if ( bTake && distance != 0.0f )  suppl = 6.0f;
		if ( bTake )  pos.x += TAKE_DIST+TAKE_DIST_OTHER+distance;
		pos = Transform(*mat, pos);
		return TRUE;
	}

	if ( type == OBJECT_NUCLEAR )
	{
		mat = pObj->RetWorldMatrix(0);
		pos.x += 22.0f;
		if ( bTake && distance != 0.0f )  suppl = 4.0f;
		if ( bTake )  pos.x += TAKE_DIST+TAKE_DIST_OTHER+distance+suppl;
		pos = Transform(*mat, pos);
		return TRUE;
	}

	if ( type == OBJECT_FACTORY )
	{
		mat = pObj->RetWorldMatrix(0);
		pos.x += 4.0f;
		if ( bTake && distance != 0.0f )  suppl = 6.0f;
		if ( bTake )  pos.x += TAKE_DIST+distance+suppl;
		pos = Transform(*mat, pos);
		return TRUE;
	}

	if ( type == OBJECT_STATION )
	{
		mat = pObj->RetWorldMatrix(0);
		pos.x += 4.0f;
		if ( bTake && distance != 0.0f )  suppl = 4.0f;
		if ( bTake )  pos.x += distance;
		pos = Transform(*mat, pos);
		return TRUE;
	}

	if ( type == OBJECT_REPAIR )
	{
		mat = pObj->RetWorldMatrix(0);
		pos.x += 4.0f;
		if ( bTake && distance != 0.0f )  suppl = 4.0f;
		if ( bTake )  pos.x += distance;
		pos = Transform(*mat, pos);
		return TRUE;
	}

	if ( type == OBJECT_DESTROYER )
	{
		mat = pObj->RetWorldMatrix(0);
		pos.x += 0.0f;
		if ( bTake && distance != 0.0f )  suppl = 4.0f;
		if ( bTake )  pos.x += TAKE_DIST+distance+suppl;
		pos = Transform(*mat, pos);
		return TRUE;
	}

	if ( type == OBJECT_PARA && m_physics->RetType() == TYPE_FLYING )
	{
		mat = pObj->RetWorldMatrix(0);
		if ( bTake && distance != 0.0f )  suppl = 20.0f;
		if ( bTake )  pos.x += distance+suppl;
		pos = Transform(*mat, pos);
		return TRUE;
	}

	suppl = 0.0f;
	return FALSE;
}


// Cherche un objet trop proche qu'il faut fuire.

BOOL CTaskGoto::LeakSearch(D3DVECTOR &pos, float &delay)
{
	CObject		*pObj, *pObstacle;
	D3DVECTOR	iPos, oPos, bPos;
	float		iRadius, oRadius, bRadius, dist, min, dir;
	int			i, j;

	if ( !m_physics->RetLand() )  return FALSE;  // en vol ?

	m_object->GetCrashSphere(0, iPos, iRadius);

	min = 100000.0f;
	bRadius = 0.0f;
	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( pObj == m_object )  continue;
		if ( !pObj->RetActif() )  continue;
		if ( pObj->RetTruck() != 0 )  continue;  // objet porté ?

		j = 0;
		while ( pObj->GetCrashSphere(j++, oPos, oRadius) )
		{
			dist = Length2d(oPos, iPos);
			if ( dist < min )
			{
				min = dist;
				bPos = oPos;
				bRadius = oRadius;
				pObstacle = pObj;
			}
		}
	}
	if ( min > iRadius+bRadius+4.0f )  return FALSE;

	m_bLeakRecede = FALSE;

	dist = 4.0f;
	dir  = 1.0f;
	if ( pObstacle->RetType() == OBJECT_FACTORY )
	{
		dist = 16.0f;
		dir  = -1.0f;
		m_bLeakRecede = TRUE;  // recule simplement
	}

	pos = bPos;
	delay = m_physics->RetLinTimeLength(dist, dir);
	return TRUE;
}


// Calcule la force de répulsion en fonction des obstacles.
// La longueur du vecteur rendu est comprise entre 0 et 1.

void CTaskGoto::ComputeRepulse(FPOINT &dir)
{
#if 0
	D3DVECTOR	iPos, oPos;
	FPOINT		repulse;
	CObject		*pObj;
	float		dist, iRadius, oRadius;
	int			i;

	dir.x = 0.0f;
	dir.y = 0.0f;

	m_object->GetCrashSphere(0, iPos, iRadius);

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( pObj == m_object )  continue;
		if ( pObj->RetTruck() != 0 )  continue;
		
		oPos = pObj->RetPosition(0);
		dist = Length(oPos, m_goalObject);
		if ( dist <= 1.0f )  continue;

		pObj->GetGlobalSphere(oPos, oRadius);
		oRadius += iRadius+m_physics->RetLinStopLength()*1.1f;
		dist = Length2d(oPos, iPos);
		if ( dist <= oRadius )
		{
			repulse.x = iPos.x-oPos.x;
			repulse.y = iPos.z-oPos.z;

//?			dist = 0.2f-(0.2f*dist/oRadius);
			dist = powf(dist/oRadius, 2.0f);
			dist = 0.2f-0.2f*dist;
			repulse.x *= dist;
			repulse.y *= dist;
//?			repulse.x /= dist;
//?			repulse.y /= dist;

			dir.x += repulse.x;
			dir.y += repulse.y;
		}
	}
#else
	ObjectType	iType, oType;
	D3DVECTOR	iPos, oPos;
	FPOINT		repulse;
	CObject		*pObj;
	float		gDist, add, addi, fac, dist, iRadius, oRadius;
	int			i, j;
	BOOL		bAlien;

	dir.x = 0.0f;
	dir.y = 0.0f;

	// Le ver passe partout et à travers tout !
	iType = m_object->RetType();
	if ( iType == OBJECT_WORM )  return;

	m_object->GetCrashSphere(0, iPos, iRadius);
	gDist = Length(iPos, m_goal);

	add = m_physics->RetLinStopLength()*1.1f;  // distance de freinage
	fac = 2.0f;

	if ( iType == OBJECT_MOBILEwa ||
		 iType == OBJECT_MOBILEwc ||
		 iType == OBJECT_MOBILEwi ||
		 iType == OBJECT_MOBILEws ||
		 iType == OBJECT_MOBILEwt )  // roues ?
	{
		add = 5.0f;
		fac = 1.5f;
	}
	if ( iType == OBJECT_MOBILEta ||
		 iType == OBJECT_MOBILEtc ||
		 iType == OBJECT_MOBILEti ||
		 iType == OBJECT_MOBILEts ||
		 iType == OBJECT_MOBILEtt ||
		 iType == OBJECT_MOBILEdr )  // chenilles ?
	{
		add = 4.0f;
		fac = 1.5f;
	}
	if ( iType == OBJECT_MOBILEfa ||
		 iType == OBJECT_MOBILEfc ||
		 iType == OBJECT_MOBILEfi ||
		 iType == OBJECT_MOBILEfs ||
		 iType == OBJECT_MOBILEft )  // volant ?
	{
		if ( m_physics->RetLand() )
		{
			add = 5.0f;
			fac = 1.5f;
		}
		else
		{
			add = 10.0f;
			fac = 1.5f;
		}
	}
	if ( iType == OBJECT_MOBILEia ||
		 iType == OBJECT_MOBILEic ||
		 iType == OBJECT_MOBILEii ||
		 iType == OBJECT_MOBILEis ||
		 iType == OBJECT_MOBILEit )  // pattes ?
	{
		add = 4.0f;
		fac = 1.5f;
	}
	if ( iType == OBJECT_BEE )  // guêpe ?
	{
		if ( m_physics->RetLand() )
		{
			add = 3.0f;
			fac = 1.5f;
		}
		else
		{
			add = 5.0f;
			fac = 1.5f;
		}
	}

	bAlien = FALSE;
	if ( iType == OBJECT_MOTHER ||
		 iType == OBJECT_ANT    ||
		 iType == OBJECT_SPIDER ||
		 iType == OBJECT_BEE    ||
		 iType == OBJECT_WORM   )
	{
		bAlien = TRUE;
	}

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( pObj == m_object )  continue;
		if ( pObj->RetTruck() != 0 )  continue;

		oType = pObj->RetType();

		if ( oType == OBJECT_WORM )  continue;

		if ( bAlien )
		{
			if ( oType == OBJECT_STONE   ||
				 oType == OBJECT_URANIUM ||
				 oType == OBJECT_METAL   ||
				 oType == OBJECT_POWER   ||
				 oType == OBJECT_ATOMIC  ||
				 oType == OBJECT_BULLET  ||
				 oType == OBJECT_BBOX    ||
				 oType == OBJECT_KEYa    ||
				 oType == OBJECT_KEYb    ||
				 oType == OBJECT_KEYc    ||
				 oType == OBJECT_KEYd    ||
				 oType == OBJECT_TNT     ||
				 oType == OBJECT_SCRAP1  ||
				 oType == OBJECT_SCRAP2  ||
				 oType == OBJECT_SCRAP3  ||
				 oType == OBJECT_SCRAP4  ||
				 oType == OBJECT_SCRAP5  ||
				 oType == OBJECT_BOMB    ||
				(oType >= OBJECT_PLANT0    &&
				 oType <= OBJECT_PLANT19   ) ||
				(oType >= OBJECT_MUSHROOM0 &&
				 oType <= OBJECT_MUSHROOM9 ) )  continue;
		}

		addi = add;
		if ( iType == OBJECT_BEE &&
			 oType == OBJECT_BEE )
		{
			addi = 2.0f;  // entre guèpes, faut pas trop s'embêter
		}
		
		j = 0;
		while ( pObj->GetCrashSphere(j++, oPos, oRadius) )
		{
			if ( oPos.y-oRadius > iPos.y+iRadius )  continue;
			if ( oPos.y+oRadius < iPos.y-iRadius )  continue;

			dist = Length(oPos, m_goal);
			if ( dist <= 1.0f )  continue;  // sur le but ?

			oRadius += iRadius+addi;
			dist = Length2d(oPos, iPos);
			if ( dist > gDist )  continue;  // plus loin que le but ?
			if ( dist <= oRadius )
			{
				repulse.x = iPos.x-oPos.x;
				repulse.y = iPos.z-oPos.z;

				dist = powf(dist/oRadius, fac);
				dist = 0.2f-0.2f*dist;
				repulse.x *= dist;
				repulse.y *= dist;

				dir.x += repulse.x;
				dir.y += repulse.y;
			}
		}
	}
#endif
}

// Calcule la force de répulsion verticale en fonction des obstacles.
// La longueur du vecteur rendu est comprise entre -1 et 1.

void CTaskGoto::ComputeFlyingRepulse(float &dir)
{
	ObjectType	oType;
	D3DVECTOR	iPos, oPos;
	CObject		*pObj;
	float		add, fac, dist, iRadius, oRadius, repulse;
	int			i, j;

	m_object->GetCrashSphere(0, iPos, iRadius);

	add = 0.0f;
	fac = 1.5f;
	dir = 0.0f;

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( pObj == m_object )  continue;
		if ( pObj->RetTruck() != 0 )  continue;

		oType = pObj->RetType();

		if ( oType == OBJECT_WORM )  continue;

		j = 0;
		while ( pObj->GetCrashSphere(j++, oPos, oRadius) )
		{
			oRadius += iRadius+add;
			dist = Length2d(oPos, iPos);
			if ( dist <= oRadius )
			{
				repulse = iPos.y-oPos.y;

				dist = powf(dist/oRadius, fac);
				dist = 0.2f-0.2f*dist;
				repulse *= dist;

				dir += repulse;
			}
		}
	}

	if ( dir < -1.0f )  dir = -1.0f;
	if ( dir >  1.0f )  dir =  1.0f;
}



// Parmi tous les points suivants, cherche s'il en existe un qui
// permet d'y aller directement à vol d'oiseau. Si oui, saute tous
// les points intermédiaires inutiles.

int CTaskGoto::BeamShortcut()
{
	int		i;

	for ( i=m_bmTotal ; i>=m_bmIndex+2 ; i-- )  // cherche depuis le dernier
	{
		if ( BitmapTestLine(m_bmPoints[m_bmIndex], m_bmPoints[i], 0.0f, FALSE) )
		{
			return i;  // bingo, trouvé
		}
	}

	return m_bmIndex+1;  // va simplement au point suivant
}

// C'est le grand départ.

void CTaskGoto::BeamStart()
{
	D3DVECTOR	min, max;

	BitmapOpen();
	BitmapObject();

	min = m_object->RetPosition(0);
	max = m_goal;
	if ( min.x > max.x )  Swap(min.x, max.x);
	if ( min.z > max.z )  Swap(min.z, max.z);
	min.x -= 10.0f*BM_DIM_STEP;
	min.z -= 10.0f*BM_DIM_STEP;
	max.x += 10.0f*BM_DIM_STEP;
	max.z += 10.0f*BM_DIM_STEP;
	BitmapTerrain(min, max);

	if ( LeakSearch(m_leakPos, m_leakDelay) )
	{
		m_phase = TGP_BEAMLEAK;  // il faut d'abord fuire
		m_leakTime = 0.0f;
	}
	else
	{
		m_physics->SetMotorSpeedX(0.0f);  // stoppe l'avance
		m_physics->SetMotorSpeedZ(0.0f);  // stoppe la rotation
		BeamInit();
		m_phase = TGP_BEAMSEARCH;  // faudra chercher le chemin
	}
}

// Initialisation avant le premier BeamSearch.

void CTaskGoto::BeamInit()
{
	int		i;

	for ( i=0 ; i<MAXPOINTS ; i++ )
	{
		m_bmIter[i] = -1;
	}
	m_bmStep = 0;
}

// Calcule les points par où passer pour aller de start à goal.
// Retourne :
// ERR_OK si c'est bon
// ERR_GOTO_IMPOSSIBLE si impossible
// ERR_GOTO_ITER si avorté car trop de récursions
// ERR_CONTINUE si pas encore fini
// goalRadius: distance à laquelle il faut s'approcher du but

Error CTaskGoto::BeamSearch(const D3DVECTOR &start, const D3DVECTOR &goal,
							float goalRadius)
{
	float     step, len;
	int       nbIter;

	m_bmStep ++;

	len = Length2d(start, goal);
	step = len/5.0f;
	if ( step < BM_DIM_STEP*2.1f )  step = BM_DIM_STEP*2.1f;
	if ( step > 20.0f            )  step = 20.0f;
	nbIter = 200;  // pour ne pas trop baisser le framerate
	m_bmIterCounter = 0;
	return BeamExplore(start, start, goal, goalRadius, 165.0f*PI/180.0f, 22, step, 0, nbIter);
}

// prevPos: position précédente
// curPos:  position courante
// goalPos: position qu'on cherche à atteindre
// angle:   angle par rapport au but qu'on explore
// nbDiv:   nombre du sous-divisions qu'on fait avec angle
// step     longuer d'un pas
// i        nombre de récursions effectuées
// nbIter   nombre max. d'iterations qu'on a le droit de faire avant d'interrompre provisoirement

Error CTaskGoto::BeamExplore(const D3DVECTOR &prevPos, const D3DVECTOR &curPos,
							 const D3DVECTOR &goalPos, float goalRadius,
							 float angle, int nbDiv, float step,
							 int i, int nbIter)
{
	D3DVECTOR	newPos;
	Error		ret;
	int			iDiv, iClear, iLar;
	
	iLar = 0;
	if ( i >= MAXPOINTS )  return ERR_GOTO_ITER;  // trop de récursion
	
	if ( m_bmIter[i] == -1 )
	{
		m_bmIter[i] = 0;

		if ( i == 0 )
		{
			m_bmPoints[i] = curPos;
		}
		else
		{
			if ( !BitmapTestLine(prevPos, curPos, angle/nbDiv, TRUE) )  return ERR_GOTO_IMPOSSIBLE;

			m_bmPoints[i] = curPos;

			if ( Length2d(curPos, goalPos)-goalRadius <= step )
			{
				if ( goalRadius == 0.0f )
				{
					newPos = goalPos;
				}
				else
				{
					newPos = BeamPoint(curPos, goalPos, 0, Length2d(curPos, goalPos)-goalRadius);
				}
				if ( BitmapTestLine(curPos, newPos, angle/nbDiv, FALSE) )
				{
					m_bmPoints[i+1] = newPos;
					m_bmTotal = i+1;
					return ERR_OK;
				}
			}
		}
	}
	
	if ( iLar >= m_bmIter[i] )
	{
		newPos = BeamPoint(curPos, goalPos, 0, step);
		ret = BeamExplore(curPos, newPos, goalPos, goalRadius, angle, nbDiv, step, i+1, nbIter);
		if ( ret != ERR_GOTO_IMPOSSIBLE )  return ret;
		m_bmIter[i] = iLar+1;
		for ( iClear=i+1 ; iClear<=MAXPOINTS ; iClear++ )  m_bmIter[iClear] = -1;
		m_bmIterCounter ++;
		if ( m_bmIterCounter >= nbIter )  return ERR_CONTINUE;
	}
	iLar ++;

	for ( iDiv=1 ; iDiv<=nbDiv ; iDiv++ )
	{
		if ( iLar >= m_bmIter[i] )
		{
			newPos = BeamPoint(curPos, goalPos, angle*iDiv/nbDiv, step);
			ret = BeamExplore(curPos, newPos, goalPos, goalRadius, angle, nbDiv, step, i+1, nbIter);
			if ( ret != ERR_GOTO_IMPOSSIBLE )  return ret;
			m_bmIter[i] = iLar+1;
			for ( iClear=i+1 ; iClear<=MAXPOINTS ; iClear++ )  m_bmIter[iClear] = -1;
			m_bmIterCounter ++;
			if ( m_bmIterCounter >= nbIter )  return ERR_CONTINUE;
		}
		iLar ++;

		if ( iLar >= m_bmIter[i] )
		{
			newPos = BeamPoint(curPos, goalPos, -angle*iDiv/nbDiv, step);
			ret = BeamExplore(curPos, newPos, goalPos, goalRadius, angle, nbDiv, step, i+1, nbIter);
			if ( ret != ERR_GOTO_IMPOSSIBLE )  return ret;
			m_bmIter[i] = iLar+1;
			for ( iClear=i+1 ; iClear<=MAXPOINTS ; iClear++ )  m_bmIter[iClear] = -1;
			m_bmIterCounter ++;
			if ( m_bmIterCounter >= nbIter )  return ERR_CONTINUE;
		}
		iLar ++;
	}

	return ERR_GOTO_IMPOSSIBLE;
}

// Soit une droite "start-goal". Calcule le point situé à la distance
// "step" du point "start" et faisant un angle "angle" avec la droite.

D3DVECTOR CTaskGoto::BeamPoint(const D3DVECTOR &startPoint,
							   const D3DVECTOR &goalPoint,
							   float angle, float step)
{
	D3DVECTOR	resPoint;
	float		goalAngle;

	goalAngle = RotateAngle(goalPoint.x-startPoint.x, goalPoint.z-startPoint.z);

	resPoint.x = startPoint.x + cosf(goalAngle+angle)*step;
	resPoint.z = startPoint.z + sinf(goalAngle+angle)*step;
	resPoint.y = 0.0f;

	return resPoint;
}

// Affiche une partion de bitmap.

void CTaskGoto::BitmapDebug(const D3DVECTOR &min, const D3DVECTOR &max,
							const D3DVECTOR &start, const D3DVECTOR &goal)
{
	int		minx, miny, maxx, maxy, x, y, i ,n;
	char	s[2000];

	minx = (int)((min.x+1600.0f)/BM_DIM_STEP);
	miny = (int)((min.z+1600.0f)/BM_DIM_STEP);
	maxx = (int)((max.x+1600.0f)/BM_DIM_STEP);
	maxy = (int)((max.z+1600.0f)/BM_DIM_STEP);

	if ( minx > maxx )  Swap(minx, maxx);
	if ( miny > maxy )  Swap(miny, maxy);

	OutputDebugString("Bitmap :\n");
	for ( y=miny ; y<=maxy ; y++ )
	{
		s[0] = 0;
		for ( x=minx ; x<=maxx ; x++ )
		{
			n = -1;
			for ( i=0 ; i<=m_bmTotal ; i++ )
			{
				if ( x == (int)((m_bmPoints[i].x+1600.0f)/BM_DIM_STEP) &&
					 y == (int)((m_bmPoints[i].z+1600.0f)/BM_DIM_STEP) )
				{
					n = i;
					break;
				}
			}

			if ( BitmapTestDot(0, x,y) )
			{
				strcat(s, "o");
			}
			else
			{
				if ( BitmapTestDot(1, x,y) )
				{
					strcat(s, "-");
				}
				else
				{
					strcat(s, ".");
				}
			}

			if ( x == (int)((start.x+1600.0f)/BM_DIM_STEP) &&
				 y == (int)((start.z+1600.0f)/BM_DIM_STEP) )
			{
				strcat(s, "s");
			}
			else
			if ( x == (int)((goal.x+1600.0f)/BM_DIM_STEP) &&
				 y == (int)((goal.z+1600.0f)/BM_DIM_STEP) )
			{
				strcat(s, "g");
			}
			else
			if ( n != -1 )
			{
				char ss[2];
				ss[0] = 'A'+n;
				ss[1] = 0;
				strcat(s, ss);
			}
			else
			{
				strcat(s, " ");
			}
		}
		strcat(s, "\n");
		OutputDebugString(s);
	}
}

// Teste si un chemin le long d'une droite est possible.

BOOL CTaskGoto::BitmapTestLine(const D3DVECTOR &start, const D3DVECTOR &goal,
							   float stepAngle, BOOL bSecond)
{
	D3DVECTOR	pos, inc;
	float		dist, step;
	float		distNoB2;
	int			i, max, x, y;

	if ( m_bmArray == 0 )  return TRUE;

	dist = Length2d(start, goal);
	if ( dist == 0.0f )  return TRUE;
	step = BM_DIM_STEP*0.5f;

	inc.x = (goal.x-start.x)*step/dist;
	inc.z = (goal.z-start.z)*step/dist;

	pos = start;

	if ( bSecond )
	{
		x = (int)((pos.x+1600.0f)/BM_DIM_STEP);
		y = (int)((pos.z+1600.0f)/BM_DIM_STEP);
		BitmapSetDot(1, x, y);  // met le flag du point de départ
	}

	max = (int)(dist/step);
	if ( max == 0 )  max = 1;
	distNoB2 = BM_DIM_STEP*sqrtf(2.0f)/sinf(stepAngle);
	for ( i=0 ; i<max ; i++ )
	{
		if ( i == max-1 )
		{
			pos = goal;  // teste le point d'arrivée
		}
		else
		{
			pos.x += inc.x;
			pos.z += inc.z;
		}

		x = (int)((pos.x+1600.0f)/BM_DIM_STEP);
		y = (int)((pos.z+1600.0f)/BM_DIM_STEP);

		if ( bSecond )
		{
			if ( i > 2 && BitmapTestDot(1, x, y) )  return FALSE;

			if ( step*(i+1) > distNoB2 && i < max-2 )
			{
				BitmapSetDot(1, x, y);
			}
		}

		if ( BitmapTestDot(0, x, y) )  return FALSE;
	}
	return TRUE;
}

// Ajoute les objets dans le bitmap.

void CTaskGoto::BitmapObject()
{
	CObject		*pObj;
	ObjectType	type;
	D3DVECTOR	iPos, oPos;
	float		iRadius, oRadius, h;
	int			i, j;

	m_object->GetCrashSphere(0, iPos, iRadius);

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		type = pObj->RetType();

		if ( pObj == m_object )  continue;
		if ( pObj == m_bmFretObject )  continue;
		if ( pObj->RetTruck() != 0 )  continue;
	
		h = m_terrain->RetFloorLevel(pObj->RetPosition(0), FALSE);
		if ( m_physics->RetType() == TYPE_FLYING && m_altitude > 0.0f )
		{
			h += m_altitude;
		}

		j = 0;
		while ( pObj->GetCrashSphere(j++, oPos, oRadius) )
		{
			if ( m_physics->RetType() == TYPE_FLYING && m_altitude > 0.0f )  // volant ?
			{
				if ( oPos.y-oRadius > h+8.0f ||
					 oPos.y+oRadius < h-8.0f )  continue;
			}
			else	// rampant ?
			{
				if ( oPos.y-oRadius > h+8.0f )  continue;
			}

			if ( type == OBJECT_PARA )  oRadius -= 2.0f;
			BitmapSetCircle(oPos, oRadius+iRadius+4.0f);
		}
	}
}

// Ajoute une portion de terrain dans le bitmap.

void CTaskGoto::BitmapTerrain(const D3DVECTOR &min, const D3DVECTOR &max)
{
	int		minx, miny, maxx, maxy;

	minx = (int)((min.x+1600.0f)/BM_DIM_STEP);
	miny = (int)((min.z+1600.0f)/BM_DIM_STEP);
	maxx = (int)((max.x+1600.0f)/BM_DIM_STEP);
	maxy = (int)((max.z+1600.0f)/BM_DIM_STEP);

	BitmapTerrain(minx, miny, maxx, maxy);
}

// Ajoute une portion de terrain dans le bitmap.

void CTaskGoto::BitmapTerrain(int minx, int miny, int maxx, int maxy)
{
	ObjectType	type;
	D3DVECTOR	p;
	float		aLimit, angle, h;
	int			x, y;
	BOOL		bAcceptWater, bFly;

	if ( minx > maxx )  Swap(minx, maxx);
	if ( miny > maxy )  Swap(miny, maxy);

	if ( minx < 0          )  minx = 0;
	if ( miny < 0          )  miny = 0;
	if ( maxx > m_bmSize-1 )  maxx = m_bmSize-1;
	if ( maxy > m_bmSize-1 )  maxy = m_bmSize-1;

	if ( minx > m_bmMinX )  minx = m_bmMinX;
	if ( miny > m_bmMinY )  miny = m_bmMinY;
	if ( maxx < m_bmMaxX )  maxx = m_bmMaxX;
	if ( maxy < m_bmMaxY )  maxy = m_bmMaxY;

	if ( minx >= m_bmMinX && maxx <= m_bmMaxX &&
		 miny >= m_bmMinY && maxy <= m_bmMaxY )  return;

	aLimit = 20.0f*PI/180.0f;
	bAcceptWater = FALSE;
	bFly = FALSE;

	type = m_object->RetType();

	if ( type == OBJECT_MOBILEwa ||
		 type == OBJECT_MOBILEwc ||
		 type == OBJECT_MOBILEws ||
		 type == OBJECT_MOBILEwi ||
		 type == OBJECT_MOBILEwt ||
		 type == OBJECT_MOBILEtg )  // roues ?
	{
		aLimit = 20.0f*PI/180.0f;
	}

	if ( type == OBJECT_MOBILEta ||
		 type == OBJECT_MOBILEtc ||
		 type == OBJECT_MOBILEti ||
		 type == OBJECT_MOBILEts )  // chenilles ?
	{
		aLimit = 35.0f*PI/180.0f;
	}

	if ( type == OBJECT_MOBILErt ||
		 type == OBJECT_MOBILErc ||
		 type == OBJECT_MOBILErr ||
		 type == OBJECT_MOBILErs )  // grosses chenilles ?
	{
		aLimit = 35.0f*PI/180.0f;
	}

	if ( type == OBJECT_MOBILEsa )  // chenilles sous-marin ?
	{
		aLimit = 35.0f*PI/180.0f;
		bAcceptWater = TRUE;
	}

	if ( type == OBJECT_MOBILEdr )  // chenilles dessinateur ?
	{
		aLimit = 35.0f*PI/180.0f;
	}

	if ( type == OBJECT_MOBILEfa ||
		 type == OBJECT_MOBILEfc ||
		 type == OBJECT_MOBILEfs ||
		 type == OBJECT_MOBILEfi ||
		 type == OBJECT_MOBILEft )  // volant ?
	{
		aLimit = 15.0f*PI/180.0f;
		bFly = TRUE;
	}

	if ( type == OBJECT_MOBILEia ||
		 type == OBJECT_MOBILEic ||
		 type == OBJECT_MOBILEis ||
		 type == OBJECT_MOBILEii )  // pattes d'insecte ?
	{
		aLimit = 60.0f*PI/180.0f;
	}
	
	for ( y=miny ; y<=maxy ; y++ )
	{
		for ( x=minx ; x<=maxx ; x++ )
		{
			if ( x >= m_bmMinX && x <= m_bmMaxX &&
				 y >= m_bmMinY && y <= m_bmMaxY )  continue;

			p.x = x*BM_DIM_STEP-1600.0f;
			p.z = y*BM_DIM_STEP-1600.0f;

			if ( bFly )  // robot volant ?
			{
				h = m_terrain->RetFloorLevel(p, TRUE);
				if ( h >= m_terrain->RetFlyingMaxHeight()-5.0f )
				{
					BitmapSetDot(0, x, y);
				}
				continue;
			}

			if ( !bAcceptWater )  // ne va pas sous l'eau ?
			{
				h = m_terrain->RetFloorLevel(p, TRUE);
				if ( h < m_water->RetLevel()-2.0f )  // sous l'eau (*) ?
				{
//?					BitmapSetDot(0, x, y);
					BitmapSetCircle(p, BM_DIM_STEP*1.0f);
					continue;
				}
			}

			angle = m_terrain->RetFineSlope(p);
			if ( angle > aLimit )
			{
				BitmapSetDot(0, x, y);
			}
		}
	}

	m_bmMinX = minx;
	m_bmMinY = miny;
	m_bmMaxX = maxx;
	m_bmMaxY = maxy;  // agrandi la zone rectangulaire
}

// (*)	Accepte qu'un robot soit 50cm sous l'eau, par exemple
//		sur Tropica 3 !

// Ouvre un bitmap vide.

BOOL CTaskGoto::BitmapOpen()
{
	BitmapClose();

	m_bmSize = (int)(3200.0f/BM_DIM_STEP);
	m_bmArray = (unsigned char*)malloc(m_bmSize*m_bmSize/8*2);
	ZeroMemory(m_bmArray, m_bmSize*m_bmSize/8*2);

	m_bmOffset = m_bmSize/2;
	m_bmLine = m_bmSize/8;

	m_bmMinX = m_bmSize;  // zone rectangulaire inexistante
	m_bmMinY = m_bmSize;
	m_bmMaxX = 0;
	m_bmMaxY = 0;

	return TRUE;
}

// Ferme le bitmap.

BOOL CTaskGoto::BitmapClose()
{
	free(m_bmArray);
	m_bmArray = 0;
	return TRUE;
}

// Met un cercle dans le bitmap.

void CTaskGoto::BitmapSetCircle(const D3DVECTOR &pos, float radius)
{
	float	d, r;
	int		cx, cy, ix, iy;

	cx = (int)((pos.x+1600.0f)/BM_DIM_STEP);
	cy = (int)((pos.z+1600.0f)/BM_DIM_STEP);
	r = radius/BM_DIM_STEP;

	for ( iy=cy-(int)r ; iy<=cy+(int)r ; iy++ )
	{
		for ( ix=cx-(int)r ; ix<=cx+(int)r ; ix++ )
		{
			d = Length((float)(ix-cx), (float)(iy-cy));
			if ( d > r )  continue;
			BitmapSetDot(0, ix, iy);
		}
	}
}

// Enlève un cercle dans le bitmap.

void CTaskGoto::BitmapClearCircle(const D3DVECTOR &pos, float radius)
{
	float	d, r;
	int		cx, cy, ix, iy;

	cx = (int)((pos.x+1600.0f)/BM_DIM_STEP);
	cy = (int)((pos.z+1600.0f)/BM_DIM_STEP);
	r = radius/BM_DIM_STEP;

	for ( iy=cy-(int)r ; iy<=cy+(int)r ; iy++ )
	{
		for ( ix=cx-(int)r ; ix<=cx+(int)r ; ix++ )
		{
			d = Length((float)(ix-cx), (float)(iy-cy));
			if ( d > r )  continue;
			BitmapClearDot(0, ix, iy);
		}
	}
}

// Met un point dans le bitmap.
// x:y: 0..m_bmSize-1

void CTaskGoto::BitmapSetDot(int rank, int x, int y)
{
	if ( x < 0 || x >= m_bmSize ||
		 y < 0 || y >= m_bmSize )  return;

	m_bmArray[rank*m_bmLine*m_bmSize + m_bmLine*y + x/8] |= (1<<x%8);
}

// Enlève un point dans le bitmap.
// x:y: 0..m_bmSize-1

void CTaskGoto::BitmapClearDot(int rank, int x, int y)
{
	if ( x < 0 || x >= m_bmSize ||
		 y < 0 || y >= m_bmSize )  return;

	m_bmArray[rank*m_bmLine*m_bmSize + m_bmLine*y + x/8] &= ~(1<<x%8);
}

// Teste un point dans le bitmap.
// x:y: 0..m_bmSize-1

BOOL CTaskGoto::BitmapTestDot(int rank, int x, int y)
{
	if ( x < 0 || x >= m_bmSize ||
		 y < 0 || y >= m_bmSize )  return FALSE;

	if ( x < m_bmMinX || x > m_bmMaxX ||
		 y < m_bmMinY || y > m_bmMaxY )
	{
		BitmapTerrain(x-10,y-10, x+10,y+10);  // refait une couche
	}

	return m_bmArray[rank*m_bmLine*m_bmSize + m_bmLine*y + x/8] & (1<<x%8);
}


