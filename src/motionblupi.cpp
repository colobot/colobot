// motionblupi.cpp

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
#include "motionblupi.h"



#define ADJUST_ANGLE	FALSE		// TRUE -> ajuste les angles des membres

#define xx	0
#define yy	1
#define zz	2



// Constructeur de l'objet.

CMotionBlupi::CMotionBlupi(CInstanceManager* iMan, CObject* object)
							  : CMotion(iMan, object)
{
	int		i;

	CMotion::CMotion(iMan, object);
	m_option = 0;

	for ( i=0 ; i<10 ; i++ )
	{
		m_additional[i] = 0;
	}

	m_faceEye = 0;
	m_faceMouth = 0;
	m_walk = MBLUPI_WALK;
	m_aTime = Rand()*10.0f;
	m_bTime = Rand()*10.0f-7.0f;
	m_walkTime = 0.0f;
	m_bFirstSound = FALSE;
	m_gimmickTime = 0.0f;
	m_closeTime = 1.0f;
	m_lastParticuleStrong = 0.0f;
	m_lastPos = D3DVECTOR(0.0f, 0.0f, 0.0f);
}

// Destructeur de l'objet.

CMotionBlupi::~CMotionBlupi()
{
}


// Supprime un objet.

void CMotionBlupi::DeleteObject(BOOL bAll)
{
}


// Démarre une action spéciale.

Error CMotionBlupi::SetAction(int action, float time)
{
	D3DVECTOR	iPos, oPos;
	CObject*	pObj;
	float		a, g, angle;

	if ( action >= MBLUPI_EYE+0 &&
		 action <= MBLUPI_EYE+3 )
	{
		UpdateFaceMappingEye(action-MBLUPI_EYE);
		return ERR_OK;
	}

	if ( action >= MBLUPI_MOUTH+0 &&
		 action <= MBLUPI_MOUTH+3 )
	{
		UpdateFaceMappingMouth(action-MBLUPI_MOUTH);
		return ERR_OK;
	}

	if ( action == MBLUPI_WALK ||
		 action == MBLUPI_PUSH )
	{
		m_walk = action;
		return ERR_OK;
	}

	if ( time == 0.2f )  // valeur par défaut ?
	{
		if ( action == MBLUPI_PUSH   )  time = 1.0f/5.0f;
		if ( action == MBLUPI_ERROR  )  time = 1.0f/2.0f;
		if ( action == MBLUPI_PFIOU  )  time = 1.0f/2.0f;
		if ( action == MBLUPI_AMAZE  )  time = 1.0f/7.0f;
		if ( action == MBLUPI_RUSH   )  time = 1.0f/0.8f;
		if ( action == MBLUPI_ROLL   )  time = 1.0f/2.0f;
		if ( action == MBLUPI_TRAX   )  time = 1.0f/1.2f;
		if ( action == MBLUPI_DRINK1 )  time = 1.0f/0.5f;
		if ( action == MBLUPI_DRINK2 )  time = 1.0f/3.0f;
		if ( action == MBLUPI_YOUPIE )  time = 1.0f/2.5f;
		if ( action == MBLUPI_STOP   )  time = 1.0f/3.0f;
	}

	if ( action == MBLUPI_ERROR )
	{
		m_sound->Play(SOUND_BLUPInon, m_object->RetPosition(0), 1.0f);
	}

	if ( action == MBLUPI_STOP )
	{
		pObj = SearchBot(m_object->RetPosition(0), 12.0f);
		if ( pObj != 0 )
		{
			oPos = pObj->RetPosition(0);
			iPos = m_object->RetPosition(0);
			a = m_object->RetAngleY(0);
			g = RotateAngle(oPos.x-iPos.x, iPos.z-oPos.z);  // CW !
			angle = Direction(a, g);
			m_startAngle = a;
			m_goalAngle = a+angle; // tourne en direction du robot
			m_sound->Play(SOUND_BLUPIblibli, m_object->RetPosition(0), 1.0f);
		}
	}

	return CMotion::SetAction(action, time);
}


// Retourne la vitesse linéaire.

float CMotionBlupi::RetLinSpeed()
{
	return 12.0f;
}

// Retourne la vitesse linéaire.

float CMotionBlupi::RetCirSpeed()
{
	return 2.0f*PI;
}

// Retourne la distance linéaire de freinage.

float CMotionBlupi::RetLinStopLength()
{
	return 1.0f;
}


// Crée un robot quelconque posé sur le sol.

BOOL CMotionBlupi::Create(D3DVECTOR pos, float angle, ObjectType type)
{
	CModFile*	pModFile;
	int			rank, i;
	char		name[50];

	if ( m_engine->RetRestCreate() < 10 )  return FALSE;

	m_object->SetType(type);
	m_option = m_object->RetOption();

	for ( i=0 ; i<10 ; i++ )
	{
		m_additional[i] = m_object->RetAdditional(i);
	}

	CreatePhysics();
	pModFile = new CModFile(m_iMan);

	// Crée la base principale.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEBLUPI);  // c'est un objet fixe
	m_object->SetObjectRank(0, rank);
	pModFile->ReadModel("objects\\blupi1.mod");
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(0, pos);
	m_object->SetAngleY(0, angle);

	// Crée la jambe droite.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEBLUPI);
	m_object->SetObjectRank(1, rank);
	m_object->SetObjectParent(1, 0);
	pModFile->ReadModel("objects\\blupi2.mod");
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(1, D3DVECTOR(-0.5f, -0.4f, -0.9f));
	m_object->SetAngleZ(1, 30.0f*PI/180.0f);

	// Crée la jambe droite.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEBLUPI);
	m_object->SetObjectRank(2, rank);
	m_object->SetObjectParent(2, 1);
	pModFile->ReadModel("objects\\blupi3.mod");
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(2, D3DVECTOR(0.0f, -0.6f, 0.0f));
	m_object->SetAngleZ(2, -60.0f*PI/180.0f);

	// Crée le pied droite.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEBLUPI);
	m_object->SetObjectRank(3, rank);
	m_object->SetObjectParent(3, 2);
	sprintf(name, "objects\\blupis%d.mod", m_additional[ADD_SHOE]);
	pModFile->ReadModel(name);
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(3, D3DVECTOR(0.2f, -0.7f, 0.0f));
	m_object->SetAngleZ(3, 30.0f*PI/180.0f);

	// Crée la jambe gauche.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEBLUPI);
	m_object->SetObjectRank(4, rank);
	m_object->SetObjectParent(4, 0);
	pModFile->ReadModel("objects\\blupi2.mod");
	pModFile->Mirror();
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(4, D3DVECTOR(-0.5f, -0.4f, 0.9f));
	m_object->SetAngleZ(4, 30.0f*PI/180.0f);

	// Crée la jambe gauche.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEBLUPI);
	m_object->SetObjectRank(5, rank);
	m_object->SetObjectParent(5, 4);
	pModFile->ReadModel("objects\\blupi3.mod");
	pModFile->Mirror();
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(5, D3DVECTOR(0.0f, -0.6f, 0.0f));
	m_object->SetAngleZ(5, -60.0f*PI/180.0f);

	// Crée le pied gauche.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEBLUPI);
	m_object->SetObjectRank(6, rank);
	m_object->SetObjectParent(6, 5);
	sprintf(name, "objects\\blupis%d.mod", m_additional[ADD_SHOE]);
	pModFile->ReadModel(name);
	pModFile->Mirror();
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(6, D3DVECTOR(0.2f, -0.7f, 0.0f));
	m_object->SetAngleZ(6, 30.0f*PI/180.0f);

	// Crée le bras droite.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEBLUPI);
	m_object->SetObjectRank(7, rank);
	m_object->SetObjectParent(7, 0);
	pModFile->ReadModel("objects\\blupi5.mod");
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(7, D3DVECTOR(-0.1f, 2.3f, -1.3f));
	m_object->SetAngleX(7, -40.0f*PI/180.0f);

	// Crée le bras droite.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEBLUPI);
	m_object->SetObjectRank(8, rank);
	m_object->SetObjectParent(8, 7);
	pModFile->ReadModel("objects\\blupi6.mod");
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(8, D3DVECTOR(0.0f, 0.0f, -1.45f));
	m_object->SetAngleX(8, -90.0f*PI/180.0f);

	// Crée la main droite.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEBLUPI);
	m_object->SetObjectRank(9, rank);
	m_object->SetObjectParent(9, 8);
	sprintf(name, "objects\\blupim%d.mod", m_additional[ADD_GLOVE]);
	pModFile->ReadModel(name);
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(9, D3DVECTOR(0.0f, 0.0f, -1.3f));
	m_object->SetAngleX(9, 10.0f*PI/180.0f);
	m_object->SetZoom(9, 1.3f);

	// Crée le bras gauche.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEBLUPI);
	m_object->SetObjectRank(10, rank);
	m_object->SetObjectParent(10, 0);
	pModFile->ReadModel("objects\\blupi5.mod");
	pModFile->Mirror();
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(10, D3DVECTOR(-0.1f, 2.3f, 1.3f));
	m_object->SetAngleX(10, 40.0f*PI/180.0f);

	// Crée le bras gauche.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEBLUPI);
	m_object->SetObjectRank(11, rank);
	m_object->SetObjectParent(11, 10);
	pModFile->ReadModel("objects\\blupi6.mod");
	pModFile->Mirror();
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(11, D3DVECTOR(0.0f, 0.0f, 1.45f));
	m_object->SetAngleX(11, 90.0f*PI/180.0f);

	// Crée la main gauche.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEBLUPI);
	m_object->SetObjectRank(12, rank);
	m_object->SetObjectParent(12, 11);
	sprintf(name, "objects\\blupim%d.mod", m_additional[ADD_GLOVE]);
	pModFile->ReadModel(name);
	pModFile->Mirror();
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(12, D3DVECTOR(0.0f, 0.0f, 1.3f));
	m_object->SetAngleX(12, -10.0f*PI/180.0f);
	m_object->SetZoom(12, 1.3f);

	// Crée le sourcil droite.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEBLUPI);
	m_object->SetObjectRank(13, rank);
	m_object->SetObjectParent(13, 0);
	pModFile->ReadModel("objects\\blupi8.mod");
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(13, D3DVECTOR(0.3f, 3.8f, -0.6f));

	// Crée le sourcil gauche.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEBLUPI);
	m_object->SetObjectRank(14, rank);
	m_object->SetObjectParent(14, 0);
	pModFile->ReadModel("objects\\blupi8.mod");
	pModFile->Mirror();
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(14, D3DVECTOR(0.3f, 3.8f, 0.6f));

	if ( m_option == 1 ||  // avec ballon ?
		 m_option == 2 )   // avec ballon ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEBLUPI);
		m_object->SetObjectRank(15, rank);
		m_object->SetObjectParent(15, 0);
		pModFile->ReadModel("objects\\goal.mod");
		pModFile->Mirror();
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(15, D3DVECTOR(0.0f, 2.0f, 0.0f));
	}

	if ( m_additional[ADD_HAT] != 0 )  // avec chapeau ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEBLUPI);
		m_object->SetObjectRank(16, rank);
		m_object->SetObjectParent(16, 0);
		sprintf(name, "objects\\blupih%d.mod", m_additional[ADD_HAT]);
		pModFile->ReadModel(name);
		pModFile->CreateEngineObject(rank);
		if ( m_additional[ADD_HAT] == 1 )  // bonnet ?
		{
			m_object->SetPosition(16, D3DVECTOR(-0.8f, 3.3f, 0.0f));
			m_object->SetAngleZ(16, 25.0f*PI/180.0f);
		}
		if ( m_additional[ADD_HAT] == 2 )  // casquette ?
		{
			m_object->SetPosition(16, D3DVECTOR(-0.3f, 3.2f, 0.0f));
			m_object->SetAngleZ(16, 0.0f*PI/180.0f);
		}
		if ( m_additional[ADD_HAT] == 3 )  // magicien ?
		{
			m_object->SetPosition(16, D3DVECTOR(-0.5f, 3.4f, 0.0f));
			m_object->SetAngleZ(16, 15.0f*PI/180.0f);
		}
		if ( m_additional[ADD_HAT] == 4 )  // visière ?
		{
			m_object->SetPosition(16, D3DVECTOR(-0.3f, 3.2f, 0.0f));
			m_object->SetAngleZ(16, 0.0f*PI/180.0f);
		}
		if ( m_additional[ADD_HAT] == 6 )  // gandalf ?
		{
			m_object->SetPosition(16, D3DVECTOR(-0.5f, 3.2f, 0.0f));
			m_object->SetAngleZ(16, 15.0f*PI/180.0f);
		}
	}

	if ( m_additional[ADD_GLASS] != 0 )  // avec lunettes ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEBLUPI);
		m_object->SetObjectRank(17, rank);
		m_object->SetObjectParent(17, 0);
		sprintf(name, "objects\\blupig%d.mod", m_additional[ADD_GLASS]);
		pModFile->ReadModel(name);
		pModFile->CreateEngineObject(rank);
	}

	if ( m_additional[ADD_BAG] != 0 )  // avec sac ?
	{
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEBLUPI);
		m_object->SetObjectRank(18, rank);
		m_object->SetObjectParent(18, 0);
		sprintf(name, "objects\\blupib%d.mod", m_additional[ADD_BAG]);
		pModFile->ReadModel(name);
		pModFile->CreateEngineObject(rank);
	}

	m_object->CreateLockZone(0, 0, LZ_BLUPI);
	m_object->CreateShadow(2.6f, 1.0f, D3DSHADOWNORM, TRUE, 0.75f);
	m_object->SetFloorHeight(0.0f);

	pos = m_object->RetPosition(0);
	m_object->SetPosition(0, pos);  // pour afficher les ombres tout de suite

	if ( m_option == 2 )
	{
		SetAction(MBLUPI_GOAL2);
	}
	else
	{
		SetAction(MBLUPI_WAIT);
	}

	delete pModFile;
	return TRUE;
}


