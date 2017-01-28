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
#include "camera.h"
#include "modfile.h"
#include "sound.h"
#include "robotmain.h"
#include "mainundo.h"
#include "task.h"
#include "tasklist.h"
#include "motion.h"
#include "motionblupi.h"
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
	m_aTime = Rand()*10.0f;
	m_lastParticule = 0.0f;
	m_lastSound = 0.0f;
	m_walkTime = 0.0f;
	m_bFirstSound = FALSE;
	m_cirVib = D3DVECTOR(0.0f, 0.0f, 0.0f);
}

// Destructeur de l'objet.

CMotionBot::~CMotionBot()
{
}


// Supprime un objet.

void CMotionBot::DeleteObject(BOOL bAll)
{
}


// Démarre une action spéciale.

Error CMotionBot::SetAction(int action, float time)
{
	ObjectType	type;

	type = m_object->RetType();

	return CMotion::SetAction(action, time);
}


// Retourne la vitesse linéaire.

float CMotionBot::RetLinSpeed()
{
	return 10.0f;
}

// Retourne la vitesse linéaire.

float CMotionBot::RetCirSpeed()
{
	return 1.0f*PI;
}

// Retourne la distance linéaire de freinage.

float CMotionBot::RetLinStopLength()
{
	return 1.0f;
}


// Crée un robot quelconque posé sur le sol.

BOOL CMotionBot::Create(D3DVECTOR pos, float angle, ObjectType type)
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
	m_engine->SetObjectType(rank, TYPEOBJECT);  // c'est un objet fixe
	m_object->SetObjectRank(0, rank);
	if ( type == OBJECT_BOT1    )  pModFile->ReadModel("objects\\bot101.mod");
	if ( type == OBJECT_BOT2    )  pModFile->ReadModel("objects\\bot201.mod");
	if ( type == OBJECT_BOT4    )  pModFile->ReadModel("objects\\bot401.mod");
	if ( type == OBJECT_WALKER  )  pModFile->ReadModel("objects\\bot401.mod");
	if ( type == OBJECT_CRAZY   )  pModFile->ReadModel("objects\\bot401.mod");
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(0, pos);
	m_object->SetAngleY(0, angle);

	radius = 2.0f;

	if ( type == OBJECT_BOT1 )
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
		pModFile->ReadModel("objects\\bot107.mod");
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

	m_object->CreateLockZone(0, 0, LZ_BOT);
	m_object->CreateShadow(radius, 1.0f, D3DSHADOWNORM, TRUE, 1.0f);
	m_object->SetFloorHeight(0.0f);

	pos = m_object->RetPosition(0);
	m_object->SetPosition(0, pos);  // pour afficher les ombres tout de suite

