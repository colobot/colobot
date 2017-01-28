// autodoor.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "D3DEngine.h"
#include "D3DMath.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "math3d.h"
#include "particule.h"
#include "terrain.h"
#include "camera.h"
#include "object.h"
#include "motion.h"
#include "interface.h"
#include "button.h"
#include "window.h"
#include "robotmain.h"
#include "mainundo.h"
#include "sound.h"
#include "auto.h"
#include "autodoor.h"



#define HEIGHT		-26.0f		// abaissement de la porte
#define TIME_CLOSE	1.0f		// durée fermeture
#define TIME_OPEN	5.0f		// durée ouverture



// Constructeur de l'objet.

CAutoDoor::CAutoDoor(CInstanceManager* iMan, CObject* object)
						 : CAuto(iMan, object)
{
	CAuto::CAuto(iMan, object);

	m_partiStop = -1;
	Init();
}

// Destructeur de l'objet.

CAutoDoor::~CAutoDoor()
{
	if ( m_partiStop != -1 )
	{
		m_particule->DeleteParticule(m_partiStop);
		m_partiStop = -1;
	}

	CAuto::~CAuto();
}


// Détruit l'objet.

void CAutoDoor::DeleteObject(BOOL bAll)
{
	m_phase = ADOP_WAIT;
	FireStopUpdate();
	CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoDoor::Init()
{
	D3DMATRIX*	mat;

	m_type = m_object->RetType();

	m_time = 0.0f;
	m_lastParticuleFire = 0.0f;
	m_lastParticuleRay = 0.0f;

	m_phase    = ADOP_WAIT;
	m_progress = 0.0f;
	m_speed    = 1.0f/1.0f;

	mat = m_object->RetWorldMatrix(0);
	m_posKey = Transform(*mat, D3DVECTOR(-8.0f, 0.0f, -8.0f));

	FireStopUpdate();
}


// Démarre l'objet.

BOOL CAutoDoor::Start(int param)
{
	return FALSE;
}


// Gestion d'un événement.

BOOL CAutoDoor::EventProcess(const Event &event)
{
	CObject*	pKey;
	CObject*	pObj;
	float		progress;

	CAuto::EventProcess(event);

	if ( m_engine->RetPause() )  return TRUE;

	m_progress += event.rTime*m_speed;
	progress = Norm(m_progress);

	if ( m_phase == ADOP_WAIT )  // attend porte fermée ?
	{
		if ( progress >= 1.0f )
		{
			pKey = SearchKey();
			if ( pKey == 0 )
			{
				m_phase    = ADOP_WAIT;
				m_progress = 0.0f;
				m_speed    = 1.0f/1.0f;
				return TRUE;
			}
			else
			{
				m_phase    = ADOP_OPEN;
				m_progress = 0.0f;
				m_speed    = 1.0f/2.0f;
				m_sound->Play(SOUND_OPEN, m_object->RetPosition(0), 1.0f, 1.0f);
				return TRUE;
			}
		}
	}

	if ( m_phase == ADOP_OPEN )
	{
		if ( progress < 1.0f )
		{
			OpenParticule();
			MoveDoor(progress);
			FireStopUpdate();  // clignotte
		}
		else
		{
			m_phase    = ADOP_STOP;
			m_progress = 0.0f;
			m_speed    = 1.0f/1.0f;
			MoveDoor(1.0f);
			FireStopUpdate();  // clignotte
			return TRUE;
		}
	}

	if ( m_phase == ADOP_STOP )  // attend porte ouverte ?
	{
		if ( progress < 1.0f )
		{
			OpenParticule();
		}
		else
		{
			pKey = SearchKey();
			pObj = SearchObject(m_object->RetPosition(0), 7.0f);
			if ( pKey == 0 && pObj == 0 )
			{
				m_phase    = ADOP_CLOSE;
				m_progress = 0.0f;
				m_speed    = 1.0f/1.0f;
				m_sound->Play(SOUND_CLOSE, m_object->RetPosition(0), 1.0f, 1.0f);
				return TRUE;
			}
			else
			{
				m_phase    = ADOP_STOP;
				m_progress = 0.0f;
				m_speed    = 1.0f/1.0f;
				return TRUE;
			}
		}
	}

	if ( m_phase == ADOP_CLOSE )
	{
		if ( progress < 1.0f )
		{
			MoveDoor(progress);
			FireStopUpdate();  // clignotte
		}
		else
		{
			m_phase    = ADOP_WAIT;
			m_progress = 0.0f;
			m_speed    = 1.0f/1.0f;
			MoveDoor(0.0f);
			FireStopUpdate();  // clignotte
			return TRUE;
		}
	}

	return TRUE;
}


// Stoppe l'automate.

BOOL CAutoDoor::Abort()
{
	return TRUE;
}


// Met à jour la position des portes.

void CAutoDoor::MoveDoor(float progress)
{
	D3DVECTOR	pos;
	float		dist, angle;

	if ( m_type == OBJECT_DOOR1 ||
		 m_type == OBJECT_DOOR3 )
	{
		if ( m_phase == ADOP_CLOSE )
		{
			progress = Bounce(1.0f-progress);
		}
		dist = progress*7.5f;

		pos = m_object->RetPosition(1);
		pos.y = dist;
		m_object->SetPosition(1, pos);
	}

	if ( m_type == OBJECT_DOOR2 ||
		 m_type == OBJECT_DOOR4 )
	{
		if ( m_phase == ADOP_CLOSE )
		{
			progress = Bounce(1.0f-progress);
		}
		angle = progress*PI/2.0f;
		m_object->SetAngleY(1, -angle);
		m_object->SetAngleY(2,  angle);
	}

	if ( progress == 0.0f )  UpdateLockZone(FALSE);
	if ( progress == 1.0f )  UpdateLockZone(TRUE);
}

// Met à jour les sphères de collision.

void CAutoDoor::UpdateLockZone(BOOL bOpen)
{
	D3DVECTOR	pos;

	pos = m_object->RetPosition(0);
	m_terrain->SetLockZone(pos, bOpen?LZ_FREE:LZ_FIX);
}

// Met à jour les feux de stop.

void CAutoDoor::FireStopUpdate()
{
	D3DMATRIX*	mat;
	D3DVECTOR	pos, speed;
	FPOINT		dim;
	BOOL		bOn;

	if ( m_type != OBJECT_DOOR1 &&
		 m_type != OBJECT_DOOR2 )  return;
	
	bOn = FALSE;

	if ( m_phase == ADOP_WAIT )
	{
		bOn = TRUE;
	}

	if ( m_phase == ADOP_OPEN )
	{
		bOn = ( Mod(m_time, 0.2f) >= 0.1f );
	}

	if ( m_phase == ADOP_STOP )
	{
		bOn = FALSE;
	}

	if ( m_phase == ADOP_CLOSE )
	{
		bOn = ( Mod(m_time, 0.2f) >= 0.1f );
	}

	if ( bOn )
	{
		if ( m_partiStop == -1 )
		{
			mat = m_object->RetWorldMatrix(0);
			if ( m_type == OBJECT_DOOR1 )
			{
				pos = Transform(*mat, D3DVECTOR(5.0f, 18.0f, 0.0f));
			}
			if ( m_type == OBJECT_DOOR2 )
			{
				pos = Transform(*mat, D3DVECTOR(0.0f, 18.0f, 0.0f));
			}
			speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
			dim.x = 2.0f;
			dim.y = dim.x;
			m_partiStop = m_particule->CreateParticule(pos, speed, dim, PARTISELR, 1.0f, 0.0f);
		}
	}
	else
	{
		if ( m_partiStop != -1 )
		{
			m_particule->DeleteParticule(m_partiStop);
			m_partiStop = -1;
		}
	}
}


// Retourne une erreur liée à l'état de l'automate.

Error CAutoDoor::RetError()
{
	return ERR_OK;
}


// Cherche la clé sous la porte.

CObject* CAutoDoor::SearchKey()
{
	CObject*	pObj;
	D3DVECTOR	oPos;
	ObjectType	oType;
	float		dist;
	int			i;

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;
		if ( pObj->RetLock() )  continue;

		oType = pObj->RetType();

		if ( m_type == OBJECT_DOOR1 ||
			 m_type == OBJECT_DOOR3 )
		{
			if ( oType != OBJECT_KEY1 )  continue;
		}
		if ( m_type == OBJECT_DOOR2 ||
			 m_type == OBJECT_DOOR4 )
		{
			if ( oType != OBJECT_KEY2 )  continue;
		}

		oPos = pObj->RetPosition(0);
		dist = Length(oPos, m_posKey);
		if ( dist <= 2.0f )  return pObj;
	}

	return 0;
}

