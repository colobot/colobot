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
#include "D3DMath.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "math3d.h"
#include "robotmain.h"
#include "terrain.h"
#include "camera.h"
#include "particule.h"
#include "light.h"
#include "object.h"
#include "motion.h"
#include "motionhuman.h"
#include "displaytext.h"
#include "sound.h"
#include "pyro.h"




// Constructeur de l'objet.

CPyro::CPyro(CInstanceManager* iMan)
{
	m_iMan = iMan;
	m_iMan->AddInstance(CLASS_PYRO, this, 100);

	m_engine      = (CD3DEngine*)m_iMan->SearchInstance(CLASS_ENGINE);
	m_terrain     = (CTerrain*)m_iMan->SearchInstance(CLASS_TERRAIN);
	m_camera      = (CCamera*)m_iMan->SearchInstance(CLASS_CAMERA);
	m_particule   = (CParticule*)m_iMan->SearchInstance(CLASS_PARTICULE);
	m_light       = (CLight*)m_iMan->SearchInstance(CLASS_LIGHT);
	m_displayText = (CDisplayText*)m_iMan->SearchInstance(CLASS_DISPLAYTEXT);
	m_main        = (CRobotMain*)m_iMan->SearchInstance(CLASS_MAIN);
	m_sound       = (CSound*)m_iMan->SearchInstance(CLASS_SOUND);
	m_object = 0;

	m_progress = 0.0f;
	m_speed = 0.0f;
	m_lightRank = -1;
	m_soundChannel = -1;
	LightOperFlush();
}

// Destructeur de l'objet.

CPyro::~CPyro()
{
	m_iMan->DeleteInstance(CLASS_PYRO, this);
}


// D�truit l'objet.

void CPyro::DeleteObject(BOOL bAll)
{
	if ( m_lightRank != -1 )
	{
		m_light->DeleteLight(m_lightRank);
		m_lightRank = -1;
	}
}


// Cr�e un effet pyrotechnique.

