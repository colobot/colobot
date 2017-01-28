// autoscrap.cpp

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
#include "water.h"
#include "camera.h"
#include "object.h"
#include "interface.h"
#include "button.h"
#include "window.h"
#include "robotmain.h"
#include "sound.h"
#include "auto.h"
#include "autoscrap.h"





// Constructeur de l'objet.

CAutoScrap::CAutoScrap(CInstanceManager* iMan, CObject* object)
						 : CAuto(iMan, object)
{
	CAuto::CAuto(iMan, object);

	Init();
}

// Destructeur de l'objet.

CAutoScrap::~CAutoScrap()
{
}


// Détruit l'objet.

void CAutoScrap::DeleteObject(BOOL bAll)
{
	CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoScrap::Init()
{
	m_lastParticule = 0.0f;
	m_time = Rand()*10.0f;
}


// Démarre l'objet.

BOOL CAutoScrap::Start(int part)
{
	return TRUE;
}


// Gestion d'un événement.

BOOL CAutoScrap::EventProcess(const Event &event)
{
	D3DVECTOR	lin, cir, pos;
	FPOINT		dim;
	float		duration;

	CAuto::EventProcess(event);

	if ( m_engine->RetPause() )  return TRUE;

	m_time += event.rTime;

	lin = D3DVECTOR(0.0f, 0.0f, 0.0f);
	cir = D3DVECTOR(0.0f, 0.0f, 0.0f);

	lin.y += sinf(m_time*0.50f)*1.00f;
	lin.y += sinf(m_time*1.30f)*0.30f;

	cir.x += sinf(m_time*0.40f)*0.05f;
	cir.y += sinf(m_time*0.33f)*0.05f;
	cir.z += sinf(m_time*0.29f)*0.05f;

	m_object->SetLinVibration(lin);
	m_object->SetCirVibration(cir);

	if ( m_lastParticule+0.50f <= m_time )
	{
		m_lastParticule = m_time;

		pos = m_object->RetPosition(0);
		pos.x += (Rand()-0.5f)*8.0f;
		pos.z += (Rand()-0.5f)*8.0f;
		pos.y = m_water->RetLevel()+1.0f;
		dim.x = 4.0f+Rand()*4.0f;
		dim.y = dim.x;
		duration = 3.0f+Rand()*3.0f;
		m_particule->CreateParticule(pos, D3DVECTOR(0.0f, 0.0f, 0.0f), dim, PARTIFLIC, duration, 0.0f);
	}

	return TRUE;
}


// Stoppe l'automate.

BOOL CAutoScrap::Abort()
{
	return TRUE;
}


// Retourne une erreur liée à l'état de l'automate.

Error CAutoScrap::RetError()
{
	return ERR_OK;
}

