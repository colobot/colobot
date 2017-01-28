// autolift.cpp

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
#include "motionblupi.h"
#include "interface.h"
#include "button.h"
#include "window.h"
#include "robotmain.h"
#include "sound.h"
#include "auto.h"
#include "autolift.h"





// Constructeur de l'objet.

CAutoLift::CAutoLift(CInstanceManager* iMan, CObject* object)
						 : CAuto(iMan, object)
{
	CAuto::CAuto(iMan, object);

	m_bCamera = FALSE;
	Init();
}

// Destructeur de l'objet.

CAutoLift::~CAutoLift()
{
}


// Détruit l'objet.

void CAutoLift::DeleteObject(BOOL bAll)
{
	CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoLift::Init()
{
	m_phase = ALI_NULL;
	m_speed = 1.0f/10.0f;
	m_progress = 0.0f;
}


// Démarre l'objet.

BOOL CAutoLift::Start(int phase)
{
	D3DVECTOR	pos;

	if ( phase == 0 )  m_bCamera = TRUE;

	m_posGround = m_object->RetPosition(0);
	m_posBlupi = m_posGround;

	m_blupi = SearchObject(m_posBlupi, 2.0f);
	if ( m_blupi == 0 )
	{
		m_angleBlupi = 0.0f;
	}
	else
	{
		m_posBlupi.y += m_blupi->RetCharacter()->height;
		m_angleBlupi = m_blupi->RetAngle(0);
	}

	m_bSelect = FALSE;

	if ( m_main->RetAgain() ||  // bouton "Recommencer" utilisé ?
		 m_main->RetEdit()  ||  // édite un puzzle ?
		 m_engine->RetSetup(ST_MOVIE) == 0.0f )
	{
		Abort();
		return TRUE;
	}

	if ( m_blupi == 0 )
	{
		m_phase = ALI_NULL;
		m_speed = 1.0f/10.0f;
		m_progress = 0.0f;
	}
	else
	{
		m_totalRot = PI+Rand()*PI;

		StartAction(MBLUPI_GOAL2);
		m_blupi->SetEnable(FALSE);

		m_phase = ALI_UP1;
		m_speed = 1.0f/5.0f;
		m_progress = 0.0f;

		pos = m_posBlupi;
		pos.y += -185.0f;
		m_blupi->SetPosition(0, pos);

		pos = m_posGround;
		pos.y += -16.0f;
		m_object->SetPosition(0, pos);
		m_object->SetZoom(0, 0.0f);

		CameraStart1();
	}

	return TRUE;
}


// Gestion d'un événement.

BOOL CAutoLift::EventProcess(const Event &event)
{
	D3DVECTOR	pos, speed, angle;
	FPOINT		dim;
	float		progress, duration;
	int			i;

	CAuto::EventProcess(event);

	if ( m_engine->RetPause() )  return TRUE;

	m_progress += event.rTime*m_speed;
	progress = Norm(m_progress);

	if ( m_phase == ALI_UP1 )  // monte ?
	{
		if ( m_blupi->RetSelect() )
		{
			m_bSelect = TRUE;
			m_blupi->SetSelect(FALSE);
		}

		pos = m_posBlupi;
		pos.y += -185.0f+progress*150.0f;
		m_blupi->SetPosition(0, pos);

		angle = m_angleBlupi;
		angle.x = sinf((m_time+m_posBlupi.x*0.45f+m_posBlupi.z*0.57f)*5.97f)*0.08f;
		angle.z = sinf((m_time+m_posBlupi.x*0.45f+m_posBlupi.z*0.57f)*5.17f)*0.08f;
		angle.y += progress*m_totalRot;
		m_lastAngleBlupi = angle;
		m_blupi->SetAngle(0, angle);

		CameraProgress(progress);

		if ( progress >= 1.0f )
		{
			CameraStart2();

			m_phase = ALI_UP2;
			m_speed = 1.0f/3.0f;
			m_progress = 0.0f;
			return TRUE;
		}
	}

	if ( m_phase == ALI_UP2 )  // monte ?
	{
		pos = m_posBlupi;
		pos.y += -35.0f+progress*50.0f;
		m_blupi->SetPosition(0, pos);

		angle = m_angleBlupi;
		angle.x = sinf((m_time+m_posBlupi.x*0.45f+m_posBlupi.z*0.57f)*5.97f)*0.08f;
		angle.z = sinf((m_time+m_posBlupi.x*0.45f+m_posBlupi.z*0.57f)*5.17f)*0.08f;
		angle.y += progress*m_totalRot;
		m_lastAngleBlupi = angle;
		m_blupi->SetAngle(0, angle);

		CameraProgress(progress);

		if ( progress >= 1.0f )
		{
			CameraStop();

			m_sound->Play(SOUND_BLUPIohhh, m_posBlupi, 1.0f, 1.0f);
			StartAction(MBLUPI_WAIT);

			m_object->SetZoom(0, 1.0f);  // ascenseur visible

			pos = m_posBlupi;
			pos.y += 10.0f+4.0f;
			speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
			dim.x = 3.0f;
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTILENS2, 0.3f);

			m_phase = ALI_DOWN;
			m_speed = 1.0f/0.6f;
			m_progress = 0.0f;
			return TRUE;
		}
	}

	if ( m_phase == ALI_DOWN )  // redescend ?
	{
		pos = m_posBlupi;
		pos.y += 10.0f-powf(progress, 2.0f)*10.0f;
		m_blupi->SetPosition(0, pos);

		pos.y = 2.0f+powf(progress, 2.0f)*20.0f;
		pos.x = 0.0f;
		pos.z = 0.0f;
		m_blupi->SetPosition(15, pos);  // ballon

		angle = m_lastAngleBlupi+(m_angleBlupi-m_lastAngleBlupi)*progress;
		m_blupi->SetAngle(0, angle);

		pos = m_posGround;
		pos.y += -16.0f+progress*16.0f;
		m_object->SetPosition(0, pos);  // monte l'ascenseur
		
		if ( progress >= 1.0f )
		{
			if ( m_bCamera )
			{
				m_main->SetMovieLock(FALSE, FALSE);
			}

			m_terrain->SetResource(m_posGround, TR_BOX);

//?			m_sound->Play(SOUND_BOUMv, m_posBlupi, 1.0f, 1.0f);
			m_sound->Play(SOUND_CHOCa, m_posBlupi, 1.0f, 1.0f);
			StartAction(MBLUPI_AMAZE, 3.0f);
			m_blupi->SetEnable(TRUE);
			if ( m_bSelect )
			{
				m_main->SelectObject(m_blupi);  // sélectionne si nécessaire
			}
			m_blupi->SetPosition(0, m_posBlupi);  // pour afficher l'ombre !

			for ( i=0 ; i<10 ; i++ )
			{
				pos = m_posBlupi;
				pos.x += (Rand()-0.5f)*8.0f;
				pos.z += (Rand()-0.5f)*8.0f;
				speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
				dim.x = 6.0f;
				dim.y = dim.x;
				duration = 2.0f+Rand()*3.0f;
				m_particule->CreateParticule(pos, speed, dim, PARTICRASH, duration);
			}

			m_blupi->DeletePart(15);  // supprime le ballon

			m_phase = ALI_NULL;
			m_speed = 1.0f/10.0f;
			m_progress = 0.0f;
			return TRUE;
		}
	}

	return TRUE;
}


