// autodock.cpp

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
#include "particule.h"
#include "terrain.h"
#include "camera.h"
#include "object.h"
#include "interface.h"
#include "button.h"
#include "window.h"
#include "robotmain.h"
#include "mainundo.h"
#include "motion.h"
#include "motionblupi.h"
#include "sound.h"
#include "tasklist.h"
#include "auto.h"
#include "autodock.h"




// Constructeur de l'objet.

CAutoDock::CAutoDock(CInstanceManager* iMan, CObject* object)
						 : CAuto(iMan, object)
{
	CAuto::CAuto(iMan, object);

	Init();
}

// Destructeur de l'objet.

CAutoDock::~CAutoDock()
{
	StopBzzz();
	CAuto::~CAuto();
}


// Détruit l'objet.

void CAutoDock::DeleteObject(BOOL bAll)
{
	m_phase = ADKP_WAIT;
	CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoDock::Init()
{
//?	m_rest = 8;
	m_rest = m_object->RetOption();
	m_progressRest = 0.0f;
	m_currentPos = D3DVECTOR(0.0f, 1.0f, 0.0f);  // en haut au centre

	m_time = 0.0f;
	m_phase    = ADKP_WAIT;
	m_progress = 0.0f;
	m_speed    = 1.0f/1.0f;

	m_lastParticule = 0.0f;
	m_channelSound = -1;
}


// Démarre l'objet.

BOOL CAutoDock::Start(int part)
{
	CObject*	pObj;

	if ( m_phase != ADKP_WAIT )  return FALSE;  // occupé ?

	if ( m_rest == 0 )
	{
		m_sound->Play(SOUND_CHOCm, m_posPiston, 1.0f, 1.0f);
		m_phase    = ADKP_ERROR;
		m_progress = 0.0f;
		m_speed    = 1.0f/1.0f;
		return FALSE;
	}

	if ( part == 4 )  // ^ ?
	{
		if ( m_currentPos.x > -3.0f &&
			 IsFreePos(D3DVECTOR(-1.0f, 0.0f, 0.0f)) )
		{
			m_phase    = ADKP_MOVEXM;
			m_progress = 0.0f;
			m_speed    = 1.0f/2.0f;
			m_bBusy    = TRUE;
			SoundManip(2.0f, 1.0f, 1.0f);
		}
	}
	if ( part == 5 )  // v ?
	{
		if ( m_currentPos.x < 3.0f &&
			 IsFreePos(D3DVECTOR(1.0f, 0.0f, 0.0f)) )
		{
			m_phase    = ADKP_MOVEXP;
			m_progress = 0.0f;
			m_speed    = 1.0f/2.0f;
			m_bBusy    = TRUE;
			SoundManip(2.0f, 1.0f, 1.0f);
		}
	}

	if ( part == 6 )  // piston ?
	{
		pObj = SearchObject(m_posPistonGround, 2.0f);

		if ( m_object->RetFret() == 0 )  // ne porte rien ?
		{
			if ( pObj != 0 && pObj->RetHeight() > 8.0f )
			{
				return FALSE;
			}
		}
		else	// porte un objet ?
		{
			if ( pObj != 0 )  return FALSE;
			if ( !IsFlatGround() )  return FALSE;
		}
		m_phase    = ADKP_TAKE;
		m_progress = 0.0f;
		m_speed    = 1.0f/1.5f;
		m_bBusy    = TRUE;
		m_bTake    = FALSE;
		m_sound->Play(SOUND_PSHHH, m_posPiston, 1.0f, 1.0f);
	}

	if ( part == 7 )  // < ?
	{
		if ( m_currentPos.z > -1.0f &&
			 IsFreePos(D3DVECTOR(0.0f, 0.0f, -1.0f)) )
		{
			m_phase    = ADKP_MOVEZM;
			m_progress = 0.0f;
			m_speed    = 1.0f/2.0f;
			m_bBusy    = TRUE;
			SoundManip(2.0f, 1.0f, 0.5f);
		}
	}
	if ( part == 8 )  // > ?
	{
		if ( m_currentPos.z < 1.0f &&
			 IsFreePos(D3DVECTOR(0.0f, 0.0f, 1.0f)) )
		{
			m_phase    = ADKP_MOVEZP;
			m_progress = 0.0f;
			m_speed    = 1.0f/2.0f;
			m_bBusy    = TRUE;
			SoundManip(2.0f, 1.0f, 0.5f);
		}
	}

	if ( m_phase == ADKP_WAIT )  return FALSE;  // rien démarré ?

	m_initialPos = m_currentPos;
	return TRUE;
}


// Gestion d'un événement.

BOOL CAutoDock::EventProcess(const Event &event)
{
	CObject*	fret;
	ObjectType	type;
	D3DVECTOR	pos, speed;
	FPOINT		dim;
	float		progress, pp;
	LockZone	lz;
	int			i;

	CAuto::EventProcess(event);

	if ( m_engine->RetPause() )  return TRUE;

	m_progress += event.rTime*m_speed;
	progress = Norm(m_progress);

	m_posPistonGround = CalcPosPiston();
	m_posPiston = m_posPistonGround;
	m_posPiston.y += 8.0f+m_currentPos.y*10.0f;

	UpdateRest(event.rTime);

	if ( m_object->RetFret() == 0 )  // porte rien ?
	{
		StopBzzz();  // stoppe le son de l'électro-aimant
	}
	else	// porte qq chose ?
	{
		StartBzzz();  // démarre le son de l'électro-aimant
		PosBzzz();  // déplace le son de l'électro-aimant

		if ( m_lastParticule+0.05f <= m_time )
		{
			m_lastParticule = m_time;

			pos = m_posPiston;
			for ( i=0 ; i<5 ; i++ )
			{
				speed.x = (Rand()-0.5f)*20.0f;
				speed.z = (Rand()-0.5f)*20.0f;
				speed.y = 5.0f+Rand()*10.0f;
				dim.x = 0.8f;
				dim.y = 0.8f;
				m_particule->CreateParticule(pos, speed, dim, PARTIBLITZ, 1.0f, 40.0f);
			}
		}
	}

	if ( m_phase == ADKP_WAIT )  // attend ?
	{
		if ( progress >= 1.0f )
		{
			m_phase    = ADKP_WAIT;
			m_progress = 0.0f;
			m_speed    = 1.0f/1.0f;
			return TRUE;
		}
	}

	if ( m_phase == ADKP_MOVEXM )
	{
		m_currentPos.x = m_initialPos.x-Soft(progress);
	}
	if ( m_phase == ADKP_MOVEXP )
	{
		m_currentPos.x = m_initialPos.x+Soft(progress);
	}

	if ( m_phase == ADKP_MOVEZM )
	{
		m_currentPos.z = m_initialPos.z-Soft(progress);
	}
	if ( m_phase == ADKP_MOVEZP )
	{
		m_currentPos.z = m_initialPos.z+Soft(progress);
	}

	if ( m_phase == ADKP_TAKE )
	{
		if ( progress < 0.33f )  // descend ?
		{
			pp = progress/0.33f;
			m_currentPos.y = m_initialPos.y-Bounce(pp);
		}
		else	// remonte ?
		{
			pp = Norm(1.0f-(progress-0.33f)/0.67f);
			m_currentPos.y = m_initialPos.y-Soft(pp);

			if ( !m_bTake )
			{
				m_bTake = TRUE;
				if ( m_object->RetFret() == 0 )  // ne porte rien ?
				{
					fret = SearchFret();
					if ( fret != 0 )
					{
						m_terrain->SetLockZone(fret->RetPosition(0), LZ_FREE);
						type = fret->RetType();
						m_object->SetFret(fret);
						fret->SetTruck(m_object);
						fret->SetTruckPart(3);
						pos = D3DVECTOR(0.0f, 0.0f, 0.0f);
						if ( type == OBJECT_BLUPI )
						{
							pos.y += 4.0f;
						}
						fret->SetPosition(0, pos);
						fret->SetLock(TRUE);

						if ( type == OBJECT_BLUPI )
						{
							StartAction(fret, MBLUPI_TRUCK);
						}
					}
				}
				else	// porte qq chose ?
				{
					fret = m_object->RetFret();
					type = fret->RetType();
					m_object->SetFret(0);
					fret->SetTruck(0);
					pos = m_posPistonGround;
					if ( type == OBJECT_BLUPI )
					{
						pos.y += fret->RetCharacter()->height;
					}
					fret->SetPosition(0, pos);
					fret->SetLock(FALSE);

					lz = LZ_BOX;
					if ( type == OBJECT_BOX7  )  lz = LZ_BOXo;
					if ( type == OBJECT_BLUPI )  lz = LZ_BLUPI;
					m_terrain->SetLockZone(pos, lz);

					if ( type == OBJECT_BLUPI )
					{
						StartAction(fret, MBLUPI_WAIT);
					}
				}
				SoundManip(0.5f, 1.0f, 1.5f);
			}
		}
	}

	UpdatePosition();

	if ( progress >= 1.0f )
	{
		if ( m_phase == ADKP_TAKE )
		{
			DecRest();
		}
		m_phase    = ADKP_WAIT;
		m_progress = 0.0f;
		m_speed    = 1.0f/1.0f;
		m_bBusy    = FALSE;
		return TRUE;
	}

	return TRUE;
}

// Met à jour les positions des différentes parties.

void CAutoDock::UpdatePosition()
{
	D3DVECTOR	pos;
	int			objRank;

	pos.x = 0.0f;
	pos.y = 0.0f;
	pos.z = m_currentPos.z*8.0f;
	m_object->SetPosition(1, pos);  // portique

	pos.x = m_currentPos.x*8.0f;
	pos.y = 0.0f;
	pos.z = 0.0f;
	m_object->SetPosition(2, pos);  // charriot

	pos.x = 0.0f;
	pos.y = m_currentPos.y*10.0f;
	pos.z = 0.0f;
	m_object->SetPosition(3, pos);  // piston

	objRank = m_object->RetObjectRank(0);
	m_engine->SetObjectShadowPos(objRank, m_posPistonGround);
}


// Diminue le nombre d'utilisations restantes.

BOOL CAutoDock::DecRest()
{
	if ( m_rest == 0 )  return FALSE;

	m_rest --;
	m_progressRest = 1.0f;

	if ( m_rest == 0 )
	{
		m_sound->Play(SOUND_CHOCm, m_posPiston, 1.0f, 1.0f);
	}
	else
	{
		m_sound->Play(SOUND_ENERGY, m_posPiston, 1.0f, 1.0f);
	}
	return TRUE;
}

// Met à jour la position du nombre d'utilisations restantes.

void CAutoDock::UpdateRest(float rTime)
{
	D3DMATRIX*	mat;
	D3DVECTOR	pos, speed;
	FPOINT		dim;
	float		suppl;

	if ( m_phase == ADKP_ERROR )
	{
		if ( m_progress < 0.5f )
		{
			suppl = Soft(m_progress/0.5f)*16.0f;
		}
		else
		{
			suppl = Soft(1.0f-(m_progress-0.5f)/0.5f)*16.0f;

			mat = m_object->RetWorldMatrix(0);
			pos = Transform(*mat, D3DVECTOR(-30.0f, 4.0f, 0.0f));
			pos.z += m_currentPos.z*8.0f;
			pos.x += (Rand()-0.5f)*2.0f;
			pos.z += (Rand()-0.5f)*2.0f;
			speed.x = (Rand()-0.5f)*2.0f;
			speed.z = (Rand()-0.5f)*2.0f;
			speed.y = 2.0f+Rand()*4.0f;
			dim.x = 6.0f;
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTISMOKE2, 3.0f);
		}

		pos.x = 0.0f;
		pos.y = m_rest*2.0f+suppl;
		pos.z = 0.0f;
		m_object->SetPosition(9, pos);  // jauge

		return;
	}
	
	if ( m_progressRest == 0.0f )
	{
		suppl = 0.0f;
	}
	else
	{
		m_progressRest -= rTime*1.0f;
		if ( m_progressRest < 0.0f )  m_progressRest = 0.0f;
		suppl = (1.0f-Bounce(1.0f-m_progressRest))*2.0f;

		if ( m_rest == 0 )
		{
			mat = m_object->RetWorldMatrix(0);
			pos = Transform(*mat, D3DVECTOR(-30.0f, 4.0f, 0.0f));
			pos.z += m_currentPos.z*8.0f;
			pos.x += (Rand()-0.5f)*2.0f;
			pos.z += (Rand()-0.5f)*2.0f;
			speed.x = (Rand()-0.5f)*2.0f;
			speed.z = (Rand()-0.5f)*2.0f;
			speed.y = 2.0f+Rand()*4.0f;
			dim.x = 6.0f;
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTISMOKE2, 3.0f);
		}
	}

	pos.x = 0.0f;
	pos.y = m_rest*2.0f+suppl;
	pos.z = 0.0f;
	m_object->SetPosition(9, pos);  // jauge
}


