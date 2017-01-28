// automeca.cpp

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
#include "automeca.h"



#define SPEED		0.15f



// Constructeur de l'objet.

CAutoMeca::CAutoMeca(CInstanceManager* iMan, CObject* object)
						 : CAuto(iMan, object)
{
	CAuto::CAuto(iMan, object);

	Init();
}

// Destructeur de l'objet.

CAutoMeca::~CAutoMeca()
{
}


// Détruit l'objet.

void CAutoMeca::DeleteObject(BOOL bAll)
{
	CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoMeca::Init()
{
	m_type = m_object->RetType();
	m_rand = Rand();
	m_lastParticule = 0.0f;
	m_param = 0;
}


// Démarre l'objet.

BOOL CAutoMeca::Start(int part)
{
	return TRUE;
}


// Gestion d'un événement.

BOOL CAutoMeca::EventProcess(const Event &event)
{
	D3DMATRIX*	mat;
	D3DVECTOR	pos, speed;
	FPOINT		dim;
	float		angle, duration;
	int			i;

	CAuto::EventProcess(event);

	if ( m_engine->RetPause() )  return TRUE;

	m_time += event.rTime;

	if ( m_type == OBJECT_BARRIER40 )
	{
		m_object->SetAngleX(1, m_time*SPEED);
	}

	if ( m_type == OBJECT_BARRIER41 )
	{
		m_object->SetAngleX(1, m_time*SPEED);
		m_object->SetAngleX(2, -m_time*SPEED*(2.0f/3.0f));
	}

	if ( m_type == OBJECT_BARRIER42 )
	{
		m_object->SetAngleX(1, m_rand*5.0f+m_time*SPEED);
	}

	if ( m_type == OBJECT_BARRIER46 )  // chateau avec drapeau ?
	{
		for ( i=0 ; i<4 ; i++ )
		{
			angle = sinf((m_rand*5.0f+m_time)*1.5f+i*2.0f)*((i+2.0f)*0.1f);
			m_object->SetAngleY(1+i, angle);
		}
	}

	if ( m_type == OBJECT_BARRIER58 )  // futura étuve ?
	{
		pos.x = 0.0f;
		pos.y = 6.0f+sinf(m_rand*5.0f+m_time*0.5f)*0.6f;
		pos.z = 0.0f;
		m_object->SetPosition(1, pos);

		pos.x = sinf(m_rand*5.0f+m_time*0.2f)*0.2f;
		pos.y = sinf(m_rand*5.0f+m_time*0.3f)*2.0f;
		pos.z = sinf(m_rand*5.0f+m_time*0.4f)*0.2f;
		m_object->SetAngle(1, pos);

		if ( m_lastParticule+0.05f <= m_time )
		{
			m_lastParticule = m_time;

			pos = m_object->RetPosition(0);
			pos.y += 1.0f;
			pos.x += (Rand()-0.5f)*3.0f;
			pos.z += (Rand()-0.5f)*3.0f;
			speed.x = (Rand()-0.5f)*1.0f;
			speed.z = (Rand()-0.5f)*1.0f;
			speed.y = 5.0f;
			dim.x = (0.2f+Rand()*0.1f);
			dim.y = dim.x;
			duration = 2.0f;
			m_particule->CreateParticule(pos, speed, dim, PARTIBUBBLE, duration);
		}
	}

	if ( m_type == OBJECT_BARRIER59 )  // futura coupleur ?
	{
		if ( m_lastParticule+0.05f <= m_time )
		{
			if ( rand()%20 == 0 )
			{
				m_lastParticule = m_time+2.0f+Rand()*2.0f;
				m_param = rand()%2;
			}
			else
			{
				m_lastParticule = m_time;
			}

			mat = m_object->RetWorldMatrix(0);
			pos = Transform(*mat, D3DVECTOR(m_param?1.5f:-1.5f, 7.5f, 2.0f));
			speed = Transform(*mat, D3DVECTOR(0.0f, 0.0f, -8.0f));
			speed -= m_object->RetPosition(0);
			dim.x = (1.0f+Rand()*0.5f);
			dim.y = dim.x;
			duration = 0.5f;
			m_particule->CreateParticule(pos, speed, dim, PARTIBLITZ, duration);
		}
	}

	if ( m_type == OBJECT_BARRIER62 )  // futura canon ?
	{
		m_object->SetAngleY(1, sinf((m_rand*20.0f+m_time)*0.2f)*15.0f);
		m_object->SetAngleZ(2, sinf((m_rand*20.0f+m_time)*4.0f)*0.5f);
	}

	if ( m_type == OBJECT_BARRIER63 )  // futura antenne ?
	{
		if ( m_lastParticule+0.05f <= m_time )
		{
			if ( rand()%20 == 0 )
			{
				m_lastParticule = m_time+2.0f+Rand()*2.0f;
			}
			else
			{
				m_lastParticule = m_time;
			}

			pos = m_object->RetPosition(0);
			pos.y += 14.0f;
			pos.x += (Rand()-0.5f)*1.0f;
			pos.z += (Rand()-0.5f)*1.0f;
			speed.x = 0.0f;
			speed.z = 0.0f;
			speed.y = 10.0f+Rand()*10.0f;
			dim.x = (0.5f+Rand()*0.5f);
			dim.y = dim.x;
			duration = 0.5f;
			m_particule->CreateParticule(pos, speed, dim, PARTIBLITZb, duration);
		}
	}

	if ( m_type == OBJECT_BARRIER64 )  // futura creuset ?
	{
		if ( m_lastParticule+0.05f <= m_time )
		{
			m_lastParticule = m_time;

			pos = m_object->RetPosition(0);
			pos.y += 8.0f;
			pos.x += (Rand()-0.5f)*1.0f;
			pos.z += (Rand()-0.5f)*1.0f;
			speed.x = (Rand()-0.5f)*1.0f;
			speed.z = (Rand()-0.5f)*1.0f;
			speed.y = 5.0f+Rand()*3.0f;
			dim.x = (1.5f+Rand()*1.5f);
			dim.y = dim.x;
			duration = 2.0f;
			m_particule->CreateParticule(pos, speed, dim, PARTICRASH, duration);
		}
	}

	if ( m_type == OBJECT_BARRIER66 )  // trésor ?
	{
		if ( m_lastParticule+0.1f <= m_time )
		{
			m_lastParticule = m_time;

			mat = m_object->RetWorldMatrix(0);
			pos.x = (Rand()-0.5f)*5.0f;
			pos.y = 3.0f+Rand()*0.4f;
			pos.z = (Rand()-0.5f)*4.0f-1.0f;
			pos = Transform(*mat, pos);
			speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
			dim.x = (0.5f+Rand()*0.8f);
			dim.y = dim.x;
			duration = 0.2f+Rand()*0.5f;
			m_particule->CreateParticule(pos, speed, dim, PARTILENS2, duration);
		}
	}

	if ( m_type == OBJECT_BARRIER74 )  // labo étuve basse ?
	{
		if ( m_lastParticule+0.05f <= m_time )
		{
			m_lastParticule = m_time;

			pos = m_object->RetPosition(0);
			pos.y += 5.0f;
			pos.x += (Rand()-0.5f)*3.0f;
			pos.z += (Rand()-0.5f)*3.0f;
			speed.x = (Rand()-0.5f)*1.0f;
			speed.z = (Rand()-0.5f)*1.0f;
			speed.y = 5.0f;
			dim.x = (0.2f+Rand()*0.1f);
			dim.y = dim.x;
			duration = 1.0f;
			m_particule->CreateParticule(pos, speed, dim, PARTIBUBBLE, duration);
		}
	}

	if ( m_type == OBJECT_BARRIER75 )  // labo étuve haute ?
	{
		if ( m_lastParticule+0.05f <= m_time )
		{
			m_lastParticule = m_time;

			pos = m_object->RetPosition(0);
			pos.y += 4.0f+Rand()*5.0f;
			pos.x += (Rand()-0.5f)*4.0f;
			pos.z += (Rand()-0.5f)*4.0f;
			speed.x = 0.0f;
			speed.z = 0.0f;
			speed.y = 0.0f;
			dim.x = (1.0f+Rand()*1.0f);
			dim.y = dim.x;
			duration = 0.1f+Rand()*0.1f;
			m_particule->CreateParticule(pos, speed, dim, PARTIGLINT, duration);
		}
	}

	if ( m_type == OBJECT_BARRIER76 )  // labo coupleur ?
	{
		if ( m_lastParticule+0.05f <= m_time )
		{
			if ( rand()%20 == 0 )
			{
				m_lastParticule = m_time+2.0f+Rand()*2.0f;
				m_param = rand()%2;
			}
			else
			{
				m_lastParticule = m_time;
			}

			mat = m_object->RetWorldMatrix(0);
			pos = Transform(*mat, D3DVECTOR(0.0f, 7.0f, 1.0f));
			speed = Transform(*mat, D3DVECTOR(0.0f, 0.0f, -8.0f));
			speed -= m_object->RetPosition(0);
			dim.x = (1.0f+Rand()*0.5f);
			dim.y = dim.x;
			duration = 0.5f;
			m_particule->CreateParticule(pos, speed, dim, PARTIBLITZ, duration);
		}
	}

	if ( m_type == OBJECT_BARRIER77 )  // labo parabole ?
	{
		m_object->SetAngleY(1, (m_rand*20.0f+m_time)*0.1f);
		m_object->SetAngleX(2, -PI*0.2f-sinf((m_rand*20.0f+m_time)*1.0f)*0.3f);
	}

	return TRUE;
}


// Stoppe l'automate.

BOOL CAutoMeca::Abort()
{
	return TRUE;
}


// Retourne une erreur liée à l'état de l'automate.

Error CAutoMeca::RetError()
{
	return ERR_OK;
}

