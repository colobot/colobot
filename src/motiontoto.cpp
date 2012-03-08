// motiontoto.cpp

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
#include "physics.h"
#include "brain.h"
#include "modfile.h"
#include "robotmain.h"
#include "sound.h"
#include "motion.h"
#include "motiontoto.h"



#define START_TIME		1000.0f		// début du temps relatif



// Constructeur de l'objet.

CMotionToto::CMotionToto(CInstanceManager* iMan, CObject* object)
						 : CMotion(iMan, object)
{
	CMotion::CMotion(iMan, object);

	m_time = 0.0f;
	m_bDisplayInfo = FALSE;
	m_bQuickPos = FALSE;
	m_bStartAction = FALSE;
	m_speedAction = 20.0f;
	m_soundChannel = -1;
	m_clownRadius = 0.0f;
	m_clownDelay  = 0.0f;
	m_clownTime   = 0.0f;
	m_blinkTime   = 0.0f;
	m_blinkProgress = -1.0f;
	m_lastMotorParticule = 0.0f;
	m_type = OBJECT_NULL;
	m_mousePos = FPOINT(0.0f, 0.0f);
}

// Destructeur de l'objet.

CMotionToto::~CMotionToto()
{
}


// Supprime un objet.

void CMotionToto::DeleteObject(BOOL bAll)
{
	if ( m_soundChannel != -1 )
	{
		m_sound->Stop(m_soundChannel);
		m_soundChannel = -1;
	}
}


// Crée un véhicule roulant quelconque posé sur le sol.

BOOL CMotionToto::Create(D3DVECTOR pos, float angle, ObjectType type,
						 float power)
{
	CModFile*		pModFile;
	int				rank;

	if ( m_engine->RetRestCreate() < 10 )  return FALSE;

	pModFile = new CModFile(m_iMan);

	m_object->SetType(type);

	// Crée la tête.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEVEHICULE);  // c'est un objet mobile
	m_object->SetObjectRank(0, rank);
	pModFile->ReadModel("objects\\toto1.mod");
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(0, pos);
	m_object->SetAngleY(0, angle);

	// Crée la bouche.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEDESCENDANT);
	m_object->SetObjectRank(1, rank);
	m_object->SetObjectParent(1, 0);
	pModFile->ReadModel("objects\\toto2.mod");
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(1, D3DVECTOR(1.00f, 0.17f, 0.00f));

	// Crée l'oeil gauche.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEDESCENDANT);
	m_object->SetObjectRank(2, rank);
	m_object->SetObjectParent(2, 0);
	pModFile->ReadModel("objects\\toto3.mod");
	pModFile->Mirror();
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(2, D3DVECTOR(0.85f, 1.04f, 0.25f));
	m_object->SetAngleY(2, -20.0f*PI/180.0f);

	// Crée l'oeil droite.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEDESCENDANT);
	m_object->SetObjectRank(3, rank);
	m_object->SetObjectParent(3, 0);
	pModFile->ReadModel("objects\\toto3.mod");
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(3, D3DVECTOR(0.85f, 1.04f, -0.25f));
	m_object->SetAngleY(3, 20.0f*PI/180.0f);

	// Crée l'antenne gauche.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEDESCENDANT);
	m_object->SetObjectRank(4, rank);
	m_object->SetObjectParent(4, 0);
	pModFile->ReadModel("objects\\toto4.mod");
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(4, D3DVECTOR(0.0f, 1.9f, 0.3f));
	m_object->SetAngleX(4, 30.0f*PI/180.0f);

	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEDESCENDANT);
	m_object->SetObjectRank(5, rank);
	m_object->SetObjectParent(5, 4);
	pModFile->ReadModel("objects\\toto4.mod");
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(5, D3DVECTOR(0.0f, 0.67f, 0.0f));
	m_object->SetAngleX(5, 30.0f*PI/180.0f);

	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEDESCENDANT);
	m_object->SetObjectRank(6, rank);
	m_object->SetObjectParent(6, 5);
	pModFile->ReadModel("objects\\toto5.mod");
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(6, D3DVECTOR(0.0f, 0.70f, 0.0f));
	m_object->SetAngleX(6, 30.0f*PI/180.0f);

	// Crée l'antenne droite.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEDESCENDANT);
	m_object->SetObjectRank(7, rank);
	m_object->SetObjectParent(7, 0);
	pModFile->ReadModel("objects\\toto4.mod");
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(7, D3DVECTOR(0.0f, 1.9f, -0.3f));
	m_object->SetAngleX(7, -30.0f*PI/180.0f);

	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEDESCENDANT);
	m_object->SetObjectRank(8, rank);
	m_object->SetObjectParent(8, 7);
	pModFile->ReadModel("objects\\toto4.mod");
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(8, D3DVECTOR(0.0f, 0.67f, 0.0f));
	m_object->SetAngleX(8, -30.0f*PI/180.0f);

	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEDESCENDANT);
	m_object->SetObjectRank(9, rank);
	m_object->SetObjectParent(9, 8);
	pModFile->ReadModel("objects\\toto5.mod");
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(9, D3DVECTOR(0.0f, 0.70f, 0.0f));
	m_object->SetAngleX(9, -30.0f*PI/180.0f);

	m_object->SetZoom(0, 0.5f);  // c'est p'tit
	m_object->SetFloorHeight(0.0f);

	pos = m_object->RetPosition(0);
	m_object->SetPosition(0, pos);  // pour afficher les ombres tout de suite

	m_engine->LoadAllTexture();

	delete pModFile;
	return TRUE;
}


