// motiongun.cpp

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
#include "pyro.h"
#include "camera.h"
#include "modfile.h"
#include "sound.h"
#include "robotmain.h"
#include "motion.h"
#include "motiongun.h"




// Constructeur de l'objet.

CMotionGun::CMotionGun(CInstanceManager* iMan, CObject* object)
							  : CMotion(iMan, object)
{
	CMotion::CMotion(iMan, object);

	m_time = 0.0f;
	m_lastParticule = 0.0f;
	m_leftTrack = 0.0f;
	m_rightTrack = 0.0f;
	m_channelSound = -1;
}

// Destructeur de l'objet.

CMotionGun::~CMotionGun()
{
	StopMotor();
}


// Supprime un objet.

void CMotionGun::DeleteObject(BOOL bAll)
{
}


// Démarre une action spéciale.

Error CMotionGun::SetAction(int action, float time)
{
	if ( time == 0.2f )  // valeur par défaut ?
	{
		if ( action == MGUN_ERROR )  time = 1.0f/1.50f;
	}

	if ( action == MGUN_ERROR )
	{
		SetActionLinSpeed(1.6f);
		SetActionCirSpeed(0.0f);
	}

	if ( action == MGUN_WAIT )
	{
		SetActionLinSpeed(0.0f);
		SetActionCirSpeed(0.0f);
	}

	if ( action == MGUN_FIRE )
	{
		Fire();
		return ERR_OK;
	}

	return CMotion::SetAction(action, time);
}


// Retourne la vitesse linéaire.

float CMotionGun::RetLinSpeed()
{
	return 5.0f;
}

// Retourne la vitesse linéaire.

float CMotionGun::RetCirSpeed()
{
	return 0.5f*PI;
}

// Retourne la distance linéaire de freinage.

float CMotionGun::RetLinStopLength()
{
	return 2.0f;
}

// Spécifie la vitesse linéaire dans l'action.

void CMotionGun::SetActionLinSpeed(float speed)
{
	CMotion::SetActionLinSpeed(speed);
}

// Spécifie la vitesse circulaire dans l'action.

void CMotionGun::SetActionCirSpeed(float speed)
{
	CMotion::SetActionCirSpeed(speed);
}


// Crée une perforatrice quelconque posé sur le sol.

BOOL CMotionGun::Create(D3DVECTOR pos, float angle, ObjectType type)
{
	CModFile*	pModFile;
	int			rank, option;

	if ( m_engine->RetRestCreate() < 10 )  return FALSE;

	m_object->SetType(type);
	pModFile = new CModFile(m_iMan);

	// Crée la base principale.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEOBJECT);  // c'est un objet fixe
	m_object->SetObjectRank(0, rank);
	pModFile->ReadModel("objects\\gun1.mod");
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(0, pos);
	m_object->SetAngleY(0, angle);

	// Crée la chenille droite.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEDESCENDANT);
	m_object->SetObjectRank(1, rank);
	m_object->SetObjectParent(1, 0);
	pModFile->ReadModel("objects\\gun2.mod");
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(1, D3DVECTOR(0.0f, 0.0f, -2.5f));

	// Crée la chenille gauche.
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEDESCENDANT);
	m_object->SetObjectRank(2, rank);
	m_object->SetObjectParent(2, 0);
	pModFile->ReadModel("objects\\gun3.mod");
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(2, D3DVECTOR(0.0f, 0.0f, 2.5f));

	// Crée le bouton "avance".
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEDESCENDANT);
	m_object->SetObjectRank(3, rank);
	m_object->SetObjectParent(3, 0);
	pModFile->ReadModel("objects\\gun4.mod");
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(3, D3DVECTOR(0.0f, 0.0f, 0.0f));

	// Crée le bouton "gauche".
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEDESCENDANT);
	m_object->SetObjectRank(4, rank);
	m_object->SetObjectParent(4, 0);
	pModFile->ReadModel("objects\\gun5.mod");
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(4, D3DVECTOR(0.0f, 0.0f, 0.0f));

	// Crée le bouton "droite".
	rank = m_engine->CreateObject();
	m_engine->SetObjectType(rank, TYPEDESCENDANT);
	m_object->SetObjectRank(5, rank);
	m_object->SetObjectParent(5, 0);
	pModFile->ReadModel("objects\\gun6.mod");
	pModFile->CreateEngineObject(rank);
	m_object->SetPosition(5, D3DVECTOR(0.0f, 0.0f, 0.0f));

	UpdateTrackMapping(m_leftTrack, m_rightTrack);

	m_object->CreateLockZone(0, 0, LZ_BOT);
	m_object->CreateShadow(4.0f, 1.0f, D3DSHADOWNORM, TRUE, 0.5f);
	m_object->SetFloorHeight(0.0f);

	pos = m_object->RetPosition(0);
	m_object->SetPosition(0, pos);  // pour afficher les ombres tout de suite

