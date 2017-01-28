// autocatapult.cpp

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
#include "pyro.h"
#include "object.h"
#include "interface.h"
#include "button.h"
#include "window.h"
#include "robotmain.h"
#include "sound.h"
#include "tasklist.h"
#include "auto.h"
#include "autocatapult.h"




// Constructeur de l'objet.

CAutoCatapult::CAutoCatapult(CInstanceManager* iMan, CObject* object)
						 : CAuto(iMan, object)
{
	CAuto::CAuto(iMan, object);

	Init();
}

// Destructeur de l'objet.

CAutoCatapult::~CAutoCatapult()
{
	CAuto::~CAuto();
}


// Détruit l'objet.

void CAutoCatapult::DeleteObject(BOOL bAll)
{
	m_phase = ACAP_WAIT;
	CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoCatapult::Init()
{
	m_time = 0.0f;
	m_phase    = ACAP_WAIT;
	m_progress = 0.0f;
	m_speed    = 1.0f/1.0f;
	m_pBox = 0;

	m_lastParticule = 0.0f;
	m_channelSound = -1;
}


// Démarre l'objet.

BOOL CAutoCatapult::Start(int part)
{
	CTaskList*	taskList;
	D3DVECTOR	pos;

	if ( m_phase != ACAP_WAIT )  return FALSE;  // occupé ?

	if ( part == 1 )  // bouton action ?
	{
		if ( SearchObject(CalcPosition(-8.0f), 7.0f) != 0 )
		{
			return FALSE;
		}

		m_pBox = SearchObject(CalcPosition(0.0f), 2.0f);
		if ( m_pBox == 0 )
		{
			return FALSE;
		}
		else
		{
			m_distFactor = CalcDistFactor();
			if ( m_distFactor == 0.0f )
			{
				return FALSE;
			}

			m_type = m_pBox->RetType();
			if ( (m_type < OBJECT_BOX1  ||
				  m_type > OBJECT_BOX20 ) &&  // pas une caisse ?
				 m_type != OBJECT_BLUPI )     // ni un blupi ?
			{
				return FALSE;
			}

			if ( m_type == OBJECT_BLUPI )
			{
				taskList = m_pBox->RetTaskList();
				if ( taskList != 0 &&
					 taskList->IsRunning() )  return FALSE;
			}

			LockZone(TRUE);

			m_hammerAngle[0] = (  0.0f*PI/180.0f);
			m_hammerAngle[1] = (-45.0f*PI/180.0f)*m_distFactor;
			m_hammerAngle[2] = ( 45.0f*PI/180.0f);
			m_hammerAngle[3] = ( 45.0f*PI/180.0f)+(45.0f*PI/180.0f)*m_distFactor;
			m_hammerAngle[4] = (  0.0f*PI/180.0f);

			pos = CalcPosition(32.0f*m_distFactor);
			m_bFall = !m_terrain->IsSolid(pos);

			m_pBox->SetLock(TRUE);

			m_phase    = ACAP_UP;
			m_progress = 0.0f;
			m_speed    = 1.0f/1.0f;
			m_bBusy    = TRUE;
		}
	}

	if ( m_phase == ACAP_WAIT )  return FALSE;  // rien démarré ?

	return TRUE;
}


// Gestion d'un événement.

BOOL CAutoCatapult::EventProcess(const Event &event)
{
	CPyro*		pyro;
	D3DVECTOR	pos, speed;
	FPOINT		dim;
	float		progress, angle, duration;
	int			i;

	CAuto::EventProcess(event);

	if ( m_engine->RetPause() )  return TRUE;

	m_progress += event.rTime*m_speed;
	progress = Norm(m_progress);

	if ( m_phase == ACAP_WAIT )  // attend ?
	{
		if ( progress >= 1.0f )
		{
			m_phase    = ACAP_WAIT;
			m_progress = 0.0f;
			m_speed    = 1.0f/1.0f;
			return TRUE;
		}
	}

	if ( m_phase == ACAP_UP )  // monte marteau ?
	{
		angle = m_hammerAngle[0]+(m_hammerAngle[1]-m_hammerAngle[0])*progress;
		m_object->SetAngleZ(2, angle);
		m_object->SetAngleZ(3, -angle);

		if ( progress >= 1.0f )
		{
			m_phase    = ACAP_DOWN;
			m_progress = 0.0f;
			m_speed    = 1.0f/0.2f;
			return TRUE;
		}
	}

	if ( m_phase == ACAP_DOWN )  // descend marteau ?
	{
		angle = m_hammerAngle[1]+(m_hammerAngle[2]-m_hammerAngle[1])*progress;
		m_object->SetAngleZ(2, angle);
		m_object->SetAngleZ(3, -angle);

		if ( progress >= 1.0f )
		{
			pos = CalcPosition(0.0f);
			m_sound->Play(m_type==OBJECT_BLUPI?SOUND_BLUPIaie:SOUND_BOUM, pos);
			m_camera->StartEffect(CE_SHOT, pos, m_distFactor);

			for ( i=0 ; i<10 ; i++ )
			{
				pos = CalcPosition(4.0f);
				pos.x += (Rand()-0.5f)*12.0f;
				pos.z += (Rand()-0.5f)*12.0f;
				pos.y += 4.0f;
				speed.x = 0.0f;
				speed.z = 0.0f;
				speed.y = 0.0f;
				dim.x = 4.0f;
				dim.y = dim.x;
				duration = 0.5f+Rand()*3.0f;
				m_particule->CreateParticule(pos, speed, dim, PARTICRASH, duration);
			}

			m_phase    = ACAP_FLY;
			m_progress = 0.0f;
			m_speed    = 1.0f/0.5f;
			return TRUE;
		}
	}

	if ( m_phase == ACAP_FLY )  // caisse vole ?
	{
		angle = m_hammerAngle[2]+(m_hammerAngle[3]-m_hammerAngle[2])*powf(progress, 0.25f);
		m_object->SetAngleZ(2, angle);
		m_object->SetAngleZ(3, -angle);

		if ( m_pBox != 0 )
		{
			pos = CalcPosition(progress*32.0f*m_distFactor);
			pos.y += m_pBox->RetCharacter()->height;
			pos.y += -(powf((progress*2.0f)-1.0f, 2.0f)-1.0f)*10.0f*m_distFactor;
			m_pBox->SetPosition(0, pos);
		}

		if ( progress >= 1.0f )
		{
			if ( m_pBox != 0 )
			{
				m_pBox->SetPosition(0, Grid(m_pBox->RetPosition(0), 8.0f));
				if ( !m_bFall )
				{
					m_pBox->SetLock(FALSE);
				}
				LockZone(FALSE);

				for ( i=0 ; i<20 ; i++ )
				{
					pos = CalcPosition(32.0f*m_distFactor);
					pos.x += (Rand()-0.5f)*12.0f;
					pos.z += (Rand()-0.5f)*12.0f;
					pos.y += 4.0f;
					speed.x = 0.0f;
					speed.z = 0.0f;
					speed.y = 0.0f;
					dim.x = 6.0f;
					dim.y = dim.x;
					duration = 0.5f+Rand()*3.0f;
					m_particule->CreateParticule(pos, speed, dim, PARTICRASH, duration);
				}

				pos = CalcPosition(32.0f*m_distFactor);
				m_sound->Play(m_type==OBJECT_BLUPI?SOUND_BLUPIohhh:SOUND_FALLo3, pos);
				m_camera->StartEffect(CE_EXPLO, pos, m_distFactor);
			}

			m_phase    = ACAP_RETURN;
			m_progress = 0.0f;
			m_speed    = 1.0f/1.0f;
			m_bBusy    = FALSE;
			return TRUE;
		}
	}

	if ( m_phase == ACAP_RETURN )  // marteau au départ ?
	{
		angle = m_hammerAngle[3]+(m_hammerAngle[4]-m_hammerAngle[3])*progress;
		m_object->SetAngleZ(2, angle);
		m_object->SetAngleZ(3, -angle);

		if ( m_bFall && m_pBox != 0 )
		{
			pos = CalcPosition(32.0f*m_distFactor);
			pos.y -= 8.0f*(progress*4.0f);
			if ( m_type == OBJECT_BLUPI )  pos.y -= 8.0f*progress;
			m_pBox->SetPosition(0, pos);

			if ( progress >= 1.0f/4.0f )
			{
				pyro = new CPyro(m_iMan);
				pyro->Create(m_type==OBJECT_BLUPI?PT_EXPLOO:PT_FRAGT, m_pBox);  // destruction caisse
				m_pBox = 0;
			}
		}

		if ( progress >= 1.0f )
		{
			m_phase    = ACAP_WAIT;
			m_progress = 0.0f;
			m_speed    = 1.0f/1.0f;
			m_bBusy    = FALSE;
			return TRUE;
		}
	}

	return TRUE;
}


// Stoppe l'automate.

BOOL CAutoCatapult::Abort()
{
	return TRUE;
}


// Indique si l'objet effectue une action.

BOOL CAutoCatapult::IsRunning()
{
	return ( m_phase != ACAP_WAIT );
}


// Retourne une erreur liée à l'état de l'automate.

Error CAutoCatapult::RetError()
{
	return ERR_OK;
}


// Bloque/débloque toute la zone de catapultage.

void CAutoCatapult::LockZone(BOOL bLock)
{
#if 0
	D3DVECTOR	pos;
	float		dist;
	int			i, max;

	max = (int)(m_distFactor*4.0f);  // 0..4
	if ( bLock )  max ++;
	dist = 0.0f;
	for ( i=0 ; i<max ; i++ )
	{
		pos = CalcPosition(dist);
		m_terrain->SetLockZone(pos, bLock?LZ_BOX:LZ_FREE);
		dist += 8.0f;
	}
#else
// Finalement, on ne s'occupe que des positions de départ et
// d'arrivée, pour permettre de survoller une trappe sans qu'elle
// ne s'ouvre !
	D3DVECTOR	pos;

	if ( bLock )
	{
		pos = CalcPosition(32.0f*m_distFactor);
		m_terrain->SetLockZone(pos, LZ_BOX);
	}
	else
	{
		pos = CalcPosition(0.0f);
		m_terrain->SetLockZone(pos, LZ_FREE);
	}
#endif
}

// Calcule le nombre de cases dont peut avancer la caisse.
//	0 case  -> 0.00
//	1 case  -> 0.25
//	2 cases -> 0.50
//	3 cases -> 0.75
//	4 cases -> 1.00

float CAutoCatapult::CalcDistFactor()
{
	CObject*	pObj;
	D3DVECTOR	pos;
	int			i;

	for ( i=0 ; i<4 ; i++ )
	{
		pos = CalcPosition((i+1)*8.0f);
		pObj = SearchObject(pos, 7.0f);
		if ( pObj != 0 )
		{
			return 0.25f*i;
		}
	}
	return 1.0f;
}

// Calcule une position dans l'axe de la catapulte.
// Une valeur positive correspond à la direction de lancement.

D3DVECTOR CAutoCatapult::CalcPosition(float dist)
{
	D3DMATRIX*	mat;

	mat = m_object->RetWorldMatrix(0);
	return Transform(*mat, D3DVECTOR(dist, 0.0f, 0.0f));
}

// Cherche un objet quelconque.

CObject* CAutoCatapult::SearchObject(D3DVECTOR center, float radius)
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
		if ( type == OBJECT_LIFT )  continue;

		pos = pObj->RetPosition(0);

		if ( type == OBJECT_MAX1X )
		{
			if ( !m_terrain->IsLockZone(pos) )  continue;
		}

		dist = Length2d(pos, center);
		if ( dist <= radius && dist < min )
		{
			min = dist;
			pBest = pObj;
		}
	}
	return pBest;
}

