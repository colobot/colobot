// motionbot.cpp

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
#include "physics.h"
#include "brain.h"
#include "camera.h"
#include "modfile.h"
#include "sound.h"
#include "robotmain.h"
#include "motion.h"
#include "motionbot.h"



#define ADJUST_ANGLE	FALSE		// TRUE -> ajuste les angles des membres

#define xx	0
#define yy	1
#define zz	2



// Constructeur de l'objet.

CMotionBot::CMotionBot(CInstanceManager* iMan, CObject* object)
							  : CMotion(iMan, object)
{
	CMotion::CMotion(iMan, object);
	m_option = 0;
	m_face = 0;
	m_aTime = Rand()*10.0f;
	m_lastParticule = 0.0f;
	m_lastSound = 0.0f;
	m_walkTime = 0.0f;
	m_starterTime = 0.0f;
	m_starterPhase = 0;
	m_partiGuide = -1;
	m_cirVib = D3DVECTOR(0.0f, 0.0f, 0.0f);
	m_bBreak = FALSE;
}

// Destructeur de l'objet.

CMotionBot::~CMotionBot()
{
	if ( m_partiGuide != -1 )
	{
		m_particule->DeleteParticule(m_partiGuide);
		m_partiGuide = -1;
	}
}


// Supprime un objet.

void CMotionBot::DeleteObject(BOOL bAll)
{
}


// Démarre une action spéciale.

Error CMotionBot::SetAction(int action, float time)
{
	ObjectType	type;
	D3DMATRIX*	mat;
	D3DVECTOR	pos;
	FPOINT		dim;
	Sound		sound;

	type = m_object->RetType();

	if ( action == MB_FEAR && RetAction() != MB_FEAR )
	{
		sound = SOUND_CLICK;
		if ( type == OBJECT_BOT1   )  sound = SOUND_BOT1p;
		if ( type == OBJECT_BOT2   )  sound = SOUND_BOT2p;
		if ( type == OBJECT_BOT3   )  sound = SOUND_BOT3p;
		if ( type == OBJECT_BOT4   )  sound = SOUND_BOT4p;
		if ( type == OBJECT_WALKER )  sound = SOUND_BOT4p;
		if ( type == OBJECT_CRAZY  )  sound = SOUND_BOT4p;
		if ( sound != SOUND_CLICK )
		{
			mat = m_object->RetWorldMatrix(0);
			pos = Transform(*mat, D3DVECTOR(0.0f, 0.0f, 0.0f));
			m_sound->Play(sound, pos);
		}
	}

	if ( (action == MB_WAIT   && RetAction() != MB_WAIT  ) ||
		 (action == MB_TRUCK  && RetAction() != MB_TRUCK ) ||
		 (action == MB_GOHOME && RetAction() != MB_GOHOME) ||
		 (action == MB_HOME1  && RetAction() != MB_HOME1 ) ||
		 (action == MB_HOME2  && RetAction() != MB_HOME2 ) )
	{
		sound = SOUND_CLICK;
		if ( type == OBJECT_BOT1   )  sound = SOUND_BOT1c;
		if ( type == OBJECT_BOT2   )  sound = SOUND_BOT2c;
		if ( type == OBJECT_BOT3   )  sound = SOUND_BOT3c;
		if ( type == OBJECT_BOT4   )  sound = SOUND_BOT4c;
		if ( type == OBJECT_WALKER )  sound = SOUND_BOT4c;
		if ( type == OBJECT_CRAZY  )  sound = SOUND_BOT4c;
		if ( sound != SOUND_CLICK )
		{
			mat = m_object->RetWorldMatrix(0);
			pos = Transform(*mat, D3DVECTOR(0.0f, 0.0f, 0.0f));
			m_sound->Play(sound, pos);
		}
	}

	if ( type == OBJECT_BOT1 )
	{
		if ( action == MB_BREAK )
		{
			m_bBreak = TRUE;
			action = MB_WALK1;  // cassé
		}
		if ( action == MB_REPAIR )
		{
			m_bBreak = FALSE;
			return ERR_OK;
		}
	}

	if ( type == OBJECT_BOT3 )  // blupi ?
	{
		if ( action == MB_HAPPY ||
			 action == MB_ANGRY )
		{
			UpdateFaceMapping(action-MB_HAPPY);
			return ERR_OK;
		}
		if ( action == MB_WALK1 )  // oreilles cassées ?
		{
			UpdateFaceMapping(1);
		}
	}

	if ( type == OBJECT_BOT4 )
	{
		if ( action == MB_WAIT && m_object->RetTruck() != 0 )
		{
			action = MB_HOME2;  // transporté dans véhicule
		}
	}

	if ( type == OBJECT_WALKER )
	{
		if ( action == MB_WAIT && m_object->RetTruck() != 0 )
		{
			action = MB_HOME2;  // transporté dans véhicule
		}
	}

	if ( type == OBJECT_GUIDE )
	{
		if ( action == MB_FLIC )
		{
			if ( m_partiGuide == -1 )
			{
				dim.x = 1.5f;
				dim.y = dim.x;
				m_partiGuide = m_particule->CreateParticule(D3DVECTOR(0.0f, 0.0f, 0.0f),
															D3DVECTOR(0.0f, 0.0f, 0.0f),
															dim, PARTISELY,
															1.0f, 0.0f);
			}
		}
		else
		{
			if ( m_partiGuide != -1 )
			{
				m_particule->DeleteParticule(m_partiGuide);
				m_partiGuide = -1;
			}
		}
	}

	return CMotion::SetAction(action, time);
}


// Crée un robot quelconque posé sur le sol.