//?	m_engine->LoadAllTexture();

	option = m_object->RetOption();
	SetAction(MGUN_WAIT);

	delete pModFile;
	return TRUE;
}


// Gestion d'un événement.

BOOL CMotionGun::EventProcess(const Event &event)
{
	CMotion::EventProcess(event);

	if ( event.event == EVENT_FRAME )
	{
		return EventFrame(event);
	}

	return TRUE;
}

// Gestion d'un événement.

BOOL CMotionGun::EventFrame(const Event &event)
{
	D3DVECTOR	linVib, cirVib, pos, speed;
	FPOINT		dim;
	float		left, right, smoke, error, freq, angle;

	if ( m_engine->RetPause() )  return TRUE;

	m_time += event.rTime;

	linVib.x = 0.0f;
	linVib.y = 0.0f;
	linVib.z = 0.0f;
	cirVib.x = 0.0f;
	cirVib.y = 0.0f;
	cirVib.z = 0.0f;

	if ( m_progress >= 1.0f &&
		(m_actionType == MGUN_ERROR ||
		 (m_actionLinSpeed == 0 && m_actionCirSpeed == 0) ) )
	{
		SetAction(MGUN_WAIT, 1.0f/10.0f);
	}

	// Fait tourner le perforateur.
	angle = m_object->RetAngleX(6);
	if ( m_actionType == MGUN_WAIT )
	{
		angle -= event.rTime*1.5f;
	}
	else
	{
		angle -= event.rTime*10.0f;
	}
	m_object->SetAngleX(6, angle);

	// Bouge les chenilles.
	if ( m_actionLinSpeed != 0 || m_actionCirSpeed != 0 )
	{
		left  = m_actionLinSpeed;
		right = m_actionLinSpeed;
		if ( m_actionCirSpeed < 0.0f )
		{
			left  = -1.0f;
			right =  1.0f;
		}
		if ( m_actionCirSpeed > 0.0f )
		{
			left  =  1.0f;
			right = -1.0f;
		}

		m_leftTrack  += event.rTime*left*4.0f;
		m_rightTrack += event.rTime*right*4.0f;

		UpdateTrackMapping(m_leftTrack, m_rightTrack);
	}

	// Fait fumer le pot d'échappement.
	smoke = m_actionLinSpeed+Abs(m_actionCirSpeed);
	error = 0.0f;
	if ( m_actionType == MGUN_ERROR )  error = 1.0f;
	ParticuleFrame(event.rTime, smoke, error);

	// Gestion du bruit du moteur.
	if ( smoke == 0.0f )
	{
		StopMotor();
	}
	else
	{
		freq = m_actionLinSpeed;
		if ( m_actionCirSpeed != 0.0f )  freq = 0.6f;
		StartMotor(freq);
	}

	// Vibrations lorque le moteur tourne.
	if ( smoke > 0.0f )
	{
		linVib.y += 0.1f+sinf(m_time*40.0f)*0.1f;
		cirVib.y += sinf(m_time*10.0f)*(2.0f*PI/180.0f);
	}

	pos = m_object->RetPosition(0);
	cirVib.z += (pos.y/8.0f)*(90.0f*PI/180.0f);  // penche en avant si tombe

	m_object->SetLinVibration(linVib);
	m_object->SetCirVibration(cirVib);

	return TRUE;
}