// Stoppe l'automate.

BOOL CAutoLift::Abort()
{
	if ( m_bCamera )
	{
		m_main->SetMovieLock(FALSE, FALSE);
	}

	m_terrain->SetResource(m_posGround, TR_BOX);

	m_object->SetZoom(0, 1.0f);  // ascenseur visible
	m_object->SetPosition(0, m_posGround);  // monte l'ascenseur

	m_blupi->SetEnable(TRUE);
	if ( m_bSelect )
	{
		m_main->SelectObject(m_blupi);  // sélectionne si nécessaire
	}
	m_blupi->SetPosition(0, m_posBlupi);
	m_blupi->SetAngle(0, m_angleBlupi);
	StartAction(MBLUPI_WAIT);

	m_blupi->DeletePart(15);  // supprime le ballon

	if ( m_bCamera )
	{
		m_main->InitEye();
	}

	m_phase = ALI_NULL;
	m_speed = 1.0f/10.0f;
	m_progress = 0.0f;

	return TRUE;
}


// Retourne une erreur liée à l'état de l'automate.

Error CAutoLift::RetError()
{
	return ERR_OK;
}


// Cherche un objet quelconque.

CObject* CAutoLift::SearchObject(D3DVECTOR center, float radius)
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

// Démarre une action pour Blupi.

