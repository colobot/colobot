// automax1x.cpp

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
#include "mainundo.h"
#include "sound.h"
#include "auto.h"
#include "automax1x.h"





// Constructeur de l'objet.

CAutoMax1x::CAutoMax1x(CInstanceManager* iMan, CObject* object)
						 : CAuto(iMan, object)
{
	CAuto::CAuto(iMan, object);

	Init();
}

// Destructeur de l'objet.

CAutoMax1x::~CAutoMax1x()
{
}


// Détruit l'objet.

void CAutoMax1x::DeleteObject(BOOL bAll)
{
	CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoMax1x::Init()
{
	m_lastParticule = 0.0f;
	m_pos = m_object->RetPosition(0);

	m_phase = AMP_WAITOBJ;
	m_progress = 0.0f;
	m_speed = 1.0f/0.2f;

	m_swingSpeed = 0.8f+Rand()*0.4f;
	m_swingTime = Rand()*5.0f;

	m_engine->SetObjectType(m_object->RetObjectRank(0), TYPETERRAIN);
	m_terrain->SetResource(m_pos, TR_BOX);
}


// Démarre l'objet.

BOOL CAutoMax1x::Start(int part)
{
	return TRUE;
}


// Gestion d'un événement.

BOOL CAutoMax1x::EventProcess(const Event &event)
{
	D3DVECTOR	pos, speed;
	FPOINT		dim;
	float		progress, angle, duration;
	LockZone	lz;
	int			i;

	CAuto::EventProcess(event);

	if ( m_engine->RetPause() )  return TRUE;

	m_progress += event.rTime*m_speed;
	progress = Norm(m_progress);

	if ( m_phase == AMP_WAITOBJ )
	{
		lz = m_terrain->RetLockZone(m_pos);
		if ( lz == LZ_MAX1X )
		{
			m_phase = AMP_WAITOBJ;
			m_progress = 0.0f;
			m_speed = 1.0f/0.2f;
			return TRUE;
		}
		else
		{
			m_sound->Play(SOUND_TUTUTU, m_pos);

			m_phase = AMP_WAITNULL;
			m_progress = 0.0f;
			m_speed = 1.0f/0.2f;
			return TRUE;
		}
	}

	if ( m_phase == AMP_WAITNULL )
	{
		if ( m_lastParticule+0.10f <= m_time )
		{
			m_lastParticule = m_time;

#if 0
			pos = m_pos;
			pos.y += 0.1f;
			speed.x = (Rand()-0.5f)*6.0f;
			speed.z = (Rand()-0.5f)*6.0f;
			speed.y = 0.0f;
			dim.x = 2.0f+Rand()*2.0f;
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTIMAX1X, 2.0f, 0.0f);
#endif

			pos.x = (Rand()-0.5f)*8.0f;
			pos.z = (rand()%2)?4.5f:-4.5f;
			if ( rand()%2 )  Swap(pos.x, pos.z);
			pos.y = 0.0f;
			pos += m_pos;
			speed.x = 0.0f;
			speed.z = 0.0f;
			speed.y = 2.0f;
			dim.x = 1.0f;
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, (ParticuleType)(PARTILENS1+rand()%4), 2.0f);
		}

		lz = m_terrain->RetLockZone(m_pos);
		if ( lz != LZ_FREE  &&
			 lz != LZ_MAX1X )
		{
			m_phase = AMP_WAITNULL;
			m_progress = 0.0f;
			m_speed = 1.0f/0.2f;
			return TRUE;
		}
		else
		{
			m_object->SetHide(0, TRUE);   // portes fermées
			m_object->SetHide(1, FALSE);  // porte gauche
			m_object->SetHide(2, FALSE);  // porte droite
			m_object->SetHide(3, FALSE);  // support 1
			m_object->SetHide(4, FALSE);  // support 2
			m_object->SetHide(5, FALSE);  // support 3
			m_object->SetHide(6, FALSE);  // clown
			m_object->SetHide(7, FALSE);  // fond

			m_engine->SetObjectType(m_object->RetObjectRank(0), TYPEOBJECT);
			m_object->CreateLockZone(0, 0, LZ_FIX);

			m_sound->Play(SOUND_CLOSE, m_pos);

			m_phase = AMP_OPEN;
			m_progress = 0.0f;
			m_speed = 1.0f/1.0f;
			return TRUE;
		}
	}

	if ( m_phase == AMP_OPEN )
	{
		SwingClown(event.rTime);

		if ( m_lastParticule+0.05f <= m_time )
		{
			m_lastParticule = m_time;

			for ( i=0 ; i<10 ; i++ )
			{
				pos = m_pos;
				pos.x += (Rand()-0.5f)*8.0f;
				pos.z += (Rand()-0.5f)*8.0f;
				pos.y -= 2.0f;
				speed.x = 0.0f;
				speed.z = 0.0f;
				speed.y = 0.0f;
				dim.x = 2.0f;
				dim.y = dim.x;
				duration = 0.5f+Rand()*3.0f;
				m_particule->CreateParticule(pos, speed, dim, PARTISMOKE1, duration);
			}
		}

		angle = progress*(75.0f*PI/180.0f);
		m_object->SetAngleZ(1,  angle);
		m_object->SetAngleZ(2, -angle);

		if ( progress >= 1.0f )
		{
			for ( i=0 ; i<100 ; i++ )
			{
				pos = m_pos;
				pos.y += 1.0f;
				speed.x = (Rand()-0.5f)*16.0f;
				speed.z = (Rand()-0.5f)*16.0f;
				speed.y = 20.0f+Rand()*15.0f;
				dim.x = 0.5f+Rand()*1.0f;
				dim.y = dim.x;
				duration = 1.0f+Rand()*1.0f;
				m_particule->CreateParticule(pos, speed, dim, (ParticuleType)(PARTILENS1+rand()%4), duration, 40.0f);
			}

			m_sound->Play(SOUND_CLOWN, m_pos);

			m_phase = AMP_UP;
			m_progress = 0.0f;
			m_speed = 1.0f/0.6f;
			return TRUE;
		}
	}

	if ( m_phase == AMP_UP )
	{
		SwingClown(event.rTime);

		pos = D3DVECTOR(0.0f, -12.5f, 0.0f);
		pos.y += Bounce(progress)*10.0f;
		m_object->SetPosition(3, pos);

		if ( progress >= 1.0f )
		{
			m_phase = AMP_TERM;
			m_progress = 0.0f;
			m_speed = 1.0f/1.0f;
			return TRUE;
		}
	}

	if ( m_phase == AMP_TERM )
	{
		SwingClown(event.rTime);
	}

	return TRUE;
}


