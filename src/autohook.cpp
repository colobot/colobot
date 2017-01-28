// autohook.cpp

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
#include "physics.h"
#include "brain.h"
#include "terrain.h"
#include "motion.h"
#include "motionbot.h"
#include "interface.h"
#include "button.h"
#include "window.h"
#include "robotmain.h"
#include "sound.h"
#include "auto.h"
#include "autohook.h"





// Constructeur de l'objet.

CAutoHook::CAutoHook(CInstanceManager* iMan, CObject* object)
						 : CAuto(iMan, object)
{
	CAuto::CAuto(iMan, object);

	Init();
}

// Destructeur de l'objet.

CAutoHook::~CAutoHook()
{
	CAuto::~CAuto();
}


// Détruit l'objet.

void CAutoHook::DeleteObject(BOOL bAll)
{
	CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoHook::Init()
{
	m_time          = 0.0f;
	m_progress      = 0.0f;
	m_phase         = AHKP_WAIT;
	m_speed         = 1.0f/1.0f;
	m_lastParticule = 0.0f;
	m_targetPos.x   = NAN;
	m_load          = 0;
}


// Démarre l'objet.

void CAutoHook::Start(int param)
{
}


// Gestion d'un événement.

BOOL CAutoHook::EventProcess(const Event &event)
{
	CObject*	car;
	CPhysics*	physics;
	D3DMATRIX*	mat;
	D3DVECTOR	pos, speed;
	FPOINT		dim;
	float		s, angle, goalAngle, progress, dist, height;

	CAuto::EventProcess(event);

	if ( m_engine->RetPause() )  return TRUE;
	if ( m_main->RetEndingGame() )  return TRUE;

	car = m_object->RetTruck();
	if ( car == 0 )  return TRUE;

	if ( m_load != 0 )
	{
		if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
		{
			m_lastParticule = m_time;

			mat = m_object->RetWorldMatrix(3);
			pos = Transform(*mat, D3DVECTOR(0.0f, 0.0f, 0.0f));
			speed.x = (Rand()-0.5f)*10.0f;
			speed.z = (Rand()-0.5f)*10.0f;
			speed.y = 2.0f+Rand()*5.0f;
			dim.x = 0.2f;
			dim.y = 0.2f;
			m_particule->CreateParticule(pos, speed, dim, PARTIBLITZ, 1.0f, 10.0f);
		}
	}

	m_progress += event.rTime*m_speed;
	progress = Norm(m_progress);

	if ( m_phase == AHKP_WAIT )
	{
		physics = car->RetPhysics();
		if ( physics == 0 )
		{
			s = 0.0f;
		}
		else
		{
			s = Abs(physics->RetLinMotionX(MO_REASPEED));
		}

		if ( s == 0.0f )
		{
			if ( m_load == 0 )  // prend ?
			{
				m_target = SearchObject(OBJECT_BOT1, car->RetPosition(0), 0.0f, 14.0f);
				if ( m_target != 0 )
				{
					car->SetLock(TRUE);  // voiture stoppée
					m_target->SetLock(TRUE);  // robot pris

					m_targetPos = m_target->RetPosition(0);
					pos = car->RetPosition(0);
					goalAngle = RotateAngle(m_targetPos.x-pos.x, pos.z-m_targetPos.z);
					goalAngle -= car->RetAngleY(0);

					m_cameraType = m_camera->RetType();
					m_camera->SetType(CAMERA_BACK);
					angle = NormAngle(goalAngle);
					if ( angle < PI )  angle =  PI*0.1f;
					else               angle = -PI*0.1f;
					m_camera->SetBackHoriz(angle);
					m_camera->SetLockRotate(TRUE);
					m_main->SetStopwatch(FALSE);  // stoppe le chrono

					pos = car->RetCharacter()->posFret;
					mat = car->RetWorldMatrix(0);
					pos = Transform(*mat, pos);
					m_goalAngle = RotateAngle(m_targetPos.x-pos.x, pos.z-m_targetPos.z);
					m_goalAngle -= car->RetAngleY(0);
					m_goalAngle += PI;
					m_goalAngle = NormAngle(m_goalAngle);
					if ( m_goalAngle < PI )
					{
						m_startAngle = m_object->RetAngleY(1);
					}
					else
					{
						m_startAngle = -m_object->RetAngleY(1);
						m_goalAngle -= PI*2.0f;
					}

					m_goalDist = -Length2d(pos, m_targetPos);
					pos = m_object->RetPosition(2);
					m_startDist = pos.x;

					SoundManip(1.0f, 0.8f, 1.0f);

					m_bGrab = TRUE;
					m_phase = AHKP_GOSTART;
					m_speed = 1.0f/1.0f;
					m_progress = 0.0f;
					progress = 0.0f;
				}
			}
			else	// dépose ?
			{
				m_target = SearchObject(OBJECT_REPAIR, car->RetPosition(0), 5.0f, 14.0f);
				if ( m_target != 0 )
				{
					car->SetLock(TRUE);  // voiture stoppée

					m_targetPos = m_target->RetPosition(0);
					pos = car->RetPosition(0);
					goalAngle = RotateAngle(m_targetPos.x-pos.x, pos.z-m_targetPos.z);
					goalAngle -= car->RetAngleY(0);

					m_cameraType = m_camera->RetType();
					m_camera->SetType(CAMERA_BACK);
					angle = NormAngle(goalAngle);
					if ( angle < PI )  angle =  PI*0.1f;
					else               angle = -PI*0.1f;
					m_camera->SetBackHoriz(angle);
					m_camera->SetLockRotate(TRUE);
					m_main->SetStopwatch(FALSE);  // stoppe le chrono

					pos = car->RetCharacter()->posFret;
					mat = car->RetWorldMatrix(0);
					pos = Transform(*mat, pos);
					m_goalAngle = RotateAngle(m_targetPos.x-pos.x, pos.z-m_targetPos.z);
					m_goalAngle -= car->RetAngleY(0);
					m_goalAngle += PI;
					m_goalAngle = NormAngle(m_goalAngle);
					if ( m_goalAngle < PI )
					{
						m_startAngle = m_object->RetAngleY(1);
					}
					else
					{
						m_startAngle = -m_object->RetAngleY(1);
						m_goalAngle -= PI*2.0f;
					}

					m_goalDist = -Length2d(pos, m_targetPos);
					pos = m_object->RetPosition(2);
					m_startDist = pos.x;

					SoundManip(1.0f, 0.8f, 1.0f);

					m_bGrab = FALSE;
					m_phase = AHKP_GOSTART;
					m_speed = 1.0f/1.0f;
					m_progress = 0.0f;
					progress = 0.0f;
				}
			}
		}
	}

	if ( m_phase == AHKP_GOSTART )
	{
		angle = m_startAngle+(m_goalAngle-m_startAngle)*progress;
		m_object->SetAngleY(1, angle);

		dist = m_startDist+(m_goalDist-m_startDist)*progress;
		m_object->SetPosition(2, D3DVECTOR(dist, 0.9f, 0.0f));

		if ( m_progress >= 1.0f )
		{
			if ( m_bGrab )  // prend ?
			{
				mat = m_object->RetWorldMatrix(3);
				pos = Transform(*mat, D3DVECTOR(0.0f, 1.2f, 0.0f));
				m_goalHeight = m_targetPos.y-pos.y-2.0f;
				m_startHeight = -2.1f;
			}
			else	// dépose ?
			{
				mat = m_object->RetWorldMatrix(3);
				pos = Transform(*mat, D3DVECTOR(0.0f, 1.2f, 0.0f));
				m_goalHeight = m_targetPos.y-pos.y+2.2f;
				m_startHeight = -2.1f;
				StartAction(MB_WALK1, 1.0f);  // cassé au sol
			}

			SoundManip(0.5f, 0.8f, 1.5f);

			m_phase = AHKP_DOWN;
			m_speed = 1.0f/1.0f;
			m_progress = 0.0f;
			progress = 0.0f;
		}
	}

	if ( m_phase == AHKP_DOWN )
	{
		height = m_startHeight+(m_goalHeight-m_startHeight)*Bounce(progress);
		m_object->SetPosition(3, D3DVECTOR(0.0f, height, 0.0f));

		if ( m_progress >= 1.0f )
		{
			car = m_object->RetTruck();

			if ( m_bGrab )  // prend ?
			{
				m_load = m_target;
				m_load->SetTruck(m_object);
				m_load->SetTruckPart(3);  // prend avec le bout du bras
				m_load->SetPosition(0, D3DVECTOR(0.0f, 1.2f, 0.0f));
				m_goalRot = PI*0.5f;
				m_startRot = m_load->RetAngleY(0)-car->RetAngleY(0)-m_object->RetAngleY(1);
				m_startRot = NormAngle(m_startRot);
				if ( m_startRot > PI )  m_startRot -= PI*2.0f;
				m_load->SetAngleY(0, m_startRot);
				StartAction(MB_WALK2, 1.0f);  // cassé transporté
			}
			else	// dépose ?
			{
				mat = m_object->RetWorldMatrix(3);
				pos = Transform(*mat, D3DVECTOR(0.0f, 1.2f, 0.0f));
				m_terrain->MoveOnFloor(pos);
				pos.y += m_load->RetCharacter()->height;
				m_load->SetTruck(0);
				m_load->SetPosition(0, pos);
				m_load->SetAngleY(0, car->RetAngleY(0)+m_object->RetAngleY(1)+m_load->RetAngleY(0));
				m_load = 0;
			}

			SoundManip(0.5f, 0.8f, 1.5f);

			m_phase = AHKP_UP;
			m_speed = 1.0f/1.0f;
			m_progress = 0.0f;
			progress = 0.0f;
		}
	}

	if ( m_phase == AHKP_UP )
	{
		height = m_startHeight+(m_goalHeight-m_startHeight)*(1.0f-Bounce(progress));
		m_object->SetPosition(3, D3DVECTOR(0.0f, height, 0.0f));

		if ( m_bGrab )  // prend ?
		{
			angle = m_startRot+(m_goalRot-m_startRot)*progress;
			m_load->SetAngleY(0, angle);
		}

		if ( m_progress >= 1.0f )
		{
			car = m_object->RetTruck();

			if ( m_bGrab )  // prend ?
			{
				m_startAngle = m_goalAngle;
				m_goalAngle = 0.0f;

				m_startDist = m_goalDist;
				m_goalDist = car->RetCharacter()->hookDist;

				m_startHeight = m_goalHeight;
				m_goalHeight = -2.1f;
			}
			else	// dépose ?
			{
				m_startAngle = m_goalAngle;
				if ( m_startAngle < 0.0f )  m_goalAngle = -PI;
				else                        m_goalAngle =  PI;

				m_startDist = m_goalDist;
				m_goalDist = -8.0f;

				m_startHeight = m_goalHeight;
				m_goalHeight = -2.1f;
			}

			SoundManip(1.0f, 0.8f, 1.0f);

			m_phase = AHKP_GOEND;
			m_speed = 1.0f/1.0f;
			m_progress = 0.0f;
			progress = 0.0f;
		}
	}

	if ( m_phase == AHKP_GOEND )
	{
		angle = m_startAngle+(m_goalAngle-m_startAngle)*progress;
		m_object->SetAngleY(1, angle);

		dist = m_startDist+(m_goalDist-m_startDist)*progress;
		m_object->SetPosition(2, D3DVECTOR(dist, 0.9f, 0.0f));

		if ( m_progress >= 1.0f )
		{
			car->SetLock(FALSE);  // voiture de nouveau mobile

			m_camera->SetType(m_cameraType);
			m_camera->SetBackHoriz(0.0f);
			m_camera->SetLockRotate(FALSE);
			m_camera->ResetLockRotate();
			m_main->SetStopwatch(TRUE);  // redémarre le chrono

			m_phase = AHKP_WAIT;
			m_speed = 1.0f/4.0f;
			m_progress = 0.0f;
			progress = 0.0f;
		}
	}

	return TRUE;
}


// Cherche un objet proche.

CObject* CAutoHook::SearchObject(ObjectType type, D3DVECTOR center,
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
		if ( pObj->RetLock() )  continue;  // déjà pris ?

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

// Démarre une action pour le robot transporté.

void CAutoHook::StartAction(int action, float delay)
{
	CMotion*	motion;

	if ( m_load == 0 )  return;

	motion = m_load->RetMotion();
	if ( motion == 0 )  return;

	motion->SetAction(action, delay);
}

// Fait entendre le son du bras manipulateur.

void CAutoHook::SoundManip(float time, float amplitude, float frequency)
{
	CObject*	car;
	int			i;

	car = m_object->RetTruck();
	if ( car == 0 )  return;

	i = m_sound->Play(SOUND_MANIP, car->RetPosition(0), 0.0f, 0.3f*frequency, TRUE);
	m_sound->AddEnvelope(i, 0.5f*amplitude, 1.0f*frequency, 0.1f, SOPER_CONTINUE);
	m_sound->AddEnvelope(i, 0.5f*amplitude, 1.0f*frequency, time-0.1f, SOPER_CONTINUE);
	m_sound->AddEnvelope(i, 0.0f, 0.3f*frequency, 0.1f, SOPER_STOP);
}


// Stoppe l'automate.

BOOL CAutoHook::Abort()
{
	return TRUE;
}


// Retourne une erreur liée à l'état de l'automate.

Error CAutoHook::RetError()
{
	return ERR_OK;
}

