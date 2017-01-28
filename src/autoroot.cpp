// autoroot.cpp

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
#include "auto.h"
#include "autoroot.h"




// Constructeur de l'objet.

CAutoRoot::CAutoRoot(CInstanceManager* iMan, CObject* object)
					 : CAuto(iMan, object)
{
	CAuto::CAuto(iMan, object);

	Init();
}

// Destructeur de l'objet.

CAutoRoot::~CAutoRoot()
{
	CAuto::~CAuto();
}


// Détruit l'objet.

void CAutoRoot::DeleteObject(BOOL bAll)
{
	CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoRoot::Init()
{
	D3DMATRIX*	mat;
	D3DVECTOR	pos, speed;
	FPOINT		dim;

	m_time = 0.0f;
	m_lastParticule = 0.0f;

	mat = m_object->RetWorldMatrix(0);
	pos = D3DVECTOR(-5.0f, 28.0f, -4.0f);  // position pointe
	pos = Transform(*mat, pos);
	m_center = pos;

	speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
	dim.x = 100.0f;
	dim.y = dim.x;
	m_particule->CreateParticule(m_center, speed, dim, PARTISPHERE5, 0.5f, 0.0f);

	m_terrain->AddFlyingLimit(pos, 100.0f, 80.0f, pos.y-60.0f);
}


// Gestion d'un événement.

BOOL CAutoRoot::EventProcess(const Event &event)
{
	D3DVECTOR	pos, speed;
	FPOINT		dim;

	CAuto::EventProcess(event);

	if ( m_engine->RetPause() )  return TRUE;
	if ( event.event != EVENT_FRAME )  return TRUE;

	m_object->SetZoomX(1, 1.0f+sinf(m_time*2.0f)*0.2f);
	m_object->SetZoomY(1, 1.0f+sinf(m_time*2.3f)*0.2f);
	m_object->SetZoomZ(1, 1.0f+sinf(m_time*2.7f)*0.2f);

	if ( m_lastParticule+m_engine->ParticuleAdapt(0.10f) <= m_time )
	{
		m_lastParticule = m_time;

		pos = m_center;
		pos.x += (Rand()-0.5f)*8.0f;
		pos.z += (Rand()-0.5f)*8.0f;
		pos.y += 0.0f;
		speed.x = (Rand()-0.5f)*12.0f;
		speed.z = (Rand()-0.5f)*12.0f;
		speed.y = Rand()*12.0f;
		dim.x = Rand()*6.0f+4.0f;
		dim.y = dim.x;
		m_particule->CreateParticule(pos, speed, dim, PARTIROOT, 1.0f, 0.0f);
	}

	return TRUE;
}


// Retourne une erreur liée à l'état de l'automate.

Error CAutoRoot::RetError()
{
	return ERR_OK;
}