BOOL CPyro::Create(PyroType type, CObject* pObj, float force)
{
	D3DMATRIX*		mat;
	CObject*		power;
	CMotion*		motion;
	D3DVECTOR		min, max, pos, speed;
	FPOINT			dim;
	ObjectType		oType;
	Sound			sound;
	float			duration, mass, h, limit;
	int				part, objRank, total, i, channel;

	m_object = pObj;
	m_force = force;

	oType = pObj->RetType();
	objRank = pObj->RetObjectRank(0);
	if ( objRank == -1 )  return FALSE;
	m_engine->GetBBox(objRank, min, max);
	pos = pObj->RetPosition(0);

	DisplayError(type, pObj);  // affiche message �ventuel

	// Copie toutes les sph�res de l'objet.
	for ( i=0 ; i<50 ; i++ )
	{
		if ( !pObj->GetCrashSphere(i, m_crashSpherePos[i], m_crashSphereRadius[i]) )  break;
	}
	m_crashSphereUsed = i;

	// Calcule la dimension de l'effet.
	if ( oType == OBJECT_ANT    ||
		 oType == OBJECT_BEE    ||
		 oType == OBJECT_WORM   ||
		 oType == OBJECT_SPIDER )
	{
		m_size = 40.0f;
	}
	else
	{
		m_size = Length(min, max)*2.0f;
		if ( m_size <  4.0f )  m_size =  4.0f;
		if ( m_size > 80.0f )  m_size = 80.0f;
	}
	if ( oType == OBJECT_TNT  ||
		 oType == OBJECT_BOMB )
	{
		m_size *= 2.0f;
	}

	m_pos = pos+(min+max)/2.0f;
	m_type = type;
	m_progress = 0.0f;
	m_speed = 1.0f/20.0f;
	m_time = 0.0f;
	m_lastParticule = 0.0f;
	m_lastParticuleSmoke = 0.0f;
	m_lightRank = -1;

	if ( oType == OBJECT_TEEN28 ||
		 oType == OBJECT_TEEN31 )
	{
		m_pos.y = pos.y+1.0f;
	}

	// Cherche la position de la pile.
	power = pObj->RetPower();
	if ( power == 0 )
	{
		m_bPower = FALSE;
	}
	else
	{
		m_bPower = TRUE;
		pos = power->RetPosition(0);
		pos.y += 1.0f;
		mat = pObj->RetWorldMatrix(0);
		m_posPower = Transform(*mat, pos);
	}
	if ( oType == OBJECT_POWER   ||
		 oType == OBJECT_ATOMIC  ||
		 oType == OBJECT_URANIUM ||
		 oType == OBJECT_TNT     ||
		 oType == OBJECT_BOMB    )
	{
		m_bPower = TRUE;
		m_posPower = m_pos;
		m_posPower.y += 1.0f;
		m_pos = m_posPower;
	}
	if ( oType == OBJECT_STATION )
	{
		m_bPower = TRUE;
		mat = pObj->RetWorldMatrix(0);
		m_posPower = Transform(*mat, D3DVECTOR(-15.0f, 7.0f, 0.0f));
		m_pos = m_posPower;
	}
	if ( oType == OBJECT_ENERGY )
	{
		m_bPower = TRUE;
		mat = pObj->RetWorldMatrix(0);
		m_posPower = Transform(*mat, D3DVECTOR(-7.0f, 6.0f, 0.0f));
		m_pos = m_posPower;
	}
	if ( oType == OBJECT_NUCLEAR )
	{
		m_bPower = TRUE;
		m_posPower = m_pos;
	}
	if ( oType == OBJECT_PARA )
	{
		m_bPower = TRUE;
		m_posPower = m_pos;
	}
	if ( oType == OBJECT_SCRAP4 ||
		 oType == OBJECT_SCRAP5 )  // mati�re plastique ?
	{
		m_bPower = TRUE;
		m_posPower = m_pos;
	}

	// Fait entendre le bruit de l'effet pyrotechnique.
	if ( type == PT_FRAGT  ||
		 type == PT_FRAGW  ||
		 type == PT_EXPLOT ||
		 type == PT_EXPLOW )
	{
		if ( m_bPower )
		{
			sound = SOUND_EXPLOp;
		}
		else
		{
			sound = SOUND_EXPLO;
		}
		if ( oType == OBJECT_STONE   ||
			 oType == OBJECT_METAL   ||
			 oType == OBJECT_BULLET  ||
			 oType == OBJECT_BBOX    ||
			 oType == OBJECT_KEYa    ||
			 oType == OBJECT_KEYb    ||
			 oType == OBJECT_KEYc    ||
			 oType == OBJECT_KEYd    )
		{
			sound = SOUND_EXPLOl;
		}
		if ( oType == OBJECT_URANIUM ||
			 oType == OBJECT_POWER   ||
			 oType == OBJECT_ATOMIC  ||
			 oType == OBJECT_TNT     ||
			 oType == OBJECT_BOMB    )
		{
			sound = SOUND_EXPLOlp;
		}
		m_sound->Play(sound, m_pos);
	}
	if ( type == PT_FRAGO  ||
		 type == PT_EXPLOO ||
		 type == PT_SPIDER ||
		 type == PT_SHOTM  )
	{
		m_sound->Play(SOUND_EXPLOi, m_pos);
	}
	if ( type == PT_BURNT ||
		 type == PT_BURNO )
	{
		m_soundChannel = m_sound->Play(SOUND_BURN, m_pos, 1.0f, 1.0f, TRUE);
		m_sound->AddEnvelope(m_soundChannel, 1.0f, 1.0f, 12.0f, SOPER_CONTINUE);
		m_sound->AddEnvelope(m_soundChannel, 0.0f, 1.0f,  5.0f, SOPER_STOP);
	}
	if ( type == PT_BURNO )
	{
		m_sound->Play(SOUND_DEADi, m_pos);
		m_sound->Play(SOUND_DEADi, m_engine->RetEyePt());
	}
	if ( type == PT_EGG )
	{
		m_sound->Play(SOUND_EGG, m_pos);
	}
	if ( type == PT_WPCHECK  ||
		 type == PT_FLCREATE ||
		 type == PT_FLDELETE )
	{
		m_sound->Play(SOUND_WAYPOINT, m_pos);
	}
	if ( oType == OBJECT_HUMAN )
	{
		if ( type == PT_DEADG )
		{
			m_sound->Play(SOUND_DEADg, m_pos);
		}
		if ( type == PT_DEADW )
		{
			m_sound->Play(SOUND_DEADw, m_pos);
		}
		if ( type == PT_SHOTH && m_object->RetSelect() )
		{
			m_sound->Play(SOUND_AIE, m_pos);
			m_sound->Play(SOUND_AIE, m_engine->RetEyePt());
		}
	}

	if ( m_type == PT_FRAGT ||
		 m_type == PT_FRAGO ||
		 m_type == PT_FRAGW )
	{
		m_engine->ShadowDelete(m_object->RetObjectRank(0));
	}

	if ( m_type == PT_DEADG )
	{
		m_object->SetDead(TRUE);

		motion = m_object->RetMotion();
		if ( motion != 0 )
		{
			motion->SetAction(MHS_DEADg, 1.0f);
		}
		m_camera->StartCentering(m_object, PI*0.5f, 99.9f, 0.0f, 1.5f);
		m_camera->StartOver(OE_FADEOUTw, m_pos, 1.0f);
		m_speed = 1.0f/10.0f;
		return TRUE;
	}
	if ( m_type == PT_DEADW )
	{
		m_object->SetDead(TRUE);

		motion = m_object->RetMotion();
		if ( motion != 0 )
		{
			motion->SetAction(MHS_DEADw, 4.0f);
		}
		m_camera->StartCentering(m_object, PI*0.5f, 99.9f, 0.0f, 3.0f);
		m_camera->StartOver(OE_FADEOUTb, m_pos, 1.0f);
		m_speed = 1.0f/10.0f;
		return TRUE;
	}

	if ( m_type == PT_SHOTT ||
		 m_type == PT_SHOTM )
	{
		m_camera->StartEffect(CE_SHOT, m_pos, force);
		m_speed = 1.0f/1.0f;
		return TRUE;
	}
	if ( m_type == PT_SHOTH )
	{
		if ( m_object->RetSelect() )
		{
			m_camera->StartOver(OE_BLOOD, m_pos, force);
		}
		m_speed = 1.0f/0.2f;
		return TRUE;
	}

	if ( m_type == PT_SHOTW )
	{
		m_speed = 1.0f/1.0f;
	}

	if ( m_type == PT_BURNT )
	{
		BurnStart();
	}

	if ( m_type == PT_WPCHECK )
	{
		m_speed = 1.0f/8.0f;
		m_object->SetEnable(FALSE);  // objet plus fonctionnel
	}
	if ( m_type == PT_FLCREATE )
	{
		m_speed = 1.0f/2.0f;
	}
	if ( m_type == PT_FLDELETE )
	{
		m_speed = 1.0f/2.0f;
		m_object->SetEnable(FALSE);  // objet plus fonctionnel
	}
	if ( m_type == PT_RESET )
	{
		m_speed = 1.0f/2.0f;
		m_object->SetPosition(0, m_object->RetResetPosition());
		m_object->SetAngle(0, m_object->RetResetAngle());
		m_object->SetZoom(0, 0.0f);
	}
	if ( m_type == PT_FINDING )
	{
		limit = (m_size-1.0f)/4.0f;
		if ( limit > 8.0f )  limit = 8.0f;
		if ( oType == OBJECT_APOLLO2 )  limit = 2.0f;
		m_speed = 1.0f/limit;
	}

	if ( m_type == PT_EXPLOT ||
		 m_type == PT_EXPLOO ||
		 m_type == PT_EXPLOW )
	{
		CreateTriangle(pObj, oType, 0);
		m_engine->ShadowDelete(m_object->RetObjectRank(0));
		ExploStart();
	}

	if ( m_type == PT_FALL )
	{
		FallStart();
		return TRUE;
	}

	if ( m_type == PT_BURNT ||
		 m_type == PT_BURNO )
	{
		m_speed = 1.0f/15.0f;

		LightOperAdd(0.00f, 0.0f,  2.0f,  1.0f,  0.0f);  // rouge-orange
		LightOperAdd(0.30f, 1.0f, -0.8f, -0.8f, -0.8f);  // gris fonc�
		LightOperAdd(0.80f, 1.0f, -0.8f, -0.8f, -0.8f);  // gris fonc�
		LightOperAdd(1.00f, 0.0f, -0.8f, -0.8f, -0.8f);  // gris fonc�
		CreateLight(m_pos, 40.0f);
		return TRUE;
	}

	if ( m_type == PT_SPIDER )
	{
		m_speed = 1.0f/15.0f;

		pos = D3DVECTOR(-3.0f, 2.0f, 0.0f);
		mat = pObj->RetWorldMatrix(0);
		m_pos = Transform(*mat, pos);

		m_engine->ShadowDelete(m_object->RetObjectRank(0));
	}

	if ( m_type != PT_EGG  &&
		 m_type != PT_WIN  &&
		 m_type != PT_LOST )
	{
		h = 40.0f;
		if ( m_type == PT_FRAGO  ||
			 m_type == PT_EXPLOO )
		{
			LightOperAdd(0.00f, 0.0f, -1.0f, -0.5f, -1.0f);  // vert fonc�
			LightOperAdd(0.05f, 1.0f, -1.0f, -0.5f, -1.0f);  // vert fonc�
			LightOperAdd(1.00f, 0.0f, -1.0f, -0.5f, -1.0f);  // vert fonc�
		}
		else if ( m_type == PT_FRAGT  ||
				  m_type == PT_EXPLOT )
		{
			LightOperAdd(0.00f, 1.0f,  4.0f,  4.0f,  2.0f);  // jaune
			LightOperAdd(0.02f, 1.0f,  4.0f,  2.0f,  0.0f);  // rouge-orange
			LightOperAdd(0.16f, 1.0f, -0.8f, -0.8f, -0.8f);  // gris fonc�
			LightOperAdd(1.00f, 0.0f, -0.8f, -0.8f, -0.8f);  // gris fonc�
			h = m_size*2.0f;
		}
		else if ( m_type == PT_SPIDER )
		{
			LightOperAdd(0.00f, 0.0f, -0.5f, -1.0f, -1.0f);  // rouge fonc�
			LightOperAdd(0.05f, 1.0f, -0.5f, -1.0f, -1.0f);  // rouge fonc�
			LightOperAdd(1.00f, 0.0f, -0.5f, -1.0f, -1.0f);  // rouge fonc�
		}
		else if ( m_type == PT_FRAGW  ||
				  m_type == PT_EXPLOW ||
				  m_type == PT_SHOTW  )
		{
			LightOperAdd(0.00f, 0.0f, -0.5f, -0.5f, -1.0f);  // jaune fonc�
			LightOperAdd(0.05f, 1.0f, -0.5f, -0.5f, -1.0f);  // jaune fonc�
			LightOperAdd(1.00f, 0.0f, -0.5f, -0.5f, -1.0f);  // jaune fonc�
		}
		else if ( m_type == PT_WPCHECK  ||
				  m_type == PT_FLCREATE ||
				  m_type == PT_FLDELETE ||
				  m_type == PT_RESET    ||
				  m_type == PT_FINDING  )
		{
			LightOperAdd(0.00f, 1.0f,  4.0f,  4.0f,  2.0f);  // jaune
			LightOperAdd(1.00f, 0.0f,  4.0f,  4.0f,  2.0f);  // jaune
		}
		else
		{
			LightOperAdd(0.00f, 0.0f, -0.8f, -0.8f, -0.8f);  // gris fonc�
			LightOperAdd(0.05f, 1.0f, -0.8f, -0.8f, -0.8f);  // gris fonc�
			LightOperAdd(1.00f, 0.0f, -0.8f, -0.8f, -0.8f);  // gris fonc�
		}
		CreateLight(m_pos, h);

		if ( m_type != PT_SHOTW    &&
			 m_type != PT_WPCHECK  &&
			 m_type != PT_FLCREATE &&
			 m_type != PT_FLDELETE &&
			 m_type != PT_RESET    &&
			 m_type != PT_FINDING  )
		{
			m_camera->StartEffect(CE_EXPLO, m_pos, force);
		}
	}

	if ( m_type == PT_SHOTW )  return TRUE;
	
	// G�n�re les triangles de l'explosion.
	if ( m_type == PT_FRAGT  ||
		 m_type == PT_FRAGO  ||
		 m_type == PT_FRAGW  ||
		 m_type == PT_SPIDER ||
		 m_type == PT_EGG    ||
		(m_type == PT_EXPLOT && oType == OBJECT_MOBILEtg) ||
		(m_type == PT_EXPLOT && oType == OBJECT_TEEN28  ) ||
		(m_type == PT_EXPLOT && oType == OBJECT_TEEN31  ) )
	{
		for ( part=0 ; part<OBJECTMAXPART ; part++ )
		{
			CreateTriangle(pObj, oType, part);
		}
	}

	if ( m_type == PT_FRAGT  ||
		 m_type == PT_EXPLOT )
	{
		if ( m_bPower )
		{
			total = (int)(10.0f*m_engine->RetParticuleDensity());
			if ( oType == OBJECT_TNT  ||
				 oType == OBJECT_BOMB )  total *= 3;
			for ( i=0 ; i<total ; i++ )
			{
				pos = m_posPower;
				speed.x = (Rand()-0.5f)*30.0f;
				speed.z = (Rand()-0.5f)*30.0f;
				speed.y = Rand()*30.0f;
				dim.x = 1.0f;
				dim.y = dim.x;
				duration = Rand()*3.0f+2.0f;
				mass = Rand()*10.0f+15.0f;
				m_particule->CreateTrack(pos, speed, dim, PARTITRACK1,
										 duration, mass, Rand()+0.7f, 1.0f);
			}
		}

		if ( m_size > 10.0f )  // assez grand (fret exclu) ?
		{
			if ( m_bPower )
			{
				pos = m_posPower;
			}
			else
			{
				pos = m_pos;
				m_terrain->MoveOnFloor(pos);
				pos.y += 1.0f;
			}
			dim.x = m_size*0.4f;
			dim.y = dim.x;
			m_particule->CreateParticule(pos, D3DVECTOR(0.0f,0.0f,0.0f), dim, PARTISPHERE0, 2.0f, 0.0f, 0.0f);
		}
	}

	if ( m_type == PT_FRAGO  ||
		 m_type == PT_EXPLOO )
	{
		total = (int)(10.0f*m_engine->RetParticuleDensity());
		for ( i=0 ; i<total ; i++ )
		{
			pos = m_pos;
			speed.x = (Rand()-0.5f)*30.0f;
			speed.z = (Rand()-0.5f)*30.0f;
			speed.y = Rand()*50.0f;
			dim.x = 1.0f;
			dim.y = dim.x;
			duration = Rand()*1.0f+0.8f;
			mass = Rand()*10.0f+15.0f;
			m_particule->CreateParticule(pos, speed, dim, PARTIORGANIC1,
										 duration, mass);
		}
		total = (int)(5.0f*m_engine->RetParticuleDensity());
		for ( i=0 ; i<total ; i++ )
		{
			pos = m_pos;
			speed.x = (Rand()-0.5f)*30.0f;
			speed.z = (Rand()-0.5f)*30.0f;
			speed.y = Rand()*50.0f;
			dim.x = 1.0f;
			dim.y = dim.x;
			duration = Rand()*2.0f+1.4f;
			mass = Rand()*10.0f+15.0f;
			m_particule->CreateTrack(pos, speed, dim, PARTITRACK4,
									 duration, mass, duration*0.5f, dim.x*2.0f);
		}
	}

	if ( m_type == PT_SPIDER )
	{
		for ( i=0 ; i<50 ; i++ )
		{
			pos = m_pos;
			pos.x += (Rand()-0.5f)*3.0f;
			pos.z += (Rand()-0.5f)*3.0f;
			pos.y += (Rand()-0.5f)*2.0f;
			speed.x = (Rand()-0.5f)*24.0f;
			speed.z = (Rand()-0.5f)*24.0f;
			speed.y = 10.0f+Rand()*10.0f;
			dim.x = 1.0f;
			dim.y = dim.x;
			channel = m_particule->CreateParticule(pos, speed, dim, PARTIGUN3, 2.0f+Rand()*2.0f, 10.0f);
			m_particule->SetObjectFather(channel, pObj);
		}
		total = (int)(10.0f*m_engine->RetParticuleDensity());
		for ( i=0 ; i<total ; i++ )
		{
			pos = m_pos;
			pos.x += (Rand()-0.5f)*3.0f;
			pos.z += (Rand()-0.5f)*3.0f;
			pos.y += (Rand()-0.5f)*2.0f;
			speed.x = (Rand()-0.5f)*24.0f;
			speed.z = (Rand()-0.5f)*24.0f;
			speed.y = 7.0f+Rand()*7.0f;
			dim.x = 1.0f;
			dim.y = dim.x;
			m_particule->CreateTrack(pos, speed, dim, PARTITRACK3,
									 2.0f+Rand()*2.0f, 10.0f, 2.0f, 0.6f);
		}
	}

	if ( type == PT_FRAGT  ||
		 type == PT_FRAGW  ||
		 type == PT_EXPLOT ||
		 type == PT_EXPLOW )
	{
		if ( m_size > 10.0f || m_bPower )
		{
			pos = m_pos;
//?			m_terrain->MoveOnFloor(pos);
//?			pos.y += 2.0f;
			speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
			dim.x = m_size;
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTICHOC, 2.0f);
		}
	}

	return TRUE;
}

// Cr�e une explosion sous forme de particules triangulaires.