// Cherche un objet quelconque.

CObject* CAutoDoor::SearchObject(D3DVECTOR center, float radius)
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
		if ( pObj == m_object )  continue;  // soi-même ?
		if ( pObj->RetLock() )  continue;
		if ( !pObj->RetEnable() )  continue;

		type = pObj->RetType();

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

// Particules avec la clé lorsque la porte est ouverte.

void CAutoDoor::OpenParticule()
{
	D3DVECTOR	pos, goal, speed;
	FPOINT		dim, rot;

	if ( m_lastParticuleFire+0.05f <= m_time )
	{
		m_lastParticuleFire = m_time;

		pos = m_posKey;
		pos.y += 11.0f;
		speed.x = (Rand()-0.5f)*1.0f;
		speed.z = (Rand()-0.5f)*1.0f;
		speed.y = -5.0f;
		dim.x = Rand()*0.5f+0.5f;
		dim.y = dim.x;
		m_particule->CreateParticule(pos, speed, dim, PARTIFIREZ, 1.0f);
	}

	if ( m_lastParticuleRay+0.25f <= m_time )
	{
		m_lastParticuleRay = m_time;

		rot = RotatePoint(Rand()*PI*2.0f, 0.5f);
		pos = m_posKey;
		pos.y += 13.0f;
		pos.x += rot.x;
		pos.z += rot.y;
		goal = m_posKey;
		goal.y += 4.0f;
		goal.x -= rot.x;
		goal.z -= rot.y;
		dim.x = 1.5f;
		dim.y = dim.x;
		m_particule->CreateRay(pos, goal, PARTIRAY3, dim, 1.0f);
	}
}


// Ecrit la situation de l'objet.

void CAutoDoor::WriteSituation()
{
	m_undo->WriteTokenInt("phase", m_phase);
}

// lit la situation de l'objet.

void CAutoDoor::ReadSituation()
{
	D3DVECTOR	pos;
	int			phase;

	if ( m_undo->ReadTokenInt("phase", phase) )
	{
		m_phase = (AutoDoorPhase)phase;
		m_progress = 0.0f;
		m_speed = 1.0f/0.001f;  // instantané (ou presque)

		if ( m_phase == ADOP_WAIT  ||
			 m_phase == ADOP_CLOSE )
		{
			m_phase = ADOP_CLOSE;
			MoveDoor(1.0f);
			m_phase == ADOP_WAIT;
		}
		else
		{
			m_phase = ADOP_OPEN;
			MoveDoor(1.0f);
			m_phase == ADOP_STOP;
		}
	}
}

