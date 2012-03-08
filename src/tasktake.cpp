// tasktake.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "D3DEngine.h"
#include "D3DMath.h"
#include "math3d.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "terrain.h"
#include "object.h"
#include "physics.h"
#include "brain.h"
#include "water.h"
#include "camera.h"
#include "motion.h"
#include "motionhuman.h"
#include "sound.h"
#include "robotmain.h"
#include "task.h"
#include "tasktake.h"




// Constructeur de l'objet.

CTaskTake::CTaskTake(CInstanceManager* iMan, CObject* object)
					 : CTask(iMan, object)
{
	CTask::CTask(iMan, object);

	m_terrain = (CTerrain*)m_iMan->SearchInstance(CLASS_TERRAIN);

	m_arm  = TTA_NEUTRAL;
}

// Destructeur de l'objet.

CTaskTake::~CTaskTake()
{
}


// Gestion d'un événement.

BOOL CTaskTake::EventProcess(const Event &event)
{
	float		a, g, cirSpeed;

	if ( m_engine->RetPause() )  return TRUE;
	if ( event.event != EVENT_FRAME )  return TRUE;
	if ( m_bError )  return FALSE;

	if ( m_bTurn )  // rotation préliminaire ?
	{
		a = m_object->RetAngleY(0);
		g = m_angle;
		cirSpeed = Direction(a, g)*2.0f;
		if ( cirSpeed >  1.0f )  cirSpeed =  1.0f;
		if ( cirSpeed < -1.0f )  cirSpeed = -1.0f;

		m_physics->SetMotorSpeedZ(cirSpeed);  // tourne à gauche/droite
		return TRUE;
	}

	m_progress += event.rTime*m_speed;  // ça avance

	m_physics->SetMotorSpeed(D3DVECTOR(0.0f, 0.0f, 0.0f));  // immobile !

	return TRUE;
}


// Assigne le but à atteindre.

