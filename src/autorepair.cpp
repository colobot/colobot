// autorepair.cpp

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
#include "camera.h"
#include "object.h"
#include "brain.h"
#include "motion.h"
#include "motionbot.h"
#include "interface.h"
#include "button.h"
#include "window.h"
#include "robotmain.h"
#include "sound.h"
#include "auto.h"
#include "autorepair.h"





// Constructeur de l'objet.

CAutoRepair::CAutoRepair(CInstanceManager* iMan, CObject* object)
						 : CAuto(iMan, object)
{
	CAuto::CAuto(iMan, object);

	Init();
}

// Destructeur de l'objet.

CAutoRepair::~CAutoRepair()
{
	CAuto::~CAuto();
}


// Détruit l'objet.

void CAutoRepair::DeleteObject(BOOL bAll)
{
	CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoRepair::Init()
{
	m_phase = AREP_WAIT;
	m_pos = m_object->RetPosition(0);
	m_time = 0.0f;
	m_progress = 0.0f;
	m_speed    = 1.0f/1.0f;
	m_lastParticule = 0.0f;
	m_bot = 0;
}


// Démarre l'objet.

void CAutoRepair::Start(int param)
{
}


// Gestion d'un événement.

BOOL CAutoRepair::EventProcess(const Event &event)
{
	D3DMATRIX*	mat;
	D3DVECTOR	pos, src, speed;
	FPOINT		dim, p, c;
	float		progress, angle;

	CAuto::EventProcess(event);

	if ( m_engine->RetPause() )  return TRUE;

	m_progress += event.rTime*m_speed;
	progress = Norm(m_progress);

	if ( m_phase == AREP_WAIT )
	{
		if ( progress >= 1.0f )
		{
			m_bot = SearchObject(OBJECT_BOT1, m_pos, 0.0f, 4.0f);
			if ( m_bot == 0 )
			{
				m_phase    = AREP_WAIT;
				m_speed    = 1.0f/1.0f;
				m_progress = 0.0f;
				progress = 0.0f;
			}
			else
			{
				m_phase    = AREP_DOWN;
				m_speed    = 1.0f/1.5f;
				m_progress = 0.0f;
				progress = 0.0f;
			}
		}
	}

	if ( m_phase == AREP_DOWN )
	{
		angle = PI*0.5f*(1.0f-progress);
		m_object->SetAngleZ(1, angle);  // descend le capteur

		if ( progress >= 1.0f )
		{
			m_phase    = AREP_REPAIR1;
			m_speed    = 1.0f/3.0f;
			m_progress = 0.0f;
			progress = 0.0f;
		}
	}

	if ( m_phase == AREP_REPAIR1 )
	{
		if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
		{
			m_lastParticule = m_time;

			mat = m_object->RetWorldMatrix(0);
			src = Transform(*mat, D3DVECTOR(0.0f, 1.0f, 0.0f));

			pos = src;
			pos.y += 11.0f;
			speed.x = (Rand()-0.5f)*2.0f;
			speed.z = (Rand()-0.5f)*2.0f;
			speed.y = -12.0f;
			dim.x = Rand()*2.0f+2.0f;
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTIFIREZ, 1.0f, 0.0f);
		}

		if ( progress >= 1.0f )
		{
			m_phase    = AREP_REPAIR2;
			m_speed    = 1.0f/4.0f;
			m_progress = 0.0f;
			progress = 0.0f;
		}
	}

	if ( m_phase == AREP_REPAIR2 )
	{
		if ( m_lastParticule+m_engine->ParticuleAdapt(0.10f) <= m_time )
		{
			m_lastParticule = m_time;

			mat = m_object->RetWorldMatrix(0);
			src = Transform(*mat, D3DVECTOR(0.0f, 1.0f, 0.0f));

			pos = src;
			c.x = pos.x;
			c.y = pos.z;
			p.x = c.x;
			p.y = c.y;
			p = RotatePoint(c, Rand()*PI*2.0f, p);
			pos.x = p.x+(Rand()-0.5f)*6.0f;
			pos.z = p.y+(Rand()-0.5f)*6.0f;
			pos.y += 2.0f+Rand()*3.0f;
			speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
			dim.x = Rand()*6.0f+3.0f;
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTIGLINT, 2.0f, 0.0f);

			pos = src;
			speed.x = (Rand()-0.5f)*50.0f;
			speed.z = (Rand()-0.5f)*50.0f;
			speed.y = Rand()*30.0f+15.0f;
			dim.x = Rand()*0.6f+0.6f;
			dim.y = dim.x;
			m_particule->CreateTrack(pos, speed, dim, PARTITRACK2, 2.0f, 50.0f, 1.2f, 1.8f);

			pos = src;
			pos.y += 11.0f;
			speed.x = (Rand()-0.5f)*2.0f;
			speed.z = (Rand()-0.5f)*2.0f;
			speed.y = -12.0f;
			dim.x = Rand()*2.0f+2.0f;
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTIFIREZ, 1.0f, 0.0f);

			m_sound->Play(SOUND_ENERGY, m_object->RetPosition(0),
						  1.0f, 1.0f+Rand()*1.5f);
		}

		if ( progress >= 1.0f )
		{
			m_bot->SetAngleY(0, m_object->RetAngleY(0));
			StartAction(MB_REPAIR, 1.0f);  // debout
			StartAction(MB_WAIT, 1.0f);  // normal
			m_object->SetLock(TRUE);  // bâtiment plus compté par CheckEndMission
//?			m_main->IncProgress();

			m_phase    = AREP_TERM;
			m_speed    = 1.0f/10.0f;
			m_progress = 0.0f;
			progress = 0.0f;
		}
	}

	if ( m_phase == AREP_TERM )
	{
		if ( progress >= 1.0f )
		{
			m_phase    = AREP_TERM;
			m_speed    = 1.0f/10.0f;
			m_progress = 0.0f;
			progress = 0.0f;
		}
	}

	return TRUE;
}


// Cherche un objet proche.

CObject* CAutoRepair::SearchObject(ObjectType type, D3DVECTOR center,
								 float minRadius, float maxRadius)
{
	CObject		*pObj, *pBest;
	CMotion*	motion;
	D3DVECTOR	pos;
	float		min, dist;
	int			i;

	pBest = 0;
	min = 100000.0f;
	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( pObj->RetExplo() )  continue;
		if ( !pObj->RetLock() )  continue;  // libre ?

		if ( type != pObj->RetType() )  continue;

		if ( type == OBJECT_BOT1 )
		{
			motion = pObj->RetMotion();
			if ( motion == 0 )  continue;
			if ( motion->RetAction() != MB_WALK1 )  continue;
		}

		pos = pObj->RetPosition(0);
		dist = Length2d(pos, center);

		if ( dist >= minRadius && dist <= maxRadius && dist < min )
		{
			min = dist;
			pBest = pObj;
		}
	}
	return pBest;
}

// Démarre une action pour le robot.

void CAutoRepair::StartAction(int action, float delay)
{
	CMotion*	motion;

	if ( m_bot == 0 )  return;

	motion = m_bot->RetMotion();
	if ( motion == 0 )  return;

	motion->SetAction(action, delay);
}


// Stoppe l'automate.

BOOL CAutoRepair::Abort()
{
	return TRUE;
}


// Retourne une erreur liée à l'état de l'automate.

Error CAutoRepair::RetError()
{
	return ERR_OK;
}

