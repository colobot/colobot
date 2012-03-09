// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see .

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
#include "motion.h"
#include "motionspider.h"



#define ADJUST_ANGLE	FALSE		// TRUE -> ajuste les angles des membres
#define START_TIME		1000.0f		// d�but du temps relatif



// Constructeur de l'objet.

CMotionSpider::CMotionSpider(CInstanceManager* iMan, CObject* object)
							: CMotion(iMan, object)
{
	CMotion::CMotion(iMan, object);

	m_armMember      = START_TIME;
	m_armTimeAbs     = START_TIME;
	m_armTimeMarch   = START_TIME;
	m_armTimeAction  = START_TIME;
	m_armTimeIndex   = 0;
	m_armPartIndex   = 0;
	m_armMemberIndex = 0;
	m_armLastAction  = -1;
	m_bArmStop = FALSE;
	m_lastParticule = 0.0f;
}

// Destructeur de l'objet.

CMotionSpider::~CMotionSpider()
{
}


// Supprime un objet.

void CMotionSpider::DeleteObject(BOOL bAll)
{
}


// Cr�e un v�hicule roulant quelconque pos� sur le sol.

BOOL CMotionSpider::Create(D3DVECTOR pos, float angle, ObjectType type,
						   float power)
{
	CModFile*	pModFile;
	int			rank, i, j, parent;
	char		name[50];

	float			table[] =
	{
	//	  x		  y		  z
		 0.6f,	 0.0f,	 0.0f,	// patte arri�re
		 0.0f,	 0.0f,	-2.0f,
		 0.0f,	 0.0f,	-2.0f,
		 0.0f,	 0.0f,	-2.0f,

		 0.8f,	 0.0f,	-0.2f,	// patte arri�re-milieu
		 0.0f,	 0.0f,	-2.0f,
		 0.0f,	 0.0f,	-2.0f,
		 0.0f,	 0.0f,	-2.0f,

		 1.0f,	 0.0f,	-0.2f,	// patte avant-milieu
		 0.0f,	 0.0f,	-2.0f,
		 0.0f,	 0.0f,	-2.0f,
		 0.0f,	 0.0f,	-2.0f,

		 1.2f,	 0.0f,	 0.0f,	// patte avant
		 0.0f,	 0.0f,	-2.0f,
		 0.0f,	 0.0f,	-2.0f,
		 0.0f,	 0.0f,	-2.0f,
	};

	if ( m_engine->RetRestCreate() < 3+32+2 )  return FALSE;

	pModFile = new CModFile(m_iMan);

	m_object->SetType(type);

	// Cr�e la base principale.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEVEHICULE);  // c'est un objet mobile
	m_object->SetObjectRank(0, rank);
	pModFile->ReadModel("objects\\spider0.mod");  // n'existe pas
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(0, pos);
	m_object->SetAngleY(0, angle);

	// Un v�hicule doit avoir obligatoirement une sph�re de
	// collision avec un centre (0;y;0) (voir GetCrashSphere).
	m_object->CreateCrashSphere(D3DVECTOR(0.0f, -2.0f, 0.0f), 4.0f, SOUND_BOUM, 0.20f);
	m_object->SetGlobalSphere(D3DVECTOR(-0.5f, 1.0f, 0.0f), 4.0f);

	// Cr�e l'abdomen.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEDESCENDANT);
	m_object->SetObjectRank(1, rank);
	m_object->SetObjectParent(1, 0);
	pModFile->ReadModel("objects\\spider1.mod");
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(1, D3DVECTOR(1.0f, 0.0f, 0.0f));

	// Cr�e la t�te.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEDESCENDANT);
	m_object->SetObjectRank(2, rank);
	m_object->SetObjectParent(2, 0);
	pModFile->ReadModel("objects\\spider2.mod");
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(2, D3DVECTOR(1.0f, 0.0f, 0.0f));

	// Cr�e les pattes.
	for ( i=0 ; i<4 ; i++ )
	{
		for ( j=0 ; j<4 ; j++ )
		{
			sprintf(name, "objects\\spider%d.mod", j+3);  // 3..6

			// Cr�e la patte droite.
			rank = m_engine->CreateObject();
			m_engine->SetObjectType(rank, TYPEDESCENDANT);
			m_object->SetObjectRank(3+i*4+j, rank);
			if ( j == 0 )  parent = 0;
			else           parent = 3+i*4+j-1;
			m_object->SetObjectParent(3+i*4+j, parent);
			pModFile->ReadModel(name);
			pModFile->CreateEngineObject(rank);
			pos.x = table[i*12+j*3+0];
			pos.y = table[i*12+j*3+1];
			pos.z = table[i*12+j*3+2];
			m_object->SetPosition(3+i*4+j, pos);

			// Cr�e la patte gauche.
			rank = m_engine->CreateObject();
			m_engine->SetObjectType(rank, TYPEDESCENDANT);
			m_object->SetObjectRank(19+i*4+j, rank);
			if ( j == 0 )  parent = 0;
			else           parent = 19+i*4+j-1;
			m_object->SetObjectParent(19+i*4+j, parent);
			pModFile->ReadModel(name);
			pModFile->Mirror();
			pModFile->CreateEngineObject(rank);
			pos.x =  table[i*12+j*3+0];
			pos.y =  table[i*12+j*3+1];
			pos.z = -table[i*12+j*3+2];
			m_object->SetPosition(19+i*4+j, pos);
		}
	}

	// Cr�e la mandibule droite.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEDESCENDANT);
	m_object->SetObjectRank(35, rank);
	m_object->SetObjectParent(35, 1);
	pModFile->ReadModel("objects\\spider7.mod");
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(35, D3DVECTOR(0.0f, 0.0f, -0.3f));

	// Cr�e la mandibule gauche.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEDESCENDANT);
	m_object->SetObjectRank(36, rank);
	m_object->SetObjectParent(36, 1);
	pModFile->ReadModel("objects\\spider7.mod");
	pModFile->Mirror();
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(36, D3DVECTOR(0.0f, 0.0f, 0.3f));

	m_object->CreateShadowCircle(4.0f, 0.5f);

	CreatePhysics();
	m_object->SetFloorHeight(0.0f);

	pos = m_object->RetPosition(0);
	m_object->SetPosition(0, pos);  // pour afficher les ombres tout de suite

	m_engine->LoadAllTexture();

	delete pModFile;
	return TRUE;
}