// Crée la physique de l'objet.

void CMotionBlupi::CreatePhysics()
{
	Character*	character;
	ObjectType	type;
	int			i;

	character = m_object->RetCharacter();
	type = m_object->RetType();

	short member[] =
	{
						// MBLUPI_WAIT:
		  0,   0,   0,  // corps
		  0,   0,  30,  // jambe droite
		  0,   0, -60,  // jambe droite
		  0,   0,  30,  // pied droite
		  0,   0,  30,  // jambe gauche
		  0,   0, -60,  // jambe gauche
		  0,   0,  30,  // pied gauche
		-40,   0,   0,  // bras droite
		-90,   0,   0,  // bras droite
		 10,   0,   0,  // main droite
		 40,   0,   0,  // bras gauche
		 90,   0,   0,  // bras gauche
		-10,   0,   0,  // main gauche
		  0,   0,   0,  // sourcil droite
		  0,   0,   0,  // sourcil gauche
						// MBLUPI_TURN:
		  0,   0,   0,  // corps
		  0,   0,   0,  // jambe droite
		  0,   0,   0,  // jambe droite
		  0,   0,   0,  // pied droite
		  0,   0,   0,  // jambe gauche
		  0,   0,   0,  // jambe gauche
		  0,   0,   0,  // pied gauche
		-50,  35,   0,  // bras droite
		  0,   0,   0,  // bras droite
		  0,   0,   0,  // main droite
		 50, -35,   0,  // bras gauche
		  0,   0,   0,  // bras gauche
		  0,   0,   0,  // main gauche
		  0,   0,   0,  // sourcil droite
		  0,   0,   0,  // sourcil gauche
						// MBLUPI_ERROR:
		  0,   0,   0,  // corps
		  0,  30,   0,  // jambe droite
		  0,   0, -30,  // jambe droite
		  0, -10,  30,  // pied droite
		-10,  30,  30,  // jambe gauche
		  0,   0,   0,  // jambe gauche
		  0,  10, -30,  // pied gauche
		-20, -30,   0,  // bras droite
	   -110,   0,   0,  // bras droite
		 30,   0,   0,  // main droite
		 20,  30,   0,  // bras gauche
		110,   0,   0,  // bras gauche
		-30,   0,   0,  // main gauche
		 20,   0,   0,  // sourcil droite
		-30,   0,   0,  // sourcil gauche
						// MBLUPI_ERROR2:
		  0,   0,   0,  // corps
		 10, -30,  30,  // jambe droite
		  0,   0,   0,  // jambe droite
		  0, -10, -30,  // pied droite
		  0, -30,   0,  // jambe gauche
		  0,   0, -30,  // jambe gauche
		  0,  10,  30,  // pied gauche
		-20, -30,   0,  // bras droite
	   -110,   0,   0,  // bras droite
		 30,   0,   0,  // main droite
		 20,  30,   0,  // bras gauche
		110,   0,   0,  // bras gauche
		-30,   0,   0,  // main gauche
		 30,   0,   0,  // sourcil droite
		-20,   0,   0,  // sourcil gauche
						// MBLUPI_HELLO:
		  0,   0,   0,  // corps
		  0,   0,  50,  // jambe droite
		  0,   0,-100,  // jambe droite
		  0,   0,  50,  // pied droite
		  0,   0,   0,  // jambe gauche
		  0,   0,   0,  // jambe gauche
		  0,   0,   0,  // pied gauche
		 20,   0,   0,  // bras droite
		 40,   0,   0,  // bras droite
		 10,   0,   0,  // main droite
		 40,   0,   0,  // bras gauche
		 90,   0,   0,  // bras gauche
		-10,   0,   0,  // main gauche
		-10,   0,   0,  // sourcil droite
		 10,   0,   0,  // sourcil gauche
						// MBLUPI_HELLO2:
		  0,   0,   0,  // corps
		  0,   0,   0,  // jambe droite
		  0,   0,   0,  // jambe droite
		  0,   0,   0,  // pied droite
		  0,   0,  50,  // jambe gauche
		  0,   0,-100,  // jambe gauche
		  0,   0,  50,  // pied gauche
		 70,   0,   0,  // bras droite
		 80,   0,   0,  // bras droite
		 10,   0,   0,  // main droite
		 40,   0,   0,  // bras gauche
		 90,   0,   0,  // bras gauche
		-10,   0,   0,  // main gauche
		-10,   0,   0,  // sourcil droite
		 10,   0,   0,  // sourcil gauche
						// MBLUPI_AMAZE:
		  0,   0,   0,  // corps
		  0,   0,  30,  // jambe droite
		  0,   0, -60,  // jambe droite
		  0, -10,  30,  // pied droite
		  0,   0,  30,  // jambe gauche
		  0,   0, -60,  // jambe gauche
		  0,  10,  30,  // pied gauche
		-40, -40,   0,  // bras droite
		-90, -20,  55,  // bras droite
		 10,   0,   0,  // main droite
		 20,  30,   0,  // bras gauche
		110,   0,   0,  // bras gauche
		-30,   0,   0,  // main gauche
		  0,   0,   0,  // sourcil droite
		  0,   0,   0,  // sourcil gauche
						// MBLUPI_AMAZE2:
		  0,   0,   0,  // corps
		  0,   0,  40,  // jambe droite
		  0,   0, -60,  // jambe droite
		  0, -10,  30,  // pied droite
		  0,   0,  40,  // jambe gauche
		  0,   0, -60,  // jambe gauche
		  0,  10,  30,  // pied gauche
		-40, -30,   0,  // bras droite
		-90,  20,  55,  // bras droite
		 10,   0,   0,  // main droite
		 20,  30,   0,  // bras gauche
		110,   0,   0,  // bras gauche
		-30,   0,   0,  // main gauche
		  0,   0,   0,  // sourcil droite
		  0,   0,   0,  // sourcil gauche
						// MBLUPI_PFIOU:
		  0,   0,   0,  // corps
		  0,  30, -10,  // jambe droite
		  0,   0, -30,  // jambe droite
		  0, -10,  30,  // pied droite
		-10,  30,  20,  // jambe gauche
		  0,   0,   0,  // jambe gauche
		  0,  10, -30,  // pied gauche
		 70, -50,  -5,  // bras droite
		 95,  -5, -55,  // bras droite
		 10,   0,   0,  // main droite
		 60,   0,   0,  // bras gauche
		  0,   0,   0,  // bras gauche
		-10,   0,   0,  // main gauche
		-20,   0,   0,  // sourcil droite
		 20,   0,   0,  // sourcil gauche
						// MBLUPI_PFIOU2:
		  0,   0,   0,  // corps
		 10, -30,  20,  // jambe droite
		  0,   0,   0,  // jambe droite
		  0, -10, -30,  // pied droite
		  0, -30, -10,  // jambe gauche
		  0,   0, -30,  // jambe gauche
		  0,  10,  30,  // pied gauche
		 20, -30,   0,  // bras droite
		145,  10, -25,  // bras droite
		 10,   0,   0,  // main droite
		 60,   0,   0,  // bras gauche
		  0,   0,   0,  // bras gauche
		-10,   0,   0,  // main gauche
		-20,   0,   0,  // sourcil droite
		 20,   0,   0,  // sourcil gauche
						// MBLUPI_PFIOU3:
		  0,   0,   0,  // corps
		  0,   0,  30,  // jambe droite
		  0,   0, -60,  // jambe droite
		  0,   0,  30,  // pied droite
		  0,   0,  30,  // jambe gauche
		  0,   0, -60,  // jambe gauche
		  0,   0,  30,  // pied gauche
		-40,   0,   0,  // bras droite
		-90,   0,   0,  // bras droite
		 10,   0,   0,  // main droite
		 40,   0,   0,  // bras gauche
		 90,   0,   0,  // bras gauche
		-10,   0,   0,  // main gauche
		-20,   0,   0,  // sourcil droite
		 20,   0,   0,  // sourcil gauche
						// MBLUPI_RUSH:
		  0,   0,   0,  // corps
		  0,   0,  10,  // jambe droite
		  0,   0, -60,  // jambe droite
		  0,   0,  30,  // pied droite
		  0,   0,  60,  // jambe gauche
		  0,   0, -60,  // jambe gauche
		  0,   0,  15,  // pied gauche
		 20, -15,   0,  // bras droite
		 55, -35,   0,  // bras droite
		 10,   0,   0,  // main droite
		-20,  15,   0,  // bras gauche
		-55,  35,   0,  // bras gauche
		-10,   0,   0,  // main gauche
		  0,   0,   0,  // sourcil droite
		  0,   0,   0,  // sourcil gauche
						// MBLUPI_ROLL:
		  0,   0,   0,  // corps
		  0,   0,  10,  // jambe droite
		  0,   0, -60,  // jambe droite
		  0,   0,  30,  // pied droite
		  0,   0,  60,  // jambe gauche
		  0,   0, -60,  // jambe gauche
		  0,   0,  15,  // pied gauche
		 20, -15,   0,  // bras droite
		 55, -35,   0,  // bras droite
		 10,   0,   0,  // main droite
		-20,  15,   0,  // bras gauche
		-55,  35,   0,  // bras gauche
		-10,   0,   0,  // main gauche
		  0,   0,   0,  // sourcil droite
		  0,   0,   0,  // sourcil gauche
						// MBLUPI_ROLL2:
		  0,   0,   0,  // corps
		  0,   0,  60,  // jambe droite
		  0,   0, -60,  // jambe droite
		  0,   0,  30,  // pied droite
		  0,   0,   0,  // jambe gauche
		  0,   0, -60,  // jambe gauche
		  0,   0,  40,  // pied gauche
		-70,   0,  90,  // bras droite
		-65,  30,  70,  // bras droite
		  0,   0,   0,  // main droite
		 70,   0,  90,  // bras gauche
		 65, -30,  70,  // bras gauche
		  0,   0,   0,  // main gauche
		-20,   0,   0,  // sourcil droite
		 20,   0,   0,  // sourcil gauche
						// MBLUPI_TRAX:
		  0,   0,   0,  // corps
		  0,   0,  10,  // jambe droite
		  0,   0, -60,  // jambe droite
		  0,   0,  30,  // pied droite
		  0,   0,  60,  // jambe gauche
		  0,   0, -60,  // jambe gauche
		  0,   0,  15,  // pied gauche
		 20, -15,   0,  // bras droite
		 55, -35,   0,  // bras droite
		 10,   0,   0,  // main droite
		-20,  15,   0,  // bras gauche
		-55,  35,   0,  // bras gauche
		-10,   0,   0,  // main gauche
		  0,   0,   0,  // sourcil droite
		  0,   0,   0,  // sourcil gauche
						// MBLUPI_TRAX2:
		  0,   0,   0,  // corps
		  0,   0,  60,  // jambe droite
		  0,   0, -60,  // jambe droite
		  0,   0,  30,  // pied droite
		  0,   0,   0,  // jambe gauche
		  0,   0, -60,  // jambe gauche
		  0,   0,  40,  // pied gauche
		-70, -30, 130,  // bras droite
		-65,  30,  70,  // bras droite
		  0,   0,   0,  // main droite
		 70,  30, 130,  // bras gauche
		 65, -30,  70,  // bras gauche
		  0,   0,   0,  // main gauche
		-20,   0,   0,  // sourcil droite
		 20,   0,   0,  // sourcil gauche
						// MBLUPI_TAPTAP:
		  0,   0,   0,  // corps
		  0,   0,  35,  // jambe droite
		  0,   0, -70,  // jambe droite
		  0,   0,  35,  // pied droite
		  0,   0,  25,  // jambe gauche
		  0,   0, -50,  // jambe gauche
		  0,   0,  25,  // pied gauche
		-20,   0,   0,  // bras droite
	   -110,   0,   0,  // bras droite
		 30,   0,   0,  // main droite
		 20,   0,   0,  // bras gauche
		110,   0,   0,  // bras gauche
		-30,   0,   0,  // main gauche
		  0,   0,   0,  // sourcil droite
		  0,   0,   0,  // sourcil gauche
						// MBLUPI_TAPTAP2:
		  0,   0,   0,  // corps
		  0,   0,  25,  // jambe droite
		  0,   0, -50,  // jambe droite
		  0,   0,  45,  // pied droite
		  0,   0,  35,  // jambe gauche
		  0,   0, -70,  // jambe gauche
		  0,   0,  35,  // pied gauche
		-20,   0,   0,  // bras droite
	   -110,   0,   0,  // bras droite
		 30,   0,   0,  // main droite
		 20,   0,   0,  // bras gauche
		110,   0,   0,  // bras gauche
		-30,   0,   0,  // main gauche
		  0,   0,   0,  // sourcil droite
		  0,   0,   0,  // sourcil gauche
						// MBLUPI_DRINK1:
		  0,   0,   0,  // corps
		  0,   0,  60,  // jambe droite
		  0,   0, -60,  // jambe droite
		  0,   0,  30,  // pied droite
		  0,   0,  60,  // jambe gauche
		  0,   0, -60,  // jambe gauche
		  0,   0,  30,  // pied gauche
		-70,   0,  70,  // bras droite
		-65,  30,  40,  // bras droite
		 10,   0,   0,  // main droite
		 70,   0,  70,  // bras gauche
		 65, -30,  40,  // bras gauche
		-10,   0,   0,  // main gauche
		  0,   0,   0,  // sourcil droite
		  0,   0,   0,  // sourcil gauche
						// MBLUPI_DRINK2:
		  0,   0,   0,  // corps
		  0,   0, -10,  // jambe droite
		  0,   0, -60,  // jambe droite
		  0,   0,  30,  // pied droite
		  0,   0, -10,  // jambe gauche
		  0,   0, -60,  // jambe gauche
		  0,   0,  30,  // pied gauche
		-90,  15,  70,  // bras droite
		-15,  -5,   0,  // bras droite
		 10,   0,   0,  // main droite
		 90, -15,  70,  // bras gauche
		 15,   5,   0,  // bras gauche
		-10,   0,   0,  // main gauche
		  0,   0,   0,  // sourcil droite
		  0,   0,   0,  // sourcil gauche
						// MBLUPI_YOUPIE:
		  0,   0,   0,  // corps
		  0,   0,  40,  // jambe droite
		  0,   0, -80,  // jambe droite
		  0,   0,  40,  // pied droite
		  0,   0,   0,  // jambe gauche
		  0,   0,   0,  // jambe gauche
		  0,   0,   0,  // pied gauche
		-35,   0,   0,  // bras droite
		140,   0,   0,  // bras droite
		  0,   0,   0,  // main droite
		-40,   0,   0,  // bras gauche
		-70,   0,   0,  // bras gauche
		  0,   0,   0,  // main gauche
		  0,   0,   0,  // sourcil droite
		  0,   0,   0,  // sourcil gauche
						// MBLUPI_YOUPIE2:
		  0,   0,   0,  // corps
		  0,   0,   0,  // jambe droite
		  0,   0,   0,  // jambe droite
		  0,   0,   0,  // pied droite
		  0,   0,  40,  // jambe gauche
		  0,   0, -80,  // jambe gauche
		  0,   0,  40,  // pied gauche
		 40,   0,   0,  // bras droite
		 70,   0,   0,  // bras droite
		  0,   0,   0,  // main droite
		 35,   0,   0,  // bras gauche
	   -140,   0,   0,  // bras gauche
		  0,   0,   0,  // main gauche
		  0,   0,   0,  // sourcil droite
		  0,   0,   0,  // sourcil gauche
						// MBLUPI_GOAL1:
		  0,   0,   0,  // corps
		  0,   0,  30,  // jambe droite
		  0,   0, -60,  // jambe droite
		  0,   0,  30,  // pied droite
		  0,   0,  30,  // jambe gauche
		  0,   0, -60,  // jambe gauche
		  0,   0,  30,  // pied gauche
		 80,  20,   0,  // bras droite
		 65,   0,   0,  // bras droite
		 10,   0,   0,  // main droite
		-80, -20,   0,  // bras gauche
		-65,   0,   0,  // bras gauche
		-10,   0,   0,  // main gauche
		  0,   0,   0,  // sourcil droite
		  0,   0,   0,  // sourcil gauche
						// MBLUPI_GOAL2:
		  0,   0,   0,  // corps
		  0,   0,  30,  // jambe droite
		  0,   0, -60,  // jambe droite
		  0,  20,  10,  // pied droite
		  0,   0,  30,  // jambe gauche
		  0,   0, -60,  // jambe gauche
		  0, -20,  10,  // pied gauche
		 80,  10,   0,  // bras droite
		 65,   0,   0,  // bras droite
		 10,   0,   0,  // main droite
		-80, -10,   0,  // bras gauche
		-65,   0,   0,  // bras gauche
		-10,   0,   0,  // main gauche
		  0,   0,   0,  // sourcil droite
		  0,   0,   0,  // sourcil gauche
						// MBLUPI_STOP:
		  0,   0,   0,  // corps
		  0,   0,  30,  // jambe droite
		  0,   0, -60,  // jambe droite
		  0,   0,  30,  // pied droite
		  0,   0,  30,  // jambe gauche
		  0,   0, -60,  // jambe gauche
		  0,   0,  30,  // pied gauche
		  0,   0,   0,  // bras droite
		 20,   0,   0,  // bras droite
		 10,   0,   0,  // main droite
		  0,   0,   0,  // bras gauche
		-20,   0,   0,  // bras gauche
		-10,   0,   0,  // main gauche
		  0,   0,   0,  // sourcil droite
		  0,   0,   0,  // sourcil gauche
						// MBLUPI_TRUCK:
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
						// MBLUPI_OUPS:
		  0,   0,   0,  // corps
		  0,   0,  30,  // jambe droite
		  0,   0, -60,  // jambe droite
		  0,   0,  30,  // pied droite
		  0,   0,  30,  // jambe gauche
		  0,   0, -60,  // jambe gauche
		  0,   0,  30,  // pied gauche
		  0,   0,   0,  // bras droite
		  0,   0,   0,  // bras droite
		  0,   0,   0,  // main droite
		  0,   0,   0,  // bras gauche
		  0,   0,   0,  // bras gauche
		  0,   0,   0,  // main gauche
		  0,   0,   0,  // sourcil droite
		  0,   0,   0,  // sourcil gauche
						// MBLUPI_WALK1:
		  0,   0,   0,  // corps
		 10,   5,  20,  // jambe droite (milieu au sol)
		  0,   0, -40,  // jambe droite
		  0,   0,  30,  // pied droite
		  0,  -5,  90,  // jambe gauche (milieu en l'air)
		  0,   0,-170,  // jambe gauche
		  0,   0, 140,  // pied gauche
		-55,   0, -20,  // bras droite
		-35,   0,   0,  // bras droite
		  0,   0,   0,  // main droite
		 55,   0,  70,  // bras gauche
		 35,   0,  50,  // bras gauche
		  0,   0,   0,  // main gauche
		  0,   0,   0,  // sourcil droite
		  0,   0,   0,  // sourcil gauche
						// MBLUPI_WALK2:
		  0,   0,   0,  // corps
		  0,   5, -35,  // jambe droite (arrière)
		  0,   0, -60,  // jambe droite
		  0,   0,  30,  // pied droite
		  0,  -5,  80,  // jambe gauche (avant)
		  0,   0, -50,  // jambe gauche
		  0,   0,  10,  // pied gauche
		-55,   0,   0,  // bras droite
		-35,   0,   0,  // bras droite
		  0,   0,   0,  // main droite
		 55,   0,   0,  // bras gauche
		 35,   0,   0,  // bras gauche
		  0,   0,   0,  // main gauche
		  0,   0,   0,  // sourcil droite
		  0,   0,   0,  // sourcil gauche
						// MBLUPI_WALK3:
		  0,   0,   0,  // corps
		  0,   5,  90,  // jambe droite (milieu en l'air)
		  0,   0,-170,  // jambe droite
		  0,   0, 140,  // pied droite
		-10,  -5,  20,  // jambe gauche (milieu au sol)
		  0,   0, -40,  // jambe gauche
		  0,   0,  30,  // pied gauche
		-55,   0,  70,  // bras droite
		-35,   0,  50,  // bras droite
		  0,   0,   0,  // main droite
		 55,   0, -20,  // bras gauche
		 35,   0,   0,  // bras gauche
		  0,   0,   0,  // main gauche
		  0,   0,   0,  // sourcil droite
		  0,   0,   0,  // sourcil gauche
						// MBLUPI_WALK4:
		  0,   0,   0,  // corps
		  0,   5,  80,  // jambe droite (avant)
		  0,   0, -50,  // jambe droite
		  0,   0,  10,  // pied droite
		  0,  -5, -35,  // jambe gauche (arrière)
		  0,   0, -60,  // jambe gauche
		  0,   0,  30,  // pied gauche
		-55,   0,   0,  // bras droite
		-35,   0,   0,  // bras droite
		  0,   0,   0,  // main droite
		 55,   0,   0,  // bras gauche
		 35,   0,   0,  // bras gauche
		  0,   0,   0,  // main gauche
		  0,   0,   0,  // sourcil droite
		  0,   0,   0,  // sourcil gauche
						// MBLUPI_PUSH1:
		  0,   0,   0,  // corps
		 10,   5,  50,  // jambe droite (milieu au sol)
		  0,   0, -60,  // jambe droite
		  0,   0,  35,  // pied droite
		  0,  -5,  85,  // jambe gauche (milieu en l'air)
		  0,   0,-125,  // jambe gauche
		  0,   0,  90,  // pied gauche
		-55, -10,  90,  // bras droite
		-35,   0,  70,  // bras droite
		  0,   0,   0,  // main droite
		 55, -10,  90,  // bras gauche
		 35,   0,  30,  // bras gauche
		  0,   0,   0,  // main gauche
		  0,   0,   0,  // sourcil droite
		  0,   0,   0,  // sourcil gauche
						// MBLUPI_PUSH2:
		  0,   0,   0,  // corps
		  0,   5,  20,  // jambe droite (arrière)
		  0,   0, -70,  // jambe droite
		  0,   0,  35,  // pied droite
		  0,  -5,  80,  // jambe gauche (avant)
		  0,   0, -65,  // jambe gauche
		  0,   0,  25,  // pied gauche
		-55,   0,  90,  // bras droite
		-35,   0,  50,  // bras droite
		  0,   0,   0,  // main droite
		 55,   0,  90,  // bras gauche
		 35,   0,  50,  // bras gauche
		  0,   0,   0,  // main gauche
		  0,   0,   0,  // sourcil droite
		  0,   0,   0,  // sourcil gauche
						// MBLUPI_PUSH3:
		  0,   0,   0,  // corps
		  0,   5,  85,  // jambe droite (milieu en l'air)
		  0,   0,-125,  // jambe droite
		  0,   0,  90,  // pied droite
		-10,  -5,  50,  // jambe gauche (milieu au sol)
		  0,   0, -60,  // jambe gauche
		  0,   0,  35,  // pied gauche
		-55,  10,  90,  // bras droite
		-35,   0,  30,  // bras droite
		  0,   0,   0,  // main droite
		 55,  10,  90,  // bras gauche
		 35,   0,  70,  // bras gauche
		  0,   0,   0,  // main gauche
		  0,   0,   0,  // sourcil droite
		  0,   0,   0,  // sourcil gauche
						// MBLUPI_PUSH4:
		  0,   0,   0,  // corps
		  0,   5,  80,  // jambe droite (avant)
		  0,   0, -65,  // jambe droite
		  0,   0,  25,  // pied droite
		  0,  -5,  20,  // jambe gauche (arrière)
		  0,   0, -70,  // jambe gauche
		  0,   0,  35,  // pied gauche
		-55,   0,  90,  // bras droite
		-35,   0,  50,  // bras droite
		  0,   0,   0,  // main droite
		 55,   0,  90,  // bras gauche
		 35,   0,  50,  // bras gauche
		  0,   0,   0,  // main gauche
		  0,   0,   0,  // sourcil droite
		  0,   0,   0,  // sourcil gauche
	};

	for ( i=0 ; i<3*15*MBLUPI_MAX ; i++ )
	{
		m_armAngles[i] = member[i];
	}

	character->height = 2.6f;
	character->mass = 1000.0f;

	m_armPartIndex = 0;
}


