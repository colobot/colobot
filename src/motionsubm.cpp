// motionsubm.cpp

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
#include "camera.h"
#include "modfile.h"
#include "sound.h"
#include "robotmain.h"
#include "motion.h"
#include "motionsubm.h"




// Constructeur de l'objet.

CMotionSubm::CMotionSubm(CInstanceManager* iMan, CObject* object)
							  : CMotion(iMan, object)
{
	CMotion::CMotion(iMan, object);

	m_level = m_water->RetLevel();

	m_phase = MSBP_WAIT;
	m_speed = 1.0f/1.0f;
	m_progress = 1.0f;
	m_time = 0.0f;
	m_lastParticule = 0.0f;
}

// Destructeur de l'objet.

CMotionSubm::~CMotionSubm()
{
}


// Supprime un objet.

void CMotionSubm::DeleteObject(BOOL bAll)
{
}


// Démarre une action spéciale.

Error CMotionSubm::SetAction(int action, float time)
{
	return CMotion::SetAction(action, time);
}


// Retourne la vitesse linéaire.

float CMotionSubm::RetLinSpeed()
{
	return 5.0f;
}

// Retourne la vitesse linéaire.

float CMotionSubm::RetCirSpeed()
{
	return 0.5f*PI;
}

// Retourne la distance linéaire de freinage.

float CMotionSubm::RetLinStopLength()
{
	return 2.0f;
}


// Crée un oiseau.

BOOL CMotionSubm::Create(D3DVECTOR pos, float angle, ObjectType type)
{
	CModFile*	pModFile;
	int			rank;

	if ( m_engine->RetRestCreate() < 10 )  return FALSE;
	
	m_object->SetType(type);
	pModFile = new CModFile(m_iMan);

	// Crée l'objet principal.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEMETAL);  // c'est un objet fixe
	m_object->SetObjectRank(0, rank);
	pModFile->ReadModel("objects\\subm.mod");
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(0, pos);
	m_object->SetAngleY(0, angle);

	pos = m_object->RetPosition(0);
	m_object->SetPosition(0, pos);  // pour afficher les ombres tout de suite

	delete pModFile;
	return TRUE;
}


// Gestion d'un événement.

BOOL CMotionSubm::EventProcess(const Event &event)
{
	CMotion::EventProcess(event);

	if ( event.event == EVENT_FRAME )
	{
		return EventFrame(event);
	}

	return TRUE;
}

// Gestion d'un événement.