// Stoppe l'automate.

BOOL CAutoMax1x::Abort()
{
	return TRUE;
}


// Retourne une erreur liée à l'état de l'automate.

Error CAutoMax1x::RetError()
{
	return ERR_OK;
}


// Bouge le clown.

void CAutoMax1x::SwingClown(float rTime)
{
	D3DVECTOR	angle;

	m_swingTime += rTime*m_swingSpeed;

	angle = D3DVECTOR(0.0f, 0.0f, 0.0f);

	angle.x += sinf(m_swingTime*3.3f)*0.04f;
	angle.x += sinf(m_swingTime*1.8f)*0.04f;

	angle.y += sinf(m_swingTime*6.3f)*0.04f;
	angle.y += sinf(m_swingTime*2.2f)*0.04f;

	angle.z += sinf(m_swingTime*4.7f)*0.04f;
	angle.z += sinf(m_swingTime*1.1f)*0.04f;

	m_object->SetAngle(3, angle);
	m_object->SetAngle(4, angle);
	m_object->SetAngle(5, angle);
}


// Ecrit la situation de l'objet.

void CAutoMax1x::WriteSituation()
{
	m_undo->WriteTokenInt("phase", m_phase);
}

// lit la situation de l'objet.

void CAutoMax1x::ReadSituation()
{
	int		phase;

	if ( m_undo->ReadTokenInt("phase", phase) )
	{
		m_phase = (AutoMax1xPhase)phase;
		m_progress = 0.0f;
		m_speed = 1.0f/0.2f;

		if ( phase == AMP_WAITOBJ  ||
			 phase == AMP_WAITNULL )
		{
			m_engine->SetObjectType(m_object->RetObjectRank(0), TYPETERRAIN);

			m_object->SetHide(0, FALSE);  // portes fermées
			m_object->SetHide(1, TRUE);   // porte gauche
			m_object->SetHide(2, TRUE);   // porte droite
			m_object->SetHide(3, TRUE);   // support 1
			m_object->SetHide(4, TRUE);   // support 2
			m_object->SetHide(5, TRUE);   // support 3
			m_object->SetHide(6, TRUE);   // clown
			m_object->SetHide(7, TRUE);   // fond

			m_object->SetAngleZ(1, 0.0f);
			m_object->SetAngleZ(2, 0.0f);
			m_object->SetPosition(3, D3DVECTOR(0.0f, -12.5f, 0.0f));
		}
	}
}

