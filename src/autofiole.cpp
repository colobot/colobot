// autofiole.cpp

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
#include "autofiole.h"





// Constructeur de l'objet.

CAutoFiole::CAutoFiole(CInstanceManager* iMan, CObject* object)
						 : CAuto(iMan, object)
{
	CAuto::CAuto(iMan, object);

	Init();
}

// Destructeur de l'objet.

CAutoFiole::~CAutoFiole()
{
}


// Détruit l'objet.

void CAutoFiole::DeleteObject(BOOL bAll)
{
	CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoFiole::Init()
{
	m_lastParticule = 0.0f;

	m_pos = m_object->RetPosition(0);
	m_pos.y += 2.0f;

	m_type = m_object->RetType();
}


// Démarre l'objet.

BOOL CAutoFiole::Start(int part)
{
	return TRUE;
}


// Gestion d'un événement.

BOOL CAutoFiole::EventProcess(const Event &event)
{
	D3DVECTOR	pos, speed;
	FPOINT		dim;

	CAuto::EventProcess(event);

	if ( m_engine->RetPause() )  return TRUE;

	if ( m_lastParticule+0.10f <= m_time )
	{
		m_lastParticule = m_time;

		if ( m_type == OBJECT_FIOLE )
		{
			pos = m_pos;
			if ( m_object->RetTruck() == 0 )
			{
				speed.x = (Rand()-0.5f)*4.0f;
				speed.z = (Rand()-0.5f)*4.0f;
				speed.y = 10.0f+Rand()*10.0f;
				dim.x = 0.5f;
				dim.y = 0.5f;
			}
			else
			{
				speed.x = (Rand()-0.5f)*8.0f;
				speed.z = (Rand()-0.5f)*8.0f;
				speed.y = 10.0f+Rand()*10.0f;
				dim.x = 1.0f;
				dim.y = 1.0f;
			}
			m_particule->CreateParticule(pos, speed, dim, PARTILENS4, 1.0f, 40.0f);
		}
		else
		{
			pos = m_pos;
			pos.y += 1.0f;
			if ( m_object->RetTruck() == 0 )
			{
				speed.x = (Rand()-0.5f)*4.0f;
				speed.z = (Rand()-0.5f)*4.0f;
				speed.y = 5.0f+Rand()*5.0f;
				dim.x = 1.0f;
				dim.y = 1.0f;
			}
			else
			{
				speed.x = (Rand()-0.5f)*8.0f;
				speed.z = (Rand()-0.5f)*8.0f;
				speed.y = 5.0f+Rand()*5.0f;
				dim.x = 1.5f;
				dim.y = 1.5f;
			}
			m_particule->CreateParticule(pos, speed, dim, PARTIEXPLOO, 1.5f);
		}
	}

	return TRUE;
}


// Stoppe l'automate.

BOOL CAutoFiole::Abort()
{
	return TRUE;
}


// Retourne une erreur liée à l'état de l'automate.

Error CAutoFiole::RetError()
{
	return ERR_OK;
}


