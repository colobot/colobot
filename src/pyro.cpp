// pyro.cpp

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
#include "sound.h"
#include "displaytext.h"
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

	m_impact = D3DVECTOR(NAN, NAN, NAN);
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


// Détruit l'objet.

void CPyro::DeleteObject(BOOL bAll)
{
	if ( m_lightRank != -1 )
	{
		m_light->DeleteLight(m_lightRank);
		m_lightRank = -1;
	}
}


// Détermine le point d'impact.

void CPyro::SetImpact(D3DVECTOR impact)
{
	m_impact = impact;
}

// Crée un effet pyrotechnique.

BOOL CPyro::Create(PyroType type, CObject* pObj, float force, int param)
{
	D3DVECTOR		min, max, pos, speed, p1, p2, p3, p4;
	FPOINT			dim;
	ObjectType		oType;
	ParticuleType	pType;
	Sound			sound;
	float			duration, mass, h;
	int				part, objRank, total, i;

	m_object = pObj;
	m_posStart = pObj->RetPosition(0);
	m_angleStart = pObj->RetAngle(0);
	m_force = force;
	m_param = param;

	oType = pObj->RetType();
	objRank = pObj->RetObjectRank(0);
	if ( objRank == -1 )  return FALSE;
	m_engine->GetBBox(objRank, min, max);
	pos = m_posStart;

	if ( oType == OBJECT_BLUPI && m_object->RetSelect() )
	{
		m_main->SelectOther(m_object);
	}

	DisplayError(type, pObj);  // affiche message éventuel

	pos = m_object->RetPosition(0);
	m_terrain->SetLockZone(pos, LZ_FREE);  // zone maintenant accessible

	// Calcule la dimension de l'effet.
	m_size = Length(min, max)*2.0f;
	if ( m_size <  4.0f )  m_size =  4.0f;
	if ( m_size > 80.0f )  m_size = 80.0f;

	if ( oType == OBJECT_TNT  ||
		 oType == OBJECT_MINE )
	{
		m_size *= 2.0f;
	}
	if ( oType == OBJECT_BOX10 )
	{
		m_size = 60.0f;
	}
	if ( oType == OBJECT_GLASS1 ||
		 oType == OBJECT_GLASS2 )
	{
		m_size = 0.0f;
	}

	if ( m_impact.x == NAN )
	{
		m_pos = pos+(min+max)/2.0f;
	}
	else
	{
		m_pos = m_impact;
	}

	m_type = type;
	m_progress = 0.0f;
	m_speed = 1.0f/20.0f;
	m_time = 0.0f;
	m_lastParticule = 0.0f;
	m_lastParticuleSmoke = 0.0f;
	m_lightRank = -1;

	// Cherche s'il faut générer les traînées.
	m_bTracks = FALSE;
	m_posTracks = m_pos;

	if ( type == PT_EXPLOS ||
		 type == PT_EXPLOP )
	{
		m_bTracks = TRUE;
	}
	if ( type == PT_EXPLOO )
	{
//?		m_terrain->MoveOnFloor(m_pos);
		m_pos.y -= m_object->RetCharacter()->height;
		m_pos.y += 0.2f;
		m_speed = 1.0f/5.0f;
	}
	if ( oType == OBJECT_BARREL   ||
		 oType == OBJECT_BARRELa  ||
		 oType == OBJECT_ATOMIC   ||
		 oType == OBJECT_URANIUM  ||
		 oType == OBJECT_TNT      ||
		 oType == OBJECT_MINE     ||
		 oType == OBJECT_BOX10    )
	{
		m_bTracks = TRUE;
		m_posTracks = m_pos;
		m_posTracks.y += 1.0f;
		m_pos = m_posTracks;
	}
	if ( (oType >= OBJECT_BOX1 && oType <= OBJECT_BOX7 ) ||
		 (oType >= OBJECT_BOX9 && oType <= OBJECT_BOX10) ||
		 (oType >= OBJECT_KEY1 && oType <= OBJECT_KEY5 ) )
	{
		m_bTracks = TRUE;
		m_posTracks = m_pos;
		m_posTracks.y += 1.0f;
		m_pos = m_posTracks;
	}

	// Cherche s'il faut générer la sphère.
	m_bSphere = FALSE;

	if ( m_type == PT_FRAGT  ||
		 m_type == PT_EXPLOT ||
		 m_type == PT_EXPLOS )
	{
		if ( m_size > 30.0f )
		{
			m_bSphere = TRUE;
		}
	}

	// Cherche s'il faut générer l'onde de choc.
	m_bChoc = FALSE;

	if ( type == PT_FRAGT  ||
		 type == PT_EXPLOT ||
		 type == PT_EXPLOS )
	{
		if ( m_size > 10.0f || m_bTracks )
		{
			m_bChoc = TRUE;
		}
		if ( (oType >= OBJECT_BOX1 && oType <= OBJECT_BOX20) ||
			 (oType >= OBJECT_KEY1 && oType <= OBJECT_KEY5 ) )
		{
			m_bChoc = FALSE;
		}
	}

	// Fait entendre le bruit de l'effet pyrotechnique.
	if ( type == PT_FRAGT  ||
		 type == PT_GOAL   ||
		 type == PT_GLASS  ||
		 type == PT_EXPLOT ||
		 type == PT_BREAKT ||
		 type == PT_EXPLOS ||
		 type == PT_PLOUF  )
	{
		if ( m_bTracks )
		{
			sound = SOUND_EXPLOp;
		}
		else
		{
			sound = SOUND_EXPLO;
		}
		if ( oType == OBJECT_CRAZY )
		{
			sound = SOUND_FALLo2;
		}
		if ( oType == OBJECT_GOAL )
		{
			sound = SOUND_ENERGY;
		}
		if ( oType == OBJECT_GLASS1 ||
			 oType == OBJECT_GLASS2 )
		{
			sound = SOUND_GLASS;
		}
		if ( oType == OBJECT_STONE   ||
			 oType == OBJECT_METAL   ||
			 oType == OBJECT_BBOX    )
		{
			sound = SOUND_EXPLOl;
		}
		if ( oType == OBJECT_URANIUM  ||
			 oType == OBJECT_BARREL   ||
			 oType == OBJECT_BARRELa  ||
			 oType == OBJECT_ATOMIC   ||
			 oType == OBJECT_TNT      ||
			 oType == OBJECT_MINE     ||
			 (oType >= OBJECT_BOX1 && oType <= OBJECT_BOX20) ||
			 (oType >= OBJECT_KEY1 && oType <= OBJECT_KEY5 ) )
		{
			if ( m_force < 0.5f )
			{
				sound = SOUND_CHOCo;
			}
			else
			{
				sound = SOUND_EXPLOlp;
			}
		}
		if ( type == PT_PLOUF )
		{
			sound = SOUND_PLOUF;
		}
		m_sound->Play(sound, m_pos);
	}
	if ( type == PT_FRAGO )
	{
		m_sound->Play(SOUND_EXPLOlp, m_pos);
	}
	if ( type == PT_FRAGG )
	{
		m_sound->Play(SOUND_CHOCm, m_pos);
	}
	if ( type == PT_EXPLOO )
	{
		m_sound->Play(SOUND_BLUPIaie, m_pos);
	}
	if ( type == PT_BURNT ||
		 type == PT_BURNO ||
		 type == PT_BURNS )
	{
		m_soundChannel = m_sound->Play(SOUND_BURN, m_pos, 1.0f, 1.0f, TRUE);
		m_sound->AddEnvelope(m_soundChannel, 1.0f, 1.0f, 12.0f, SOPER_CONTINUE);
		m_sound->AddEnvelope(m_soundChannel, 0.0f, 1.0f,  5.0f, SOPER_STOP);
	}

	if ( type == PT_FRAGT  ||
		 type == PT_EXPLOT ||
		 type == PT_EXPLOS )
	{
		if ( oType == OBJECT_MINE && m_posStart.y >= 0.0f )
		{
			// Crée une tache d'explosion au sol.
			h = 8.0f;
			if ( !m_terrain->IsSolid(D3DVECTOR(m_pos.x+8.0f, m_pos.y, m_pos.z+0.0f)) ||
				 !m_terrain->IsSolid(D3DVECTOR(m_pos.x-8.0f, m_pos.y, m_pos.z+0.0f)) ||
				 !m_terrain->IsSolid(D3DVECTOR(m_pos.x+0.0f, m_pos.y, m_pos.z+8.0f)) ||
				 !m_terrain->IsSolid(D3DVECTOR(m_pos.x+0.0f, m_pos.y, m_pos.z-8.0f)) )
			{
				h = 5.0f;
			}
			p1 = p2 = p3 = p4 = Grid(m_pos, 8.0f);
			p1.x -= h;  p1.z += h;
			p2.x += h;  p2.z += h;
			p3.x -= h;  p3.z -= h;
			p4.x += h;  p4.z -= h;
			m_particule->CreateWheelTrace(p1, p2, p3, p4, PARTITRACE3);
		}
	}
	if ( type == PT_EXPLOO )
	{
		if ( m_posStart.y >= 0.0f )
		{
			// Crée une tache de sang au sol.
			h = 10.0f;
			if ( !m_terrain->IsSolid(D3DVECTOR(m_pos.x+8.0f, m_pos.y, m_pos.z+0.0f)) ||
				 !m_terrain->IsSolid(D3DVECTOR(m_pos.x-8.0f, m_pos.y, m_pos.z+0.0f)) ||
				 !m_terrain->IsSolid(D3DVECTOR(m_pos.x+0.0f, m_pos.y, m_pos.z+8.0f)) ||
				 !m_terrain->IsSolid(D3DVECTOR(m_pos.x+0.0f, m_pos.y, m_pos.z-8.0f)) )
			{
				h = 4.0f;
			}
			p1 = p2 = p3 = p4 = Grid(m_pos, 8.0f);
			p1.x -= h;  p1.z += h;
			p2.x += h;  p2.z += h;
			p3.x -= h;  p3.z -= h;
			p4.x += h;  p4.z -= h;
			m_particule->CreateWheelTrace(p1, p2, p3, p4, PARTITRACE4);
		}
	}

	if ( m_type == PT_FRAGT ||
		 m_type == PT_FRAGO ||
		 m_type == PT_FRAGG )
	{
		m_engine->ShadowDelete(m_object->RetObjectRank(0));
	}

	if ( m_type == PT_SHOTT )
	{
		m_camera->StartEffect(CE_SHOT, m_pos, force);
		m_speed = 1.0f/1.0f;
		return TRUE;
	}
	if ( m_type == PT_EXPLOO )
	{
		m_camera->StartOver(OE_BLOOD, m_pos, 1.0f);
	}

	if ( m_type == PT_BURNT ||
		 m_type == PT_BURNO )
	{
		BurnStart();
	}

	if ( m_type == PT_EXPLOT )
	{
		CreateTriangle(pObj, oType, 0);
		m_engine->ShadowDelete(m_object->RetObjectRank(0));
		ExploStart(oType);
	}
	if ( m_type == PT_BREAKT )
	{
		m_engine->ShadowDelete(m_object->RetObjectRank(0));
		ExploStart(oType);
	}
	if ( m_type == PT_GLASS )
	{
		m_speed = 1.0f/4.0f;
		m_engine->ShadowDelete(m_object->RetObjectRank(0));
		GlassStart(oType);
	}
	if ( m_type == PT_PLOUF )
	{
		m_speed = 1.0f/1.0f;
		m_engine->ShadowDelete(m_object->RetObjectRank(0));
		PloufStart();
	}

	if ( m_type == PT_EXPLOO )
	{
		m_engine->ShadowDelete(m_object->RetObjectRank(0));
		OrgaStart();
	}

	if ( m_type == PT_BURNT ||
		 m_type == PT_BURNO ||
		 m_type == PT_BURNS )
	{
		m_speed = 1.0f/15.0f;

		LightOperAdd(0.00f, 0.0f,  2.0f,  1.0f,  0.0f);  // rouge-orange
		LightOperAdd(0.30f, 1.0f, -0.4f, -0.4f, -0.4f);  // gris foncé
		LightOperAdd(0.80f, 1.0f, -0.4f, -0.4f, -0.4f);  // gris foncé
		LightOperAdd(1.00f, 0.0f, -0.4f, -0.4f, -0.4f);  // gris foncé
		CreateLight(m_pos, 40.0f);
		return TRUE;
	}

	// Génère les effets de lumière.
	if ( m_type != PT_EXPLOP &&
		 m_type != PT_BREAKT &&
		 m_type != PT_GLASS  &&
		 m_type != PT_GOAL   )
	{
		h = 40.0f;
		if ( m_type == PT_FRAGO  ||
			 m_type == PT_EXPLOO )
		{
			LightOperAdd(0.00f, 0.0f,  4.0f,  4.0f,  4.0f);  // blanc
			LightOperAdd(0.05f, 1.0f,  4.0f, -1.0f, -1.0f);  // rouge
			LightOperAdd(1.00f, 0.0f,  4.0f, -1.0f, -1.0f);  // rouge
			h = m_size*10.0f;
		}
		else if ( m_type == PT_FRAGT  ||
				  m_type == PT_EXPLOT ||
				  m_type == PT_EXPLOS )
		{
			LightOperAdd(0.00f, 1.0f,  4.0f,  4.0f,  2.0f);  // jaune
			LightOperAdd(0.02f, 1.0f,  4.0f,  2.0f,  0.0f);  // rouge-orange
			LightOperAdd(0.06f, 1.0f, -0.1f, -0.1f, -0.1f);  // gris foncé
			LightOperAdd(1.00f, 0.0f, -0.1f, -0.1f, -0.1f);  // gris foncé
			h = m_size*5.0f;
		}
		else
		{
			LightOperAdd(0.00f, 0.0f, -0.4f, -0.4f, -0.4f);  // gris foncé
			LightOperAdd(0.05f, 1.0f, -0.4f, -0.4f, -0.4f);  // gris foncé
			LightOperAdd(1.00f, 0.0f, -0.4f, -0.4f, -0.4f);  // gris foncé
		}
		CreateLight(m_pos, h);

		m_camera->StartEffect(CE_EXPLO, m_pos, force);
	}
	if ( m_type == PT_BREAKT )
	{
		m_camera->StartEffect(CE_EXPLO, m_pos, force);
	}

	// Génère les triangles de l'explosion.
	if ( m_type == PT_FRAGT  ||
		 m_type == PT_FRAGO  ||
		 m_type == PT_FRAGG  )
	{
		if ( (oType >= OBJECT_BOX1 && oType <= OBJECT_BOX6) ||
			  oType == OBJECT_BOX9                          ||
			 (oType >= OBJECT_KEY1 && oType <= OBJECT_KEY5) )
		{
			CreateTriangle(pObj, oType, 0);
		}
		else
		{
			for ( part=0 ; part<OBJECTMAXPART ; part++ )
			{
				CreateTriangle(pObj, oType, part);
			}
		}
	}
	if ( m_type == PT_GLASS )
	{
		CreateTriangle(pObj, oType, 0);
	}

	// Génère les particules avec traînées.
	if ( m_bTracks )
	{
		if ( m_type == PT_FRAGT  ||
			 m_type == PT_EXPLOT ||
			 m_type == PT_EXPLOS )
		{
			total = 10;
			if ( oType == OBJECT_TNT   ||
				 oType == OBJECT_MINE  ||
				 oType == OBJECT_BOX10 )  total *= 3;
			if ( oType == OBJECT_GLASS1 ||
				 oType == OBJECT_GLASS2 )  total = 0;
			for ( i=0 ; i<total ; i++ )
			{
				pos = m_posTracks;
				speed.x = (Rand()-0.5f)*30.0f*m_force;
				speed.z = (Rand()-0.5f)*30.0f*m_force;
				speed.y = Rand()*30.0f*m_force;
				dim.x = 1.0f;
				dim.y = dim.x;
				duration = Rand()*3.0f+2.0f;
				mass = Rand()*10.0f+15.0f;
				if ( (oType >= OBJECT_BOX1 && oType <= OBJECT_BOX20) ||
					 (oType >= OBJECT_KEY1 && oType <= OBJECT_KEY5 ) )
				{
					pType = (ParticuleType)(PARTITRACK1+rand()%4);
				}
				else
				{
					pType = PARTITRACK1;  // orange
				}
				m_particule->CreateTrack(pos, speed, dim, pType,
										 duration, mass, Rand()+0.7f, 1.0f);
			}
		}
		if ( m_type == PT_EXPLOP )
		{
			total = 10;
			for ( i=0 ; i<total ; i++ )
			{
				pos = m_posTracks;
				speed.x = (Rand()-0.5f)*60.0f;
				speed.z = (Rand()-0.5f)*60.0f;
				speed.y = Rand()*30.0f;
				dim.x = 0.5f;
				dim.y = dim.x;
				duration = Rand()*0.2f+0.2f;
				mass = Rand()*5.0f+5.0f;
				m_particule->CreateTrack(pos, speed, dim, PARTITRACK1,
										 duration, mass, 10.0f);
			}
		}
	}

	if ( m_type == PT_GOAL )
	{
		pos = m_posStart;
		pos.y += 8.0f;
		speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
		dim.x = 4.0f;
		dim.y = dim.x;
		m_particule->CreateParticule(pos, speed, dim, PARTILENS2, 0.5f);
	}

	// Génère la grosse sphère translucide.
	if ( m_bSphere )  // sphère ?
	{
		if ( m_bTracks )
		{
			pos = m_posTracks;
		}
		else
		{
			pos = m_pos;
			m_terrain->MoveOnFloor(pos);
			pos.y += 1.0f;
		}
		dim.x = m_size*0.4f;
		dim.y = dim.x;
		m_particule->CreateParticule(pos, D3DVECTOR(0.0f,0.0f,0.0f), dim,
									 PARTISPHERE0,2.0f, 0.0f);
	}

	if ( m_type == PT_FRAGO  ||
		 m_type == PT_EXPLOO )
	{
		pos = m_pos;
		pos.y += 5.0f;
		speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
		dim.x = 8.0f;
		dim.y = dim.x;
		duration = 0.3f;
		m_particule->CreateParticule(pos, speed, dim, PARTIBIGT, duration);

		total = 20;
		for ( i=0 ; i<total ; i++ )
		{
			pos = m_pos;
			pos.y += 2.0f;
			speed.x = (Rand()-0.5f)*30.0f;
			speed.z = (Rand()-0.5f)*30.0f;
			speed.y = 15.0f+Rand()*15.0f;
			dim.x = 2.0f;
			dim.y = dim.x;
			duration = Rand()*1.0f+1.0f;
			mass = Rand()*30.0f+40.0f;
			m_particule->CreateParticule(pos, speed, dim, PARTIORGANIC1,
										 duration, mass);
		}
		total = 20;
		for ( i=0 ; i<total ; i++ )
		{
			pos = m_pos;
			pos.y += 2.0f;
			speed.x = (Rand()-0.5f)*30.0f;
			speed.z = (Rand()-0.5f)*30.0f;
			speed.y = 15.0f+Rand()*15.0f;
			dim.x = 1.0f;
			dim.y = dim.x;
			duration = Rand()*1.0f+1.0f;
			mass = Rand()*30.0f+40.0f;
			m_particule->CreateTrack(pos, speed, dim, PARTITRACK4,
									 duration, mass, duration*0.5f, dim.x*2.0f);
		}
	}

	if ( m_bChoc )  // onde de choc circulaire et horizontale ?
	{
		pos = m_pos;
//?		m_terrain->MoveOnFloor(pos);
//?		pos.y += 2.0f;
		speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
		dim.x = m_size;
		dim.y = dim.x;
		m_particule->CreateParticule(pos, speed, dim, PARTICHOC, 2.0f);
	}

	return TRUE;
}