// Fait évoluer les particules.

void CMotionGun::ParticuleFrame(float rTime, float smoke, float error)
{
	D3DMATRIX*	mat;
	D3DVECTOR	pos, speed;
	FPOINT		dim;
	float		duration;

	if ( m_lastParticule+0.05f <= m_time )
	{
		m_lastParticule = m_time;

		mat = m_object->RetWorldMatrix(0);
		pos = Transform(*mat, D3DVECTOR(0.5f, 9.0f, 0.0f));

		if ( smoke > 0.0f )
		{
			speed.x = (Rand()-0.5f)*2.0f;
			speed.z = (Rand()-0.5f)*2.0f;
			speed.y = Rand()*5.0f;
			dim.x = (1.5f+Rand()*1.5f)*smoke;
			dim.y = dim.x;
			duration = 2.0f+Rand()*2.0f;
			m_particule->CreateParticule(pos, speed, dim, PARTISMOKE1, duration);
		}

		if ( error > 0.0f )
		{
			speed.x = (Rand()-0.5f)*4.0f;
			speed.z = (Rand()-0.5f)*4.0f;
			speed.y = 10.0f+Rand()*10.0f;
			dim.x = 1.0f;
			dim.y = 1.0f;
			duration = 2.0f+Rand()*2.0f;
			m_particule->CreateParticule(pos, speed, dim, PARTILENS4, 1.0f, 40.0f);
		}
	}
}

// Met à jour le mapping de la texture des chenilles.

void CMotionGun::UpdateTrackMapping(float left, float right)
{
	D3DMATERIAL7	mat;
	int				rRank, lRank;

	ZeroMemory( &mat, sizeof(D3DMATERIAL7) );
	mat.diffuse.r = 1.0f;
	mat.diffuse.g = 1.0f;
	mat.diffuse.b = 1.0f;  // blanc
	mat.ambient.r = 0.5f;
	mat.ambient.g = 0.5f;
	mat.ambient.b = 0.5f;

	rRank = m_object->RetObjectRank(1);
	lRank = m_object->RetObjectRank(2);

	m_engine->TrackTextureMapping(rRank, mat, D3DSTATEPART1, "trax.tga", "",
								  0.0f, 1000000.0f, D3DMAPPINGX,
								  right, 1.0f, 8.0f, 192.0f, 256.0f);

	m_engine->TrackTextureMapping(lRank, mat, D3DSTATEPART2, "trax.tga", "",
								  0.0f, 1000000.0f, D3DMAPPINGX,
								  left, 1.0f, 8.0f, 192.0f, 256.0f);
}

// Fait entendre le son du moteur.

void CMotionGun::StartMotor(float freq)
{
	if ( m_channelSound != -1 )  return;

	m_motorFreq = freq;
	m_channelSound = m_sound->Play(SOUND_TRAX, m_object->RetPosition(0), 1.0f, 0.5f*m_motorFreq, TRUE);
	m_sound->AddEnvelope(m_channelSound, 1.0f, 0.5f*m_motorFreq, 0.01f, SOPER_CONTINUE);
	m_sound->AddEnvelope(m_channelSound, 1.0f, 1.0f*m_motorFreq, 0.25f, SOPER_CONTINUE);
	m_sound->AddEnvelope(m_channelSound, 1.0f, 1.0f*m_motorFreq, 1.00f, SOPER_LOOP);
}

// Stoppe le son du moteur.

