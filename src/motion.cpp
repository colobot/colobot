// motion.cpp

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
#include "light.h"
#include "particule.h"
#include "terrain.h"
#include "water.h"
#include "object.h"
#include "camera.h"
#include "robotmain.h"
#include "mainundo.h"
#include "sound.h"
#include "cmdtoken.h"
#include "motion.h"




// Constructeur de l'objet.

CMotion::CMotion(CInstanceManager* iMan, CObject* object)
{
	m_iMan = iMan;
	m_iMan->AddInstance(CLASS_MOTION, this, 100);

	m_engine    = (CD3DEngine*)m_iMan->SearchInstance(CLASS_ENGINE);
	m_light     = (CLight*)m_iMan->SearchInstance(CLASS_LIGHT);
	m_particule = (CParticule*)m_iMan->SearchInstance(CLASS_PARTICULE);
	m_terrain   = (CTerrain*)m_iMan->SearchInstance(CLASS_TERRAIN);
	m_water     = (CWater*)m_iMan->SearchInstance(CLASS_WATER);
	m_camera    = (CCamera*)m_iMan->SearchInstance(CLASS_CAMERA);
	m_main      = (CRobotMain*)m_iMan->SearchInstance(CLASS_MAIN);
	m_undo        = (CMainUndo*)m_iMan->SearchInstance(CLASS_UNDO);
	m_sound     = (CSound*)m_iMan->SearchInstance(CLASS_SOUND);

	m_object    = object;

	m_actionType  = -1;
	m_actionSpeed = 0.0f;
	m_actionTime  = 0.0f;
	m_progress    = 0.0f;

	m_linVibration  = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_cirVibration  = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_inclinaison   = D3DVECTOR(0.0f, 0.0f, 0.0f);

	m_actionProgress = 0.0f;
	m_actionLinSpeed = 0.0f;
	m_actionCirSpeed = 0.0f;

	m_wheelType = WT_NORM;
}

// Destructeur de l'objet.

CMotion::~CMotion()
{
	m_iMan->DeleteInstance(CLASS_MOTION, this);
}

// Supprime l'objet.

void CMotion::DeleteObject(BOOL bAll)
{
}


// Crée.

BOOL CMotion::Create(D3DVECTOR pos, float angle, ObjectType type)
{
	return TRUE;
}

// Gestion d'un événement.

BOOL CMotion::EventProcess(const Event &event)
{
	D3DVECTOR	pos, dir;
	float		time;

	if ( m_engine->RetPause() )  return TRUE;

	if ( event.event != EVENT_FRAME )  return TRUE;
	
	m_progress += event.rTime*m_actionSpeed;
	if ( m_progress > 1.0f )  m_progress = 1.0f;  // (*)
	m_actionTime += event.rTime;

	pos = m_object->RetPosition(0);
	if ( pos.y < m_water->RetLevel(m_object) )  // sous l'eau ?
	{
		time = event.rTime*3.0f;  // tout est plus lent
	}
	else
	{
		time = event.rTime*10.0f;
	}

	dir = m_object->RetLinVibration();
	dir.x = Smooth(dir.x, m_linVibration.x, time);
	dir.y = Smooth(dir.y, m_linVibration.y, time);
	dir.z = Smooth(dir.z, m_linVibration.z, time);
	m_object->SetLinVibration(dir);

	dir = m_object->RetCirVibration();
	dir.x = Smooth(dir.x, m_cirVibration.x, time);
	dir.y = Smooth(dir.y, m_cirVibration.y, time);
	dir.z = Smooth(dir.z, m_cirVibration.z, time);
	m_object->SetCirVibration(dir);

	dir = m_object->RetInclinaison();
	dir.x = Smooth(dir.x, m_inclinaison.x, time);
	dir.y = Smooth(dir.y, m_inclinaison.y, time);
	dir.z = Smooth(dir.z, m_inclinaison.z, time);
	m_object->SetInclinaison(dir);

	return TRUE;
}

// (*)	Evite le bug des fourmis retournées par le thumper et dont
//		l'abdomen grossi à l'infini !


// Démarre une action.

Error CMotion::SetAction(int action, float speed)
{
	m_actionType = action;
	m_actionSpeed = speed;
	m_actionTime = 0.0f;
	m_progress = 0.0f;
	return ERR_OK;
}

// Retourne l'action en cours.

int CMotion::RetAction()
{
	return m_actionType;
}

// Retourne la progressionde l'action en cours.

float CMotion::RetActionProgress()
{
	return m_progress;
}


// Spécifie un paramètre spécial.

BOOL CMotion::SetParam(int rank, float value)
{
	return FALSE;
}

float CMotion::RetParam(int rank)
{
	return 0.0f;
}


// Donne la vibration linéaire.

void CMotion::SetLinVibration(D3DVECTOR dir)
{
	m_linVibration = dir;
}

D3DVECTOR CMotion::RetLinVibration()
{
	return m_linVibration;
}

// Donne la vibration circulaire.

void CMotion::SetCirVibration(D3DVECTOR dir)
{
	m_cirVibration = dir;
}

D3DVECTOR CMotion::RetCirVibration()
{
	return m_cirVibration;
}

// Donne l'inclinaison.

void CMotion::SetInclinaison(D3DVECTOR dir)
{
	m_inclinaison = dir;
}

D3DVECTOR CMotion::RetInclinaison()
{
	return m_inclinaison;
}


// Retourne la vitesse linéaire.

float CMotion::RetLinSpeed()
{
	return 0.0f;
}

// Retourne la vitesse circulaire.

float CMotion::RetCirSpeed()
{
	return 0.0f;
}

// Retourne la distance linéaire de freinage.

float CMotion::RetLinStopLength()
{
	return 0.0f;
}


// Spécifie la progression dans l'action.

void CMotion::SetActionProgress(float progress)
{
	m_actionProgress = progress;
}

// Spécifie la vitesse linéaire dans l'action.

void CMotion::SetActionLinSpeed(float speed)
{
	m_actionLinSpeed = speed;
}

// Spécifie la vitesse circulaire dans l'action.

void CMotion::SetActionCirSpeed(float speed)
{
	m_actionCirSpeed = speed;
}


// Gestion du type de roues.

void CMotion::SetWheelType(WheelType type)
{
	m_wheelType = type;
}

WheelType CMotion::RetWheelType()
{
	return m_wheelType;
}


// Gestion de l'état d'un véhicule.

int CMotion::RetStateLength()
{
	return 0;
}

void CMotion::GetStateBuffer(char *buffer)
{
}


// Ecrit la situation de l'objet.

void CMotion::WriteSituation()
{
}

// lit la situation de l'objet.

void CMotion::ReadSituation()
{
}

