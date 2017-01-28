// motionbird.cpp

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
#include "motionbird.h"



#define ADJUST_MOVE		FALSE		// TRUE -> ajuste les mouvements


#define FLY_HEIGHT		25.0f		// hauteur standard de vol




// Constructeur de l'objet.

CMotionBird::CMotionBird(CInstanceManager* iMan, CObject* object)
							  : CMotion(iMan, object)
{
	CMotion::CMotion(iMan, object);

	m_progress = 1.0f;
	m_total = 0;
}

// Destructeur de l'objet.

CMotionBird::~CMotionBird()
{
}


// Supprime un objet.

void CMotionBird::DeleteObject(BOOL bAll)
{
}


// Démarre une action spéciale.

Error CMotionBird::SetAction(int action, float time)
{
	return CMotion::SetAction(action, time);
}


// Retourne la vitesse linéaire.

float CMotionBird::RetLinSpeed()
{
	return 5.0f;
}

// Retourne la vitesse linéaire.

float CMotionBird::RetCirSpeed()
{
	return 0.5f*PI;
}

// Retourne la distance linéaire de freinage.

float CMotionBird::RetLinStopLength()
{
	return 2.0f;
}


// Crée un oiseau.

BOOL CMotionBird::Create(D3DVECTOR pos, float angle, ObjectType type)
{
	CModFile*	pModFile;
	int			rank, option, i;

	float	tOffset[10] =
	{
		  0.0f,
		 11.0f,
		-12.0f,
		 24.0f,
		-23.0f,
		 35.0f,
		-34.0f,
		 48.0f,
		-46.0f,
		 60.0f,
	};

	if ( m_engine->RetRestCreate() < 10 )  return FALSE;
	
	m_object->SetType(type);
	pModFile = new CModFile(m_iMan);

	option = m_object->RetOption();
	if ( option > OBJECTMAXPART/5-1 )  option = OBJECTMAXPART/5-1;
	if ( option > 10 )  option = 10;
	m_total = option;  // nombre d'oiseaux

	// Crée l'objet principal, inexistant !.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEOBJECT);  // c'est un objet fixe
	m_object->SetObjectRank(0, rank);
	pModFile->ReadModel("objects\\bird0.mod");  // n'existe pas !
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(0, pos);
	m_object->SetAngleY(0, angle);

	for ( i=0 ; i<m_total ; i++ )
	{
		// Crée le corps principal.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEOBJECT);  // c'est un objet fixe
		m_object->SetObjectRank(i*5+1, rank);
		m_object->SetObjectParent(i*5+1, 0);
		pModFile->ReadModel("objects\\bird1.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(i*5+1, pos);
		m_object->SetAngleY(i*5+1, angle);

		// Crée l'aile droite.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(i*5+2, rank);
		m_object->SetObjectParent(i*5+2, i*5+1);
		pModFile->ReadModel("objects\\bird2.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(i*5+2, D3DVECTOR(0.0f, 0.0f, -0.4f));

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(i*5+3, rank);
		m_object->SetObjectParent(i*5+3, i*5+2);
		pModFile->ReadModel("objects\\bird3.mod");
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(i*5+3, D3DVECTOR(0.0f, 0.0f, -2.1f));

		// Crée l'aile gauche.
		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(i*5+4, rank);
		m_object->SetObjectParent(i*5+4, i*5+1);
		pModFile->ReadModel("objects\\bird2.mod");
		pModFile->Mirror();
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(i*5+4, D3DVECTOR(0.0f, 0.0f, 0.4f));

		rank = m_engine->CreateObject();
		m_engine->SetObjectType(rank, TYPEDESCENDANT);
		m_object->SetObjectRank(i*5+5, rank);
		m_object->SetObjectParent(i*5+5, i*5+4);
		pModFile->ReadModel("objects\\bird3.mod");
		pModFile->Mirror();
		pModFile->CreateEngineObject(rank);
		m_object->SetPosition(i*5+5, D3DVECTOR(0.0f, 0.0f, 2.1f));

		m_birdTable[i].speed = 1.0f+(Rand()-0.5f)*0.4f;
		m_birdTable[i].time  = Rand()*100.0f;
		m_birdTable[i].pos.x = 0.0f;
		m_birdTable[i].pos.y = 0.0f;
		m_birdTable[i].pos.z = tOffset[i];  // espacement

		CreateShadow(i, 5.0f, 1.0f, D3DSHADOWBIRD);
	}

	pos = m_object->RetPosition(0);
	m_object->SetPosition(0, pos);  // pour afficher les ombres tout de suite

//?	m_engine->LoadAllTexture();

	delete pModFile;
	return TRUE;
}


// Gestion d'un événement.

BOOL CMotionBird::EventProcess(const Event &event)
{
	CMotion::EventProcess(event);

	if ( event.event == EVENT_FRAME )
	{
		return EventFrame(event);
	}

	return TRUE;
}

// Gestion d'un événement.

BOOL CMotionBird::EventFrame(const Event &event)
{
	D3DVECTOR	pos, linVib, cirVib;
	float		progress, angle;
	int			i, *table1, *table2;

	static int tableA1Down[3] = { 80, -20, -70};
	static int tableA1Up  [3] = { 80,   0, -70};
	static int tableA2Down[3] = {-20,  20, -40};
	static int tableA2Up  [3] = {-20, -90, -40};

	if ( m_engine->RetPause() )  return TRUE;

	// Gestion de la position globale de tous les oiseaux.
	if ( m_progress >= 1.0f )
	{
#if ADJUST_MOVE
		m_startPos.x = 0.0f;
		m_startPos.z = 0.0f;
		m_startPos.y = 0.0f;
		m_goalPos.x  = 0.0f;
		m_goalPos.z  = 0.0f;
		m_goalPos.y  = 0.0f;

		m_object->SetAngleY(0, 45.0f*PI/180.0f);

		m_progress = 0.0f;
		m_speed = 1.0f/100.0f;
#else
		m_startPos.x = -300.0f;
		m_startPos.z = (Rand()-0.5f)*200.0f;
		m_startPos.y = 0.0f;
		m_goalPos.x  = 300.0f;
		m_goalPos.z  = (Rand()-0.5f)*200.0f;
		m_goalPos.y  = 0.0f;

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
		m_speed = 1.0f/(60.0f+Rand()*10.0f);
#endif
	}
	m_progress += event.rTime*m_speed;

	pos = m_startPos+(m_goalPos-m_startPos)*m_progress;
	m_object->SetPosition(0, pos);  // déplace tout le groupe

	// Gestion des oiseaux individuels.
	for ( i=0 ; i<m_total ; i++ )
	{
		// Progression du temps propre à chaque oiseau.
		m_birdTable[i].time += event.rTime*m_birdTable[i].speed;

		// Choix des angles pour les ailes.
		progress = Mod(m_birdTable[i].time, 2.0f)/2.0f;

		if ( progress < 0.75f )  // l'aile descend ?
		{
			progress = progress/0.75f;
			if ( progress < 0.5f )
			{
				progress = progress/0.5f;
				table1 = tableA1Down+0;
				table2 = tableA2Down+0;
			}
			else
			{
				progress = (progress-0.5f)/0.5f;
				table1 = tableA1Down+1;
				table2 = tableA2Down+1;
			}
		}
		else	// l'aile remonte ?
		{
			progress = 1.0f-((progress-0.75f)/0.25f);
			if ( progress < 0.5f )
			{
				progress = progress/0.5f;
				table1 = tableA1Up+0;
				table2 = tableA2Up+0;
			}
			else
			{
				progress = (progress-0.5f)/0.5f;
				table1 = tableA1Up+1;
				table2 = tableA2Up+1;
			}
		}

		angle = table1[0]+(table1[1]-table1[0])*progress;
		angle = angle*PI/180.0f;
		m_object->SetAngleX(i*5+2,  angle);
		m_object->SetAngleX(i*5+4, -angle);

		angle = table2[0]+(table2[1]-table2[0])*progress;
		angle = angle*PI/180.0f;
		m_object->SetAngleX(i*5+3,  angle);
		m_object->SetAngleX(i*5+5, -angle);

		// Mouvement relatif de chaque oiseau.
		linVib.x = 0.0f;
		linVib.y = FLY_HEIGHT;
		linVib.z = 0.0f;
		cirVib.x = 0.0f;
		cirVib.y = 0.0f;
		cirVib.z = 0.0f;

		// Mouvement synchrone avec les ailes.
		progress = Mod(m_birdTable[i].time, 2.0f)/2.0f;
		angle = progress*PI*2.0f;
		linVib.y += -sinf(angle)*2.0f;
		cirVib.z += -cosf(angle)*0.1f;

		// Mouvement très lent.
		linVib.x += sinf(m_birdTable[i].time*0.12f)*20.0f;
		linVib.y += sinf(m_birdTable[i].time*0.25f)*4.0f;
		linVib.z += sinf(m_birdTable[i].time*0.43f)*1.0f;

		// Mouvement faible et rapide pseudo-aléatoire.
		cirVib.x += sinf(m_birdTable[i].time*5.42f)*0.02f;
		cirVib.x += sinf(m_birdTable[i].time*4.33f)*0.02f;
		cirVib.y += sinf(m_birdTable[i].time*3.18f)*0.02f;
		cirVib.y += sinf(m_birdTable[i].time*4.07f)*0.02f;
		cirVib.z += sinf(m_birdTable[i].time*5.60f)*0.02f;
		cirVib.z += sinf(m_birdTable[i].time*4.93f)*0.02f;

		cirVib.x += sinf(m_birdTable[i].time*15.32f)*0.005f;
		cirVib.x += sinf(m_birdTable[i].time*17.08f)*0.005f;
		cirVib.y += sinf(m_birdTable[i].time*16.90f)*0.005f;
		cirVib.y += sinf(m_birdTable[i].time*18.45f)*0.005f;
		cirVib.z += sinf(m_birdTable[i].time*15.33f)*0.005f;
		cirVib.z += sinf(m_birdTable[i].time*18.12f)*0.005f;

		linVib += m_birdTable[i].pos;
		m_object->SetPosition(i*5+1, linVib);
		m_object->SetAngle(i*5+1, cirVib);

		// Gestion de l'ombre.
		progress = Mod(m_birdTable[i].time-0.5f, 2.0f)/2.0f;
		MoveShadow(i, progress);
	}

	return TRUE;
}


// Crée l'ombre circulaire sous un oiseau.

BOOL CMotionBird::CreateShadow(int i, float radius, float intensity,
							   D3DShadowType type)
{
	int		objRank;

	objRank = m_object->RetObjectRank(i*5+1);
	m_engine->ShadowCreate(objRank);

	m_engine->SetObjectShadowRadius(objRank, radius);
	m_engine->SetObjectShadowIntensity(objRank, intensity);
	m_engine->SetObjectShadowHeight(objRank, 0.0f);
	m_engine->SetObjectShadowType(objRank, type);

	return TRUE;
}

// Déplace les ombres des oiseaux.

void CMotionBird::MoveShadow(int i, float progress)
{
	D3DMATRIX*	mat;
	D3DVECTOR	pos;
	float		radius;
	int			objRank;

	objRank = m_object->RetObjectRank(i*5+1);

	mat = m_object->RetWorldMatrix(i*5+1);
	pos = Transform(*mat, D3DVECTOR(0.0f, 0.0f, 0.0f));
	pos.y = 0.0f;

	if ( m_terrain->ValidPosition(pos, 16.0f) )
	{
		radius = 5.0f-sinf(progress*PI)*2.0f;
		m_engine->SetObjectShadowHide(objRank, FALSE);
		m_engine->SetObjectShadowPos(objRank, pos);
		m_engine->SetObjectShadowHeight(objRank, 0.0f);
		m_engine->SetObjectShadowRadius(objRank, radius);
		m_engine->SetObjectShadowAngle(objRank, m_object->RetAngleY(0)+m_object->RetAngleY(i*5+1));
	}
	else
	{
		m_engine->SetObjectShadowHide(objRank, TRUE);
	}
}