void CPyro::CreateTriangle(CObject* pObj, ObjectType oType, int part)
{
	D3DTriangle		buffer[100];
	D3DMATRIX*		mat;
	D3DVECTOR		offset, pos, speed;
	float			percent, min, max, h, duration, mass;
	int				objRank, total, i;

	objRank = pObj->RetObjectRank(part);
	if ( objRank == -1 )  return;

	min = 0.0f;
	max = m_engine->RetLimitLOD(0);
	total = m_engine->RetTotalTriangles(objRank);
	percent = 0.10f;
	if ( total < 50 )  percent = 0.25f;
	if ( total < 20 )  percent = 0.50f;
	if ( m_type == PT_EGG )  percent = 0.30f;
	if ( oType == OBJECT_POWER    ||
		 oType == OBJECT_ATOMIC   ||
		 oType == OBJECT_URANIUM  ||
		 oType == OBJECT_TNT      ||
		 oType == OBJECT_BOMB     )  percent = 0.75f;
	if ( oType == OBJECT_MOBILEtg )  percent = 0.50f;
	if ( oType == OBJECT_TEEN28   )  percent = 0.75f;
	if ( oType == OBJECT_MOTHER   )  max = 1000000.0f;
	if ( oType == OBJECT_TEEN28   )  max = 1000000.0f;
	if ( oType == OBJECT_TEEN31   )  max = 1000000.0f;
	total = m_engine->GetTriangles(objRank, min, max, buffer, 100, percent);

	for ( i=0 ; i<total ; i++ )
	{
		D3DVECTOR	p1, p2, p3;

		p1.x = buffer[i].triangle[0].x;
		p1.y = buffer[i].triangle[0].y;
		p1.z = buffer[i].triangle[0].z;
		p2.x = buffer[i].triangle[1].x;
		p2.y = buffer[i].triangle[1].y;
		p2.z = buffer[i].triangle[1].z;
		p3.x = buffer[i].triangle[2].x;
		p3.y = buffer[i].triangle[2].y;
		p3.z = buffer[i].triangle[2].z;

		h = Length(p1, p2);
		if ( h > 5.0f )
		{
			p2.x = p1.x+((p2.x-p1.x)*5.0f/h);
			p2.y = p1.y+((p2.y-p1.y)*5.0f/h);
			p2.z = p1.z+((p2.z-p1.z)*5.0f/h);
		}

		h = Length(p2, p3);
		if ( h > 5.0f )
		{
			p3.x = p2.x+((p3.x-p2.x)*5.0f/h);
			p3.y = p2.y+((p3.y-p2.y)*5.0f/h);
			p3.z = p2.z+((p3.z-p2.z)*5.0f/h);
		}

		h = Length(p3, p1);
		if ( h > 5.0f )
		{
			p1.x = p3.x+((p1.x-p3.x)*5.0f/h);
			p1.y = p3.y+((p1.y-p3.y)*5.0f/h);
			p1.z = p3.z+((p1.z-p3.z)*5.0f/h);
		}

		buffer[i].triangle[0].x = p1.x;
		buffer[i].triangle[0].y = p1.y;
		buffer[i].triangle[0].z = p1.z;
		buffer[i].triangle[1].x = p2.x;
		buffer[i].triangle[1].y = p2.y;
		buffer[i].triangle[1].z = p2.z;
		buffer[i].triangle[2].x = p3.x;
		buffer[i].triangle[2].y = p3.y;
		buffer[i].triangle[2].z = p3.z;

		offset.x = (buffer[i].triangle[0].x+buffer[i].triangle[1].x+buffer[i].triangle[2].x)/3.0f;
		offset.y = (buffer[i].triangle[0].y+buffer[i].triangle[1].y+buffer[i].triangle[2].y)/3.0f;
		offset.z = (buffer[i].triangle[0].z+buffer[i].triangle[1].z+buffer[i].triangle[2].z)/3.0f;

		buffer[i].triangle[0].x -= offset.x;
		buffer[i].triangle[1].x -= offset.x;
		buffer[i].triangle[2].x -= offset.x;

		buffer[i].triangle[0].y -= offset.y;
		buffer[i].triangle[1].y -= offset.y;
		buffer[i].triangle[2].y -= offset.y;

		buffer[i].triangle[0].z -= offset.z;
		buffer[i].triangle[1].z -= offset.z;
		buffer[i].triangle[2].z -= offset.z;

		mat = pObj->RetWorldMatrix(part);
		pos = Transform(*mat, offset);
		if ( m_type == PT_EGG )
		{
			speed.x = (Rand()-0.5f)*10.0f;
			speed.z = (Rand()-0.5f)*10.0f;
			speed.y = Rand()*15.0f;
			mass = Rand()*20.0f+20.0f;
		}
		else if ( m_type == PT_SPIDER )
		{
			speed.x = (Rand()-0.5f)*10.0f;
			speed.z = (Rand()-0.5f)*10.0f;
			speed.y = Rand()*20.0f;
			mass = Rand()*10.0f+15.0f;
		}
		else
		{
			speed.x = (Rand()-0.5f)*30.0f;
			speed.z = (Rand()-0.5f)*30.0f;
			speed.y = Rand()*30.0f;
			mass = Rand()*10.0f+15.0f;
		}
		if ( oType == OBJECT_STONE   )  speed *= 0.5f;
		if ( oType == OBJECT_URANIUM )  speed *= 0.4f;
		duration = Rand()*3.0f+3.0f;
		m_particule->CreateFrag(pos, speed, &buffer[i], PARTIFRAG,
								duration, mass, 0.5f);
	}
}

// Affiche l'erreur ou l'information �ventuelle, li�e � la destruction
// d'un insecte, d'un v�hicule ou d'un batiment.

void CPyro::DisplayError(PyroType type, CObject* pObj)
{
	ObjectType		oType;
	Error			err;

	oType = pObj->RetType();

	if ( type == PT_FRAGT  ||
		 type == PT_FRAGO  ||
		 type == PT_FRAGW  ||
		 type == PT_EXPLOT ||
		 type == PT_EXPLOO ||
		 type == PT_EXPLOW ||
		 type == PT_BURNT  ||
		 type == PT_BURNO  )
	{
		err = ERR_OK;
		if ( oType == OBJECT_MOTHER )  err = INFO_DELETEMOTHER;
		if ( oType == OBJECT_ANT    )  err = INFO_DELETEANT;
		if ( oType == OBJECT_BEE    )  err = INFO_DELETEBEE;
		if ( oType == OBJECT_WORM   )  err = INFO_DELETEWORM;
		if ( oType == OBJECT_SPIDER )  err = INFO_DELETESPIDER;

		if ( oType == OBJECT_MOBILEwa ||
			 oType == OBJECT_MOBILEta ||
			 oType == OBJECT_MOBILEfa ||
			 oType == OBJECT_MOBILEia ||
			 oType == OBJECT_MOBILEwc ||
			 oType == OBJECT_MOBILEtc ||
			 oType == OBJECT_MOBILEfc ||
			 oType == OBJECT_MOBILEic ||
			 oType == OBJECT_MOBILEwi ||
			 oType == OBJECT_MOBILEti ||
			 oType == OBJECT_MOBILEfi ||
			 oType == OBJECT_MOBILEii ||
			 oType == OBJECT_MOBILEws ||
			 oType == OBJECT_MOBILEts ||
			 oType == OBJECT_MOBILEfs ||
			 oType == OBJECT_MOBILEis ||
			 oType == OBJECT_MOBILErt ||
			 oType == OBJECT_MOBILErc ||
			 oType == OBJECT_MOBILErr ||
			 oType == OBJECT_MOBILErs ||
			 oType == OBJECT_MOBILEsa ||
			 oType == OBJECT_MOBILEwt ||
			 oType == OBJECT_MOBILEtt ||
			 oType == OBJECT_MOBILEft ||
			 oType == OBJECT_MOBILEit ||
			 oType == OBJECT_MOBILEdr )
		{
			err = ERR_DELETEMOBILE;
		}

		if ( oType == OBJECT_DERRICK  ||
			 oType == OBJECT_FACTORY  ||
			 oType == OBJECT_STATION  ||
			 oType == OBJECT_CONVERT  ||
			 oType == OBJECT_REPAIR   ||
			 oType == OBJECT_DESTROYER||
			 oType == OBJECT_TOWER    ||
			 oType == OBJECT_RESEARCH ||
			 oType == OBJECT_RADAR    ||
			 oType == OBJECT_INFO     ||
			 oType == OBJECT_ENERGY   ||
			 oType == OBJECT_LABO     ||
			 oType == OBJECT_NUCLEAR  ||
			 oType == OBJECT_PARA     ||
			 oType == OBJECT_SAFE     ||
			 oType == OBJECT_HUSTON   ||
			 oType == OBJECT_START    ||
			 oType == OBJECT_END      )
		{
			err = ERR_DELETEBUILDING;
			m_displayText->DisplayError(err, pObj->RetPosition(0), 5.0f);
			return;
		}

		if ( err != ERR_OK )
		{
			m_displayText->DisplayError(err, pObj);
		}
	}
}


// Gestion d'un �v�nement.