Error CTaskTake::Start()
{
	ObjectType	type;
	CObject*	other;
	float		iAngle, oAngle, h;
	D3DVECTOR	pos;

	m_height   = 0.0f;
	m_step     = 0;
	m_progress = 0.0f;

	iAngle = m_object->RetAngleY(0);
	iAngle = NormAngle(iAngle);  // 0..2*PI
	oAngle = iAngle;

	m_bError = TRUE;  // opération impossible
	if ( !m_physics->RetLand() )
	{
		pos = m_object->RetPosition(0);
		h = m_water->RetLevel(m_object);
		if ( pos.y < h )  return ERR_MANIP_WATER;  // impossible sous l'eau
		return ERR_MANIP_FLY;
	}

	type = m_object->RetType();
	if ( type != OBJECT_HUMAN &&
		 type != OBJECT_TECH  )  return ERR_MANIP_VEH;

	m_physics->SetMotorSpeed(D3DVECTOR(0.0f, 0.0f, 0.0f));

	if ( m_object->RetFret() == 0 )
	{
		m_order = TTO_TAKE;
	}
	else
	{
		m_order = TTO_DEPOSE;
	}

	if ( m_order == TTO_TAKE )
	{
		pos = m_object->RetPosition(0);
		h = m_water->RetLevel(m_object);
		if ( pos.y < h )  return ERR_MANIP_WATER;  // impossible sous l'eau

		other = SearchFriendObject(oAngle, 1.5f, PI*0.50f);
		if ( other != 0 && other->RetPower() != 0 )
		{
			type = other->RetPower()->RetType();
			if ( type == OBJECT_URANIUM )  return ERR_MANIP_RADIO;
			if ( type != OBJECT_FRET    &&
				 type != OBJECT_STONE   &&
				 type != OBJECT_BULLET  &&
				 type != OBJECT_METAL   &&
				 type != OBJECT_POWER   &&
				 type != OBJECT_ATOMIC  &&
				 type != OBJECT_BBOX    &&
				 type != OBJECT_KEYa    &&
				 type != OBJECT_KEYb    &&
				 type != OBJECT_KEYc    &&
				 type != OBJECT_KEYd    &&
				 type != OBJECT_TNT     )  return ERR_MANIP_FRIEND;
//?			m_camera->StartCentering(m_object, PI*0.3f, -PI*0.1f, 0.0f, 0.8f);
			m_arm = TTA_FRIEND;
		}
		else
		{
			other = SearchTakeObject(oAngle, 1.5f, PI*0.45f);
			if ( other == 0 )  return ERR_MANIP_NIL;
			type = other->RetType();
			if ( type == OBJECT_URANIUM )  return ERR_MANIP_RADIO;
//?			m_camera->StartCentering(m_object, PI*0.3f, 99.9f, 0.0f, 0.8f);
			m_arm = TTA_FFRONT;
			m_main->HideDropZone(other);  // cache zone constructible
		}
	}

	if ( m_order == TTO_DEPOSE )
	{
//?		speed = m_physics->RetMotorSpeed();
//?		if ( speed.x != 0.0f ||
//?			 speed.z != 0.0f )  return ERR_MANIP_MOTOR;

		other = SearchFriendObject(oAngle, 1.5f, PI*0.50f);
		if ( other != 0 && other->RetPower() == 0 )
		{
//?			m_camera->StartCentering(m_object, PI*0.3f, -PI*0.1f, 0.0f, 0.8f);
			m_arm = TTA_FRIEND;
		}
		else
		{
			if ( !IsFreeDeposeObject(D3DVECTOR(2.5f, 0.0f, 0.0f)) )  return ERR_MANIP_OCC;
//?			m_camera->StartCentering(m_object, PI*0.3f, 99.9f, 0.0f, 0.8f);
			m_arm = TTA_FFRONT;
		}
	}

	m_bTurn = TRUE;  // rotation préliminaire nécessaire
	m_angle = oAngle;  // angle à atteindre

	m_physics->SetFreeze(TRUE);  // on ne bouge plus

	m_bError = FALSE;  // ok
	return ERR_OK;
}

// Indique si l'action est terminée.

Error CTaskTake::IsEnded()
{
	CObject*	fret;
	float		angle;

	if ( m_engine->RetPause() )  return ERR_CONTINUE;
	if ( m_bError )  return ERR_STOP;

	if ( m_bTurn )  // rotation préliminaire ?
	{
		angle = m_object->RetAngleY(0);
		angle = NormAngle(angle);  // 0..2*PI

		if ( TestAngle(angle, m_angle-PI*0.01f, m_angle+PI*0.01f) )
		{
			m_bTurn = FALSE;  // rotation terminée
			m_physics->SetMotorSpeedZ(0.0f);

			if ( m_arm == TTA_FFRONT )
			{
				m_motion->SetAction(MHS_TAKE, 0.2f);  // se baisse
			}
			if ( m_arm == TTA_FRIEND )
			{
				if ( m_height <= 3.0f )
				{
					m_motion->SetAction(MHS_TAKEOTHER, 0.2f);  // se baisse
				}
				else
				{
					m_motion->SetAction(MHS_TAKEHIGH, 0.2f);  // se baisse
				}
			}
			m_progress = 0.0f;
			m_speed = 1.0f/0.6f;
		}
		return ERR_CONTINUE;
	}

	if ( m_progress < 1.0f )  return ERR_CONTINUE;
	m_progress = 0.0f;

	m_step ++;

	if ( m_order == TTO_TAKE )
	{
		if ( m_step == 1 )
		{
			if ( TruckTakeObject() )
			{
				if ( m_arm == TTA_FRIEND &&
					 (m_fretType == OBJECT_POWER  ||
					  m_fretType == OBJECT_ATOMIC ) )
				{
					m_sound->Play(SOUND_POWEROFF, m_object->RetPosition(0));
				}
			}
			m_motion->SetAction(MHS_UPRIGHT, 0.4f);  // se relève
			m_progress = 0.0f;
			m_speed = 1.0f/0.8f;
			m_camera->StopCentering(m_object, 0.8f);
			return ERR_CONTINUE;
		}
	}

	if ( m_order == TTO_DEPOSE )
	{
		if ( m_step == 1 )
		{
			fret = m_object->RetFret();
			TruckDeposeObject();
			if ( m_arm == TTA_FRIEND &&
				 (m_fretType == OBJECT_POWER  ||
				  m_fretType == OBJECT_ATOMIC ) )
			{
				m_sound->Play(SOUND_POWERON, m_object->RetPosition(0));
			}
			if ( fret != 0 && m_fretType == OBJECT_METAL && m_arm == TTA_FFRONT )
			{
				m_main->ShowDropZone(fret, m_object);  // montre zone constructible
			}
			m_motion->SetAction(-1);  // se relève
			m_progress = 0.0f;
			m_speed = 1.0f/0.4f;
			m_camera->StopCentering(m_object, 0.8f);
			return ERR_CONTINUE;
		}
	}

	Abort();
	return ERR_STOP;
}

