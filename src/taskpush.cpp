// taskpush.cpp

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
#include "motiontrax.h"
#include "pyro.h"
#include "camera.h"
#include "sound.h"
#include "robotmain.h"
#include "tasklist.h"
#include "task.h"
#include "taskpush.h"



#define BOX_DIST	8.0f



// Constructeur de l'objet.

CTaskPush::CTaskPush(CInstanceManager* iMan, CObject* object)
					 : CTask(iMan, object)
{
	CTask::CTask(iMan, object);

	m_bFirst = TRUE;
	m_bLast = FALSE;
	m_bStop = FALSE;
	m_bMultiTiles = FALSE;
	m_phase = TPU_NULL;
	m_totalBox = 0;
	m_pBox[0]  = 0;
	m_pMine = 0;
	m_time = 0.0f;
	m_lastParticuleSmoke = 0.0f;
}

// Destructeur de l'objet.

CTaskPush::~CTaskPush()
{
}


// Gestion d'un événement.

BOOL CTaskPush::EventProcess(const Event &event)
{
	CPyro*		pyro;
	D3DVECTOR	pos, dir, speed, angle;
	FPOINT		dim;
	float		progress, softProgress, mass, duration, pp;
	int			i;

	if ( m_engine->RetPause() )  return TRUE;
	if ( event.event != EVENT_FRAME )  return TRUE;
	if ( m_bError )  return FALSE;

	m_time += event.rTime;
	m_progress += event.rTime*m_speed;
	progress = Norm(m_progress);
	softProgress = progress;

	if ( m_phase == TPU_ROLL )  // roule une sphère ?
	{
		ProgressAction(progress);
		if ( m_progress >= 0.75f && !m_bTask )
		{
			m_bTask = TRUE;
			pos = m_pBox[0]->RetPosition(0);
			pos = Grid(pos, BOX_DIST);
			dir = Grid(m_startPos, BOX_DIST);
			dir = pos-dir;
			m_pBox[0]->StartTaskList(TO_ROLL, dir, 0, 0, 0.0f);
			if ( m_type == OBJECT_BLUPI )
			{
				m_sound->Play(SOUND_BLUPIohhh, m_startPos, 1.0f, 1.0f);
			}
		}
		return TRUE;
	}

	if ( m_phase == TPU_PUSH )
	{
		if ( m_bRepeat || m_bMultiTiles)
		{
			if ( m_bFirst && progress >= 0.5f && !m_bMiddle )
			{
				m_bMiddle = TRUE;
				m_speed *= PI*0.5f;
			}
			if ( m_bFirst && !m_bMiddle )
			{
				softProgress = Soft(progress);
			}
			if ( m_bMultiTiles && m_bLast )
			{
				if ( progress >= 0.5f && !m_bMiddle )
				{
					m_bMiddle = TRUE;
					m_speed /= PI*0.5f;
				}
				if ( m_bMiddle )
				{
					softProgress = Soft(progress);
				}
			}
		}
		else
		{
			if ( m_bHole && progress >= 0.5f && !m_bMiddle )
			{
				m_bMiddle = TRUE;
				m_speed *= PI*0.5f;
			}
			if ( !m_bMiddle )
			{
				softProgress = Soft(progress);
			}
		}

		if ( m_bImpossible )
		{
			ProgressAction(progress);
		}
		else
		{
			// Bouge blupi.
			pos = LineProgress(m_startPos, m_goalPos, softProgress);
			m_object->SetPosition(0, pos);
			ProgressAction(softProgress);

			// Bouge les caisses.
			for ( i=0 ; i<m_totalBox ; i++ )
			{
				pos = LineProgress(RetBoxStartPos(i), RetBoxGoalPos(i), softProgress);

				if ( i == m_totalBox-1 && m_bHole )
				{
					if ( m_bSpace || m_bPipe )  // tombe dans trou infini ?
					{
						if ( m_bColomnFall )  // colonne ?
						{
							if ( progress > 0.7f )
							{
								pos.y -= powf((progress-0.7f)/0.3f, 2.0f)*PUSH_DIST*2.0f;

								if ( progress < 0.90f )
								{
									pp = (progress-0.70f)/0.20f;
									angle = RetAngleFreeFall(pp*(PI*0.25f*0.3f));
								}
								else
								{
									pp = Norm((progress-0.90f)/0.05f);
									angle = RetAngleFreeFall((1.0f-pp)*(PI*0.25f*0.3f));
								}
								m_pBox[i]->SetAngle(0, angle);
							}
						}
						else if ( m_bPipe )  // tombe sur tuyau ?
						{
							if ( progress > 0.7f )
							{
								pos.y -= powf((progress-0.7f)/0.3f, 2.0f)*2.0f;

								angle = RetAngleFreeFall(((progress-0.7f)/0.3f)*(PI*0.05f));
								m_pBox[i]->SetAngle(0, angle);
							}
						}
						else if ( m_bFreeFall )  // tombe en vrille ?
						{
							if ( progress > 0.7f )
							{
								if ( m_bWater )
								{
									pos.y += powf((progress-0.7f)/0.3f, 2.0f)*m_level;
								}
								else
								{
									pos.y -= powf((progress-0.7f)/0.3f, 2.0f)*PUSH_DIST*1.5f;
								}

								angle = RetAngleFreeFall(((progress-0.7f)/0.3f)*(PI*0.25f));
								m_pBox[i]->SetAngle(0, angle);
							}
						}
						else	// tombe droit ?
						{
							if ( progress > 0.7f )
							{
								if ( m_bWater )
								{
									pos.y += powf((progress-0.7f)/0.3f, 2.0f)*m_level;
								}
								else
								{
									pos.y -= powf((progress-0.7f)/0.3f, 2.0f)*PUSH_DIST*1.5f;
								}

								if ( progress < 0.90f )
								{
									pp = (progress-0.70f)/0.20f;
									angle = RetAngleFreeFall(pp*(PI*0.25f*0.1f));
								}
								else
								{
									pp = Norm((progress-0.90f)/0.05f);
									angle = RetAngleFreeFall((1.0f-pp)*(PI*0.25f*0.1f));
								}
								m_pBox[i]->SetAngle(0, angle);
							}
						}
					}
					else	// tombe dans trou fini ?
					{
						if ( m_bFreeFall )  // tombe en vrille ?
						{
							if ( progress < 0.7f )
							{
								angle = RetAngleFreeFall(0.0f);
							}
							else if ( progress < 0.9f )
							{
								pp = (progress-0.7f)/0.2f;
								pos.y -= pp*1.5f;
								angle = RetAngleFreeFall(pp*14.0f*PI/180.0f);
							}
							else
							{
								pp = (progress-0.9f)/0.1f;
								pos.y -= 1.5f+Bounce(pp, 0.7f, 0.2f)*6.5f;
//?								pos.y -= 1.5f+pp*6.5f;
								angle = RetAngleFreeFall((1.0f-pp)*14.0f*PI/180.0f);
							}
						}
						else
						{
							if ( progress < 0.7f )
							{
								angle = RetAngleFreeFall(0.0f);
							}
							else if ( progress < 0.9f )
							{
								pp = (progress-0.7f)/0.2f;
								pos.y -= pp*1.0f;
								angle = RetAngleFreeFall(pp*10.0f*PI/180.0f);
							}
							else
							{
								pp = (progress-0.9f)/0.1f;
								pos.y -= 1.0f+Bounce(pp, 0.7f, 0.2f)*7.0f;
//?								pos.y -= 1.0f+pp*7.0f;
								angle = RetAngleFreeFall((1.0f-pp)*14.0f*PI/180.0f);
							}
						}
						m_pBox[i]->SetAngle(0, angle);

						if ( progress > 0.95f && !m_bFallSound )
						{
							m_sound->Play(SOUND_FALLo2, RetBoxGoalPos(i), 1.0f);
							m_camera->StartEffect(CE_CRASH, m_goalPos, 0.5f);
							m_bFallSound = TRUE;
						}
					}
				}

				AdaptBox(i, pos);
				m_pBox[i]->SetPosition(0, pos);
			}

			// Bouge la mine.
			if ( m_bMine && !m_bMineDown )
			{
				speed.x = (Rand()-0.5f)*softProgress*0.1f;
				speed.y = (Rand()-0.5f)*softProgress*0.1f;
				speed.z = (Rand()-0.5f)*softProgress*0.1f;
				m_pBox[m_totalBox-1]->SetCirVibration(speed);

				if ( m_mineType == OBJECT_FIOLE ||
					 m_mineType == OBJECT_GLU   )
				{
					if ( softProgress < 0.4f )
					{
						softProgress = 0.0f;
					}
					else
					{
						softProgress = (softProgress-0.4f)/0.6f;
					}
				}
				pos = LineProgress(m_mineStartPos, m_mineGoalPos, softProgress);
				m_terrain->MoveOnFloor(pos);
				pos.y += m_pMine->RetCharacter()->height;
				m_pMine->SetPosition(0, pos);

				speed.x = Rand()*softProgress*0.4f;
				speed.z = Rand()*softProgress*0.4f;
				speed.y = 0.0f;
				m_pMine->SetLinVibration(speed);

				speed.x = (Rand()-0.5f)*softProgress*0.2f;
				speed.y = (Rand()-0.5f)*softProgress*0.2f;
				speed.z = (Rand()-0.5f)*softProgress*0.2f;
				m_pMine->SetCirVibration(speed);

				if ( m_lastParticuleSmoke+0.05f <= m_time )
				{
					m_lastParticuleSmoke = m_time;

					pos.y += 2.0f;
					pos.x += (Rand()-0.5f)*8.0f;
					pos.z += (Rand()-0.5f)*8.0f;
					speed.x = (Rand()-0.5f)*2.0f;
					speed.z = (Rand()-0.5f)*2.0f;
					speed.y = Rand()*8.0f;
					dim.x = 8.0f;
					dim.y = dim.x;
					m_particule->CreateParticule(pos, speed, dim, PARTISMOKE1, 3.0f);

					if ( m_mineType == OBJECT_MINE )
					{
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
			}

			if ( m_bGoal )
			{
				if ( m_progress >= 0.5f && m_pGoal != 0 )
				{
					pos = m_pGoal->RetPosition(0);
					pyro = new CPyro(m_iMan);
					pyro->Create(PT_GOAL, m_pGoal);  // destruction ballon
					m_pGoal = 0;
					m_terrain->SetLockZone(pos, LZ_BOX);  // zone occupée !
				}
			}
		}

		if ( m_progress >= 0.2f && !m_bMiddleSound && m_bFirst )
		{
			if ( m_type == OBJECT_BLUPI )
			{
//?				m_sound->Play(m_middleSound, m_object->RetPosition(0), 1.0f);
				m_sound->Play(m_middleSound, m_object->RetPosition(0), 1.0f, 0.9f+Rand()*0.2f);
			}
			m_bMiddleSound = TRUE;
		}

		if ( m_progress >= 0.6f && !m_bEndingJostle )
		{
			JostlePlant(RetBoxGoalPos(0));
			m_bEndingJostle = TRUE;
		}
	}

	return TRUE;
}


// Assigne le but à atteindre.

Error CTaskPush::Start(int part, int nbTiles)
{
	D3DMATRIX*	mat;
	D3DVECTOR	pos, delta;
	LockZone	lz;
	CObject*	pObj;
	ObjectType	type;
	int			i;

	if ( m_bFirst)  m_main->IncTotalManip(nbTiles);

	if ( nbTiles > 1 )  m_bMultiTiles = TRUE;

	m_part = part;
	m_nbTiles = nbTiles;
	m_bRepeat = (part >= 5 || nbTiles > 1);
	m_bMiddle = FALSE;
	m_bFreeFall = FALSE;
	m_bColomnFall = FALSE;
	m_bPull = FALSE;
	m_randFreeFall = NAN;
	m_type = m_object->RetType();
	m_mineType = OBJECT_NULL;

	m_level = m_water->RetLevel();
	m_bWater = TRUE;
	if ( m_terrain->RetModel() != 0 )  m_bWater = FALSE;

	if ( m_object->RetStrong() < 0.0f )  m_bPull = TRUE;

	if ( nbTiles == 1 && m_bMultiTiles )  m_bLast = TRUE;

	m_maxBox = 2;
	if ( m_object->RetStrong() > 0.0f )  // blupi hyper costaud ?
	{
		m_maxBox = 20;
		m_bRepeat = FALSE;
	}
	if ( m_object->RetStrong() < 0.0f )  // blupi englué ?
	{
		m_maxBox = 1;
		m_bRepeat = FALSE;
	}

	m_startPos = m_object->RetPosition(0);

	// Si blupi est dans un tunnel, interdit de pousser une caisse
	// située à côté du tunnel (perpendiculairement au tunnel) !
	lz = m_terrain->RetLockZone(m_startPos, TRUE);
	if ( lz == LZ_TUNNELh && part%2 == 0 )
	{
		m_bError = TRUE;
		return ERR_GENERIC;
	}
	if ( lz == LZ_TUNNELv && part%2 == 1 )
	{
		m_bError = TRUE;
		return ERR_GENERIC;
	}

	// Cherche combien on peut pousser de caisses.
	if ( m_bFirst )
	{
		m_totalBox = 0;
		pos = m_object->RetPosition(0);
		mat = m_object->RetWorldMatrix(0);
		delta = Transform(*mat, D3DVECTOR(PUSH_DIST, 0.0f, 0.0f))-pos;
		delta = Grid(delta, PUSH_DIST);
		for ( i=0 ; i<m_maxBox ; i++ )
		{
			pos += delta;
			m_pBox[i] = SearchBox(pos, 4.0f);
			if ( m_pBox[i] == 0 )  break;
			m_totalBox ++;
		}

		if ( m_pBox[0] == 0 )
		{
			m_bError = TRUE;
			return ERR_GENERIC;
		}
	}

	m_boxStartPos = m_pBox[0]->RetPosition(0);
	m_boxGoalPos = m_boxStartPos;
	m_goalPos = m_boxStartPos;

	if ( m_bPull )  // tire ?
	{
		if ( Abs(m_boxStartPos.x-m_startPos.x) > Abs(m_boxStartPos.z-m_startPos.z) )
		{
			if ( m_boxStartPos.x > m_startPos.x )
			{
				m_boxGoalPos.x -= BOX_DIST;
				m_goalPos.x -= BOX_DIST*2.0f;
			}
			else
			{
				m_boxGoalPos.x += BOX_DIST;
				m_goalPos.x += BOX_DIST*2.0f;
			}
		}
		else
		{
			if ( m_boxStartPos.z > m_startPos.z )
			{
				m_boxGoalPos.z -= BOX_DIST;
				m_goalPos.z -= BOX_DIST*2.0f;
			}
			else
			{
				m_boxGoalPos.z += BOX_DIST;
				m_goalPos.z += BOX_DIST*2.0f;
			}
		}
	}
	else	// pousse ?
	{
		if ( Abs(m_boxStartPos.x-m_startPos.x) > Abs(m_boxStartPos.z-m_startPos.z) )
		{
			if ( m_boxStartPos.x > m_startPos.x )
			{
				m_boxGoalPos.x += BOX_DIST;
			}
			else
			{
				m_boxGoalPos.x -= BOX_DIST;
			}
		}
		else
		{
			if ( m_boxStartPos.z > m_startPos.z )
			{
				m_boxGoalPos.z += BOX_DIST;
			}
			else
			{
				m_boxGoalPos.z -= BOX_DIST;
			}
		}
	}
	m_goalPos.y = 0.0f+m_object->RetCharacter()->height;

	if ( (m_pBox[0]->RetType() == OBJECT_BOX7 ||    // sphère ?
		  m_pBox[0]->RetType() == OBJECT_BOX13) &&  // cca ?
		 m_totalBox == 1 )
	{
		if ( !IsPosFree(m_boxGoalPos) )
		{
			pObj = SearchObject(m_boxGoalPos, 4.0f);
			type = OBJECT_NULL;
			if ( pObj != 0 )  type = pObj->RetType();
			if ( type != OBJECT_GLASS1 &&
				 type != OBJECT_GLASS2 )
			{
				m_bError = TRUE;
				return ERR_GENERIC;
			}
		}

		StartAction(MBLUPI_ROLL);
		ProgressAction(0.0f);
		m_object->SetStrong(0.0f);

		m_bTask       = FALSE;
		m_bHole       = FALSE;
		m_bSpace      = FALSE;
		m_bPipe       = FALSE;
		m_bBoum       = FALSE;
		m_bMine       = FALSE;
		m_bMineDown   = FALSE;
		m_bGoal       = FALSE;
		m_bImpossible = FALSE;

		m_phase = TPU_ROLL;
		m_progress = 0.0f;
		m_speed = 1.0f/1.0f;
		m_bError = FALSE;
		return ERR_OK;
	}

	if ( m_pBox[0]->RetType() == OBJECT_BOX10 &&  // bombe ?
		 m_totalBox == 1 )
	{
		if ( !IsPosFree(m_boxGoalPos) )
		{
			pObj = SearchObject(m_boxGoalPos, 4.0f);
			type = OBJECT_NULL;
			if ( pObj != 0 )  type = pObj->RetType();
			if ( type < OBJECT_BOX1  ||
				 type > OBJECT_BOX20 )
			{
				m_bError = TRUE;
				return ERR_GENERIC;
			}
		}

		StartAction(MBLUPI_ROLL);
		ProgressAction(0.0f);
		m_object->SetStrong(0.0f);

		m_bTask       = FALSE;
		m_bHole       = FALSE;
		m_bSpace      = FALSE;
		m_bPipe       = FALSE;
		m_bBoum       = FALSE;
		m_bMine       = FALSE;
		m_bMineDown   = FALSE;
		m_bGoal       = FALSE;
		m_bImpossible = FALSE;

		m_phase = TPU_ROLL;
		m_progress = 0.0f;
		m_speed = 1.0f/1.0f;
		m_bError = FALSE;
		return ERR_OK;
	}

	if ( m_bPull )
	{
		m_bImpossible = ( SearchObject(m_goalPos, 4.0f) != 0 ||
						  !m_terrain->IsSolid(m_goalPos) );
	}
	else
	{
		m_bImpossible = !IsPosFree(RetBoxGoalPos(m_totalBox-1));
	}

	// Vérifie si les caisses à pousser sont compatibles avec le pousseur.
	if ( !m_bImpossible && !IsPusherAdhoc() )
	{
		m_bImpossible = TRUE;
	}

	m_bHole  = FALSE;
	m_bSpace = FALSE;
	m_bPipe  = FALSE;
	m_bHole  = IsHole(RetBoxGoalPos(m_totalBox-1));
	m_bSpace = IsSpace(RetBoxGoalPos(m_totalBox-1));
	m_bBoum  = FALSE;
	type = m_pBox[m_totalBox-1]->RetType();
	if ( type < OBJECT_BOX1 || type > OBJECT_BOX20 ||  // objet fragile ?
		 type == OBJECT_BOX7  ||
		 type == OBJECT_BOX10 )
	{
		if ( m_bHole )  m_bBoum = TRUE;
	}
	m_bFreeFall = IsHole(RetBoxGoalPos(m_totalBox));
	if ( type == OBJECT_BOX8 && m_bHole )  // colonne tombe dans trou ?
	{
		m_bColomnFall = TRUE;
		m_bFreeFall = FALSE;
	}

	m_bMine     = FALSE;
	m_bMineDown = FALSE;
	m_pMine = SearchObject(RetBoxGoalPos(m_totalBox-1), 4.0f);
	if ( m_pMine != 0 && !m_bImpossible )
	{
		m_mineType = m_pMine->RetType();
		if ( m_mineType == OBJECT_MINE  ||
			 m_mineType == OBJECT_FIOLE ||
			 m_mineType == OBJECT_GLU   )
		{
			m_mineStartPos = RetBoxStartPos(m_totalBox);
			m_mineGoalPos = m_mineStartPos+(m_boxGoalPos-m_boxStartPos)*0.49f;  // (*)
			m_pMine->SetLock(TRUE);
			m_bMine = TRUE;

			pos = m_pMine->RetPosition(0);
			if ( pos.y < 0.0f )  m_bMineDown = TRUE;
		}
		if ( m_mineType == OBJECT_BARRIER29 ||  // tuyaux ?
			 m_mineType == OBJECT_BARRIER39 ||  // tuyaux ?
			 m_mineType == OBJECT_BARRIER48 ||  // tuyaux ?
			 m_mineType == OBJECT_BARRIER49 ||  // tuyaux ?
			 m_mineType == OBJECT_BARRIER67 )   // tuyaux ?
		{
			m_bPipe  = TRUE;
			m_bWater = FALSE;
		}
	}

	if ( m_bHole || m_bSpace || m_bPipe || m_bMine )  m_bLast = FALSE;

	m_bGoal = FALSE;
	m_pGoal = SearchObject(RetBoxGoalPos(m_totalBox-1), 4.0f);
	if ( m_pGoal != 0 && m_pGoal->RetType() == OBJECT_GOAL )
	{
		m_pGoal->SetLock(TRUE);
		m_bGoal = TRUE;
	}

	if ( m_bMine )
	{
//?		m_sound->Play(SOUND_FALLo2, m_boxStartPos, 1.0f, 0.50f);
		m_sound->Play(SOUND_CHOCo, m_boxStartPos, 1.0f, 0.30f);
		m_middleSound = SOUND_BLUPIshibi;
	}
	else if ( m_bImpossible )
	{
		m_sound->Play(SOUND_CHOCa, m_boxStartPos, 1.0f, 0.50f);
		m_middleSound = SOUND_BLUPIpousse;
	}
	else
	{
		m_sound->Play(SOUND_WHEELg, m_boxStartPos, 0.5f, 0.69f);
		m_middleSound = rand()%2==0?SOUND_BLUPIeffort:SOUND_BLUPIpousse;
	}
	m_bMiddleSound = FALSE;
	m_bFallSound = FALSE;
	m_bEndingJostle = FALSE;

	for ( i=0 ; i<m_totalBox ; i++ )
	{
		m_pBox[i]->SetLock(TRUE);
	}

	if ( m_bFirst )
	{
		StartAction(MBLUPI_PUSH);
		StartAction(MBLUPI_MOUTH+MBLUPI_SAD);  // triste
		ProgressAction(0.0f);
	}

	if ( m_bHole )
	{
		m_pBox[m_totalBox-1]->SetAdjustShadow(FALSE);
	}

	JostlePlant(RetBoxStartPos(0));
	SetLockZone(TRUE);

	m_phase = TPU_PUSH;
	m_progress = 0.0f;
	m_speed = 1.0f/2.0f;
	if ( !m_bFirst )  m_speed *= PI*0.5f;
	m_bError = FALSE;
	return ERR_OK;
}

// (*)	< 0.5 pour que m_terrain->SetZoneLock() libère toujours
//		la case initiale où est la mine !


// Indique si l'action est terminée.

Error CTaskPush::IsEnded()
{
	CPyro*		pyro;
	CTaskList*	taskList;
	int			i;

	if ( m_engine->RetPause() )  return ERR_CONTINUE;

	if ( m_bError )
	{
		return ERR_STOP;
	}

	if ( m_progress < 1.0f )  // phase pas finie ?
	{
		return ERR_CONTINUE;
	}

	if ( m_phase == TPU_ROLL )  // roule une sphère ?
	{
		if ( m_type == OBJECT_BLUPI )
		{
			m_sound->Play(SOUND_BLUPIpfiou, m_object->RetPosition(0), 1.0f);
			StartAction(MBLUPI_PFIOU);  // pfiouuuu...
		}
		return ERR_STOP;
	}

	SetLockZone(FALSE);

	if ( m_phase == TPU_PUSH )
	{
		m_object->SetStrong(0.0f);

		if ( m_bMine )  // arrivée sur une mine ?
		{
			pyro = new CPyro(m_iMan);
			pyro->Create(PT_FRAGT, m_pMine);  // destruction mine

			i = m_totalBox-1;
			m_pBox[i]->DeletePart(1);  // supprime les flèches
			m_pBox[i]->DeletePart(2);
			m_pBox[i]->DeletePart(3);
			m_pBox[i]->DeletePart(4);
			pyro = new CPyro(m_iMan);
			pyro->Create(PT_FRAGT, m_pBox[i]);  // destruction caisse
			m_pBox[i] = 0;

			if ( m_type == OBJECT_BLUPI )
			{
				m_sound->Play(SOUND_BLUPIouaou, m_object->RetPosition(0), 1.0f);
				StartAction(MBLUPI_AMAZE);  // étonné
			}
		}
		else if ( m_bImpossible )  // impossible de pousser ?
		{
			StartAction(MBLUPI_ERROR);
		}
		else if ( m_bHole )  // arrivée dans un trou ?
		{
			if ( m_bColomnFall )  // colonne ?
			{
				if ( m_bSpace )
				{
					m_terrain->SetResource(RetBoxGoalPos(m_totalBox-1), TR_BOX);
					i = m_totalBox-1;
					m_pBox[i]->SetDead(TRUE);
					m_pBox[i]->TerrainEmbedded();  // incruste dans le terrain
					m_pBox[i] = 0;
				}
				else
				{
					m_terrain->SetResource(RetBoxGoalPos(m_totalBox-1), TR_BOX);
					i = m_totalBox-1;
					m_pBox[i]->SetType(OBJECT_FIX);
				}
			}
			else if ( m_bSpace || m_bPipe )  // dans un trou infini ?
			{
				if ( m_type == OBJECT_BLUPI )
				{
					m_sound->Play(SOUND_BLUPIohhh, m_object->RetPosition(0), 1.0f);
					StartAction(MBLUPI_WALK);
					StartAction(MBLUPI_AMAZE);  // étonné
				}

				for ( i=0 ; i<m_totalBox ; i++ )
				{
					if ( m_pBox[i] != 0 )  // caisse existe encore ?
					{
						m_pBox[i]->SetLock(FALSE);  // de nouveau utilisable
					}
				}

				i = m_totalBox-1;
				m_pBox[i]->DeletePart(1);  // supprime les flèches
				m_pBox[i]->DeletePart(2);
				m_pBox[i]->DeletePart(3);
				m_pBox[i]->DeletePart(4);
				pyro = new CPyro(m_iMan);
				pyro->Create(m_bWater?PT_PLOUF:PT_FRAGT, m_pBox[i], 0.4f);  // destruction caisse
				m_pBox[i] = 0;
				return ERR_STOP;
			}
			else
			{
				if ( m_bBoum )  // objet fragile ?
				{
					i = m_totalBox-1;
					m_pBox[i]->DeletePart(1);  // supprime les flèches
					m_pBox[i]->DeletePart(2);
					m_pBox[i]->DeletePart(3);
					m_pBox[i]->DeletePart(4);
					pyro = new CPyro(m_iMan);
					pyro->Create(PT_FRAGT, m_pBox[i], 0.6f);  // destruction caisse
					m_pBox[i] = 0;

					if ( m_type == OBJECT_BLUPI )
					{
						m_sound->Play(SOUND_BLUPIgrrr, m_object->RetPosition(0), 1.0f);
						StartAction(MBLUPI_AMAZE);  // étonné
					}
				}
				else	// caisse normale ?
				{
					m_terrain->SetResource(RetBoxGoalPos(m_totalBox-1), TR_BOX);
					i = m_totalBox-1;
					m_pBox[i]->SetDead(TRUE);
					m_pBox[i]->TerrainEmbedded();  // incruste dans le terrain
					m_pBox[i] = 0;
				}
			}
		}
		else	// arrivée sur sol plat ?
		{
			if ( m_bRepeat && !m_bStop )
			{
				m_bFirst = FALSE;
				Start(m_part, m_nbTiles-1);  // on recommence
				return ERR_CONTINUE;
			}
			else
			{
				taskList = m_object->RetTaskList();
				if ( taskList != 0 && !taskList->IsOtherTask() &&
					 m_type == OBJECT_BLUPI )
				{
//?					m_sound->Play(rand()%2==0?SOUND_BLUPIpfiou:SOUND_BLUPIhhuu, m_object->RetPosition(0), 1.0f);
					m_sound->Play(rand()%2==0?SOUND_BLUPIpfiou:SOUND_BLUPIhhuu, m_object->RetPosition(0), 1.0f, 0.9f+Rand()*0.2f);
					StartAction(MBLUPI_PFIOU);  // pfiouuuu...
				}
			}
		}

		for ( i=0 ; i<m_totalBox ; i++ )
		{
			if ( m_pBox[i] != 0 )  // caisse existe encore ?
			{
				m_pBox[i]->SetLock(FALSE);  // de nouveau utilisable
			}
		}
		StartAction(MBLUPI_WALK);
		StartAction(MBLUPI_MOUTH+MBLUPI_HAPPY);
		return ERR_STOP;
	}

	return ERR_STOP;
}

// Indique si la tache en cours est stoppable.

BOOL CTaskPush::IsStopable()
{
	return m_bRepeat;
}

// Stoppe proprement la tache en cours.

BOOL CTaskPush::Stop()
{
	m_bStop = TRUE;
	return TRUE;
}


// Vérifie si les caisses à pousser sont compatibles avec le pousseur.
 
BOOL CTaskPush::IsPusherAdhoc()
{
	ObjectType	type;
	int			totalLittle, totalLight, totalBig, i;

	totalLittle = 0;
	totalLight  = 0;
	totalBig    = 0;

	for ( i=0 ; i<m_totalBox ; i++ )
	{
		type = m_pBox[i]->RetType();

		if ( type == OBJECT_BOX8 )  // colonne ?
		{
			totalBig ++;
		}
		else if ( type == OBJECT_BOX9 )  // caisse light ?
		{
			totalLight ++;
		}
		else if ( type == OBJECT_BOX11 )  // caisse x-x ?
		{
			if ( m_part%2 == 0 )  return FALSE;
			totalLight ++;
		}
		else if ( type == OBJECT_BOX12 )  // caisse z-z ?
		{
			if ( m_part%2 == 1 )  return FALSE;
			totalLight ++;
		}
		else	// caisse normale ?
		{
			totalLittle ++;
		}
	}

	if ( m_type == OBJECT_BLUPI )
	{
		if ( m_object->RetStrong() == 0.0f )  // blupi normalement fort ?
		{
			if ( totalBig != 0 )  return FALSE;
			if ( totalLittle > 1 )  return FALSE;
			if ( totalLight > 2 )  return FALSE;
			if ( totalLight != 0 && totalLittle != 0 )  return FALSE;
		}
		else	// blupi hyper costaud ?
		{
			if ( totalBig != 0 )  return FALSE;
		}
		return TRUE;
	}

	if ( m_type == OBJECT_TRAX )
	{
		if ( totalBig > 1 )  return FALSE;
		if ( totalBig != 0 && totalLittle != 0 )  return FALSE;
		return TRUE;
	}

	return FALSE;
}


// Bloque toute la zone des opérations.

void CTaskPush::SetLockZone(BOOL bLock)
{
	D3DVECTOR	pos;

	if ( m_bImpossible )  return;

	if ( m_bPull )  // tire ?
	{
		if ( bLock )
		{
			m_terrain->SetLockZone(m_goalPos, LZ_BLUPI);
		}
		else
		{
			m_terrain->SetLockZone(m_boxStartPos, LZ_FREE);
			m_terrain->SetLockZone(m_startPos, LZ_BOX);
		}
	}
	else	// pousse ?
	{
		if ( bLock )
		{
			pos = RetBoxGoalPos(m_totalBox-1);
			m_terrain->SetLockZone(pos, LZ_BOX);
		}
		else
		{
			m_terrain->SetLockZone(m_startPos, LZ_FREE);

			if ( m_type == OBJECT_TRAX )
			{
				m_terrain->SetLockZone(m_goalPos, LZ_BOT);
			}
			else
			{
				m_terrain->SetLockZone(m_goalPos, LZ_BLUPI);
			}
		}
	}
}


// Retourne la position de start d'une caisse.

D3DVECTOR CTaskPush::RetBoxStartPos(int rank)
{
	return m_boxStartPos+(m_boxGoalPos-m_boxStartPos)*(float)rank;
}

// Retourne la position d'arrivée d'une caisse.

D3DVECTOR CTaskPush::RetBoxGoalPos(int rank)
{
	return m_boxGoalPos+(m_boxGoalPos-m_boxStartPos)*(float)rank;
}


// Cherche une caisse proche.

CObject* CTaskPush::SearchBox(D3DVECTOR center, float radius)
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
		if ( pObj->RetExplo() )  continue;
		if ( pObj->RetLock() )  continue;  // déjà stoppé ?
		if ( !pObj->RetEnable() )  continue;

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

CObject* CTaskPush::SearchObject(D3DVECTOR center, float radius)
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

// Fait bouger les plantes.

void CTaskPush::JostlePlant(D3DVECTOR center)
{
	CObject*	pObj;
	D3DVECTOR	pos;
	ObjectType	type;
	float		dist;
	int			i;

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( pObj->RetExplo() )  continue;
		if ( pObj->RetLock() )  continue;  // déjà stoppé ?

		type = pObj->RetType();

		if ( type >= OBJECT_PLANT5 &&
			 type <= OBJECT_PLANT9 )
		{
			pos = pObj->RetPosition(0);
			dist = Length2d(pos, center);

			if ( dist <= 5.0f )
			{
				pObj->JostleObject(1.0f);
			}
		}

		if ( (type >= OBJECT_PLANT0  &&
			  type <= OBJECT_PLANT4  ) ||
			 (type >= OBJECT_PLANT10 &&
			  type <= OBJECT_PLANT19 ) )
		{
			pos = pObj->RetPosition(0);
			if ( pos.y < 0.0f )  continue;
			dist = Length2d(pos, center);

			if ( dist <= 10.0f )
			{
				pObj->JostleObject(1.0f);
			}
		}
	}
}


// Cherche si une position est libre.

BOOL CTaskPush::IsPosFree(D3DVECTOR center)
{
	LockZone	lz;

	lz = m_terrain->RetLockZone(center);

	if ( lz == LZ_FREE  ||
		 lz == LZ_MAX1X ||
		 lz == LZ_MINE  ||
		 lz == LZ_FIOLE ||
		 lz == LZ_PIPE  )  return TRUE;

	return FALSE;
}

// Cherche si une position est un trou quelconque (TR_HOLE ou TR_SPACE).

BOOL CTaskPush::IsHole(D3DVECTOR center)
{
	float		h;

	h = m_terrain->RetFloorLevel(center);
	return ( h < 0.0f );
}

// Cherche si une position est un trou infini (TR_SPACE).

BOOL CTaskPush::IsSpace(D3DVECTOR center)
{
	float		h;

	h = m_terrain->RetFloorLevel(center);
	return ( h < -8.0f );
}


// Calcule l'angle d'une caisse en chute libre.

D3DVECTOR CTaskPush::RetAngleFreeFall(float angle)
{
	D3DVECTOR	angle3D, pos;

	angle3D = D3DVECTOR(0.0f, 0.0f, 0.0f);

	// Si nécessaire, calcule l'amplitude de la rotation
	// latérale de la caisse.
	if ( m_randFreeFall == NAN )
	{
		m_randFreeFall = 0.0f;
		pos = RetBoxGoalPos(m_totalBox-1);

		if ( m_part%2 == 1 )  // flèche ouest/est ?
		{
			if ( rand()%2 == 0 )
			{
				pos.z += PUSH_DIST;
				if ( IsSpace(pos) )  m_randFreeFall = 1.0f;
				pos.z -= PUSH_DIST*2.0f;
				if ( IsSpace(pos) )  m_randFreeFall = -1.0f;
			}
			else
			{
				pos.z -= PUSH_DIST;
				if ( IsSpace(pos) )  m_randFreeFall = -1.0f;
				pos.z += PUSH_DIST*2.0f;
				if ( IsSpace(pos) )  m_randFreeFall = 1.0f;
			}
		}
		else	// flèche nord/sud ?
		{
			if ( rand()%2 == 0 )
			{
				pos.x += PUSH_DIST;
				if ( IsSpace(pos) )  m_randFreeFall = -1.0f;
				pos.x -= PUSH_DIST*2.0f;
				if ( IsSpace(pos) )  m_randFreeFall = 1.0f;
			}
			else
			{
				pos.x -= PUSH_DIST;
				if ( IsSpace(pos) )  m_randFreeFall = 1.0f;
				pos.x += PUSH_DIST*2.0f;
				if ( IsSpace(pos) )  m_randFreeFall = -1.0f;
			}
		}
	}

	// Calcule l'angle (x;z) de la caisse selon le
	// sens dans lequel elle est poussée.
	if ( m_part == 1 || m_part == 5 )  // flèche ouest ?
	{
		angle3D.z = -angle;
		angle3D.x =  angle*0.2f*m_randFreeFall;
	}

	if ( m_part == 2 || m_part == 6 )  // flèche nord ?
	{
		angle3D.x = -angle;
		angle3D.z =  angle*0.2f*m_randFreeFall;
	}

	if ( m_part == 3 || m_part == 7 )  // flèche est ?
	{
		angle3D.z = angle;
		angle3D.x = angle*0.2f*m_randFreeFall;
	}

	if ( m_part == 4 || m_part == 8 )  // flèche sud ?
	{
		angle3D.x = angle;
		angle3D.z = angle*0.2f*m_randFreeFall;
	}

	return angle3D;
}

// Adapte une caisse en mouvement (fait tourner les roues).

void CTaskPush::AdaptBox(int i, const D3DVECTOR &pos)
{
	CObject*	pBox;
	ObjectType	type;
	float		dist, angle;

	pBox = m_pBox[i];
	type = pBox->RetType();

	if ( type == OBJECT_BOX11 )  // caisse x-x ?
	{
		dist = Length2d(pos, RetBoxStartPos(i));
		if ( m_part == 1 || m_part == 5 )  dist = -dist;
		angle = dist/8.0f*PI*2.0f;
		pBox->SetAngleZ(10, angle);
		pBox->SetAngleZ(11, angle);
		pBox->SetAngleZ(12, angle);
		pBox->SetAngleZ(13, angle);  // tourne les 4 roues
	}

	if ( type == OBJECT_BOX12 )  // caisse z-z ?
	{
		dist = Length2d(pos, RetBoxStartPos(i));
		if ( m_part == 2 || m_part == 6 )  dist = -dist;
		angle = dist/8.0f*PI*2.0f;
		pBox->SetAngleX(10, angle);
		pBox->SetAngleX(11, angle);
		pBox->SetAngleX(12, angle);
		pBox->SetAngleX(13, angle);  // tourne les 4 roues
	}
}


// Démarre une action pour Blupi.

void CTaskPush::StartAction(int action, float speed)
{
	CMotion*	motion;

	motion = m_object->RetMotion();
	if ( motion == 0 )  return;

	if ( m_type == OBJECT_BLUPI )
	{
		motion->SetAction(action, speed);
	}

	if ( m_type == OBJECT_TRAX )
	{
		if ( action == MBLUPI_PUSH )
		{
			ProgressLinSpeed(1.4f);  // moteur
			ProgressCirSpeed(0.0f);
		}
		if ( action == MBLUPI_WALK )
		{
			ProgressLinSpeed(0.0f);  // stoppe
			ProgressCirSpeed(0.0f);
		}
		if ( action == MBLUPI_ROLL )
		{
			motion->SetAction(MTRAX_ROLL);
		}
	}
}

// Spécifie la progression pour une action pour Blupi.

void CTaskPush::ProgressAction(float progress)
{
	CMotionBlupi*	motion;

	motion = (CMotionBlupi*)m_object->RetMotion();
	if ( motion == 0 )  return;

	motion->SetActionProgress(progress);
}

// Spécifie la vitesse linéaire pour une action pour Blupi.

void CTaskPush::ProgressLinSpeed(float speed)
{
	CMotion*	motion;

	motion = m_object->RetMotion();
	if ( motion == 0 )  return;

	motion->SetActionLinSpeed(speed);
}

// Spécifie la vitesse circulaire pour une action pour Blupi.

void CTaskPush::ProgressCirSpeed(float speed)
{
	CMotion*	motion;

	motion = m_object->RetMotion();
	if ( motion == 0 )  return;

	motion->SetActionCirSpeed(speed);
}