BOOL CPyro::EventProcess(const Event &event)
{
	ParticuleType	type;
	D3DVECTOR		pos, speed, angle;
	FPOINT			dim;
	float			prog, factor, duration;
	int				i, r;

	if ( event.event != EVENT_FRAME )  return TRUE;
	if ( m_engine->RetPause() )  return TRUE;

	m_time += event.rTime;
	m_progress += event.rTime*m_speed;

	if ( m_soundChannel != -1 && m_object != 0 )
	{
		pos = m_object->RetPosition(0);
		m_sound->Position(m_soundChannel, pos);

		if ( m_lightRank != -1 )
		{
			pos.y += m_lightHeight;
			m_light->SetLightPos(m_lightRank, pos);
		}
	}

	if ( m_type == PT_SHOTT &&
		 m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
	{
		m_lastParticule = m_time;

		if ( m_crashSphereUsed > 0 )
		{
			i = rand()%m_crashSphereUsed;
			pos = m_crashSpherePos[i];
			pos.x += (Rand()-0.5f)*m_crashSphereRadius[i]*2.0f;
			pos.z += (Rand()-0.5f)*m_crashSphereRadius[i]*2.0f;
			speed.x = (Rand()-0.5f)*m_crashSphereRadius[i]*0.5f;
			speed.z = (Rand()-0.5f)*m_crashSphereRadius[i]*0.5f;
			speed.y = Rand()*m_crashSphereRadius[i]*1.0f;
			dim.x = Rand()*m_crashSphereRadius[i]*0.5f+m_crashSphereRadius[i]*0.75f*m_force;
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTISMOKE1, 3.0f);
		}
		else
		{
			pos = m_pos;
			pos.x += (Rand()-0.5f)*m_size*0.3f;
			pos.z += (Rand()-0.5f)*m_size*0.3f;
			speed.x = (Rand()-0.5f)*m_size*0.1f;
			speed.z = (Rand()-0.5f)*m_size*0.1f;
			speed.y = Rand()*m_size*0.2f;
			dim.x = Rand()*m_size/10.0f+m_size/10.0f*m_force;
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTISMOKE1, 3.0f);
		}
	}

	if ( m_type == PT_SHOTH &&
		 m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
	{
		m_lastParticule = m_time;

		for ( i=0 ; i<10 ; i++ )
		{
			pos = m_pos;
			pos.x += (Rand()-0.5f)*m_size*0.2f;
			pos.z += (Rand()-0.5f)*m_size*0.2f;
			pos.y += (Rand()-0.5f)*m_size*0.5f;
			speed.x = (Rand()-0.5f)*5.0f;
			speed.z = (Rand()-0.5f)*5.0f;
			speed.y = Rand()*1.0f;
			dim.x = 1.0f;
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTIBLOOD, Rand()*3.0f+3.0f, Rand()*10.0f+15.0f, 0.5f);
		}
	}

	if ( m_type == PT_SHOTM &&
		 m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
	{
		m_lastParticule = m_time;

		r = (int)(10.0f*m_engine->RetParticuleDensity());
		for ( i=0 ; i<r ; i++ )
		{
			pos = m_pos;
			pos.x += (Rand()-0.5f)*20.0f;
			pos.z += (Rand()-0.5f)*20.0f;
			pos.y += 8.0f;
			speed.x = (Rand()-0.5f)*40.0f;
			speed.z = (Rand()-0.5f)*40.0f;
			speed.y = Rand()*40.0f;
			dim.x = Rand()*8.0f+8.0f*m_force;
			dim.y = dim.x;

			m_particule->CreateParticule(pos, speed, dim, PARTIBLOODM, 2.0f, 50.0f, 0.0f);
		}
	}

	if ( m_type == PT_SHOTW &&
		 m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
	{
		m_lastParticule = m_time;

		if ( m_crashSphereUsed > 0 )
		{
			i = rand()%m_crashSphereUsed;
			pos = m_crashSpherePos[i];
			pos.x += (Rand()-0.5f)*m_crashSphereRadius[i]*2.0f;
			pos.z += (Rand()-0.5f)*m_crashSphereRadius[i]*2.0f;
			speed.x = (Rand()-0.5f)*m_crashSphereRadius[i]*0.5f;
			speed.z = (Rand()-0.5f)*m_crashSphereRadius[i]*0.5f;
			speed.y = Rand()*m_crashSphereRadius[i]*1.0f;
			dim.x = 1.0f*m_force;
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTIBLITZ, 0.5f, 0.0f, 0.0f);
		}
		else
		{
			pos = m_pos;
			pos.x += (Rand()-0.5f)*m_size*0.3f;
			pos.z += (Rand()-0.5f)*m_size*0.3f;
			speed.x = (Rand()-0.5f)*m_size*0.1f;
			speed.z = (Rand()-0.5f)*m_size*0.1f;
			speed.y = Rand()*m_size*0.2f;
			dim.x = 1.0f*m_force;
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTIBLITZ, 0.5f, 0.0f, 0.0f);
		}
	}

	if ( m_type == PT_SHOTW &&
		 m_lastParticuleSmoke+m_engine->ParticuleAdapt(0.10f) <= m_time )
	{
		m_lastParticuleSmoke = m_time;

		pos = m_pos;
		pos.y -= 2.0f;
		pos.x += (Rand()-0.5f)*4.0f;
		pos.z += (Rand()-0.5f)*4.0f;
		speed.x = 0.0f;
		speed.z = 0.0f;
		speed.y = 10.0f+Rand()*10.0f;
		dim.x = Rand()*2.5f+2.0f*m_force;
		dim.y = dim.x;
		m_particule->CreateParticule(pos, speed, dim, PARTICRASH, 4.0f);
	}

	if ( (m_type == PT_FRAGT || m_type == PT_EXPLOT) &&
		 m_progress < 0.05f &&
		 m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
	{
		m_lastParticule = m_time;

		pos = m_pos;
		speed.x = (Rand()-0.5f)*m_size*1.0f;
		speed.z = (Rand()-0.5f)*m_size*1.0f;
		speed.y = Rand()*m_size*0.50f;
		dim.x = Rand()*m_size/5.0f+m_size/5.0f;
		dim.y = dim.x;

		m_particule->CreateParticule(pos, speed, dim, PARTIEXPLOT);
	}

	if ( (m_type == PT_FRAGT || m_type == PT_EXPLOT) &&
		 m_progress < 0.10f &&
		 m_lastParticuleSmoke+m_engine->ParticuleAdapt(0.10f) <= m_time )
	{
		m_lastParticuleSmoke = m_time;

		dim.x = Rand()*m_size/3.0f+m_size/3.0f;
		dim.y = dim.x;
		pos = m_pos;
		pos.x += (Rand()-0.5f)*m_size*0.5f;
		pos.z += (Rand()-0.5f)*m_size*0.5f;
		m_terrain->MoveOnFloor(pos);
		speed.x = 0.0f;
		speed.z = 0.0f;
		speed.y = -dim.x/2.0f/4.0f;
		pos.y += dim.x/2.0f;

		r = rand()%2;
		if ( r == 0 )  type = PARTISMOKE1;
		if ( r == 1 )  type = PARTISMOKE2;
		m_particule->CreateParticule(pos, speed, dim, type, 6.0f);
	}

	if ( (m_type == PT_FRAGO || m_type == PT_EXPLOO) &&
		 m_progress < 0.03f &&
		 m_lastParticule+m_engine->ParticuleAdapt(0.1f) <= m_time )
	{
		m_lastParticule = m_time;

		pos = m_pos;
		speed.x = (Rand()-0.5f)*m_size*2.0f;
		speed.z = (Rand()-0.5f)*m_size*2.0f;
		speed.y = Rand()*m_size*1.0f;
		dim.x = Rand()*m_size/2.0f+m_size/2.0f;
		dim.y = dim.x;

		m_particule->CreateParticule(pos, speed, dim, PARTIEXPLOO);
	}

	if ( (m_type == PT_FRAGW || m_type == PT_EXPLOW) &&
		 m_progress < 0.05f &&
		 m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
	{
		m_lastParticule = m_time;

		pos = m_pos;
		speed.x = (Rand()-0.5f)*m_size*1.0f;
		speed.z = (Rand()-0.5f)*m_size*1.0f;
		speed.y = Rand()*m_size*0.50f;
		dim.x = 1.0f;
		dim.y = dim.x;

		m_particule->CreateParticule(pos, speed, dim, PARTIBLITZ, 0.5f, 0.0f, 0.0f);
	}

	if ( (m_type == PT_FRAGW || m_type == PT_EXPLOW) &&
		 m_progress < 0.25f &&
		 m_lastParticuleSmoke+m_engine->ParticuleAdapt(0.05f) <= m_time )
	{
		m_lastParticuleSmoke = m_time;

		pos = m_pos;
		pos.y -= 2.0f;
		pos.x += (Rand()-0.5f)*4.0f;
		pos.z += (Rand()-0.5f)*4.0f;
		speed.x = 0.0f;
		speed.z = 0.0f;
		speed.y = 4.0f+Rand()*4.0f;
		dim.x = Rand()*2.5f+2.0f;
		dim.y = dim.x;
		m_particule->CreateParticule(pos, speed, dim, PARTICRASH, 4.0f);
	}

	if ( m_type == PT_WPCHECK )
	{
		if ( m_progress < 0.25f )
		{
			factor = 0.0f;
		}
		else
		{
			factor = powf((m_progress-0.25f)/0.75f, 2.0f)*30.0f;
		}

		if ( m_progress < 0.85f &&
			 m_lastParticule+m_engine->ParticuleAdapt(0.10f) <= m_time )
		{
			m_lastParticule = m_time;

			pos = m_pos;
			pos.y += factor;
			pos.x += (Rand()-0.5f)*3.0f;
			pos.z += (Rand()-0.5f)*3.0f;
			speed.x = 0.0f;
			speed.z = 0.0f;
			speed.y = 5.0f+Rand()*5.0f;
			dim.x = Rand()*1.5f+1.5f;
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTIGLINT, 2.0f);
//?			m_particule->CreateParticule(pos, speed, dim, (ParticuleType)(PARTILENS1+rand()%4), 2.0f);
		}

		angle = m_object->RetAngle(0);
		angle.y = m_progress*20.0f;
		angle.x = sinf(m_progress*49.0f)*0.3f;
		angle.z = sinf(m_progress*47.0f)*0.2f;
		m_object->SetAngle(0, angle);

		pos = m_pos;
		pos.y += factor;
		m_object->SetPosition(0, pos);

		if ( m_progress > 0.85f )
		{
			m_object->SetZoom(0, 1.0f-(m_progress-0.85f)/0.15f);
		}
	}

	if ( m_type == PT_FLCREATE )
	{
		if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
		{
			m_lastParticule = m_time;

			pos = m_pos;
			m_terrain->MoveOnFloor(pos);
			pos.x += (Rand()-0.5f)*1.0f;
			pos.z += (Rand()-0.5f)*1.0f;
			speed.x = (Rand()-0.5f)*2.0f;
			speed.z = (Rand()-0.5f)*2.0f;
			speed.y = 2.0f+Rand()*2.0f;
			dim.x = (Rand()*1.0f+1.0f)*(0.2f+m_progress*0.8f);
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTIGLINT, 2.0f, 0.0f, 0.0f);
		}

		angle = m_object->RetAngle(0);
//?		angle.y = powf(m_progress, 0.2f)*20.0f;
		angle.x = sinf(m_progress*49.0f)*0.3f*(1.0f-m_progress);
		angle.z = sinf(m_progress*47.0f)*0.2f*(1.0f-m_progress);
		m_object->SetAngle(0, angle);

		m_object->SetZoom(0, m_progress);
	}

	if ( m_type == PT_FLDELETE )
	{
		if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
		{
			m_lastParticule = m_time;

			pos = m_pos;
			m_terrain->MoveOnFloor(pos);
			pos.x += (Rand()-0.5f)*1.0f;
			pos.z += (Rand()-0.5f)*1.0f;
			speed.x = (Rand()-0.5f)*2.0f;
			speed.z = (Rand()-0.5f)*2.0f;
			speed.y = 2.0f+Rand()*2.0f;
			dim.x = (Rand()*1.0f+1.0f)*(0.2f+m_progress*0.8f);
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTIGLINT, 2.0f, 0.0f, 0.5f);
		}

		angle = m_object->RetAngle(0);
		angle.y = m_progress*20.0f;
		angle.x = sinf(m_progress*49.0f)*0.3f;
		angle.z = sinf(m_progress*47.0f)*0.2f;
		m_object->SetAngle(0, angle);

		m_object->SetZoom(0, 1.0f-m_progress);
	}

	if ( m_type == PT_RESET )
	{
#if 0
		if ( m_lastParticule+m_engine->ParticuleAdapt(0.10f) <= m_time )
		{
			m_lastParticule = m_time;

			pos = m_pos;
			speed.x = (Rand()-0.5f)*6.0f;
			speed.z = (Rand()-0.5f)*6.0f;
			speed.y = Rand()*12.0f;
			dim.x = (Rand()*2.5f+2.5f)*(1.0f-m_progress*0.9f);
			dim.y = dim.x;
			pos.y += dim.y;
			m_particule->CreateParticule(pos, speed, dim, 
										 (ParticuleType)(PARTILENS1+rand()%4),
										 Rand()*2.5f+2.5f,
										 Rand()*5.0f+5.0f, 0.0f);
		}
#else
		if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
		{
			m_lastParticule = m_time;

			pos = m_pos;
			pos.x += (Rand()-0.5f)*5.0f;
			pos.z += (Rand()-0.5f)*5.0f;
			speed.x = 0.0f;
			speed.z = 0.0f;
			speed.y = 5.0f+Rand()*5.0f;
			dim.x = Rand()*2.0f+2.0f;
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTIGLINTb, 2.0f);

			pos = m_pos;
			speed.x = (Rand()-0.5f)*20.0f;
			speed.z = (Rand()-0.5f)*20.0f;
			speed.y = Rand()*10.0f;
			speed *= 0.5f+m_progress*0.5f;
			dim.x = 0.6f;
			dim.y = dim.x;
			pos.y += dim.y;
			duration = Rand()*1.5f+1.5f;
			m_particule->CreateTrack(pos, speed, dim, PARTITRACK6,
									 duration, 0.0f,
									 duration*0.9f, 0.7f);
		}