// Termine brutalement l'action en cours.

BOOL CTaskTake::Abort()
{
	m_motion->SetAction(-1);
	m_camera->StopCentering(m_object, 0.8f);
	m_physics->SetFreeze(FALSE);  // on bouge de nouveau
	return TRUE;
}


// Cherche l'objet à prendre devant.

CObject* CTaskTake::SearchTakeObject(float &angle,
									 float dLimit, float aLimit)
{
	CObject		*pObj, *pBest;
	D3DVECTOR	iPos, oPos;
	ObjectType	type;
	float		min, iAngle, bAngle, a, distance;
	int			i;

	iPos   = m_object->RetPosition(0);
	iAngle = m_object->RetAngleY(0);
	iAngle = NormAngle(iAngle);  // 0..2*PI

	min = 1000000.0f;
	pBest = 0;
	bAngle = 0.0f;
	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		type = pObj->RetType();

		if ( type != OBJECT_FRET    &&
			 type != OBJECT_STONE   &&
			 type != OBJECT_URANIUM &&
			 type != OBJECT_BULLET  &&
			 type != OBJECT_METAL   &&
			 type != OBJECT_POWER   &&
			 type != OBJECT_ATOMIC  &&
			 type != OBJECT_BBOX    &&
			 type != OBJECT_KEYa    &&
			 type != OBJECT_KEYb    &&
			 type != OBJECT_KEYc    &&
			 type != OBJECT_KEYd    &&
			 type != OBJECT_TNT     )  continue;

		if ( pObj->RetTruck() != 0 )  continue;  // objet transporté ?
		if ( pObj->RetLock() )  continue;
		if ( pObj->RetZoomY(0) != 1.0f )  continue;

		oPos = pObj->RetPosition(0);
		distance = Length(oPos, iPos);
		if ( distance >= 4.0f-dLimit &&
			 distance <= 4.0f+dLimit )
		{
			angle = RotateAngle(oPos.x-iPos.x, iPos.z-oPos.z);  // CW !
			if ( TestAngle(angle, iAngle-aLimit, iAngle+aLimit) )
			{
				a = Abs(angle-iAngle);
				if ( a > PI )  a = PI*2.0f-a;
				if ( a < min )
				{
					min = a;
					pBest = pObj;
					bAngle = angle;
				}
			}
		}
	}
	angle = bAngle;
	return pBest;
}

// Cherche le robot sur lequel on veut prendre ou poser une pile.