// Gestion d'un événement.

BOOL CMotionBlupi::EventProcess(const Event &event)
{
	CMotion::EventProcess(event);

	if ( event.event == EVENT_FRAME )
	{
		return EventFrame(event);
	}

	if ( event.event == EVENT_KEYDOWN )
	{
#if ADJUST_ANGLE
		CObject*	fret;
		D3DVECTOR	pos;
		int			i;

		if ( event.param == 'Q' )  m_armPartIndex++;
		if ( m_armPartIndex >= 15 )  m_armPartIndex = 0;
		if ( event.param == 'A' )  m_armPartIndex--;
		if ( m_armPartIndex < 0 )  m_armPartIndex = 15-1;

		m_actionType = MBLUPI_TRAX2;
		i = 3*15*m_actionType;
		i += m_armPartIndex*3;

		if ( event.param == 'E' )  m_armAngles[i+xx] += 5;
		if ( event.param == 'D' )  m_armAngles[i+xx] -= 5;
		if ( event.param == 'R' )  m_armAngles[i+yy] += 5;
		if ( event.param == 'F' )  m_armAngles[i+yy] -= 5;
		if ( event.param == 'T' )  m_armAngles[i+zz] += 5;
		if ( event.param == 'G' )  m_armAngles[i+zz] -= 5;

#if 0
		fret = m_object->RetFret();
		if ( fret != 0 )
		{
			if ( event.param == 'Z' )  fret->SetAngleX(0, fret->RetAngleX(0)+10.0f*PI/180.0f);
			if ( event.param == 'H' )  fret->SetAngleX(0, fret->RetAngleX(0)-10.0f*PI/180.0f);
			if ( event.param == 'U' )  fret->SetAngleY(0, fret->RetAngleY(0)+10.0f*PI/180.0f);
			if ( event.param == 'J' )  fret->SetAngleY(0, fret->RetAngleY(0)-10.0f*PI/180.0f);
			if ( event.param == 'I' )  fret->SetAngleZ(0, fret->RetAngleZ(0)+10.0f*PI/180.0f);
			if ( event.param == 'K' )  fret->SetAngleZ(0, fret->RetAngleZ(0)-10.0f*PI/180.0f);
		}
#else
		fret = m_object->RetFret();
		if ( fret != 0 )
		{
			pos = fret->RetPosition(0);
			if ( event.param == 'Z' )  pos.x += 1.0f;
			if ( event.param == 'H' )  pos.x -= 1.0f;
			if ( event.param == 'U' )  pos.y += 1.0f;
			if ( event.param == 'J' )  pos.y -= 1.0f;
			if ( event.param == 'I' )  pos.z += 1.0f;
			if ( event.param == 'K' )  pos.z -= 1.0f;
			fret->SetPosition(0, pos);
		}
#endif
#endif
	}

	return TRUE;
}

