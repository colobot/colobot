// auto.cpp

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
#include "light.h"
#include "terrain.h"
#include "water.h"
#include "cloud.h"
#include "planet.h"
#include "blitz.h"
#include "camera.h"
#include "object.h"
#include "modfile.h"
#include "interface.h"
#include "button.h"
#include "list.h"
#include "label.h"
#include "gauge.h"
#include "window.h"
#include "robotmain.h"
#include "mainundo.h"
#include "sound.h"
#include "displaytext.h"
#include "cmdtoken.h"
#include "auto.h"




// Constructeur de l'objet.

CAuto::CAuto(CInstanceManager* iMan, CObject* object)
{
	m_iMan = iMan;
	m_iMan->AddInstance(CLASS_AUTO, this, 100);

	m_object      = object;
	m_event       = (CEvent*)m_iMan->SearchInstance(CLASS_EVENT);
	m_engine      = (CD3DEngine*)m_iMan->SearchInstance(CLASS_ENGINE);
	m_particule   = (CParticule*)m_iMan->SearchInstance(CLASS_PARTICULE);
	m_light       = (CLight*)m_iMan->SearchInstance(CLASS_LIGHT);
	m_terrain     = (CTerrain*)m_iMan->SearchInstance(CLASS_TERRAIN);
	m_water       = (CWater*)m_iMan->SearchInstance(CLASS_WATER);
	m_cloud       = (CCloud*)m_iMan->SearchInstance(CLASS_CLOUD);
	m_planet      = (CPlanet*)m_iMan->SearchInstance(CLASS_PLANET);
	m_blitz       = (CBlitz*)m_iMan->SearchInstance(CLASS_BLITZ);
	m_camera      = (CCamera*)m_iMan->SearchInstance(CLASS_CAMERA);
	m_interface   = (CInterface*)m_iMan->SearchInstance(CLASS_INTERFACE);
	m_main        = (CRobotMain*)m_iMan->SearchInstance(CLASS_MAIN);
	m_undo        = (CMainUndo*)m_iMan->SearchInstance(CLASS_UNDO);
	m_displayText = (CDisplayText*)m_iMan->SearchInstance(CLASS_DISPLAYTEXT);
	m_sound       = (CSound*)m_iMan->SearchInstance(CLASS_SOUND);

	m_type = m_object->RetType();
	m_time = 0.0f;
	m_lastUpdateTime = 0.0f;
	m_bMotor = FALSE;
	m_progressTime = 0.0f;
	m_progressTotal = 1.0f;
	m_actionType = -1;
	m_actionTime = 0.0f;
	m_actionProgress = 0.0f;

	Init();
}

// Destructeur de l'objet.

CAuto::~CAuto()
{
	m_iMan->DeleteInstance(CLASS_AUTO, this);
}


// Détruit l'objet.

void CAuto::DeleteObject(BOOL bAll)
{
}


// Initialise l'objet.

void CAuto::Init()
{
	m_bBusy = FALSE;
}

// Démarre l'objet.

BOOL CAuto::Start(int param)
{
	return FALSE;
}


// Donne un type.

BOOL CAuto::SetType(ObjectType type)
{
	return FALSE;
}

// Donne une valeur.

BOOL CAuto::SetValue(int rank, float value)
{
	return FALSE;
}

// Donne la string.

BOOL CAuto::SetString(char *string)
{
	return FALSE;
}


// Gestion d'un événement.

BOOL CAuto::EventProcess(const Event &event)
{
	if ( event.event == EVENT_FRAME &&
		 !m_engine->RetPause() )
	{
		m_time += event.rTime;

		m_actionProgress += event.rTime*m_actionTime;
		if ( m_actionProgress > 1.0f )  m_actionProgress = 1.0f;

		UpdateInterface(event.rTime);
	}

	if ( !m_object->RetSelect() )  // robot pas sélectionné ?
	{
		return TRUE;
	}

	return TRUE;
}

// Indique si l'automate a terminé son activité.

Error CAuto::IsEnded()
{
	return ERR_CONTINUE;
}

// Stoppe l'automate.

BOOL CAuto::Abort()
{
	return FALSE;
}

// Indique si l'objet effectue une action.

BOOL CAuto::IsRunning()
{
	return FALSE;
}


// Démarre une action.

Error CAuto::SetAction(int action, float time)
{
	m_actionType = action;
	m_actionTime = 1.0f/time;
	m_actionProgress = 0.0f;
	return ERR_OK;
}

// Retourne l'action en cours.

int CAuto::RetAction()
{
	return m_actionType;
}


// Met à jour l'état de tous les boutons de l'interface,
// suite au temps qui s'écoule ...

void CAuto::UpdateInterface(float rTime)
{
	CWindow*	pw;

	if ( m_time < m_lastUpdateTime+0.1f )  return;
	m_lastUpdateTime = m_time;

	if ( !m_object->RetSelect() )  return;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW0);
	if ( pw == 0 )  return;
}


// Retourne une erreur liée à l'état de l'automate.

Error CAuto::RetError()
{
	return ERR_OK;
}


// Gestion de l'occupation.

BOOL CAuto::RetBusy()
{
	return m_bBusy;
}

void CAuto::SetBusy(BOOL bBusy)
{
	m_bBusy = bBusy;
}

void CAuto::InitProgressTotal(float total)
{
	m_progressTime = 0.0f;
	m_progressTotal = total;
}

void CAuto::EventProgress(float rTime)
{
	m_progressTime += rTime/m_progressTotal;
}


// Gestion du moteur.

BOOL CAuto::RetMotor()
{
	return m_bMotor;
}

void CAuto::SetMotor(BOOL bMotor)
{
	m_bMotor = bMotor;
}


// Ecrit la situation de l'objet.

void CAuto::WriteSituation()
{
}

// lit la situation de l'objet.

void CAuto::ReadSituation()
{
}