// Crée une explosion sous forme de particules triangulaires.

void CPyro::CreateTriangle(CObject* pObj, ObjectType oType, int part)
{
	D3DTriangle		buffer[100];
	D3DMATRIX*		mat;
	D3DVECTOR		pos, offset, p, speed;
	float			percent, min, max, duration, mass;
	int				objRank, total, i, j;

	objRank = pObj->RetObjectRank(part);
	if ( objRank == -1 )  return;

	min = 0.0f;
	max = 1000000.0f;
	if ( (oType >= OBJECT_BOX1 && oType <= OBJECT_BOX6) ||
		 oType == OBJECT_BOX9 )
	{
		max = m_engine->RetLimitLOD(0);
	}

	total = m_engine->RetTotalTriangles(objRank);
	percent = 0.10f;
	if ( total < 50 )  percent = 0.25f;
	if ( total < 20 )  percent = 0.50f;
	if ( oType == OBJECT_BARREL   ||
		 oType == OBJECT_BARRELa  ||
		 oType == OBJECT_ATOMIC   ||
		 oType == OBJECT_URANIUM  ||
		 oType == OBJECT_TNT      ||
		 oType == OBJECT_MINE     )  percent = 0.50f;
	if ( oType == OBJECT_MOBILEtg )  percent = 0.50f;
	if ( oType == OBJECT_MARK     )  percent = 0.75f;
	if ( oType == OBJECT_FIOLE    )  percent = 0.75f;
	if ( oType == OBJECT_GLU      )  percent = 0.75f;
	if ( oType == OBJECT_GLASS1   )  percent = 1.00f;
	if ( oType == OBJECT_GLASS2   )  percent = 1.00f;
	if ( oType == OBJECT_TRAX     )  percent = 0.50f;
	if ( oType == OBJECT_PERFO    )  percent = 0.50f;
	if ( oType == OBJECT_GUN      )  percent = 0.50f;
	if ( oType >= OBJECT_BOX1 && oType <= OBJECT_BOX7  )  percent = 1.00f;
	if ( oType >= OBJECT_BOX9 && oType <= OBJECT_BOX10 )  percent = 1.00f;
	if ( oType >= OBJECT_KEY1 && oType <= OBJECT_KEY5  )  percent = 1.00f;
	total = m_engine->GetTriangles(objRank, min, max, buffer, 100, percent);

	mat = pObj->RetWorldMatrix(part);

	for ( i=0 ; i<total ; i++ )
	{
		pos = Transform(*mat, D3DVECTOR(0.0f, 0.0f, 0.0f));

		offset = D3DVECTOR(0.0f, 0.0f, 0.0f);
		for ( j=0 ; j<3 ; j++ )
		{
			p.x = buffer[i].triangle[j].x;
			p.y = buffer[i].triangle[j].y;
			p.z = buffer[i].triangle[j].z;
			p = Transform(*mat, p);
			p -= pos;
			buffer[i].triangle[j].x = p.x;
			buffer[i].triangle[j].y = p.y;
			buffer[i].triangle[j].z = p.z;
			offset += p;
		}
		offset /= 3.0f;

		for ( j=0 ; j<3 ; j++ )
		{
			p.x = buffer[i].triangle[j].x;
			p.y = buffer[i].triangle[j].y;
			p.z = buffer[i].triangle[j].z;
			p -= offset;
			buffer[i].triangle[j].x = p.x;
			buffer[i].triangle[j].y = p.y;
			buffer[i].triangle[j].z = p.z;
		}
		pos += offset;

		buffer[i].state |= D3DSTATE2FACE;

		if ( oType == OBJECT_FIOLE  ||
			 oType == OBJECT_GLU    ||
			 oType == OBJECT_GLASS1 ||
			 oType == OBJECT_GLASS2 ||
			 oType == OBJECT_TRAX   ||
			 oType == OBJECT_PERFO  ||
			 oType == OBJECT_GUN    ||
			 oType == OBJECT_BOX7   ||  // sphère ?
			 oType == OBJECT_BOX8   )   // colonne ?
		{
			speed.x = (Rand()-0.5f)*8.0f*m_force;
			speed.z = (Rand()-0.5f)*8.0f*m_force;
			speed.y = Rand()*8.0f*m_force;
			mass = Rand()*10.0f+15.0f;
		}
		else
		{
			speed.x = (Rand()-0.5f)*30.0f*m_force;
			speed.z = (Rand()-0.5f)*30.0f*m_force;
			speed.y = Rand()*30.0f*m_force;
			mass = Rand()*10.0f+15.0f;
		}
		if ( oType == OBJECT_GLASS1 ||
			 oType == OBJECT_GLASS2 )
		{
			pos.y += 20.0f;  // car vitre caché dessous
		}
		if ( oType == OBJECT_STONE   )  speed *= 0.5f;
		if ( oType == OBJECT_URANIUM )  speed *= 0.4f;
		if ( oType >= OBJECT_CARCASS1 && oType <= OBJECT_CARCASS10 )  speed *= 0.1f;
		duration = Rand()*3.0f+3.0f;
		m_particule->CreateFrag(pos, speed, &buffer[i], PARTIFRAG,
								duration, mass);
	}
}