BOOL CMotionBot::Create(D3DVECTOR pos, float angle, ObjectType type, BOOL bPlumb)
{
	CModFile*	pModFile;
	float		radius;
	int			rank, option;

	if ( m_engine->RetRestCreate() < 10 )  return FALSE;

	m_object->SetType(type);
	m_option = m_object->RetOption();
	CreatePhysics();
	pModFile = new CModFile(m_iMan);

	// Crée la base principale.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEFIX);  // c'est un objet fixe
	m_object->SetObjectRank(0, rank);
	if ( type == OBJECT_BOT1    )  pModFile->ReadModel("objects\\bot101.mod");
	if ( type == OBJECT_BOT2    )  pModFile->ReadModel("objects\\bot201.mod");
	if ( type == OBJECT_BOT3    )  pModFile->ReadModel("objects\\bot301.mod");
	if ( type == OBJECT_BOT4    )  pModFile->ReadModel("objects\\bot401.mod");
	if ( type == OBJECT_CARROT  )  pModFile->ReadModel("objects\\bot101.mod");
	if ( type == OBJECT_STARTER )  pModFile->ReadModel("objects\\bot101.mod");
	if ( type == OBJECT_WALKER  )  pModFile->ReadModel("objects\\bot401.mod");
	if ( type == OBJECT_CRAZY   )  pModFile->ReadModel("objects\\bot401.mod");
	if ( type == OBJECT_GUIDE   )  pModFile->ReadModel("objects\\bot101.mod");
	if ( type == OBJECT_EVIL1   )  pModFile->ReadModel("objects\\evil101.mod");
	if ( type == OBJECT_EVIL2   )  pModFile->ReadModel("objects\\evil101.mod");
	if ( type == OBJECT_EVIL3   )  pModFile->ReadModel("objects\\evil301.mod");
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(0, pos);
	m_object->SetAngleY(0, angle);

	radius = 2.0f;

	if ( type == OBJECT_BOT1    ||
		 type == OBJECT_CARROT  ||
		 type == OBJECT_STARTER ||
		 type == OBJECT_GUIDE   )
	{
		// Crée la jambe unique.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(1, rank);
		m_object->SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\bot102.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(1, D3DVECTOR(0.0f, -2.3f, 0.0f));

		// Crée le corps.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(2, rank);
		m_object->SetObjectParent(2, 1);
		if ( type == OBJECT_BOT1 )
		{
			pModFile->ReadModel("objects\\bot103.mod");
		}
		else if ( type == OBJECT_GUIDE )
		{
			pModFile->ReadModel("objects\\bot103g.mod");
		}
		else
		{
			pModFile->ReadModel("objects\\bot103b.mod");
		}
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(2, D3DVECTOR(0.0f, 1.4f, 0.0f));

		// Crée la tête.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(3, rank);
		m_object->SetObjectParent(3, 2);
		pModFile->ReadModel("objects\\bot104.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(3, D3DVECTOR(0.0f, 2.2f, 0.0f));

		// Crée l'antenne droite.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(4, rank);
		m_object->SetObjectParent(4, 3);
		pModFile->ReadModel("objects\\bot105.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(4, D3DVECTOR(-0.2f, 1.4f, -1.4f));

		// Crée l'antenne gauche.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(5, rank);
		m_object->SetObjectParent(5, 3);
		pModFile->ReadModel("objects\\bot105.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(5, D3DVECTOR(-0.2f, 1.4f, 1.4f));

		// Crée le bras droite.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(6, rank);
		m_object->SetObjectParent(6, 2);
		pModFile->ReadModel("objects\\bot106.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(6, D3DVECTOR(0.0f, 1.6f, -1.1f));

		// Crée l'avant bras droite.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(7, rank);
		m_object->SetObjectParent(7, 6);
		pModFile->ReadModel("objects\\bot107.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(7, D3DVECTOR(0.0f, 0.0f, -1.9f));

		// Crée le bras gauche.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(8, rank);
		m_object->SetObjectParent(8, 2);
		pModFile->ReadModel("objects\\bot106.mod");
		pModFile->Mirror();
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(8, D3DVECTOR(0.0f, 1.6f, 1.1f));

		// Crée l'avant bras gauche.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(9, rank);
		m_object->SetObjectParent(9, 8);
		if ( type == OBJECT_GUIDE )
		{
			pModFile->ReadModel("objects\\bot107g.mod");
		}
		else
		{
			pModFile->ReadModel("objects\\bot107.mod");
		}
		pModFile->Mirror();
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(9, D3DVECTOR(0.0f, 0.0f, 1.9f));
	}

	if ( type == OBJECT_BOT2 )
	{
		// Crée la tête.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(1, rank);
		m_object->SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\bot202.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(1, D3DVECTOR(0.0f, 2.7f, 0.0f));

		// Crée la jambe droite.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(2, rank);
		m_object->SetObjectParent(2, 0);
		pModFile->ReadModel("objects\\bot203.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(2, D3DVECTOR(0.0f, 0.0f, -0.25f));

		// Crée le pied droite.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(3, rank);
		m_object->SetObjectParent(3, 2);
		pModFile->ReadModel("objects\\bot204.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(3, D3DVECTOR(0.0f, -1.0f, 0.0f));

		// Crée la jambe gauche.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(4, rank);
		m_object->SetObjectParent(4, 0);
		pModFile->ReadModel("objects\\bot203.mod");
		pModFile->Mirror();
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(4, D3DVECTOR(0.0f, 0.0f, 0.25f));

		// Crée le pied gauche.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(5, rank);
		m_object->SetObjectParent(5, 4);
		pModFile->ReadModel("objects\\bot204.mod");
		pModFile->Mirror();
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(5, D3DVECTOR(0.0f, -1.0f, 0.0f));

		// Crée le bras droite.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(6, rank);
		m_object->SetObjectParent(6, 0);
		pModFile->ReadModel("objects\\bot205.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(6, D3DVECTOR(0.0f, 1.8f, -1.15f));

		// Crée la main droite.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(7, rank);
		m_object->SetObjectParent(7, 6);
		pModFile->ReadModel("objects\\bot206.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(7, D3DVECTOR(0.0f, -1.25f, 0.0f));

		// Crée le bras gauche.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(8, rank);
		m_object->SetObjectParent(8, 0);
		pModFile->ReadModel("objects\\bot205.mod");
		pModFile->Mirror();
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(8, D3DVECTOR(0.0f, 1.6f, 1.15f));

		// Crée la main gauche.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(9, rank);
		m_object->SetObjectParent(9, 8);
		pModFile->ReadModel("objects\\bot206.mod");
		pModFile->Mirror();
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(9, D3DVECTOR(0.0f, -1.25f, 0.0f));
	}

	if ( type == OBJECT_BOT3 )
	{
		// Crée la jambe droite.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(1, rank);
		m_object->SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\bot302.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(1, D3DVECTOR(-0.5f, -0.3f, -0.9f));

		// Crée la jambe droite.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(2, rank);
		m_object->SetObjectParent(2, 1);
		pModFile->ReadModel("objects\\bot303.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(2, D3DVECTOR(0.0f, -0.4f, 0.0f));

		// Crée le pied droite.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(3, rank);
		m_object->SetObjectParent(3, 2);
		pModFile->ReadModel("objects\\bot304.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(3, D3DVECTOR(0.0f, -0.4f, 0.0f));

		// Crée la jambe gauche.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(4, rank);
		m_object->SetObjectParent(4, 0);
		pModFile->ReadModel("objects\\bot302.mod");
		pModFile->Mirror();
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(4, D3DVECTOR(-0.5f, -0.3f, 0.9f));

		// Crée la jambe gauche.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(5, rank);
		m_object->SetObjectParent(5, 4);
		pModFile->ReadModel("objects\\bot303.mod");
		pModFile->Mirror();
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(5, D3DVECTOR(0.0f, -0.4f, 0.0f));

		// Crée le pied gauche.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(6, rank);
		m_object->SetObjectParent(6, 5);
		pModFile->ReadModel("objects\\bot304.mod");
		pModFile->Mirror();
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(6, D3DVECTOR(0.0f, -0.4f, 0.0f));

		// Crée le bras droite.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(7, rank);
		m_object->SetObjectParent(7, 0);
		pModFile->ReadModel("objects\\bot305.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(7, D3DVECTOR(-0.1f, 2.3f, -1.3f));

		// Crée le bras droite.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(8, rank);
		m_object->SetObjectParent(8, 7);
		pModFile->ReadModel("objects\\bot306.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(8, D3DVECTOR(0.0f, 0.0f, -1.45f));

		// Crée la main droite.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(9, rank);
		m_object->SetObjectParent(9, 8);
		pModFile->ReadModel("objects\\bot307.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(9, D3DVECTOR(0.0f, 0.0f, -1.3f));
		m_object->SetZoom(9, 1.3f);

		// Crée le bras gauche.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(10, rank);
		m_object->SetObjectParent(10, 0);
		pModFile->ReadModel("objects\\bot305.mod");
		pModFile->Mirror();
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(10, D3DVECTOR(-0.1f, 2.3f, 1.3f));

		// Crée le bras gauche.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(11, rank);
		m_object->SetObjectParent(11, 10);
		pModFile->ReadModel("objects\\bot306.mod");
		pModFile->Mirror();
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(11, D3DVECTOR(0.0f, 0.0f, 1.45f));

		// Crée la main gauche.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(12, rank);
		m_object->SetObjectParent(12, 11);
		pModFile->ReadModel("objects\\bot307.mod");
		pModFile->Mirror();
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(12, D3DVECTOR(0.0f, 0.0f, 1.3f));
		m_object->SetZoom(12, 1.3f);

		// Crée le sourcil droite.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(13, rank);
		m_object->SetObjectParent(13, 0);
		pModFile->ReadModel("objects\\bot308.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(13, D3DVECTOR(0.3f, 3.8f, -0.6f));

		// Crée le sourcil gauche.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(14, rank);
		m_object->SetObjectParent(14, 0);
		pModFile->ReadModel("objects\\bot308.mod");
		pModFile->Mirror();
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(14, D3DVECTOR(0.3f, 3.8f, 0.6f));
	}

	if ( type == OBJECT_BOT4   ||
		 type == OBJECT_WALKER ||
		 type == OBJECT_CRAZY  )
	{
		// Crée le coup.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(1, rank);
		m_object->SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\bot402.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(1, D3DVECTOR(0.0f, 2.0f, 0.0f));

		// Crée la tête.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(2, rank);
		m_object->SetObjectParent(2, 1);
		pModFile->ReadModel("objects\\bot403.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(2, D3DVECTOR(0.0f, 0.5f, 0.0f));

		// Crée la jambe droite.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(3, rank);
		m_object->SetObjectParent(3, 0);
		pModFile->ReadModel("objects\\bot404.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(3, D3DVECTOR(0.0f, -0.4f, -0.7f));

		// Crée la jambe droite.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(4, rank);
		m_object->SetObjectParent(4, 3);
		pModFile->ReadModel("objects\\bot405.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(4, D3DVECTOR(0.0f, -1.5f, 0.0f));

		// Crée le pied droite.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(5, rank);
		m_object->SetObjectParent(5, 4);
		pModFile->ReadModel("objects\\bot406.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(5, D3DVECTOR(0.0f, -1.6f, 0.0f));

		// Crée la jambe gauche.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(6, rank);
		m_object->SetObjectParent(6, 0);
		pModFile->ReadModel("objects\\bot404.mod");
		pModFile->Mirror();
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(6, D3DVECTOR(0.0f, -0.4f, 0.7f));

		// Crée la jambe gauche.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(7, rank);
		m_object->SetObjectParent(7, 6);
		pModFile->ReadModel("objects\\bot405.mod");
		pModFile->Mirror();
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(7, D3DVECTOR(0.0f, -1.5f, 0.0f));

		// Crée le pied gauche.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(8, rank);
		m_object->SetObjectParent(8, 7);
		pModFile->ReadModel("objects\\bot406.mod");
		pModFile->Mirror();
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(8, D3DVECTOR(0.0f, -1.6f, 0.0f));

		// Crée le bras droite.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(9, rank);
		m_object->SetObjectParent(9, 0);
		pModFile->ReadModel("objects\\bot407.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(9, D3DVECTOR(0.0f, 1.8f, -0.7f));

		// Crée le bras droite.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(10, rank);
		m_object->SetObjectParent(10, 9);
		pModFile->ReadModel("objects\\bot408.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(10, D3DVECTOR(0.0f, 0.0f, -1.5f));

		// Crée la main droite sup.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(11, rank);
		m_object->SetObjectParent(11, 10);
		pModFile->ReadModel("objects\\bot409.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(11, D3DVECTOR(0.0f, 0.0f, -1.5f));

		// Crée la main droite inf.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(12, rank);
		m_object->SetObjectParent(12, 10);
		pModFile->ReadModel("objects\\bot410.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(12, D3DVECTOR(0.0f, 0.0f, -1.5f));

		// Crée le bras gauche.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(13, rank);
		m_object->SetObjectParent(13, 0);
		pModFile->ReadModel("objects\\bot407.mod");
		pModFile->Mirror();
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(13, D3DVECTOR(0.0f, 1.8f, 0.7f));

		// Crée le bras gauche.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(14, rank);
		m_object->SetObjectParent(14, 13);
		pModFile->ReadModel("objects\\bot408.mod");
		pModFile->Mirror();
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(14, D3DVECTOR(0.0f, 0.0f, 1.5f));

		// Crée la main gauche sup.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(15, rank);
		m_object->SetObjectParent(15, 14);
		pModFile->ReadModel("objects\\bot409.mod");
		pModFile->Mirror();
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(15, D3DVECTOR(0.0f, 0.0f, 1.5f));

		// Crée la main gauche inf.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(16, rank);
		m_object->SetObjectParent(16, 14);
		pModFile->ReadModel("objects\\bot410.mod");
		pModFile->Mirror();
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(16, D3DVECTOR(0.0f, 0.0f, 1.5f));
	}

	if ( type == OBJECT_EVIL1 )
	{
		// Crée la trompe.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(1, rank);
		m_object->SetObjectParent(1, 0);
		pModFile->ReadModel("objects\\evil102.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(1, D3DVECTOR(0.7f, 5.6f, 0.0f));
		m_object->SetAngleZ(1, 120.0f*PI/180.0f);
		m_object->SetZoom(1, 0.2f);
	}

	if ( type == OBJECT_EVIL3 )  // roi ?
	{
		// Crée la couronne.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(1, rank);
		m_object->SetObjectParent(1, 0);
//?		pModFile->ReadModel("objects\\evil302.mod");
		pModFile->ReadModel("objects\\crown.mod");
		pModFile->CreateEngineObject(rank);
//?		m_object->SetPosition(1, D3DVECTOR(0.0f, 35.0f, 0.0f));
		m_object->SetPosition(1, D3DVECTOR(0.0f, 31.0f, 0.0f));

		radius = 10.0f;
	}

	m_object->CreateCrashSphere(D3DVECTOR(0.0f, 2.0f, 0.0f), 2.0f, SOUND_BOUMm, 0.45f);
	m_object->SetGlobalSphere(D3DVECTOR(0.0f, 2.0f, 0.0f), 2.0f);
	m_object->CreateShadowCircle(radius, 1.0f);
	m_object->SetFloorHeight(0.0f);

	pos = m_object->RetPosition(0);
	m_object->SetPosition(0, pos);  // pour afficher les ombres tout de suite

	m_engine->LoadAllTexture();

	option = m_object->RetOption();
	if ( option == 0 )
	{
		SetAction(MB_WAIT);
	}
	else
	{
		SetAction(option);
	}

	delete pModFile;
	return TRUE;
}


// Crée la physique de l'objet.

void CMotionBot::CreatePhysics()
{
	Character*	character;
	ObjectType	type;
	int			i;

	character = m_object->RetCharacter();
	type = m_object->RetType();

	if ( type == OBJECT_BOT1 )
	{
		short member[] =
		{
							// MB_WAIT:
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0,   0,  // antenne droite
			  0,   0,   0,  // antenne gauche
			-15, -20,   0,  // bras droite
			-35,-120,   0,  // avant-bras droite
			 15,  20,   0,  // bras gauche
			 35, 120,   0,  // avant-bras gauche
							// MB_TRUCK:
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			-65,   0,   0,  // antenne droite
			 65,   0,   0,  // antenne gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // avant-bras droite
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // avant-bras gauche
							// MB_FEAR:
			  0,   0,   0,  // socle
			  0,   0,  15,  // jambe
			  0,   0, -30,  // corps
			  0,   0, -20,  // tête
			  0,   0, -40,  // antenne droite
			  0,   0, -40,  // antenne gauche
			 20, -20, -15,  // bras droite
			 80, -90, -30,  // avant-bras droite
			-20,  20, -15,  // bras gauche
			-80,  90, -30,  // avant-bras gauche
							// MB_GOHOME:
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0,   0,  // antenne droite
			  0,   0,   0,  // antenne gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // avant-bras droite
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // avant-bras gauche
							// MB_HOME1:
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0,   0,  // antenne droite
			  0,   0,   0,  // antenne gauche
			-15, -20,   0,  // bras droite
			-35,-120,   0,  // avant-bras droite
			 15,  20,   0,  // bras gauche
			 35, 120,   0,  // avant-bras gauche
							// MB_HOME2:
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0,   0,  // antenne droite
			  0,   0,   0,  // antenne gauche
			  5, -20,   0,  // bras droite
			-20,-110, -75,  // avant-bras droite
			 -5,  20,   0,  // bras gauche
			 20, 110, -75,  // avant-bras gauche
							// MB_FLIC:
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0, -30,  // antenne droite
			  0,   0, -30,  // antenne gauche
			 35, -20,   0,  // bras droite
			 55,  15, -10,  // avant-bras droite
			 15,  20,   0,  // bras gauche
			 55, 170,   5,  // avant-bras gauche
							// MB_WALK1:  (cassé au sol)
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,  20,   0,  // tête
			-30,   0, -10,  // antenne droite
			 30,   0,  40,  // antenne gauche
			-15,  10,   0,  // bras droite
			-70,  20,   0,  // avant-bras droite
			 15, -10,   0,  // bras gauche
			 70, -20,   0,  // avant-bras gauche
							// MB_WALK2:  (cassé transporté)
			  0,   0,   0,  // socle
			  0,   0,  20,  // jambe
			  0,   0,  20,  // corps
			  0,   0,  30,  // tête
			  0,   0,  30,  // antenne droite
			  0,   0,  30,  // antenne gauche
			  0,  70,   0,  // bras droite
			  0,  20,   0,  // avant-bras droite
			  0, -70,   0,  // bras gauche
			  0, -20,   0,  // avant-bras gauche
							// MB_WALK3:
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0,   0,  // antenne droite
			  0,   0,   0,  // antenne gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // avant-bras droite
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // avant-bras gauche
							// MB_WALK4:
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0,   0,  // antenne droite
			  0,   0,   0,  // antenne gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // avant-bras droite
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // avant-bras gauche
		};

		for ( i=0 ; i<3*10*MB_MAX ; i++ )
		{
			m_armAngles[i] = member[i];
		}

		character->height = 3.5f;
	}

	if ( type == OBJECT_BOT2 )
	{
		short member[] =
		{
							// MB_WAIT:
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // main droite
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche
							// MB_TRUCK:
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // main droite
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche
							// MB_FEAR:
			  0,   0,   0,  // corps
			  0,   0,   5,  // tête
			  0,   0,  40,  // jambe droite
			  0,   0, -20,  // pied droite
			  0,   0,  40,  // jambe gauche
			  0,   0, -20,  // pied gauche
			  0,   0,  45,  // bras droite
			  0,   0, -25,  // main droite
			  0,   0,  45,  // bras gauche
			  0,   0, -25,  // main gauche
							// MB_GOHOME:
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // main droite
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche
							// MB_HOME1:
			  0,   0,   0,  // corps
			  0,   0,   5,  // tête
			  0,   0,  40,  // jambe droite
			  0,   0, -20,  // pied droite
			  0,   0,  40,  // jambe gauche
			  0,   0, -20,  // pied gauche
			  0,   0,  45,  // bras droite
			  0,   0, -25,  // main droite
			  0,   0,  45,  // bras gauche
			  0,   0, -25,  // main gauche
							// MB_HOME2: (tir)
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0, -55,  // jambe droite
			  0,   0,  40,  // pied droite
			  0,   0, -55,  // jambe gauche
			  0,   0,  40,  // pied gauche
			  0,   0,  30,  // bras droite
			  0,   0, -15,  // main droite
			  0,   0,  30,  // bras gauche
			  0,   0, -15,  // main gauche
							// MB_FLIC:
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // main droite
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche
							// MB_WALK1:
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // main droite
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche
							// MB_WALK2:
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // main droite
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche
							// MB_WALK3:
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // main droite
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche
							// MB_WALK4:
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // main droite
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche
		};

		for ( i=0 ; i<3*10*MB_MAX ; i++ )
		{
			m_armAngles[i] = member[i];
		}
	}

	if ( type == OBJECT_BOT3 )
	{
		short member[] =
		{
							// MB_WAIT:
			  0,   0,   0,  // corps
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
			-40,   0,   0,  // bras droite
			-90,   0,   0,  // bras droite
			 10,   0,   0,  // main droite
			 40,   0,   0,  // bras gauche
			 90,   0,   0,  // bras gauche
			-10,   0,   0,  // main gauche
			  0,   0,   0,  // sourcil droite
			  0,   0,   0,  // sourcil gauche
							// MB_TRUCK:
			  0,   0,   0,  // corps
			 20,  20,  15,  // jambe droite
			  0,   0, -20,  // jambe droite
			  0,  -5, -10,  // pied droite
			-20, -20, -15,  // jambe gauche
			  0,   0,  20,  // jambe gauche
			  0,   5,  10,  // pied gauche
			-50,  35,   0,  // bras droite
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // main droite
			 50, -35,   0,  // bras gauche
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche
			  0,   0,   0,  // sourcil droite
			  0,   0,   0,  // sourcil gauche
							// MB_FEAR:
			  0,   0,   0,  // corps
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
//?			-50,  35,   0,  // bras droite
//?			-35,-105,   0,  // bras droite
//?			  0,   0,   0,  // main droite
//?			 50, -35,   0,  // bras gauche
//?			 35, 105,   0,  // bras gauche
//?			  0,   0,   0,  // main gauche
			 40, -35,   0,  // bras droite
			120,  15, -55,  // bras droite
			  0,   0,   0,  // main droite
			-40,  35,   0,  // bras gauche
		   -120, -15, -55,  // bras gauche
			  0,   0,   0,  // main gauche
			  0,   0,   0,  // sourcil droite
			  0,   0,   0,  // sourcil gauche
							// MB_GOHOME:
			  0,   0,   0,  // corps
			 20,  20,  15,  // jambe droite
			  0,   0, -20,  // jambe droite
			  0,  -5, -10,  // pied droite
			-20, -20, -15,  // jambe gauche
			  0,   0,  20,  // jambe gauche
			  0,   5,  10,  // pied gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // main droite
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche
			  0,   0,   0,  // sourcil droite
			  0,   0,   0,  // sourcil gauche
							// MB_HOME1:
			  0,   0,   0,  // corps
			 20,  20,  15,  // jambe droite
			  0,   0, -20,  // jambe droite
			  0,  -5, -10,  // pied droite
			-20, -20, -15,  // jambe gauche
			  0,   0,  20,  // jambe gauche
			  0,   5,  10,  // pied gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // main droite
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche
			  0,   0,   0,  // sourcil droite
			  0,   0,   0,  // sourcil gauche
							// MB_HOME2:
			  0,   0,   0,  // corps
			 10,  10,  85,  // jambe droite
			  0,   0,   0,  // jambe droite
			  0,  10, -30,  // pied droite
			-10, -10,  85,  // jambe gauche
			  0,   0,   0,  // jambe gauche
			  0, -10, -30,  // pied gauche
			-55,  50,   0,  // bras droite
			  0,   0,   0,  // bras droite
			 15,   0,  10,  // main droite
			 55, -50,   0,  // bras gauche
			  0,   0,   0,  // bras gauche
			-15,   0,  10,  // main gauche
			  0,   0,   0,  // sourcil droite
			  0,   0,   0,  // sourcil gauche
							// MB_FLIC:
			  0,   0,   0,  // corps
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
			 65,   5, -15,  // bras droite
			 25,   0,   0,  // bras droite
			  0,   0,   0,  // main droite
			 50, -35,   0,  // bras gauche
			 35, 105,   0,  // bras gauche
			  0,   0,   0,  // main gauche
			  0,   0,   0,  // sourcil droite
			  0,   0,   0,  // sourcil gauche
							// MB_WALK1: (oreilles cassées)
			  0,   0,   0,  // corps
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
			 40, -35,   0,  // bras droite
			120,  15, -55,  // bras droite
			  0,   0,   0,  // main droite
			-40,  35,   0,  // bras gauche
		   -120, -15, -55,  // bras gauche
			  0,   0,   0,  // main gauche
			  0,   0,   0,  // sourcil droite
			  0,   0,   0,  // sourcil gauche
							// MB_WALK2:
			  0,   0,   0,  // corps
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // main droite
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche
			  0,   0,   0,  // sourcil droite
			  0,   0,   0,  // sourcil gauche
							// MB_WALK3:
			  0,   0,   0,  // corps
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // main droite
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche
			  0,   0,   0,  // sourcil droite
			  0,   0,   0,  // sourcil gauche
							// MB_WALK4:
			  0,   0,   0,  // corps
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // main droite
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche
			  0,   0,   0,  // sourcil droite
			  0,   0,   0,  // sourcil gauche
		};

		for ( i=0 ; i<3*15*MB_MAX ; i++ )
		{
			m_armAngles[i] = member[i];
		}

		character->height = 2.0f;
	}

	if ( type == OBJECT_BOT4 )
	{
		short member[] =
		{
							// MB_WAIT:
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0,  -5,  // tête
			  0,   0,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  20,  // pied droite
			  0,   0,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  20,  // pied gauche
			-15,  20,   0,  // bras droite (bras écartés)
			-90, -30,  30,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 15, -20,   0,  // bras gauche
			 90,  30,  30,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_TRUCK:
			  0,   0,   0,  // corps
			  0,   0,   0,  // coup
			  0,   0,   0,  // tête
			  0,  10,  50,  // jambe droite
			  0,   0,-100,  // jambe droite
			  0,   0,  35,  // pied droite
			  0, -10,  50,  // jambe gauche
			  0,   0,-100,  // jambe gauche
			  0,   0,  35,  // pied gauche
			 15,  20,   0,  // bras droite
			-90, -30,  30,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			-15, -20,   0,  // bras gauche
			 90,  30,  30,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_FEAR:
			  0,   0,   0,  // corps
			  0,   0,  15,  // coup
			  0,   0,  10,  // tête
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
			 20,  30,   0,  // bras droite
			-55,-175, -55,  // bras droite
			 60,   0,   0,  // main droite sup
			-60,   0,   0,  // main droite inf
			-20, -30,   0,  // bras gauche
			 55, 175, -55,  // bras gauche
			-60,   0,   0,  // main gauche sup
			 60,   0,   0,  // main gauche inf
							// MB_GOHOME:
			  0,   0,   0,  // corps
			  0,   0,   0,  // coup
			  0,   0,   0,  // tête
			  0,  10,  50,  // jambe droite
			  0,   0,-100,  // jambe droite
			  0,   0,  35,  // pied droite
			  0, -10,  50,  // jambe gauche
			  0,   0,-100,  // jambe gauche
			  0,   0,  35,  // pied gauche
			-25, -65,   0,  // bras droite
			-25,-115,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 30,  65,   0,  // bras gauche
			-10, 115,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_HOME1:
			  0,   0,   0,  // corps
			  0,   0,   0,  // coup
			  0,   0,   0,  // tête
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_HOME2 (dans véhicule):
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0,  -5,  // tête
			  0,   0,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  20,  // pied droite
			  0,   0,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  20,  // pied gauche
			-30, -55,   0,  // bras droite (bras croisés)
			-25, -90,  55,  // bras droite
			 10, -40,   0,  // main droite sup
			-10, -40,   0,  // main droite inf
			 20,  65,   0,  // bras gauche
			  0, 120,-110,  // bras gauche
			-10,  40,   0,  // main gauche sup
			 10,  40,   0,  // main gauche inf
							// MB_FLIC:
			  0,   0,   0,  // corps
			  0,   0,   0,  // coup
			  0,   0,   0,  // tête
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_WALK1:
			  0,   0,   0,  // corps
			  0,   0,   0,  // coup
			  0,   0,   0,  // tête
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_WALK2:
			  0,   0,   0,  // corps
			  0,   0,   0,  // coup
			  0,   0,   0,  // tête
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_WALK3:
			  0,   0,   0,  // corps
			  0,   0,   0,  // coup
			  0,   0,   0,  // tête
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_WALK4:
			  0,   0,   0,  // corps
			  0,   0,   0,  // coup
			  0,   0,   0,  // tête
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
		};

		for ( i=0 ; i<3*17*MB_MAX ; i++ )
		{
			m_armAngles[i] = member[i];
		}

		character->height = 3.8f;
	}

	if ( type == OBJECT_CARROT )
	{
		short member[] =
		{
							// MB_WAIT:
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0,   0,  // antenne droite
			  0,   0,   0,  // antenne gauche
			-15, -20,   0,  // bras droite
			-35,-120,   0,  // avant-bras droite
			 15,  20,   0,  // bras gauche
			 35, 120,   0,  // avant-bras gauche
							// MB_TRUCK:
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			-65,   0,   0,  // antenne droite
			 65,   0,   0,  // antenne gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // avant-bras droite
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // avant-bras gauche
							// MB_FEAR:
			  0,   0,   0,  // socle
			  0,   0,  15,  // jambe
			  0,   0, -30,  // corps
			  0,   0, -20,  // tête
			  0,   0, -40,  // antenne droite
			  0,   0, -40,  // antenne gauche
			 20, -20, -15,  // bras droite
			 80, -90, -30,  // avant-bras droite
			-20,  20, -15,  // bras gauche
			-80,  90, -30,  // avant-bras gauche
							// MB_GOHOME:
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0,   0,  // antenne droite
			  0,   0,   0,  // antenne gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // avant-bras droite
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // avant-bras gauche
							// MB_HOME1:
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0,   0,  // antenne droite
			  0,   0,   0,  // antenne gauche
			-15, -20,   0,  // bras droite
			-35,-120,   0,  // avant-bras droite
			 15,  20,   0,  // bras gauche
			 35, 120,   0,  // avant-bras gauche
							// MB_HOME2:
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0,   0,  // antenne droite
			  0,   0,   0,  // antenne gauche
			  5, -20,   0,  // bras droite
			-20,-110, -75,  // avant-bras droite
			 -5,  20,   0,  // bras gauche
			 20, 110, -75,  // avant-bras gauche
							// MB_FLIC:
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0, -30,  // antenne droite
			  0,   0, -30,  // antenne gauche
			 35, -20,   0,  // bras droite
			 55,  15, -10,  // avant-bras droite
			 15,  20,   0,  // bras gauche
			 55, 170,   5,  // avant-bras gauche
							// MB_WALK1:
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0,   0,  // antenne droite
			  0,   0,   0,  // antenne gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // avant-bras droite
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // avant-bras gauche
							// MB_WALK2:
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0,   0,  // antenne droite
			  0,   0,   0,  // antenne gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // avant-bras droite
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // avant-bras gauche
							// MB_WALK3:
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0,   0,  // antenne droite
			  0,   0,   0,  // antenne gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // avant-bras droite
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // avant-bras gauche
							// MB_WALK4:
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0,   0,  // antenne droite
			  0,   0,   0,  // antenne gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // avant-bras droite
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // avant-bras gauche
		};

		for ( i=0 ; i<3*10*MB_MAX ; i++ )
		{
			m_armAngles[i] = member[i];
		}

		character->height = 3.5f;
	}

	if ( type == OBJECT_STARTER )
	{
		short member[] =
		{
							// MB_WAIT:
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0,   0,  // antenne droite
			  0,   0,   0,  // antenne gauche
			-15, -20,   0,  // bras droite
			-35,-120,   0,  // avant-bras droite
			 15,  20,   0,  // bras gauche
			 35, 120,   0,  // avant-bras gauche
							// MB_TRUCK:
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			-65,   0,   0,  // antenne droite
			 65,   0,   0,  // antenne gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // avant-bras droite
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // avant-bras gauche
							// MB_FEAR:
			  0,   0,   0,  // socle
			  0,   0,  15,  // jambe
			  0,   0, -30,  // corps
			  0,   0, -20,  // tête
			  0,   0, -40,  // antenne droite
			  0,   0, -40,  // antenne gauche
			 20, -20, -15,  // bras droite
			 80, -90, -30,  // avant-bras droite
			-20,  20, -15,  // bras gauche
			-80,  90, -30,  // avant-bras gauche
							// MB_GOHOME:
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0,   0,  // antenne droite
			  0,   0,   0,  // antenne gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // avant-bras droite
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // avant-bras gauche
							// MB_HOME1: (circulez)
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,  30,   0,  // tête
			  0,   0,  40,  // antenne droite
			  0,   0,  40,  // antenne gauche
			 35, -20, -30,  // bras droite
			 55,  15, -10,  // avant-bras droite
			-35,  20, -30,  // bras gauche
			-55, -15, -10,  // avant-bras gauche
							// MB_HOME2:
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0,   0,  // antenne droite
			  0,   0,   0,  // antenne gauche
			  5, -20,   0,  // bras droite
			-20,-110, -75,  // avant-bras droite
			 -5,  20,   0,  // bras gauche
			 20, 110, -75,  // avant-bras gauche
							// MB_FLIC:
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0, -30,  // antenne droite
			  0,   0, -30,  // antenne gauche
			 35, -20, -30,  // bras droite
			 55,  15, -10,  // avant-bras droite
			 15,  20,   0,  // bras gauche
			 55, 170, -10,  // avant-bras gauche
							// MB_WALK1:
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0,   0,  // antenne droite
			  0,   0,   0,  // antenne gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // avant-bras droite
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // avant-bras gauche
							// MB_WALK2:
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0,   0,  // antenne droite
			  0,   0,   0,  // antenne gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // avant-bras droite
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // avant-bras gauche
							// MB_WALK3:
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0,   0,  // antenne droite
			  0,   0,   0,  // antenne gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // avant-bras droite
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // avant-bras gauche
							// MB_WALK4:
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0,   0,  // antenne droite
			  0,   0,   0,  // antenne gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // avant-bras droite
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // avant-bras gauche
		};

		for ( i=0 ; i<3*10*MB_MAX ; i++ )
		{
			m_armAngles[i] = member[i];
		}

		character->height = 3.5f;
	}

	if ( type == OBJECT_WALKER )
	{
		short member[] =
		{
							// MB_WAIT:
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0,  -5,  // tête
			  0,   0,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  20,  // pied droite
			  0,   0,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  20,  // pied gauche
			-15,  20,   0,  // bras droite (bras écartés)
			-90, -30,  30,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 15, -20,   0,  // bras gauche
			 90,  30,  30,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_TRUCK:
			  0,   0,   0,  // corps
			  0,   0,   0,  // coup
			  0,   0,   0,  // tête
			  0,  10,  50,  // jambe droite
			  0,   0,-100,  // jambe droite
			  0,   0,  35,  // pied droite
			  0, -10,  50,  // jambe gauche
			  0,   0,-100,  // jambe gauche
			  0,   0,  35,  // pied gauche
			 15,  20,   0,  // bras droite
			-90, -30,  30,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			-15, -20,   0,  // bras gauche
			 90,  30,  30,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_FEAR:
			  0,   0,   0,  // corps
			  0,   0,  15,  // coup
			  0,   0,  10,  // tête
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
			 20,  30,   0,  // bras droite
			-55,-175, -55,  // bras droite
			 60,   0,   0,  // main droite sup
			-60,   0,   0,  // main droite inf
			-20, -30,   0,  // bras gauche
			 55, 175, -55,  // bras gauche
			-60,   0,   0,  // main gauche sup
			 60,   0,   0,  // main gauche inf
							// MB_GOHOME:
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0, -10,  // tête
			  0,   5,  70,  // jambe droite
			  0,   0,-130,  // jambe droite
			  0,   0,  45,  // pied droite
			-10,  -5,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  30,  // pied gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_HOME1:
			  0,   0,   0,  // corps
			  0,   0,   0,  // coup
			  0,   0,   0,  // tête
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_HOME2 (dans véhicule):
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0,  -5,  // tête
			  0,   0,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  20,  // pied droite
			  0,   0,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  20,  // pied gauche
			-30, -55,   0,  // bras droite (bras croisés)
			-25, -90,  55,  // bras droite
			 10, -40,   0,  // main droite sup
			-10, -40,   0,  // main droite inf
			 20,  65,   0,  // bras gauche
			  0, 120,-110,  // bras gauche
			-10,  40,   0,  // main gauche sup
			 10,  40,   0,  // main gauche inf
							// MB_FLIC:
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0,  -5,  // tête
			  0,   0,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  25,  // pied droite
			  0,   0,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  25,  // pied gauche
			 35, -20,   0,  // bras droite
			 70, -75,  15,  // bras droite
			-45,   0,   0,  // main droite sup
			-100,  0,   0,  // main droite inf
			 15,  20,   0,  // bras gauche
			 70, 155, -20,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_WALK1:
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0, -10,  // tête
			 10,   5,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  30,  // pied droite
			  0,  -5,  70,  // jambe gauche
			  0,   0,-130,  // jambe gauche
			  0,   0,  45,  // pied gauche
			-55,   0, -50,  // bras droite
			-35,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 55,   0,  70,  // bras gauche
			 35,   0,  50,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_WALK2:
			  0,   0,   0,  // corps
			  0,   0,   0,  // coup
			  0,   0,   0,  // tête
			  0,   5, -35,  // jambe droite
			  0,   0, -60,  // jambe droite
			  0,   0,  20,  // pied droite
			  0,  -5,  80,  // jambe gauche
			  0,   0, -50,  // jambe gauche
			  0,   0,  10,  // pied gauche
			-55,   0,   0,  // bras droite
			-35,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 55,   0,   0,  // bras gauche
			 35,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_WALK3:
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0, -10,  // tête
			  0,   5,  70,  // jambe droite
			  0,   0,-130,  // jambe droite
			  0,   0,  45,  // pied droite
			-10,  -5,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  30,  // pied gauche
			-55,   0,  70,  // bras droite
			-35,   0,  50,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 55,   0, -50,  // bras gauche
			 35,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_WALK4:
			  0,   0,   0,  // corps
			  0,   0,   0,  // coup
			  0,   0,   0,  // tête
			  0,   5,  80,  // jambe droite
			  0,   0, -50,  // jambe droite
			  0,   0,  10,  // pied droite
			  0,  -5, -35,  // jambe gauche
			  0,   0, -60,  // jambe gauche
			  0,   0,  20,  // pied gauche
			-55,   0,   0,  // bras droite
			-35,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 55,   0,   0,  // bras gauche
			 35,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
		};

		for ( i=0 ; i<3*17*MB_MAX ; i++ )
		{
			m_armAngles[i] = member[i];
		}

		character->height = 3.8f;
	}

	if ( type == OBJECT_CRAZY )
	{
		short member[] =
		{
							// MB_WAIT:
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0,  -5,  // tête
			  0,   0,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  20,  // pied droite
			  0,   0,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  20,  // pied gauche
			-15,  20,   0,  // bras droite (bras écartés)
			-90, -30,  30,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 15, -20,   0,  // bras gauche
			 90,  30,  30,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_TRUCK:
			  0,   0,   0,  // corps
			  0,   0,   0,  // coup
			  0,   0,   0,  // tête
			  0,  10,  50,  // jambe droite
			  0,   0,-100,  // jambe droite
			  0,   0,  35,  // pied droite
			  0, -10,  50,  // jambe gauche
			  0,   0,-100,  // jambe gauche
			  0,   0,  35,  // pied gauche
			 15,  20,   0,  // bras droite
			-90, -30,  30,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			-15, -20,   0,  // bras gauche
			 90,  30,  30,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_FEAR:
			  0,   0,   0,  // corps
			  0,   0,  15,  // coup
			  0,   0,  10,  // tête
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
			 20,  30,   0,  // bras droite
			-55,-175, -55,  // bras droite
			 60,   0,   0,  // main droite sup
			-60,   0,   0,  // main droite inf
			-20, -30,   0,  // bras gauche
			 55, 175, -55,  // bras gauche
			-60,   0,   0,  // main gauche sup
			 60,   0,   0,  // main gauche inf
							// MB_GOHOME:
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0, -10,  // tête
			  0,   5,  70,  // jambe droite
			  0,   0,-130,  // jambe droite
			  0,   0,  45,  // pied droite
			-10,  -5,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  30,  // pied gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_HOME1:
			  0,   0,   0,  // corps
			  0,   0,   0,  // coup
			  0,   0,   0,  // tête
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_HOME2 (dans véhicule):
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0,  -5,  // tête
			  0,   0,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  20,  // pied droite
			  0,   0,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  20,  // pied gauche
			-30, -55,   0,  // bras droite (bras croisés)
			-25, -90,  55,  // bras droite
			 10, -40,   0,  // main droite sup
			-10, -40,   0,  // main droite inf
			 20,  65,   0,  // bras gauche
			  0, 120,-110,  // bras gauche
			-10,  40,   0,  // main gauche sup
			 10,  40,   0,  // main gauche inf
							// MB_FLIC:
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0,  -5,  // tête
			  0,   0,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  25,  // pied droite
			  0,   0,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  25,  // pied gauche
			 35, -20,   0,  // bras droite
			 70, -75,  15,  // bras droite
			-45,   0,   0,  // main droite sup
			-100,  0,   0,  // main droite inf
			 15,  20,   0,  // bras gauche
			 70, 155, -20,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_WALK1:
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0, -10,  // tête
			 10,   5,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  30,  // pied droite
			  0,  -5,  70,  // jambe gauche
			  0,   0,-130,  // jambe gauche
			  0,   0,  45,  // pied gauche
			-55,   0, -50,  // bras droite
			-35,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 55,   0,  70,  // bras gauche
			 35,   0,  50,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_WALK2:
			  0,   0,   0,  // corps
			  0,   0,   0,  // coup
			  0,   0,   0,  // tête
			  0,   5, -35,  // jambe droite
			  0,   0, -60,  // jambe droite
			  0,   0,  20,  // pied droite
			  0,  -5,  80,  // jambe gauche
			  0,   0, -50,  // jambe gauche
			  0,   0,  10,  // pied gauche
			-55,   0,   0,  // bras droite
			-35,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 55,   0,   0,  // bras gauche
			 35,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_WALK3:
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0, -10,  // tête
			  0,   5,  70,  // jambe droite
			  0,   0,-130,  // jambe droite
			  0,   0,  45,  // pied droite
			-10,  -5,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  30,  // pied gauche
			-55,   0,  70,  // bras droite
			-35,   0,  50,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 55,   0, -50,  // bras gauche
			 35,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_WALK4:
			  0,   0,   0,  // corps
			  0,   0,   0,  // coup
			  0,   0,   0,  // tête
			  0,   5,  80,  // jambe droite
			  0,   0, -50,  // jambe droite
			  0,   0,  10,  // pied droite
			  0,  -5, -35,  // jambe gauche
			  0,   0, -60,  // jambe gauche
			  0,   0,  20,  // pied gauche
			-55,   0,   0,  // bras droite
			-35,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 55,   0,   0,  // bras gauche
			 35,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
		};

		short member_crazy1[] =		// fou
		{
							// MB_WAIT:
			  0,   0,   0,  // corps
			-10,   0,   0,  // coup
			-10,   0,   0,  // tête
			  0,   0, 100,  // jambe droite
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,  25,  // jambe gauche
			  0,   0, -45,  // jambe gauche
			-10,   0,  25,  // pied gauche
			-15,   0,   0,  // bras droite
			 -5,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 15,   0,   0,  // bras gauche
			  5,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_TRUCK (attend ordi stoppé) :
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0,  -5,  // tête
			  0,   0,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  20,  // pied droite
			  0,   0,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  20,  // pied gauche
			-15,  20,   0,  // bras droite (bras écartés)
			-90, -30,  30,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 15, -20,   0,  // bras gauche
			 90,  30,  30,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_FEAR:
			  0,   0,   0,  // corps
			  0,   0,  15,  // coup
			  0,   0,  10,  // tête
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
			 20,  30,   0,  // bras droite
			-55,-175, -55,  // bras droite
			 60,   0,   0,  // main droite sup
			-60,   0,   0,  // main droite inf
			-20, -30,   0,  // bras gauche
			 55, 175, -55,  // bras gauche
			-60,   0,   0,  // main gauche sup
			 60,   0,   0,  // main gauche inf
							// MB_GOHOME:
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0, -10,  // tête
			  0,   5,  70,  // jambe droite
			  0,   0,-130,  // jambe droite
			  0,   0,  45,  // pied droite
			-10,  -5,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  30,  // pied gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_HOME1:
			  0,   0,   0,  // corps
			  0,   0,   0,  // coup
			  0,   0,   0,  // tête
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_HOME2 (dans véhicule):
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0,  -5,  // tête
			  0,   0,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  20,  // pied droite
			  0,   0,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  20,  // pied gauche
			-30, -55,   0,  // bras droite (bras croisés)
			-25, -90,  55,  // bras droite
			 10, -40,   0,  // main droite sup
			-10, -40,   0,  // main droite inf
			 20,  65,   0,  // bras gauche
			  0, 120,-110,  // bras gauche
			-10,  40,   0,  // main gauche sup
			 10,  40,   0,  // main gauche inf
							// MB_FLIC:
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0,  -5,  // tête
			  0,   0,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  25,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0, -30,  // jambe gauche
			  0,   0,  30,  // pied gauche
			 35, -20,   0,  // bras droite
			 70, -75,  15,  // bras droite
			-45,   0,   0,  // main droite sup
			-100,  0,   0,  // main droite inf
			 15,  20,   0,  // bras gauche
			 70, 155, -20,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_WALK1:
			  0,   0,   0,  // corps
			-10,   0, -10,  // coup
			-10,   0, -10,  // tête
			  0,   0,   0,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  60,  // pied droite
			  0,   0,  85,  // jambe gauche
			  0,   0, -65,  // jambe gauche
			  0,   0,  15,  // pied gauche
			-15,   0, -20,  // bras droite
			-65,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 15,   0,  20,  // bras gauche
			 65,   0,  50,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_WALK2:
			  0,   0,   0,  // corps
			  0,   0,  10,  // coup
			  0,   0,  10,  // tête
			  0,   0, 120,  // jambe droite
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0, -35,  // jambe gauche
			  0,   0, -30,  // jambe gauche
			  0,   0,  -5,  // pied gauche
			-15,   0,   0,  // bras droite
			-65,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 15,   0,   0,  // bras gauche
			 65,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_WALK3:
			  0,   0,   0,  // corps
			 10,   0, -10,  // coup
			 10,   0, -10,  // tête
			  0,   0,  85,  // jambe droite
			  0,   0, -65,  // jambe droite
			  0,   0,  15,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  60,  // pied gauche
			-15,   0,  20,  // bras droite
			-65,   0,  50,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 15,   0, -20,  // bras gauche
			 65,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_WALK4:
			  0,   0,   0,  // corps
			  0,   0,  10,  // coup
			  0,   0,  10,  // tête
			  0,   0, -35,  // jambe droite
			  0,   0, -30,  // jambe droite
			  0,   0,  -5,  // pied droite
			  0,   0, 120,  // jambe gauche
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
			-15,   0,   0,  // bras droite
			-65,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 15,   0,   0,  // bras gauche
			 65,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
		};

		short member_crazy2[] =		// bras croisés
		{
							// MB_WAIT:
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0,  -5,  // tête
			  0,   0, 105,  // jambe droite
			  0,   0,-150,  // jambe droite
			  0,   0,  15,  // pied droite
			  0,   0,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  20,  // pied gauche
			-30, -55,   0,  // bras droite (bras croisés)
			-25, -90,  55,  // bras droite
			 10, -40,   0,  // main droite sup
			-10, -40,   0,  // main droite inf
			 20,  65,   0,  // bras gauche
			  0, 120,-110,  // bras gauche
			-10,  40,   0,  // main gauche sup
			 10,  40,   0,  // main gauche inf
							// MB_TRUCK (attend ordi stoppé) :
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0,  -5,  // tête
			  0,   0,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  20,  // pied droite
			  0,   0,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  20,  // pied gauche
			-15,  20,   0,  // bras droite (bras écartés)
			-90, -30,  30,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 15, -20,   0,  // bras gauche
			 90,  30,  30,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_FEAR:
			  0,   0,   0,  // corps
			  0,   0,  15,  // coup
			  0,   0,  10,  // tête
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
			 20,  30,   0,  // bras droite
			-55,-175, -55,  // bras droite
			 60,   0,   0,  // main droite sup
			-60,   0,   0,  // main droite inf
			-20, -30,   0,  // bras gauche
			 55, 175, -55,  // bras gauche
			-60,   0,   0,  // main gauche sup
			 60,   0,   0,  // main gauche inf
							// MB_GOHOME:
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0, -10,  // tête
			  0,   5,  70,  // jambe droite
			  0,   0,-130,  // jambe droite
			  0,   0,  45,  // pied droite
			-10,  -5,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  30,  // pied gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_HOME1:
			  0,   0,   0,  // corps
			  0,   0,   0,  // coup
			  0,   0,   0,  // tête
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_HOME2 (dans véhicule):
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0,  -5,  // tête
			  0,   0,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  20,  // pied droite
			  0,   0,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  20,  // pied gauche
			-30, -55,   0,  // bras droite (bras croisés)
			-25, -90,  55,  // bras droite
			 10, -40,   0,  // main droite sup
			-10, -40,   0,  // main droite inf
			 20,  65,   0,  // bras gauche
			  0, 120,-110,  // bras gauche
			-10,  40,   0,  // main gauche sup
			 10,  40,   0,  // main gauche inf
							// MB_FLIC:
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0,  -5,  // tête
			  0,   0,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  25,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0, -30,  // jambe gauche
			  0,   0,  30,  // pied gauche
			 35, -20,   0,  // bras droite
			 70, -75,  15,  // bras droite
			-45,   0,   0,  // main droite sup
			-100,  0,   0,  // main droite inf
			 15,  20,   0,  // bras gauche
			 70, 155, -20,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_WALK1:
			  0,   0,   0,  // corps
			-20,   0,   0,  // coup
			-20,   0,   0,  // tête
			  0,   0,   0,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  60,  // pied droite
			  0,   0,  65,  // jambe gauche
			  0,   0, -30,  // jambe gauche
			  0,   0,  -5,  // pied gauche
			-30, -55,   0,  // bras droite (bras croisés)
			-25, -90,  55,  // bras droite
			 10, -40,   0,  // main droite sup
			-10, -40,   0,  // main droite inf
			 20,  65,   0,  // bras gauche
			  0, 120,-110,  // bras gauche
			-10,  40,   0,  // main gauche sup
			 10,  40,   0,  // main gauche inf
							// MB_WALK2:
			  0,   0,   0,  // corps
			  0,   0,   0,  // coup
			  0,   0,   0,  // tête
			  0,   0, 105,  // jambe droite
			  0,   0,-150,  // jambe droite
			  0,   0,  15,  // pied droite
			  0,   0, -35,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  25,  // pied gauche
			-30, -55,   0,  // bras droite (bras croisés)
			-25, -90,  55,  // bras droite
			 10, -40,   0,  // main droite sup
			-10, -40,   0,  // main droite inf
			 20,  65,   0,  // bras gauche
			  0, 120,-110,  // bras gauche
			-10,  40,   0,  // main gauche sup
			 10,  40,   0,  // main gauche inf
							// MB_WALK3:
			  0,   0,   0,  // corps
			 20,   0,   0,  // coup
			 20,   0,   0,  // tête
			  0,   0,  65,  // jambe droite
			  0,   0, -30,  // jambe droite
			  0,   0,  -5,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  60,  // pied gauche
			-30, -55,   0,  // bras droite (bras croisés)
			-25, -90,  55,  // bras droite
			 10, -40,   0,  // main droite sup
			-10, -40,   0,  // main droite inf
			 20,  65,   0,  // bras gauche
			  0, 120,-110,  // bras gauche
			-10,  40,   0,  // main gauche sup
			 10,  40,   0,  // main gauche inf
							// MB_WALK4:
			  0,   0,   0,  // corps
			  0,   0,   0,  // coup
			  0,   0,   0,  // tête
			  0,   0, -35,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  25,  // pied droite
			  0,   0, 105,  // jambe gauche
			  0,   0,-150,  // jambe gauche
			  0,   0,  15,  // pied gauche
			-30, -55,   0,  // bras droite (bras croisés)
			-25, -90,  55,  // bras droite
			 10, -40,   0,  // main droite sup
			-10, -40,   0,  // main droite inf
			 20,  65,   0,  // bras gauche
			  0, 120,-110,  // bras gauche
			-10,  40,   0,  // main gauche sup
			 10,  40,   0,  // main gauche inf
		};

		short member_crazy3[] =		// ss
		{
							// MB_WAIT:
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0,  -5,  // tête
			  0,   0,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  20,  // pied droite
			  0,   0,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  20,  // pied gauche
			-15,  20,   0,  // bras droite (bras écartés)
			-90, -30,  30,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 15, -20,   0,  // bras gauche
			 90,  30,  30,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_TRUCK (attend ordi stoppé) :
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0,  -5,  // tête
			  0,   0,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  20,  // pied droite
			  0,   0,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  20,  // pied gauche
			-15,  20,   0,  // bras droite (bras écartés)
			-90, -30,  30,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 15, -20,   0,  // bras gauche
			 90,  30,  30,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_FEAR:
			  0,   0,   0,  // corps
			  0,   0,  15,  // coup
			  0,   0,  10,  // tête
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
			 20,  30,   0,  // bras droite
			-55,-175, -55,  // bras droite
			 60,   0,   0,  // main droite sup
			-60,   0,   0,  // main droite inf
			-20, -30,   0,  // bras gauche
			 55, 175, -55,  // bras gauche
			-60,   0,   0,  // main gauche sup
			 60,   0,   0,  // main gauche inf
							// MB_GOHOME:
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0, -10,  // tête
			  0,   5,  70,  // jambe droite
			  0,   0,-130,  // jambe droite
			  0,   0,  45,  // pied droite
			-10,  -5,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  30,  // pied gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_HOME1:
			  0,   0,   0,  // corps
			  0,   0,   0,  // coup
			  0,   0,   0,  // tête
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_HOME2 (dans véhicule):
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0,  -5,  // tête
			  0,   0,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  20,  // pied droite
			  0,   0,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  20,  // pied gauche
			-30, -55,   0,  // bras droite (bras croisés)
			-25, -90,  55,  // bras droite
			 10, -40,   0,  // main droite sup
			-10, -40,   0,  // main droite inf
			 20,  65,   0,  // bras gauche
			  0, 120,-110,  // bras gauche
			-10,  40,   0,  // main gauche sup
			 10,  40,   0,  // main gauche inf
							// MB_FLIC:
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0,  -5,  // tête
			  0,   0,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  25,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0, -30,  // jambe gauche
			  0,   0,  30,  // pied gauche
			 35, -20,   0,  // bras droite
			 70, -75,  15,  // bras droite
			-45,   0,   0,  // main droite sup
			-100,  0,   0,  // main droite inf
			 15,  20,   0,  // bras gauche
			 70, 155, -20,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_WALK1:
			  0,   0,   0,  // corps
			  0,   0,  10,  // coup
			  0,   0,  10,  // tête
			  0,   0,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  20,  // pied droite
			  0,   0,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  20,  // pied gauche
			-55,   0, -50,  // bras droite
			-35,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 55,   0,  70,  // bras gauche
			 35,   0,  50,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_WALK2:
			  0,   0,   0,  // corps
			  0,   0,  20,  // coup
			  0,   0,  20,  // tête
			  0,   0,  90,  // jambe droite
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0, -30,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  30,  // pied gauche
			-55,   0,   0,  // bras droite
			-35,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 55,   0,   0,  // bras gauche
			 35,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_WALK3:
			  0,   0,   0,  // corps
			  0,   0,  10,  // coup
			  0,   0,  10,  // tête
			  0,   0,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  20,  // pied droite
			  0,   0,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  20,  // pied gauche
			-55,   0,  70,  // bras droite
			-35,   0,  50,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 55,   0, -50,  // bras gauche
			 35,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_WALK4:
			  0,   0,   0,  // corps
			  0,   0,  20,  // coup
			  0,   0,  20,  // tête
			  0,   0, -30,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  30,  // pied droite
			  0,   0,  90,  // jambe gauche
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
			-55,   0,   0,  // bras droite
			-35,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 55,   0,   0,  // bras gauche
			 35,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
		};

		short member_crazy4[] =		// random
		{
							// MB_WAIT:
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0,  -5,  // tête
			  0,   0,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  20,  // pied droite
			  0,   0,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  20,  // pied gauche
			-15,  20,   0,  // bras droite (bras écartés)
			-90, -30,  30,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 15, -20,   0,  // bras gauche
			 90,  30,  30,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_TRUCK (attend ordi stoppé) :
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0,  -5,  // tête
			  0,   0,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  20,  // pied droite
			  0,   0,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  20,  // pied gauche
			-15,  20,   0,  // bras droite (bras écartés)
			-90, -30,  30,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 15, -20,   0,  // bras gauche
			 90,  30,  30,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_FEAR:
			  0,   0,   0,  // corps
			  0,   0,  15,  // coup
			  0,   0,  10,  // tête
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
			 20,  30,   0,  // bras droite
			-55,-175, -55,  // bras droite
			 60,   0,   0,  // main droite sup
			-60,   0,   0,  // main droite inf
			-20, -30,   0,  // bras gauche
			 55, 175, -55,  // bras gauche
			-60,   0,   0,  // main gauche sup
			 60,   0,   0,  // main gauche inf
							// MB_GOHOME:
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0, -10,  // tête
			  0,   5,  70,  // jambe droite
			  0,   0,-130,  // jambe droite
			  0,   0,  45,  // pied droite
			-10,  -5,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  30,  // pied gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_HOME1:
			  0,   0,   0,  // corps
			  0,   0,   0,  // coup
			  0,   0,   0,  // tête
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_HOME2 (dans véhicule):
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0,  -5,  // tête
			  0,   0,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  20,  // pied droite
			  0,   0,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  20,  // pied gauche
			-30, -55,   0,  // bras droite (bras croisés)
			-25, -90,  55,  // bras droite
			 10, -40,   0,  // main droite sup
			-10, -40,   0,  // main droite inf
			 20,  65,   0,  // bras gauche
			  0, 120,-110,  // bras gauche
			-10,  40,   0,  // main gauche sup
			 10,  40,   0,  // main gauche inf
							// MB_FLIC:
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0,  -5,  // tête
			  0,   0,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  25,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0, -30,  // jambe gauche
			  0,   0,  30,  // pied gauche
			 35, -20,   0,  // bras droite
			 70, -75,  15,  // bras droite
			-45,   0,   0,  // main droite sup
			-100,  0,   0,  // main droite inf
			 15,  20,   0,  // bras gauche
			 70, 155, -20,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_WALK1:
			  0,   0,   0,  // corps
			-10,   0, -10,  // coup
			-10,   0, -10,  // tête
			  0,   0,   0,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  60,  // pied droite
			  0,   0,  85,  // jambe gauche
			  0,   0, -65,  // jambe gauche
			  0,   0,  15,  // pied gauche
			-15,   0, -20,  // bras droite
			-65,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 15,   0,  20,  // bras gauche
			 65,   0,  50,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_WALK2:
			  0,   0,   0,  // corps
			  0,   0,  10,  // coup
			  0,   0,  10,  // tête
			  0,   0, 120,  // jambe droite
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0, -35,  // jambe gauche
			  0,   0, -30,  // jambe gauche
			  0,   0,  -5,  // pied gauche
			-15,   0,   0,  // bras droite
			-65,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 15,   0,   0,  // bras gauche
			 65,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_WALK3:
			  0,   0,   0,  // corps
			 10,   0, -10,  // coup
			 10,   0, -10,  // tête
			  0,   0,  85,  // jambe droite
			  0,   0, -65,  // jambe droite
			  0,   0,  15,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  60,  // pied gauche
			-15,   0,  20,  // bras droite
			-65,   0,  50,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 15,   0, -20,  // bras gauche
			 65,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MB_WALK4:
			  0,   0,   0,  // corps
			  0,   0,  10,  // coup
			  0,   0,  10,  // tête
			  0,   0, -35,  // jambe droite
			  0,   0, -30,  // jambe droite
			  0,   0,  -5,  // pied droite
			  0,   0, 120,  // jambe gauche
			  0,   0,   0,  // jambe gauche
			  0,   0,   0,  // pied gauche
			-15,   0,   0,  // bras droite
			-65,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 15,   0,   0,  // bras gauche
			 65,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
		};

		for ( i=0 ; i<3*17*MB_MAX ; i++ )
		{
			if ( m_option == 1 )
			{
				m_armAngles[i] = member_crazy1[i];
			}
			else if ( m_option == 2 )
			{
				m_armAngles[i] = member_crazy2[i];
			}
			else if ( m_option == 3 )
			{
				m_armAngles[i] = member_crazy3[i];
			}
			else if ( m_option == 4 )
			{
				m_armAngles[i] = member_crazy4[i];
			}
			else
			{
				m_armAngles[i] = member[i];
			}
		}

		character->height = 3.8f;
	}

	if ( type == OBJECT_GUIDE )
	{
		short member[] =
		{
							// MB_WAIT:
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0,   0,  // antenne droite
			  0,   0,   0,  // antenne gauche
			-15, -20,   0,  // bras droite
			-35,-120,   0,  // avant-bras droite
			 15,  20,   0,  // bras gauche
			 35, 120,   0,  // avant-bras gauche
							// MB_TRUCK:
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			-65,   0,   0,  // antenne droite
			 65,   0,   0,  // antenne gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // avant-bras droite
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // avant-bras gauche
							// MB_FEAR:
			  0,   0,   0,  // socle
			  0,   0,  15,  // jambe
			  0,   0, -30,  // corps
			  0,   0, -20,  // tête
			-15,   0, -40,  // antenne droite
			 30,   0, -40,  // antenne gauche
			 20, -20, -15,  // bras droite
			 80, -90, -30,  // avant-bras droite
			-20,  20, -15,  // bras gauche
			-80,  90, -10,  // avant-bras gauche
							// MB_GOHOME:
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0,   0,  // antenne droite
			  0,   0,   0,  // antenne gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // avant-bras droite
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // avant-bras gauche
							// MB_HOME1: (circulez)
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,  30,   0,  // tête
			  0,   0,  40,  // antenne droite
			  0,   0,  40,  // antenne gauche
			 35, -20, -30,  // bras droite
			 55,  15, -10,  // avant-bras droite
			-35,  20, -30,  // bras gauche
			-55, -15, -10,  // avant-bras gauche
							// MB_HOME2:
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0,   0,  // antenne droite
			  0,   0,   0,  // antenne gauche
			  5, -20,   0,  // bras droite
			-20,-110, -75,  // avant-bras droite
			 -5,  20,   0,  // bras gauche
			 20, 110, -75,  // avant-bras gauche
							// MB_FLIC:
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0, -30,  // antenne droite
			  0,   0, -30,  // antenne gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // avant-bras droite
			 20,   0,   0,  // bras gauche
			-40, -20,   0,  // avant-bras gauche
							// MB_WALK1:
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0,   0,  // antenne droite
			  0,   0,   0,  // antenne gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // avant-bras droite
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // avant-bras gauche
							// MB_WALK2:
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0,   0,  // antenne droite
			  0,   0,   0,  // antenne gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // avant-bras droite
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // avant-bras gauche
							// MB_WALK3:
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0,   0,  // antenne droite
			  0,   0,   0,  // antenne gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // avant-bras droite
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // avant-bras gauche
							// MB_WALK4:
			  0,   0,   0,  // socle
			  0,   0,   0,  // jambe
			  0,   0,   0,  // corps
			  0,   0,   0,  // tête
			  0,   0,   0,  // antenne droite
			  0,   0,   0,  // antenne gauche
			  0,   0,   0,  // bras droite
			  0,   0,   0,  // avant-bras droite
			  0,   0,   0,  // bras gauche
			  0,   0,   0,  // avant-bras gauche
		};

		for ( i=0 ; i<3*10*MB_MAX ; i++ )
		{
			m_armAngles[i] = member[i];
		}

		character->height = 3.5f;
	}

	if ( type == OBJECT_CARROT )
	{
		m_physics->SetType(TYPE_TANK);

		character->wheelFrontPos = D3DVECTOR( 6.0f, 3.0f, 7.0f);
		character->wheelBackPos  = D3DVECTOR(-8.0f, 3.0f, 7.0f);
		character->wheelFrontDim = 1.0f;
		character->wheelBackDim  = 1.0f;
		character->suspDetect    = 2.0f;
		character->mass          = 3000.0f;

		m_physics->SetLinMotionX(MO_ADVSPEED,120.0f);
		m_physics->SetLinMotionX(MO_RECSPEED, 40.0f);
		m_physics->SetLinMotionX(MO_ADVACCEL, 50.0f);
		m_physics->SetLinMotionX(MO_RECACCEL, 50.0f);
		m_physics->SetLinMotionX(MO_STOACCEL, 30.0f);
		m_physics->SetLinMotionX(MO_TERSLIDE,  5.0f);
		m_physics->SetLinMotionZ(MO_TERSLIDE,  5.0f);
		m_physics->SetLinMotionX(MO_TERFORCE, 50.0f);
		m_physics->SetLinMotionZ(MO_TERFORCE, 30.0f);
		m_physics->SetLinMotionZ(MO_MOTACCEL, 20.0f);

		m_physics->SetCirMotionY(MO_ADVSPEED,  2.0f*PI);
		m_physics->SetCirMotionY(MO_RECSPEED,  2.0f*PI);
		m_physics->SetCirMotionY(MO_ADVACCEL,  8.0f);
		m_physics->SetCirMotionY(MO_RECACCEL,  8.0f);
		m_physics->SetCirMotionY(MO_STOACCEL, 12.0f);
	}

	if ( type == OBJECT_STARTER )
	{
		m_physics->SetType(TYPE_TANK);

		character->wheelFrontPos = D3DVECTOR( 6.0f, 3.0f, 7.0f);
		character->wheelBackPos  = D3DVECTOR(-8.0f, 3.0f, 7.0f);
		character->wheelFrontDim = 1.0f;
		character->wheelBackDim  = 1.0f;
		character->suspDetect    = 2.0f;
		character->mass          = 3000.0f;

		m_physics->SetLinMotionX(MO_ADVSPEED,120.0f);
		m_physics->SetLinMotionX(MO_RECSPEED, 40.0f);
		m_physics->SetLinMotionX(MO_ADVACCEL, 50.0f);
		m_physics->SetLinMotionX(MO_RECACCEL, 50.0f);
		m_physics->SetLinMotionX(MO_STOACCEL, 30.0f);
		m_physics->SetLinMotionX(MO_TERSLIDE,  5.0f);
		m_physics->SetLinMotionZ(MO_TERSLIDE,  5.0f);
		m_physics->SetLinMotionX(MO_TERFORCE, 50.0f);
		m_physics->SetLinMotionZ(MO_TERFORCE, 30.0f);
		m_physics->SetLinMotionZ(MO_MOTACCEL, 20.0f);

		m_physics->SetCirMotionY(MO_ADVSPEED,  2.0f*PI);
		m_physics->SetCirMotionY(MO_RECSPEED,  2.0f*PI);
		m_physics->SetCirMotionY(MO_ADVACCEL,  8.0f);
		m_physics->SetCirMotionY(MO_RECACCEL,  8.0f);
		m_physics->SetCirMotionY(MO_STOACCEL, 12.0f);
	}

	if ( type == OBJECT_WALKER ||
		 type == OBJECT_CRAZY  )
	{
		m_physics->SetType(TYPE_TANK);

		character->wheelFrontPos = D3DVECTOR( 6.0f, 3.0f, 7.0f);
		character->wheelBackPos  = D3DVECTOR(-8.0f, 3.0f, 7.0f);
		character->wheelFrontDim = 1.0f;
		character->wheelBackDim  = 1.0f;
		character->suspDetect    = 2.0f;
		character->mass          = 3000.0f;

		if ( m_option == 0 )
		{
			m_physics->SetLinMotionX(MO_ADVSPEED, 17.0f);
			m_physics->SetLinMotionX(MO_RECSPEED, 10.0f);
			m_physics->SetLinMotionX(MO_ADVACCEL, 50.0f);
			m_physics->SetLinMotionX(MO_RECACCEL, 50.0f);
			m_physics->SetLinMotionX(MO_STOACCEL, 50.0f);
			m_physics->SetLinMotionX(MO_TERSLIDE,  5.0f);
			m_physics->SetLinMotionZ(MO_TERSLIDE,  5.0f);
			m_physics->SetLinMotionX(MO_TERFORCE, 10.0f);
			m_physics->SetLinMotionZ(MO_TERFORCE, 10.0f);
			m_physics->SetLinMotionZ(MO_MOTACCEL, 20.0f);

			m_physics->SetCirMotionY(MO_ADVSPEED,  1.0f*PI);
			m_physics->SetCirMotionY(MO_RECSPEED,  1.0f*PI);
			m_physics->SetCirMotionY(MO_ADVACCEL,  8.0f);
			m_physics->SetCirMotionY(MO_RECACCEL,  8.0f);
			m_physics->SetCirMotionY(MO_STOACCEL, 12.0f);
		}
		if ( m_option == 1 )  // fou ?
		{
			m_physics->SetLinMotionX(MO_ADVSPEED,  7.0f);
			m_physics->SetLinMotionX(MO_RECSPEED,  5.0f);
			m_physics->SetLinMotionX(MO_ADVACCEL, 50.0f);
			m_physics->SetLinMotionX(MO_RECACCEL, 50.0f);
			m_physics->SetLinMotionX(MO_STOACCEL, 50.0f);
			m_physics->SetLinMotionX(MO_TERSLIDE,  5.0f);
			m_physics->SetLinMotionZ(MO_TERSLIDE,  5.0f);
			m_physics->SetLinMotionX(MO_TERFORCE, 10.0f);
			m_physics->SetLinMotionZ(MO_TERFORCE, 10.0f);
			m_physics->SetLinMotionZ(MO_MOTACCEL, 20.0f);

			m_physics->SetCirMotionY(MO_ADVSPEED,  0.5f*PI);
			m_physics->SetCirMotionY(MO_RECSPEED,  0.5f*PI);
			m_physics->SetCirMotionY(MO_ADVACCEL,  4.0f);
			m_physics->SetCirMotionY(MO_RECACCEL,  4.0f);
			m_physics->SetCirMotionY(MO_STOACCEL,  6.0f);
		}
		if ( m_option == 2 )  // bras croisés ?
		{
			m_physics->SetLinMotionX(MO_ADVSPEED,  8.0f);
			m_physics->SetLinMotionX(MO_RECSPEED,  4.0f);
			m_physics->SetLinMotionX(MO_ADVACCEL, 50.0f);
			m_physics->SetLinMotionX(MO_RECACCEL, 50.0f);
			m_physics->SetLinMotionX(MO_STOACCEL, 50.0f);
			m_physics->SetLinMotionX(MO_TERSLIDE,  5.0f);
			m_physics->SetLinMotionZ(MO_TERSLIDE,  5.0f);
			m_physics->SetLinMotionX(MO_TERFORCE, 10.0f);
			m_physics->SetLinMotionZ(MO_TERFORCE, 10.0f);
			m_physics->SetLinMotionZ(MO_MOTACCEL, 20.0f);

			m_physics->SetCirMotionY(MO_ADVSPEED,  1.0f*PI);
			m_physics->SetCirMotionY(MO_RECSPEED,  1.0f*PI);
			m_physics->SetCirMotionY(MO_ADVACCEL,  8.0f);
			m_physics->SetCirMotionY(MO_RECACCEL,  8.0f);
			m_physics->SetCirMotionY(MO_STOACCEL, 12.0f);
		}
		if ( m_option == 3 )  // ss ?
		{
			m_physics->SetLinMotionX(MO_ADVSPEED,  6.0f);
			m_physics->SetLinMotionX(MO_RECSPEED,  3.0f);
			m_physics->SetLinMotionX(MO_ADVACCEL, 50.0f);
			m_physics->SetLinMotionX(MO_RECACCEL, 50.0f);
			m_physics->SetLinMotionX(MO_STOACCEL, 50.0f);
			m_physics->SetLinMotionX(MO_TERSLIDE,  5.0f);
			m_physics->SetLinMotionZ(MO_TERSLIDE,  5.0f);
			m_physics->SetLinMotionX(MO_TERFORCE, 10.0f);
			m_physics->SetLinMotionZ(MO_TERFORCE, 10.0f);
			m_physics->SetLinMotionZ(MO_MOTACCEL, 20.0f);

			m_physics->SetCirMotionY(MO_ADVSPEED,  1.0f*PI);
			m_physics->SetCirMotionY(MO_RECSPEED,  1.0f*PI);
			m_physics->SetCirMotionY(MO_ADVACCEL,  8.0f);
			m_physics->SetCirMotionY(MO_RECACCEL,  8.0f);
			m_physics->SetCirMotionY(MO_STOACCEL, 12.0f);
		}
		if ( m_option == 4 )  // random ?
		{
			m_physics->SetLinMotionX(MO_ADVSPEED,  8.0f);
			m_physics->SetLinMotionX(MO_RECSPEED,  4.0f);
			m_physics->SetLinMotionX(MO_ADVACCEL, 50.0f);
			m_physics->SetLinMotionX(MO_RECACCEL, 50.0f);
			m_physics->SetLinMotionX(MO_STOACCEL, 50.0f);
			m_physics->SetLinMotionX(MO_TERSLIDE,  5.0f);
			m_physics->SetLinMotionZ(MO_TERSLIDE,  5.0f);
			m_physics->SetLinMotionX(MO_TERFORCE, 10.0f);
			m_physics->SetLinMotionZ(MO_TERFORCE, 10.0f);
			m_physics->SetLinMotionZ(MO_MOTACCEL, 20.0f);

			m_physics->SetCirMotionY(MO_ADVSPEED, 10.0f*PI);
			m_physics->SetCirMotionY(MO_RECSPEED, 10.0f*PI);
			m_physics->SetCirMotionY(MO_ADVACCEL, 80.0f);
			m_physics->SetCirMotionY(MO_RECACCEL, 80.0f);
			m_physics->SetCirMotionY(MO_STOACCEL, 99.0f);
		}
	}

	if ( type == OBJECT_GUIDE )
	{
		m_physics->SetType(TYPE_TANK);

		character->wheelFrontPos = D3DVECTOR( 6.0f, 3.0f, 7.0f);
		character->wheelBackPos  = D3DVECTOR(-8.0f, 3.0f, 7.0f);
		character->wheelFrontDim = 1.0f;
		character->wheelBackDim  = 1.0f;
		character->suspDetect    = 2.0f;
		character->mass          = 3000.0f;

		m_physics->SetLinMotionX(MO_ADVSPEED,120.0f);
		m_physics->SetLinMotionX(MO_RECSPEED, 40.0f);
		m_physics->SetLinMotionX(MO_ADVACCEL, 50.0f);
		m_physics->SetLinMotionX(MO_RECACCEL, 50.0f);
		m_physics->SetLinMotionX(MO_STOACCEL, 30.0f);
		m_physics->SetLinMotionX(MO_TERSLIDE,  5.0f);
		m_physics->SetLinMotionZ(MO_TERSLIDE,  5.0f);
		m_physics->SetLinMotionX(MO_TERFORCE, 50.0f);
		m_physics->SetLinMotionZ(MO_TERFORCE, 30.0f);
		m_physics->SetLinMotionZ(MO_MOTACCEL, 20.0f);

		m_physics->SetCirMotionY(MO_ADVSPEED,  2.0f*PI);
		m_physics->SetCirMotionY(MO_RECSPEED,  2.0f*PI);
		m_physics->SetCirMotionY(MO_ADVACCEL,  8.0f);
		m_physics->SetCirMotionY(MO_RECACCEL,  8.0f);
		m_physics->SetCirMotionY(MO_STOACCEL, 12.0f);
	}

	if ( type == OBJECT_EVIL1 ||
		 type == OBJECT_EVIL3 )
	{
		m_physics->SetType(TYPE_TANK);

		character->wheelFrontPos = D3DVECTOR( 2.0f, 0.0f, 2.0f);
		character->wheelBackPos  = D3DVECTOR(-2.0f, 0.0f, 2.0f);
		character->wheelFrontDim = 1.0f;
		character->wheelBackDim  = 1.0f;
		character->suspDetect    = 2.0f;
		character->mass          = 3000.0f;

		m_physics->SetLinMotionX(MO_ADVSPEED,120.0f);
		m_physics->SetLinMotionX(MO_RECSPEED, 40.0f);
		m_physics->SetLinMotionX(MO_ADVACCEL, 50.0f);
		m_physics->SetLinMotionX(MO_RECACCEL, 50.0f);
		m_physics->SetLinMotionX(MO_STOACCEL, 30.0f);
		m_physics->SetLinMotionX(MO_TERSLIDE,  5.0f);
		m_physics->SetLinMotionZ(MO_TERSLIDE,  5.0f);
		m_physics->SetLinMotionX(MO_TERFORCE, 50.0f);
		m_physics->SetLinMotionZ(MO_TERFORCE, 30.0f);
		m_physics->SetLinMotionZ(MO_MOTACCEL, 20.0f);

		m_physics->SetCirMotionY(MO_ADVSPEED,  2.0f*PI);
		m_physics->SetCirMotionY(MO_RECSPEED,  2.0f*PI);
		m_physics->SetCirMotionY(MO_ADVACCEL,  8.0f);
		m_physics->SetCirMotionY(MO_RECACCEL,  8.0f);
		m_physics->SetCirMotionY(MO_STOACCEL, 12.0f);
	}

	m_armPartIndex = 0;
}


// Gestion d'un événement.

BOOL CMotionBot::EventProcess(const Event &event)
{
	CMotion::EventProcess(event);

	if ( event.event == EVENT_FRAME )
	{
		return EventFrame(event);
	}

	if ( event.event == EVENT_KEYDOWN )
	{
#if ADJUST_ANGLE
		int		i;

		if ( event.param == 'Q' )  m_armPartIndex++;
		if ( m_armPartIndex >= 17 )  m_armPartIndex = 0;
		if ( event.param == 'A' )  m_armPartIndex--;
		if ( m_armPartIndex < 0 )  m_armPartIndex = 17-1;

		m_actionType = MB_WAIT;
		i = 3*17*m_actionType;
		i += m_armPartIndex*3;

		if ( event.param == 'E' )  m_armAngles[i+xx] += 5;
		if ( event.param == 'D' )  m_armAngles[i+xx] -= 5;
		if ( event.param == 'R' )  m_armAngles[i+yy] += 5;
		if ( event.param == 'F' )  m_armAngles[i+yy] -= 5;
		if ( event.param == 'T' )  m_armAngles[i+zz] += 5;
		if ( event.param == 'G' )  m_armAngles[i+zz] -= 5;
#endif
	}

	return TRUE;
}

// Gestion d'un événement.

BOOL CMotionBot::EventFrame(const Event &event)
{
	CBrain*		brain;
	ObjectType	type;
	D3DMATRIX*	mat;
	D3DVECTOR	linVib, cirVib, zoom, zFactor, pos;
	FPOINT		rot;
	float		time, effect[3*20], angle, z, progress, dist, factor, s, prog;
	int			i, j, is, ie, max, action;

	if ( m_engine->RetPause() )  return TRUE;
//?	if ( !m_engine->IsVisiblePoint(m_object->RetPosition(0)) )  return TRUE;

	type = m_object->RetType();

	if ( (type != OBJECT_BOT1 || m_actionType != MB_WALK1) &&
		 type != OBJECT_GUIDE )
	{
		if ( m_object->RetExplo() )  return TRUE;  // en cours d'explosion ?
	}

#if ADJUST_ANGLE
	if ( m_object->RetSelect() )
	{
		char s[100];
		sprintf(s, "Q:part=%d", m_armPartIndex);
		m_engine->SetInfoText(4, s);
	}
#endif

	m_aTime += event.rTime;

	m_lastSound -= event.rTime;
	action = m_actionType;

	if ( type == OBJECT_STARTER )
	{
		if ( m_starterTime >= 0.0f )
		{
			if ( m_starterTime == 0.0f )
			{
				m_cameraType = m_camera->RetType();
				m_cameraObj = m_camera->RetObject();
				m_camera->SetObject(m_object);
				m_camera->SetType(CAMERA_BACK);
				m_camera->SetBackDist(65.0f);
				m_camera->SetSmooth(CS_HARD);
				m_camera->FixCamera();
			}
			m_starterTime += event.rTime;

			if ( m_main->RetStarterType() == STARTER_321 )
			{
				if ( m_starterTime >= 2.5f && m_starterPhase == 0 )
				{
					m_starterPhase = 1;
					SetAction(MB_FLIC);
				}

				if ( m_starterTime >= 3.5f && m_starterPhase == 1 )
				{
					m_starterPhase = 2;
				}

				if ( m_starterPhase == 0 )
				{
					dist = 65.0f-m_starterTime*55.0f/2.5f;
					m_camera->SetBackDist(dist);
				}
				if ( m_starterPhase == 2 )
				{
					dist = Mod(m_starterTime-3.5f, 1.0f);
					if ( dist < 0.5f )
					{
						dist = dist/0.5f;
						dist = 10.0f-dist*5.0f;
					}
					else
					{
						dist = (dist-0.5f)/0.5f;
						dist = 10.0f-(1.0f-dist)*5.0f;
					}
					m_camera->SetBackDist(dist);
				}

				if ( m_starterTime >= 7.0f )
				{
					if ( m_main->IsStartCounter() )
					{
						SetAction(MB_HOME1);
						m_camera->SetObject(m_cameraObj);
						m_camera->SetType(m_cameraType);
						m_camera->SetSmooth(CS_NORM);
					}
					else	// moteur explosé ?
					{
						SetAction(MB_FEAR);
					}
					m_starterTime = -1.0f;
				}
			}
			else
			{
				m_camera->SetBackDist(10.0f);
				if ( m_starterTime >= 1.5f && m_starterPhase == 0 )
				{
					m_starterPhase = 1;
					SetAction(MB_FLIC);
				}
				if ( m_starterTime >= 2.5f )
				{
					if ( m_main->IsStartCounter() )
					{
						SetAction(MB_HOME1);
						m_camera->SetObject(m_cameraObj);
						m_camera->SetType(m_cameraType);
						m_camera->SetSmooth(CS_NORM);
					}
					else	// moteur explosé ?
					{
						SetAction(MB_FEAR);
					}
					m_starterTime = -1.0f;
				}
			}
		}
	}

	time = event.rTime;
#if ADJUST_ANGLE
	time *= 100.0f;
#endif

	if ( type == OBJECT_BOT1    )  max = 10;
	if ( type == OBJECT_BOT2    )  max = 10;
	if ( type == OBJECT_BOT3    )  max = 15;
	if ( type == OBJECT_BOT4    )  max = 17;
	if ( type == OBJECT_CARROT  )  max = 10;
	if ( type == OBJECT_STARTER )  max = 10;
	if ( type == OBJECT_WALKER  )  max = 17;
	if ( type == OBJECT_CRAZY   )  max = 17;
	if ( type == OBJECT_GUIDE   )  max = 10;
	if ( type == OBJECT_EVIL1   )  max = 0;
	if ( type == OBJECT_EVIL2   )  max = 0;
	if ( type == OBJECT_EVIL3   )  max = 0;

	for ( i=0 ; i<3*max ; i++ )
	{
		effect[i] = 0.0f;
	}
	linVib = D3DVECTOR(0.0f, 0.0f, 0.0f);
	cirVib = D3DVECTOR(0.0f, 0.0f, 0.0f);

	if ( type == OBJECT_BOT1 )
	{
		if ( action == MB_WAIT )
		{
			effect[3*1+xx] = sinf(m_aTime*4.1f)*0.03f;  // jambe et corps
			effect[3*2+xx] = sinf(m_aTime*4.1f)*0.03f;
			effect[3*1+zz] = cosf(m_aTime*4.8f)*0.03f;
			effect[3*2+zz] = cosf(m_aTime*4.8f)*0.03f;

			effect[3*2+yy] = sinf(m_aTime*1.3f)*0.06f;  // corps
			effect[3*3+xx] = sinf(m_aTime*9.0f)*0.04f;  // tête
			effect[3*3+yy] = sinf(m_aTime*1.0f)*0.30f;
			effect[3*6+xx] = sinf(m_aTime*3.0f)*0.02f;  // bras droite
			effect[3*7+xx] = sinf(m_aTime*3.3f)*0.02f;
			effect[3*8+xx] = sinf(m_aTime*3.1f)*0.02f;  // bras gauche
			effect[3*9+xx] = sinf(m_aTime*3.5f)*0.02f;
			SpeedAdapt(effect, linVib, cirVib, event.rTime);
			time *= 5.0f;
		}

		if ( action == MB_TRUCK )
		{
			effect[3*3+yy] =  sinf(m_aTime*10.0f)*0.2f;
			effect[3*2+yy] = -sinf(m_aTime*10.0f)*0.3f;

			// Le corps se tortille.
			effect[3*1+xx] = sinf(m_aTime*5.0f)*0.1f;
			effect[3*2+xx] = sinf(m_aTime*5.0f)*0.1f;
			effect[3*3+xx] = sinf(m_aTime*5.0f)*0.1f;
			effect[3*1+zz] = cosf(m_aTime*5.0f)*0.1f;
			effect[3*2+zz] = cosf(m_aTime*5.0f)*0.1f;
			effect[3*3+zz] = cosf(m_aTime*5.0f)*0.1f;

			// Correction pour que la tête reste immobile sur l'aimant.
			rot.x = sinf(m_aTime*5.0f)*1.5f;
			rot.y = cosf(m_aTime*5.0f)*1.5f;
			rot = RotatePoint(-(m_object->RetAngleY(0)-PI*1.5f), rot);
			linVib.x += rot.x;
			linVib.z += rot.y;

			effect[3*6+xx] =  sinf(m_aTime*20.0f)*0.4f;  // bras droite
			effect[3*6+yy] =  cosf(m_aTime*20.0f)*0.4f;
			effect[3*7+xx] =  sinf(m_aTime*20.0f)*0.4f;  // avant-bras droite
			effect[3*7+yy] =  cosf(m_aTime*20.0f)*0.4f;
			effect[3*8+xx] = -sinf(m_aTime*20.0f)*0.4f;  // bras gauche
			effect[3*8+yy] = -cosf(m_aTime*20.0f)*0.4f;
			effect[3*9+xx] = -sinf(m_aTime*20.0f)*0.4f;  // avant-bras gauche
			effect[3*9+yy] = -cosf(m_aTime*20.0f)*0.4f;
			time *= 10.0f;
		}

		if ( action == MB_FEAR )
		{
			effect[3*3+yy] = sinf(m_aTime*12.0f)*0.06f;
			time *= 10.0f;
			if ( m_progress >= 1.0f )
			{
				SetAction(MB_WAIT);
			}
		}

		if ( action == MB_GOHOME )
		{
			time *= 3.0f;
		}

		if ( action == MB_HOME1 )
		{
			effect[3*1+xx] = sinf(m_aTime*4.1f)*0.03f;  // jambe et corps
			effect[3*2+xx] = sinf(m_aTime*4.1f)*0.03f;
			effect[3*1+zz] = cosf(m_aTime*4.8f)*0.03f;
			effect[3*2+zz] = cosf(m_aTime*4.8f)*0.03f;

			effect[3*2+yy] = sinf(m_aTime*1.3f)*0.06f;  // corps
			effect[3*3+xx] = sinf(m_aTime*9.0f)*0.04f;  // tête
			effect[3*3+yy] = sinf(m_aTime*1.0f)*0.30f;
			effect[3*6+xx] = sinf(m_aTime*3.0f)*0.02f;  // bras droite
			effect[3*7+xx] = sinf(m_aTime*3.3f)*0.02f;
			effect[3*8+xx] = sinf(m_aTime*3.1f)*0.02f;  // bras gauche
			effect[3*9+xx] = sinf(m_aTime*3.5f)*0.02f;
			time *= 10.0f;
		}

		if ( action == MB_HOME2 )
		{
			effect[3*1+xx] = sinf(m_aTime*8.1f)*0.03f;  // jambe et corps
			effect[3*2+xx] = sinf(m_aTime*8.1f)*0.03f;
			effect[3*1+zz] = cosf(m_aTime*8.8f)*0.03f;
			effect[3*2+zz] = cosf(m_aTime*8.8f)*0.03f;

			effect[3*2+yy] = sinf(m_aTime*5.3f)*0.30f;  // corps
			effect[3*3+xx] = sinf(m_aTime*9.0f)*0.04f;  // tête
			effect[3*3+yy] = sinf(m_aTime*3.1f)*0.30f;
			effect[3*3+zz] = sinf(m_aTime*8.1f)*0.30f;
			effect[3*4+zz] = sinf(m_aTime*8.1f)*0.60f;  // antenne droite
			effect[3*5+zz] = sinf(m_aTime*8.1f)*0.60f;  // antenne gauche
			effect[3*6+xx] = sinf(m_aTime*5.0f)*0.30f;  // bras droite
			effect[3*7+xx] = sinf(m_aTime*5.3f)*0.30f;
			effect[3*8+xx] = sinf(m_aTime*5.1f)*0.30f;  // bras gauche
			effect[3*9+xx] = sinf(m_aTime*5.5f)*0.30f;
			time *= 10.0f;
		}

		if ( action == MB_FLIC )
		{
			effect[3*1+xx] = sinf(m_aTime*4.1f)*0.03f;  // jambe et corps
			effect[3*2+xx] = sinf(m_aTime*4.1f)*0.03f;
			effect[3*1+zz] = cosf(m_aTime*4.8f)*0.03f;
			effect[3*2+zz] = cosf(m_aTime*4.8f)*0.03f;

			effect[3*3+yy] = sinf(m_aTime*2.3f)*0.30f;  // tête
			time *= 10.0f;
		}

		if ( action == MB_WALK1 )  // cassé au sol ?
		{
			BubbleBot1();
		}

		if ( action == MB_WALK2 )  // transporté cassé ?
		{
			cirVib.z -= 40.0f*PI/180.0f;
			linVib.y += 1.1f;
			linVib.z -= 0.5f;
			SpeedAdapt(effect, linVib, cirVib, event.rTime);
		}

		if ( m_bBreak )  // cassé ?
		{
			cirVib.z += PI*0.53f;  // couché sur le dos
			linVib.y -= 2.1f;
		}
	}

	if ( type == OBJECT_BOT2 )
	{
		if ( action == MB_WAIT )
		{
			float head[] = {0.0f, 0.0f, 0.78f, 0.0f, 0.0f, -0.78f};
			effect[3*1+yy] = head[(int)(m_aTime*1.5f)%6];

			effect[3*6+zz] =  sinf(m_aTime*3.0f)*0.2f;
			effect[3*7+zz] = -sinf(m_aTime*3.0f)*0.2f;
			effect[3*8+zz] = -sinf(m_aTime*3.0f)*0.2f;
			effect[3*9+zz] =  sinf(m_aTime*3.0f)*0.2f;
			SpeedAdapt(effect, linVib, cirVib, event.rTime);
			time *= 5.0f;
		}

		if ( action == MB_TRUCK )
		{
			effect[3*2+zz] =  sinf(m_aTime*8.0f)*0.2f;
			effect[3*3+zz] =  sinf(m_aTime*8.0f)*0.1f;
			effect[3*4+zz] = -sinf(m_aTime*8.0f)*0.2f;
			effect[3*5+zz] = -sinf(m_aTime*8.0f)*0.1f;
			effect[3*6+zz] = -sinf(m_aTime*8.0f)*0.2f;
			effect[3*8+zz] =  sinf(m_aTime*8.0f)*0.2f;
			time *= 5.0f;
		}

		if ( action == MB_FEAR )
		{
//?			cirVib.z += -20.0f*PI/180.0f;  // penche en avant
			linVib.x += -0.3f;
			linVib.y += -0.3f;
			time *= 5.0f;
			if ( m_progress >= 1.0f )
			{
				SetAction(MB_WAIT);
			}
		}

		if ( action == MB_GOHOME )
		{
			time *= 3.0f;
		}

		if ( action == MB_HOME1 )
		{
			float head[] = {0.0f, 0.0f, 0.78f, 0.0f, 0.0f, -0.78f};
			effect[3*1+yy] = head[(int)(m_aTime*1.5f)%6];

			linVib.x += -0.3f;
			linVib.y += -0.3f;
			time *= 5.0f;
		}

		if ( action == MB_HOME2 )
		{
			FireBot2();
			effect[3*6+zz] =  sinf(m_aTime*3.0f)*0.2f;
			effect[3*8+zz] =  cosf(m_aTime*3.0f)*0.2f;
			linVib.x +=  0.5f;
			linVib.y += -0.3f;
			time *= 5.0f;
		}

		linVib.y += 1.8f;  // pour poser les pieds sur le sol
	}

	if ( type == OBJECT_BOT3 )
	{
		if ( action == MB_WAIT )
		{
			effect[3* 1+yy] =  sinf(m_aTime*0.8f)*0.02f;  // pied droite
			effect[3* 4+yy] = -sinf(m_aTime*0.8f)*0.02f;  // pied gauche

			effect[3* 7+zz] =  sinf(m_aTime*1.0f)*0.06f;  // bras droite
			effect[3* 8+yy] =  sinf(m_aTime*1.3f)*0.04f;
			effect[3*10+zz] =  sinf(m_aTime*1.7f)*0.06f;  // bras droite
			effect[3*11+yy] =  sinf(m_aTime*1.4f)*0.04f;

			effect[3*13+xx] =  sinf(m_aTime*0.5f)*0.04f;  // sourcil droite
			effect[3*14+xx] =  sinf(m_aTime*0.7f)*0.04f;  // sourcil gauche

			SpeedAdapt(effect, linVib, cirVib, event.rTime);
			time *= 5.0f;
		}

		if ( action == MB_TRUCK )
		{
			// Le corps se tortille.
			cirVib.x = sinf(m_aTime*10.0f)*0.25f;
			cirVib.z = cosf(m_aTime*10.0f)*0.25f;

			// Correction pour que la tête reste immobile sur l'aimant.
			rot.x = sinf(m_aTime*10.0f)*1.0f;
			rot.y = cosf(m_aTime*10.0f)*1.0f;
			rot = RotatePoint(-(m_object->RetAngleY(0)-PI*1.5f), rot);
			linVib.x += rot.x;
			linVib.z += rot.y;

			effect[3* 1+zz] =  sinf(m_aTime*8.0f)*0.3f;  // jambe droite
			effect[3* 4+zz] = -sinf(m_aTime*8.0f)*0.3f;  // jambe gauche

			effect[3* 7+zz] = -sinf(m_aTime*12.0f)*0.4f;  // bras droite
			effect[3* 8+zz] = -sinf(m_aTime*12.0f)*0.4f;  // avant-bras droite
			effect[3*10+zz] =  sinf(m_aTime*12.0f)*0.4f;  // bras gauche
			effect[3*11+zz] =  sinf(m_aTime*12.0f)*0.4f;  // avant-bras gauche
			time *= 10.0f;
		}

		if ( action == MB_FEAR )
		{
			SpeedAdapt(effect, linVib, cirVib, event.rTime);
			time *= 10.0f;
			if ( m_progress >= 1.0f )
			{
				SetAction(MB_WAIT);
			}
		}

		if ( action == MB_GOHOME )
		{
			time *= 3.0f;
		}

		if ( action == MB_HOME1 )
		{
			time *= 10.0f;
		}

		if ( action == MB_HOME2 )
		{
			cirVib.z += 0.2f;  // assis en arrière
			linVib.y -= 1.0f;
			time *= 10.0f;
		}

		if ( action == MB_FLIC )
		{
			time *= 10.0f;
		}
	}

	if ( type == OBJECT_BOT4 )
	{
		if ( action == MB_WAIT )
		{
			float head[] = {0.0f, 0.0f, 0.0f, 0.4f, 0.0f, 0.0f, 0.0f, -0.4f};
			effect[3* 2+yy] = head[(int)(m_aTime*2.0f)%8];

			effect[3* 9+xx] = -powf(Abs( sinf(m_aTime*1.0f)), 0.5f)*0.08f;
			effect[3*10+xx] =  powf(Abs(-sinf(m_aTime*1.0f)), 0.5f)*0.08f;
			effect[3*13+xx] =  powf(Abs( sinf(m_aTime*1.0f)), 0.5f)*0.08f;
			effect[3*14+xx] = -powf(Abs(-sinf(m_aTime*1.0f)), 0.5f)*0.08f;
			effect[3* 9+zz] =  sinf(m_aTime*0.5f)*0.04f;
			effect[3*13+zz] = -sinf(m_aTime*0.5f)*0.04f;
			SpeedAdapt(effect, linVib, cirVib, event.rTime);
		}

		if ( action == MB_TRUCK )
		{
			effect[3* 1+yy] =  sinf(m_aTime*10.0f)*0.20f;  // coup
			effect[3* 2+yy] =  sinf(m_aTime*10.0f)*0.20f;  // tête
			cirVib.y = -sinf(m_aTime*10.0f)*0.40f;

			effect[3* 3+zz] =  sinf(m_aTime*10.0f)*0.12f;  // jambe droite
			effect[3* 4+zz] = -sinf(m_aTime*10.0f)*0.24f;
			effect[3* 5+zz] =  sinf(m_aTime*10.0f)*0.12f;
			effect[3* 6+zz] =  cosf(m_aTime*10.0f)*0.12f;  // jambe gauche
			effect[3* 7+zz] = -cosf(m_aTime*10.0f)*0.24f;
			effect[3* 8+zz] =  cosf(m_aTime*10.0f)*0.12f;
			effect[3* 9+xx] =  sinf(m_aTime*10.0f)*0.12f;  // bras droite
			effect[3*10+xx] = -sinf(m_aTime*10.0f)*0.24f;
			effect[3*13+xx] = -cosf(m_aTime*10.0f)*0.12f;  // bras gauche
			effect[3*14+xx] =  cosf(m_aTime*10.0f)*0.24f;
		}

		if ( action == MB_HOME2 )  // transporté dans véhicule ?
		{
			SpeedAdapt(effect, linVib, cirVib, event.rTime);
		}

		if ( action == MB_FEAR )
		{
			SpeedAdapt(effect, linVib, cirVib, event.rTime);
			if ( m_progress >= 1.0f )
			{
				SetAction(MB_WAIT);
			}
		}
		time *= 5.0f;
	}

	if ( type == OBJECT_EVIL1 ||
		 type == OBJECT_EVIL2 ||
		 type == OBJECT_EVIL3 )
	{
		cirVib = m_object->RetCirVibration();
		zFactor = D3DVECTOR(1.0f, 1.0f, 1.0f);

		if ( action == MB_WAIT )
		{
			SpeedAdapt(effect, linVib, cirVib, event.rTime);
		}

		if ( action == MB_TRUCK )
		{
			zFactor = D3DVECTOR(3.0f, 4.0f, 3.0f);
		}

		if ( action == MB_FEAR )
		{
			m_aTime += event.rTime*(1.0f-m_progress)*5.0f;  // tout va plus vite
			if ( m_progress >= 1.0f )
			{
				SetAction(MB_WAIT);
			}
		}

		if ( action != MB_GOHOME &&
			 action != -1        )
		{
			zoom.x = 1.0f+(sinf(m_aTime*1.7f)*0.10f+sinf(m_aTime*5.0f)*0.06f+cosf(m_aTime*15.0f)*0.03f)*zFactor.x;
			zoom.y = 1.0f+(sinf(m_aTime*1.9f)*0.05f+sinf(m_aTime*3.3f)*0.03f+cosf(m_aTime*13.1f)*0.02f)*zFactor.y;
			zoom.z = 1.0f+(sinf(m_aTime*1.3f)*0.10f+sinf(m_aTime*4.2f)*0.06f+cosf(m_aTime*17.9f)*0.03f)*zFactor.z;
			m_object->SetZoom(0, zoom);
		}

		if ( action == MB_TRUCK )
		{
			linVib.y += 5.6f-5.6f*zoom.y;  // collé en haut
		}

		if ( action == MB_FIRE )  // tir peu efficace ?
		{
			if ( m_progress < 0.2f )
			{
				progress = m_progress/0.2f;
				angle = 120.0f*PI/180.0f*(1.0f-progress);
				z = 0.2f+progress*0.8f;
			}
			else if ( m_progress < 0.8f )
			{
				FireEvil1a();
				progress = (m_progress-0.2f)/0.6f;
				angle = 0.0f;
				z = 1.0f;
			}
			else
			{
				progress = (m_progress-0.8f)/0.2f;
				angle = 120.0f*PI/180.0f*progress;
				z = 0.2f+(1.0f-progress)*0.8f;
			}
			m_object->SetAngleZ(1, angle);
			m_object->SetZoom(1, z);
		}

		if ( action == MB_WALK1 )  // tir très efficace ?
		{
			if ( m_progress < 0.1f )
			{
				progress = m_progress/0.1f;
				angle = 120.0f*PI/180.0f*(1.0f-progress*0.6f);
				z = 0.2f+progress*0.8f;
				pos.x = 0.7f+progress*0.3f;
				pos.y = 5.6f;
				pos.z = 0.0f;
			}
			else if ( m_progress < 0.9f )
			{
				FireEvil1b();
				progress = (m_progress-0.1f)/0.8f;
				angle = 120.0f*PI/180.0f*(1.0f-0.6f);
				z = 1.0f;
				pos.x = 0.7f+0.3f;
				pos.y = 5.6f;
				pos.z = 0.0f;
			}
			else
			{
				progress = (m_progress-0.9f)/0.1f;
				angle = 120.0f*PI/180.0f*(0.4f+progress*0.6f);
				z = 0.2f+(1.0f-progress)*0.8f;
				pos.x = 0.7f+(1.0f-progress)*0.3f;
				pos.y = 5.6f;
				pos.z = 0.0f;
			}
			m_object->SetAngleZ(1, angle);
			m_object->SetZoom(1, z);
			m_object->SetPosition(1, pos);
		}

		cirVib.y = sinf(m_aTime*2.8f)*0.3f+cosf(m_aTime*3.9f)*0.2f;
	}

	if ( type == OBJECT_CARROT )
	{
		if ( action == MB_WAIT )
		{
			effect[3*1+xx] = sinf(m_aTime*4.1f)*0.03f;  // jambe et corps
			effect[3*2+xx] = sinf(m_aTime*4.1f)*0.03f;
			effect[3*1+zz] = cosf(m_aTime*4.8f)*0.03f;
			effect[3*2+zz] = cosf(m_aTime*4.8f)*0.03f;

			effect[3*2+yy] = sinf(m_aTime*1.3f)*0.06f;  // corps
			effect[3*3+xx] = sinf(m_aTime*9.0f)*0.04f;  // tête
			effect[3*3+yy] = sinf(m_aTime*1.0f)*0.30f;
			effect[3*6+xx] = sinf(m_aTime*3.0f)*0.02f;  // bras droite
			effect[3*7+xx] = sinf(m_aTime*3.3f)*0.02f;
			effect[3*8+xx] = sinf(m_aTime*3.1f)*0.02f;  // bras gauche
			effect[3*9+xx] = sinf(m_aTime*3.5f)*0.02f;
			SpeedAdapt(effect, linVib, cirVib, event.rTime);
			time *= 5.0f;
		}
	}

	if ( type == OBJECT_STARTER )
	{
		factor = 0.2f;
		if ( action == MB_FLIC )
		{
			if ( m_starterTime >= 4.0f )
			{
				effect[3*3+zz] = cosf((m_starterTime-2.5f)*PI*2.0f)*0.04f;  // tête
				effect[3*6+zz] = cosf((m_starterTime-2.5f)*PI*2.0f)*0.40f;  // bras droite
				factor = 0.1f;
			}
		}
		if ( action == MB_HOME1 )
		{
			effect[3*7+xx] = sinf(m_aTime*PI*2.0f)*0.5f;  // bras droite
			effect[3*7+zz] = cosf(m_aTime*PI*2.0f)*0.5f;
			effect[3*9+xx] = sinf(m_aTime*PI*2.0f)*0.5f;  // bras gauche
			effect[3*9+zz] = cosf(m_aTime*PI*2.0f)*0.5f;
			factor = 2.0f;
		}

		effect[3*1+xx] += sinf(m_aTime*PI*2.0f)*0.03f*factor;  // jambe et corps
		effect[3*2+xx] += sinf(m_aTime*PI*2.0f)*0.03f*factor;
		effect[3*1+zz] += cosf(m_aTime*PI*2.0f)*0.03f*factor;
		effect[3*2+zz] += cosf(m_aTime*PI*2.0f)*0.03f*factor;

		effect[3*2+yy] += sinf(m_aTime*PI*2.0f)*0.06f*factor;  // corps
		effect[3*3+xx] += sinf(m_aTime*PI*2.0f)*0.04f*factor;  // tête
		effect[3*3+yy] += sinf(m_aTime*PI*2.0f)*0.10f*factor;
		effect[3*6+xx] += sinf(m_aTime*PI*2.0f)*0.02f;  // bras droite
		effect[3*7+xx] += sinf(m_aTime*PI*2.0f)*0.02f;
		effect[3*8+xx] += sinf(m_aTime*PI*2.0f)*0.02f;  // bras gauche
		effect[3*9+xx] += sinf(m_aTime*PI*2.0f)*0.02f;

		time *= 5.0f;
	}

	if ( type == OBJECT_WALKER )
	{
		s = m_physics->RetLinMotionX(MO_REASPEED)/m_physics->RetLinMotionX(MO_ADVSPEED);
		if ( s != 0.0f )
		{
			m_walkTime += event.rTime;
			prog = Mod(m_walkTime, 1.0f);

			cirVib.x = -cosf(prog*PI*2.0f)*0.2f;
			cirVib.y =  sinf(prog*PI*2.0f)*0.2f;
			linVib.z = -tanf(cirVib.x)*0.5f;
			linVib.y =  sinf(prog*PI*4.0f)*0.5f;

			if ( prog < 0.25f )
			{
				is = MB_WALK1;
				ie = MB_WALK2;
				prog = prog/0.25f;
				cirVib.z = -(1.0f-prog)*0.4f;
			}
			else if ( prog < 0.50f )
			{
				is = MB_WALK2;
				ie = MB_WALK3;
				prog = (prog-0.25f)/0.25f;
				cirVib.z = -prog*0.4f;
			}
			else if ( prog < 0.75f )
			{
				is = MB_WALK3;
				ie = MB_WALK4;
				prog = (prog-0.50f)/0.25f;
				cirVib.z = -(1.0f-prog)*0.4f;
			}
			else
			{
				is = MB_WALK4;
				ie = MB_WALK1;
				prog = (prog-0.75f)/0.25f;
				cirVib.z = -prog*0.4f;
			}

			for ( i=1 ; i<max ; i++ )
			{
				m_object->SetAngleX(i, Prop(m_armAngles[is*3*max+i*3+xx], m_armAngles[ie*3*max+i*3+xx], prog));
				m_object->SetAngleY(i, Prop(m_armAngles[is*3*max+i*3+yy], m_armAngles[ie*3*max+i*3+yy], prog));
				m_object->SetAngleZ(i, Prop(m_armAngles[is*3*max+i*3+zz], m_armAngles[ie*3*max+i*3+zz], prog));
			}

			pos = m_object->RetLinVibration();
			linVib.x = SmoothA(pos.x, linVib.x, event.rTime*12.0f);
			linVib.y = SmoothA(pos.y, linVib.y, event.rTime*12.0f);
			linVib.z = SmoothA(pos.z, linVib.z, event.rTime*12.0f);

			pos = m_object->RetCirVibration();
			cirVib.x = SmoothA(pos.x, cirVib.x, event.rTime*12.0f);
			cirVib.y = SmoothA(pos.y, cirVib.y, event.rTime*12.0f);
			cirVib.z = SmoothA(pos.z, cirVib.z, event.rTime*12.0f);

			m_object->SetLinVibration(linVib);
			m_object->SetCirVibration(cirVib);
			return TRUE;
		}
		m_walkTime = 0.0f;
		
		if ( m_actionType == MB_WAIT )
		{
			float head[] = {0.0f, 0.0f, 0.0f, 0.4f, 0.0f, 0.0f, 0.0f, -0.4f};
			effect[3* 2+yy] = head[(int)(m_aTime*2.0f)%8];

			effect[3* 9+xx] = -powf(Abs( sinf(m_aTime*1.0f)), 0.5f)*0.08f;
			effect[3*10+xx] =  powf(Abs(-sinf(m_aTime*1.0f)), 0.5f)*0.08f;
			effect[3*13+xx] =  powf(Abs( sinf(m_aTime*1.0f)), 0.5f)*0.08f;
			effect[3*14+xx] = -powf(Abs(-sinf(m_aTime*1.0f)), 0.5f)*0.08f;
			effect[3* 9+zz] =  sinf(m_aTime*0.5f)*0.04f;
			effect[3*13+zz] = -sinf(m_aTime*0.5f)*0.04f;
			SpeedAdapt(effect, linVib, cirVib, event.rTime);
		}

		if ( m_actionType == MB_TRUCK )
		{
			effect[3* 1+yy] =  sinf(m_aTime*10.0f)*0.20f;  // coup
			effect[3* 2+yy] =  sinf(m_aTime*10.0f)*0.20f;  // tête
			cirVib.y = -sinf(m_aTime*10.0f)*0.40f;

			effect[3* 3+zz] =  sinf(m_aTime*10.0f)*0.12f;  // jambe droite
			effect[3* 4+zz] = -sinf(m_aTime*10.0f)*0.24f;
			effect[3* 5+zz] =  sinf(m_aTime*10.0f)*0.12f;
			effect[3* 6+zz] =  cosf(m_aTime*10.0f)*0.12f;  // jambe gauche
			effect[3* 7+zz] = -cosf(m_aTime*10.0f)*0.24f;
			effect[3* 8+zz] =  cosf(m_aTime*10.0f)*0.12f;
			effect[3* 9+xx] =  sinf(m_aTime*10.0f)*0.12f;  // bras droite
			effect[3*10+xx] = -sinf(m_aTime*10.0f)*0.24f;
			effect[3*13+xx] = -cosf(m_aTime*10.0f)*0.12f;  // bras gauche
			effect[3*14+xx] =  cosf(m_aTime*10.0f)*0.24f;
		}

		if ( m_actionType == MB_FEAR )
		{
			SpeedAdapt(effect, linVib, cirVib, event.rTime);
			if ( m_progress >= 1.0f )
			{
				SetAction(MB_WAIT);
			}
		}

		if ( m_actionType == MB_HOME2 )  // transporté dans véhicule ?
		{
			SpeedAdapt(effect, linVib, cirVib, event.rTime);
		}

		time *= 5.0f;

		if ( m_actionType == MB_WAIT )
		{
			pos = m_object->RetLinVibration();
			linVib.x = SmoothA(pos.x, linVib.x, event.rTime*2.0f);
			linVib.y = SmoothA(pos.y, linVib.y, event.rTime*2.0f);
			linVib.z = SmoothA(pos.z, linVib.z, event.rTime*2.0f);

			pos = m_object->RetCirVibration();
			cirVib.x = SmoothA(pos.x, cirVib.x, event.rTime*2.0f);
			cirVib.y = SmoothA(pos.y, cirVib.y, event.rTime*2.0f);
			cirVib.z = SmoothA(pos.z, cirVib.z, event.rTime*2.0f);
		}
	}

	if ( type == OBJECT_CRAZY )
	{
		s = m_physics->RetLinMotionX(MO_REASPEED)/m_physics->RetLinMotionX(MO_ADVSPEED);
		if ( s != 0.0f )
		{
			m_walkTime += event.rTime;

			if ( m_option == 0 )
			{
				prog = Mod(m_walkTime, 1.0f);

				cirVib.x = -cosf(prog*PI*2.0f)*0.2f;
				cirVib.y =  sinf(prog*PI*2.0f)*0.2f;
				linVib.z = -tanf(cirVib.x)*0.5f;
				linVib.y =  sinf(prog*PI*4.0f)*0.5f;

				if ( prog < 0.25f )
				{
					is = MB_WALK1;
					ie = MB_WALK2;
					prog = prog/0.25f;
					cirVib.z = -(1.0f-prog)*0.4f;
				}
				else if ( prog < 0.50f )
				{
					is = MB_WALK2;
					ie = MB_WALK3;
					prog = (prog-0.25f)/0.25f;
					cirVib.z = -prog*0.4f;
				}
				else if ( prog < 0.75f )
				{
					is = MB_WALK3;
					ie = MB_WALK4;
					prog = (prog-0.50f)/0.25f;
					cirVib.z = -(1.0f-prog)*0.4f;
				}
				else
				{
					is = MB_WALK4;
					ie = MB_WALK1;
					prog = (prog-0.75f)/0.25f;
					cirVib.z = -prog*0.4f;
				}
			}
			if ( m_option == 1 )  // fou ?
			{
				prog = Mod(m_walkTime, 1.2f)/1.2f;

				cirVib.x = -cosf(prog*PI*2.0f)*0.1f;
				cirVib.y =  sinf(prog*PI*2.0f)*0.4f;
				linVib.z = -tanf(cirVib.x)*0.5f;
				linVib.y =  sinf(prog*PI*4.0f)*0.5f;

				if ( prog < 0.25f )
				{
					is = MB_WALK1;
					ie = MB_WALK2;
					prog = prog/0.25f;
					cirVib.z = -(1.0f-prog)*0.8f;
				}
				else if ( prog < 0.50f )
				{
					is = MB_WALK2;
					ie = MB_WALK3;
					prog = (prog-0.25f)/0.25f;
					cirVib.z = -prog*0.8f;
				}
				else if ( prog < 0.75f )
				{
					is = MB_WALK3;
					ie = MB_WALK4;
					prog = (prog-0.50f)/0.25f;
					cirVib.z = -(1.0f-prog)*0.8f;
				}
				else
				{
					is = MB_WALK4;
					ie = MB_WALK1;
					prog = (prog-0.75f)/0.25f;
					cirVib.z = -prog*0.8f;
				}
			}
			if ( m_option == 2 )  // bras croisés ?
			{
				prog = Mod(m_walkTime, 1.0f);

				cirVib.x = -cosf(prog*PI*2.0f)*0.2f;
				cirVib.y =  sinf(prog*PI*2.0f)*0.1f;
				linVib.z = -tanf(cirVib.x)*1.0f;
				linVib.y =  sinf(prog*PI*4.0f)*1.0f;

				if ( prog < 0.25f )
				{
					is = MB_WALK1;
					ie = MB_WALK2;
					prog = prog/0.25f;
					cirVib.z = -(1.0f-prog)*0.2f;
				}
				else if ( prog < 0.50f )
				{
					is = MB_WALK2;
					ie = MB_WALK3;
					prog = (prog-0.25f)/0.25f;
					cirVib.z = -prog*0.2f;
				}
				else if ( prog < 0.75f )
				{
					is = MB_WALK3;
					ie = MB_WALK4;
					prog = (prog-0.50f)/0.25f;
					cirVib.z = -(1.0f-prog)*0.2f;
				}
				else
				{
					is = MB_WALK4;
					ie = MB_WALK1;
					prog = (prog-0.75f)/0.25f;
					cirVib.z = -prog*0.2f;
				}
			}
			if ( m_option == 3 )  // ss ?
			{
				prog = Mod(m_walkTime, 1.4f)/1.4f;

//?				cirVib.x = -cosf(prog*PI*2.0f)*0.05f;
//?				cirVib.y =  sinf(prog*PI*2.0f)*0.05f;
//?				linVib.z = -tanf(cirVib.x)*0.1f;
				linVib.y =  sinf(prog*PI*4.0f)*0.1f;
				linVib.x =  sinf(prog*PI*4.0f)*1.0f;

				if ( prog < 0.25f )
				{
					is = MB_WALK1;
					ie = MB_WALK2;
					prog = 1.0f;
					cirVib.z = 0.4f;
				}
				else if ( prog < 0.50f )
				{
					is = MB_WALK2;
					ie = MB_WALK3;
					prog = 1.0f;
					cirVib.z = 0.0f;
				}
				else if ( prog < 0.75f )
				{
					is = MB_WALK3;
					ie = MB_WALK4;
					prog = 1.0f;
					cirVib.z = 0.4f;
				}
				else
				{
					is = MB_WALK4;
					ie = MB_WALK1;
					prog = 1.0f;
					cirVib.z = 0.0f;
				}
			}
			if ( m_option == 4 )  // random ?
			{
				prog = Mod(m_walkTime, 1.0f);

				cirVib.x = -cosf(prog*PI*2.0f)*0.2f;
				cirVib.y =  sinf(prog*PI*2.0f)*0.2f;
				linVib.z = -tanf(cirVib.x)*0.5f;
				linVib.y =  sinf(prog*PI*4.0f)*0.5f;

				if ( prog < 0.25f )
				{
					is = MB_WALK1;
					ie = MB_WALK2;
					prog = prog/0.25f;
					cirVib.z = -(1.0f-prog)*0.4f;
				}
				else if ( prog < 0.50f )
				{
					is = MB_WALK2;
					ie = MB_WALK3;
					prog = (prog-0.25f)/0.25f;
					cirVib.z = -prog*0.4f;
				}
				else if ( prog < 0.75f )
				{
					is = MB_WALK3;
					ie = MB_WALK4;
					prog = (prog-0.50f)/0.25f;
					cirVib.z = -(1.0f-prog)*0.4f;
				}
				else
				{
					is = MB_WALK4;
					ie = MB_WALK1;
					prog = (prog-0.75f)/0.25f;
					cirVib.z = -prog*0.4f;
				}
			}

			time *= 4.0f;
			for ( i=1 ; i<max ; i++ )
			{
				m_object->SetAngleX(i, SmoothA(m_object->RetAngleX(i), Prop(m_armAngles[is*3*max+i*3+xx], m_armAngles[ie*3*max+i*3+xx], prog), time));
				m_object->SetAngleY(i, SmoothA(m_object->RetAngleY(i), Prop(m_armAngles[is*3*max+i*3+yy], m_armAngles[ie*3*max+i*3+yy], prog), time));
				m_object->SetAngleZ(i, SmoothA(m_object->RetAngleZ(i), Prop(m_armAngles[is*3*max+i*3+zz], m_armAngles[ie*3*max+i*3+zz], prog), time));
			}

			rot = RotatePoint(-m_object->RetAngleY(0), FPOINT(linVib.x, linVib.z));
			linVib.x = rot.x;
			linVib.z = rot.y;
			pos = m_object->RetLinVibration();
			linVib.x = SmoothA(pos.x, linVib.x, event.rTime*12.0f);
			linVib.y = SmoothA(pos.y, linVib.y, event.rTime*12.0f);
			linVib.z = SmoothA(pos.z, linVib.z, event.rTime*12.0f);

			pos = m_object->RetCirVibration();
			cirVib.x = SmoothA(pos.x, cirVib.x, event.rTime*12.0f);
			cirVib.y = SmoothA(pos.y, cirVib.y, event.rTime*12.0f);
			cirVib.z = SmoothA(pos.z, cirVib.z, event.rTime*12.0f);

			m_object->SetLinVibration(linVib);
			m_object->SetCirVibration(cirVib);
			return TRUE;
		}
		m_walkTime = 0.0f;
		
		brain = m_object->RetBrain();
		if ( brain != 0 && brain->IsProgram() )
		{
			action = MB_WAIT;
		}
		else
		{
			action = MB_TRUCK;
		}

		if ( action == MB_WAIT )  // demi-tour ?
		{
			if ( m_option == 4 )  // random ?
			{
				effect[3* 1+yy] =  sinf(m_aTime*40.0f)*0.20f;  // coup
				effect[3* 1+zz] =  sinf(m_aTime*37.0f)*0.20f;
				effect[3* 2+yy] =  sinf(m_aTime*46.0f)*0.20f;  // tête
				effect[3* 2+zz] =  sinf(m_aTime*43.0f)*0.20f;

				effect[3* 9+xx] =  sinf(m_aTime*42.0f)*0.20f;  // bras droite
				effect[3* 9+yy] =  sinf(m_aTime*47.0f)*0.20f;
				effect[3* 9+zz] =  sinf(m_aTime*39.0f)*0.20f;
				effect[3*10+xx] =  sinf(m_aTime*40.0f)*0.20f;
				effect[3*10+yy] =  sinf(m_aTime*37.0f)*0.20f;
				effect[3*10+zz] =  sinf(m_aTime*41.0f)*0.20f;
				effect[3*11+xx] =  sinf(m_aTime*49.0f)*0.20f;
				effect[3*11+yy] =  sinf(m_aTime*35.0f)*0.20f;
				effect[3*11+zz] =  sinf(m_aTime*43.0f)*0.20f;

				effect[3*12+xx] =  sinf(m_aTime*38.0f)*0.20f;  // bras gauche
				effect[3*12+yy] =  sinf(m_aTime*43.0f)*0.20f;
				effect[3*12+zz] =  sinf(m_aTime*40.0f)*0.20f;
				effect[3*13+xx] =  sinf(m_aTime*35.0f)*0.20f;
				effect[3*13+yy] =  sinf(m_aTime*39.0f)*0.20f;
				effect[3*13+zz] =  sinf(m_aTime*49.0f)*0.20f;
				effect[3*14+xx] =  sinf(m_aTime*41.0f)*0.20f;
				effect[3*14+yy] =  sinf(m_aTime*36.0f)*0.20f;
				effect[3*14+zz] =  sinf(m_aTime*44.0f)*0.20f;
			}
			else if ( m_option == 1 )  // fou ?
			{
				effect[3* 1+yy] =  sinf(m_aTime*10.0f)*0.20f;  // coup
				effect[3* 2+yy] =  sinf(m_aTime*10.0f)*0.20f;  // tête
				cirVib.y = -sinf(m_aTime*10.0f)*0.40f;
				cirVib.x = 20.0f*PI/180.0f;  // penche

				effect[3* 3+zz] =  sinf(m_aTime*10.0f)*0.20f;  // jambe droite
				effect[3* 4+zz] = -sinf(m_aTime*10.0f)*0.40f;
				effect[3* 5+zz] =  sinf(m_aTime*10.0f)*0.20f;

				effect[3* 9+xx] =  sinf(m_aTime*20.0f)*0.4f;  // bras droite
				effect[3* 9+yy] =  cosf(m_aTime*20.0f)*0.4f;
				effect[3*10+xx] =  sinf(m_aTime*20.0f)*0.4f;  // avant-bras droite
				effect[3*10+yy] =  cosf(m_aTime*20.0f)*0.4f;
				effect[3*12+xx] = -sinf(m_aTime*20.0f)*0.4f;  // bras gauche
				effect[3*12+yy] = -cosf(m_aTime*20.0f)*0.4f;
				effect[3*13+xx] = -sinf(m_aTime*20.0f)*0.4f;  // avant-bras gauche
				effect[3*13+yy] = -cosf(m_aTime*20.0f)*0.4f;
			}
			else
			{
				effect[3* 1+yy] =  sinf(m_aTime*10.0f)*0.20f;  // coup
				effect[3* 2+yy] =  sinf(m_aTime*10.0f)*0.20f;  // tête
				cirVib.y = -sinf(m_aTime*10.0f)*0.40f;

				effect[3* 3+zz] =  sinf(m_aTime*10.0f)*0.12f;  // jambe droite
				effect[3* 4+zz] = -sinf(m_aTime*10.0f)*0.24f;
				effect[3* 5+zz] =  sinf(m_aTime*10.0f)*0.12f;
				effect[3* 6+zz] =  cosf(m_aTime*10.0f)*0.12f;  // jambe gauche
				effect[3* 7+zz] = -cosf(m_aTime*10.0f)*0.24f;
				effect[3* 8+zz] =  cosf(m_aTime*10.0f)*0.12f;
			}
		}

		if ( action == MB_TRUCK )  // attend ?
		{
			float head[] = {0.0f, 0.0f, 0.0f, 0.4f, 0.0f, 0.0f, 0.0f, -0.4f};
			effect[3* 2+yy] = head[(int)(m_aTime*2.0f)%8];

			effect[3* 9+xx] = -powf(Abs( sinf(m_aTime*1.0f)), 0.5f)*0.08f;
			effect[3*10+xx] =  powf(Abs(-sinf(m_aTime*1.0f)), 0.5f)*0.08f;
			effect[3*13+xx] =  powf(Abs( sinf(m_aTime*1.0f)), 0.5f)*0.08f;
			effect[3*14+xx] = -powf(Abs(-sinf(m_aTime*1.0f)), 0.5f)*0.08f;
			effect[3* 9+zz] =  sinf(m_aTime*0.5f)*0.04f;
			effect[3*13+zz] = -sinf(m_aTime*0.5f)*0.04f;
			SpeedAdapt(effect, linVib, cirVib, event.rTime);
		}

		time *= 5.0f;

		if ( action == MB_WAIT )  // demi-tour ?
		{
			pos = m_object->RetLinVibration();
			linVib.x = SmoothA(pos.x, linVib.x, event.rTime*2.0f);
			linVib.y = SmoothA(pos.y, linVib.y, event.rTime*2.0f);
			linVib.z = SmoothA(pos.z, linVib.z, event.rTime*2.0f);

			pos = m_object->RetCirVibration();
			cirVib.x = SmoothA(pos.x, cirVib.x, event.rTime*2.0f);
			cirVib.y = SmoothA(pos.y, cirVib.y, event.rTime*2.0f);
			cirVib.z = SmoothA(pos.z, cirVib.z, event.rTime*2.0f);
		}
	}

	if ( type == OBJECT_GUIDE )
	{
		factor = 0.2f;
		if ( action == MB_FEAR )
		{
			effect[3*3+yy] = sinf(m_aTime*12.0f)*0.06f;
		}

		if ( action == MB_FLIC )
		{
			effect[3*3+yy] = 0.5f;  // tête
			effect[3*3+zz] = cosf((m_aTime*1.0f)*PI*2.0f)*0.20f;  // tête
			effect[3*8+xx] = cosf((m_aTime*1.0f)*PI*2.0f)*0.20f;  // bras droite
			effect[3*9+xx] = cosf((m_aTime*1.0f)*PI*2.0f)*0.70f;  // bras droite
			factor = 0.5f;
		}

		effect[3*1+xx] += sinf(m_aTime*PI*2.0f)*0.03f*factor;  // jambe et corps
		effect[3*2+xx] += sinf(m_aTime*PI*2.0f)*0.03f*factor;
		effect[3*1+zz] += cosf(m_aTime*PI*2.0f)*0.03f*factor;
		effect[3*2+zz] += cosf(m_aTime*PI*2.0f)*0.03f*factor;

		effect[3*2+yy] += sinf(m_aTime*PI*2.0f)*0.06f*factor;  // corps
		effect[3*3+xx] += sinf(m_aTime*PI*2.0f)*0.04f*factor;  // tête
		effect[3*3+yy] += sinf(m_aTime*PI*2.0f)*0.10f*factor;
		effect[3*6+xx] += sinf(m_aTime*PI*2.0f)*0.02f;  // bras droite
		effect[3*7+xx] += sinf(m_aTime*PI*2.0f)*0.02f;
		effect[3*8+xx] += sinf(m_aTime*PI*2.0f)*0.02f;  // bras gauche
		effect[3*9+xx] += sinf(m_aTime*PI*2.0f)*0.02f;

		time *= 5.0f;
	}

	j = action;
	if ( j <  0      )  j = 0;
	if ( j >= MB_MAX )  j = MB_MAX-1;

	for ( i=1 ; i<max ; i++ )
	{
//?		m_object->SetAngleX(i, SmoothA(m_object->RetAngleX(i), m_armAngles[i*3+xx+j*3*max]*PI/180.0f, time)+effect[i*3+xx]);
//?		m_object->SetAngleY(i, SmoothA(m_object->RetAngleY(i), m_armAngles[i*3+yy+j*3*max]*PI/180.0f, time)+effect[i*3+yy]);
//?		m_object->SetAngleZ(i, SmoothA(m_object->RetAngleZ(i), m_armAngles[i*3+zz+j*3*max]*PI/180.0f, time)+effect[i*3+zz]);
		m_object->SetAngleX(i, SmoothA(m_object->RetAngleX(i), m_armAngles[i*3+xx+j*3*max]*PI/180.0f+effect[i*3+xx], time));
		m_object->SetAngleY(i, SmoothA(m_object->RetAngleY(i), m_armAngles[i*3+yy+j*3*max]*PI/180.0f+effect[i*3+yy], time));
		m_object->SetAngleZ(i, SmoothA(m_object->RetAngleZ(i), m_armAngles[i*3+zz+j*3*max]*PI/180.0f+effect[i*3+zz], time));
//?		m_object->SetAngleX(i, m_armAngles[i*3+xx+j*3*max]*PI/180.0f+effect[i*3+xx]);
//?		m_object->SetAngleY(i, m_armAngles[i*3+yy+j*3*max]*PI/180.0f+effect[i*3+yy]);
//?		m_object->SetAngleZ(i, m_armAngles[i*3+zz+j*3*max]*PI/180.0f+effect[i*3+zz]);
	}

#if 0
	// Si le robot est transporté par un véhicule et que la caméra
	// est embarquée, met-le loin derrière pour ne pas le voir.
	if ( m_object->RetTruck() != 0 &&
		 m_camera->RetType() == CAMERA_ONBOARD )
	{
		linVib.x -= 10.0f;
	}
#endif

	if ( type == OBJECT_BOT2 )
	{
		if ( m_actionType == MB_WAIT  ||
			 m_actionType == MB_FEAR  ||
			 m_actionType == MB_HOME1 ||
			 m_actionType == MB_HOME2 )
		{
			// Rotation du corps en fonction de l'angle des jambes et
			// des pieds pour que les pieds restent plaqués au sol.
			angle = m_object->RetAngleZ(2)+m_object->RetAngleZ(3);
			cirVib.z = -angle;
		}
	}

	m_object->SetLinVibration(linVib);
	m_object->SetCirVibration(cirVib);

	if ( type == OBJECT_GUIDE )
	{
		if ( m_partiGuide != -1 )
		{
			mat = m_object->RetWorldMatrix(9);
			pos = Transform(*mat, D3DVECTOR(0.4f, 1.9f, 2.2f));
			m_particule->SetPosition(m_partiGuide, pos);
		}
	}

	return TRUE;
}

// Adapte le robot en fonction de la vitesse du transporteur.

void CMotionBot::SpeedAdapt(float effect[],
							D3DVECTOR &linVib, D3DVECTOR &cirVib,
							float rTime)
{
	ObjectType	type, tType;
	CObject*	vehicle;
	CPhysics*	physics;
	CMotion*	motion;
	float		lin, cir, factor, top;

	if ( m_bBreak )  // cassé ?
	{
		vehicle = m_object->RetTruck();
		if ( vehicle == 0 )  return;
		if ( vehicle->RetType() != OBJECT_HOOK )  return;
		vehicle = vehicle->RetTruck();
		if ( vehicle == 0 )  return;
		physics = vehicle->RetPhysics();
		if ( physics == 0 )  return;
		motion = vehicle->RetMotion();
		if ( motion == 0 )  return;
	}
	else
	{
		vehicle = m_object->RetTruck();
		if ( vehicle == 0 )  return;
		physics = vehicle->RetPhysics();
		if ( physics == 0 )  return;
		motion = vehicle->RetMotion();
		if ( motion == 0 )  return;
	}

	lin = NormSign(physics->RetLinMotionX(MO_REAACCEL)/50.0f);
	cir = NormSign(physics->RetCirMotionY(MO_REASPEED)/physics->RetCirMotionY(MO_ADVSPEED));

	top = 1.0f;
	if ( !motion->ExistPart(TP_TOP) )  top = 2.0f;  // + si pas de toît

	type = m_object->RetType();
	tType = vehicle->RetType();

	if ( type == OBJECT_BOT1 )
	{
		if ( m_actionType == MB_WALK2 )  // transporté cassé ?
		{
			effect[3*3+xx] += cir*1.00f;  // tête
			effect[3*3+yy] += cir*0.50f;
			effect[3*6+xx] += cir*1.00f;  // bras droite
			effect[3*6+zz] -= cir*1.00f;
			effect[3*8+xx] -= cir*0.50f;  // bras gauche (proche voiture)
			effect[3*8+zz] -= cir*0.50f;
		}
		else
		{
			// Diminue certains mouvements plus on va vite.
			factor = 1.0f-Norm(Max(Abs(lin), Abs(cir))*1.5f);
			effect[3*1+xx] *= factor;
			effect[3*2+xx] *= factor;
			effect[3*1+zz] *= factor;
			effect[3*2+zz] *= factor;  // jambe et corps

			effect[3*3+xx] *= factor;  // tête
			effect[3*3+yy] *= factor;

			effect[3*1+zz] += lin*0.04f*top;  // jambe
			effect[3*2+zz] += lin*0.04f*top;  // corps
			effect[3*4+zz] += lin*0.15f;  // antenne droite
			effect[3*5+zz] += lin*0.15f;  // antenne gauche

			effect[3*1+xx] += cir*0.04f*top;  // jambe
			effect[3*2+xx] += cir*0.04f*top;  // corps
			effect[3*2+yy] -= cir*0.10f;
			effect[3*3+yy] -= cir*0.15f;  // tête
			effect[3*4+xx] += cir*0.15f;  // antenne droite
			effect[3*5+xx] += cir*0.15f;  // antenne gauche
		}
	}

	if ( type == OBJECT_BOT2 )
	{
		effect[3*2+zz] += lin*0.04f*top;  // jambe droite
		effect[3*4+zz] += lin*0.04f*top;  // jambe gauche
	}

	if ( type == OBJECT_BOT3 )
	{
		effect[3*3+zz] -= lin*0.03f*top;
		effect[3*6+zz] -= lin*0.03f*top;

		if ( cir > 0.0f )
		{
			effect[3*1+zz] += cir*0.20f*top;
			effect[3*2+zz] -= cir*0.40f*top;
			effect[3*3+zz] += cir*0.20f*top;
		}
		else
		{
			effect[3*4+zz] -= cir*0.20f*top;
			effect[3*5+zz] += cir*0.40f*top;
			effect[3*6+zz] -= cir*0.20f*top;
		}

		m_cirVib.z = SmoothA(m_cirVib.z,  lin*0.3f*top, rTime*2.0f*top);
		m_cirVib.x = SmoothA(m_cirVib.x, -cir*0.6f*top, rTime*2.0f*top);
		if ( m_cirVib.z >  0.2f )  m_cirVib.z =  0.2f;
		if ( m_cirVib.z < -0.2f )  m_cirVib.z = -0.2f;
		if ( m_cirVib.x >  0.2f )  m_cirVib.x =  0.2f;
		if ( m_cirVib.x < -0.2f )  m_cirVib.x = -0.2f;
		cirVib += m_cirVib;

		linVib.y -= tanf(Abs(cirVib.x)+Abs(cirVib.z))*0.7f;
	}

	if ( type == OBJECT_BOT4 )
	{
		effect[3*2+yy] += cir*0.60f;
	}

	if ( type == OBJECT_EVIL1 ||
		 type == OBJECT_EVIL2 ||
		 type == OBJECT_EVIL3 )
	{
		if ( tType == OBJECT_TRAX )
		{
			m_cirVib.z = SmoothA(m_cirVib.z, lin*0.4f, rTime*1.0f);
			m_cirVib.x = SmoothA(m_cirVib.x, cir*0.4f, rTime*1.0f);
		}
		else
		{
			m_cirVib.z = SmoothA(m_cirVib.z, lin*0.2f*top, rTime*2.0f*top);
			m_cirVib.x = SmoothA(m_cirVib.x, cir*0.2f*top, rTime*2.0f*top);
		}
		if ( m_cirVib.z >  0.2f )  m_cirVib.z =  0.2f;
		if ( m_cirVib.z < -0.2f )  m_cirVib.z = -0.2f;
		if ( m_cirVib.x >  0.2f )  m_cirVib.x =  0.2f;
		if ( m_cirVib.x < -0.2f )  m_cirVib.x = -0.2f;
		cirVib = m_cirVib;
	}
}

// Fait feu avec le robot canon #2.

void CMotionBot::FireBot2()
{
	D3DMATRIX*	mat;
	D3DVECTOR	pos, speed;
	FPOINT		dim;
	int			i, channel;

	if ( m_lastSound <= 0.0f )
	{
		m_lastSound = Rand()*0.4f+0.4f;
		m_sound->Play(SOUND_FIREp, m_object->RetPosition(0));
	}

	if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_aTime )
	{
		m_lastParticule = m_aTime;

		for ( i=0 ; i<4 ; i++ )
		{
			mat = m_object->RetWorldMatrix(7+2*(i/2));  // canon droite/gauche
			pos = D3DVECTOR(1.2f, 0.6f, (i/2==0)?-0.5f:0.5f);
			pos = Transform(*mat, pos);

			speed = D3DVECTOR(200.0f, 0.0f, 0.0f);
			speed.x += (Rand()-0.5f)*12.0f;
			speed.y += (Rand()-0.5f)*24.0f;
			speed.z += (Rand()-0.5f)*24.0f;
			speed = Transform(*mat, speed);
			speed -= pos;

			dim.x = 1.0f;
			dim.y = dim.x;
			channel = m_particule->CreateTrack(pos, speed, dim, PARTITRACK11,
											   2.0f, 200.0f, 0.5f, 1.0f);
			m_particule->SetObjectFather(channel, m_object);

			speed = D3DVECTOR(5.0f, 0.0f, 0.0f);
			speed.x += (Rand()-0.5f)*1.0f;
			speed.y += (Rand()-0.5f)*2.0f;
			speed.z += (Rand()-0.5f)*2.0f;
			speed = Transform(*mat, speed);
			speed -= pos;
			speed.y += 5.0f;

			dim.x = 2.0f;
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTISMOKE2, 2.0f, 0.0f);
		}
	}
}

// Fait feu (moyeu) avec la trompe du méchant #1.

void CMotionBot::FireEvil1a()
{
	D3DMATRIX*	mat;
	D3DVECTOR	pos, speed;
	FPOINT		dim;
	int			channel;

	if ( m_lastParticule+0.05f <= m_aTime )
	{
		m_lastParticule = m_aTime;

		mat = m_object->RetWorldMatrix(0);
		pos = D3DVECTOR(0.6f, 6.7f, 0.0f);
		speed.x = Rand()*60.0f;
		speed.y = (Rand()-0.5f)*8.0f;
		speed.z = (Rand()-0.5f)*8.0f;
		speed += pos;
		pos = Transform(*mat, pos);
		speed = Transform(*mat, speed);
		speed -= pos;
		dim.x = 0.8f;
		dim.y = dim.x;
		channel = m_particule->CreateParticule(pos, speed, dim, PARTIGUN2, 2.0f, 100.0f);
		m_particule->SetObjectFather(channel, m_object);
	}
}

// Fait feu (très efficace) avec la trompe du méchant #1.

void CMotionBot::FireEvil1b()
{
	D3DVECTOR	pos, speed;
	FPOINT		dim, rot;
	int			channel, i;

	if ( m_lastParticule+0.05f <= m_aTime )
	{
		m_lastParticule = m_aTime;

		for ( i=0 ; i<5 ; i++ )
		{
			pos = m_object->RetPosition(0);
			pos.y += 6.7f;
			rot = RotatePoint(Rand()*PI*2.0f, 20.0f+Rand()*20.0f);
			speed.x = rot.x;
			speed.z = rot.y;
			speed.y = 15.0f+Rand()*15.0f;
			dim.x = 0.8f;
			dim.y = dim.x;
			channel = m_particule->CreateParticule(pos, speed, dim, PARTIGUN2, 2.0f, 100.0f);
			m_particule->SetObjectFather(channel, m_object);
		}
	}
}

// Bulles émises sous l'eau par le robot cassé.

void CMotionBot::BubbleBot1()
{
	D3DMATRIX*	mat;
	D3DVECTOR	pos, speed;
	FPOINT		dim;
	int			channel;

	if ( m_lastParticule+0.05f <= m_aTime )
	{
		m_lastParticule = m_aTime;

		mat = m_object->RetWorldMatrix(0);
		pos.x = (Rand()-0.5f)*1.0f;
		pos.z = (Rand()-0.5f)*1.0f;
		pos.y = 2.2f;
		pos = Transform(*mat, pos);
		speed.y = (Rand()-0.5f)*8.0f+8.0f;
		speed.x = (Rand()-0.5f)*0.2f;
		speed.z = (Rand()-0.5f)*0.2f;
		dim.x = Rand()*0.1f+0.2f;
		dim.y = dim.x;
		channel = m_particule->CreateParticule(pos, speed, dim, PARTIBUBBLE, 3.0f, 0.0f);
	}
}

// Met à jour le mapping de la texture du visage.

void CMotionBot::UpdateFaceMapping(int face)
{
	D3DMATERIAL7	mat;
	float			limit[4], au, bu;
	int				i;

	if ( face == m_face )  return;
	m_face = face;

	ZeroMemory( &mat, sizeof(D3DMATERIAL7) );
	mat.diffuse.r = 1.0f;
	mat.diffuse.g = 1.0f;
	mat.diffuse.b = 1.0f;  // blanc
	mat.ambient.r = 0.5f;
	mat.ambient.g = 0.5f;
	mat.ambient.b = 0.5f;

//?	limit[0] = 0.0f;
//?	limit[1] = m_engine->RetLimitLOD(0);
//?	limit[2] = limit[1];
//?	limit[3] = m_engine->RetLimitLOD(1);
	limit[0] = 0.0f;
	limit[1] = 1000000.0f;
	limit[2] = 0.0f;
	limit[3] = 1000000.0f;

	// faces des roues :
	au = 0.25f/2.0f/1.4f;
	bu = 0.25f/2.0f+face*0.25f;
	for ( i=0 ; i<1 ; i++ )
	{
		m_engine->ChangeTextureMapping(m_object->RetObjectRank(0),
									   mat, D3DSTATEPART1, "bot2.tga", "",
									   limit[i*2+0], limit[i*2+1], D3DMAPPING1Z,
									   au, bu, 1.0f, 0.0f);
	}
}

