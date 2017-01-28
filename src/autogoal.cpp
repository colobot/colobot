// autogoal.cpp

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
#include "interface.h"
#include "button.h"
#include "window.h"
#include "robotmain.h"
#include "sound.h"
#include "auto.h"
#include "autogoal.h"





// Constructeur de l'objet.

CAutoGoal::CAutoGoal(CInstanceManager* iMan, CObject* object)
						 : CAuto(iMan, object)
{
	CAuto::CAuto(iMan, object);

	Init();
}

// Destructeur de l'objet.

CAutoGoal::~CAutoGoal()
{
}


// Détruit l'objet.

void CAutoGoal::DeleteObject(BOOL bAll)
{
	CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoGoal::Init()
{
	m_phase = AGO_FIX;
	m_progress = 0.0f;
	m_speed = 1.0f/1.0f;
	m_pos = m_object->RetPosition(0);
	m_bLastGoal = FALSE;
	m_jostle = 0.0f;
}


// Démarre l'objet.

BOOL CAutoGoal::Start(int phase)
{
	m_phase = (AutoGoalPhase)phase;
	m_progress = 0.0f;
	m_speed = 1.0f/6.0f;

	m_bLastGoal = (m_phase == AGO_LASTFLY);

	if ( m_bLastGoal )
	{
		ChangeColumn();
		CameraStart();
	}

	return TRUE;
}


// Gestion d'un événement.

BOOL CAutoGoal::EventProcess(const Event &event)
{
	CObject*	blupi;
	D3DVECTOR	pos, cGoalAngle, hGoalAngle, cBlupiAngle, hBlupiAngle;
	float		time, progress;

	CAuto::EventProcess(event);

	if ( m_engine->RetPause() )  return TRUE;

	m_progress += event.rTime*m_speed;
	progress = Norm(m_progress);
	time = event.rTime*0.1f;

	cGoalAngle = m_object->RetAngle(0);
	hGoalAngle = cGoalAngle;

	if ( m_phase == AGO_FIX )
	{
		hGoalAngle.x = sinf((m_time+m_pos.x+m_pos.z)*0.47f)*0.07f;
		hGoalAngle.z = sinf((m_time+m_pos.x+m_pos.z)*0.53f)*0.07f;
	}

	if ( m_jostle > 0.0f )
	{
		m_jostle -= event.rTime*0.5f;
		if ( m_jostle < 0.0f )  m_jostle = 0.0f;

		hGoalAngle.x = sinf((m_time+m_pos.x+m_pos.z)*2.57f)*0.60f*m_jostle;
		hGoalAngle.z = sinf((m_time+m_pos.x+m_pos.z)*2.62f)*0.60f*m_jostle;
	}

	if ( m_phase == AGO_FLY     ||
		 m_phase == AGO_LASTFLY )
	{
		blupi = m_object->RetTruck();
		if ( blupi != 0 )
		{
			// Monte un peu le ballon par-rapport à blupi.
			pos = m_object->RetPosition(0);
			pos.y += event.rTime;
			if ( pos.y > 0.0f )  pos.y = 0.0f;
			m_object->SetPosition(0, pos);

			cBlupiAngle = blupi->RetAngle(0);
			hBlupiAngle = cBlupiAngle;

			hBlupiAngle.x = sinf(m_time*5.97f)*0.15f;
			hBlupiAngle.z = sinf(m_time*5.17f)*0.15f;
			hBlupiAngle.y += event.rTime*20.0f;

			cBlupiAngle.x = SmoothA(cBlupiAngle.x, hBlupiAngle.x, time);
			cBlupiAngle.y = SmoothA(cBlupiAngle.y, hBlupiAngle.y, time);
			cBlupiAngle.z = SmoothA(cBlupiAngle.z, hBlupiAngle.z, time);
			blupi->SetAngle(0, cBlupiAngle);
		}

		hGoalAngle.x = 0.0f;
		hGoalAngle.z = 0.0f;

		if ( m_bLastGoal )
		{
			CameraProgress(progress);
		}
	}

	cGoalAngle.x = SmoothA(cGoalAngle.x, hGoalAngle.x, time);
	cGoalAngle.y = SmoothA(cGoalAngle.y, hGoalAngle.y, time);
	cGoalAngle.z = SmoothA(cGoalAngle.z, hGoalAngle.z, time);
	m_object->SetAngle(0, cGoalAngle);

	return TRUE;
}


// Stoppe l'automate.

BOOL CAutoGoal::Abort()
{
	return TRUE;
}


// Retourne une erreur liée à l'état de l'automate.

Error CAutoGoal::RetError()
{
	return ERR_OK;
}


// Début d'une action spéciale.

Error CAutoGoal::SetAction(int action, float time)
{
	if ( action == 1 )  // jostle ?
	{
		m_jostle = 1.0f;
	}
	return ERR_OK;
}


// Modifie l'objet pilier pour que la caméra puisse le voir
// depuis en haut.

void CAutoGoal::ChangeColumn()
{
	D3DVECTOR	pos;
	CObject*	pObj;
	int			i;

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( pObj->RetType() >= OBJECT_COLUMN1 &&
			 pObj->RetType() <= OBJECT_COLUMN4 )
		{
			pos = pObj->RetPosition(2);
			pos.y += 800.0f;
			pObj->SetPosition(2, pos);

			pos = pObj->RetPosition(3);
			pos.y += 800.0f;
			pObj->SetPosition(3, pos);
		}
	}
}