// Stoppe l'automate.

BOOL CAutoDock::Abort()
{
	return TRUE;
}


// Indique si l'objet effectue une action.

BOOL CAutoDock::IsRunning()
{
	return ( m_phase != ADKP_WAIT );
}


// Retourne une erreur liée à l'état de l'automate.

Error CAutoDock::RetError()
{
	return ERR_OK;
}


// Cherche si le sol est à niveau sous le piston.

BOOL CAutoDock::IsFlatGround()
{
	float	level;

	level = m_terrain->RetFloorLevel(m_posPistonGround);
	return ( level == 0.0f );
}

// Cherche l'objet transportable sous le piston.

CObject* CAutoDock::SearchFret()
{
	CObject*	fret;
	CTaskList*	taskList;
	ObjectType	type;

	fret = SearchObject(m_posPistonGround, 2.0f);
	if ( fret == 0 )  return 0;

	type = fret->RetType();

	if ( (type >= OBJECT_BOX1 && type <= OBJECT_BOX20) ||
		 (type >= OBJECT_KEY1 && type <= OBJECT_KEY5 ) )
	{
		return fret;
	}

	if ( type == OBJECT_BLUPI )
	{
		taskList = fret->RetTaskList();
		if ( taskList == 0 )  return fret;
		if ( !taskList->IsRunning() )  return fret;
	}

	return 0;
}

