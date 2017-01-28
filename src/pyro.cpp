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
#include "motionhuman.h"
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
	D3DMATRIX*		mat;
	CMotion*		motion;
	D3DVECTOR		min, max, pos, speed, p1, p2, p3, p4;
	FPOINT			dim;
	ObjectType		oType;
	ParticuleType	pType;
	Sound			sound;
	float			duration, mass, h, limit;
	int				part, objRank, total, i, channel;

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

	DisplayError(type, pObj);  // affiche message éventuel

	// Copie toutes les sphères de l'objet.
	for ( i=0 ; i<50 ; i++ )
	{
		if ( !pObj->GetCrashSphere(i, m_crashSpherePos[i], m_crashSphereRadius[i]) )  break;
	}
	m_crashSphereUsed = i;

	// Calcule la dimension de l'effet.
	m_size = Length(min, max)*2.0f;
	if ( m_size <  4.0f )  m_size =  4.0f;
	if ( m_size > 80.0f )  m_size = 80.0f;

	if ( oType == OBJECT_TNT  ||
		 oType == OBJECT_MINE ||
		 oType == OBJECT_BOMB )
	{
		m_size *= 2.0f;
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
		m_terrain->MoveOnFloor(m_pos);
		m_pos.y += 0.2f;
		m_speed = 1.0f/5.0f;
	}
	if ( oType == OBJECT_BARREL   ||
		 oType == OBJECT_BARRELa  ||
		 oType == OBJECT_ATOMIC   ||
		 oType == OBJECT_URANIUM  ||
		 oType == OBJECT_TNT      ||
		 oType == OBJECT_MINE     ||
		 oType == OBJECT_BOMB     )
	{
		m_bTracks = TRUE;
		m_posTracks = m_pos;
		m_posTracks.y += 1.0f;
		m_pos = m_posTracks;
	}
	if ( oType >= OBJECT_BOX1  &&
		 oType <= OBJECT_BOX10 )
	{
		m_bTracks = TRUE;
		m_posTracks = m_pos;
		m_posTracks.y += 1.0f;
		m_pos = m_posTracks;
	}
	if ( oType == OBJECT_NUCLEAR )
	{
		m_bTracks = TRUE;
	}
	if ( oType == OBJECT_PARA )
	{
		m_bTracks = TRUE;
	}
	if ( oType == OBJECT_TRAX )
	{
		m_bTracks = TRUE;
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
		 type == PT_FRAGW  ||
		 type == PT_EXPLOT ||
		 type == PT_EXPLOS ||
		 type == PT_EXPLOW )
	{
		if ( m_size > 10.0f || m_bTracks )
		{
			m_bChoc = TRUE;
		}
		if ( oType >= OBJECT_BOX1 && oType <= OBJECT_BOX10 )
		{
			m_bChoc = FALSE;
		}
	}

	// Fait entendre le bruit de l'effet pyrotechnique.
	if ( type == PT_FRAGT  ||
		 type == PT_FRAGA  ||
		 type == PT_FRAGW  ||
		 type == PT_EXPLOT ||
		 type == PT_EXPLOS ||
		 type == PT_EXPLOW )
	{
		m_main->IncDecorStamp();  // un objet sera détruit

		if ( m_bTracks )
		{
			sound = SOUND_EXPLOp;
		}
		else
		{
			sound = SOUND_EXPLO;
		}
		if ( oType == OBJECT_STONE   ||
			 oType == OBJECT_METAL   ||
			 oType == OBJECT_BBOX    ||
			 oType == OBJECT_KEYa    ||
			 oType == OBJECT_KEYb    ||
			 oType == OBJECT_KEYc    ||
			 oType == OBJECT_KEYd    ||
			 (oType >= OBJECT_ROADSIGN1 && oType <= OBJECT_ROADSIGN30) )
		{
			sound = SOUND_EXPLOl;
		}
		if ( oType == OBJECT_URANIUM  ||
			 oType == OBJECT_BARREL   ||
			 oType == OBJECT_BARRELa  ||
			 oType == OBJECT_ATOMIC   ||
			 oType == OBJECT_TNT      ||
			 oType == OBJECT_MINE     ||
			 oType == OBJECT_BOMB     ||
			 (oType >= OBJECT_BOX1 && oType <= OBJECT_BOX10) )
		{
			sound = SOUND_EXPLOlp;
		}
		m_sound->Play(sound, m_pos);
	}
	if ( type == PT_EXPLOO )
	{
		m_sound->Play(SOUND_EXPLOi, m_pos);
	}
	if ( type == PT_FRAGO )
	{
		m_sound->Play(SOUND_EXPLOlp, m_pos);
	}
	if ( type == PT_EJECT )
	{
		if ( oType >= OBJECT_CARCASS1 && oType <= OBJECT_CARCASS10 )
		{
			m_sound->Play(SOUND_BOUMm, m_pos);
		}
	}
	if ( type == PT_BURNT ||
		 type == PT_BURNO ||
		 type == PT_BURNS )
	{
		m_soundChannel = m_sound->Play(SOUND_BURN, m_pos, 1.0f, 1.0f, TRUE);
		m_sound->AddEnvelope(m_soundChannel, 1.0f, 1.0f, 12.0f, SOPER_CONTINUE);
		m_sound->AddEnvelope(m_soundChannel, 0.0f, 1.0f,  5.0f, SOPER_STOP);
	}
	if ( type == PT_BURNO )
	{
		m_sound->Play(SOUND_EVIL1, m_pos, 1.0f, 0.5f);
	}
	if ( type == PT_FLCREATE ||
		 type == PT_FLDELETE )
	{
		m_sound->Play(SOUND_WAYPOINT, m_pos);
	}

	if ( type == PT_FRAGT  ||
		 type == PT_FRAGW  ||
		 type == PT_EXPLOT ||
		 type == PT_EXPLOS ||
		 type == PT_EXPLOW )
	{
		// Crée une tache au sol.
		p1 = p2 = p3 = p4 = m_pos;
		p1.x -= 8.0f;  p1.z += 8.0f;
		p2.x += 8.0f;  p2.z += 8.0f;
		p3.x -= 8.0f;  p3.z -= 8.0f;
		p4.x += 8.0f;  p4.z -= 8.0f;
		m_particule->CreateWheelTrace(p1, p2, p3, p4, PARTITRACE3);
	}
	if ( type == PT_FRAGA )
	{
		// Crée une tache au sol.
		p1 = p2 = p3 = p4 = m_pos;
		p1.x -= 24.0f;  p1.z += 24.0f;
		p2.x += 24.0f;  p2.z += 24.0f;
		p3.x -= 24.0f;  p3.z -= 24.0f;
		p4.x += 24.0f;  p4.z -= 24.0f;
		m_particule->CreateWheelTrace(p1, p2, p3, p4, PARTITRACE5);
	}
	if ( type == PT_EXPLOO )
	{
		// Crée une tache au sol.
		p1 = p2 = p3 = p4 = m_pos;
		p1.x -= 24.0f;  p1.z += 24.0f;
		p2.x += 24.0f;  p2.z += 24.0f;
		p3.x -= 24.0f;  p3.z -= 24.0f;
		p4.x += 24.0f;  p4.z -= 24.0f;
		m_particule->CreateWheelTrace(p1, p2, p3, p4, PARTITRACE8);
	}

	if ( m_type == PT_FRAGT ||
		 m_type == PT_FRAGA ||
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
	if ( m_type == PT_EXPLOO )
	{
		m_camera->StartOver(OE_ORGA, m_pos, 1.0f);
	}

	if ( m_type == PT_SHOTW )
	{
		m_speed = 1.0f/1.0f;
	}

	if ( m_type == PT_BURNT ||
		 m_type == PT_BURNO )
	{
		BurnStart();
	}

	if ( m_type == PT_WPCHECK )
	{
		m_speed = 1.0f/4.0f;
		m_object->SetEnable(FALSE);  // objet plus fonctionnel
	}
	if ( m_type == PT_WPVIBRA )
	{
		m_speed = 1.0f/4.0f;
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
	if ( m_type == PT_FINDING )
	{
		limit = (m_size-1.0f)/4.0f;
		if ( limit > 8.0f )  limit = 8.0f;
		m_speed = 1.0f/limit;
	}
	if ( m_type == PT_PAINTING )
	{
		limit = (m_size-1.0f)/8.0f;
		if ( limit > 4.0f )  limit = 4.0f;
		m_speed = 1.0f/limit;
	}

	if ( m_type == PT_EXPLOT ||
		 m_type == PT_EXPLOW ||
		 m_type == PT_EJECT  )
	{
		CreateTriangle(pObj, oType, 0);
		m_engine->ShadowDelete(m_object->RetObjectRank(0));
		ExploStart(oType);
	}

	if ( m_type == PT_EXPLOO )
	{
		m_engine->ShadowDelete(m_object->RetObjectRank(0));
		OrgaStart();
	}

	if ( m_type == PT_ACROBATIC ||
		 m_type == PT_SABOTAGE  ||
		 m_type == PT_PIECE     )
	{
		AcrobaticStart(min, max);
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

	if ( m_type == PT_SPIDER )
	{
		m_speed = 1.0f/15.0f;

		pos = D3DVECTOR(-3.0f, 2.0f, 0.0f);
		mat = pObj->RetWorldMatrix(0);
		m_pos = Transform(*mat, pos);

		m_engine->ShadowDelete(m_object->RetObjectRank(0));
	}

	// Génère les effets de lumière.
	if ( m_type != PT_EGG       &&
		 m_type != PT_WIN       &&
		 m_type != PT_LOST      &&
		 m_type != PT_EJECT     &&
		 m_type != PT_EXPLOP    &&
		 m_type != PT_SABOTAGE  &&
		 m_type != PT_ACROBATIC &&
		 m_type != PT_PIECE     )
	{
		h = 40.0f;
		if ( m_type == PT_FRAGO  ||
			 m_type == PT_EXPLOO )
		{
			LightOperAdd(0.00f, 0.0f,  4.0f,  4.0f,  4.0f);  // blanc
			LightOperAdd(0.05f, 1.0f, -1.0f,  4.0f, -1.0f);  // vert clair
			LightOperAdd(1.00f, 0.0f, -1.0f,  4.0f, -1.0f);  // vert clair
			h = m_size*10.0f;
		}
		else if ( m_type == PT_FRAGT  ||
				  m_type == PT_EXPLOT ||
				  m_type == PT_EXPLOS )
		{
			LightOperAdd(0.00f, 1.0f,  4.0f,  4.0f,  2.0f);  // jaune
			LightOperAdd(0.02f, 1.0f,  4.0f,  2.0f,  0.0f);  // rouge-orange
			LightOperAdd(0.16f, 1.0f, -0.4f, -0.4f, -0.4f);  // gris foncé
			LightOperAdd(1.00f, 0.0f, -0.4f, -0.4f, -0.4f);  // gris foncé
			h = m_size*5.0f;
		}
		else if ( m_type == PT_FRAGA )
		{
			LightOperAdd(0.00f, 1.0f, -0.4f, -0.4f, -0.4f);  // gris foncé
			LightOperAdd(0.05f, 1.0f, -2.0f,  4.0f,  4.0f);  // bleu
			LightOperAdd(1.00f, 0.0f, -2.0f,  4.0f,  4.0f);  // bleu
			h = m_size*10.0f;
		}
		else if ( m_type == PT_SPIDER )
		{
			LightOperAdd(0.00f, 0.0f, -0.5f, -1.0f, -1.0f);  // rouge foncé
			LightOperAdd(0.05f, 1.0f, -0.5f, -1.0f, -1.0f);  // rouge foncé
			LightOperAdd(1.00f, 0.0f, -0.5f, -1.0f, -1.0f);  // rouge foncé
		}
		else if ( m_type == PT_FRAGW  ||
				  m_type == PT_EXPLOW ||
				  m_type == PT_SHOTW  )
		{
			LightOperAdd(0.00f, 0.0f, -0.5f, -0.5f, -1.0f);  // jaune foncé
			LightOperAdd(0.05f, 1.0f, -0.5f, -0.5f, -1.0f);  // jaune foncé
			LightOperAdd(1.00f, 0.0f, -0.5f, -0.5f, -1.0f);  // jaune foncé
		}
		else if ( m_type == PT_WPCHECK  ||
				  m_type == PT_WPVIBRA  ||
				  m_type == PT_FLCREATE ||
				  m_type == PT_FLDELETE ||
				  m_type == PT_FINDING  ||
				  m_type == PT_PAINTING )
		{
			LightOperAdd(0.00f, 1.0f,  4.0f,  4.0f,  2.0f);  // jaune
			LightOperAdd(1.00f, 0.0f,  4.0f,  4.0f,  2.0f);  // jaune
		}
		else
		{
			LightOperAdd(0.00f, 0.0f, -0.4f, -0.4f, -0.4f);  // gris foncé
			LightOperAdd(0.05f, 1.0f, -0.4f, -0.4f, -0.4f);  // gris foncé
			LightOperAdd(1.00f, 0.0f, -0.4f, -0.4f, -0.4f);  // gris foncé
		}
		CreateLight(m_pos, h);

		if ( m_type != PT_SHOTW    &&
			 m_type != PT_WPCHECK  &&
			 m_type != PT_WPVIBRA  &&
			 m_type != PT_FLCREATE &&
			 m_type != PT_FLDELETE &&
			 m_type != PT_FINDING  &&
			 m_type != PT_PAINTING )
		{
			m_camera->StartEffect(CE_EXPLO, m_pos, force);
		}
	}

	if ( m_type == PT_SHOTW )  return TRUE;
	
	// Génère les triangles de l'explosion.
	if ( m_type == PT_FRAGT  ||
		 m_type == PT_FRAGA  ||
		 m_type == PT_FRAGO  ||
		 m_type == PT_FRAGW  ||
		 m_type == PT_SPIDER ||
		 m_type == PT_EGG    )
	{
		for ( part=0 ; part<OBJECTMAXPART ; part++ )
		{
			CreateTriangle(pObj, oType, part);
		}
	}

	// Génère les particules avec traînées.
	if ( m_bTracks )
	{
		if ( m_type == PT_FRAGT  ||
			 m_type == PT_FRAGA  ||
			 m_type == PT_EXPLOT ||
			 m_type == PT_EXPLOS )
		{
			total = (int)(10.0f*m_engine->RetParticuleDensity());
			if ( oType == OBJECT_TNT  ||
				 oType == OBJECT_MINE ||
				 oType == OBJECT_BOMB ||
				 oType == OBJECT_TRAX )  total *= 3;
			for ( i=0 ; i<total ; i++ )
			{
				pos = m_posTracks;
				speed.x = (Rand()-0.5f)*30.0f;
				speed.z = (Rand()-0.5f)*30.0f;
				speed.y = Rand()*30.0f;
				dim.x = 1.0f;
				dim.y = dim.x;
				duration = Rand()*3.0f+2.0f;
				mass = Rand()*10.0f+15.0f;
				if ( oType >= OBJECT_BOX1 && oType <= OBJECT_BOX10 )
				{
					pType = (ParticuleType)(PARTITRACK1+rand()%4);
				}
				else if ( m_type == PT_FRAGA )
				{
					pType = PARTITRACK2;  // bleu
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
			total = (int)(10.0f*m_engine->RetParticuleDensity());
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
									 (m_type==PT_FRAGA)?PARTISPHERE3:PARTISPHERE0,
									 2.0f, 0.0f);
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
		m_particule->CreateParticule(pos, speed, dim, PARTIBIGO, duration);

		total = (int)(20.0f*m_engine->RetParticuleDensity());
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
		total = (int)(20.0f*m_engine->RetParticuleDensity());
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
	D3DVECTOR		offset, pos, speed;
	float			percent, min, max, h, duration, mass;
	int				objRank, total, i;

	if ( oType == OBJECT_CONE )  return;

	objRank = pObj->RetObjectRank(part);
	if ( objRank == -1 )  return;

	min = 0.0f;
//?	max = m_engine->RetLimitLOD(0);
	max = 1000000.0f;
	total = m_engine->RetTotalTriangles(objRank);
	percent = 0.10f;
	if ( total < 50 )  percent = 0.25f;
	if ( total < 20 )  percent = 0.50f;
	if ( m_type == PT_EGG )  percent = 0.30f;
	if ( oType == OBJECT_BARREL   ||
		 oType == OBJECT_BARRELa  ||
		 oType == OBJECT_ATOMIC   ||
		 oType == OBJECT_URANIUM  ||
		 oType == OBJECT_TNT      ||
		 oType == OBJECT_MINE     ||
		 oType == OBJECT_BOMB     )  percent = 0.75f;
	if ( oType == OBJECT_MOBILEtg )  percent = 0.50f;
	if ( oType == OBJECT_MARK     )  percent = 0.75f;
	if ( oType >= OBJECT_BOX1 && oType <= OBJECT_BOX10 )  percent = 1.00f;
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
			dim.x = (Rand()*m_crashSphereRadius[i]*0.5f+m_crashSphereRadius[i]*0.75f*m_force)*5.0f;
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
	}

	if ( m_type == PT_SHOTM &&
		 m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
	{
		m_lastParticule = m_time;
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
			m_particule->CreateParticule(pos, speed, dim, PARTIBLITZ, 0.5f, 0.0f);
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
			m_particule->CreateParticule(pos, speed, dim, PARTIBLITZ, 0.5f, 0.0f);
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

	if ( (m_type == PT_FRAGT  ||
		  m_type == PT_EXPLOT ||
		  m_type == PT_EXPLOS ) &&
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

	if ( m_type == PT_FRAGA &&
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

		m_particule->CreateParticule(pos, speed, dim, PARTIEXPLOA);
	}

	if ( (m_type == PT_FRAGT  ||
		  m_type == PT_EXPLOT ||
		  m_type == PT_EXPLOS ) &&
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

	if ( m_type == PT_FRAGA &&
		 m_progress < 0.10f &&
		 m_lastParticuleSmoke+m_engine->ParticuleAdapt(0.10f) <= m_time )
	{
		m_lastParticuleSmoke = m_time;

		dim.x = Rand()*m_size/1.5f+m_size/1.5f;
		dim.y = dim.x;
		pos = m_pos;
		pos.x += (Rand()-0.5f)*m_size*2.0f;
		pos.z += (Rand()-0.5f)*m_size*2.0f;
		m_terrain->MoveOnFloor(pos);
		speed.x = 0.0f;
		speed.z = 0.0f;
		speed.y = -dim.x/2.0f/4.0f;
		pos.y += dim.x/2.0f;
		m_particule->CreateParticule(pos, speed, dim, PARTIEJECT, 6.0f);
	}

	if ( (m_type == PT_FRAGO || m_type == PT_EXPLOO) &&
		 m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
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
	if ( m_type == PT_EXPLOO )
	{
		OrgaProgress();
	}

	if ( m_type == PT_ACROBATIC ||
		 m_type == PT_SABOTAGE  ||
		 m_type == PT_PIECE     )
	{
		AcrobaticProgress();
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
		m_particule->CreateParticule(pos, speed, dim, PARTIBLITZ, 0.5f, 0.0f);
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
			pos.x += (Rand()-0.5f)*9.0f;
			pos.z += (Rand()-0.5f)*9.0f;
			speed.x = 0.0f;
			speed.z = 0.0f;
			speed.y = 5.0f+Rand()*5.0f;
			dim.x = Rand()*2.0f+2.0f;
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTIGLINT, 2.0f);
		}

		angle = m_object->RetCirVibration();
		angle.y = m_progress*20.0f;
		angle.x = sinf(m_progress*49.0f)*0.3f;
		angle.z = sinf(m_progress*47.0f)*0.2f;
		m_object->SetCirVibration(angle);

		pos = m_pos;
		pos.y += factor;
		m_object->SetPosition(0, pos);

		if ( m_progress > 0.85f )
		{
			m_object->SetZoom(0, 1.0f-(m_progress-0.85f)/0.15f);
		}
	}

	if ( m_type == PT_WPVIBRA )
	{
		if ( m_progress < 0.85f &&
			 m_lastParticule+m_engine->ParticuleAdapt(0.10f) <= m_time )
		{
			m_lastParticule = m_time;

			pos = m_pos;
			pos.x += (Rand()-0.5f)*9.0f;
			pos.z += (Rand()-0.5f)*9.0f;
			speed.x = 0.0f;
			speed.z = 0.0f;
			speed.y = 5.0f+Rand()*5.0f;
			dim.x = Rand()*2.0f+2.0f;
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTIGLINT, 2.0f);
		}

		angle = m_object->RetCirVibration();
		angle.y = m_progress*20.0f;
		angle.x = sinf(m_progress*49.0f)*0.3f;
		angle.z = sinf(m_progress*47.0f)*0.2f;
		angle *= (1.0f-m_progress);
		m_object->SetCirVibration(angle);
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
			m_particule->CreateParticule(pos, speed, dim, PARTIGLINT, 2.0f, 0.0f);
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
			m_particule->CreateParticule(pos, speed, dim, PARTIGLINT, 2.0f, 0.0f);
		}

		angle = m_object->RetAngle(0);
		angle.y = m_progress*20.0f;
		angle.x = sinf(m_progress*49.0f)*0.3f;
		angle.z = sinf(m_progress*47.0f)*0.2f;
		m_object->SetAngle(0, angle);

		m_object->SetZoom(0, 1.0f-m_progress);
	}

	if ( m_type == PT_FINDING )
	{
		if ( m_object != 0 &&
			 m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
		{
			m_lastParticule = m_time;

			factor = m_size*0.3f;
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
			m_particule->CreateParticule(pos, speed, dim, PARTIGLINT, 2.0f, 0.0f);
		}
	}

	if ( m_type == PT_PAINTING )
	{
		if ( m_object != 0 &&
			 m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
		{
			m_lastParticule = m_time;

			factor = m_size*0.3f;
			if ( factor > 40.0f )  factor = 40.0f;
			pos = m_pos;
			m_terrain->MoveOnFloor(pos);
			pos.x += (Rand()-0.5f)*factor;
			pos.z += (Rand()-0.5f)*factor;
			pos.y += 2.0f;
			speed.x = (Rand()-0.5f)*1.0f;
			speed.z = (Rand()-0.5f)*1.0f;
			speed.y = 2.0f+Rand()*2.0f;
			dim.x = (Rand()*0.2f+0.2f)*(1.0f-m_progress*0.9f);
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTIGLINT, 2.0f, 0.0f);
		}
	}

	if ( (m_type == PT_BURNT || m_type == PT_BURNO || m_type == PT_BURNS) &&
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

			if ( m_object->RetType() == OBJECT_EVIL3 )  // roi ?
			{
				m_object->SetZoom(1, 1.0f/factor);
			}
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
			m_particule->CreateParticule(pos, speed, dim, PARTISMOKE1, 8.0f, 0.0f);
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

// Indique si l'effet pyrotechnique est terminé.

Error CPyro::IsEnded()
{
	// Détruit l'objet qui explose. Il ne doit pas être détruit à la fin
	// du Create, car c'est parfois l'objet lui-même qui fait le Create :
	//	pyro->Create(PT_FRAGT, this);
	if ( m_type == PT_FRAGT  ||
		 m_type == PT_FRAGA  ||
		 m_type == PT_FRAGO  ||
		 m_type == PT_FRAGW  ||
		 m_type == PT_SPIDER ||
		 m_type == PT_EGG    )
	{
		DeleteObject(TRUE, TRUE);
	}

	if ( m_type == PT_WIN  ||
		 m_type == PT_LOST )
	{
		return ERR_CONTINUE;
	}

	// Fin de l'effet pyrotechnique ?
	if ( m_progress < 1.0f )  return ERR_CONTINUE;

	if ( m_type == PT_EXPLOT ||
		 m_type == PT_EXPLOW ||
		 m_type == PT_EJECT  )  // explosion ?
	{
		ExploTerminate();
	}

	if ( m_type == PT_EXPLOO )  // explosion ?
	{
		OrgaTerminate();
	}

	if ( m_type == PT_ACROBATIC ||
		 m_type == PT_SABOTAGE  ||
		 m_type == PT_PIECE     )
	{
		AcrobaticTerminate();
	}

	if ( m_type == PT_BURNT ||
		 m_type == PT_BURNO )  // brûle ?
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

	// N'éclaire que les objets du terrain.
	m_light->SetLightIncluType(m_lightRank, TYPETERRAIN);

	return TRUE;
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
	m_burnFall = m_terrain->RetFloorHeight(pos, TRUE);

	m_object->Simplify();
	m_object->SetLock(TRUE);  // ruine pas encore utilisable
	m_object->SetExplo(TRUE);  // en cours de destruction
	m_object->FlatParent();

	if ( m_object->RetSelect() )
	{
		m_object->SetSelect(FALSE);  // désélectionne l'objet
		m_camera->SetType(CAMERA_EXPLO);
		m_main->DeselectAll();
	}

	for ( i=0 ; i<OBJECTMAXPART ; i++ )
	{
		objRank = m_object->RetObjectRank(i);
		if ( objRank == -1 )  continue;

		if ( m_type != PT_EJECT )
		{
			m_engine->ChangeSecondTexture(objRank, "dirty04.tga");
		}

		pos = m_object->RetPosition(i);

		m_engine->GetBBox(objRank, min, max);
		weight = Length(min, max);  // poids selon dimensions !

		speed.y = 10.0f+Rand()*20.0f;
		speed.x = (Rand()-0.5f)*20.0f;
		speed.z = (Rand()-0.5f)*20.0f;

		dim.x = weight/2.0f;
		dim.y = dim.x;

		if ( m_type == PT_EJECT )
		{
			pos.y += 3.0f;
			speed.y *= 0.8f;

			if ( oType >= OBJECT_CARCASS1 && oType <= OBJECT_CARCASS10 )
			{
				pos.y += 3.0f;
				speed.y *= 1.5f;
			}
		}

		channel = m_particule->CreatePart(pos, speed, dim, PARTIPART, 10.0f, 20.0f, weight);
		if ( channel != -1 )
		{
			m_object->SetMasterParticule(i, channel);
		}
	}

	if ( m_type != PT_EJECT )
	{
		m_engine->LoadTexture("dirty04.tga", 1);
	}

	DeleteObject(FALSE, TRUE);  // détruit l'objet transporté + la pile
}

// Termine l'explosion d'un véhicule.

void CPyro::ExploTerminate()
{
	DeleteObject(TRUE, FALSE);  // supprime l'objet principal
}


// Démarre l'explosion d'un objet organique vert.

void CPyro::OrgaStart()
{
	m_object->SetLock(TRUE);  // plus utilisable
	m_object->SetExplo(TRUE);  // en cours de destruction

	if ( m_object->RetType() == m_main->RetTypeProgress() )
	{
		m_main->IncProgress();
	}

}

// Fait progresser l'explosion d'un objet organique vert.

void CPyro::OrgaProgress()
{
#if 1
	D3DVECTOR	zoom;
	float		progress;

	if ( m_progress < 0.1f )
	{
		progress = m_progress/0.1f;
		zoom.y = 1.0f-progress;
		if ( zoom.y < 0.01f )  zoom.y = 0.01f;
		zoom.x = 1.0f+progress*1.0f;
		zoom.z = zoom.x;
	}
	else if ( m_progress < 0.7f )
	{
		progress = (m_progress-0.1f)/0.6f;
		zoom.y = 0.01f;
		zoom.x = 2.0f;
		zoom.z = zoom.x;
	}
	else
	{
		progress = (m_progress-0.7f)/0.3f;
		zoom.y = 0.01f;
		zoom.x = 2.0f-progress*2.0f;
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


// Démarre la voltige d'un robot ou d'un objet heurté.

void CPyro::AcrobaticStart(D3DVECTOR min, D3DVECTOR max)
{
	ObjectType	type;
	D3DMATRIX*	mat;
	D3DVECTOR	cdg, pos, n, angle, speed, size;
	FPOINT		nn, dim;
	float		a, duration, little, radius, d;

	m_angleStart.x = NormAngle(m_angleStart.x);
	m_angleStart.y = NormAngle(m_angleStart.y);
	m_angleStart.z = NormAngle(m_angleStart.z);

	m_angleGoal = m_angleStart;

	m_height = m_force*10.0f;
	if ( m_height > 16.0f )  m_height = 16.0f;

	if ( m_height < 6.0f )
	{
		m_speed = 1.0f/1.0f;
		m_height = 0.0f;
	}
	else
	{
		m_speed = 1.0f/(1.0f+(m_height/16.0f));
	}

	m_heightSuppl = 0.0f;

	m_object->SetLock(TRUE);  // plus utilisable
	m_object->SetExplo(TRUE);  // en cours de destruction

	cdg = (min+max)/2.0f;  // centre de gravité
	size = max-min;  // dimensions

	d = 1.2f-Norm(Length(size)*0.2f)*0.4f;
	m_height *= d;
	m_speed *= 1.0f/d;

	type = m_object->RetType();
	if ( m_type == PT_PIECE )  // pièce de voiture ?
	{
		m_heightSuppl = -m_terrain->RetFloorHeight(m_posStart);

		little = Min(size.x, size.y, size.z);
		if ( m_param == 1 )  // force rotation en x ?
		{
			little = size.z;
		}
		if ( size.x == little )  // pièce mince en x ?
		{
			angle = D3DVECTOR(0.0f, 0.0f, -PI/2.0f);
		}
		else if ( size.z == little )  // pièce mince en z ?
		{
			angle = D3DVECTOR(-PI/2.0f, 0.0f, 0.0f);
		}
		else	// pièce mince en y ?
		{
			angle = D3DVECTOR(0.0f, 0.0f, 0.0f);
		}
		AcrobaticTransform(type, cdg, angle, min, max);

		m_angleGoal.y += (Rand()-0.5f)*PI*2.0f;  // autre orientation

		if ( m_terrain->GetNormal(n, m_posStart) )
		{
			a = m_angleGoal.y;
			nn = RotatePoint(-a, FPOINT(n.z, n.x));
			m_angleGoal.x =  sinf(nn.x);
			m_angleGoal.z = -sinf(nn.y);  // plaque au sol
		}
	}
	else
	{
		if ( type != OBJECT_BOT1    &&
			 type != OBJECT_BOT2    &&
			 type != OBJECT_BOT3    &&
			 type != OBJECT_BOT4    &&
			 type != OBJECT_BOT5    &&
			 type != OBJECT_CARROT  &&
			 type != OBJECT_STARTER &&
			 type != OBJECT_WALKER  &&
			 type != OBJECT_CRAZY   &&
			 type != OBJECT_GUIDE   )
		{
			if ( !m_object->RetGround() )  // pas encore à terre ?
			{
				angle = D3DVECTOR(-PI/2.0f, 0.0f, 0.0f);
				AcrobaticTransform(type, cdg, angle, min, max);
			}
			m_angleGoal.y += (Rand()-0.5f)*PI*2.0f;  // autre orientation

			pos = m_object->RetPosition(0);
			if ( m_terrain->GetNormal(n, pos) )
			{
				a = m_angleGoal.y;
				nn = RotatePoint(-a, FPOINT(n.z, n.x));
				m_angleGoal.x =  sinf(nn.x);
				m_angleGoal.z = -sinf(nn.y);  // plaque au sol
				m_angleGoal.x = m_angleStart.x+Direction(m_angleStart.x, m_angleGoal.x);
				m_angleGoal.z = m_angleStart.z+Direction(m_angleStart.z, m_angleGoal.z);
			}
		}
	}

	if ( m_height == 0.0f )
	{
		m_angleGoal.y += PI*2.0f;  // pirouette
	}
	else if ( m_height < 11.0f )
	{
		m_angleGoal.y += PI*2.0f;
		m_angleGoal.z += PI*2.0f;  // pirouette
	}
	else
	{
		m_angleGoal.x += PI*2.0f;
		m_angleGoal.y += PI*2.0f;
		m_angleGoal.z += PI*2.0f;  // pirouette
	}

	radius = Length(size)/2.0f;
	dim.x = Length(max-min)*0.7f*(m_height/5.0f);
	if ( radius > 3.0f && dim.x > 4.0f )
	{
		if ( dim.x > 12.0f )  dim.x = 12.0f;
		dim.y = dim.x;
		mat = m_object->RetWorldMatrix(0);
		pos = Transform(*mat, cdg);
		speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
		duration = 0.3f;
		m_particule->CreateParticule(pos, speed, dim, PARTIBIGT, duration);
	}
}

// Transforme un objet pour qu'il devienne indépendant.

void CPyro::AcrobaticTransform(ObjectType type, D3DVECTOR cdg, D3DVECTOR angle,
							   D3DVECTOR min, D3DVECTOR max)
{
	D3DVECTOR	move1, move2;
	int			i, objRank;

	if ( angle.z != 0.0f )
	{
		move1 = -cdg;
		move2 = D3DVECTOR(0.0f, 0.0f, 0.0f);
		AcrobaticGroundAdapt(type, angle, move2);

		for ( i=0 ; i<OBJECTMAXPART ; i++ )
		{
			objRank = m_object->RetObjectRank(i);
			if ( objRank == -1 )  continue;
			m_engine->TransformObject(objRank, move1, angle, move2);
		}

		m_posStart.y += -min.x;
	}
	else if ( angle.x != 0.0f )
	{
		move1 = -cdg;
		move2 = D3DVECTOR(0.0f, 0.0f, 0.0f);
		AcrobaticGroundAdapt(type, angle, move2);

		for ( i=0 ; i<OBJECTMAXPART ; i++ )
		{
			objRank = m_object->RetObjectRank(i);
			if ( objRank == -1 )  continue;
			m_engine->TransformObject(objRank, move1, angle, move2);
		}

		m_posStart.y += -min.z;
	}
	else
	{
		m_posStart.y += -min.y;
	}
}

// Adapte l'angle d'un objet pour qu'il repose au sol sur le flan.

void CPyro::AcrobaticGroundAdapt(ObjectType type, D3DVECTOR &angle, D3DVECTOR &pos)
{
	if ( type == OBJECT_CONE )
	{
		angle.x -= 17.0f*PI/180.0f;
		m_posStart.y -= 0.4f;
	}
	if ( type == OBJECT_BARRIER4 )
	{
		angle.z -= 6.0f*PI/180.0f;
		angle.x -= 3.0f*PI/180.0f;
		m_posStart.y -= 0.5f;
	}
	if ( type == OBJECT_BARRIER5 )
	{
		angle.x -= 3.0f*PI/180.0f;
		m_posStart.y -= 0.1f;
	}
	if ( type == OBJECT_CARCASS1 )
	{
		angle.x -= 20.0f*PI/180.0f;  // sur le côté
	}
	if ( type == OBJECT_CARCASS2 )
	{
		angle.x += 160.0f*PI/180.0f;  // sur le toît
		m_posStart.y += 1.0f;
	}
}

// Fait progresser la voltige d'un robor heurté.

void CPyro::AcrobaticProgress()
{
	D3DVECTOR	pos, angle;
	float		progress;

	progress = Norm(m_progress);

	pos = m_posStart;
	pos.y += sinf(progress*PI)*m_height;

	if ( progress > 0.5f )
	{
		pos.y += m_heightSuppl*((progress-0.5f)/0.5f);
	}

	if ( m_type == PT_SABOTAGE )
	{
		pos.x += (m_impact.x-pos.x)*progress;
		pos.z += (m_impact.z-pos.z)*progress;
	}
	m_object->SetPosition(0, pos);

//?	angle = m_angleStart+(m_angleGoal-m_angleStart)*progress;
	angle = m_angleStart+(m_angleGoal-m_angleStart)*powf(progress, 0.5f);
//?	angle.x += PI*2.0f*powf(progress, 0.5f);  // pirouette
//?	angle.y += PI*2.0f*powf(progress, 0.5f);  // pirouette
//?	angle.z += PI*2.0f*powf(progress, 0.5f);  // pirouette
	m_object->SetAngle(0, angle);
}

// Termine la voltige d'un robor heurté.

void CPyro::AcrobaticTerminate()
{
	ObjectType	type;
	Sound		sound;

	m_progress = 1.0f;
	AcrobaticProgress();

	if ( m_height > 0.0f )
	{
		type = m_object->RetType();

		if ( type == OBJECT_CONE )
		{
			sound = SOUND_BOUMv;
		}
		else
		{
			sound = (Sound)(SOUND_FALLo1+rand()%2);
		}
		m_sound->Play(sound, m_posStart);
	}

	m_object->SetLock(FALSE);  // de nouveau utilisable
	m_object->SetExplo(FALSE);  // destruction terminée
	m_object->SetGround(TRUE);  // à terre
}


// Démarre le feu d'un véhicule.

void CPyro::BurnStart()
{
	D3DVECTOR	pos, angle;
	int			i, objRank;

	m_burnType = m_object->RetType();

	pos = m_object->RetPosition(0);
	m_burnFall = m_terrain->RetFloorHeight(pos, TRUE);

	m_object->Simplify();
	m_object->SetLock(TRUE);  // ruine pas encore utilisable
	m_object->SetExplo(TRUE);  // en cours de destruction

	if ( m_object->RetSelect() )
	{
		m_object->SetSelect(FALSE);  // désélectionne l'objet
		m_camera->SetType(CAMERA_EXPLO);
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

	if ( m_burnType == OBJECT_TOWER )
	{
		pos.x =   0.0f;
		pos.y = -(4.0f+Rand()*4.0f);
		pos.z =   0.0f;
		angle.x = (Rand()-0.5f)*0.4f;
		angle.y = 0.0f;
		angle.z = (Rand()-0.5f)*0.4f;
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
	else if ( m_burnType == OBJECT_CAR )
	{
		pos.x =   0.0f;
		pos.y = -(0.5f+Rand()*1.0f);
		pos.z =   0.0f;
		angle.x = (Rand()-0.5f)*0.8f;
		angle.y = 0.0f;
		angle.z = (Rand()-0.5f)*0.4f;
	}
	else if ( m_burnType == OBJECT_EVIL1 )
	{
		pos.x =   0.0f;
		pos.y =  -7.0f;
		pos.z =   0.0f;
		angle.x = (Rand()-0.5f)*0.8f;
		angle.y = 0.0f;
		angle.z = (Rand()-0.5f)*0.4f;
	}
	else if ( m_burnType == OBJECT_EVIL3 )
	{
		pos.x =   0.0f;
		pos.y = -20.0f;
		pos.z =   0.0f;
		angle.x = (Rand()-0.5f)*0.4f;
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

	m_burnKeepPart[0] = -1;  // rien à garder

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

	if ( m_burnType == OBJECT_CAR      ||
		 m_burnType == OBJECT_MOBILEtg )  // roues ?
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

	if ( m_burnType == OBJECT_EVIL1 ||
		 m_burnType == OBJECT_EVIL3 )
	{
		m_object->SetZoomY(0, 1.0f-m_progress);  // aplatissement complet
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

	if ( m_burnType == OBJECT_TOWER    ||
		 m_burnType == OBJECT_NUCLEAR  ||
		 m_burnType == OBJECT_PARA     ||
		 m_burnType == OBJECT_DOCK     ||
		 m_burnType == OBJECT_REMOTE   ||
		 m_burnType == OBJECT_STAND    ||
		 m_burnType == OBJECT_START    ||
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