#endif

		angle = m_object->RetResetAngle();
		m_object->SetAngleY(0, angle.y-powf((1.0f-m_progress)*5.0f, 2.0f));
		m_object->SetZoom(0, m_progress);
	}

	if ( m_type == PT_FINDING )
	{
		if ( m_object != 0 &&
			 m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
		{
			m_lastParticule = m_time;

			factor = m_size*0.3f;
			if ( m_object->RetType() == OBJECT_SAFE )  factor *= 1.3f;
			if ( factor > 40.0f )  factor = 40.0f;
			pos = m_pos;
			m_terrain->MoveOnFloor(pos);
			pos.x += (Rand()-0.5f)*factor;
			pos.z += (Rand()-0.5f)*factor;
			speed.x = (Rand()-0.5f)*2.0f;
			speed.z = (Rand()-0.5f)*2.0f;
			speed.y = 4.0f+Rand()*4.0f;
			dim.x = (Rand()*3.0f+3.0f)*(1.0f-m_progress*0.9f);
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTIGLINT, 2.0f, 0.0f, 0.5f);
		}
	}

	if ( (m_type == PT_BURNT || m_type == PT_BURNO) &&
		 m_object != 0 )
	{
		if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
		{
			m_lastParticule = m_time;

			factor = m_size/25.0f;  // 1 = taille standard

			pos = m_object->RetPosition(0);
			pos.y -= m_object->RetCharacter()->height;
			pos.x += (Rand()-0.5f)*(4.0f+8.0f*m_progress)*factor;
			pos.z += (Rand()-0.5f)*(4.0f+8.0f*m_progress)*factor;
			speed.x = 0.0f;
			speed.z = 0.0f;
			speed.y = 0.0f;
			dim.x = (Rand()*2.5f+1.0f)*factor;
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTIFLAME, 2.0f, 0.0f, 0.2f);

			pos = m_object->RetPosition(0);
			pos.y -= m_object->RetCharacter()->height;
			pos.x += (Rand()-0.5f)*(2.0f+4.0f*m_progress)*factor;
			pos.z += (Rand()-0.5f)*(2.0f+4.0f*m_progress)*factor;
			speed.x = 0.0f;
			speed.z = 0.0f;
			speed.y = (Rand()*5.0f*m_progress+3.0f)*factor;
			dim.x = (Rand()*2.0f+1.0f)*factor;
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTIFLAME, 2.0f, 0.0f, 0.2f);

			pos = m_object->RetPosition(0);
			pos.y -= 2.0f;
			pos.x += (Rand()-0.5f)*5.0f*factor;
			pos.z += (Rand()-0.5f)*5.0f*factor;
			speed.x = 0.0f;
			speed.z = 0.0f;
			speed.y = (6.0f+Rand()*6.0f+m_progress*6.0f)*factor;
			dim.x = (Rand()*1.5f+1.0f+m_progress*3.0f)*factor;
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTISMOKE3, 4.0f);
		}

		if ( m_type == PT_BURNT )
		{
			BurnProgress();
		}
		else
		{
			speed.y = 0.0f;
			speed.x = (Rand()-0.5f)*m_progress*1.0f;
			speed.z = (Rand()-0.5f)*m_progress*1.0f;
			if ( m_progress > 0.8f )
			{
				prog = (m_progress-0.8f)/0.2f;  // 0..1
				speed.y = -prog*6.0f;  // s'enfonce dans le sol
				m_object->SetZoom(0, 1.0f-prog*0.5f);
			}
			m_object->SetLinVibration(speed);
		}
	}

	if ( m_type == PT_WIN )
	{
		if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
		{
			m_lastParticule = m_time;

			pos = m_object->RetPosition(0);
			pos.y += 1.5f;
			speed.x = (Rand()-0.5f)*10.0f;
			speed.z = (Rand()-0.5f)*10.0f;
			speed.y = 8.0f+Rand()*8.0f;
			dim.x = Rand()*0.2f+0.2f;
			dim.y = dim.x;
			m_particule->CreateTrack(pos, speed, dim,
									 (ParticuleType)(PARTITRACK7+rand()%4),
									 3.0f, 20.0f, 1.0f, 0.4f);
		}
	}

	if ( m_type == PT_LOST )
	{
		if ( m_lastParticule+m_engine->ParticuleAdapt(0.10f) <= m_time )
		{
			m_lastParticule = m_time;

			pos = m_object->RetPosition(0);
			pos.y -= 2.0f;
			pos.x += (Rand()-0.5f)*10.0f;
			pos.z += (Rand()-0.5f)*10.0f;
			speed.x = 0.0f;
			speed.z = 0.0f;
			speed.y = 1.0f+Rand()*1.0f;
			dim.x = Rand()*1.0f+1.0f;
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTISMOKE1, 8.0f, 0.0f, 0.0f);
		}
	}

	if ( m_type == PT_FALL )
	{
		FallProgress(event.rTime);
	}

	if ( m_lightRank != -1 )
	{
		LightOperFrame(event.rTime);
	}

	return TRUE;
}

// Indique que l'objet li� � l'effet n'existe plus, sans le d�truire.

void CPyro::CutObjectLink(CObject* pObj)
{
	if ( m_object == pObj )
	{
		m_object = 0;
	}
}

// Indique si l'effet pyrotechnique est termin�.

Error CPyro::IsEnded()
{
	// D�truit l'objet qui explose. Il ne doit pas �tre d�truit � la fin
	// du Create, car c'est parfois l'objet lui-m�me qui fait le Create :
	//	pyro->Create(PT_FRAGT, this);
	if ( m_type == PT_FRAGT  ||
		 m_type == PT_FRAGO  ||
		 m_type == PT_FRAGW  ||
		 m_type == PT_SPIDER ||
		 m_type == PT_EGG    )
	{
		DeleteObject(TRUE, TRUE);
	}

	if ( m_type == PT_FALL )  // fret qui tombe ?
	{
		return FallIsEnded();
	}

	if ( m_type == PT_WIN  ||
		 m_type == PT_LOST )
	{
		return ERR_CONTINUE;
	}

	// Fin de l'effet pyrotechnique ?
	if ( m_progress < 1.0f )  return ERR_CONTINUE;

	if ( m_type == PT_EXPLOT ||
		 m_type == PT_EXPLOO ||
		 m_type == PT_EXPLOW )  // explosion ?
	{
		ExploTerminate();
	}

	if ( m_type == PT_BURNT ||
		 m_type == PT_BURNO )  // br�le ?
	{
		BurnTerminate();
	}

	if ( m_type == PT_WPCHECK  ||
		 m_type == PT_FLDELETE )
	{
		DeleteObject(TRUE, TRUE);
	}

	if ( m_type == PT_FLCREATE )
	{
		m_object->SetAngleX(0, 0.0f);
		m_object->SetAngleZ(0, 0.0f);
		m_object->SetZoom(0, 1.0f);
	}

	if ( m_type == PT_RESET )
	{
		m_object->SetPosition(0, m_object->RetResetPosition());
		m_object->SetAngle(0, m_object->RetResetAngle());
		m_object->SetZoom(0, 1.0f);
	}

	if ( m_lightRank != -1 )
	{
		m_light->DeleteLight(m_lightRank);
		m_lightRank = -1;
	}

	return ERR_STOP;
}

// Supprime l'objet li� � l'effet pyrotechnique.

void CPyro::DeleteObject(BOOL bPrimary, BOOL bSecondary)
{
	CObject		*sub, *truck;
	D3DVECTOR	pos;
	ObjectType	type;

	if ( m_object == 0 )  return;

	if ( m_object->RetResetCap() == RESET_MOVE )  // objet resetable ?
	{
		m_object->SetEnable(FALSE);  // objet cach� et inactif
		pos = m_object->RetPosition(0);
		pos.y = -100.0f;
		m_object->SetPosition(0, pos);
		return;
	}

	type = m_object->RetType();
	if ( bSecondary             &&
		 type != OBJECT_FACTORY &&
		 type != OBJECT_NUCLEAR &&
		 type != OBJECT_ENERGY  )
	{
		sub = m_object->RetPower();
		if ( sub != 0 )
		{
			sub->DeleteObject();  // supprime la pile
			delete sub;
			m_object->SetPower(0);
		}

		sub = m_object->RetFret();
		if ( sub != 0 )
		{
			sub->DeleteObject();  // supprime l'objet transport�
			delete sub;
			m_object->SetFret(0);
		}
	}

	if ( bPrimary )
	{
		truck = m_object->RetTruck();
		if ( truck != 0 )  // objet port� ?
		{
			if ( truck->RetPower() == m_object )
			{
				truck->SetPower(0);
			}
			if ( truck->RetFret() == m_object )
			{
				truck->SetFret(0);
			}
		}

		sub = m_object;
		sub->DeleteObject();  // supprime l'objet (*)
		delete sub;
		m_object = 0;
	}
}

// (*)	CObject::DeleteObject peut remettre � z�ro m_object
//		par le biais de CPyro::CutObjectLink !


// Vide la table des op�rations d'animation de la lumi�re.

void CPyro::LightOperFlush()
{
	m_lightOperTotal = 0;
}

// Ajoute une op�ration d'animation de la lumi�re.

void CPyro::LightOperAdd(float progress, float intensity,
						 float r, float g, float b)
{
	int		i;

	i = m_lightOperTotal;

	m_lightOper[i].progress  = progress;
	m_lightOper[i].intensity = intensity;
	m_lightOper[i].color.r   = r;
	m_lightOper[i].color.g   = g;
	m_lightOper[i].color.b   = b;

	m_lightOperTotal ++;
}

// Fait �voluer la lumi�re associ�e.