// Affiche l'erreur ou l'information éventuelle, liée à la destruction
// d'un insecte, d'un véhicule ou d'un batiment.

void CPyro::DisplayError(PyroType type, CObject* pObj)
{
}


// Gestion d'un événement.

BOOL CPyro::EventProcess(const Event &event)
{
	ParticuleType	type;
	D3DVECTOR		pos, speed, angle;
	FPOINT			dim;
	float			factor;
	int				r;

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
		 m_lastParticule+0.05f <= m_time )
	{
		m_lastParticule = m_time;

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

	if ( (m_type == PT_FRAGT  ||
		  m_type == PT_EXPLOT ||
		  m_type == PT_EXPLOS ) &&
		 m_progress < 0.05f &&
		 m_lastParticule+0.05f <= m_time )
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

	if ( (m_type == PT_FRAGT  ||
		  m_type == PT_EXPLOT ||
		  m_type == PT_EXPLOS ) &&
		 m_progress < 0.10f &&
		 m_lastParticuleSmoke+0.10f <= m_time )
	{
		m_lastParticuleSmoke = m_time;

//?		dim.x = Rand()*m_size/3.0f+m_size/3.0f;
		dim.x = Rand()*m_size/6.0f+m_size/6.0f;
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
		 m_lastParticule+0.05f <= m_time )
	{
		m_lastParticule = m_time;

		pos = m_pos;
		speed.x = (Rand()-0.5f)*m_size*1.0f;
		speed.z = (Rand()-0.5f)*m_size*1.0f;
		speed.y = Rand()*m_size*0.5f;
		speed *= 1.0f-m_progress;
		dim.x = (Rand()*m_size/2.0f+m_size/2.0f)*(1.0f-m_progress);
		dim.y = dim.x;
//?		m_particule->CreateParticule(pos, speed, dim, PARTIEXPLOO);
	}
	if ( m_type == PT_EXPLOT ||
		 m_type == PT_BREAKT )
	{
		ExploProgress();
	}
	if ( m_type == PT_EXPLOO )
	{
		OrgaProgress();
	}
	if ( m_type == PT_PLOUF )
	{
		PloufProgress();
	}

	if ( m_type == PT_GOAL )
	{
		if ( m_object != 0 )
		{
			pos = m_posStart;
			pos.y += powf(m_progress*7.0f, 1.5f)*200.0f;
			m_object->SetPosition(0, pos);
		}
	}

	if ( (m_type == PT_BURNT || m_type == PT_BURNO || m_type == PT_BURNS) &&
		 m_object != 0 )
	{
		if ( m_lastParticule+0.05f <= m_time )
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
			m_particule->CreateParticule(pos, speed, dim, PARTIFLAME, 2.0f, 0.0f);

			pos = m_object->RetPosition(0);
			pos.y -= m_object->RetCharacter()->height;
			pos.x += (Rand()-0.5f)*(2.0f+4.0f*m_progress)*factor;
			pos.z += (Rand()-0.5f)*(2.0f+4.0f*m_progress)*factor;
			speed.x = 0.0f;
			speed.z = 0.0f;
			speed.y = (Rand()*5.0f*m_progress+3.0f)*factor;
			dim.x = (Rand()*2.0f+1.0f)*factor;
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTIFLAME, 2.0f, 0.0f);

			pos = m_object->RetPosition(0);
			pos.y -= 2.0f;
			pos.x += (Rand()-0.5f)*5.0f*factor;
			pos.z += (Rand()-0.5f)*5.0f*factor;
			speed.x = 0.0f;
			speed.z = 0.0f;
			speed.y = (6.0f+Rand()*6.0f+m_progress*6.0f)*factor;
			dim.x = (Rand()*1.5f+1.0f+m_progress*3.0f)*factor;
			if ( m_type == PT_BURNO )  dim.x *= 2.0f;
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTISMOKE3, 4.0f);

			if ( m_type == PT_BURNO )
			{
				pos = m_object->RetPosition(0);
				pos.y += 4.0f*(1.0f-m_progress);
				speed.x = (Rand()-0.5f)*20.0f;
				speed.z = (Rand()-0.5f)*20.0f;
				speed.y = 5.0f+Rand()*10.0f;
				dim.x = 0.8f*(1.0f-m_progress);
				dim.y = dim.x;
				m_particule->CreateParticule(pos, speed, dim, PARTIBLITZ, 1.0f, 40.0f);

				if ( Rand() < 0.05f )  // assez rare ?
				{
					speed.x = (Rand()-0.5f)*30.0f;
					speed.z = (Rand()-0.5f)*30.0f;
					speed.y = Rand()*30.0f+15.0f;
					dim.x = Rand()*1.0f+1.0f;
					dim.y = dim.x;
					m_particule->CreateTrack(pos, speed, dim, PARTITRACK4, 2.0f, 50.0f, 1.2f, 1.8f);
				}
			}
		}

		if ( m_type == PT_BURNT )
		{
			BurnProgress();
		}
		if ( m_type == PT_BURNO )
		{
			speed.y = 0.0f;
			speed.x = (Rand()-0.5f)*m_progress*0.5f;
			speed.z = (Rand()-0.5f)*m_progress*0.5f;
			m_object->SetLinVibration(speed);
			speed.y = (Rand()-0.5f)*m_progress*2.0f;
			speed.x = (Rand()-0.5f)*m_progress*0.5f;
			speed.z = (Rand()-0.5f)*m_progress*0.5f;
			m_object->SetCirVibration(speed);
			factor = 1.0f-powf(m_progress, 2.0f);
			if ( factor < 0.1f )  factor = 0.1f;
			m_object->SetZoom(0, factor);
		}
	}

	if ( m_lightRank != -1 )
	{
		LightOperFrame(event.rTime);
	}

	return TRUE;
}

