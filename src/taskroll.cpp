// taskroll.cpp

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
#include "terrain.h"
#include "water.h"
#include "object.h"
#include "particule.h"
#include "motion.h"
#include "motionblupi.h"
#include "pyro.h"
#include "camera.h"
#include "sound.h"
#include "task.h"
#include "taskroll.h"



#define BOX_DIST	8.0f



// Constructeur de l'objet.

CTaskRoll::CTaskRoll(CInstanceManager* iMan, CObject* object)
					 : CTask(iMan, object)
{
	CTask::CTask(iMan, object);

	m_phase = TRO_NULL;
	m_total = 0;
	m_totalMash = 0;
	m_pMash[0] = 0;
	m_time = 0.0f;
	m_lastParticuleSmoke = 0.0f;
}

// Destructeur de l'objet.

CTaskRoll::~CTaskRoll()
{
}


// Gestion d'un événement.

BOOL CTaskRoll::EventProcess(const Event &event)
{
	D3DVECTOR	pos, speed;
	FPOINT		dim;
	float		progress, mass, duration, angle, limit;

	if ( m_engine->RetPause() )  return TRUE;
	if ( event.event != EVENT_FRAME )  return TRUE;
	if ( m_bError )  return FALSE;

	m_time += event.rTime;
	m_progress += event.rTime*m_speed;
	progress = Norm(m_progress);

	if ( m_phase == TRO_ROLL )  // roule une sphère ?
	{
		if ( m_bImpossible )
		{
			pos = m_startPos;
			pos.y += sinf(progress*PI)*0.5f;
			m_object->SetPosition(0, pos);  // petit saut

			if ( m_lastParticuleSmoke+0.05f <= m_time )
			{
				m_lastParticuleSmoke = m_time;

				pos = m_startPos;
				pos.y += 2.0f;
				pos.x += (Rand()-0.5f)*4.0f;
				pos.z += (Rand()-0.5f)*4.0f;
				speed.x = (Rand()-0.5f)*2.0f;
				speed.z = (Rand()-0.5f)*2.0f;
				speed.y = Rand()*4.0f;
				dim.x = 8.0f;
				dim.y = dim.x;
				m_particule->CreateParticule(pos, speed, dim, PARTISMOKE1, 3.0f);
			}
		}
		else
		{
			pos = LineProgress(m_startPos, m_goalPos, progress);
			if ( m_bHole )
			{
				if ( m_bPipe )  // caisse tombe sur tuyau ?
				{
					if ( m_progress > 0.8f )
					{
						pos.y -= (m_progress-0.8f)/0.2f*2.0f;
					}
				}
				else if ( !m_bSpace )  // sphère tombe dans trou ?
				{
					if ( progress > 0.8f )
					{
						pos.y -= Bounce((progress-0.8f)/0.2f)*PUSH_DIST;
						if ( !m_bFallSound )
						{
							m_sound->Play(SOUND_FALLo2, m_goalPos, 1.0f);
							m_camera->StartEffect(CE_CRASH, m_goalPos, 0.5f);
							m_bFallSound = TRUE;
						}
					}
				}
				else if ( m_bSpace )  // caisse tombe dans infini ?
				{
					if ( m_progress > 0.8f )
					{
						pos.y -= (m_progress-0.8f)/0.2f*PUSH_DIST;
					}
				}
			}
			m_object->SetPosition(0, pos);  // avance la sphère

			if ( !m_bPlouf && pos.y < m_water->RetLevel() )
			{
				m_bPlouf = TRUE;
				pos.y = m_water->RetLevel()-1.0f;
				dim.x = 8.0f;  // hauteur
				dim.y = 4.0f;  // diamètre
				m_particule->CreateParticule(pos, D3DVECTOR(0.0f, 0.0f, 0.0f), dim, PARTIPLOUF0, 1.4f, 0.0f);
				m_sound->Play(SOUND_PLOUF, pos);
			}

			if ( m_startPos.x < m_goalPos.x )
			{
				angle = m_boxAngle.z;
				angle -= progress*2.3f;
				m_object->SetAngleZ(9, angle);
			}
			if ( m_startPos.x > m_goalPos.x )
			{
				angle = m_boxAngle.z;
				angle += progress*2.3f;
				m_object->SetAngleZ(9, angle);
			}
			if ( m_startPos.z < m_goalPos.z )
			{
				angle = m_boxAngle.x;
				angle += progress*2.3f;
				m_object->SetAngleX(9, angle);
			}
			if ( m_startPos.z > m_goalPos.z )
			{
				angle = m_boxAngle.x;
				angle -= progress*2.3f;
				m_object->SetAngleX(9, angle);
			}

			if ( m_bMash )
			{
				ExploTremble(progress);

				if ( m_mashType != OBJECT_BLUPI  &&
					 m_mashType != OBJECT_GLASS1 &&
					 m_mashType != OBJECT_GLASS2 &&
					 m_mashType != OBJECT_GOAL   &&
					 m_lastParticuleSmoke+0.05f <= m_time )
				{
					m_lastParticuleSmoke = m_time;

					pos = m_pMash[0]->RetPosition(0);
					pos.y += 2.0f;
					pos.x += (Rand()-0.5f)*8.0f;
					pos.z += (Rand()-0.5f)*8.0f;
					speed.x = (Rand()-0.5f)*2.0f;
					speed.z = (Rand()-0.5f)*2.0f;
					speed.y = Rand()*8.0f;
					dim.x = 8.0f;
					dim.y = dim.x;
					m_particule->CreateParticule(pos, speed, dim, PARTISMOKE1, 3.0f);

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

				limit = 0.5f;
				if ( m_mashType == OBJECT_GLASS1 ||
					 m_mashType == OBJECT_GLASS2 )  limit = 0.8f;
				if ( progress >= limit )
				{
					ExploObject();
				}
			}
		}
	}

	return TRUE;
}


// Assigne le but à atteindre.

Error CTaskRoll::Start(D3DVECTOR dir)
{
	LockZone	lz;
	int			i;

	m_dir = dir;
	m_dir.y = 0.0f;

	m_type = m_object->RetType();
	m_startPos = m_object->RetPosition(0);
	m_goalPos = m_startPos+m_dir;
	m_boxAngle = m_object->RetAngle(9);  // angle initial de la sphère

	m_bMash = FALSE;
	m_bExplo = FALSE;
	m_pMash[0] = SearchObject(m_goalPos, 4.0f);

	// Si on n'a pas trouvé un objet immobile, cherche si la destination
	// est éventuellement occupée par un blupi mobile. En fait, par un
	// blupi qui n'y est pas encore, mais qui s'y rend.
	if ( m_pMash[0] == 0 )
	{
		lz = m_terrain->RetLockZone(m_goalPos);
		if ( lz == LZ_BLUPI )
		{
			m_pMash[0] = SearchBlupi(m_goalPos, 8.0f);
		}
	}

	if ( m_pMash[0] == 0 )
	{
		m_totalMash = 0;
		m_mashType = OBJECT_NULL;
	}
	else
	{
		m_totalMash = 1;
		m_mashType = m_pMash[0]->RetType();
	}

	ExploProxi();  // cherche les objets proches

	if ( m_type == OBJECT_BOX7  ||  // sphère ?
		 m_type == OBJECT_BOX13 )   // cca ?
	{
		if ( m_mashType == OBJECT_MINE   ||
			 m_mashType == OBJECT_FIOLE  ||
			 m_mashType == OBJECT_GLU    ||
			 m_mashType == OBJECT_GLASS1 ||
			 m_mashType == OBJECT_GLASS2 ||
			 m_mashType == OBJECT_GOAL   ||
			 m_mashType == OBJECT_BLUPI  )
		{
			m_sound->Play(SOUND_CHOCo, m_startPos, 1.0f, 0.30f);
			for ( i=0 ; i<m_totalMash ; i++ )
			{
				m_pMash[i]->SetLock(TRUE);
			}
			m_bMash = TRUE;  // écrabouille
		}
	}

	if ( m_type == OBJECT_BOX10 )  // bombe ?
	{
		if ( (m_mashType >= OBJECT_BOX1 &&
			  m_mashType <= OBJECT_BOX20) ||
			 m_mashType == OBJECT_MINE    ||
			 m_mashType == OBJECT_FIOLE   ||
			 m_mashType == OBJECT_GLU     ||
			 m_mashType == OBJECT_GLASS1  ||
			 m_mashType == OBJECT_GLASS2  ||
			 m_mashType == OBJECT_GOAL    ||
			 m_mashType == OBJECT_BLUPI   )
		{
			m_sound->Play(SOUND_CHOCo, m_startPos, 1.0f, 0.30f);
			for ( i=0 ; i<m_totalMash ; i++ )
			{
				m_pMash[i]->SetLock(TRUE);
			}
			m_bMash = TRUE;  // écrabouille

			if ( m_mashType == OBJECT_MINE  ||
				 m_mashType == OBJECT_FIOLE ||
				 m_mashType == OBJECT_GLU   ||
				 m_mashType == OBJECT_BOX10 )
			{
				m_bExplo = TRUE;  // puis explose
			}
		}
	}

	m_bImpossible = !IsPosFree(m_goalPos);
	m_bHole = IsHole(m_goalPos);
	m_bSpace = IsSpace(m_goalPos);
	m_bPipe = FALSE;
	m_bPlouf = FALSE;
	m_bFallSound = FALSE;

	if ( m_mashType == OBJECT_BARRIER29 ||  // tuyaux ?
		 m_mashType == OBJECT_BARRIER39 ||  // tuyaux ?
		 m_mashType == OBJECT_BARRIER48 ||  // tuyaux ?
		 m_mashType == OBJECT_BARRIER49 ||  // tuyaux ?
		 m_mashType == OBJECT_BARRIER67 )   // tuyaux ?
	{
		m_bPipe  = TRUE;
	}

	if ( m_type == OBJECT_BOX10 )  // bombe ?
	{
		if ( m_bMash )  m_bImpossible = FALSE;
	}

	m_object->SetLock(TRUE);

	if ( m_bImpossible )
	{
		if ( m_total == 0 )
		{
			m_bError = TRUE;
			return ERR_GENERIC;
		}
		m_sound->Play(SOUND_CHOCo, m_startPos, 1.0f);
		m_camera->StartEffect(CE_CRASH, m_goalPos, 0.5f);
	}
	else
	{
		m_sound->Play(SOUND_WHEELg, m_startPos, 0.7f, 1.2f);
		m_total ++;
		m_terrain->SetLockZone(m_goalPos, LZ_BOXo);
	}

	m_phase = TRO_ROLL;
	m_progress = 0.0f;
	m_speed = m_bImpossible?(1.0f/0.2f):(1.0f/1.0f);
	m_bError = FALSE;
	return ERR_OK;
}

// Indique si l'action est terminée.

Error CTaskRoll::IsEnded()
{
	CPyro*		pyro;

	if ( m_engine->RetPause() )  return ERR_CONTINUE;

	if ( m_bError )
	{
		return ERR_STOP;
	}

	if ( m_progress < 1.0f )
	{
		return ERR_CONTINUE;
	}

	if ( m_phase == TRO_ROLL )
	{
		if ( m_bImpossible )
		{
			m_object->SetAngle(9, D3DVECTOR(0.0f, 0.0f, 0.0f));
			m_object->SetLock(FALSE);
			return ERR_STOP;
		}
		else if ( m_bHole )
		{
			m_terrain->SetLockZone(m_startPos, LZ_FREE);
			if ( m_bSpace || m_bPipe )  // dans un trou infini ?
			{
				m_object->DeletePart(1);  // supprime les flèches
				m_object->DeletePart(2);
				m_object->DeletePart(3);
				m_object->DeletePart(4);
				pyro = new CPyro(m_iMan);
				pyro->Create(PT_FRAGT, m_object);  // destruction caisse
				return ERR_STOP;
			}
			else
			{
				m_object->DeletePart(1);  // supprime les flèches
				m_object->DeletePart(2);
				m_object->DeletePart(3);
				m_object->DeletePart(4);
				pyro = new CPyro(m_iMan);
				pyro->Create(PT_FRAGT, m_object);  // destruction sphère
				return ERR_STOP;
			}
		}
		else
		{
			if ( m_bExplo )
			{
				m_terrain->SetLockZone(m_startPos, LZ_FREE);
				m_object->DeletePart(1);  // supprime les flèches
				m_object->DeletePart(2);
				m_object->DeletePart(3);
				m_object->DeletePart(4);
				pyro = new CPyro(m_iMan);
				pyro->Create(PT_FRAGT, m_object);  // destruction caisse
				return ERR_STOP;
			}
			else
			{
				m_terrain->SetLockZone(m_startPos, LZ_FREE);
				Start(m_dir);  // roule plus loin
				return ERR_CONTINUE;
			}
		}
	}

	return ERR_STOP;
}


// Cherche les objets proche à casser aussi. Pour les vitres,
// on considère toute la ligne comme un seul objet.

void CTaskRoll::ExploProxi()
{
	CObject*	pObj;
	D3DVECTOR	pos, v[2];
	float		dir;
	int			i;

	if ( m_mashType == OBJECT_GLASS1 ||
		 m_mashType == OBJECT_GLASS2 )
	{
		dir = m_pMash[0]->RetAngleY(0);
		dir = Grid(dir, PI/2.0f);
		dir = Mod(dir, PI);
		if ( dir == 0 )
		{
			v[0] = D3DVECTOR( 8.0f, 0.0f, 0.0f);
			v[1] = D3DVECTOR(-8.0f, 0.0f, 0.0f);
		}
		else
		{
			v[0] = D3DVECTOR(0.0f, 0.0f,  8.0f);
			v[1] = D3DVECTOR(0.0f, 0.0f, -8.0f);
		}

		pos = m_pMash[0]->RetPosition(0);
		for ( i=0 ; i<25 ; i++ )
		{
			pos += v[0];
			pObj = SearchObject(pos, 2.0f);
			if ( pObj == 0 || pObj->RetType() != m_mashType )  break;
			m_pMash[m_totalMash++] = pObj;
		}

		pos = m_pMash[0]->RetPosition(0);
		for ( i=0 ; i<25 ; i++ )
		{
			pos += v[1];
			pObj = SearchObject(pos, 2.0f);
			if ( pObj == 0 || pObj->RetType() != m_mashType )  break;
			m_pMash[m_totalMash++] = pObj;
		}
	}
}

// Fait trembler un objet rencontré par la sphère.

void CTaskRoll::ExploTremble(float progress)
{
	D3DVECTOR	speed;
	float		angle;
	int			i;

	if ( m_mashType == OBJECT_GLASS1 ||
		 m_mashType == OBJECT_GLASS2 )
	{
		angle = Norm((progress-0.5f)/0.3f)*(40.0f*PI/180.0f);
		speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
		if ( m_dir.x > 0.0f )
		{
			speed.x = angle;  // (*)
		}
		else if ( m_dir.x < 0.0f )
		{
			speed.x = -angle;  // (*)
		}
		else if ( m_dir.z > 0.0f )
		{
			speed.x = angle;
		}
		else
		{
			speed.x = -angle;
		}
		for ( i=0 ; i<m_totalMash ; i++ )
		{
			m_pMash[i]->SetCirVibration(speed);  // fait pencher les vitres
		}
	}
	else if ( m_mashType == OBJECT_GOAL )
	{
	}
	else
	{
		for ( i=0 ; i<m_totalMash ; i++ )
		{
			speed.x = (Rand()-0.5f)*progress*0.5f;
			speed.y = (Rand()-0.5f)*progress*0.5f;
			speed.z = (Rand()-0.5f)*progress*0.5f;
			m_pMash[i]->SetCirVibration(speed);
		}
	}
}

// (*)	Attention: Dans ce cas, la vitre est tournée de 90 degrés.
//		Il faut donc bien utiliser l'axe "x" !

// Fait exploser un objet rencontré par la sphère.

void CTaskRoll::ExploObject()
{
	CPyro*		pyro;
	PyroType	pt;
	D3DVECTOR	pos;
	int			i;

	pt = PT_FRAGT;
	if ( m_mashType == OBJECT_GLASS1 )  pt = PT_GLASS;
	if ( m_mashType == OBJECT_GLASS2 )  pt = PT_GLASS;
	if ( m_mashType == OBJECT_GOAL   )  pt = PT_GOAL;
	if ( m_mashType == OBJECT_BLUPI  )  pt = PT_EXPLOO;

	for ( i=0 ; i<m_totalMash ; i++ )
	{
		pos = m_pMash[i]->RetPosition(0);
		m_pMash[i]->SetDead(TRUE);  // pour le undo !
		pyro = new CPyro(m_iMan);
		pyro->Create(pt, m_pMash[i]);  // destruction objet
		m_pMash[i] = 0;

		if ( i == 0 )  // là où est la boule ?
		{
			m_terrain->SetLockZone(pos, LZ_BOXo);  // zone occupée !
		}
	}

	m_bMash = FALSE;
}


// Cherche une caisse proche.

CObject* CTaskRoll::SearchBox(D3DVECTOR center, float radius)
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

		type = pObj->RetType();
		if ( (type < OBJECT_BOX1 || type > OBJECT_BOX20) &&
			 (type < OBJECT_KEY1 || type > OBJECT_KEY5 ) )  continue;

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

// Cherche un objet quelconque.

CObject* CTaskRoll::SearchObject(D3DVECTOR center, float radius)
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

// Cherche un blupi.

CObject* CTaskRoll::SearchBlupi(D3DVECTOR center, float radius)
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
		if ( type != OBJECT_BLUPI )  continue;

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

// Cherche si une position est libre.

BOOL CTaskRoll::IsPosFree(D3DVECTOR center)
{
	LockZone	lz;

	lz = m_terrain->RetLockZone(center);

	if ( lz == LZ_FREE  ||
		 lz == LZ_MAX1X ||
		 lz == LZ_MINE  ||
		 lz == LZ_FIOLE ||
		 lz == LZ_GLASS ||
		 lz == LZ_PIPE  ||
		 lz == LZ_BLUPI )  return TRUE;

	return FALSE;
}

// Cherche si une position est un trou quelconque (TR_HOLE ou TR_SPACE).

BOOL CTaskRoll::IsHole(D3DVECTOR center)
{
	float		h;

	h = m_terrain->RetFloorLevel(center);
	return ( h < 0.0f );
}

// Cherche si une position est un trou infini (TR_SPACE).

BOOL CTaskRoll::IsSpace(D3DVECTOR center)
{
	float		h;

	h = m_terrain->RetFloorLevel(center);
	return ( h < -8.0f );
}