// Gestion d'un événement.

BOOL CMotionBlupi::EventFrame(const Event &event)
{
	ObjectType	type;
	D3DVECTOR	linVib, cirVib, zoom, zFactor, pos, ppos, ipos;
	FPOINT		rot;
	float		rTime, time, effect[3*20], s, c, prog, angle;
	int			i, j, is, ie, max, action;

	if ( m_engine->RetPause() )  return TRUE;
//?	if ( !m_engine->IsVisiblePoint(m_object->RetPosition(0)) )  return TRUE;

	type = m_object->RetType();

	if ( m_option == 2 )
	{
		BrainFrameDemo();
	}

	if ( m_object->RetExplo() )  return TRUE;  // en cours d'explosion ?

#if ADJUST_ANGLE
	if ( m_object->RetSelect() )
	{
		char s[100];
		sprintf(s, "Q:part=%d m_armAngles[%d]", m_armPartIndex, 3*15*m_actionType);
		m_engine->SetInfoText(4, s);
	}
#endif

	rTime = event.rTime;
	if ( m_object->RetStrong() > 0.0f )  // costaud ?
	{
		StrongParticule();
		rTime *= 1.5f;
	}
	if ( m_object->RetStrong() < 0.0f )  // glu ?
	{
		GluParticule();
		rTime *= 0.9f;
	}
	m_aTime += rTime;
	m_bTime += rTime;

	action = m_actionType;

	time = rTime;
#if ADJUST_ANGLE
	time *= 100.0f;
#endif

	max = 15;
	for ( i=0 ; i<3*max ; i++ )
	{
		effect[i] = 0.0f;
	}
	linVib = D3DVECTOR(0.0f, 0.0f, 0.0f);
	cirVib = D3DVECTOR(0.0f, 0.0f, 0.0f);

	// Gestion des yeux.
	m_closeTime -= rTime;
	if ( m_closeTime <= 0.0f )
	{
		if ( m_faceEye == MBLUPI_OPEN )
		{
			UpdateFaceMappingEye(MBLUPI_CLOSE);  // yeux fermés
			m_closeTime = 0.1f+Rand()*0.1f;  // choix durée
		}
		else
		{
			UpdateFaceMappingEye(MBLUPI_OPEN);  // yeux ouverts
			m_closeTime = 2.0f+Rand()*2.0f;  // choix durée
		}
	}

	s = m_actionLinSpeed;
	c = m_actionCirSpeed;

	// Gestion de la marche.
	if ( s >= 0.1f || (c != 0.0f && c < 0.5f) || m_walk == MBLUPI_PUSH )
	{
		m_walkTime += rTime*s;

		if ( m_walk == MBLUPI_WALK )  // marche normalement ?
		{
			prog = Mod(m_walkTime*1.8f, 1.0f);

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
			cirVib.y =  sinf(prog*PI*2.0f)*0.3f;
			linVib.z = -tanf(cirVib.x)*0.5f;
//?			linVib.z = cirVib.x*1.0f;
			linVib.y = (sinf(prog*PI*4.0f)*0.6f+0.0f)*s-0.2f;

			if ( prog < 0.25f )
			{
				is = MBLUPI_WALK1;
				ie = MBLUPI_WALK2;
				prog = prog/0.25f;
				cirVib.z = -(1.0f-prog)*0.4f*s;
			}
			else if ( prog < 0.50f )
			{
				is = MBLUPI_WALK2;
				ie = MBLUPI_WALK3;
				prog = (prog-0.25f)/0.25f;
				cirVib.z = -prog*0.4f*s;
			}
			else if ( prog < 0.75f )
			{
				is = MBLUPI_WALK3;
				ie = MBLUPI_WALK4;
				prog = (prog-0.50f)/0.25f;
				cirVib.z = -(1.0f-prog)*0.4f*s;
			}
			else
			{
				is = MBLUPI_WALK4;
				ie = MBLUPI_WALK1;
				prog = (prog-0.75f)/0.25f;
				cirVib.z = -prog*0.4f*s;
			}

			time *= 2.0f;
			for ( i=1 ; i<max ; i++ )
			{
				m_object->SetAngleX(i, SmoothA(m_object->RetAngleX(i), Prop(m_armAngles[is*3*max+i*3+xx], m_armAngles[ie*3*max+i*3+xx], prog), time));
				m_object->SetAngleY(i, SmoothA(m_object->RetAngleY(i), Prop(m_armAngles[is*3*max+i*3+yy], m_armAngles[ie*3*max+i*3+yy], prog), time));
				m_object->SetAngleZ(i, SmoothA(m_object->RetAngleZ(i), Prop(m_armAngles[is*3*max+i*3+zz], m_armAngles[ie*3*max+i*3+zz], prog), time));
			}
		}

		if ( m_walk == MBLUPI_PUSH )  // pousse une caisse ?
		{
			prog = Mod(m_actionProgress*3.0f, 1.0f);
			if ( m_object->RetStrong() < 0.0f )  // blupi englué ?
			{
				prog = 1.0f-prog;
			}

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
			linVib.z = -tanf(cirVib.x)*0.5f;
			linVib.y = -0.6f;
			cirVib.z = -1.0f;  // penché en avant

			if ( prog < 0.25f )
			{
				is = MBLUPI_PUSH1;
				ie = MBLUPI_PUSH2;
				prog = prog/0.25f;
			}
			else if ( prog < 0.50f )
			{
				is = MBLUPI_PUSH2;
				ie = MBLUPI_PUSH3;
				prog = (prog-0.25f)/0.25f;
			}
			else if ( prog < 0.75f )
			{
				is = MBLUPI_PUSH3;
				ie = MBLUPI_PUSH4;
				prog = (prog-0.50f)/0.25f;
			}
			else
			{
				is = MBLUPI_PUSH4;
				ie = MBLUPI_PUSH1;
				prog = (prog-0.75f)/0.25f;
			}

			time *= 2.0f;
			for ( i=1 ; i<max ; i++ )
			{
				m_object->SetAngleX(i, SmoothA(m_object->RetAngleX(i), Prop(m_armAngles[is*3*max+i*3+xx], m_armAngles[ie*3*max+i*3+xx], prog), time));
				m_object->SetAngleY(i, SmoothA(m_object->RetAngleY(i), Prop(m_armAngles[is*3*max+i*3+yy], m_armAngles[ie*3*max+i*3+yy], prog), time));
				m_object->SetAngleZ(i, SmoothA(m_object->RetAngleZ(i), Prop(m_armAngles[is*3*max+i*3+zz], m_armAngles[ie*3*max+i*3+zz], prog), time));
			}
		}

		rot = RotatePoint(-m_object->RetAngleY(0), FPOINT(linVib.x, linVib.z));
		linVib.x = rot.x;
		linVib.z = rot.y;
		pos = m_object->RetLinVibration();
		linVib.x = SmoothA(pos.x, linVib.x, rTime*3.0f);
		linVib.y = SmoothA(pos.y, linVib.y, rTime*3.0f);
		linVib.z = SmoothA(pos.z, linVib.z, rTime*3.0f);

		pos = m_object->RetCirVibration();
		cirVib.x = SmoothA(pos.x, cirVib.x, rTime*3.0f);
		cirVib.y = SmoothA(pos.y, cirVib.y, rTime*3.0f);
		cirVib.z = SmoothA(pos.z, cirVib.z, rTime*3.0f);

		m_object->SetLinVibration(linVib);
		m_object->SetCirVibration(cirVib);

		LimitArticulations(cirVib);
		return TRUE;
	}
	m_walkTime = 0.0f;
	m_bFirstSound = FALSE;

	if ( c >= 0.5f )  // tourne ?
	{
		action = MBLUPI_TURN;
	}

	if ( action == MBLUPI_TURN )  // demi-tour ?
	{
		cirVib.y = -sinf(m_aTime*10.0f)*0.4f;
		linVib.y =  sinf(m_aTime*10.0f)*0.2f-0.2f;

		effect[3* 1+zz] =  (sinf(m_aTime*5.0f)+1.0f)*0.5f;  // jambe droite
		effect[3* 2+zz] = -(sinf(m_aTime*5.0f)+1.0f)*1.0f;
		effect[3* 3+zz] =  (sinf(m_aTime*5.0f)+1.0f)*0.5f;
		effect[3* 4+zz] =  (cosf(m_aTime*5.0f)+1.0f)*0.5f;  // jambe gauche
		effect[3* 5+zz] = -(cosf(m_aTime*5.0f)+1.0f)*1.0f;
		effect[3* 6+zz] =  (cosf(m_aTime*5.0f)+1.0f)*0.5f;
	}

	if ( action == MBLUPI_WAIT )
	{
		if ( m_gimmickTime <= 0.0f )
		{
			m_gimmickTime = 1.0f+Rand()*5.0f;
		}
		m_gimmickTime -= rTime;
		if ( m_gimmickTime <= 0.0f )
		{
			i = rand()%4;
			if ( i == 0 || i == 1 )
			{
				SetAction(MBLUPI_TAPTAP, 1.0f/(2.0f+Rand()*2.0f));
				return TRUE;
			}
			if ( i == 2 )
			{
				SetAction(MBLUPI_PFIOU);
				return TRUE;
			}
			if ( i == 3 )
			{
				SetAction(MBLUPI_HELLO, 1.0f/(2.0f+Rand()*2.0f));
				return TRUE;
			}
		}

		linVib.y -= 0.2f;

		effect[3* 1+yy] =  sinf(m_aTime*0.8f)*0.04f;  // pied droite
		effect[3* 4+yy] = -sinf(m_aTime*0.8f)*0.04f;  // pied gauche

		effect[3* 7+zz] =  sinf(m_aTime*1.0f)*0.12f;  // bras droite
		effect[3* 8+yy] =  sinf(m_aTime*1.3f)*0.08f;
		effect[3*10+zz] =  sinf(m_aTime*1.7f)*0.12f;  // bras droite
		effect[3*11+yy] =  sinf(m_aTime*1.4f)*0.08f;

		effect[3*13+xx] =  sinf(m_aTime*0.5f)*0.08f;  // sourcil droite
		effect[3*14+xx] =  sinf(m_aTime*0.7f)*0.08f;  // sourcil gauche

		time *= 5.0f;
	}

	if ( action == MBLUPI_TRUCK )  // porté par dock ?
	{
		// Le corps se tortille.
		cirVib.x = sinf(m_aTime*10.0f)*0.25f;
		cirVib.z = cosf(m_aTime*10.0f)*0.25f;

		// Correction pour que la tête reste immobile sur l'aimant.
		rot.x = sinf(m_aTime*10.0f)*1.0f;
		rot.y = cosf(m_aTime*10.0f)*1.0f;
//?		rot = RotatePoint(-(m_object->RetAngleY(0)-PI*1.5f), rot);
		rot = RotatePoint(PI*1.5f, rot);
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

	if ( action == MBLUPI_GOAL2 )  // suspendu au ballon ?
	{
		effect[3* 1+zz] =  sinf(m_aTime*12.0f)*0.4f;  // pied droite
		effect[3* 4+zz] = -sinf(m_aTime*12.0f)*0.4f;  // pied gauche
	}

	if ( action == MBLUPI_OUPS )  // perd l'équilibre ?
	{
		effect[3* 1+zz] = Min(m_actionTime, 1.0f)*0.9f;  // jambe droite
		effect[3* 4+zz] = Min(m_actionTime, 1.0f)*0.9f;  // jambe gauche

		effect[3* 7+xx] = -sinf(m_aTime*16.0f)*0.2f;  // bras droite
		effect[3* 7+yy] = -cosf(m_aTime*16.0f)*0.2f;
		effect[3* 8+xx] = -sinf(m_aTime*16.0f)*0.2f;  // avant-bras droite
		effect[3* 8+yy] = -cosf(m_aTime*16.0f)*0.2f;
		effect[3*10+xx] =  sinf(m_aTime*16.0f)*0.2f;  // bras gauche
		effect[3*10+yy] =  cosf(m_aTime*16.0f)*0.2f;
		effect[3*11+xx] =  sinf(m_aTime*16.0f)*0.2f;  // avant-bras gauche
		effect[3*11+yy] =  cosf(m_aTime*16.0f)*0.2f;
		time *= 10.0f;
	}

#if !ADJUST_ANGLE
	if ( action == MBLUPI_ERROR )
	{
		if ( Mod(m_actionTime, 0.4f) < 0.2f )
		{
			action = MBLUPI_ERROR2;
		}
	}
	if ( action == MBLUPI_ERROR  ||
		 action == MBLUPI_ERROR2 )
	{
		UpdateFaceMappingMouth(MBLUPI_SAD);  // triste
		if ( m_progress >= 1.0f )
		{
			UpdateFaceMappingMouth(MBLUPI_HAPPY);
			m_actionType = MBLUPI_WAIT;
			return TRUE;
		}
	}

	if ( action == MBLUPI_HELLO )
	{
		if ( Mod(m_actionTime, 0.6f) >= 0.3f )
		{
			action = MBLUPI_HELLO2;
		}
	}
	if ( action == MBLUPI_HELLO  ||
		 action == MBLUPI_HELLO2 )
	{
		if ( m_progress >= 1.0f )
		{
			m_actionType = MBLUPI_WAIT;
			return TRUE;
		}
		time *= 0.2f;
	}

	if ( action == MBLUPI_AMAZE )
	{
		if ( Mod(m_actionTime, 0.30f) >= 0.15f )
		{
			action = MBLUPI_AMAZE2;
		}
	}
	if ( action == MBLUPI_AMAZE  ||
		 action == MBLUPI_AMAZE2 )
	{
		UpdateFaceMappingMouth(MBLUPI_ANGRY);  // fâché
		if ( m_progress >= 1.0f )
		{
			UpdateFaceMappingMouth(MBLUPI_HAPPY);
			m_actionType = MBLUPI_WAIT;
			return TRUE;
		}
	}

	if ( action == MBLUPI_PFIOU )
	{
		prog = Mod(m_actionTime, 2.0f);
		if ( prog < 0.4f )
		{
			action = MBLUPI_PFIOU;
		}
		else if ( prog < 0.8f )
		{
			action = MBLUPI_PFIOU2;
			time *= 0.05f;
		}
		else
		{
			action = MBLUPI_PFIOU3;
			time *= 0.10f;
		}
	}
	if ( action == MBLUPI_PFIOU  ||
		 action == MBLUPI_PFIOU2 ||
		 action == MBLUPI_PFIOU3 )
	{
		UpdateFaceMappingMouth(MBLUPI_SAD);  // triste
		if ( m_progress >= 1.0f )
		{
			UpdateFaceMappingMouth(MBLUPI_HAPPY);
			m_actionType = MBLUPI_WAIT;
			return TRUE;
		}
	}

	if ( action == MBLUPI_RUSH )
	{
		linVib.y -= 0.3f;
		cirVib.z = (m_actionTime)*(20.0f*PI/180.0f);
		action = MBLUPI_RUSH;
		time *= 0.1f;
		if ( m_progress >= 1.0f )
		{
			m_actionType = MBLUPI_WAIT;
			return TRUE;
		}
	}

	if ( action == MBLUPI_ROLL )
	{
		if ( m_actionTime < 0.5f )
		{
			linVib.y -= 0.3f;
			cirVib.z = (m_actionTime/0.5f)*(20.0f*PI/180.0f);
			action = MBLUPI_ROLL;
			time *= 0.1f;
		}
		else
		{
			linVib.x += sinf(Min((m_actionTime-0.5f)/0.5f, 1.0f))*2.0f;
			action = MBLUPI_ROLL2;
			time *= 0.5f;
		}
	}
	if ( action == MBLUPI_ROLL  ||
		 action == MBLUPI_ROLL2 )
	{
		if ( m_progress >= 1.0f )
		{
			m_actionType = MBLUPI_WAIT;
			return TRUE;
		}
	}

	if ( action == MBLUPI_TRAX )
	{
		if ( m_actionTime < 0.5f )
		{
			linVib.y -= 0.3f;
			cirVib.z = (m_actionTime/0.5f)*(20.0f*PI/180.0f);
			action = MBLUPI_TRAX;
			time *= 0.1f;
		}
		else
		{
			linVib.x += sinf(Min((m_actionTime-0.5f)/0.5f, 1.0f))*1.5f;
			action = MBLUPI_TRAX2;
			time *= 0.5f;
		}
	}
	if ( action == MBLUPI_TRAX  ||
		 action == MBLUPI_TRAX2 )
	{
		if ( m_progress >= 1.0f )
		{
			m_actionType = MBLUPI_WAIT;
			return TRUE;
		}
	}

	if ( action == MBLUPI_TAPTAP )
	{
		if ( Mod(m_actionTime, 0.4f) >= 0.2f )
		{
			action = MBLUPI_TAPTAP2;
		}
	}
	if ( action == MBLUPI_TAPTAP  ||
		 action == MBLUPI_TAPTAP2 )
	{
		if ( m_progress >= 1.0f )
		{
			m_actionType = MBLUPI_WAIT;
			return TRUE;
		}
	}
	if ( action == MBLUPI_DRINK2 )
	{
		UpdateFaceMappingMouth(MBLUPI_ANGRY);  // fâché
		if ( m_progress >= 1.0f )
		{
			m_actionType = MBLUPI_WAIT;
			return TRUE;
		}
	}

	if ( action == MBLUPI_YOUPIE )
	{
		if ( Mod(m_actionTime, 0.6f) >= 0.3f )
		{
			action = MBLUPI_YOUPIE2;
		}
	}
	if ( action == MBLUPI_YOUPIE  ||
		 action == MBLUPI_YOUPIE2 )
	{
		if ( m_progress >= 1.0f )
		{
			m_actionType = MBLUPI_GOAL1;
			return TRUE;
		}
		time *= 1.0f;
	}

	if ( action == MBLUPI_STOP )
	{
		angle = m_startAngle+(m_goalAngle-m_startAngle)*Norm(m_progress*10.0f);
		m_object->SetAngleY(0, angle);
		if ( m_progress >= 1.0f )
		{
			m_actionType = MBLUPI_WAIT;
			return TRUE;
		}
		time *= 0.5f;
	}
#endif

	// Bouge tous les membres d'après la table.
	j = action;
	if ( j <  0      )  j = 0;
	if ( j >= MBLUPI_MAX )  j = MBLUPI_MAX-1;

	for ( i=1 ; i<max ; i++ )
	{
		m_object->SetAngleX(i, SmoothA(m_object->RetAngleX(i), m_armAngles[i*3+xx+j*3*max]*PI/180.0f+effect[i*3+xx], time));
		m_object->SetAngleY(i, SmoothA(m_object->RetAngleY(i), m_armAngles[i*3+yy+j*3*max]*PI/180.0f+effect[i*3+yy], time));
		m_object->SetAngleZ(i, SmoothA(m_object->RetAngleZ(i), m_armAngles[i*3+zz+j*3*max]*PI/180.0f+effect[i*3+zz], time));
	}

	// Gère la position des sourcils.
	pos = D3DVECTOR(0.3f, 3.8f, 0.6f);
	if ( action == MBLUPI_ERROR  ||
		 action == MBLUPI_ERROR2 )
	{
		pos.x += 0.3f;
		pos.y -= 0.3f;
		pos.z -= 0.3f;
	}
	if ( action == MBLUPI_AMAZE  ||
		 action == MBLUPI_AMAZE2 )
	{
		pos.y += 0.2f;
		pos.z += 0.2f;
	}
	if ( action == MBLUPI_PFIOU  ||
		 action == MBLUPI_PFIOU2 ||
		 action == MBLUPI_PFIOU3 )
	{
		pos.x -= 0.2f;
		pos.y += 0.3f;
		pos.z += 0.1f;
	}
	if ( action == MBLUPI_TAPTAP  ||
		 action == MBLUPI_TAPTAP2 )
	{
		pos.y += 0.2f;
	}
	if ( action == MBLUPI_DRINK1 ||
		 action == MBLUPI_DRINK2 )
	{
		pos.y += 0.3f;
		pos.z += 0.1f;
	}
	ipos = m_object->RetPosition(14);
	ppos.x = Smooth(ipos.x, pos.x, rTime*8.0f);
	ppos.y = Smooth(ipos.y, pos.y, rTime*8.0f);
	ppos.z = Smooth(ipos.z, pos.z, rTime*8.0f);
	m_object->SetPosition(14, ppos);  // sourcil gauche

	pos.z = -pos.z;  // symétrique en x
	ipos = m_object->RetPosition(13);
	ppos.x = Smooth(ipos.x, pos.x, rTime*8.0f);
	ppos.y = Smooth(ipos.y, pos.y, rTime*8.0f);
	ppos.z = Smooth(ipos.z, pos.z, rTime*8.0f);
	m_object->SetPosition(13, ppos);  // sourcil droite

	// Gestion de l'inclinaison d'après certains membres.
	if ( action == MBLUPI_ERROR  ||
		 action == MBLUPI_ERROR2 )
	{
		s = m_object->RetAngleY(1);
		cirVib.y = -s;
		linVib.y -= 0.1f;
	}

	if ( action == MBLUPI_HELLO  ||
		 action == MBLUPI_HELLO2 )
	{
		s = m_object->RetAngleZ(4)-m_object->RetAngleZ(1);
		cirVib.x = s*0.25f;
		linVib.z = s*0.5f;
		linVib.y -= 0.2f;
	}

	if ( action == MBLUPI_AMAZE  ||
		 action == MBLUPI_AMAZE2 )
	{
		s = m_object->RetAngleZ(1)-30.0f*PI/180.0f;
		cirVib.z = -s;
		linVib.y -= 0.3f+s*0.5f;
		linVib.x += s*0.5f;
	}

	if ( action == MBLUPI_PFIOU  ||
		 action == MBLUPI_PFIOU2 )
	{
		s = m_object->RetAngleY(1);
		cirVib.y = -s;
		cirVib.z = 10.0f*PI/180.0f;  // penché en arrière
		linVib.y -= 0.2f;
		linVib.x -= 0.3f;
	}
	if ( action == MBLUPI_PFIOU3 )
	{
		linVib.y -= 0.2f;
	}

	if ( action == MBLUPI_ROLL2 )
	{
		s = m_object->RetAngleZ(1)-30.0f*PI/180.0f;
		cirVib.z = -s;
		linVib.y -= 0.6f;
	}

	if ( action == MBLUPI_TRAX2 )
	{
		s = m_object->RetAngleZ(1)-30.0f*PI/180.0f;
		cirVib.z = -s;
		linVib.y -= 0.6f;
	}

	if ( action == MBLUPI_TAPTAP  ||
		 action == MBLUPI_TAPTAP2 )
	{
		s = m_object->RetAngleZ(4)-m_object->RetAngleZ(1);
		cirVib.x = s*0.25f;
		linVib.z = s*0.5f;
		linVib.y -= 0.2f;
	}

	if ( action == MBLUPI_DRINK1 )
	{
		s = m_object->RetAngleZ(1)-30.0f*PI/180.0f;
		cirVib.z = -s;
		linVib.y -= 0.6f;
	}
	if ( action == MBLUPI_DRINK2 )
	{
		s = m_object->RetAngleZ(1)-30.0f*PI/180.0f;
		cirVib.z = -s;
		linVib.y += 0.1f+sinf(m_aTime*20.0f)*0.5f;
	}

	if ( action == MBLUPI_YOUPIE  ||
		 action == MBLUPI_YOUPIE2 )
	{
		s = m_object->RetAngleZ(4)-m_object->RetAngleZ(1);
		cirVib.x = s*0.25f;
		linVib.z = s*0.5f;
		linVib.y -= 0.3f;
	}

	if ( action == MBLUPI_OUPS )
	{
		s = m_object->RetAngleZ(1)+m_object->RetAngleZ(2)+m_object->RetAngleZ(3);
		cirVib.z = -s;
	}

	rot = RotatePoint(-m_object->RetAngleY(0), FPOINT(linVib.x, linVib.z));
	linVib.x = rot.x;
	linVib.z = rot.y;
	m_object->SetLinVibration(linVib);
	m_object->SetCirVibration(cirVib);

	LimitArticulations(cirVib);
	return TRUE;
}

// Limite certaines articulations.

void CMotionBlupi::LimitArticulations(const D3DVECTOR &cirVib)
{
	float	limit, angle;

	if ( m_additional[ADD_SHOE] == 4 )  // palmes ?
	{
		limit = cirVib.z;
		limit += m_object->RetAngleZ(1);
		limit += m_object->RetAngleZ(2);
		angle = m_object->RetAngleZ(3);
		if ( angle < -limit )
		{
			m_object->SetAngleZ(3, -limit);
		}

		limit = cirVib.z;
		limit += m_object->RetAngleZ(4);
		limit += m_object->RetAngleZ(5);
		angle = m_object->RetAngleZ(6);
		if ( angle < -limit )
		{
			m_object->SetAngleZ(6, -limit);
		}
	}
}

// Gestion des particules lorsque Blupi est fort.

void CMotionBlupi::StrongParticule()
{
	D3DMATRIX*	mat;
	D3DVECTOR	oPos, pos, speed;
	FPOINT		dim;
	float		duration, dist;
	int			i;

	if ( m_lastParticuleStrong+0.10f <= m_aTime )
	{
		m_lastParticuleStrong = m_aTime;

		oPos = m_object->RetPosition(0);

		pos = oPos;
		pos.y += 4.0f;
		speed.x = (Rand()-0.5f)*8.0f;
		speed.z = (Rand()-0.5f)*8.0f;
		speed.y = 10.0f+Rand()*10.0f;
		dim.x = 1.0f;
		dim.y = dim.x;
		m_particule->CreateParticule(pos, speed, dim, PARTILENS4, 1.0f, 40.0f);

		pos = oPos;
		pos.y += 3.0f;
		pos.x += (Rand()-0.5f)*5.0f;
		pos.z += (Rand()-0.5f)*5.0f;
		speed.x = 0.0f;
		speed.z = 0.0f;
		speed.y = 0.0f;
		dim.x = 4.0f+Rand()*6.0f;
		dim.y = dim.x;
		duration = 2.0f+Rand()*1.0f;
		m_particule->CreateParticule(pos, speed, dim, PARTICRASH, duration);

		if ( m_object->RetFret() == 0 )
		{
			dist = Length2d(m_lastPos, oPos);
			if ( dist > 0.0f || rand()%5 == 0 ) // moins souvent si immobile
			{
				m_lastPos = oPos;
				pos = oPos;
				pos.y = 0.1f;
				pos.x += (Rand()-0.5f)*3.0f;
				pos.z += (Rand()-0.5f)*3.0f;
				speed.y = 0.0f;
				speed.x = 0.0f;
				speed.z = 0.0f;
				dim.x = 1.5f+Rand()*1.5f;
				dim.y = dim.x;
				m_particule->CreateParticule(pos, speed, dim, PARTIPOWER, 2.0f, 0.0f);
			}
		}

		if ( m_walk == MBLUPI_PUSH )  // pousse une caisse ?
		{
			for ( i=0 ; i<4 ; i++ )
			{
				mat = m_object->RetWorldMatrix(0);
				pos = Transform(*mat, D3DVECTOR(1.0f, 3.0f, 0.0f));
				speed.x = (Rand()-0.5f)*40.0f;
				speed.z = (Rand()-0.5f)*40.0f;
				speed.y = (Rand()-0.5f)*40.0f;
				dim.x = 0.6f;
				dim.y = dim.x;
				duration = Rand()*0.5f+0.5f;
				m_particule->CreateTrack(pos, speed, dim, PARTITRACK6,
										 duration, 0.0f,
										 duration*0.9f, 0.7f);
			}
		}
	}
}

// Gestion des particules lorsque Blupi est englué.

void CMotionBlupi::GluParticule()
{
	D3DMATRIX*	mat;
	D3DVECTOR	oPos, pos, speed;
	FPOINT		dim;
	float		duration, dist;
	int			i;

	if ( m_lastParticuleStrong+0.10f <= m_aTime )
	{
		m_lastParticuleStrong = m_aTime;

		oPos = m_object->RetPosition(0);

		pos = oPos;
		pos.y += 4.0f;
		speed.x = (Rand()-0.5f)*8.0f;
		speed.z = (Rand()-0.5f)*8.0f;
		speed.y = 10.0f+Rand()*10.0f;
		dim.x = 0.6f;
		dim.y = dim.x;
		m_particule->CreateParticule(pos, speed, dim, PARTIEXPLOG2, 1.0f, 40.0f);

		pos = oPos;
		pos.y += 3.0f;
		pos.x += (Rand()-0.5f)*5.0f;
		pos.z += (Rand()-0.5f)*5.0f;
		speed.x = 0.0f;
		speed.z = 0.0f;
		speed.y = 0.0f;
		dim.x = 1.0f+Rand()*1.0f;
		dim.y = dim.x;
		duration = 2.0f+Rand()*1.0f;
		m_particule->CreateParticule(pos, speed, dim, PARTIEXPLOO, duration);

		if ( m_object->RetFret() == 0 )
		{
			dist = Length2d(m_lastPos, oPos);
			if ( dist > 0.0f || rand()%5 == 0 ) // moins souvent si immobile
			{
				m_lastPos = oPos;
				pos = oPos;
				pos.y = 0.1f;
				pos.x += (Rand()-0.5f)*3.0f;
				pos.z += (Rand()-0.5f)*3.0f;
				speed.y = 0.0f;
				speed.x = 0.0f;
				speed.z = 0.0f;
				dim.x = 1.5f+Rand()*1.5f;
				dim.y = dim.x;
				m_particule->CreateParticule(pos, speed, dim, PARTIGLU, 2.0f, 0.0f);
			}
		}

		if ( m_walk == MBLUPI_PUSH )  // pousse une caisse ?
		{
			for ( i=0 ; i<4 ; i++ )
			{
				mat = m_object->RetWorldMatrix(0);
				pos = Transform(*mat, D3DVECTOR(1.0f, 3.0f, 0.0f));
				speed.x = (Rand()-0.5f)*40.0f;
				speed.z = (Rand()-0.5f)*40.0f;
				speed.y = (Rand()-0.5f)*40.0f;
				dim.x = 0.6f;
				dim.y = dim.x;
				duration = Rand()*0.5f+0.5f;
				m_particule->CreateTrack(pos, speed, dim, PARTITRACK2,
										 duration, 0.0f,
										 duration*0.9f, 0.7f);
			}
		}
	}
}


// Cherche un objet quelconque.

CObject* CMotionBlupi::SearchBot(D3DVECTOR center, float radius)
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

		if ( pObj->RetExplo() )  continue;
		if ( pObj->RetLock() )  continue;  // déjà stoppé ?
		if ( !pObj->RetEnable() )  continue;

		type = pObj->RetType();
		if ( type != OBJECT_CRAZY )  continue;

		pos = pObj->RetPosition(0);
		dist = Length2d(pos, center);

		if ( dist <= radius && dist < min )
		{
			min = dist;
			pBest = pObj;
		}
	}
	return pBest;
}


