// autotower.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "D3DEngine.h"
#include "D3DMath.h"
#include "global.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "math3d.h"
#include "particule.h"
#include "terrain.h"
#include "camera.h"
#include "object.h"
#include "physics.h"
#include "interface.h"
#include "button.h"
#include "gauge.h"
#include "window.h"
#include "sound.h"
#include "displaytext.h"
#include "cmdtoken.h"
#include "auto.h"
#include "autotower.h"




// Constructeur de l'objet.

CAutoTower::CAutoTower(CInstanceManager* iMan, CObject* object)
					   : CAuto(iMan, object)
{
	int		i;

	CAuto::CAuto(iMan, object);

	for ( i=0 ; i<4 ; i++ )
	{
		m_partiStop[i] = -1;
	}

	Init();
}

// Destructeur de l'objet.

CAutoTower::~CAutoTower()
{
	CAuto::~CAuto();
}


// Détruit l'objet.

void CAutoTower::DeleteObject(BOOL bAll)
{
	FireStopUpdate(0.0f, FALSE);
	CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoTower::Init()
{
	m_phase    = ATP_ZERO;
	m_progress = 0.0f;
	m_speed    = 1.0f/2.0f;
	m_time = 0.0f;
}


// Gestion d'un événement.

BOOL CAutoTower::EventProcess(const Event &event)
{
	float		angle;

	CAuto::EventProcess(event);

	if ( m_engine->RetPause() )  return TRUE;
	if ( event.event != EVENT_FRAME )  return TRUE;

	m_progress += event.rTime*m_speed;

	if ( m_phase == ATP_ZERO )
	{
		FireStopUpdate(m_progress, TRUE);  // clignotte
		if ( m_progress < 1.0f )
		{
			angle = m_object->RetAngleY(1);
			angle -= event.rTime*2.0f;
			m_object->SetAngleY(1, angle);

			angle = sinf(m_progress*PI*2.0f)*PI*0.25f;
			m_object->SetAngleZ(2, angle);
		}
		else
		{
			m_phase    = ATP_ZERO;
			m_progress = 0.0f;
			m_speed    = 1.0f/2.0f;
		}
	}

	return TRUE;
}


// Retourne une erreur liée à l'état de l'automate.

Error CAutoTower::RetError()
{
	return ERR_OK;
}


// Met à jour les feux de stop.

void CAutoTower::FireStopUpdate(float progress, BOOL bLightOn)
{
	D3DMATRIX*	mat;
	D3DVECTOR	pos, speed;
	FPOINT		dim;
	int			i;

	static float listpos[8] =
	{
		 9.0f,	 0.0f,
		 0.0f,	 9.0f,
		-9.0f,	 0.0f,
		 0.0f,	-9.0f,
	};

	if ( !bLightOn )  // éteint ?
	{
		for ( i=0 ; i<4 ; i++ )
		{
			if ( m_partiStop[i] != -1 )
			{
				m_particule->DeleteParticule(m_partiStop[i]);
				m_partiStop[i] = -1;
			}
		}
		return;
	}

	mat = m_object->RetWorldMatrix(0);

	speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
	dim.x = 2.0f;
	dim.y = dim.x;

	for ( i=0 ; i<4 ; i++ )
	{
		if ( Mod(progress+i*0.125f, 0.5f) < 0.2f )
		{
			if ( m_partiStop[i] != -1 )
			{
				m_particule->DeleteParticule(m_partiStop[i]);
				m_partiStop[i] = -1;
			}
		}
		else
		{
			if ( m_partiStop[i] == -1 )
			{
				pos.x = listpos[i*2+0];
				pos.y = 36.0f;
				pos.z = listpos[i*2+1];
				pos = Transform(*mat, pos);
				m_partiStop[i] = m_particule->CreateParticule(pos, speed,
															  dim, PARTISELY,
															  1.0f, 0.0f);
			}
		}
	}
}