// Rotation d'un point de la caméra.

void RotateGoal(D3DVECTOR &pos, D3DVECTOR center, float angle)
{
	FPOINT	rot;

	rot = RotatePoint(FPOINT(center.x, center.z), angle, FPOINT(pos.x, pos.z));
	pos.x = rot.x;
	pos.z = rot.y;
}

// Début du mouvement vertical de la caméra.

void CAutoGoal::CameraStart()
{
	D3DVECTOR	eye, lookat, dir;
	float		dirH, dirV, dist;

	m_camera->RetCamera(eye, lookat, dirH, dirV, dist);
	m_eyeStart    = eye;
	m_lookatStart = lookat;

#if 1
	m_lookatGoal = m_object->RetPosition(0);
	dir = Normalize(m_eyeStart-m_lookatStart);
	m_eyeGoal = m_lookatGoal+dir*6.0f;
	m_eyeGoal.y = 500.0f;
#else
	m_eyeGoal     = m_lookatStart+D3DVECTOR(0.0f, 500.0f, -200.0f);
	m_lookatGoal  = m_lookatStart+D3DVECTOR(0.0f,   0.0f,  -20.0f);

	dirH = Grid(dirH, PI/2.0f)+30.0f*PI/180.0f;
	RotateGoal(m_eyeGoal,     m_lookatStart, dirH);
	RotateGoal(m_lookatGoal,  m_lookatStart, dirH);
#endif

	m_camera->SetType(CAMERA_SCRIPT);
	m_camera->SetScriptEye(m_eyeStart);
	m_camera->SetScriptLookat(m_lookatStart);
	m_camera->FixCamera();
	m_main->SetMovieLock(TRUE, TRUE);

	m_startView = m_engine->RetDeepView(0);
	m_goalView = 800.0f;
}

// Progression du mouvement spécial de la caméra.

void CAutoGoal::CameraProgress(float progress)
{
	D3DVECTOR	eye, lookat;
	float		intensity, view;

	eye = m_eyeStart+(m_eyeGoal-m_eyeStart)*progress;
	m_camera->SetScriptEye(eye);

	lookat = m_lookatStart+(m_lookatGoal-m_lookatStart)*progress;
	m_camera->SetScriptLookat(lookat);

	intensity = 1.0f-Norm(progress*5.0f);
	m_particule->SetSunbeamIntensity(intensity);

	view = m_startView+(m_goalView-m_startView)*progress;
	m_engine->SetDeepView(view, 0, TRUE);
	m_engine->ApplyChange();
}