// Met à jour le mapping de la texture des yeux.

void CMotionBlupi::UpdateFaceMappingEye(int face)
{
	D3DMATERIAL7	mat;
	float			au, bu;

	if ( face == m_faceEye )  return;
	m_faceEye = face;

	ZeroMemory( &mat, sizeof(D3DMATERIAL7) );
	mat.diffuse.r = 1.0f;
	mat.diffuse.g = 1.0f;
	mat.diffuse.b = 1.0f;  // blanc
	mat.ambient.r = 0.5f;
	mat.ambient.g = 0.5f;
	mat.ambient.b = 0.5f;

	au = 0.25f/2.0f/1.4f;
	bu = 0.25f/2.0f+face*0.25f;

	m_engine->ChangeTextureMapping(m_object->RetObjectRank(0),
								   mat, D3DSTATEPART2, "bot2.tga", "",
								   0.0f, 1000000.0f, D3DMAPPING1Z,
								   au, bu, 1.0f, 0.0f);
}

// Met à jour le mapping de la texture de la bouche.

void CMotionBlupi::UpdateFaceMappingMouth(int face)
{
	D3DMATERIAL7	mat;
	float			au, bu;

	if ( face == m_faceMouth )  return;
	m_faceMouth = face;

	ZeroMemory( &mat, sizeof(D3DMATERIAL7) );
	mat.diffuse.r = 1.0f;
	mat.diffuse.g = 1.0f;
	mat.diffuse.b = 1.0f;  // blanc
	mat.ambient.r = 0.5f;
	mat.ambient.g = 0.5f;
	mat.ambient.b = 0.5f;

	au = 0.25f/2.0f/1.4f;
	bu = 0.25f/2.0f+face*0.25f;

	m_engine->ChangeTextureMapping(m_object->RetObjectRank(0),
								   mat, D3DSTATEPART1, "bot2.tga", "",
								   0.0f, 1000000.0f, D3DMAPPING1Z,
								   au, bu, 1.0f, 0.0f);
}