// Cr�e la physique de l'objet.

void CMotionSpider::CreatePhysics()
{
	Character*	character;
	int			i;

	int member_march[] =
	{
	//	x1,y1,z1,	x2,y2,z2,	x3,y3,z3,	x4,y4,z4,	// en l'air :
		60,25,0,	60,0,0,		60,-25,0,	60,-50,0,	// t0: cuisses 1..4
		-35,40,0,	-35,0,0,	-35,0,0,	-35,-40,0,	// t0: jambes 1..4
		-65,0,-30,	-65,0,0,	-65,0,0,	-65,0,30,	// t0: pieds 1..4
		25,0,0,		25,0,0,		25,0,0,		25,0,0,		// t0: doigt 1..4
														// au sol devant :
		30,15,0,	30,-10,0,	30,-35,0,	30,-60,0,	// t1: cuisses 1..4
		-10,40,0,	-45,0,0,	-45,0,0,	-45,-40,0,	// t1: jambes 1..4
		-90,0,0,	-20,0,0,	-20,0,0,	-20,0,0,	// t1: pieds 1..4
		-5,0,0,		-5,0,0,		-5,0,0,		-5,0,0,		// t1: doigt 1..4
														// au sol derri�re :
		35,35,0,	40,10,0,	40,-15,0,	40,-40,0,	// t2: cuisses 1..4
		-35,40,0,	-35,0,0,	-35,0,0,	-25,-40,0,	// t2: jambes 1..4
		-50,-25,-30,-65,0,0,	-65,0,0,	-90,0,30,	// t2: pieds 1..4
		-5,0,0,		-5,0,0,		-5,0,0,		-5,0,0,		// t2: doigt 1..4
	};

	int member_stop[] =
	{
	//	x1,y1,z1,	x2,y2,z2,	x3,y3,z3,	x4,y4,z4,	// en l'air :
		35,35,0,	30,0,0,		30,-25,0,	30,-50,0,	// t0: cuisses 1..4
		-35,40,0,	-45,0,0,	-45,0,0,	-45,-40,0,	// t0: jambes 1..4
		-50,-25,-30,-20,0,0,	-20,0,0,	-20,0,30,	// t0: pieds 1..4
		-5,0,0,		-5,0,0,		-5,0,0,		-5,0,0,		// t0: doigt 1..4
														// au sol devant :
		35,35,0,	30,0,0,		30,-25,0,	30,-50,0,	// t1: cuisses 1..4
		-30,40,0,	-40,0,0,	-40,0,0,	-40,-40,0,	// t1: jambes 1..4
		-55,-25,-30,-25,0,0,	-25,0,0,	-25,0,0,	// t1: pieds 1..4
		-5,0,0,		-5,0,0,		-5,0,0,		-5,0,0,		// t1: doigt 1..4
														// au sol derri�re :
		35,35,0,	30,0,0,		30,-25,0,	30,-50,0,	// t2: cuisses 1..4
		-30,40,0,	-40,0,0,	-40,0,0,	-40,-40,0,	// t2: jambes 1..4
		-50,-25,-30,-20,0,0,	-20,0,0,	-20,0,30,	// t2: pieds 1..4
		-10,0,0,	-10,0,0,	-10,0,0,	-10,0,0,	// t2: doigt 1..4
	};

	int member_spec[] =
	{
	//	x1,y1,z1,	x2,y2,z2,	x3,y3,z3,	x4,y4,z4,	// br�le :
		30,25,0,	30,0,0,		30,-25,0,	30,-50,0,	// s0: cuisses 1..4
		-45,0,0,	-45,0,0,	-45,0,0,	-45,0,0,	// s0: jambes 1..4
		-20,0,0,	-20,0,0,	-20,0,0,	-20,0,0,	// s0: pieds 1..4
		-5,0,0,		-5,0,0,		-5,0,0,		-5,0,0,		// s0: doigt 1..4
														// ruine :
		30,25,0,	30,0,0,		30,-25,0,	30,-50,0,	// s1: cuisses 1..4
		-45,0,0,	-45,0,0,	-45,0,0,	-45,0,0,	// s1: jambes 1..4
		-20,0,0,	-20,0,0,	-20,0,0,	-20,0,0,	// s1: pieds 1..4
		-5,0,0,		-5,0,0,		-5,0,0,		-5,0,0,		// s1: doigt 1..4
														// explose :
		40,25,0,	40,0,0,		40,-25,0,	40,-50,0,	// s2: cuisses 1..4
		-55,0,0,	-55,0,0,	-55,0,0,	-55,0,0,	// s2: jambes 1..4
		-30,0,0,	-30,0,0,	-30,0,0,	-30,0,0,	// s2: pieds 1..4
		-5,0,0,		-5,0,0,		-5,0,0,		-5,0,0,		// s2: doigt 1..4
														// back1 :
		35,35,0,	30,0,0,		30,-25,0,	30,-50,0,	// s3: cuisses 1..4
		-30,40,0,	-40,0,0,	-40,0,0,	-40,-40,0,	// s3: jambes 1..4
		-55,-25,-30,-25,0,0,	-25,0,0,	-25,0,0,	// s3: pieds 1..4
		-5,0,0,		-5,0,0,		-5,0,0,		-5,0,0,		// s3: doigt 1..4
														// back2 :
		15,35,0,	15,0,0,		15,-25,0,	15,-50,0,	// s4: cuisses 1..4
		-60,40,0,	-60,0,0,	-60,0,0,	-60,-40,0,	// s4: jambes 1..4
		-65,-25,-30,-65,0,0,	-65,0,0,	-65,0,0,	// s4: pieds 1..4
		-15,0,0,	-15,0,0,	-15,0,0,	-15,0,0,	// s4: doigt 1..4
														// back3 :
		35,35,0,	30,0,0,		30,-25,0,	30,-50,0,	// s5: cuisses 1..4
		-30,40,0,	-40,0,0,	-40,0,0,	-40,-40,0,	// s5: jambes 1..4
		-55,-25,-30,-25,0,0,	-25,0,0,	-25,0,0,	// s5: pieds 1..4
		-5,0,0,		-5,0,0,		-5,0,0,		-5,0,0,		// s5: doigt 1..4
	};

	m_physics->SetType(TYPE_ROLLING);

	character = m_object->RetCharacter();
	character->wheelFront = 4.0f;
	character->wheelBack  = 4.0f;
	character->wheelLeft  = 6.0f;
	character->wheelRight = 6.0f;
	character->height     = 0.6f;

	m_physics->SetLinMotionX(MO_ADVSPEED,  12.0f);
	m_physics->SetLinMotionX(MO_RECSPEED,  12.0f);
	m_physics->SetLinMotionX(MO_ADVACCEL,  15.0f);
	m_physics->SetLinMotionX(MO_RECACCEL,  15.0f);
	m_physics->SetLinMotionX(MO_STOACCEL,  40.0f);
	m_physics->SetLinMotionX(MO_TERSLIDE,   5.0f);
	m_physics->SetLinMotionZ(MO_TERSLIDE,   5.0f);
	m_physics->SetLinMotionX(MO_TERFORCE,   5.0f);
	m_physics->SetLinMotionZ(MO_TERFORCE,   5.0f);
	m_physics->SetLinMotionZ(MO_MOTACCEL,  10.0f);

	m_physics->SetCirMotionY(MO_ADVSPEED,   1.0f*PI);
	m_physics->SetCirMotionY(MO_RECSPEED,   1.0f*PI);
	m_physics->SetCirMotionY(MO_ADVACCEL,  20.0f);
	m_physics->SetCirMotionY(MO_RECACCEL,  20.0f);
	m_physics->SetCirMotionY(MO_STOACCEL,  40.0f);

	for ( i=0 ; i<3*4*4*3 ; i++ )
	{
		m_armAngles[3*4*4*3*MS_MARCH+i] = member_march[i];
	}
	for ( i=0 ; i<3*4*4*3 ; i++ )
	{
		m_armAngles[3*4*4*3*MS_STOP+i] = member_stop[i];
	}
	for ( i=0 ; i<3*4*4*6 ; i++ )
	{
		m_armAngles[3*4*4*3*MS_SPEC+i] = member_spec[i];
	}
}


