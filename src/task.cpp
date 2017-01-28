// task.cpp

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
#include "motion.h"
#include "camera.h"
#include "sound.h"
#include "robotmain.h"
#include "mainundo.h"
#include "displaytext.h"
#include "task.h"




// Constructeur de l'objet.

CTask::CTask(CInstanceManager* iMan, CObject* object)
{
	m_iMan = iMan;

	m_engine      = (CD3DEngine*)m_iMan->SearchInstance(CLASS_ENGINE);
	m_light       = (CLight*)m_iMan->SearchInstance(CLASS_LIGHT);
	m_particule   = (CParticule*)m_iMan->SearchInstance(CLASS_PARTICULE);
	m_terrain     = (CTerrain*)m_iMan->SearchInstance(CLASS_TERRAIN);
	m_water       = (CWater*)m_iMan->SearchInstance(CLASS_WATER);
	m_camera      = (CCamera*)m_iMan->SearchInstance(CLASS_CAMERA);
	m_main        = (CRobotMain*)m_iMan->SearchInstance(CLASS_MAIN);
	m_undo        = (CMainUndo*)m_iMan->SearchInstance(CLASS_UNDO);
	m_displayText = (CDisplayText*)m_iMan->SearchInstance(CLASS_DISPLAYTEXT);
	m_sound       = (CSound*)m_iMan->SearchInstance(CLASS_SOUND);

	m_object      = object;
	m_motion      = m_object->RetMotion();
}

// Destructeur de l'objet.

CTask::~CTask()
{
}


// Gestion d'un événement.

BOOL CTask::EventProcess(const Event &event)
{
	return TRUE;
}


// Indique si l'action est terminée.

Error CTask::IsEnded()
{
	return ERR_STOP;
}


// Termine brutalement l'action en cours.

BOOL CTask::Abort()
{
	return TRUE;
}


// Indique si l'action est annulable.

BOOL CTask::IsUndoable()
{
	return FALSE;
}

// Indique si l'action est stoppable.

BOOL CTask::IsStopable()
{
	return FALSE;
}

// Stoppe proprement l'action en cours.

BOOL CTask::Stop()
{
	return FALSE;
}


// Ecrit la situation de l'objet.

void CTask::WriteSituation()
{
}

// lit la situation de l'objet.

void CTask::ReadSituation()
{
}