BOOL CMotionSubm::EventFrame(const Event &event)
{
	D3DVECTOR	pos;
	FPOINT		dim;
	float		progress, angle, duration;

	if ( m_engine->RetPause() )  return TRUE;

	m_time += event.rTime;

	m_progress += event.rTime*m_speed;
	progress = Norm(m_progress);

	if ( m_phase == MSBP_WAIT )
	{
		if ( progress >= 1.0f )
		{
			if ( !SearchPosition() )
			{
				m_object->SetPosition(0, D3DVECTOR(0.0f, -100.0f, 0.0f));
				m_phase = MSBP_WAIT;
				m_speed = 1.0f/(2.0f+Rand()*10.0f);
				m_progress = 0.0f;
				return TRUE;
			}

			m_angle = RotateAngle(m_goalPos.x-m_startPos.x, m_startPos.z-m_goalPos.z);
			m_object->SetAngleY(0, m_angle);

			m_phase = MSBP_UP;
			m_speed = 1.0f/2.0f;
			m_progress = 0.0f;
			return TRUE;
		}
	}

	if ( m_phase == MSBP_UP )
	{
		if ( progress < 1.0f )
		{
			pos = m_startPos;
			pos.y = m_level-10.0f+Soft(progress)*10.0f;
			m_object->SetPosition(0, pos);

			if ( progress > 0.3f && m_lastParticule+0.10f <= m_time )
			{
				m_lastParticule = m_time;
				pos = m_object->RetPosition(0);
				pos.x += (Rand()-0.5f)*4.0f;
				pos.z += (Rand()-0.5f)*4.0f;
				pos.y = m_water->RetLevel()+1.0f;
				dim.x = 3.0f+Rand()*3.0f;
				dim.y = dim.x;
				duration = 2.0f+Rand()*2.0f;
				m_particule->CreateParticule(pos, D3DVECTOR(0.0f, 0.0f, 0.0f), dim, PARTIFLIC, duration, 0.0f);
			}
		}
		else
		{
			m_phase = MSBP_LOOK;
			m_speed = 1.0f/6.0f;
			m_progress = 0.0f;
			return TRUE;
		}
	}

	if ( m_phase == MSBP_LOOK )
	{
		if ( progress < 1.0f )
		{
//?			angle = m_angle+Soft(progress)*PI*2.0f;
			angle = m_angle+sinf(Soft(progress)*PI)*PI*2.0f;
			m_object->SetAngleY(0, angle);

			if ( m_lastParticule+0.25f <= m_time )
			{
				m_time = m_lastParticule;
				pos = m_object->RetPosition(0);
				pos.x += (Rand()-0.5f)*4.0f;
				pos.z += (Rand()-0.5f)*4.0f;
				pos.y = m_water->RetLevel()+1.0f;
				dim.x = 3.0f+Rand()*3.0f;
				dim.y = dim.x;
				duration = 2.0f+Rand()*2.0f;
				m_particule->CreateParticule(pos, D3DVECTOR(0.0f, 0.0f, 0.0f), dim, PARTIFLIC, duration, 0.0f);
			}
		}
		else
		{
			m_phase = MSBP_MOVE;
			m_speed = 1.0f/12.0f;
			m_progress = 0.0f;
			return TRUE;
		}
	}

	if ( m_phase == MSBP_MOVE )
	{
		if ( progress < 1.0f )
		{
			pos = m_startPos+(m_goalPos-m_startPos)*Soft(progress);
			pos.y = m_level;
			m_object->SetPosition(0, pos);

			angle = m_angle+sinf(Soft(progress)*PI*4.0f)*(30.0f*PI/180.0f);
			m_object->SetAngleY(0, angle);

			if ( m_lastParticule+0.02f <= m_time )
			{
				m_time = m_lastParticule;
				pos = m_object->RetPosition(0);
				pos.x += (Rand()-0.5f)*2.0f;
				pos.z += (Rand()-0.5f)*2.0f;
				pos.y = m_water->RetLevel()+1.0f;
				dim.x = 2.5f+Rand()*2.5f;
				dim.y = dim.x;
				duration = 2.0f+Rand()*2.0f;
				m_particule->CreateParticule(pos, D3DVECTOR(0.0f, 0.0f, 0.0f), dim, PARTIFLIC, duration, 0.0f);
			}
		}
		else
		{
			m_phase = MSBP_DOWN;
			m_speed = 1.0f/1.5f;
			m_progress = 0.0f;
			return TRUE;
		}
	}

	if ( m_phase == MSBP_DOWN )
	{
		if ( progress < 1.0f )
		{
			pos = m_goalPos;
			pos.y = m_level-Soft(progress)*10.0f;
			m_object->SetPosition(0, pos);

			if ( progress < 0.3f && m_lastParticule+0.10f <= m_time )
			{
				m_time = m_lastParticule;
				pos = m_object->RetPosition(0);
				pos.x += (Rand()-0.5f)*4.0f;
				pos.z += (Rand()-0.5f)*4.0f;
				pos.y = m_water->RetLevel()+1.0f;
				dim.x = 3.0f+Rand()*3.0f;
				dim.y = dim.x;
				duration = 2.0f+Rand()*2.0f;
				m_particule->CreateParticule(pos, D3DVECTOR(0.0f, 0.0f, 0.0f), dim, PARTIFLIC, duration, 0.0f);
			}
		}
		else
		{
			m_phase = MSBP_WAIT;
			m_speed = 1.0f/(2.0f+Rand()*10.0f);
			m_progress = 0.0f;
			return TRUE;
		}
	}

	return TRUE;
}

// Cherche une position pour un saut.

BOOL CMotionSubm::SearchPosition()
{
	float		dim;
	int			i;

	dim = m_terrain->RetDim();

	for ( i=0 ; i<100 ; i++ )
	{
		m_startPos.x = (Rand()-0.5f)*dim;
		m_startPos.z = (Rand()-0.5f)*dim;
		m_startPos.y = 0.0f;
		m_startPos = Grid(m_startPos, 8.0f);

		m_goalPos = m_startPos;
		if ( rand()%2 == 0 )
		{
			m_goalPos.x = m_startPos.x+(rand()%2?48.0f:-48.0f);
		}
		else
		{
			m_goalPos.z = m_startPos.z+(rand()%2?48.0f:-48.0f);
		}

		if ( m_water->IsWaterRect(m_startPos, m_goalPos) )  return TRUE;
	}

	return FALSE;
}

