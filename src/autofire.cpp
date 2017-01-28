// autofire.cpp

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
#include "autohome.h"
#include "autofire.h"





// Constructeur de l'objet.

CAutoFire::CAutoFire(CInstanceManager* iMan, CObject* object)
						 : CAuto(iMan, object)
{
	CAuto::CAuto(iMan, object);

	Init();
}

// Destructeur de l'objet.

CAutoFire::~CAutoFire()
{
	CAuto::~CAuto();
}


// Détruit l'objet.

void CAutoFire::DeleteObject(BOOL bAll)
{
	CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoFire::Init()
{
	m_time          = 0.0f;
	m_progress      = 0.0f;
	m_phase         = AFIP_WAIT;
	m_speed         = 1.0f/1.0f;
	m_progress      = 0.0f;
	m_lastParticule = 0.0f;
	m_lastPlouf     = 0.0f;
}


// Démarre l'objet.

void CAutoFire::Start(int param)
{
}


// Gestion d'un événement.

BOOL CAutoFire::EventProcess(const Event &event)
{
	CObject*	target;
	CObject*	car;
	CPhysics*	physics;
	D3DMATRIX*	mat;
	D3DVECTOR	pos, speed;
	FPOINT		dim;
	float		s, angle, dist;
	int			i, channel;

	CAuto::EventProcess(event);

	if ( m_engine->RetPause() )  return TRUE;

	car = m_object->RetTruck();
	if ( car == 0 )  return TRUE;

	m_progress += event.rTime*m_speed;

	if ( m_phase == AFIP_WAIT )
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
			target = SearchObject(OBJECT_HOME1, car->RetPosition(0), 80.0f);
			if ( target != 0 )
			{
				car->SetLock(TRUE);

				m_targetPos = target->RetPosition(0);
				pos = car->RetPosition(0);
				m_startAngleH = NormAngle(m_object->RetAngleY(1));
				m_goalAngleH = RotateAngle(m_targetPos.x-pos.x, pos.z-m_targetPos.z);
				m_goalAngleH -= car->RetAngleY(0);
				m_goalAngleH += PI*4.0f;  // +2 tour = rigolo
				m_startAngleV = 20.0f*PI/180.0f;
				m_goalAngleV = 40.0f*PI/180.0f;

				dist = Length2d(pos, m_targetPos);
				m_trackMass = 1500.0f*(3.0f-dist/40.0f);

				i = m_sound->Play(SOUND_MANIP, car->RetPosition(0), 0.0f, 0.3f, TRUE);
				m_sound->AddEnvelope(i, 0.5f, 1.0f, 0.1f, SOPER_CONTINUE);
				m_sound->AddEnvelope(i, 0.5f, 1.0f, 0.8f, SOPER_CONTINUE);
				m_sound->AddEnvelope(i, 0.0f, 0.3f, 0.1f, SOPER_STOP);

				m_cameraType = m_camera->RetType();
				m_camera->SetType(CAMERA_BACK);
				angle = NormAngle(m_goalAngleH);
				if ( angle < PI )  angle -= PI*0.15f;
				else               angle += PI*0.15f;
				m_camera->SetBackHoriz(-angle);
				m_camera->SetLockRotate(TRUE);
				m_main->SetStopwatch(FALSE);  // stoppe le chrono

				m_phase = AFIP_TURNIN;
				m_speed = 1.0f/1.0f;
				m_progress = 0.0f;
			}
		}
	}

	if ( m_phase == AFIP_WAIT    ||
		 m_phase == AFIP_WAITOUT )
	{
		angle = m_object->RetAngleY(1);
		angle += event.rTime;
		m_object->SetAngleY(1, angle);
	}

	if ( m_phase == AFIP_TURNIN )
	{
		if ( m_progress < 1.0f )
		{
			angle = m_startAngleH+(m_goalAngleH-m_startAngleH)*m_progress;
			m_object->SetAngleY(1, angle);

			angle = m_startAngleV+(m_goalAngleV-m_startAngleV)*m_progress;
			m_object->SetAngleZ(2, angle);
		}
		else
		{
			m_object->SetAngleY(1, m_goalAngleH);
			m_object->SetAngleZ(2, m_goalAngleV);

			i = m_sound->Play(SOUND_SPRINKLE, car->RetPosition(0), 0.0f, 0.3f, TRUE);
			m_sound->AddEnvelope(i, 0.5f, 1.0f, 0.1f, SOPER_CONTINUE);
			m_sound->AddEnvelope(i, 0.5f, 1.0f, 3.8f, SOPER_CONTINUE);
			m_sound->AddEnvelope(i, 0.0f, 0.3f, 0.1f, SOPER_STOP);

			m_phase = AFIP_SPRINKLE;
			m_speed = 1.0f/4.0f;
			m_progress = 0.0f;
			m_lastParticule = 0.0f;
			m_lastPlouf = 0.1f;  // pas tout de suite
			m_lastSound = 0.1f;
			m_lastBreakdown = 0.2f;
		}
	}

	if ( m_phase == AFIP_SPRINKLE )
	{
		if ( m_progress < 1.0f )
		{
			if ( m_lastParticule+m_engine->ParticuleAdapt(0.025f) <= m_progress )
			{
				m_lastParticule = m_progress;

				for ( i=0 ; i<4 ; i++ )
				{
					mat = m_object->RetWorldMatrix(2);
					pos = D3DVECTOR(2.0f, 0.0f, 0.0f);
					pos = Transform(*mat, pos);

					speed = D3DVECTOR(200.0f, 0.0f, 0.0f);
					speed.x += (Rand()-0.5f)*24.0f;
					speed.y += (Rand()-0.5f)*48.0f;
					speed.z += (Rand()-0.5f)*48.0f;
					speed = Transform(*mat, speed);
					speed -= pos;

					dim.x = 1.0f;
					dim.y = dim.x;
					channel = m_particule->CreateTrack(pos, speed, dim, PARTITRACK13,
													   2.0f, m_trackMass, 0.1f, 2.0f);
					m_particule->SetObjectFather(channel, m_object);
				}
			}

			if ( m_lastPlouf+m_engine->ParticuleAdapt(0.05f) <= m_progress )
			{
				m_lastPlouf = m_progress;

				pos = m_targetPos;
				pos.x += (Rand()-0.5f)*15.0f;
				pos.z += (Rand()-0.5f)*15.0f;
				m_terrain->MoveOnFloor(pos);
				dim.x = 5.0f+5.0f*Rand();  // hauteur
				dim.y = 3.0f+3.0f*Rand();  // diamètre
				m_particule->CreateParticule(pos, D3DVECTOR(0.0f, 0.0f, 0.0f), dim, PARTIPLOUF0, 1.4f, 0.0f);
			}

			if ( m_lastSound <= m_progress )
			{
				m_lastSound = 10.0f;  // plus jamais
				m_sound->Play(SOUND_PLOUF, m_targetPos, 1.0f);
			}

			if ( m_lastBreakdown < m_progress )
			{
				m_lastBreakdown = 10.0f;  // plus jamais
				BreakDown();
			}
		}
		else
		{
			i = m_sound->Play(SOUND_MANIP, car->RetPosition(0), 0.0f, 0.3f, TRUE);
			m_sound->AddEnvelope(i, 0.5f, 1.0f, 0.1f, SOPER_CONTINUE);
			m_sound->AddEnvelope(i, 0.5f, 1.0f, 0.8f, SOPER_CONTINUE);
			m_sound->AddEnvelope(i, 0.0f, 0.3f, 0.1f, SOPER_STOP);

			m_phase = AFIP_TURNOUT;
			m_speed = 1.0f/1.0f;
			m_progress = 0.0f;
		}
	}

	if ( m_phase == AFIP_TURNOUT )
	{
		if ( m_progress < 1.0f )
		{
			angle = m_startAngleH+(m_goalAngleH-m_startAngleH)*(1.0f-m_progress);
			m_object->SetAngleY(1, angle);

			angle = m_startAngleV+(m_goalAngleV-m_startAngleV)*(1.0f-m_progress);
			m_object->SetAngleZ(2, angle);
		}
		else
		{
			car->SetLock(FALSE);
			m_object->SetAngleY(1, m_startAngleH);
			m_object->SetAngleZ(2, m_startAngleV);

			m_camera->SetType(m_cameraType);
			m_camera->SetBackHoriz(0.0f);
			m_camera->SetLockRotate(FALSE);
			m_camera->ResetLockRotate();
			m_main->SetStopwatch(TRUE);  // redémarre le chrono

			m_phase = AFIP_WAITOUT;
			m_speed = 1.0f/4.0f;
			m_progress = 0.0f;
		}
	}

	if ( m_phase == AFIP_WAITOUT )
	{
		if ( m_progress >= 1.0f )
		{
			m_phase = AFIP_WAIT;
			m_speed = 1.0f/1.0f;
			m_progress = 0.0f;
		}
	}

	return TRUE;
}


// Cherche un objet proche.

CObject* CAutoFire::SearchObject(ObjectType type, D3DVECTOR center, float radius)
{
	CObject		*pObj, *pBest;
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
		if ( pObj->RetLock() )  continue;  // déjà stoppé ?

		if ( type != pObj->RetType() )  continue;

		pos = pObj->RetPosition(0);
		dist = Length(pos, center);

		if ( dist <= radius && dist < min )
		{
			min = dist;
			pBest = pObj;
		}
	}
	return pBest;
}


// Casse la maison.

void CAutoFire::BreakDown()
{
	CObject*	home;
	CAuto*		automat;

	home = SearchObject(OBJECT_HOME1, m_targetPos, 10.0f);
	if ( home == 0 )  return;

	automat = home->RetAuto();
	if ( automat == 0 )  return;

	automat->Start(AHOP_BREAKDOWN);
}


// Stoppe l'automate.

BOOL CAutoFire::Abort()
{
	return TRUE;
}


// Retourne une erreur liée à l'état de l'automate.

Error CAutoFire::RetError()
{
	return ERR_OK;
}