// Début de l'affichage des informations, avec toto dans la marge gauche.

void CMotionToto::StartDisplayInfo()
{
return;
//?
	m_bDisplayInfo = TRUE;

	m_actionType = -1;
	m_actionTime = 0.0f;
	m_progress   = 0.0f;

	m_object->SetAngleY(0, 0.0f);
	m_mousePos = FPOINT(0.5f, 0.5f);
}

// Fin de l'affichage des informartions.

void CMotionToto::StopDisplayInfo()
{
	m_bDisplayInfo = FALSE;
	m_bQuickPos = TRUE;
}

// Donne la position de la souris.

void CMotionToto::SetMousePos(FPOINT pos)
{
	m_mousePos = pos;
}


// Gestion d'un événement.

BOOL CMotionToto::EventProcess(const Event &event)
{
	CMotion::EventProcess(event);

	if ( event.event == EVENT_FRAME )
	{
		return EventFrame(event);
	}

	return TRUE;
}

// Gestion d'un événement.

BOOL CMotionToto::EventFrame(const Event &event)
{
	D3DMATRIX*		mat;
	D3DVECTOR		eye, lookat, dir, perp, nPos, aPos, pos, speed;
	D3DVECTOR		vibLin, vibCir, dirSpeed, aAntenna;
	FPOINT			dim;
	POINT			wDim;
	ParticuleType	type;
	float			progress, focus, distance, shift, verti, level, zoom;
	float			aAngle, nAngle, mAngle, angle, linSpeed, cirSpeed;
	int				sheet, i, r;
	BOOL			bHidden;

	if ( m_engine->RetPause() &&
		 !m_main->RetInfoLock() )  return TRUE;

	if ( m_bDisplayInfo )  // "regarde" la souris ?
	{
		bHidden = FALSE;
	}
	else
	{
		bHidden = FALSE;

		if ( m_main->RetMovieLock() )  // film en cours ?
		{
			bHidden = TRUE;
		}
		if ( !m_engine->RetTotoMode() )
		{
			if ( !m_main->RetEditLock() )  // édition en cours ?
			{
				bHidden = TRUE;
			}
		}
	}

	if ( bHidden )
	{
		nPos = m_object->RetPosition(0);
		m_terrain->MoveOnFloor(nPos, TRUE);
		nPos.y -= 100.0f;  // cache sous le sol !
		m_object->SetPosition(0, nPos);
		return TRUE;
	}

	m_time += event.rTime;
	m_blinkTime -= event.rTime;

	progress = 0.0f;
	if ( m_actionType != -1 )  // action en cours ?
	{
		if ( m_progress < 0.15f )
		{
			progress = m_progress/0.15f;
		}
		else if ( m_progress < 0.85f )
		{
			progress = 1.0f;
		}
		else
		{
			progress = (1.0f-m_progress)/0.15f;
		}
	}

	if ( m_progress >= 1.0f )
	{
		m_actionType = -1;  // action terminée
		m_actionTime = 0.0f;
		m_progress   = 0.0f;

		m_clownTime  = 0.0f;
		m_clownDelay = 0.0f;
	}

	focus  = m_engine->RetFocus();
	eye    = m_engine->RetEyePt();
	lookat = m_engine->RetLookatPt();

	vibLin   = D3DVECTOR(0.0f, 0.0f, 0.0f);
	vibCir   = D3DVECTOR(0.0f, 0.0f, 0.0f);
	aAntenna = D3DVECTOR(0.0f, 0.0f, 0.0f);
	aAntenna.x += 30.0f*PI/180.0f;

	// Calcule la nouvelle position.
	if ( m_bDisplayInfo )
	{
		wDim = m_engine->RetDim();
		nPos.x = -4.0f*((float)wDim.x/(float)wDim.y)/(640.0f/480.0f);
		nPos.y = -0.5f;
		nPos.z =  7.0f;  // dans la marge gauche

		linSpeed = 0.0f;
	}
	else
	{
#if 0
		distance = 30.0f-progress*24.5f;  // éloignement
		shift    = 18.0f-progress*15.4f;  // décalage à gauche
		verti    = 10.0f-progress* 9.6f;  // décalage en haut
#else
		distance = 30.0f-progress*18.0f;  // éloignement
		shift    = 18.0f-progress*11.0f;  // décalage à gauche
		verti    = 10.0f-progress* 8.0f;  // décalage en haut
#endif

		if ( m_actionType == -1 &&
			 (m_type == OBJECT_HUMAN    ||
			  m_type == OBJECT_TECH     ||
			  m_type == OBJECT_MOBILEwa ||
			  m_type == OBJECT_MOBILEta ||
			  m_type == OBJECT_MOBILEfa ||
			  m_type == OBJECT_MOBILEia ||
			  m_type == OBJECT_MOBILEwc ||
			  m_type == OBJECT_MOBILEtc ||
			  m_type == OBJECT_MOBILEfc ||
			  m_type == OBJECT_MOBILEic ||
			  m_type == OBJECT_MOBILEwi ||
			  m_type == OBJECT_MOBILEti ||
			  m_type == OBJECT_MOBILEfi ||
			  m_type == OBJECT_MOBILEii ||
			  m_type == OBJECT_MOBILEws ||
			  m_type == OBJECT_MOBILEts ||
			  m_type == OBJECT_MOBILEfs ||
			  m_type == OBJECT_MOBILEis ||
			  m_type == OBJECT_MOBILErt ||
			  m_type == OBJECT_MOBILErc ||
			  m_type == OBJECT_MOBILErr ||
			  m_type == OBJECT_MOBILErs ||
			  m_type == OBJECT_MOBILEsa ||
			  m_type == OBJECT_MOBILEwt ||
			  m_type == OBJECT_MOBILEtt ||
			  m_type == OBJECT_MOBILEft ||
			  m_type == OBJECT_MOBILEit ||
			  m_type == OBJECT_MOBILEdr ) )  // véhicule ?
		{
			m_clownTime += event.rTime;
			if ( m_clownTime >= m_clownDelay )
			{
				if ( rand()%10 < 2 )
				{
					m_clownRadius = 2.0f+Rand()*10.0f;
//?					m_clownDelay  = m_clownRadius/(2.0f+Rand()*2.0f);
					m_clownDelay  = 1.5f+Rand()*1.0f;
				}
				else
				{
					m_clownRadius = 0.0f;
					m_clownDelay  = 2.0f+Rand()*2.0f;
				}
				pos = m_object->RetPosition(0);
				if ( pos.y < m_water->RetLevel() )  // sous l'eau ?
				{
					m_clownRadius /= 1.5f;
					m_clownDelay  *= 2.0f;
				}
				m_clownTime   = 0.0f;
			}
			else
			{
				distance -=               m_clownRadius*sinf(m_clownTime*PI*2.0f/m_clownDelay);
				shift    -= m_clownRadius-m_clownRadius*cosf(m_clownTime*PI*2.0f/m_clownDelay);
			}

			verti += (18.0f-shift)*0.2f;
		}

		distance /= focus;
//?		shift    *= focus;
		verti    /= focus;

		dir = Normalize(lookat-eye);
		nPos = eye + dir*distance;

		perp.x = -dir.z;
		perp.y =  dir.y;
		perp.z =  dir.x;
		nPos = nPos + perp*shift;

		nPos.y += verti;

		if ( m_bQuickPos )  // tout de suite en place ?
		{
			m_bQuickPos = FALSE;
			linSpeed = 0.0f;
		}
		else
		{
			aPos = m_object->RetPosition(0);
			if ( m_actionType == -1 )
			{
				level = 4.0f;
			}
			else
			{
				if ( m_bStartAction )
				{
					m_bStartAction = FALSE;
					m_speedAction = Length(nPos, aPos)/15.0f;
					if ( m_speedAction < 20.0f )  m_speedAction = 20.0f;
				}
				level = m_speedAction;
			}
			if ( level > 1.0f/event.rTime )  level = 1.0f/event.rTime;
			nPos = aPos + (nPos-aPos)*event.rTime*level;  // progression aPos -> nPos

			linSpeed = Length2d(nPos, aPos)/event.rTime;
			dirSpeed = (nPos-aPos)/event.rTime;
			nPos.y -= linSpeed*0.015f*(1.0f-progress);  // au raz du sol si avance vite
		}
	}

	// Calcule le nouvel angle.
	nAngle = NormAngle(RotateAngle(eye.x-lookat.x, lookat.z-eye.z)-0.9f);
	if ( linSpeed == 0.0f || m_actionType != -1 )
	{
		mAngle = nAngle;
	}
	else
	{
		mAngle = NormAngle(RotateAngle(dirSpeed.x, -dirSpeed.z));
	}
	level = Min(linSpeed*0.1f, 1.0f);
	nAngle = nAngle*(1.0f-level) + mAngle*level;
	aAngle = NormAngle(m_object->RetAngleY(0));

	if ( nAngle < aAngle )
	{
		if ( nAngle+PI*2.0f-aAngle < aAngle-nAngle )  nAngle += PI*2.0f;
	}
	else
	{
		if ( aAngle+PI*2.0f-nAngle < nAngle-aAngle )  aAngle += PI*2.0f;
	}
	nAngle = aAngle + (nAngle-aAngle)*event.rTime*4.0f;

	// Penche de côté si tourne.
	cirSpeed = (aAngle-nAngle)/event.rTime;
	angle = cirSpeed*0.3f*(1.0f-progress);
	if ( angle >  0.7f )  angle =  0.7f;
	if ( angle < -0.7f )  angle = -0.7f;
	vibCir.x += angle*1.5f;
	aAntenna.x += Abs(angle)*0.8f;  // écarte

	// Penche en avant si avance vite.
	angle = linSpeed*0.10f*(1.0f-progress);
	if ( angle > 1.0f )  angle = 1.0f;
	vibCir.z -= angle/2.0f;  // penche en avant
	aAntenna.z -= angle;  // penche en avant

	// Calcule le mouvement résiduel.
#if 1
	vibLin.y += (sinf(m_time*2.00f)*0.5f+
				 sinf(m_time*2.11f)*0.2f)*(1.0f-progress);

	vibCir.z += sinf(m_time*PI* 2.01f)*(PI/ 75.0f)+
				sinf(m_time*PI* 2.51f)*(PI/100.0f)+
				sinf(m_time*PI*19.01f)*(PI/200.0f);

	vibCir.x += sinf(m_time*PI* 2.03f)*(PI/ 75.0f)+
				sinf(m_time*PI* 2.52f)*(PI/100.0f)+
				sinf(m_time*PI*19.53f)*(PI/200.0f);

	vibCir.y += (sinf(m_time*PI* 1.07f)*(PI/ 10.0f)+
				 sinf(m_time*PI* 1.19f)*(PI/ 17.0f)+
				 sinf(m_time*PI* 1.57f)*(PI/ 31.0f))*(1.0f-progress);
#endif

	// Calcule les animations lors d'une action.
	if ( m_actionType == MT_ERROR )  // non-non ?
	{
		vibCir.y += progress*sinf(m_progress*PI*11.0f)*1.0f;
		vibCir.z -= progress*0.5f;  // penche en avant

		aAntenna.x -= progress*0.4f;  // resserre
		aAntenna.z += progress*1.0f;  // penche en arrière
	}

	if ( m_actionType == MT_WARNING )  // avertissement ?
	{
		vibCir.x += progress*sinf(m_progress*PI*17.0f)*0.5f;

		aAntenna.x += progress*sinf(m_progress*PI*17.0f)*0.5f;  // écarte
		aAntenna.z += progress*cosf(m_progress*PI*17.0f)*0.5f;  // tourne
	}

	if ( m_actionType == MT_INFO )  // oui-oui ?
	{
		vibCir.z += progress*sinf(m_progress*PI*19.0f)*0.7f;

		aAntenna.x -= progress*0.2f;  // resserre
		aAntenna.z -= progress*cosf(m_progress*PI*19.0f)*0.9f;  // tourne
	}

	if ( m_actionType == MT_MESSAGE )  // message ?
	{
		vibCir.x += progress*sinf(m_progress*PI*15.0f)*0.3f;
		vibCir.z += progress*cosf(m_progress*PI*15.0f)*0.3f;

		aAntenna.x -= progress*0.4f;  // resserre
		aAntenna.z -= progress*cosf(m_progress*PI*19.0f)*0.8f;
	}

	// Initialise l'objet.
	if ( m_bDisplayInfo )  // "regarde" la souris ?
	{
		if ( m_mousePos.x < 0.15f )
		{
			progress = 1.0f-m_mousePos.x/0.15f;
			vibCir.y += progress*PI/2.0f;
		}
		else
		{
			progress = (m_mousePos.x-0.15f)/0.85f;
			vibCir.y -= progress*PI/3.0f;
		}

		angle = RotateAngle(m_mousePos.x-0.1f, m_mousePos.y-0.5f-vibLin.y*0.2f);
		if ( angle < PI )
		{
			if ( angle > PI*0.5f )  angle = PI-angle;
			if ( angle > PI*0.3f )  angle = PI*0.3f;
			vibCir.z += angle;
		}
		else
		{
			angle = PI*2.0f-angle;
			if ( angle > PI*0.5f )  angle = PI-angle;
			if ( angle > PI*0.3f )  angle = PI*0.3f;
			vibCir.z -= angle;
		}
	}
	else
	{
		nPos.y += vibLin.y;
		level = m_terrain->RetFloorLevel(nPos);
		if ( nPos.y < level+2.0f )
		{
			nPos.y = level+2.0f;  // juste au-dessus du sol
		}
		nPos.y -= vibLin.y;
	}
	m_object->SetPosition(0, nPos);
	m_object->SetAngleY(0, nAngle);

	SetLinVibration(vibLin);
	SetCirVibration(vibCir);

	// Calcule le mouvement résiduel des antennes.
	pos = aAntenna*0.40f;
	pos.x += sinf(m_time*PI*2.07f)*(PI/50.0f)+
			 sinf(m_time*PI*2.59f)*(PI/70.0f)+
			 sinf(m_time*PI*2.67f)*(PI/90.0f);

	pos.y += sinf(m_time*PI*2.22f)*(PI/50.0f)+
			 sinf(m_time*PI*2.36f)*(PI/70.0f)+
			 sinf(m_time*PI*3.01f)*(PI/90.0f);

	pos.z += sinf(m_time*PI*2.11f)*(PI/50.0f)+
			 sinf(m_time*PI*2.83f)*(PI/70.0f)+
			 sinf(m_time*PI*3.09f)*(PI/90.0f);
	
	m_object->SetAngle(4, pos);  // antenne gauche
	m_object->SetAngle(5, pos);  // antenne gauche
	m_object->SetAngle(6, pos);  // antenne gauche

	pos = aAntenna*0.40f;
	pos.x = -pos.x;
	pos.x += sinf(m_time*PI*2.33f)*(PI/50.0f)+
			 sinf(m_time*PI*2.19f)*(PI/70.0f)+
			 sinf(m_time*PI*2.07f)*(PI/90.0f);

	pos.y += sinf(m_time*PI*2.44f)*(PI/50.0f)+
			 sinf(m_time*PI*2.77f)*(PI/70.0f)+
			 sinf(m_time*PI*3.22f)*(PI/90.0f);

	pos.z += sinf(m_time*PI*2.05f)*(PI/50.0f)+
			 sinf(m_time*PI*2.38f)*(PI/70.0f)+
			 sinf(m_time*PI*2.79f)*(PI/90.0f);
	
	m_object->SetAngle(7, pos);  // antenne droite
	m_object->SetAngle(8, pos);  // antenne droite
	m_object->SetAngle(9, pos);  // antenne droite

	// Mouvement de la bouche.
	if ( m_actionType == MT_ERROR )  // non-non ?
	{
		m_object->SetAngleX(1, 0.0f);
		m_object->SetAngleZ(1, 0.2f+sinf(m_time*10.0f)*0.2f);
		m_object->SetZoomY(1, 2.0f+sinf(m_time*10.0f));
		m_object->SetZoomZ(1, 1.0f);
	}
	else if ( m_actionType == MT_WARNING )  // avertissement ?
	{
		m_object->SetAngleX(1, 15.0f*PI/180.0f);
		m_object->SetAngleZ(1, 0.0f);
		m_object->SetZoomY(1, 1.0f);
		m_object->SetZoomZ(1, 1.0f);
	}
	else if ( m_actionType == MT_INFO )  // oui-oui ?
	{
		m_object->SetAngleX(1, 0.0f);
		m_object->SetAngleZ(1, 0.0f);
		m_object->SetZoomY(1, 1.0f);
		m_object->SetZoomZ(1, 0.7f+sinf(m_time*10.0f)*0.3f);
	}
	else if ( m_actionType == MT_MESSAGE )  // message ?
	{
		m_object->SetAngleX(1, 0.0f);
		m_object->SetAngleZ(1, 0.0f);
		m_object->SetZoomY(1, 1.0f);
		m_object->SetZoomZ(1, 0.8f+sinf(m_time*7.0f)*0.2f);
	}
	else
	{
		m_object->SetAngleX(1, 0.0f);
		m_object->SetAngleZ(1, 0.0f);
		m_object->SetZoomY(1, 1.0f);
		m_object->SetZoomZ(1, 1.0f);
	}

	// Gestion du clignement des yeux.
	if ( m_blinkTime <= 0.0f && m_blinkProgress == -1.0f )
	{
		m_blinkProgress = 0.0f;
	}

	if ( m_blinkProgress >= 0.0f )
	{
		m_blinkProgress += event.rTime*3.2f;

		if ( m_blinkProgress < 1.0f )
		{
			if ( m_blinkProgress < 0.5f )  zoom = m_blinkProgress/0.5f;
			else                           zoom = 2.0f-m_blinkProgress/0.5f;
			m_object->SetZoomY(2, 1.0f-zoom*0.9f);
			m_object->SetZoomY(3, 1.0f-zoom*0.9f);
		}
		else
		{
			m_blinkProgress = -1.0f;
			m_blinkTime = 0.1f+Rand()*4.0f;
			m_object->SetZoomY(2, 1.0f);
			m_object->SetZoomY(3, 1.0f);
		}
	}

	if ( m_actionType == MT_ERROR )  // non-non ?
	{
		m_object->SetAngleX(2, -30.0f*PI/180.0f);
		m_object->SetAngleX(3,  30.0f*PI/180.0f);
	}
	else if ( m_actionType == MT_WARNING )  // avertissement ?
	{
		m_object->SetAngleX(2, -15.0f*PI/180.0f);
		m_object->SetAngleX(3,  15.0f*PI/180.0f);
	}
	else if ( m_actionType == MT_INFO )  // oui-oui ?
	{
		m_object->SetAngleX(2,  40.0f*PI/180.0f);
		m_object->SetAngleX(3, -40.0f*PI/180.0f);
	}
	else if ( m_actionType == MT_MESSAGE )  // message ?
	{
		m_object->SetAngleX(2,  20.0f*PI/180.0f);
		m_object->SetAngleX(3, -20.0f*PI/180.0f);
	}
	else
	{
		m_object->SetAngleX(2, 0.0f);
		m_object->SetAngleX(3, 0.0f);
	}
	
	mat = m_object->RetWorldMatrix(0);  // doit être fait chaque fois !

	// Génère les particules.
	if ( m_time-m_lastMotorParticule >= m_engine->ParticuleAdapt(0.05f) )
	{
		m_lastMotorParticule = m_time;

		if ( m_bDisplayInfo )  sheet = SH_FRONT;
		else                   sheet = SH_WORLD;

		pos = m_object->RetPosition(0);
		if ( !m_bDisplayInfo             &&
			 pos.y < m_water->RetLevel() )  // sous l'eau ?
		{
			float t = Mod(m_time, 3.5f);
			if ( t >= 2.2f || ( t >= 1.2f && t <= 1.4f ) )  // respire ?
			{
				pos = D3DVECTOR(1.0f, 0.2f, 0.0f);
				pos.z += (Rand()-0.5f)*0.5f;

				speed = pos;
				speed.y += 5.0f+Rand()*5.0f;
				speed.x += Rand()*2.0f;
				speed.z += (Rand()-0.5f)*2.0f;
				
				pos   = Transform(*mat, pos);
				speed = Transform(*mat, speed)-pos;

				dim.x = 0.12f;
				dim.y = dim.x;
				m_particule->CreateParticule(pos, speed, dim, PARTIBUBBLE, 3.0f, 0.0f, 0.0f);
			}
		}
		else	// hors de l'eau ?
		{
			pos = D3DVECTOR(0.0f, -0.5f, 0.0f);
			pos.z += (Rand()-0.5f)*0.5f;

			speed = pos;
			speed.y -= (1.5f+Rand()*1.5f) + vibLin.y;
			speed.x += (Rand()-0.5f)*2.0f;
			speed.z += (Rand()-0.5f)*2.0f;
			
//			mat = m_object->RetWorldMatrix(0);
			pos   = Transform(*mat, pos);
			speed = Transform(*mat, speed)-pos;

			dim.x = (Rand()*0.4f+0.4f)*(1.0f+Min(linSpeed*0.1f, 5.0f));
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTITOTO, 1.0f+Rand()*1.0f, 0.0f, 1.0f, sheet);
		}

		if ( m_actionType != -1  &&  // action en cours ?
			 m_progress <= 0.85f )
		{
			pos.x = (Rand()-0.5f)*1.0f;
			pos.y = (Rand()-0.5f)*1.0f+3.5f;
			pos.z = (Rand()-0.5f)*1.0f;
			pos   = Transform(*mat, pos);
			speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
			dim.x = (Rand()*0.3f+0.3f);
			dim.y = dim.x;
			if ( m_actionType == MT_ERROR   )  type = PARTIERROR;
			if ( m_actionType == MT_WARNING )  type = PARTIWARNING;
			if ( m_actionType == MT_INFO    )  type = PARTIINFO;
			if ( m_actionType == MT_MESSAGE )  type = PARTIWARNING;
			m_particule->CreateParticule(pos, speed, dim, type, 0.5f+Rand()*0.5f, 0.0f, 1.0f, sheet);

			pos.x = 0.50f+(Rand()-0.5f)*0.80f;
			pos.y = 0.86f+(Rand()-0.5f)*0.08f;
			pos.z = 0.00f;
			dim.x = (Rand()*0.04f+0.04f);
			dim.y = dim.x/0.75f;
			m_particule->CreateParticule(pos, speed, dim, type, 0.5f+Rand()*0.5f, 0.0f, 1.0f, SH_INTERFACE);
		}

//?		if ( m_bDisplayInfo && m_main->RetGlint() )
		if ( FALSE )
		{
			pos.x = (Rand()-0.5f)*1.4f;
			pos.y = (Rand()-0.5f)*1.4f+3.5f;
			pos.z = (Rand()-0.5f)*1.4f;
			pos   = Transform(*mat, pos);
			speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
			dim.x = (Rand()*0.5f+0.5f);
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTIERROR, 0.5f+Rand()*0.5f, 0.0f, 1.0f, sheet);

			for ( i=0 ; i<10 ; i++ )
			{
				pos.x = 0.60f+(Rand()-0.5f)*0.76f;
				pos.y = 0.47f+(Rand()-0.5f)*0.90f;
				pos.z = 0.00f;
				r = rand()%4;
					 if ( r == 0 )  pos.x = 0.21f;  // sur le bord gauche
				else if ( r == 1 )  pos.x = 0.98f;  // sur le bord droite
				else if ( r == 2 )  pos.y = 0.02f;  // sur le bord inférieur
				else                pos.y = 0.92f;  // sur le bord supérieur
				dim.x = (Rand()*0.02f+0.02f);
				dim.y = dim.x/0.75f;
				m_particule->CreateParticule(pos, speed, dim, PARTIERROR, 0.5f+Rand()*0.5f, 0.0f, 1.0f, SH_INTERFACE);
			}
		}
	}

	// Bouge le son.
	if ( m_soundChannel != -1 )
	{
		if ( !m_sound->Position(m_soundChannel, m_object->RetPosition(0)) )
		{
			m_soundChannel = -1;
		}
	}

	return TRUE;
}


// Démarre une action.

Error CMotionToto::SetAction(int action, float time)
{
	Sound	sound;

	CMotion::SetAction(action, time);

	m_bStartAction = TRUE;

	sound = SOUND_CLICK;
	if ( action == MT_ERROR   )  sound = SOUND_ERROR;
	if ( action == MT_WARNING )  sound = SOUND_WARNING;
	if ( action == MT_INFO    )  sound = SOUND_INFO;
	if ( action == MT_MESSAGE )  sound = SOUND_MESSAGE;

	if ( sound != SOUND_CLICK )
	{
		m_soundChannel = m_sound->Play(sound, m_object->RetPosition(0));
	}

	return ERR_OK;
}

// Spécifie le type de l'objet rattaché à toto.

void CMotionToto::SetLinkType(ObjectType type)
{
	m_type = type;
}