//?	m_engine->LoadAllTexture();

	option = m_object->RetOption();
	if ( option == 0 )
	{
		SetAction(MBOT_WAIT);
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
							// MBOT_WAIT:
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
							// MBOT_YOUPIE:
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
							// MBOT_GOAL:
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
							// MBOT_TURN:
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
							// MBOT_WALK1:  (cassé au sol)
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
							// MBOT_WALK2:  (cassé transporté)
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
							// MBOT_WALK3:
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
							// MBOT_WALK4:
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

		for ( i=0 ; i<3*10*MBOT_MAX ; i++ )
		{
			m_armAngles[i] = member[i];
		}

		character->height = 3.5f;
	}

	if ( type == OBJECT_BOT2 )
	{
		short member[] =
		{
							// MBOT_WAIT:
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
							// MBOT_YOUPIE:
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
							// MBOT_GOAL:
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
							// MBOT_TURN:
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
							// MBOT_WALK1:
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
							// MBOT_WALK2:
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
							// MBOT_WALK3:
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
							// MBOT_WALK4:
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

		for ( i=0 ; i<3*10*MBOT_MAX ; i++ )
		{
			m_armAngles[i] = member[i];
		}
	}

	if ( type == OBJECT_BOT4 )
	{
		short member[] =
		{
							// MBOT_WAIT:
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
							// MBOT_YOUPIE:
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
							// MBOT_GOAL:
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
							// MBOT_TURN:
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
							// MBOT_WALK1:
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
							// MBOT_WALK2:
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
							// MBOT_WALK3:
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
							// MBOT_WALK4:
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

		for ( i=0 ; i<3*17*MBOT_MAX ; i++ )
		{
			m_armAngles[i] = member[i];
		}

		character->height = 3.8f;
	}

	if ( type == OBJECT_WALKER )
	{
		short member[] =
		{
							// MBOT_WAIT:
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
							// MBOT_YOUPIE:
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
							// MBOT_GOAL:
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
							// MBOT_TURN:
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
							// MBOT_WALK1:
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
							// MBOT_WALK2:
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
							// MBOT_WALK3:
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
							// MBOT_WALK4:
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

		for ( i=0 ; i<3*17*MBOT_MAX ; i++ )
		{
			m_armAngles[i] = member[i];
		}

		character->height = 3.8f;
	}

	if ( type == OBJECT_CRAZY )
	{
		short member[] =
		{
							// MBOT_WAIT:
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
							// MBOT_YOUPIE:
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0,  -5,  // tête
			  0,   0,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  20,  // pied droite
			  0,   0,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  20,  // pied gauche
			-30,   0,   0,  // bras droite
			140,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 30,   0,   0,  // bras gauche
		   -140,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MBOT_GOAL:
			  0,   0,   0,  // corps
			  0,   0,  10,  // coup
			  0,   0,   5,  // tête
			  0,   0,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,   0,  // pied gauche
			 60,  10,   0,  // bras droite
			 65,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			-60, -10,   0,  // bras gauche
			-65,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MBOT_TURN:
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0,  -5,  // tête
			  0,   0,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  20,  // pied droite
			  0,   0,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  20,  // pied gauche
			-15,  20,   0,  // bras droite
			-90, -30,  30,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			  0,   0,   0,  // bras gauche (tendu à l'horizontale)
			  0,   0,  30,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MBOT_WALK1:
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
							// MBOT_WALK2:
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
							// MBOT_WALK3:
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
							// MBOT_WALK4:
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
							// MBOT_WAIT:
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
							// MBOT_YOUPIE:
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0,  -5,  // tête
			  0,   0,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  20,  // pied droite
			  0,   0,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  20,  // pied gauche
			-30,   0,   0,  // bras droite
			140,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 30,   0,   0,  // bras gauche
		   -140,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MBOT_GOAL:
			  0,   0,   0,  // corps
			  0,   0,  10,  // coup
			  0,   0,   5,  // tête
			  0,   0,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,   0,  // pied gauche
			 60,  10,   0,  // bras droite
			 65,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			-60, -10,   0,  // bras gauche
			-65,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MBOT_TURN:
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
							// MBOT_WALK1:
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
							// MBOT_WALK2:
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
							// MBOT_WALK3:
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
							// MBOT_WALK4:
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
							// MBOT_WAIT:
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
							// MBOT_YOUPIE:
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0,  -5,  // tête
			  0,   0,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  20,  // pied droite
			  0,   0,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  20,  // pied gauche
			-30,   0,   0,  // bras droite
			140,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 30,   0,   0,  // bras gauche
		   -140,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MBOT_GOAL:
			  0,   0,   0,  // corps
			  0,   0,  10,  // coup
			  0,   0,   5,  // tête
			  0,   0,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,   0,  // pied gauche
			 60,  10,   0,  // bras droite
			 65,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			-60, -10,   0,  // bras gauche
			-65,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MBOT_TURN:
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
							// MBOT_WALK1:
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
							// MBOT_WALK2:
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
							// MBOT_WALK3:
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
							// MBOT_WALK4:
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
							// MBOT_WAIT:
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0,  -5,  // tête
			  0,   0, 100,  // jambe droite
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,  25,  // jambe gauche
			  0,   0, -45,  // jambe gauche
			-10,   0,  25,  // pied gauche
			-15,  20,   0,  // bras droite (bras écartés)
			-90, -30,  30,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 15, -20,   0,  // bras gauche
			 90,  30,  30,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MBOT_YOUPIE:
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0,  -5,  // tête
			  0,   0,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  20,  // pied droite
			  0,   0,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  20,  // pied gauche
			-30,   0,   0,  // bras droite
			140,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 30,   0,   0,  // bras gauche
		   -140,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MBOT_GOAL:
			  0,   0,   0,  // corps
			  0,   0,  10,  // coup
			  0,   0,   5,  // tête
			  0,   0,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,   0,  // pied gauche
			 60,  10,   0,  // bras droite
			 65,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			-60, -10,   0,  // bras gauche
			-65,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MBOT_TURN:
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0,  -5,  // tête
			  0,   0, 100,  // jambe droite
			  0,   0,   0,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,  25,  // jambe gauche
			  0,   0, -45,  // jambe gauche
			-10,   0,  25,  // pied gauche
			-15,  20,   0,  // bras droite (bras écartés)
			-90, -30,  30,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 15, -20,   0,  // bras gauche
			 90,  30,  30,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MBOT_WALK1:
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
							// MBOT_WALK2:
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
							// MBOT_WALK3:
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
							// MBOT_WALK4:
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
							// MBOT_WAIT:
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
							// MBOT_YOUPIE:
			  0,   0,   0,  // corps
			  0,   0, -10,  // coup
			  0,   0,  -5,  // tête
			  0,   0,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  20,  // pied droite
			  0,   0,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  20,  // pied gauche
			-30,   0,   0,  // bras droite
			140,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 30,   0,   0,  // bras gauche
		   -140,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MBOT_GOAL:
			  0,   0,   0,  // corps
			  0,   0,  10,  // coup
			  0,   0,   5,  // tête
			  0,   0,  20,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,   0,  // pied droite
			  0,   0,  20,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,   0,  // pied gauche
			 60,  10,   0,  // bras droite
			 65,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			-60, -10,   0,  // bras gauche
			-65,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MBOT_TURN:
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
							// MBOT_WALK1:
			  0,   0,   0,  // corps
			-10,   0, -10,  // coup
			-10,   0, -10,  // tête
			  0,   0,   0,  // jambe droite
			  0,   0, -40,  // jambe droite
			  0,   0,  60,  // pied droite
			  0,   0,  85,  // jambe gauche
			  0,   0, -65,  // jambe gauche
			  0,   0,  15,  // pied gauche
			-15,   0, -30,  // bras droite
			-65,   0,   0,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 15,   0,  30,  // bras gauche
			 65,   0,  80,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MBOT_WALK2:
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
							// MBOT_WALK3:
			  0,   0,   0,  // corps
			 10,   0, -10,  // coup
			 10,   0, -10,  // tête
			  0,   0,  85,  // jambe droite
			  0,   0, -65,  // jambe droite
			  0,   0,  15,  // pied droite
			  0,   0,   0,  // jambe gauche
			  0,   0, -40,  // jambe gauche
			  0,   0,  60,  // pied gauche
			-15,   0,  30,  // bras droite
			-65,   0,  80,  // bras droite
			  0,   0,   0,  // main droite sup
			  0,   0,   0,  // main droite inf
			 15,   0, -30,  // bras gauche
			 65,   0,   0,  // bras gauche
			  0,   0,   0,  // main gauche sup
			  0,   0,   0,  // main gauche inf
							// MBOT_WALK4:
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

		for ( i=0 ; i<3*17*MBOT_MAX ; i++ )
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

	if ( type == OBJECT_BOT1 )
	{
		character->mass = 1000.0f;
	}

	if ( type == OBJECT_BOT2 ||
		 type == OBJECT_BOT4 ||
		 type == OBJECT_BOT5 )
	{
		character->mass = 1000.0f;
	}

	if ( type == OBJECT_WALKER ||
		 type == OBJECT_CRAZY  )
	{
		character->mass = 3000.0f;
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

		m_actionType = MBOT_WAIT;
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
	ObjectType	type;
	D3DVECTOR	linVib, cirVib, zoom, zFactor, pos;
	FPOINT		rot;
	float		time, effect[3*20], angle, s, c, prog;
	int			i, j, is, ie, max, action;

	if ( m_engine->RetPause() )  return TRUE;
//?	if ( !m_engine->IsVisiblePoint(m_object->RetPosition(0)) )  return TRUE;

	type = m_object->RetType();
	if ( type == OBJECT_CRAZY )
	{
		BrainFrameCrazy();
	}

	if ( (type != OBJECT_BOT1 || m_actionType != MBOT_WALK1) )
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

	time = event.rTime;
#if ADJUST_ANGLE
	time *= 100.0f;
#endif

	if ( type == OBJECT_BOT1    )  max = 10;
	if ( type == OBJECT_BOT2    )  max = 10;
	if ( type == OBJECT_BOT4    )  max = 17;
	if ( type == OBJECT_WALKER  )  max = 17;
	if ( type == OBJECT_CRAZY   )  max = 17;

	for ( i=0 ; i<3*max ; i++ )
	{
		effect[i] = 0.0f;
	}
	linVib = D3DVECTOR(0.0f, 0.0f, 0.0f);
	cirVib = D3DVECTOR(0.0f, 0.0f, 0.0f);

	if ( type == OBJECT_BOT1 )
	{
		if ( action == MBOT_WAIT )
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
			time *= 5.0f;
		}
	}

	if ( type == OBJECT_BOT2 )
	{
		if ( action == MBOT_WAIT )
		{
			float head[] = {0.0f, 0.0f, 0.78f, 0.0f, 0.0f, -0.78f};
			effect[3*1+yy] = head[(int)(m_aTime*1.5f)%6];

			effect[3*6+zz] =  sinf(m_aTime*3.0f)*0.2f;
			effect[3*7+zz] = -sinf(m_aTime*3.0f)*0.2f;
			effect[3*8+zz] = -sinf(m_aTime*3.0f)*0.2f;
			effect[3*9+zz] =  sinf(m_aTime*3.0f)*0.2f;
			time *= 5.0f;
		}

		linVib.y += 1.8f;  // pour poser les pieds sur le sol
	}

	if ( type == OBJECT_BOT4 )
	{
		if ( action == MBOT_WAIT )
		{
			float head[] = {0.0f, 0.0f, 0.0f, 0.4f, 0.0f, 0.0f, 0.0f, -0.4f};
			effect[3* 2+yy] = head[(int)(m_aTime*2.0f)%8];

			effect[3* 9+xx] = -powf(Abs( sinf(m_aTime*1.0f)), 0.5f)*0.08f;
			effect[3*10+xx] =  powf(Abs(-sinf(m_aTime*1.0f)), 0.5f)*0.08f;
			effect[3*13+xx] =  powf(Abs( sinf(m_aTime*1.0f)), 0.5f)*0.08f;
			effect[3*14+xx] = -powf(Abs(-sinf(m_aTime*1.0f)), 0.5f)*0.08f;
			effect[3* 9+zz] =  sinf(m_aTime*0.5f)*0.04f;
			effect[3*13+zz] = -sinf(m_aTime*0.5f)*0.04f;
		}

		time *= 5.0f;
	}

	if ( type == OBJECT_WALKER )
	{
		s = m_actionLinSpeed;
		c = m_actionCirSpeed;
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
				is = MBOT_WALK1;
				ie = MBOT_WALK2;
				prog = prog/0.25f;
				cirVib.z = -(1.0f-prog)*0.4f;
			}
			else if ( prog < 0.50f )
			{
				is = MBOT_WALK2;
				ie = MBOT_WALK3;
				prog = (prog-0.25f)/0.25f;
				cirVib.z = -prog*0.4f;
			}
			else if ( prog < 0.75f )
			{
				is = MBOT_WALK3;
				ie = MBOT_WALK4;
				prog = (prog-0.50f)/0.25f;
				cirVib.z = -(1.0f-prog)*0.4f;
			}
			else
			{
				is = MBOT_WALK4;
				ie = MBOT_WALK1;
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
		m_bFirstSound = FALSE;
		
		if ( m_actionType == MBOT_WAIT )
		{
			float head[] = {0.0f, 0.0f, 0.0f, 0.4f, 0.0f, 0.0f, 0.0f, -0.4f};
			effect[3* 2+yy] = head[(int)(m_aTime*2.0f)%8];

			effect[3* 9+xx] = -powf(Abs( sinf(m_aTime*1.0f)), 0.5f)*0.08f;
			effect[3*10+xx] =  powf(Abs(-sinf(m_aTime*1.0f)), 0.5f)*0.08f;
			effect[3*13+xx] =  powf(Abs( sinf(m_aTime*1.0f)), 0.5f)*0.08f;
			effect[3*14+xx] = -powf(Abs(-sinf(m_aTime*1.0f)), 0.5f)*0.08f;
			effect[3* 9+zz] =  sinf(m_aTime*0.5f)*0.04f;
			effect[3*13+zz] = -sinf(m_aTime*0.5f)*0.04f;
		}

		time *= 5.0f;

		if ( m_actionType == MBOT_WAIT )
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
		s = m_actionLinSpeed;
		c = m_actionCirSpeed;
//?		if ( s != 0.0f || c != 0.0f )
		if ( s >= 0.1f )
		{
			m_walkTime += event.rTime;
			CrazyParticule(FALSE);

			if ( m_option == 0 )
			{
				prog = Mod(m_walkTime, 1.0f);

				if ( prog < 0.5f && !m_bFirstSound )
				{
					StepSound();
					m_bFirstSound = TRUE;
				}
				else if ( prog >= 0.5f && m_bFirstSound )
				{
					StepSound();
					m_bFirstSound = FALSE;
				}

				cirVib.x = -cosf(prog*PI*2.0f)*0.2f;
				cirVib.y =  sinf(prog*PI*2.0f)*0.2f;
				linVib.z = -tanf(cirVib.x)*0.5f;
				linVib.y = (sinf(prog*PI*4.0f)*2.0f+1.5f)*s;

				if ( prog < 0.25f )
				{
					is = MBOT_WALK1;
					ie = MBOT_WALK2;
					prog = prog/0.25f;
					cirVib.z = -(1.0f-prog)*0.4f;
				}
				else if ( prog < 0.50f )
				{
					is = MBOT_WALK2;
					ie = MBOT_WALK3;
					prog = (prog-0.25f)/0.25f;
					cirVib.z = -prog*0.4f;
				}
				else if ( prog < 0.75f )
				{
					is = MBOT_WALK3;
					ie = MBOT_WALK4;
					prog = (prog-0.50f)/0.25f;
					cirVib.z = -(1.0f-prog)*0.4f;
				}
				else
				{
					is = MBOT_WALK4;
					ie = MBOT_WALK1;
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
				linVib.y = (sinf(prog*PI*4.0f)*1.5f+0.5f)*s;

				if ( prog < 0.25f )
				{
					is = MBOT_WALK1;
					ie = MBOT_WALK2;
					prog = prog/0.25f;
					cirVib.z = -(1.0f-prog)*0.8f;
				}
				else if ( prog < 0.50f )
				{
					is = MBOT_WALK2;
					ie = MBOT_WALK3;
					prog = (prog-0.25f)/0.25f;
					cirVib.z = -prog*0.8f;
				}
				else if ( prog < 0.75f )
				{
					is = MBOT_WALK3;
					ie = MBOT_WALK4;
					prog = (prog-0.50f)/0.25f;
					cirVib.z = -(1.0f-prog)*0.8f;
				}
				else
				{
					is = MBOT_WALK4;
					ie = MBOT_WALK1;
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
				linVib.y = (sinf(prog*PI*4.0f)*1.5f)*s;

				if ( prog < 0.25f )
				{
					is = MBOT_WALK1;
					ie = MBOT_WALK2;
					prog = prog/0.25f;
					cirVib.z = -(1.0f-prog)*0.2f;
				}
				else if ( prog < 0.50f )
				{
					is = MBOT_WALK2;
					ie = MBOT_WALK3;
					prog = (prog-0.25f)/0.25f;
					cirVib.z = -prog*0.2f;
				}
				else if ( prog < 0.75f )
				{
					is = MBOT_WALK3;
					ie = MBOT_WALK4;
					prog = (prog-0.50f)/0.25f;
					cirVib.z = -(1.0f-prog)*0.2f;
				}
				else
				{
					is = MBOT_WALK4;
					ie = MBOT_WALK1;
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
				linVib.x = (sinf(prog*PI*4.0f)*1.0f)*s;

				if ( prog < 0.25f )
				{
					is = MBOT_WALK1;
					ie = MBOT_WALK2;
					prog = 1.0f;
					cirVib.z = 0.4f;
				}
				else if ( prog < 0.50f )
				{
					is = MBOT_WALK2;
					ie = MBOT_WALK3;
					prog = 1.0f;
					cirVib.z = 0.0f;
				}
				else if ( prog < 0.75f )
				{
					is = MBOT_WALK3;
					ie = MBOT_WALK4;
					prog = 1.0f;
					cirVib.z = 0.4f;
				}
				else
				{
					is = MBOT_WALK4;
					ie = MBOT_WALK1;
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
				linVib.y = (sinf(prog*PI*4.0f)*2.0f+1.0f)*s;

				if ( prog < 0.25f )
				{
					is = MBOT_WALK1;
					ie = MBOT_WALK2;
					prog = prog/0.25f;
					cirVib.z = -(1.0f-prog)*0.4f;
				}
				else if ( prog < 0.50f )
				{
					is = MBOT_WALK2;
					ie = MBOT_WALK3;
					prog = (prog-0.25f)/0.25f;
					cirVib.z = -prog*0.4f;
				}
				else if ( prog < 0.75f )
				{
					is = MBOT_WALK3;
					ie = MBOT_WALK4;
					prog = (prog-0.50f)/0.25f;
					cirVib.z = -(1.0f-prog)*0.4f;
				}
				else
				{
					is = MBOT_WALK4;
					ie = MBOT_WALK1;
					prog = (prog-0.75f)/0.25f;
					cirVib.z = -prog*0.4f;
				}
			}

//?			time *= 4.0f;
			time *= 1.0f;
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
			linVib.x = SmoothA(pos.x, linVib.x, event.rTime*3.0f);
			linVib.y = SmoothA(pos.y, linVib.y, event.rTime*3.0f);
			linVib.z = SmoothA(pos.z, linVib.z, event.rTime*3.0f);

			pos = m_object->RetCirVibration();
			cirVib.x = SmoothA(pos.x, cirVib.x, event.rTime*3.0f);
			cirVib.y = SmoothA(pos.y, cirVib.y, event.rTime*3.0f);
			cirVib.z = SmoothA(pos.z, cirVib.z, event.rTime*3.0f);

			m_object->SetLinVibration(linVib);
			m_object->SetCirVibration(cirVib);
			return TRUE;
		}
		m_walkTime = 0.0f;
		m_bFirstSound = FALSE;

		if ( c != 0.0f )  // tourne ?
		{
			action = MBOT_TURN;
			CrazyParticule(TRUE);

			if ( c > 0.5f )
			{
				if ( m_option == 1 ||  // fou ?
					 m_option == 3 )   // ss ?
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
					effect[3* 1+yy] =  sinf(m_aTime*5.0f)*0.2f;  // coup
					effect[3* 2+yy] =  sinf(m_aTime*5.0f)*0.2f;  // tête
					cirVib.y = -sinf(m_aTime*10.0f)*0.8f;
					linVib.y =  sinf(m_aTime*10.0f)*1.0f+0.5f;

					effect[3* 3+zz] =  (sinf(m_aTime*5.0f)+1.0f)*0.3f;  // jambe droite
					effect[3* 4+zz] = -(sinf(m_aTime*5.0f)+1.0f)*0.6f;
					effect[3* 5+zz] =  (sinf(m_aTime*5.0f)+1.0f)*0.3f;
					effect[3* 6+zz] =  (cosf(m_aTime*5.0f)+1.0f)*0.3f;  // jambe gauche
					effect[3* 7+zz] = -(cosf(m_aTime*5.0f)+1.0f)*0.6f;
					effect[3* 8+zz] =  (cosf(m_aTime*5.0f)+1.0f)*0.3f;
				}
			}
		}

		if ( action == MBOT_WAIT )  // attend ?
		{
			float head[] = {0.0f, 0.0f, 0.0f, 0.4f, 0.0f, 0.0f, 0.0f, -0.4f};
			effect[3* 2+yy] = head[(int)(m_aTime*2.0f)%8];

			effect[3* 9+xx] = -powf(Abs( sinf(m_aTime*1.0f)), 0.5f)*0.08f;
			effect[3*10+xx] =  powf(Abs(-sinf(m_aTime*1.0f)), 0.5f)*0.08f;
			effect[3*13+xx] =  powf(Abs( sinf(m_aTime*1.0f)), 0.5f)*0.08f;
			effect[3*14+xx] = -powf(Abs(-sinf(m_aTime*1.0f)), 0.5f)*0.08f;
			effect[3* 9+zz] =  sinf(m_aTime*0.5f)*0.04f;
			effect[3*13+zz] = -sinf(m_aTime*0.5f)*0.04f;
		}

		if ( action == MBOT_YOUPIE )  // trouvé ballon ?
		{
			cirVib.y = sinf(m_aTime*20.0f)*0.2f;
		}

		if ( action == MBOT_GOAL )  // suspendu au ballon ?
		{
			effect[3* 3+zz] =  sinf(m_aTime*6.0f)*0.8f;  // jambe droite
			effect[3* 4+zz] =  sinf(m_aTime*6.0f)*0.2f;
			effect[3* 5+zz] =  sinf(m_aTime*6.0f)*0.3f;
			effect[3* 6+zz] = -sinf(m_aTime*6.0f)*0.8f;  // jambe gauche
			effect[3* 7+zz] = -sinf(m_aTime*6.0f)*0.2f;
			effect[3* 8+zz] = -sinf(m_aTime*6.0f)*0.3f;
		}

		time *= 1.0f;

		if ( action == MBOT_TURN )  // demi-tour ?
		{
			pos = m_object->RetLinVibration();
			linVib.x = SmoothA(pos.x, linVib.x, event.rTime*1.0f);
			linVib.y = SmoothA(pos.y, linVib.y, event.rTime*1.0f);
			linVib.z = SmoothA(pos.z, linVib.z, event.rTime*1.0f);

			pos = m_object->RetCirVibration();
			cirVib.x = SmoothA(pos.x, cirVib.x, event.rTime*1.0f);
			cirVib.y = SmoothA(pos.y, cirVib.y, event.rTime*1.0f);
			cirVib.z = SmoothA(pos.z, cirVib.z, event.rTime*1.0f);
		}
	}

	j = action;
	if ( j <  0      )  j = 0;
	if ( j >= MBOT_MAX )  j = MBOT_MAX-1;

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

	if ( type == OBJECT_BOT2 )
	{
		if ( m_actionType == MBOT_WAIT )
		{
			// Rotation du corps en fonction de l'angle des jambes et
			// des pieds pour que les pieds restent plaqués au sol.
			angle = m_object->RetAngleZ(2)+m_object->RetAngleZ(3);
			cirVib.z = -angle;
		}
	}

	m_object->SetLinVibration(linVib);
	m_object->SetCirVibration(cirVib);

	return TRUE;
}

// Cerveau du robot, pour éviter l'usage d'un script cbot.

void CMotionBot::BrainFrameCrazy()
{
	D3DMATRIX*	mat;
	D3DVECTOR	pos;
	CTaskList*	taskList;
	CObject*	pObj;
	CMotion*	motion;
	LockZone	lz;

	if ( m_main->RetEdit() )  return;
	if ( m_object->RetLock() )  return;

	taskList = m_object->RetTaskList();
	if ( taskList == 0 )  return;

	if ( taskList->IsRunning() )  return;

	mat = m_object->RetWorldMatrix(0);
	pos = Transform(*mat, D3DVECTOR(8.0f, 0.0f, 0.0f));
	lz = m_terrain->RetLockZone(pos);

	if ( lz == LZ_BLUPI )
	{
		pObj = SearchBlupi(pos);
		{
			if ( pObj != 0 && pObj->RetType() == OBJECT_BLUPI )
			{
				motion = pObj->RetMotion();
				if ( motion != 0 )
				{
					motion->SetAction(MBLUPI_STOP);
				}
			}
		}
	}

	if ( lz == LZ_FREE || lz == LZ_MINE )
	{
		m_object->StartTaskList(TO_MOVE, D3DVECTOR(0.0f, 0.0f, 0.0f), 0, 0, 8.0f);
	}
	else
	{
		m_object->StartTaskList(TO_TURN, D3DVECTOR(0.0f, 0.0f, 0.0f), 0, 0, -PI/2.0f);
	}
}

// Cherche un blupi.

CObject* CMotionBot::SearchBlupi(D3DVECTOR center)
{
	CObject		*pObj, *pBest;
	D3DVECTOR	pos;
	ObjectType	type;
	float		min, dist;
	int			i;

	pBest = 0;
	min = 100000.0f;
	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;
		if ( pObj == m_object )  continue;  // soi-même ?
		if ( pObj->RetLock() )  continue;
		if ( !pObj->RetEnable() )  continue;

		type = pObj->RetType();
		if ( type != OBJECT_BLUPI )  continue;

		pos = pObj->RetPosition(0);
		dist = Length2d(pos, center);

		if ( dist <= 2.0f && dist < min )
		{
			min = dist;
			pBest = pObj;
		}
	}
	return pBest;
}


// Génère les particules du robot fou.

void CMotionBot::CrazyParticule(BOOL bTurn)
{
	D3DVECTOR	pos, speed;
	FPOINT		dim;

	if ( m_lastParticule+0.10f <= m_aTime )
	{
		m_lastParticule = m_aTime;

		pos = m_object->RetPosition(0);
		pos.y += 3.0f;

		if ( bTurn )
		{
			speed.x = (Rand()-0.5f)*4.0f;
			speed.z = (Rand()-0.5f)*4.0f;
			speed.y = 10.0f+Rand()*10.0f;
			dim.x = 1.5f;
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTILENS3, 1.0f, 40.0f);

			speed.x = (Rand()-0.5f)*2.0f;
			speed.z = (Rand()-0.5f)*2.0f;
			speed.y = Rand()*4.0f;
			dim.x = 2.0f;
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTISMOKE1, 3.0f);
		}
		else
		{
			speed.x = (Rand()-0.5f)*4.0f;
			speed.z = (Rand()-0.5f)*4.0f;
			speed.y = 10.0f+Rand()*10.0f;
			dim.x = 0.5f;
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTILENS4, 1.0f, 40.0f);
		}
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
//?		m_sound->Play(SOUND_FIREp, m_object->RetPosition(0));
	}

	if ( m_lastParticule+0.05f <= m_aTime )
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


// Détermine le son à utiliser pour la marche (pour un pas).

void CMotionBot::StepSound()
{
	D3DVECTOR	pos;
	Sound		sound;
	float		hard, amplitude, frequency;

	pos = m_object->RetPosition(0);
	hard = m_terrain->RetHardness(pos);

	amplitude = 1.0f;

	if ( hard <= 0.2f )  // 0.0 -> 0.2 ?
	{
		hard = hard/0.2f;
		sound = SOUND_STEPs;
		frequency = 1.0f+hard*0.4f;
	}
	else if ( hard <= 0.5f )  // 0.2 -> 0.5 ?
	{
		hard = (hard-0.2f)/0.3f;
		sound = SOUND_STEPh;
		frequency = 1.0f+hard*0.2f;
	}
	else		// 0.5 -> 1.0 ?
	{
		hard = (hard-0.5f)/0.5f;
		sound = SOUND_STEPm;
		frequency = 0.6f+hard*1.2f;
	}

	frequency *= 1.3f;

	m_sound->Play(sound, pos, amplitude, frequency);
}


// Ecrit la situation de l'objet.

void CMotionBot::WriteSituation()
{
	CTaskList*	taskList;
	CTask*		task;
	TaskOrder	to;

	taskList = m_object->RetTaskList();
	if ( taskList == 0 )  return;

	task = taskList->RetRunningTask();
	if ( task == 0 )  return;

	to = taskList->RetRunningOrder();
	m_undo->WriteTokenInt("order", to);

	task->WriteSituation();
}

// lit la situation de l'objet.

void CMotionBot::ReadSituation()
{
	CTaskList*	taskList;
	CTask*		task;
	D3DVECTOR	pos;
	int			to;

	taskList = m_object->RetTaskList();
	if ( taskList == 0 )  return;

	taskList->Abort();

	if ( m_undo->ReadTokenInt("order", to) )
	{
		if ( to == TO_MOVE )
		{
			if ( m_undo->ReadTokenPos("start", pos) )
			{
				m_object->SetPosition(0, pos);  // (*)
			}
			taskList->StartTaskMove(8.0f, TRUE);
		}
		if ( to == TO_TURN )
		{
			taskList->StartTaskTurn(-PI/2.0f);
		}

		task = taskList->RetRunningTask();
		if ( task != 0 )
		{
			task->ReadSituation();
		}
	}
}

// (*)	CTaskAdvance::Start peut générer une erreur si le robot
//		n'est pas à la position de départ. Si le robot est proche
//		de la position d'arrivée, il considère cette position
//		comme occupée (par lui-même) !

