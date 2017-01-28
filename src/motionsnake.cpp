// motionsnake.cpp

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
#include "motionsnake.h"



#define DEBUG_JUMP		FALSE		// TRUE -> ajuste les mouvements




// Constructeur de l'objet.

CMotionSnake::CMotionSnake(CInstanceManager* iMan, CObject* object)
							  : CMotion(iMan, object)
{
	CMotion::CMotion(iMan, object);

	m_level = m_water->RetLevel();

	m_phase = MSP_WAIT;
	m_speed = 1.0f/1.0f;
	m_progress = 1.0f;
}

// Destructeur de l'objet.

CMotionSnake::~CMotionSnake()
{
}


// Supprime un objet.

void CMotionSnake::DeleteObject(BOOL bAll)
{
}


// Démarre une action spéciale.

Error CMotionSnake::SetAction(int action, float time)
{
	return CMotion::SetAction(action, time);
}


// Retourne la vitesse linéaire.

float CMotionSnake::RetLinSpeed()
{
	return 5.0f;
}

// Retourne la vitesse linéaire.

float CMotionSnake::RetCirSpeed()
{
	return 0.5f*PI;
}

// Retourne la distance linéaire de freinage.

float CMotionSnake::RetLinStopLength()
{
	return 2.0f;
}


// Crée un oiseau.

BOOL CMotionSnake::Create(D3DVECTOR pos, float angle, ObjectType type)
{
	CModFile*	pModFile;
	int			rank;

	if ( m_engine->RetRestCreate() < 10 )  return FALSE;
	
	m_object->SetType(type);
	pModFile = new CModFile(m_iMan);

	// Crée l'objet principal.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEOBJECT);  // c'est un objet fixe
	m_object->SetObjectRank(0, rank);
	pModFile->ReadModel("objects\\snake.mod");
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(0, pos);
	m_object->SetAngleY(0, angle);

	pos = m_object->RetPosition(0);
	m_object->SetPosition(0, pos);  // pour afficher les ombres tout de suite

//?	m_engine->LoadAllTexture();

	delete pModFile;
	return TRUE;
}


// Gestion d'un événement.

BOOL CMotionSnake::EventProcess(const Event &event)
{
	CMotion::EventProcess(event);

	if ( event.event == EVENT_FRAME )
	{
		return EventFrame(event);
	}

	return TRUE;
}

// Gestion d'un événement.

BOOL CMotionSnake::EventFrame(const Event &event)
{
	D3DVECTOR	pos;
	FPOINT		dim;
	float		progress, angle, duration;
	int			i;

	if ( m_engine->RetPause() )  return TRUE;

	m_progress += event.rTime*m_speed;
	progress = Norm(m_progress);

	if ( m_phase == MSP_WAIT )
	{
		if ( progress >= 1.0f )
		{
			if ( !SearchPosition() )
			{
				m_object->SetPosition(0, D3DVECTOR(0.0f, 1000.0f, 0.0f));
				m_object->SetHide(-1, TRUE);

				m_phase = MSP_WAIT;
				m_speed = 1.0f/(2.0f+Rand()*10.0f);
				m_progress = 0.0f;
				return TRUE;
			}

			angle = RotateAngle(m_goalPos.x-m_startPos.x, m_startPos.z-m_goalPos.z);
			m_object->SetAngleY(0, angle);

			m_height = 7.0f;
			m_phase = MSP_JUMP;
			m_speed = 1.0f/4.0f;
			m_progress = 0.0f;
			m_bPlouf1 = FALSE;
			m_bPlouf2 = FALSE;
			return TRUE;
		}
	}

	if ( m_phase == MSP_JUMP )
	{
		if ( progress < 1.0f )
		{
			pos = m_startPos+(m_goalPos-m_startPos)*m_progress;
			pos.y = m_level-8.0f+sinf(progress*PI)*m_height;
			m_object->SetPosition(0, pos);
			m_object->SetHide(-1, FALSE);

			angle = (120.0f*PI/180.0f)-(240.0f*PI/180.0f)*progress;
			m_object->SetAngleZ(0, angle);

			if ( progress > 0.10f && !m_bPlouf1 )
			{
				m_bPlouf1 = TRUE;

				for ( i=0 ; i<2 ; i++ )
				{
					pos = m_object->RetPosition(0);
					pos.x += (Rand()-0.5f)*8.0f;
					pos.z += (Rand()-0.5f)*8.0f;
					pos.y = m_water->RetLevel()+1.0f;
					dim.x = 4.0f+Rand()*4.0f;
					dim.y = dim.x;
					duration = 2.0f+Rand()*2.0f;
					m_particule->CreateParticule(pos, D3DVECTOR(0.0f, 0.0f, 0.0f), dim, PARTIFLIC, duration, 0.0f);
				}

				m_sound->Play(SOUND_BLUP, pos, 0.4f);
			}

			if ( progress > 0.75f && !m_bPlouf2 )
			{
				m_bPlouf2 = TRUE;

				for ( i=0 ; i<5 ; i++ )
				{
					pos = m_object->RetPosition(0);
					pos.x += (Rand()-0.5f)*8.0f;
					pos.z += (Rand()-0.5f)*8.0f;
					pos.y = m_water->RetLevel()+1.0f;
					dim.x = 4.0f+Rand()*4.0f;
					dim.y = dim.x;
					duration = 2.0f+Rand()*2.0f;
					m_particule->CreateParticule(pos, D3DVECTOR(0.0f, 0.0f, 0.0f), dim, PARTIFLIC, duration, 0.0f);
				}

				m_sound->Play(SOUND_BLUP, pos, 1.0f);
			}
		}
		else
		{
			pos = D3DVECTOR(0.0f, 1000.0f, 0.0f);  // invisible !
			m_object->SetPosition(0, pos);
			m_object->SetAngleZ(0, 0.0f);
			m_object->SetHide(-1, TRUE);

#if DEBUG_JUMP
			m_phase = MSP_WAIT;
			m_speed = 1.0f/1.0f;
			m_progress = 0.0f;
#else
			m_phase = MSP_WAIT;
			m_speed = 1.0f/(2.0f+Rand()*10.0f);
			m_progress = 0.0f;
#endif
			return TRUE;
		}
	}

	return TRUE;
}

// Cherche une position pour un saut.

BOOL CMotionSnake::SearchPosition()
{
#if DEBUG_JUMP
	m_startPos = D3DVECTOR(32.0f, 0.0f, 32.0f);
	m_goalPos  = D3DVECTOR(40.0f, 0.0f, 40.0f);
	return TRUE;
#else
	float		dim;
	int			i;

	dim = m_terrain->RetDim();

	for ( i=0 ; i<100 ; i++ )
	{
		m_startPos.x = (Rand()-0.5f)*dim;
		m_startPos.z = (Rand()-0.5f)*dim;
		m_startPos.y = 0.0f;
		m_startPos = Grid(m_startPos, 8.0f);

		m_goalPos.x = m_startPos.x+(rand()%2?8.0f:-8.0f);
		m_goalPos.z = m_startPos.z+(rand()%2?8.0f:-8.0f);
		m_goalPos.y = 0.0f;

		if ( m_water->IsWaterRect(m_startPos, m_goalPos) )
		{
			if ( rand()%2 == 0 )
			{
				m_goalPos.x = m_startPos.x+(m_goalPos.x-m_startPos.x)*Rand();
			}
			else
			{
				m_goalPos.z = m_startPos.z+(m_goalPos.z-m_startPos.z)*Rand();
			}
			return TRUE;
		}
	}

	return FALSE;
#endif
}