void CPyro::LightOperFrame(float rTime)
{
	D3DCOLORVALUE	color;
	float			progress, intensity;
	int				i;

	for ( i=0 ; i<m_lightOperTotal ; i++ )
	{
		if ( m_progress < m_lightOper[i].progress )
		{
			progress = (m_progress-m_lightOper[i-1].progress) / (m_lightOper[i].progress-m_lightOper[i-1].progress);

			intensity = m_lightOper[i-1].intensity + (m_lightOper[i].intensity-m_lightOper[i-1].intensity)*progress;
			color.r = m_lightOper[i-1].color.r + (m_lightOper[i].color.r-m_lightOper[i-1].color.r)*progress;
			color.g = m_lightOper[i-1].color.g + (m_lightOper[i].color.g-m_lightOper[i-1].color.g)*progress;
			color.b = m_lightOper[i-1].color.b + (m_lightOper[i].color.b-m_lightOper[i-1].color.b)*progress;

			m_light->SetLightIntensity(m_lightRank, intensity);
			m_light->SetLightColor(m_lightRank, color);
			break;
		}
	}
}


// Cr�e la lumi�re pour accompagner un effet pyrotechnique.

BOOL CPyro::CreateLight(D3DVECTOR pos, float height)
{
	D3DLIGHT7	light;

	if ( !m_engine->RetLightMode() )  return TRUE;

	m_lightHeight = height;

    ZeroMemory( &light, sizeof(light) );
	light.dltType       = D3DLIGHT_SPOT;
	light.dvPosition.x  = pos.x;
	light.dvPosition.y  = pos.y+height;
	light.dvPosition.z  = pos.z;
	light.dvDirection.x =  0.0f;
	light.dvDirection.y = -1.0f;  // contre en bas
	light.dvDirection.z =  0.0f;
	light.dvRange = D3DLIGHT_RANGE_MAX;
	light.dvFalloff = 1.0f;
	light.dvAttenuation0 = 1.0f;
	light.dvAttenuation1 = 0.0f;
	light.dvAttenuation2 = 0.0f;
	light.dvTheta = 0.0f;
	light.dvPhi = PI/4.0f;

	m_lightRank = m_light->CreateLight();
	if ( m_lightRank == -1 )  return FALSE;

	m_light->SetLight(m_lightRank, light);
	m_light->SetLightIntensity(m_lightRank, 0.0f);

	// N'�claire que les objets du terrain.
	m_light->SetLightIncluType(m_lightRank, TYPETERRAIN);

	return TRUE;
}


// D�marre l'explosion d'un v�hicule.

void CPyro::ExploStart()
{
	D3DVECTOR	pos, angle, speed, min, max;
	float		weight;
	int			i, objRank, channel;

	m_burnType = m_object->RetType();

	pos = m_object->RetPosition(0);
	m_burnFall = m_terrain->RetFloorHeight(pos, TRUE);

	m_object->Simplify();
	m_object->SetLock(TRUE);  // ruine pas encore utilisable
	m_object->SetExplo(TRUE);  // en cours de destruction
	m_object->FlatParent();

	if ( m_object->RetSelect() )
	{
		m_object->SetSelect(FALSE);  // d�s�lectionne l'objet
		m_camera->SetType(CAMERA_EXPLO);
		m_main->DeselectAll();
	}
	m_object->DeleteDeselList(m_object);

	for ( i=0 ; i<OBJECTMAXPART ; i++ )
	{
		objRank = m_object->RetObjectRank(i);
		if ( objRank == -1 )  continue;
		m_engine->ChangeSecondTexture(objRank, "dirty04.tga");

		pos = m_object->RetPosition(i);

		if ( i == 0 )  // partie principale ?
		{
			weight = 0.0f;

			speed.y = -1.0f;
			speed.x = 0.0f;
			speed.z = 0.0f;
		}
		else
		{
			m_engine->GetBBox(objRank, min, max);
			weight = Length(min, max);  // poids selon dimensions !

			speed.y = 10.0f+Rand()*20.0f;
			speed.x = (Rand()-0.5f)*20.0f;
			speed.z = (Rand()-0.5f)*20.0f;
		}

		channel = m_particule->CreatePart(pos, speed, PARTIPART, 10.0f, 20.0f, weight, 0.5f);
		if ( channel != -1 )
		{
			m_object->SetMasterParticule(i, channel);
		}
	}
	m_engine->LoadTexture("dirty04.tga", 1);

	DeleteObject(FALSE, TRUE);  // d�truit l'objet transport� + la pile
}

// Termine l'explosion d'un v�hicule.

void CPyro::ExploTerminate()
{
	DeleteObject(TRUE, FALSE);  // supprime l'objet principal
}


// D�marre le feu d'un v�hicule.