void CAutoLift::StartAction(int action, float speed)
{
	CMotion*	motion;

	if ( m_blupi == 0 )  return;

	motion = m_blupi->RetMotion();
	if ( motion == 0 )  return;

	motion->SetAction(action, speed);
}


// Rotation d'un point de la caméra.

void RotateLift(D3DVECTOR &pos, D3DVECTOR center, float angle)
{
	FPOINT	rot;

	rot = RotatePoint(FPOINT(center.x, center.z), angle, FPOINT(pos.x, pos.z));
	pos.x = rot.x;
	pos.z = rot.y;
}

// Début du mouvement vertical de la caméra.

void CAutoLift::CameraStart1()
{
	D3DVECTOR	eye, lookat;
	float		dirH, dirV, dist;

	if ( !m_bCamera )  return;

	m_camera->RetCamera(eye, lookat, dirH, dirV, dist);
	m_eyeFinal    = eye;
	m_lookatFinal = lookat;
	m_dirFinal    = dirH;

	m_eyeStart    = m_lookatFinal+D3DVECTOR(0.0f,  -50.0f, -200.0f);
	m_lookatStart = m_lookatFinal+D3DVECTOR(0.0f, -200.0f,  -50.0f);
	m_eyeGoal     = m_lookatFinal+D3DVECTOR(0.0f,  100.0f, -200.0f);
	m_lookatGoal  = m_lookatFinal+D3DVECTOR(0.0f,    0.0f, -100.0f);

	RotateLift(m_eyeStart,    m_lookatFinal, m_dirFinal);
	RotateLift(m_lookatStart, m_lookatFinal, m_dirFinal);
	RotateLift(m_eyeGoal,     m_lookatFinal, m_dirFinal);
	RotateLift(m_lookatGoal,  m_lookatFinal, m_dirFinal);

	m_camera->SetType(CAMERA_SCRIPT);
	m_camera->SetScriptEye(m_eyeStart);
	m_camera->SetScriptLookat(m_lookatStart);
	m_camera->FixCamera();
	m_main->SetMovieLock(TRUE, FALSE);
}

// Début du mouvement horizontal de la caméra.

void CAutoLift::CameraStart2()
{
	float	dist;

	if ( !m_bCamera )  return;

	dist = sqrtf(FREE_DIST*FREE_DIST/2.0f);

	m_eyeStart    = m_eyeGoal;
	m_lookatStart = m_lookatGoal;
	m_eyeGoal     = m_lookatFinal+D3DVECTOR(0.0f,  dist,   -dist);
	m_lookatGoal  = m_lookatFinal+D3DVECTOR(0.0f,  0.0f,    0.0f);

	RotateLift(m_eyeGoal,     m_lookatFinal, m_dirFinal);
	RotateLift(m_lookatGoal,  m_lookatFinal, m_dirFinal);
}

// Progression du mouvement spécial de la caméra.

void CAutoLift::CameraProgress(float progress)
{
	D3DVECTOR	eye, lookat;

	if ( !m_bCamera )  return;

	eye = m_eyeStart+(m_eyeGoal-m_eyeStart)*progress;
	m_camera->SetScriptEye(eye);

	lookat = m_lookatStart+(m_lookatGoal-m_lookatStart)*progress;
	m_camera->SetScriptLookat(lookat);
}

// Fin du mouvement spécial de la caméra.

void CAutoLift::CameraStop()
{
	if ( !m_bCamera )  return;

	m_camera->SetType(CAMERA_FREE);
}

