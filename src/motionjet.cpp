// motionjet.cpp

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
#include "object.h"
#include "camera.h"
#include "modfile.h"
#include "sound.h"
#include "robotmain.h"
#include "motion.h"
#include "motionjet.h"



#define FLY_HEIGHT		25.0f		// hauteur standard de vol




// Constructeur de l'objet.

CMotionJet::CMotionJet(CInstanceManager* iMan, CObject* object)
							  : CMotion(iMan, object)
{
	CMotion::CMotion(iMan, object);

	m_progress = 1.0f;
	m_channelSound = -1;
}

// Destructeur de l'objet.

CMotionJet::~CMotionJet()
{
	if ( m_channelSound != -1 )
	{
		m_sound->FlushEnvelope(m_channelSound);
		m_sound->AddEnvelope(m_channelSound, 1.0f, 1.0f, 0.1f, SOPER_STOP);
		m_channelSound = -1;
	}
}


// Supprime un objet.

void CMotionJet::DeleteObject(BOOL bAll)
{
}


// Démarre une action spéciale.

Error CMotionJet::SetAction(int action, float time)
{
	return CMotion::SetAction(action, time);
}


// Retourne la vitesse linéaire.

float CMotionJet::RetLinSpeed()
{
	return 5.0f;
}

// Retourne la vitesse linéaire.

float CMotionJet::RetCirSpeed()
{
	return 0.5f*PI;
}

// Retourne la distance linéaire de freinage.

float CMotionJet::RetLinStopLength()
{
	return 2.0f;
}


// Crée un oiseau.

BOOL CMotionJet::Create(D3DVECTOR pos, float angle, ObjectType type)
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
	pModFile->ReadModel("objects\\jet.mod");
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(0, pos);
	m_object->SetAngleY(0, angle);

	CreateShadow(5.0f, 1.0f, D3DSHADOWBIRD);

	pos = m_object->RetPosition(0);
	m_object->SetPosition(0, pos);  // pour afficher les ombres tout de suite

	m_channelSound = m_sound->Play(SOUND_JET, pos, 0.0f, 1.0f, TRUE);

	delete pModFile;
	return TRUE;
}


// Gestion d'un événement.

BOOL CMotionJet::EventProcess(const Event &event)
{
	CMotion::EventProcess(event);

	if ( event.event == EVENT_FRAME )
	{
		return EventFrame(event);
	}

	return TRUE;
}

// Gestion d'un événement.

BOOL CMotionJet::EventFrame(const Event &event)
{
	D3DVECTOR	pos, linVib, cirVib, eye;
	float		angle, r, dist, amplitude;

	if ( m_engine->RetPause() )  return TRUE;

	if ( m_progress >= 1.0f )
	{
		r = Rand();
		dist = 300.0f+r*300.0f;
		m_startPos.x = -dist;
		m_startPos.z = (Rand()-0.5f)*200.0f;
		m_startPos.y = FLY_HEIGHT;
		m_goalPos.x  = dist;
		m_goalPos.z  = (Rand()-0.5f)*200.0f;
		m_goalPos.y  = FLY_HEIGHT;

		if ( rand()%2 == 0 )
		{
			Swap(m_startPos.x, m_goalPos.x);
		}

		if ( rand()%2 == 0 )
		{
			Swap(m_startPos.x, m_startPos.z);
			Swap(m_goalPos.x,  m_goalPos.z);
		}

		angle = RotateAngle(m_goalPos.x-m_startPos.x, m_startPos.z-m_goalPos.z);
		m_object->SetAngleY(0, angle);

		m_progress = 0.0f;
		m_speed = 1.0f/(8.0f+r+8.0f);
	}
	m_progress += event.rTime*m_speed;

	pos = m_startPos+(m_goalPos-m_startPos)*m_progress;
	m_object->SetPosition(0, pos);

	if ( m_channelSound != -1 )
	{
		m_sound->Position(m_channelSound, pos);

		eye = m_engine->RetEyePt();
		dist = Length2d(pos, eye);
		amplitude = 1.0f-Norm(dist/100.0f-1.0f);
		m_sound->Amplitude(m_channelSound, amplitude);
	}

	return TRUE;
}


// Crée l'ombre circulaire sous un oiseau.

BOOL CMotionJet::CreateShadow(float radius, float intensity,
							  D3DShadowType type)
{
	int		objRank;

	objRank = m_object->RetObjectRank(0);
	m_engine->ShadowCreate(objRank);

	m_engine->SetObjectShadowRadius(objRank, radius);
	m_engine->SetObjectShadowIntensity(objRank, intensity);
	m_engine->SetObjectShadowHeight(objRank, 0.0f);
	m_engine->SetObjectShadowType(objRank, type);

	return TRUE;
}

// Déplace les ombres des oiseaux.

void CMotionJet::MoveShadow(float progress)
{
	D3DMATRIX*	mat;
	D3DVECTOR	pos;
	float		radius;
	int			objRank;

	objRank = m_object->RetObjectRank(0);

	mat = m_object->RetWorldMatrix(0);
	pos = Transform(*mat, D3DVECTOR(0.0f, 0.0f, 0.0f));
	pos.y = 0.0f;

	if ( m_terrain->ValidPosition(pos, 16.0f) )
	{
		radius = 5.0f-sinf(progress*PI)*2.0f;
		m_engine->SetObjectShadowHide(objRank, FALSE);
		m_engine->SetObjectShadowPos(objRank, pos);
		m_engine->SetObjectShadowHeight(objRank, 0.0f);
		m_engine->SetObjectShadowRadius(objRank, radius);
		m_engine->SetObjectShadowAngle(objRank, m_object->RetAngleY(0));
	}
	else
	{
		m_engine->SetObjectShadowHide(objRank, TRUE);
	}
}