// Cherche un objet quelconque.

CObject* CAutoDock::SearchObject(D3DVECTOR center, float radius)
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

// Regarde s'il est possible d'atteindre une position donnée.
// Si le dock porte un objet, on ne peut pas passer par-dessus
// les objets hauts !

BOOL CAutoDock::IsFreePos(D3DVECTOR dir)
{
	CObject*	pObj;
	D3DVECTOR	piston;

	if ( m_object->RetFret() == 0 )  return TRUE;  // ne porte rien ?

	piston = CalcPosPiston(m_currentPos+dir);
	pObj = SearchObject(piston, 2.0f);
	if ( pObj == 0 )  return TRUE;

	if ( pObj->RetHeight() > 8.0f )  return FALSE;
	return TRUE;
}

// Calcule la position du piston au sol.

D3DVECTOR CAutoDock::CalcPosPiston()
{
	return CalcPosPiston(m_currentPos);
}

// Calcule la position du piston au sol.

D3DVECTOR CAutoDock::CalcPosPiston(D3DVECTOR piston)
{
	D3DVECTOR	pos;

	pos = m_object->RetPosition(0);
	pos += piston*8.0f;
	pos.y = 0.0f;
	return pos;
}


// Démarre une action pour Blupi.

void CAutoDock::StartAction(CObject *pObj, int action, float speed)
{
	CMotion*	motion;

	motion = pObj->RetMotion();
	if ( motion == 0 )  return;

	motion->SetAction(action, speed);
}