// Gestion d'un �v�nement.

BOOL CMotionSpider::EventProcess(const Event &event)
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

		if ( event.param == 'A' )  m_armTimeIndex++;
		if ( m_armTimeIndex >= 3 )  m_armTimeIndex = 0;

		if ( event.param == 'Q' )  m_armPartIndex++;
		if ( m_armPartIndex >= 4 )  m_armPartIndex = 0;

		if ( event.param == 'W' )  m_armMemberIndex++;
		if ( m_armMemberIndex >= 4 )  m_armMemberIndex = 0;

		i  = m_armMemberIndex*3;
		i += m_armPartIndex*3*4;
		i += m_armTimeIndex*3*4*4;

		if ( event.param == 'E' )  m_armAngles[i+0] += 5;
		if ( event.param == 'D' )  m_armAngles[i+0] -= 5;
		if ( event.param == 'R' )  m_armAngles[i+1] += 5;
		if ( event.param == 'F' )  m_armAngles[i+1] -= 5;
		if ( event.param == 'T' )  m_armAngles[i+2] += 5;
		if ( event.param == 'G' )  m_armAngles[i+2] -= 5;
		if ( event.param == 'Z' )  m_armAngles[i+3] += 5;
		if ( event.param == 'H' )  m_armAngles[i+3] -= 5;

		if ( event.param == 'Y' )  m_bArmStop = !m_bArmStop;