CObject* CTaskTake::SearchFriendObject(float &angle,
									   float dLimit, float aLimit)
{
	Character*	character;
	CObject*	pObj;
	CObject*	pPower;
	D3DMATRIX*	mat;
	D3DVECTOR	iPos, oPos;
	ObjectType	type, powerType;
	float		iAngle, iRad, distance;
	int			i;

	if ( !m_object->GetCrashSphere(0, iPos, iRad) )  return 0;
	iAngle = m_object->RetAngleY(0);
	iAngle = NormAngle(iAngle);  // 0..2*PI

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( pObj == m_object )  continue;  // soi-même ?

		type = pObj->RetType();
		if ( type != OBJECT_MOBILEfa &&
			 type != OBJECT_MOBILEta &&
			 type != OBJECT_MOBILEwa &&
			 type != OBJECT_MOBILEia &&
			 type != OBJECT_MOBILEfc &&
			 type != OBJECT_MOBILEtc &&
			 type != OBJECT_MOBILEwc &&
			 type != OBJECT_MOBILEic &&
			 type != OBJECT_MOBILEfi &&
			 type != OBJECT_MOBILEti &&
			 type != OBJECT_MOBILEwi &&
			 type != OBJECT_MOBILEii &&
			 type != OBJECT_MOBILEfs &&
			 type != OBJECT_MOBILEts &&
			 type != OBJECT_MOBILEws &&
			 type != OBJECT_MOBILEis &&
			 type != OBJECT_MOBILErt &&
			 type != OBJECT_MOBILErc &&
			 type != OBJECT_MOBILErr &&
			 type != OBJECT_MOBILErs &&
			 type != OBJECT_MOBILEsa &&
			 type != OBJECT_MOBILEtg &&
			 type != OBJECT_MOBILEft &&
			 type != OBJECT_MOBILEtt &&
			 type != OBJECT_MOBILEwt &&
			 type != OBJECT_MOBILEit &&
			 type != OBJECT_TOWER    &&
			 type != OBJECT_RESEARCH &&
			 type != OBJECT_ENERGY   &&
			 type != OBJECT_LABO     &&
			 type != OBJECT_NUCLEAR  )  continue;

		pPower = pObj->RetPower();
		if ( pPower != 0 )
		{
			if ( pPower->RetLock() )  continue;
			if ( pPower->RetZoomY(0) != 1.0f )  continue;

			powerType = pPower->RetType();
			if ( powerType == OBJECT_NULL ||
				 powerType == OBJECT_FIX  )  continue;
		}

		mat = pObj->RetWorldMatrix(0);
		character = pObj->RetCharacter();
		oPos = Transform(*mat, character->posPower);

		distance = Abs(Length(oPos, iPos) - (iRad+1.0f));
		if ( distance <= dLimit )
		{
			angle = RotateAngle(oPos.x-iPos.x, iPos.z-oPos.z);  // CW !
			if ( TestAngle(angle, iAngle-aLimit, iAngle+aLimit) )
			{
				character = pObj->RetCharacter();
				m_height = character->posPower.y;
				return pObj;
			}
		}
	}

	return 0;
}

// Prend l'objet placé devant.