// Détermine le son à utiliser pour la marche (pour un pas).

void CMotionBlupi::StepSound()
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

	if ( m_walk == MBLUPI_PUSH )  // pousse une caisse ?
	{
		frequency /= 1.5f;
	}

	frequency *= 0.5f;
	m_sound->Play(sound, pos, amplitude, frequency);
}


// Fait monter blupi.

void CMotionBlupi::BrainFrameDemo()
{
	D3DVECTOR	pos, angle;
	float		ih, dh, delay;
	int			i;

	pos = m_object->RetPosition(0);
	if ( pos.z > -160.0f )
	{
		ih = -300.0f;  // hauteur initiale
		dh =  250.0f;  // total montée
	}
	else
	{
		ih = -150.0f;  // hauteur initiale
		dh =  110.0f;  // total montée
	}
	delay = dh/10.0f;

	if ( m_bTime > delay )
	{
		m_bTime = 0.0f;

		for ( i=0 ; i<OBJECTMAXPART ; i++ )
		{
			m_object->DeletePart(i);
		}

		if ( rand()%2 == 0 )
		{
			m_object->SetAdditional(ADD_HAT,   rand()%7);
			m_object->SetAdditional(ADD_GLASS, 0);
			m_object->SetAdditional(ADD_GLOVE, 0);
			m_object->SetAdditional(ADD_SHOE,  rand()%5);
			m_object->SetAdditional(ADD_BAG,   rand()%4);
		}
		else
		{
			m_object->SetAdditional(ADD_HAT,   0);
			m_object->SetAdditional(ADD_GLASS, rand()%5);
			m_object->SetAdditional(ADD_GLOVE, rand()%2);
			m_object->SetAdditional(ADD_SHOE,  rand()%5);
			m_object->SetAdditional(ADD_BAG,   rand()%4);
		}

		pos = m_object->RetPosition(0);
		Create(pos, 0.0f, OBJECT_BLUPI);
	}

	pos.y = ih+(m_bTime/delay)*dh;
	m_object->SetPosition(0, pos);

	angle.x = sinf(m_bTime*5.97f)*0.06f+sinf(m_bTime*3.46f)*0.03f;
	angle.z = sinf(m_bTime*5.17f)*0.06f+sinf(m_bTime*3.61f)*0.03f;
	angle.y = m_bTime;
	m_object->SetAngle(0, angle);

	if ( m_object->RetID() == 2 )
	{
		pos.x =    0.0f+sinf(m_aTime*0.171f)*4.0f;
		pos.y =  -50.0f+sinf(m_aTime*0.152f)*4.0f;
		pos.z = -200.0f+sinf(m_aTime*0.167f)*4.0f;
		m_camera->SetScriptEye(pos);

		pos.x =    0.0f+sinf(m_aTime*0.158f)*4.0f;
		pos.y = -200.0f+sinf(m_aTime*0.181f)*4.0f;
		pos.z =  -50.0f+sinf(m_aTime*0.173f)*4.0f;
		m_camera->SetScriptLookat(pos);

		m_camera->FixCamera();
	}
}

