// autoflash.cpp

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
#include "autoflash.h"





// Constructeur de l'objet.

CAutoFlash::CAutoFlash(CInstanceManager* iMan, CObject* object)
						 : CAuto(iMan, object)
{
	CAuto::CAuto(iMan, object);

	Init();
}

// Destructeur de l'objet.

CAutoFlash::~CAutoFlash()
{
}


// Détruit l'objet.

void CAutoFlash::DeleteObject(BOOL bAll)
{
	int		i;

	for ( i=0 ; i<10 ; i++ )
	{
		if ( m_parti[i] != -1 )
		{
			m_particule->DeleteParticule(m_parti[i]);
			m_parti[i] = -1;
		}
	}

	CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoFlash::Init()
{
	D3DMATRIX*	mat;
	D3DVECTOR	pos, speed;
	FPOINT		dim;
	int			i;

	for ( i=0 ; i<10 ; i++ )
	{
		m_parti[i] = -1;
	}

	m_type = m_object->RetType();
	m_time = Rand()*10.0f;

	mat = m_object->RetWorldMatrix(0);
	speed = D3DVECTOR(0.0f, 0.0f, 0.0f);

	if ( m_type == OBJECT_BARRIER27 )  // antenne ?
	{
		pos = Transform(*mat, D3DVECTOR(0.0f, 32.5f, 0.0f));
		dim.x = 3.0f;
		dim.y = dim.x;
		m_parti[0] = m_particule->CreateParticule(pos, speed, dim, PARTISELR, 1.0f, 0.0f);
	}

	if ( m_type == OBJECT_BARRIER28 )  // pupitre ?
	{
		pos = Transform(*mat, D3DVECTOR(1.2f, 6.6f, 1.3f));
		dim.x = 0.2f;
		dim.y = dim.x;
		m_parti[0] = m_particule->CreateParticule(pos, speed, dim, PARTISELR, 1.0f, 0.0f);

		pos = Transform(*mat, D3DVECTOR(1.8f, 3.3f, -2.2f));
		dim.x = 0.2f;
		dim.y = dim.x;
		m_parti[1] = m_particule->CreateParticule(pos, speed, dim, PARTISELR, 1.0f, 0.0f);
		pos = Transform(*mat, D3DVECTOR(1.8f, 3.1f, -2.0f));
		dim.x = 0.2f;
		dim.y = dim.x;
		m_parti[2] = m_particule->CreateParticule(pos, speed, dim, PARTISELR, 1.0f, 0.0f);

		pos = Transform(*mat, D3DVECTOR(1.8f, 3.2f, 1.8f));
		dim.x = 0.4f;
		dim.y = dim.x;
		m_parti[3] = m_particule->CreateParticule(pos, speed, dim, PARTISELY, 1.0f, 0.0f);

		pos = Transform(*mat, D3DVECTOR(1.0f, 5.6f, 1.7f));
		dim.x = 1.2f;
		dim.y = dim.x;
		m_parti[4] = m_particule->CreateParticule(pos, speed, dim, PARTISELY, 1.0f, 0.0f);

		pos = Transform(*mat, D3DVECTOR(4.0f, 2.6f, -3.6f));
		dim.x = 0.3f;
		dim.y = dim.x;
		m_parti[5] = m_particule->CreateParticule(pos, speed, dim, PARTISELR, 1.0f, 0.0f);
	}

	if ( m_type == OBJECT_BARRIER43 )  // pupitre ?
	{
		pos = Transform(*mat, D3DVECTOR(1.6f, 2.8f, -3.6f));
		dim.x = 0.6f;
		dim.y = dim.x;
		m_parti[0] = m_particule->CreateParticule(pos, speed, dim, PARTISELY, 1.0f, 0.0f);

		pos = Transform(*mat, D3DVECTOR(-3.6f, 2.8f, -3.5f));
		dim.x = 0.3f;
		dim.y = dim.x;
		m_parti[1] = m_particule->CreateParticule(pos, speed, dim, PARTISELR, 1.0f, 0.0f);
		pos = Transform(*mat, D3DVECTOR(-3.3f, 2.4f, -3.6f));
		dim.x = 0.3f;
		dim.y = dim.x;
		m_parti[2] = m_particule->CreateParticule(pos, speed, dim, PARTISELR, 1.0f, 0.0f);

		pos = Transform(*mat, D3DVECTOR(0.2f, 7.4f, -0.2f));
		dim.x = 0.5f;
		dim.y = dim.x;
		m_parti[3] = m_particule->CreateParticule(pos, speed, dim, PARTISELY, 1.0f, 0.0f);
	}
}


// Démarre l'objet.

BOOL CAutoFlash::Start(int part)
{
	return TRUE;
}


// Gestion d'un événement.

BOOL CAutoFlash::EventProcess(const Event &event)
{
	float		zoom;

	CAuto::EventProcess(event);

	if ( m_engine->RetPause() )  return TRUE;

	m_time += event.rTime;

	if ( m_type == OBJECT_BARRIER27 )  // antenne ?
	{
		if ( Mod(m_time, 2.0f) < 0.1f )  zoom = 1.0f;
		else                             zoom = 0.0f;
		m_particule->SetZoom(m_parti[0], zoom);
	}

	if ( m_type == OBJECT_BARRIER28 )  // pupitre ?
	{
		if ( Mod(m_time, 1.0f) < 0.5f )  zoom = 1.0f;
		else                             zoom = 0.0f;
		m_particule->SetZoom(m_parti[0], zoom);

		if ( Mod(m_time, 0.5f) < 0.1f )  zoom = 1.0f;
		else                             zoom = 0.0f;
		m_particule->SetZoom(m_parti[1], zoom);

		if ( Mod(m_time, 0.4f) < 0.1f )  zoom = 1.0f;
		else                             zoom = 0.0f;
		m_particule->SetZoom(m_parti[2], zoom);

		if ( Mod(m_time, 1.3f) < 1.0f )  zoom = 1.0f;
		else                             zoom = 0.0f;
		m_particule->SetZoom(m_parti[3], zoom);

		if ( Mod(m_time, 5.0f) < 0.1f )  zoom = 1.0f;
		else                             zoom = 0.0f;
		m_particule->SetZoom(m_parti[4], zoom);

		if ( Mod(m_time, 12.0f) < 8.0f )  zoom = (Rand()<0.5f)?1.0f:0.0f;
		else                              zoom = 0.0f;
		m_particule->SetZoom(m_parti[5], zoom);
	}

	if ( m_type == OBJECT_BARRIER43 )  // pupitre ?
	{
		if ( Mod(m_time, 1.0f) < 0.5f )  zoom = 1.0f;
		else                             zoom = 0.0f;
		m_particule->SetZoom(m_parti[0], zoom);

		if ( Mod(m_time, 0.5f) < 0.1f )  zoom = 1.0f;
		else                             zoom = 0.0f;
		m_particule->SetZoom(m_parti[1], zoom);

		if ( Mod(m_time, 0.4f) < 0.1f )  zoom = 1.0f;
		else                             zoom = 0.0f;
		m_particule->SetZoom(m_parti[2], zoom);

		if ( Mod(m_time, 2.4f) < 0.5f )  zoom = 1.0f;
		else                             zoom = 0.0f;
		m_particule->SetZoom(m_parti[3], zoom);
	}

	return TRUE;
}


// Stoppe l'automate.

BOOL CAutoFlash::Abort()
{
	return TRUE;
}


// Retourne une erreur liée à l'état de l'automate.

Error CAutoFlash::RetError()
{
	return ERR_OK;
}