BOOL CTaskTake::TruckTakeObject()
{
	CObject*	fret;
	CObject*	other;
	D3DMATRIX	matRotate;
	float		angle;

	if ( m_arm == TTA_FFRONT )  // prend au sol devant ?
	{
//?		fret = SearchTakeObject(angle, 1.5f, PI*0.04f);
		fret = SearchTakeObject(angle, 1.5f, PI*0.15f);  //OK 1.9
		if ( fret == 0 )  return FALSE;  // rien à prendre ?
		m_fretType = fret->RetType();

		fret->SetTruck(m_object);
		fret->SetTruckPart(4);  // prend avec la main

//?		fret->SetPosition(0, D3DVECTOR(2.2f, -1.0f, 1.1f));
		fret->SetPosition(0, D3DVECTOR(1.7f, -0.5f, 1.1f));
		fret->SetAngleY(0, 0.1f);
		fret->SetAngleX(0, 0.0f);
		fret->SetAngleZ(0, 0.8f);

		m_object->SetFret(fret);  // prend
	}

	if ( m_arm == TTA_FRIEND )  // prend pile sur amis ?
	{
		other = SearchFriendObject(angle, 1.5f, PI*0.04f);
		if ( other == 0 )  return FALSE;

		fret = other->RetPower();
		if ( fret == 0 )  return FALSE;  // l'autre n'a pas de pile ?
		m_fretType = fret->RetType();

		other->SetPower(0);
		fret->SetTruck(m_object);
		fret->SetTruckPart(4);  // prend avec la main

//?		fret->SetPosition(0, D3DVECTOR(2.2f, -1.0f, 1.1f));
		fret->SetPosition(0, D3DVECTOR(1.7f, -0.5f, 1.1f));
		fret->SetAngleY(0, 0.1f);
		fret->SetAngleX(0, 0.0f);
		fret->SetAngleZ(0, 0.8f);

		m_object->SetFret(fret);  // prend
	}

	return TRUE;
}

// Dépose l'objet pris.

BOOL CTaskTake::TruckDeposeObject()
{
	Character*	character;
	CObject*	fret;
	CObject*	other;
	D3DMATRIX*	mat;
	D3DVECTOR	pos;
	float		angle;

	if ( m_arm == TTA_FFRONT )  // dépose au sol devant ?
	{
		fret = m_object->RetFret();
		if ( fret == 0 )  return FALSE;  // ne porte rien ?
		m_fretType = fret->RetType();

		mat = fret->RetWorldMatrix(0);
		pos = Transform(*mat, D3DVECTOR(-0.5f, 1.0f, 0.0f));
		m_terrain->MoveOnFloor(pos);
		fret->SetPosition(0, pos);
		fret->SetAngleY(0, m_object->RetAngleY(0)+PI/2.0f);
		fret->SetAngleX(0, 0.0f);
		fret->SetAngleZ(0, 0.0f);
		fret->FloorAdjust();  // plaque bien au sol

		fret->SetTruck(0);
		m_object->SetFret(0);  // dépose
	}

	if ( m_arm == TTA_FRIEND )  // dépose pile sur amis ?
	{
		other = SearchFriendObject(angle, 1.5f, PI*0.04f);
		if ( other == 0 )  return FALSE;

		fret = other->RetPower();
		if ( fret != 0 )  return FALSE;  // l'autre a déjà une pile ?

		fret = m_object->RetFret();
		if ( fret == 0 )  return FALSE;
		m_fretType = fret->RetType();

		other->SetPower(fret);
		fret->SetTruck(other);

		character = other->RetCharacter();
		fret->SetPosition(0, character->posPower);
		fret->SetAngleY(0, 0.0f);
		fret->SetAngleX(0, 0.0f);
		fret->SetAngleZ(0, 0.0f);
		fret->SetTruckPart(0);  // porté par la base

		m_object->SetFret(0);  // dépose
	}

	return TRUE;
}

// Cherche si un emplacement permet de déposer un objet.

BOOL CTaskTake::IsFreeDeposeObject(D3DVECTOR pos)
{
	CObject*	pObj;
	D3DMATRIX*	mat;
	D3DVECTOR	iPos, oPos;
	float		oRadius;
	int			i, j;

	mat = m_object->RetWorldMatrix(0);
	iPos = Transform(*mat, pos);

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( pObj == m_object )  continue;
		if ( !pObj->RetActif() )  continue;  // inactif ?
		if ( pObj->RetTruck() != 0 )  continue;  // objet transporté ?

		j = 0;
		while ( pObj->GetCrashSphere(j++, oPos, oRadius) )
		{
			if ( Length(iPos, oPos)-(oRadius+1.0f) < 1.0f )
			{
				return FALSE;  // emplacement occupé
			}
		}
	}
	return TRUE;  // emplacement libre
}