// Indique que l'objet lié à l'effet n'existe plus, sans le détruire.

void CPyro::CutObjectLink(CObject* pObj)
{
	if ( m_object == pObj )
	{
		m_object = 0;
	}
}

// Indique si l'effet pyrotechnique utilise un objet.

BOOL CPyro::IsObject(CObject* pObj)
{
	return (pObj == m_object);
}


// Indique si l'effet pyrotechnique est terminé.

Error CPyro::IsEnded()
{
	// Détruit l'objet qui explose. Il ne doit pas être détruit à la fin
	// du Create, car c'est parfois l'objet lui-même qui fait le Create :
	//	pyro->Create(PT_FRAGT, this);
	if ( m_type == PT_FRAGT  ||
		 m_type == PT_FRAGO  ||
		 m_type == PT_FRAGG  )
	{
		DeleteObject(TRUE, TRUE);
	}

	if ( m_type == PT_GOAL  &&
		 m_progress >= 0.3f &&
		 m_object != 0      )
	{
		DeleteObject(TRUE, TRUE);
	}

	// Fin de l'effet pyrotechnique ?
	if ( m_progress < 1.0f )  return ERR_CONTINUE;

	if ( m_type == PT_EXPLOT ||  // explosion ?
		 m_type == PT_BREAKT ||
		 m_type == PT_GLASS  )
	{
		ExploTerminate();
	}

	if ( m_type == PT_EXPLOO )  // explosion ?
	{
		OrgaTerminate();
	}

	if ( m_type == PT_PLOUF )  // plouf dans l'eau ?
	{
		PloufTerminate();
	}

	if ( m_type == PT_BURNT ||
		 m_type == PT_BURNO )  // brûle ?
	{
		BurnTerminate();
	}

	if ( m_lightRank != -1 )
	{
		m_light->DeleteLight(m_lightRank);
		m_lightRank = -1;
	}

	return ERR_STOP;
}

