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
#include "physics.h"
#include "brain.h"
#include "motion.h"
#include "camera.h"
#include "sound.h"
#include "robotmain.h"
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
	m_displayText = (CDisplayText*)m_iMan->SearchInstance(CLASS_DISPLAYTEXT);
	m_sound       = (CSound*)m_iMan->SearchInstance(CLASS_SOUND);

	m_object      = object;
	m_physics     = m_object->RetPhysics();
	m_brain       = m_object->RetBrain();
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


// Indique si l'action est en cours.

BOOL CTask::IsBusy()
{
	return TRUE;
}


// Termine brutalement l'action en cours.

BOOL CTask::Abort()
{
	return TRUE;
}