void CPyro::BurnStart()
{
	D3DVECTOR	pos, angle;
	int			i, objRank;

	m_burnType = m_object->RetType();

	pos = m_object->RetPosition(0);
	m_burnFall = m_terrain->RetFloorHeight(pos, TRUE);

	m_object->Simplify();
	m_object->SetLock(TRUE);  // ruine pas encore utilisable

	if ( m_object->RetSelect() )
	{
		m_object->SetSelect(FALSE);  // d�s�lectionne l'objet
		m_camera->SetType(CAMERA_EXPLO);
		m_main->DeselectAll();
	}
	m_object->DeleteDeselList(m_object);

	for ( i=0 ; i<OBJECTMAXPART ; i++ )
	{
		objRank = m_object->RetObjectRank(i);
		if ( objRank == -1 )  continue;
		m_engine->ChangeSecondTexture(objRank, "dirty04.tga");
	}
	m_engine->LoadTexture("dirty04.tga", 1);

	m_burnPartTotal = 0;

	if ( m_burnType == OBJECT_DERRICK  ||
		 m_burnType == OBJECT_FACTORY  ||
		 m_burnType == OBJECT_REPAIR   ||
		 m_burnType == OBJECT_DESTROYER||
		 m_burnType == OBJECT_CONVERT  ||
		 m_burnType == OBJECT_TOWER    ||
		 m_burnType == OBJECT_RESEARCH ||
		 m_burnType == OBJECT_ENERGY   ||
		 m_burnType == OBJECT_LABO     )
	{
		pos.x =   0.0f;
		pos.y = -(4.0f+Rand()*4.0f);
		pos.z =   0.0f;
		angle.x = (Rand()-0.5f)*0.4f;
		angle.y = 0.0f;
		angle.z = (Rand()-0.5f)*0.4f;
	}
	else if ( m_burnType == OBJECT_STATION ||
			  m_burnType == OBJECT_RADAR   ||
			  m_burnType == OBJECT_INFO    )
	{
		pos.x =   0.0f;
		pos.y = -(1.0f+Rand()*1.0f);
		pos.z =   0.0f;
		angle.x = (Rand()-0.5f)*0.2f;
		angle.y = 0.0f;
		angle.z = (Rand()-0.5f)*0.2f;
	}
	else if ( m_burnType == OBJECT_NUCLEAR )
	{
		pos.x =   0.0f;
		pos.y = -(10.0f+Rand()*10.0f);
		pos.z =   0.0f;
		angle.x = (Rand()-0.5f)*0.4f;
		angle.y = 0.0f;
		angle.z = (Rand()-0.5f)*0.4f;
	}
	else if ( m_burnType == OBJECT_PARA )
	{
		pos.x =   0.0f;
		pos.y = -(10.0f+Rand()*10.0f);
		pos.z =   0.0f;
		angle.x = (Rand()-0.5f)*0.4f;
		angle.y = 0.0f;
		angle.z = (Rand()-0.5f)*0.4f;
	}
	else if ( m_burnType == OBJECT_SAFE )
	{
		pos.x =   0.0f;
		pos.y = -(10.0f+Rand()*10.0f);
		pos.z =   0.0f;
		angle.x = (Rand()-0.5f)*0.4f;
		angle.y = 0.0f;
		angle.z = (Rand()-0.5f)*0.4f;
	}
	else if ( m_burnType == OBJECT_HUSTON )
	{
		pos.x =   0.0f;
		pos.y = -(10.0f+Rand()*10.0f);
		pos.z =   0.0f;
		angle.x = (Rand()-0.5f)*0.4f;
		angle.y = 0.0f;
		angle.z = (Rand()-0.5f)*0.4f;
	}
	else if ( m_burnType == OBJECT_MOBILEwa ||
			  m_burnType == OBJECT_MOBILEwc ||
			  m_burnType == OBJECT_MOBILEwi ||
			  m_burnType == OBJECT_MOBILEws ||
			  m_burnType == OBJECT_MOBILEwt )
	{
		pos.x =   0.0f;
		pos.y = -(0.5f+Rand()*1.0f);
		pos.z =   0.0f;
		angle.x = (Rand()-0.5f)*0.8f;
		angle.y = 0.0f;
		angle.z = (Rand()-0.5f)*0.4f;
	}
	else if ( m_burnType == OBJECT_TEEN31 )  // basket ?
	{
		pos.x =   0.0f;
		pos.y =   0.0f;
		pos.z =   0.0f;
		angle.x = (Rand()-0.5f)*0.8f;
		angle.y = 0.0f;
		angle.z = (Rand()-0.5f)*0.2f;
	}
	else
	{
		pos.x =   0.0f;
		pos.y = -(2.0f+Rand()*2.0f);
		pos.z =   0.0f;
		angle.x = (Rand()-0.5f)*0.8f;
		angle.y = 0.0f;
		angle.z = (Rand()-0.5f)*0.8f;
	}
	BurnAddPart(0, pos, angle);  // mouvement de la partie principale

	m_burnKeepPart[0] = -1;  // rien � garder

	if ( m_burnType == OBJECT_DERRICK )
	{
		pos.x =   0.0f;
		pos.y = -40.0f;
		pos.z =   0.0f;
		angle.x = 0.0f;
		angle.y = 0.0f;
		angle.z = 0.0f;
		BurnAddPart(1, pos, angle);  // descend le foret
	}

	if ( m_burnType == OBJECT_REPAIR )
	{
		pos.x =   0.0f;
		pos.y = -12.0f;
		pos.z =   0.0f;
		angle.x = (Rand()-0.5f)*0.2f;
		angle.y = (Rand()-0.5f)*0.2f;
		angle.z = -90.0f*PI/180.0f;
		BurnAddPart(1, pos, angle);  // descend le capteur
	}

	if ( m_burnType == OBJECT_DESTROYER )
	{
		pos.x =   0.0f;
		pos.y = -12.0f;
		pos.z =   0.0f;
		angle.x = (Rand()-0.5f)*0.2f;
		angle.y = (Rand()-0.5f)*0.2f;
		angle.z = -90.0f*PI/180.0f;
		BurnAddPart(1, pos, angle);  // descend le capteur
	}

	if ( m_burnType == OBJECT_CONVERT )
	{
		pos.x =    0.0f;
		pos.y = -200.0f;
		pos.z =    0.0f;
		angle.x = (Rand()-0.5f)*0.5f;
		angle.y = (Rand()-0.5f)*0.5f;
		angle.z = 0.0f;
		BurnAddPart(1, pos, angle);  // descend le couvercle
		BurnAddPart(2, pos, angle);
		BurnAddPart(3, pos, angle);
	}

	if ( m_burnType == OBJECT_TOWER )
	{
		pos.x =  0.0f;
		pos.y = -7.0f;
		pos.z =  0.0f;
		angle.x = (Rand()-0.5f)*0.4f;
		angle.y = (Rand()-0.5f)*0.4f;
		angle.z = 0.0f;
		BurnAddPart(1, pos, angle);  // descend le canon
	}

	if ( m_burnType == OBJECT_RESEARCH )
	{
		pos.x =  0.0f;
		pos.y = -7.0f;
		pos.z =  0.0f;
		angle.x = (Rand()-0.5f)*0.2f;
		angle.y = (Rand()-0.5f)*0.2f;
		angle.z = 0.0f;
		BurnAddPart(1, pos, angle);  // descend l'an�mom�tre
	}

	if ( m_burnType == OBJECT_RADAR )
	{
		pos.x =   0.0f;
		pos.y = -14.0f;
		pos.z =   0.0f;
		angle.x = (Rand()-0.5f)*0.4f;
		angle.y = (Rand()-0.5f)*0.4f;
		angle.z = 0.0f;
		BurnAddPart(1, pos, angle);  // descend le radar
		BurnAddPart(2, pos, angle);
	}

	if ( m_burnType == OBJECT_INFO )
	{
		pos.x =   0.0f;
		pos.y = -14.0f;
		pos.z =   0.0f;
		angle.x = (Rand()-0.5f)*0.4f;
		angle.y = (Rand()-0.5f)*0.4f;
		angle.z = 0.0f;
		BurnAddPart(1, pos, angle);  // descend la borne d'information
		BurnAddPart(2, pos, angle);
	}

	if ( m_burnType == OBJECT_LABO )
	{
		pos.x =   0.0f;
		pos.y = -12.0f;
		pos.z =   0.0f;
		angle.x = 0.0f;
		angle.y = 0.0f;
		angle.z = 0.0f;
		BurnAddPart(1, pos, angle);  // descend le bras
	}

	if ( m_burnType == OBJECT_NUCLEAR )
	{
		pos.x = 0.0f;
		pos.y = 0.0f;
		pos.z = 0.0f;
		angle.x = 0.0f;
		angle.y = 0.0f;
		angle.z = -135.0f*PI/180.0f;
		BurnAddPart(1, pos, angle);  // descend le couvercle
	}

	if ( m_burnType == OBJECT_MOBILEfa ||
		 m_burnType == OBJECT_MOBILEta ||
		 m_burnType == OBJECT_MOBILEwa ||
		 m_burnType == OBJECT_MOBILEia )
	{
		pos.x =  2.0f;
		pos.y = -5.0f;
		pos.z =  0.0f;
		angle.x = (Rand()-0.5f)*0.2f;
		angle.y = (Rand()-0.5f)*0.2f;
		angle.z = 40.0f*PI/180.0f;
		BurnAddPart(1, pos, angle);  // descend le bras
	}

	if ( m_burnType == OBJECT_MOBILEfs ||
		 m_burnType == OBJECT_MOBILEts ||
		 m_burnType == OBJECT_MOBILEws ||
		 m_burnType == OBJECT_MOBILEis )
	{
		pos.x =  0.0f;
		pos.y = -7.0f;
		pos.z =  0.0f;
		angle.x = (Rand()-0.5f)*0.2f;
		angle.y = (Rand()-0.5f)*0.2f;
		angle.z = 50.0f*PI/180.0f;
		BurnAddPart(1, pos, angle);  // descend le capteur
	}

	if ( m_burnType == OBJECT_MOBILEfc ||
		 m_burnType == OBJECT_MOBILEtc ||
		 m_burnType == OBJECT_MOBILEwc ||
		 m_burnType == OBJECT_MOBILEic )
	{
		pos.x = -1.5f;
		pos.y = -5.0f;
		pos.z =  0.0f;
		angle.x = (Rand()-0.5f)*0.2f;
		angle.y = (Rand()-0.5f)*0.2f;
		angle.z = -25.0f*PI/180.0f;
		BurnAddPart(1, pos, angle);  // descend le canon
	}

	if ( m_burnType == OBJECT_MOBILEfi ||
		 m_burnType == OBJECT_MOBILEti ||
		 m_burnType == OBJECT_MOBILEwi ||
		 m_burnType == OBJECT_MOBILEii )
	{
		pos.x = -1.5f;
		pos.y = -5.0f;
		pos.z =  0.0f;
		angle.x = (Rand()-0.5f)*0.2f;
		angle.y = (Rand()-0.5f)*0.2f;
		angle.z = -25.0f*PI/180.0f;
		BurnAddPart(1, pos, angle);  // descend le canon-insecte
	}

	if ( m_burnType == OBJECT_MOBILErt ||
		 m_burnType == OBJECT_MOBILErc )
	{
		pos.x =   0.0f;
		pos.y = -10.0f;
		pos.z =   0.0f;
		angle.x = 0.0f;
		angle.y = 0.0f;
		angle.z = 0.0f;
		BurnAddPart(1, pos, angle);  // descend le support

		pos.x =   0.0f;
		pos.y = -10.0f;
		pos.z =   0.0f;
		angle.x = 0.0f;
		angle.y = 0.0f;
		angle.z = 0.0f;
		BurnAddPart(2, pos, angle);  // descend le pilon/canon
	}

	if ( m_burnType == OBJECT_MOBILErr )
	{
		pos.x =   0.0f;
		pos.y = -10.0f;
		pos.z =   0.0f;
		angle.x = 0.0f;
		angle.y = 0.0f;
		angle.z = 0.0f;
		BurnAddPart(1, pos, angle);  // descend le support

		pos.x =   0.0f;
		pos.y =   0.0f;
		pos.z =   0.0f;
		angle.x = 0.0f;
		angle.y = 0.0f;
		angle.z = -PI/2.0f;
		BurnAddPart(4, pos, angle);

		pos.x =   0.0f;
		pos.y =   0.0f;
		pos.z =   0.0f;
		angle.x = 0.0f;
		angle.y = 0.0f;
		angle.z = PI/2.5f;
		BurnAddPart(2, pos, angle);
	}

	if ( m_burnType == OBJECT_MOBILErs )
	{
		pos.x =   0.0f;
		pos.y = -10.0f;
		pos.z =   0.0f;
		angle.x = 0.0f;
		angle.y = 0.0f;
		angle.z = 0.0f;
		BurnAddPart(1, pos, angle);  // descend le support

		pos.x =   0.0f;
		pos.y =  -5.0f;
		pos.z =   0.0f;
		angle.x = 0.0f;
		angle.y = 0.0f;
		angle.z = 0.0f;
		BurnAddPart(2, pos, angle);

		pos.x =   0.0f;
		pos.y =  -5.0f;
		pos.z =   0.0f;
		angle.x = 0.0f;
		angle.y = 0.0f;
		angle.z = 0.0f;
		BurnAddPart(3, pos, angle);
	}

	if ( m_burnType == OBJECT_MOBILEsa )
	{
		pos.x =   0.0f;
		pos.y = -10.0f;
		pos.z =   0.0f;
		angle.x = 0.0f;
		angle.y = 0.0f;
		angle.z = 0.0f;
		BurnAddPart(1, pos, angle);  // descend le support
	}

	if ( m_burnType == OBJECT_MOBILEwa ||
		 m_burnType == OBJECT_MOBILEwc ||
		 m_burnType == OBJECT_MOBILEwi ||
		 m_burnType == OBJECT_MOBILEws ||
		 m_burnType == OBJECT_MOBILEwt )  // roues ?
	{
		for ( i=0 ; i<4 ; i++ )
		{
			pos.x = 0.0f;
			pos.y = Rand()*0.5f;
			pos.z = 0.0f;
			angle.x = (Rand()-0.5f)*PI/2.0f;
			angle.y = (Rand()-0.5f)*PI/2.0f;
			angle.z = 0.0f;
			BurnAddPart(6+i, pos, angle);  // roue

			m_burnKeepPart[i] = 6+i;  // on garde les roues
		}
		m_burnKeepPart[i] = -1;
	}

	if ( m_burnType == OBJECT_MOBILEta ||
		 m_burnType == OBJECT_MOBILEtc ||
		 m_burnType == OBJECT_MOBILEti ||
		 m_burnType == OBJECT_MOBILEts ||
		 m_burnType == OBJECT_MOBILErt ||
		 m_burnType == OBJECT_MOBILErc ||
		 m_burnType == OBJECT_MOBILErr ||
		 m_burnType == OBJECT_MOBILErs ||
		 m_burnType == OBJECT_MOBILEsa ||
		 m_burnType == OBJECT_MOBILEdr )  // chenilles ?
	{
		pos.x =   0.0f;
		pos.y =  -4.0f;
		pos.z =   2.0f;
		angle.x = (Rand()-0.5f)*20.0f*PI/180.0f;
		angle.y = (Rand()-0.5f)*10.0f*PI/180.0f;
		angle.z = (Rand()-0.5f)*30.0f*PI/180.0f;
		BurnAddPart(6, pos, angle);  // descend la chenille droite

		pos.x =   0.0f;
		pos.y =  -4.0f;
		pos.z =  -2.0f;
		angle.x = (Rand()-0.5f)*20.0f*PI/180.0f;
		angle.y = (Rand()-0.5f)*10.0f*PI/180.0f;
		angle.z = (Rand()-0.5f)*30.0f*PI/180.0f;
		BurnAddPart(7, pos, angle);  // descend la chenille gauche
	}

	if ( m_burnType == OBJECT_MOBILEfa ||
		 m_burnType == OBJECT_MOBILEfc ||
		 m_burnType == OBJECT_MOBILEfi ||
		 m_burnType == OBJECT_MOBILEfs ||
		 m_burnType == OBJECT_MOBILEft )  // volant ?
	{
		for ( i=0 ; i<3 ; i++ )
		{
			pos.x =  0.0f;
			pos.y = -3.0f;
			pos.z =  0.0f;
			angle.x = 0.0f;
			angle.y = 0.0f;
			angle.z = (Rand()-0.5f)*PI/2.0f;
			BurnAddPart(6+i, pos, angle);  // pied
		}
		m_burnKeepPart[i] = -1;
	}

	if ( m_burnType == OBJECT_MOBILEia ||
		 m_burnType == OBJECT_MOBILEic ||
		 m_burnType == OBJECT_MOBILEii ||
		 m_burnType == OBJECT_MOBILEis )  // pattes ?
	{
		for ( i=0 ; i<6; i++ )
		{
			pos.x =  0.0f;
			pos.y = -3.0f;
			pos.z =  0.0f;
			angle.x = 0.0f;
			angle.y = (Rand()-0.5f)*PI/4.0f;
			angle.z = (Rand()-0.5f)*PI/4.0f;
			BurnAddPart(6+i, pos, angle);  // patte
		}
	}
}