// Supprime l'objet lié à l'effet pyrotechnique.

void CPyro::DeleteObject(BOOL bPrimary, BOOL bSecondary)
{
	CObject		*sub, *truck;
	D3DVECTOR	pos;
	ObjectType	type;

	if ( m_object == 0 )  return;

	type = m_object->RetType();
	if ( bSecondary )
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
			sub->DeleteObject();  // supprime l'objet transporté
			delete sub;
			m_object->SetFret(0);
		}
	}

	if ( bPrimary )
	{
		truck = m_object->RetTruck();
		if ( truck != 0 )  // objet porté ?
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

// (*)	CObject::DeleteObject peut remettre à zéro m_object
//		par le biais de CPyro::CutObjectLink !


// Vide la table des opérations d'animation de la lumière.

void CPyro::LightOperFlush()
{
	m_lightOperTotal = 0;
}

// Ajoute une opération d'animation de la lumière.

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

// Fait évoluer la lumière associée.

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


// Crée la lumière pour accompagner un effet pyrotechnique.

BOOL CPyro::CreateLight(D3DVECTOR pos, float height)
{
	D3DLIGHT7	light;

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

	// N'éclaire que les objets du terrain.
	m_light->SetLightIncluType(m_lightRank, TYPETERRAIN);

	return TRUE;
}


// Démarre l'explosion d'une vitre.

void CPyro::GlassStart(ObjectType oType)
{
	D3DVECTOR	pos, angle, speed, min, max;
	FPOINT		dim;
	float		weight;
	int			i, objRank, channel;

	m_burnType = m_object->RetType();

	pos = m_object->RetPosition(0);
	m_burnFall = m_terrain->RetFloorHeight(pos);

	m_object->SetLock(TRUE);  // ruine pas encore utilisable
	m_object->SetExplo(TRUE);  // en cours de destruction
	m_object->SetNoUndoable(TRUE);  // undo impossible pendant la casse
	m_object->FlatParent();

	for ( i=1 ; i<OBJECTMAXPART ; i++ )
	{
		objRank = m_object->RetObjectRank(i);
		if ( objRank == -1 )  continue;

		pos = m_object->RetPosition(i);

		m_engine->GetBBox(objRank, min, max);
		weight = Length(min, max);  // poids selon dimensions !

		speed.y = 5.0f+Rand()*10.0f;
		speed.x = (Rand()-0.5f)*5.0f;
		speed.z = (Rand()-0.5f)*5.0f;

		dim.x = weight/2.0f;
		dim.y = dim.x;

		channel = m_particule->CreatePart(pos, speed, dim, PARTIPART, 10.0f, 20.0f, weight);
		if ( channel != -1 )
		{
			m_object->SetMasterParticule(i, channel);
		}
	}

	pos = m_object->RetPosition(0);
	pos.y -= 20.0f;
	m_object->SetPosition(0, pos);  // cache la vitre dessous
}

// Démarre l'explosion d'un véhicule.

void CPyro::ExploStart(ObjectType oType)
{
	D3DVECTOR	pos, angle, speed, min, max;
	FPOINT		dim;
	float		weight;
	int			i, objRank, channel;

	m_burnType = m_object->RetType();

	pos = m_object->RetPosition(0);
	m_burnFall = m_terrain->RetFloorHeight(pos);

//?	m_object->Simplify();
	m_object->SetLock(TRUE);  // ruine pas encore utilisable
	m_object->SetExplo(TRUE);  // en cours de destruction
	m_object->FlatParent();

	if ( m_object->RetSelect() )
	{
		m_object->SetSelect(FALSE);  // désélectionne l'objet
		m_main->DeselectAll();
	}

	for ( i=0 ; i<OBJECTMAXPART ; i++ )
	{
		objRank = m_object->RetObjectRank(i);
		if ( objRank == -1 )  continue;

		pos = m_object->RetPosition(i);

		m_engine->GetBBox(objRank, min, max);
		weight = Length(min, max);  // poids selon dimensions !

		if ( m_type == PT_EXPLOT )
		{
			speed.y = 10.0f+Rand()*20.0f;
			speed.x = (Rand()-0.5f)*20.0f;
			speed.z = (Rand()-0.5f)*20.0f;
		}
		else	// PT_BREAKT ?
		{
			speed.y = 5.0f+Rand()*10.0f;
			speed.x = (Rand()-0.5f)*5.0f;
			speed.z = (Rand()-0.5f)*5.0f;
		}

		dim.x = weight/2.0f;
		dim.y = dim.x;

		channel = m_particule->CreatePart(pos, speed, dim, PARTIPART, 10.0f, 20.0f, weight);
		if ( channel != -1 )
		{
			m_object->SetMasterParticule(i, channel);
		}
	}

	DeleteObject(FALSE, TRUE);  // détruit l'objet transporté + la pile
}

// Progression de l'explosion d'un véhicule.

void CPyro::ExploProgress()
{
	if ( m_object == 0 )  return;

	if ( m_object->RetType() == OBJECT_CRAZY &&
		 m_object->RetEnable() &&
		 m_time > 3.0f )
	{
		m_object->SetEnable(FALSE);  // pour CRobotMain::CheckEndMission()
	}
}

// Termine l'explosion d'un véhicule.

void CPyro::ExploTerminate()
{
	DeleteObject(TRUE, FALSE);  // supprime l'objet principal
}


// Démarre la chute dans l'eau d'un objet.

void CPyro::PloufStart()
{
	D3DVECTOR	pos, speed;
	FPOINT		dim;
	float		force, duration;
	int			i;

	pos = m_posStart;
	pos.y = m_posStart.y-1.0f;
	dim.x = 8.0f;  // hauteur
	dim.y = 4.0f;  // diamètre
	m_particule->CreateParticule(pos, D3DVECTOR(0.0f, 0.0f, 0.0f), dim, PARTIPLOUF0, 1.4f, 0.0f);

	for ( i=0 ; i<2 ; i++ )
	{
		pos = m_posStart;
		pos.x += (Rand()-0.5f)*8.0f;
		pos.z += (Rand()-0.5f)*8.0f;
		pos.y = m_posStart.y-1.0f;
		force = 1.0f+Rand()*1.0f;
		dim.x = 1.5f*force;  // hauteur
		dim.y = 0.8f*force;  // diamètre
		duration = 0.8f+Rand()*1.0f;
		m_particule->CreateParticule(pos, D3DVECTOR(0.0f, 0.0f, 0.0f), dim, PARTIPLOUF0, duration, 0.0f);

		pos = m_posStart;
		pos.x += (Rand()-0.5f)*8.0f;
		pos.z += (Rand()-0.5f)*8.0f;
		pos.y = m_posStart.y+1.0f;
		dim.x = 4.0f+Rand()*4.0f;
		dim.y = dim.x;
		duration = 2.0f+Rand()*2.0f;
		m_particule->CreateParticule(pos, D3DVECTOR(0.0f, 0.0f, 0.0f), dim, PARTIFLIC, duration, 0.0f);
	}

	for ( i=0 ; i<20 ; i++ )
	{
		pos = m_posStart;
		pos.x += (Rand()-0.5f)*4.0f;
		pos.z += (Rand()-0.5f)*4.0f;
		pos.y = m_posStart.y+1.0f;
		speed.x = (Rand()-0.5f)*6.0f;
		speed.y = Rand()*10.0f+20.0f;
		speed.z = (Rand()-0.5f)*6.0f;
		dim.x = 0.5f+Rand()*0.5f;
		dim.y = dim.x;
		duration = 4.0f;
		m_particule->CreateParticule(pos, speed, dim, PARTIDROP, duration, 30.0f);
	}

	m_object->SetLock(TRUE);  // ruine pas encore utilisable
	m_object->SetExplo(TRUE);  // en cours de destruction
	m_object->SetNoUndoable(TRUE);  // undo impossible pendant la chute

	DeleteObject(FALSE, TRUE);  // détruit l'objet transporté + la pile
}

// Progression de la chute dans l'eau d'un objet.

void CPyro::PloufProgress()
{
	D3DVECTOR	pos;

	if ( m_object == 0 )  return;

	pos = m_posStart;
	pos.y -= m_progress*12.0f;
	m_object->SetPosition(0, pos);
}

// Termine la chute dans l'eau d'un objet.

void CPyro::PloufTerminate()
{
	DeleteObject(TRUE, FALSE);  // supprime l'objet principal
}


// Démarre l'explosion d'un objet organique vert.

void CPyro::OrgaStart()
{
	m_object->SetLock(TRUE);  // plus utilisable
	m_object->SetExplo(TRUE);  // en cours de destruction
}

// Fait progresser l'explosion d'un objet organique vert.

void CPyro::OrgaProgress()
{
#if 0
	D3DVECTOR	zoom;
	float		progress;

	m_object->RetCharacter()->height = 0.5f;

	if ( m_progress < 0.1f )
	{
		progress = Norm(m_progress/0.1f);  // 0..1
		zoom.y = 1.0f-progress*0.7f;
		if ( zoom.y < 0.01f )  zoom.y = 0.01f;
		zoom.x = 1.0f+progress*0.5f;
		zoom.z = zoom.x;
	}
	else if ( m_progress < 0.7f )
	{
		progress = Norm((m_progress-0.1f)/0.6f);  // 0..1
		zoom.y = 0.3f;
		zoom.x = 1.5f;
		zoom.z = zoom.x;
	}
	else
	{
		progress = Norm((m_progress-0.7f)/0.3f);  // 0..1
		zoom.y = 0.3f;
		zoom.x = 1.5f-progress*1.5f;
		zoom.z = zoom.x;
	}
	m_object->SetZoom(0, zoom);
#else
	float	zoom;
	float	progress;

	if ( m_progress < 0.1f )
	{
		progress = m_progress/0.1f;
		zoom = 1.0f-progress;
	}
	else
	{
		zoom = 0.0f;
	}
	m_object->SetZoom(0, zoom);
#endif
}

// Termine l'explosion d'un objet organique vert.

void CPyro::OrgaTerminate()
{
	DeleteObject(TRUE, TRUE);
}


// Démarre le feu d'un véhicule.

void CPyro::BurnStart()
{
	D3DVECTOR	pos, angle;
	int			i, objRank;

	m_burnType = m_object->RetType();

	pos = m_object->RetPosition(0);
	m_burnFall = m_terrain->RetFloorHeight(pos);

	m_object->Simplify();
	m_object->SetLock(TRUE);  // ruine pas encore utilisable
	m_object->SetExplo(TRUE);  // en cours de destruction

	if ( m_object->RetSelect() )
	{
		m_object->SetSelect(FALSE);  // désélectionne l'objet
		m_main->DeselectAll();
	}

	for ( i=0 ; i<OBJECTMAXPART ; i++ )
	{
		objRank = m_object->RetObjectRank(i);
		if ( objRank == -1 )  continue;
		m_engine->ChangeSecondTexture(objRank, "dirty04.tga");
	}
	m_engine->LoadTexture("dirty04.tga", 1);

	m_burnPartTotal = 0;

	pos.x =   0.0f;
	pos.y = -(2.0f+Rand()*2.0f);
	pos.z =   0.0f;
	angle.x = (Rand()-0.5f)*0.8f;
	angle.y = 0.0f;
	angle.z = (Rand()-0.5f)*0.8f;
	BurnAddPart(0, pos, angle);  // mouvement de la partie principale

	m_burnKeepPart[0] = -1;  // rien à garder

	if ( m_burnType == OBJECT_MOBILEfb )
	{
		pos.x = -1.5f;
		pos.y = -5.0f;
		pos.z =  0.0f;
		angle.x = (Rand()-0.5f)*0.2f;
		angle.y = (Rand()-0.5f)*0.2f;
		angle.z = -25.0f*PI/180.0f;
		BurnAddPart(1, pos, angle);  // descend le canon
	}

	if ( m_burnType == OBJECT_MOBILEob )
	{
		pos.x = -1.5f;
		pos.y = -5.0f;
		pos.z =  0.0f;
		angle.x = (Rand()-0.5f)*0.2f;
		angle.y = (Rand()-0.5f)*0.2f;
		angle.z = -25.0f*PI/180.0f;
		BurnAddPart(1, pos, angle);  // descend le canon-insecte
	}

	if ( m_burnType == OBJECT_MOBILEtg )  // roues ?
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

	if ( m_burnType == OBJECT_MOBILEfb ||
		 m_burnType == OBJECT_MOBILEob )  // chenilles ?
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
}

// Ajoute une partie à bouger.

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

// Fait progresser le feu d'un véhicule.

void CPyro::BurnProgress()
{
	D3DVECTOR	pos;
	float		h;
	int			i;

//?	m_object->SetZoomY(0, 1.0f-m_progress*0.2f);  // léger applatissement

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
}

// Indique si une partie doit être conservée.

BOOL CPyro::BurnIsKeepPart(int part)
{
	int		i;

	i = 0;
	while ( m_burnKeepPart[i] != -1 )
	{
		if ( part == m_burnKeepPart[i++] )  return TRUE;  // faut garder
	}
	return FALSE;  // faut détruire
}

// Termine le feu d'un insecte ou d'un véhicule.

void CPyro::BurnTerminate()
{
	int			i, objRank;

	if ( m_type == PT_BURNO )  // brûle objet organique ?
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

	DeleteObject(FALSE, TRUE);  // détruit l'objet transporté + la pile

	if ( m_burnType == OBJECT_START    ||
		 m_burnType == OBJECT_END      )
	{
		m_object->SetType(OBJECT_RUINfactory);  // ça devient une ruine
		m_object->SetLock(FALSE);
	}
	else
	{
		m_object->SetType(OBJECT_RUINmobilew1);  // ça devient une ruine
		m_object->SetLock(FALSE);
	}

	m_object->SetBurn(FALSE);  // ruine utilisable (c-à-d. récupérable)
}