// Fait entendre le son du bras manipulateur.

void CAutoDock::SoundManip(float time, float amplitude, float frequency)
{
	int			i;

	i = m_sound->Play(SOUND_MANIP, m_posPiston, 0.0f, 0.3f*frequency, TRUE);
	m_sound->AddEnvelope(i, 0.5f*amplitude, 1.0f*frequency, 0.1f, SOPER_CONTINUE);
	m_sound->AddEnvelope(i, 0.5f*amplitude, 1.0f*frequency, time-0.1f, SOPER_CONTINUE);
	m_sound->AddEnvelope(i, 0.0f, 0.3f*frequency, 0.1f, SOPER_STOP);
}

// Fait entendre le son de l'électro-aimant.

void CAutoDock::StartBzzz()
{
	if ( m_channelSound != -1 )  return;
	m_channelSound = m_sound->Play(SOUND_NUCLEAR, m_posPiston, 0.5f, 1.0f, TRUE);
	m_sound->AddEnvelope(m_channelSound, 0.5f, 1.0f, 1.0f, SOPER_LOOP);
}

// Stoppe le son de l'électro-aimant.

void CAutoDock::StopBzzz()
{
	if ( m_channelSound == -1 )  return;
	m_sound->FlushEnvelope(m_channelSound);
	m_sound->AddEnvelope(m_channelSound, 0.5f, 1.0f, 0.1f, SOPER_STOP);
	m_channelSound = -1;
}

// Déplace le son de l'électro-aimant.

void CAutoDock::PosBzzz()
{
	if ( m_channelSound == -1 )  return;
	m_sound->Position(m_channelSound, m_posPiston);
}


// Ecrit la situation de l'objet.

void CAutoDock::WriteSituation()
{
	m_undo->WriteTokenInt("phase", m_phase);
	m_undo->WriteTokenPos("piston", m_currentPos);
	m_undo->WriteTokenInt("rest", m_rest);
}

// lit la situation de l'objet.

void CAutoDock::ReadSituation()
{
	D3DVECTOR	pos;
	int			phase;

	if ( m_undo->ReadTokenInt("phase", phase) )
	{
		m_phase = (AutoDockPhase)phase;
		m_progress = 0.0f;
		m_speed = 1.0f/0.001f;  // instantané (ou presque)

	}

	if ( m_undo->ReadTokenPos("piston", pos) )
	{
		m_currentPos = pos;
		UpdatePosition();
	}

	m_undo->ReadTokenInt("rest", m_rest);
}

