// autobarrel.cpp

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
#include "sound.h"
#include "auto.h"
#include "autobarrel.h"



// Constructeur de l'objet.

CAutoBarrel::CAutoBarrel(CInstanceManager* iMan, CObject* object)
						 : CAuto(iMan, object)
{
	CAuto::CAuto(iMan, object);

	Init();
}

// Destructeur de l'objet.

CAutoBarrel::~CAutoBarrel()
{
	CAuto::~CAuto();
}


// Détruit l'objet.

void CAutoBarrel::DeleteObject(BOOL bAll)
{
	CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoBarrel::Init()
{
	m_time = 0.0f;

	m_progress = 0.0f;
	m_speed    = 1.0f/0.05f;

	m_type = m_object->RetType();
	m_force = Rand();
}


// Démarre l'objet.

void CAutoBarrel::Start(int param)
{
}


// Gestion d'un événement.

BOOL CAutoBarrel::EventProcess(const Event &event)
{
	D3DMATRIX*	mat;
	D3DVECTOR	lookat, pos, speed;
	FPOINT		dim;
	float		dist, duration;

	CAuto::EventProcess(event);

	if ( m_engine->RetPause() )  return TRUE;

	m_progress += event.rTime*m_speed;

	if ( m_progress >= 1.0f )
	{
		m_progress = 0.0f;
		m_speed    = 1.0f/0.05f;

		lookat = m_engine->RetLookatPt();
		pos = m_object->RetPosition(0);
		dist = Length2d(lookat, pos);
		if ( dist < 400.0f )
		{
			mat = m_object->RetWorldMatrix(0);

			if ( m_type == OBJECT_BARRELa )
			{
				pos.x = (Rand()-0.5f)*2.0f;
				pos.z = (Rand()-0.5f)*2.0f;
				pos.y = 4.5f;
				pos = Transform(*mat, pos);
				speed.x = (Rand()-0.5f)*1.5f;
				speed.z = (Rand()-0.5f)*1.5f;
				speed.y = 2.0f+Rand()*2.0f;
				dim.x = 0.4f+Rand()*0.8f;
				dim.y = dim.x;
				duration = 1.0f+Rand()*2.0f;
				m_particule->CreateParticule(pos, speed, dim, PARTIEJECT, duration);
			}
			else if ( m_type == OBJECT_BARREL )
			{
				if ( m_force > 0.3f )
				{
					pos.x = (Rand()-0.5f)*1.0f;
					pos.z = (Rand()-0.5f)*1.0f;
					pos.y = 4.5f;
					pos = Transform(*mat, pos);
					speed.x = (Rand()-0.5f)*1.0f*m_force;
					speed.z = (Rand()-0.5f)*1.0f*m_force;
					speed.y = (1.0f+Rand()*2.0f)*m_force;
					dim.x = (0.8f+Rand()*1.6f)*m_force;
					dim.y = dim.x;
					duration = 1.0f+Rand()*2.0f;
					m_particule->CreateParticule(pos, speed, dim, PARTIFLAME, duration);

					pos.x = (Rand()-0.5f)*2.0f;
					pos.z = (Rand()-0.5f)*2.0f;
					pos.y = 4.5f;
					pos = Transform(*mat, pos);
					speed.x = (Rand()-0.5f)*1.0f*m_force;
					speed.z = (Rand()-0.5f)*1.0f*m_force;
					speed.y = (4.0f+Rand()*5.0f)*m_force;
					dim.x = (1.2f+Rand()*2.2f)*m_force;
					dim.y = dim.x;
					duration = 2.0f+Rand()*4.0f;
					m_particule->CreateParticule(pos, speed, dim, PARTISMOKE3, duration);
				}
			}
			else if ( m_type == OBJECT_CARCASS1 )
			{
				pos.x = 3.0f+(Rand()-0.5f)*1.0f;
				pos.z = 0.0f+(Rand()-0.5f)*2.0f;
				pos.y = 1.5f;
				pos = Transform(*mat, pos);
				speed.x = (Rand()-0.5f)*1.0f;
				speed.z = (Rand()-0.5f)*1.0f;
				speed.y = (1.0f+Rand()*2.0f);
				dim.x = (1.2f+Rand()*2.4f);
				dim.y = dim.x;
				duration = 1.0f+Rand()*2.0f;
				m_particule->CreateParticule(pos, speed, dim, PARTIFLAME, duration);

				pos.x = 3.0f+(Rand()-0.5f)*1.0f;
				pos.z = 0.0f+(Rand()-0.5f)*1.0f;
				pos.y = 1.5f;
				pos = Transform(*mat, pos);
				speed.x = (Rand()-0.5f)*1.0f;
				speed.z = (Rand()-0.5f)*1.0f;
				speed.y = (4.0f+Rand()*5.0f);
				dim.x = (1.8f+Rand()*3.3f);
				dim.y = dim.x;
				duration = 2.0f+Rand()*4.0f;
				m_particule->CreateParticule(pos, speed, dim, PARTISMOKE3, duration);
			}
			else if ( m_type == OBJECT_CARCASS2 )
			{
				pos.x = 3.0f+(Rand()-0.5f)*1.0f;
				pos.z = 1.0f+(Rand()-0.5f)*2.0f;
				pos.y = 1.5f;
				pos = Transform(*mat, pos);
				speed.x = (Rand()-0.5f)*1.0f;
				speed.z = (Rand()-0.5f)*1.0f;
				speed.y = (1.0f+Rand()*2.0f);
				dim.x = (1.2f+Rand()*2.4f);
				dim.y = dim.x;
				duration = 1.0f+Rand()*2.0f;
				m_particule->CreateParticule(pos, speed, dim, PARTIFLAME, duration);

				pos.x = 3.0f+(Rand()-0.5f)*1.0f;
				pos.z = 1.0f+(Rand()-0.5f)*1.0f;
				pos.y = 1.5f;
				pos = Transform(*mat, pos);
				speed.x = (Rand()-0.5f)*1.0f;
				speed.z = (Rand()-0.5f)*1.0f;
				speed.y = (4.0f+Rand()*5.0f);
				dim.x = (1.8f+Rand()*3.3f);
				dim.y = dim.x;
				duration = 2.0f+Rand()*4.0f;
				m_particule->CreateParticule(pos, speed, dim, PARTISMOKE3, duration);
			}
		}
	}

	return TRUE;
}

// Stoppe l'automate.

BOOL CAutoBarrel::Abort()
{
	return TRUE;
}


// Retourne une erreur liée à l'état de l'automate.

Error CAutoBarrel::RetError()
{
	return ERR_OK;
}