void CMotionGun::StopMotor()
{
	if ( m_channelSound == -1 )  return;

	m_sound->FlushEnvelope(m_channelSound);
	m_sound->AddEnvelope(m_channelSound, 1.0f, 1.0f*m_motorFreq, 0.01f, SOPER_CONTINUE);
	m_sound->AddEnvelope(m_channelSound, 0.0f, 0.5f*m_motorFreq, 0.50f, SOPER_CONTINUE);
	m_sound->AddEnvelope(m_channelSound, 0.0f, 0.5f*m_motorFreq, 0.01f, SOPER_STOP);
	m_channelSound = -1;
}


// Fait feu avec le canon.

void CMotionGun::Fire()
{
	D3DMATRIX*	mat;
	D3DVECTOR	pos, goal, speed;
	FPOINT		dim;
	CPyro*		pyro;
	CObject*	target;
	ObjectType	type;
	float		duration, mass;
	int			i;

	pos = m_object->RetPosition(0);
	if ( pos.y < 0.0 )  return;  // en train de tomber ?

	mat = m_object->RetWorldMatrix(0);

	for ( i=0 ; i<20 ; i++ )
	{
		goal = Transform(*mat, D3DVECTOR(8.0f*(i+1), 0.0f, 0.0f));
		target = SearchTarget(goal, 4.0f);
		if ( target != 0 )  break;
	}

	m_sound->Play(SOUND_GUN, m_object->RetPosition(0));

	pos = m_object->RetPosition(0);
	goal.y = 4.5f;
	pos.y  = 4.5f;
	dim.x = 3.0f;
	dim.y = dim.x;
	m_particule->CreateRay(pos, goal, PARTIRAY1, dim, 1.0f);

	for ( i=0 ; i<10 ; i++ )
	{
		pos = Transform(*mat, D3DVECTOR(3.0f, 4.5f, 0.0f));
		speed.x = 3.0f+Rand()*3.0f;
		speed.y = 1.0f+Rand()*1.0f;
		speed.z = (Rand()-0.5f)*2.0f;
		speed = Transform(*mat, speed);
		speed -= m_object->RetPosition(0);
		dim.x = 4.0f;
		dim.y = dim.x;
		duration = 2.0f+Rand()*2.0f;
		m_particule->CreateParticule(pos, speed, dim, PARTISMOKE1, duration);

		pos = Transform(*mat, D3DVECTOR(3.0f, 4.5f, 0.0f));
		speed.x = 10.0f+Rand()*5.0f;
		speed.z = (Rand()-0.5f)*10.0f;
		speed.y = 5.0f+Rand()*5.0f;
		speed = Transform(*mat, speed);
		speed -= m_object->RetPosition(0);
		dim.x = 0.2f;
		dim.y = dim.x;
		duration = Rand()*3.0f+2.0f;
		mass = Rand()*10.0f+15.0f;
		m_particule->CreateTrack(pos, speed, dim, PARTITRACK8,
								 duration, mass, Rand()+0.7f, 0.7f);
	}

	if ( target != 0 )
	{
		type = target->RetType();
		if ( type >= OBJECT_BOX1  &&
			 type <= OBJECT_BOX20 )
		{
			pyro = new CPyro(m_iMan);
			pyro->Create(PT_FRAGT, target);  // destruction caisse
		}
		if ( type == OBJECT_BLUPI )
		{
			target->SetDead(TRUE);  // pour le undo !
			pyro = new CPyro(m_iMan);
			pyro->Create(PT_EXPLOO, target);  // mort blupi
		}
	}
}

// Cherche une cible quelconque sur le sol.

CObject* CMotionGun::SearchTarget(D3DVECTOR center, float radius)
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
		if ( type == OBJECT_LIFT )  continue;
		if ( type == OBJECT_MINE )  continue;

		pos = pObj->RetPosition(0);
		if ( pos.y < 0.0f )  continue;  // en dessous du sol ?

		dist = Length2d(pos, center);
		if ( dist <= radius && dist < min )
		{
			min = dist;
			pBest = pObj;
		}
	}
	return pBest;
}