#endif
	}

	return TRUE;
}

// Calcule une valeur (radians) proportionnelle comprise
// entre a et b (degr�s).

inline float Propf(float a, float b, float p)
{
	float	aa, bb;

	aa = a*PI/180.0f;
	bb = b*PI/180.0f;

	return aa+p*(bb-aa);
}

// Gestion d'un �v�nement.

BOOL CMotionSpider::EventFrame(const Event &event)
{
	D3DVECTOR	dir, pos, speed;
	FPOINT		dim;
	float		s, a, prog, time;
	float		tSt[12], tNd[12];
	int			i, ii, st, nd, action;
	BOOL		bStop;

	if ( m_engine->RetPause() )  return TRUE;
	if ( !m_engine->IsVisiblePoint(m_object->RetPosition(0)) )  return TRUE;

	s =     m_physics->RetLinMotionX(MO_MOTSPEED)*1.5f;
	a = Abs(m_physics->RetCirMotionY(MO_MOTSPEED)*2.0f);

	if ( s == 0.0f && a != 0.0f )  a *= 1.5f;

	m_armTimeAbs += event.rTime;
	m_armTimeAction += event.rTime;
	m_armTimeMarch += (s)*event.rTime*0.15f;
	m_armMember += (s+a)*event.rTime*0.15f;

	bStop = ( a == 0.0f && s == 0.0f );  // a l'arr�t ?

	action = MS_MARCH;  // marche
	if ( s == 0.0f && a == 0.0f )
	{
		action = MS_STOP;  // stop
	}

	if ( bStop )
	{
		prog = Mod(m_armTimeAbs, 2.0f)/10.0f;
		a = Mod(m_armMember, 1.0f);
		a = (prog-a)*event.rTime*2.0f;  // vient gentiment � position stop
		m_armMember += a;
	}

	if ( m_object->RetRuin() )  // ruine ?
	{
		m_actionType = MSS_RUIN;
	}
	if ( m_object->RetBurn() )  // br�le ?
	{
		if ( m_object->RetFixed() )
		{
			m_actionType = MSS_BURN;
		}
		else
		{
			m_actionType = -1;
		}
	}

	for ( i=0 ; i<8 ; i++ )  // les 8 pattes
	{
		if ( m_actionType != -1 )  // action sp�ciale en cours ?
		{
			st = 3*4*4*3*MS_SPEC + 3*4*4*m_actionType + (i%4)*3;
			nd = st;
			time = event.rTime*m_actionTime;
			m_armTimeAction = 0.0f;
		}
		else
		{
//?			if ( i < 4 )  prog = Mod(m_armMember+(2.0f-(i%4))*0.25f+0.0f, 1.0f);
//?			else          prog = Mod(m_armMember+(2.0f-(i%4))*0.25f+0.3f, 1.0f);
			if ( i < 4 )  prog = Mod(m_armMember+(2.0f-(i%4))*0.25f+0.0f, 1.0f);
			else          prog = Mod(m_armMember+(2.0f-(i%4))*0.25f+0.5f, 1.0f);
			if ( m_bArmStop )
			{
				prog = (float)m_armTimeIndex/3.0f;
				action = MS_MARCH;
			}
			if ( prog < 0.33f )  // t0..t1 ?
			{
				prog = prog/0.33f;  // 0..1
				st = 0;  // index start
				nd = 1;  // index end
			}
			else if ( prog < 0.67f )  // t1..t2 ?
			{
				prog = (prog-0.33f)/0.33f;  // 0..1
				st = 1;  // index start
				nd = 2;  // index end
			}
			else	// t2..t0 ?
			{
				prog = (prog-0.67f)/0.33f;  // 0..1
				st = 2;  // index start
				nd = 0;  // index end
			}
			st = 3*4*4*3*action + st*3*4*4 + (i%4)*3;
			nd = 3*4*4*3*action + nd*3*4*4 + (i%4)*3;

			// De moins en moins mou ...
//?			time = event.rTime*(2.0f+Min(m_armTimeAction*20.0f, 40.0f));
			time = event.rTime*10.0f;
		}

		tSt[ 0] = m_armAngles[st+ 0];  // x
		tSt[ 1] = m_armAngles[st+ 1];  // y
		tSt[ 2] = m_armAngles[st+ 2];  // z
		tSt[ 3] = m_armAngles[st+12];  // x
		tSt[ 4] = m_armAngles[st+13];  // y
		tSt[ 5] = m_armAngles[st+14];  // z
		tSt[ 6] = m_armAngles[st+24];  // x
		tSt[ 7] = m_armAngles[st+25];  // y
		tSt[ 8] = m_armAngles[st+26];  // z
		tSt[ 9] = m_armAngles[st+36];  // x
		tSt[10] = m_armAngles[st+37];  // y
		tSt[11] = m_armAngles[st+38];  // z

		tNd[ 0] = m_armAngles[nd+ 0];  // x
		tNd[ 1] = m_armAngles[nd+ 1];  // y
		tNd[ 2] = m_armAngles[nd+ 2];  // z
		tNd[ 3] = m_armAngles[nd+12];  // x
		tNd[ 4] = m_armAngles[nd+13];  // y
		tNd[ 5] = m_armAngles[nd+14];  // z
		tNd[ 6] = m_armAngles[nd+24];  // x
		tNd[ 7] = m_armAngles[nd+25];  // y
		tNd[ 8] = m_armAngles[nd+26];  // z
		tNd[ 9] = m_armAngles[nd+36];  // z
		tNd[10] = m_armAngles[nd+37];  // z
		tNd[11] = m_armAngles[nd+38];  // z

		if ( m_actionType == MSS_BACK2 )   // sur le dos ?
		{
			for ( ii=0 ; ii<12 ; ii++ )
			{
				tSt[ii] += Rand()*20.0f;
				tNd[ii] = tSt[ii];
			}
//?			time = 100.0f;
			time = event.rTime*10.0f;
		}

		if ( i < 4 )  // patte droite (1..4) ?
		{
			m_object->SetAngleX(3+4*i+0, Smooth(m_object->RetAngleX(3+4*i+0), Propf(tSt[ 0], tNd[ 0], prog), time));
			m_object->SetAngleY(3+4*i+0, Smooth(m_object->RetAngleY(3+4*i+0), Propf(tSt[ 1], tNd[ 1], prog), time));
			m_object->SetAngleZ(3+4*i+0, Smooth(m_object->RetAngleZ(3+4*i+0), Propf(tSt[ 2], tNd[ 2], prog), time));
			m_object->SetAngleX(3+4*i+1, Smooth(m_object->RetAngleX(3+4*i+1), Propf(tSt[ 3], tNd[ 3], prog), time));
			m_object->SetAngleY(3+4*i+1, Smooth(m_object->RetAngleY(3+4*i+1), Propf(tSt[ 4], tNd[ 4], prog), time));
			m_object->SetAngleZ(3+4*i+1, Smooth(m_object->RetAngleZ(3+4*i+1), Propf(tSt[ 5], tNd[ 5], prog), time));
			m_object->SetAngleX(3+4*i+2, Smooth(m_object->RetAngleX(3+4*i+2), Propf(tSt[ 6], tNd[ 6], prog), time));
			m_object->SetAngleY(3+4*i+2, Smooth(m_object->RetAngleY(3+4*i+2), Propf(tSt[ 7], tNd[ 7], prog), time));
			m_object->SetAngleZ(3+4*i+2, Smooth(m_object->RetAngleZ(3+4*i+2), Propf(tSt[ 8], tNd[ 8], prog), time));
			m_object->SetAngleX(3+4*i+3, Smooth(m_object->RetAngleX(3+4*i+3), Propf(tSt[ 9], tNd[ 9], prog), time));
			m_object->SetAngleY(3+4*i+3, Smooth(m_object->RetAngleY(3+4*i+3), Propf(tSt[10], tNd[10], prog), time));
			m_object->SetAngleZ(3+4*i+3, Smooth(m_object->RetAngleZ(3+4*i+3), Propf(tSt[11], tNd[11], prog), time));
		}
		else	// patte gauche (5..8) ?
		{
			m_object->SetAngleX(3+4*i+0, Smooth(m_object->RetAngleX(3+4*i+0), Propf(-tSt[ 0], -tNd[ 0], prog), time));
			m_object->SetAngleY(3+4*i+0, Smooth(m_object->RetAngleY(3+4*i+0), Propf(-tSt[ 1], -tNd[ 1], prog), time));
			m_object->SetAngleZ(3+4*i+0, Smooth(m_object->RetAngleZ(3+4*i+0), Propf( tSt[ 2],  tNd[ 2], prog), time));
			m_object->SetAngleX(3+4*i+1, Smooth(m_object->RetAngleX(3+4*i+1), Propf(-tSt[ 3], -tNd[ 3], prog), time));
			m_object->SetAngleY(3+4*i+1, Smooth(m_object->RetAngleY(3+4*i+1), Propf(-tSt[ 4], -tNd[ 4], prog), time));
			m_object->SetAngleZ(3+4*i+1, Smooth(m_object->RetAngleZ(3+4*i+1), Propf( tSt[ 5],  tNd[ 5], prog), time));
			m_object->SetAngleX(3+4*i+2, Smooth(m_object->RetAngleX(3+4*i+2), Propf(-tSt[ 6], -tNd[ 6], prog), time));
			m_object->SetAngleY(3+4*i+2, Smooth(m_object->RetAngleY(3+4*i+2), Propf(-tSt[ 7], -tNd[ 7], prog), time));
			m_object->SetAngleZ(3+4*i+2, Smooth(m_object->RetAngleZ(3+4*i+2), Propf( tSt[ 8],  tNd[ 8], prog), time));
			m_object->SetAngleX(3+4*i+3, Smooth(m_object->RetAngleX(3+4*i+3), Propf(-tSt[ 9], -tNd[ 9], prog), time));
			m_object->SetAngleY(3+4*i+3, Smooth(m_object->RetAngleY(3+4*i+3), Propf(-tSt[10], -tNd[10], prog), time));
			m_object->SetAngleZ(3+4*i+3, Smooth(m_object->RetAngleZ(3+4*i+3), Propf( tSt[11],  tNd[11], prog), time));
		}
	}

#if ADJUST_ANGLE
	if ( m_object->RetSelect() )
	{
		char s[100];
		sprintf(s, "A:time=%d Q:part=%d W:member=%d", m_armTimeIndex, m_armPartIndex, m_armMemberIndex);
		m_engine->SetInfoText(4, s);
	}
#endif

	if ( m_actionType == MSS_BURN )  // br�le ?
	{
		dir = D3DVECTOR(PI, 0.0f, 0.0f);
		SetCirVibration(dir);
		dir = D3DVECTOR(0.0f, 0.0f, 0.0f);
		SetLinVibration(dir);
		SetInclinaison(dir);

		time = event.rTime*1.0f;
		m_object->SetAngleZ(1, Smooth(m_object->RetAngleZ(1), 0.0f, time));  // t�te
	}
	else if ( m_actionType == MSS_RUIN )  // ruine ?
	{
		dir = D3DVECTOR(0.0f, 0.0f, 0.0f);
		SetLinVibration(dir);
		SetCirVibration(dir);
		SetInclinaison(dir);
	}
	else if ( m_actionType == MSS_EXPLO )  // explose ?
	{
		m_object->SetZoomY(1, 1.0f+m_progress);
		m_object->SetZoomZ(1, 1.0f+m_progress);
		m_object->SetZoomX(1, 1.0f+m_progress/2.0f);

		dir.x = (Rand()-0.5f)*0.1f*m_progress;
		dir.y = (Rand()-0.5f)*0.1f*m_progress;
		dir.z = (Rand()-0.5f)*0.1f*m_progress;
		m_object->SetCirVibration(dir);
	}
	else if ( m_actionType == MSS_BACK1 )  // se met sur le dos ?
	{
		if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_armTimeAbs )
		{
			m_lastParticule = m_armTimeAbs;

			pos = m_object->RetPosition(0);
			speed.x = (Rand()-0.5f)*10.0f;
			speed.z = (Rand()-0.5f)*10.0f;
			speed.y = Rand()*5.0f;
			dim.x = Rand()*3.0f+2.0f;
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTICRASH, 2.0f);
		}

		if ( m_progress < 0.5f )
		{
			dir.x = 0.0f;
			dir.y = powf(m_progress/0.5f, 2.0f)*12.0f;
			dir.z = 0.0f;
			SetLinVibration(dir);
		}
		else
		{
			dir.x = 0.0f;
			dir.y = powf(2.0f-m_progress/0.5f, 2.0f)*12.0f;
			dir.z = 0.0f;
			SetLinVibration(dir);
		}
		dir.x = m_progress*PI;
		dir.y = 0.0f;
		dir.z = 0.0f;
		SetCirVibration(dir);

		dir = D3DVECTOR(0.0f, 0.0f, 0.0f);
		SetInclinaison(dir);

		if ( m_progress >= 1.0f )
		{
			SetAction(MSS_BACK2, 55.0f+Rand()*10.0f);
		}
	}
	else if ( m_actionType == MSS_BACK2 )  // bouge sur le dos ?
	{
		if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_armTimeAbs )
		{
			m_lastParticule = m_armTimeAbs;

			if ( rand()%10 == 0 )
			{
				pos = m_object->RetPosition(0);
				pos.x += (Rand()-0.5f)*8.0f;
				pos.z += (Rand()-0.5f)*8.0f;
				pos.y -= 1.0f;
				speed.x = (Rand()-0.5f)*2.0f;
				speed.z = (Rand()-0.5f)*2.0f;
				speed.y = Rand()*2.0f;
				dim.x = Rand()*1.0f+1.0f;
				dim.y = dim.x;
				m_particule->CreateParticule(pos, speed, dim, PARTICRASH, 2.0f);
			}
		}

		dir = D3DVECTOR(0.0f, 0.0f, 0.0f);
		SetLinVibration(dir);
		dir.x = sinf(m_armTimeAbs* 3.0f)*0.20f+
				sinf(m_armTimeAbs* 6.0f)*0.20f+
				sinf(m_armTimeAbs*10.0f)*0.20f+
				sinf(m_armTimeAbs*17.0f)*0.30f+PI;
		dir.y = sinf(m_armTimeAbs* 4.0f)*0.02f+
				sinf(m_armTimeAbs* 5.0f)*0.02f+
				sinf(m_armTimeAbs*11.0f)*0.02f+
				sinf(m_armTimeAbs*18.0f)*0.03f;
		dir.z = sinf(m_armTimeAbs* 2.0f)*0.02f+
				sinf(m_armTimeAbs* 7.0f)*0.02f+
				sinf(m_armTimeAbs*13.0f)*0.02f+
				sinf(m_armTimeAbs*15.0f)*0.03f;
		SetCirVibration(dir);
		dir = D3DVECTOR(0.0f, 0.0f, 0.0f);
		SetInclinaison(dir);

		m_object->SetAngleY(1, sinf(m_armTimeAbs*5.0f)*0.05f);  // queue
		m_object->SetAngleY(2, cosf(m_armTimeAbs*5.0f)*0.20f);  // t�te
		m_object->SetAngleZ(1, 0.4f);  // queue
		m_object->SetAngleZ(2, 0.0f);  // t�te

		if ( m_progress >= 1.0f )
		{
			SetAction(MSS_BACK3, 0.4f);
		}
	}
	else if ( m_actionType == MSS_BACK3 )  // se remet sur les pattes ?
	{
		if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_armTimeAbs )
		{
			m_lastParticule = m_armTimeAbs;

			pos = m_object->RetPosition(0);
			speed.x = (Rand()-0.5f)*10.0f;
			speed.z = (Rand()-0.5f)*10.0f;
			speed.y = Rand()*5.0f;
			dim.x = Rand()*3.0f+2.0f;
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTICRASH, 2.0f);
		}

		if ( m_progress < 0.5f )
		{
			dir.x = 0.0f;
			dir.y = powf(m_progress/0.5f, 2.0f)*5.0f;
			dir.z = 0.0f;
			SetLinVibration(dir);
		}
		else
		{
			dir.x = 0.0f;
			dir.y = powf(2.0f-m_progress/0.5f, 2.0f)*5.0f;
			dir.z = 0.0f;
			SetLinVibration(dir);
		}
		dir.x = (1.0f-m_progress)*PI;
		dir.y = 0.0f;
		dir.z = 0.0f;
		SetCirVibration(dir);

		dir = D3DVECTOR(0.0f, 0.0f, 0.0f);
		SetInclinaison(dir);

		if ( m_progress >= 1.0f )
		{
			SetAction(-1);
			m_object->SetFixed(FALSE);  // bouge de nouveau
		}
	}
	else
	{
		if ( bStop )
		{
			dir = D3DVECTOR(0.0f, 0.0f, 0.0f);
			SetInclinaison(dir);
		}
		else
		{
			a = Mod(m_armMember, 1.0f);
			if ( a < 0.5f )  a = -1.0f+4.0f*a;  // -1..1
			else             a =  3.0f-4.0f*a;  // 1..-1
			dir.x = sinf(a)*0.05f;

			s = Mod(m_armMember/2.0f, 1.0f);
			if ( s < 0.5f )  s = -1.0f+4.0f*s;  // -1..1
			else             s =  3.0f-4.0f*s;  // 1..-1
			dir.z = sinf(s)*0.1f;

			dir.y = 0.0f;
			SetInclinaison(dir);
		}

		dir = D3DVECTOR(0.0f, 0.0f, 0.0f);
		SetLinVibration(dir);
		SetCirVibration(dir);

		m_object->SetAngleZ(1, sinf(m_armTimeAbs*1.7f)*0.02f);  // queue
		m_object->SetAngleX(1, sinf(m_armTimeAbs*1.3f)*0.05f);
		m_object->SetAngleY(1, sinf(m_armTimeAbs*2.4f)*0.10f);
		m_object->SetZoom(1, 1.0f+sinf(m_armTimeAbs*3.3f)*0.05f);

		m_object->SetAngleZ(2, sinf(m_armTimeAbs*1.4f)*0.20f);  // t�te
		m_object->SetAngleX(2, sinf(m_armTimeAbs*1.9f)*0.10f);
		m_object->SetAngleY(2, sinf(m_armTimeAbs*2.1f)*0.10f);

		m_object->SetAngleY(35,  sinf(m_armTimeAbs*3.1f)*0.20f);  // mandibule
		m_object->SetAngleY(36, -sinf(m_armTimeAbs*3.1f)*0.20f);  // mandibule
	}

	return TRUE;
}