// Ajoute une partie � bouger.

void CPyro::BurnAddPart(int part, D3DVECTOR pos, D3DVECTOR angle)
{
	int		i;

	i = m_burnPartTotal;
	m_burnPart[i].part = part;
	m_burnPart[i].initialPos = m_object->RetPosition(part);
	m_burnPart[i].finalPos = m_burnPart[i].initialPos+pos;
	m_burnPart[i].initialAngle = m_object->RetAngle(part);
	m_burnPart[i].finalAngle = m_burnPart[i].initialAngle+angle;

	m_burnPartTotal ++;
}

// Fait progresser le feu d'un v�hicule.

void CPyro::BurnProgress()
{
	CObject*	sub;
	D3DVECTOR	pos;
	float		h;
	int			i;

	if ( m_burnType == OBJECT_TEEN31 )  // basket ?
	{
		m_object->SetZoomY(0, 1.0f-m_progress*0.5f);  // l�ger applatissement
	}

	for ( i=0 ; i<m_burnPartTotal ; i++ )
	{
		pos = m_burnPart[i].initialPos + m_progress*(m_burnPart[i].finalPos-m_burnPart[i].initialPos);
		if ( i == 0 && m_burnFall > 0.0f )
		{
			h = powf(m_progress, 2.0f)*1000.0f;
			if ( h > m_burnFall )  h = m_burnFall;
			pos.y -= h;
		}
		m_object->SetPosition(m_burnPart[i].part, pos);

		pos = m_burnPart[i].initialAngle + m_progress*(m_burnPart[i].finalAngle-m_burnPart[i].initialAngle);
		m_object->SetAngle(m_burnPart[i].part, pos);
	}

	sub = m_object->RetPower();
	if ( sub != 0 )  // y a-t-il une pile ?
	{
		sub->SetZoomY(0, 1.0f-m_progress);  // aplatissement complet
	}
}

// Indique si une partie doit �tre conserv�e.

BOOL CPyro::BurnIsKeepPart(int part)
{
	int		i;

	i = 0;
	while ( m_burnKeepPart[i] != -1 )
	{
		if ( part == m_burnKeepPart[i++] )  return TRUE;  // faut garder
	}
	return FALSE;  // faut d�truire
}

// Termine le feu d'un insecte ou d'un v�hicule.

void CPyro::BurnTerminate()
{
	int			i, objRank;

	if ( m_type == PT_BURNO )  // br�le objet organique ?
	{
		DeleteObject(TRUE, TRUE);  // supprime l'insecte
		return;
	}

	for ( i=1 ; i<OBJECTMAXPART ; i++ )
	{
		objRank = m_object->RetObjectRank(i);
		if ( objRank == -1 )  continue;
		if ( BurnIsKeepPart(i) )  continue;

		m_object->DeletePart(i);
	}

	DeleteObject(FALSE, TRUE);  // d�truit l'objet transport� + la pile

	if ( m_burnType == OBJECT_DERRICK  ||
		 m_burnType == OBJECT_STATION  ||
		 m_burnType == OBJECT_FACTORY  ||
		 m_burnType == OBJECT_REPAIR   ||
		 m_burnType == OBJECT_DESTROYER||
		 m_burnType == OBJECT_CONVERT  ||
		 m_burnType == OBJECT_TOWER    ||
		 m_burnType == OBJECT_RESEARCH ||
		 m_burnType == OBJECT_RADAR    ||
		 m_burnType == OBJECT_INFO     ||
		 m_burnType == OBJECT_ENERGY   ||
		 m_burnType == OBJECT_LABO     ||
		 m_burnType == OBJECT_NUCLEAR  ||
		 m_burnType == OBJECT_PARA     ||
		 m_burnType == OBJECT_SAFE     ||
		 m_burnType == OBJECT_HUSTON   ||
		 m_burnType == OBJECT_START    ||
		 m_burnType == OBJECT_END      )
	{
		m_object->SetType(OBJECT_RUINfactory);  // �a devient une ruine
		m_object->SetLock(FALSE);
	}
	else
	{
		m_object->SetType(OBJECT_RUINmobilew1);  // �a devient une ruine
		m_object->SetLock(FALSE);
	}

	m_object->SetBurn(FALSE);  // ruine utilisable (c-�-d. r�cup�rable)
}


// D�but d'un objet fret qui tombe.

void CPyro::FallStart()
{
	D3DVECTOR	pos;

	m_object->SetBurn(TRUE);  // plus utilisable

	pos = m_object->RetPosition(0);
	m_fallFloor = m_terrain->RetFloorLevel(pos);
	m_fallSpeed = 0.0f;
	m_fallBulletTime = 0.0f;
	m_bFallEnding = FALSE;
}

// Cherche un objet � exploser par le boulet de l'abeille qui tombe.

CObject* CPyro::FallSearchBeeExplo()
{
	CObject*	pObj;
	D3DVECTOR	iPos, oPos;
	ObjectType	oType;
	float		iRadius, oRadius, distance, shieldRadius;
	int			i, j;

	m_object->GetCrashSphere(0, iPos, iRadius);

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		oType = pObj->RetType();
		if ( oType != OBJECT_HUMAN    &&
			 oType != OBJECT_MOBILEfa &&
			 oType != OBJECT_MOBILEta &&
			 oType != OBJECT_MOBILEwa &&
			 oType != OBJECT_MOBILEia &&
			 oType != OBJECT_MOBILEfc &&
			 oType != OBJECT_MOBILEtc &&
			 oType != OBJECT_MOBILEwc &&
			 oType != OBJECT_MOBILEic &&
			 oType != OBJECT_MOBILEfi &&
			 oType != OBJECT_MOBILEti &&
			 oType != OBJECT_MOBILEwi &&
			 oType != OBJECT_MOBILEii &&
			 oType != OBJECT_MOBILEfs &&
			 oType != OBJECT_MOBILEts &&
			 oType != OBJECT_MOBILEws &&
			 oType != OBJECT_MOBILEis &&
			 oType != OBJECT_MOBILErt &&
			 oType != OBJECT_MOBILErc &&
			 oType != OBJECT_MOBILErr &&
			 oType != OBJECT_MOBILErs &&
			 oType != OBJECT_MOBILEsa &&
			 oType != OBJECT_MOBILEtg &&
			 oType != OBJECT_MOBILEft &&
			 oType != OBJECT_MOBILEtt &&
			 oType != OBJECT_MOBILEwt &&
			 oType != OBJECT_MOBILEit &&
			 oType != OBJECT_MOBILEdr &&
			 oType != OBJECT_BASE     &&
			 oType != OBJECT_DERRICK  &&
			 oType != OBJECT_STATION  &&
			 oType != OBJECT_FACTORY  &&
			 oType != OBJECT_REPAIR   &&
			 oType != OBJECT_DESTROYER&&
			 oType != OBJECT_CONVERT  &&
			 oType != OBJECT_TOWER    &&
			 oType != OBJECT_RESEARCH &&
			 oType != OBJECT_RADAR    &&
			 oType != OBJECT_INFO     &&
			 oType != OBJECT_ENERGY   &&
			 oType != OBJECT_LABO     &&
			 oType != OBJECT_NUCLEAR  &&
			 oType != OBJECT_PARA     &&
			 oType != OBJECT_SAFE     &&
			 oType != OBJECT_HUSTON   &&
			 oType != OBJECT_METAL    &&
			 oType != OBJECT_POWER    &&
			 oType != OBJECT_ATOMIC   )  continue;

		if ( pObj->RetTruck() != 0 )  continue;  // objet transport� ?

		oPos = pObj->RetPosition(0);

		shieldRadius = pObj->RetShieldRadius();
		if ( shieldRadius > 0.0f )
		{
			distance = Length(oPos, iPos);
			if ( distance <= shieldRadius )  return pObj;
		}

		if ( oType == OBJECT_BASE )
		{
			distance = Length(oPos, iPos);
			if ( distance < 25.0f )  return pObj;
		}

		// Test au centre de l'objet, ce qui est n�cessaire pour
		// les objets qui n'ont pas de sph�re au centre (station).
		distance = Length(oPos, iPos)-4.0f;
		if ( distance < 5.0f )  return pObj;

		// Test avec toutes les sph�res de l'objet.
		j = 0;
		while ( pObj->GetCrashSphere(j++, oPos, oRadius) )
		{
			distance = Length(oPos, iPos);
			if ( distance <= iRadius+oRadius )
			{
				return pObj;
			}
		}
	}
	return 0;
}

// Chute d'un objet fret.

void CPyro::FallProgress(float rTime)
{
	CObject*	pObj;
	D3DVECTOR	pos;
	BOOL		bFloor = FALSE;

	if ( m_object == 0 )  return;

	m_fallSpeed += rTime*50.0f;  // v2 = v1 + a*dt
	pos = m_object->RetPosition(0);
	pos.y -= m_fallSpeed*rTime;  // dd -= v2*dt

	if ( pos.y <= m_fallFloor )  // sous le niveau du sol ?
	{
		pos.y = m_fallFloor;
		bFloor = TRUE;
	}
	m_object->SetPosition(0, pos);

	if ( m_object->RetType() == OBJECT_BULLET )
	{
		m_fallBulletTime += rTime;

		if ( m_fallBulletTime > 0.2f || bFloor )
		{
			m_fallBulletTime = 0.0f;

			pObj = FallSearchBeeExplo();
			if ( pObj == 0 )
			{
				if ( bFloor )  // arriv� au niveau du sol ?
				{
					m_object->ExploObject(EXPLO_BOUM, 0.0f);  // d�marre explosion
				}
			}
			else
			{
				if ( pObj->RetShieldRadius() > 0.0f )  // prot�g� par bouclier ?
				{
					m_particule->CreateParticule(pos, D3DVECTOR(0.0f, 0.0f, 0.0f), FPOINT(6.0f, 6.0f), PARTIGUNDEL, 2.0f, 0.0f, 0.0f);
					m_sound->Play(SOUND_GUNDEL);

					DeleteObject(TRUE, TRUE);  // supprime le boulet
				}
				else
				{
					if ( pObj->ExploObject(EXPLO_BOUM, 1.0f) )  // d�marre explosion
					{
						DeleteObject(TRUE, TRUE);  // supprime le boulet
					}
					else
					{
						m_object->ExploObject(EXPLO_BOUM, 0.0f);  // d�marre explosion
					}
				}
			}

			if ( bFloor || pObj != 0 )
			{
				m_bFallEnding = TRUE;
			}
		}
	}
}

// Indique si la chute est termin�e.

Error CPyro::FallIsEnded()
{
	D3DVECTOR	pos;

	if ( m_bFallEnding || m_object == 0 )  return ERR_STOP;

	pos = m_object->RetPosition(0);
	if ( pos.y > m_fallFloor )  return ERR_CONTINUE;

	m_sound->Play(SOUND_BOUM, pos);
	m_object->SetBurn(FALSE);  // de nouveau utilisable

	return ERR_STOP;
}

