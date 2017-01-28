// taskdive.cpp

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
#include "motion.h"
#include "motionblupi.h"
#include "motionbot.h"
#include "auto.h"
#include "autogoal.h"
#include "particule.h"
#include "pyro.h"
#include "robotmain.h"
#include "sound.h"
#include "task.h"
#include "tasklist.h"
#include "taskdive.h"



#define DIVE_HEIGHT		3.7f



// Constructeur de l'objet.

CTaskDive::CTaskDive(CInstanceManager* iMan, CObject* object)
					 : CTask(iMan, object)
{
	CTask::CTask(iMan, object);

	m_time = 0.0f;
	m_bError = TRUE;
}

// Destructeur de l'objet.

CTaskDive::~CTaskDive()
{
}


// Gestion d'un événement.

BOOL CTaskDive::EventProcess(const Event &event)
{
	D3DVECTOR	pos, speed, linVib;
	FPOINT		dim, rot;
	float		progress, angle, force, duration;
	int			i;

	if ( m_engine->RetPause() )  return TRUE;
	if ( event.event != EVENT_FRAME )  return TRUE;
	if ( m_bError )  return FALSE;

	m_time += event.rTime;
	m_progress += event.rTime*m_speed;
	progress = Norm(m_progress);

	if ( m_phase == TDI_MARCH )
	{
		pos = m_startPos + (m_goalPos-m_startPos)*progress;
		pos.y = m_startPos.y;

		if ( progress < 0.05f )
		{
			pos.y += 0.0f;
		}
		else if ( progress < 0.20f )
		{
			progress = (progress-0.05f)/0.15f;
			pos.y += DIVE_HEIGHT*progress;
		}
		else
		{
			pos.y += DIVE_HEIGHT;
		}

		m_object->SetPosition(0, pos);
	}

	if ( m_phase == TDI_TRY )
	{
	}

	if ( m_phase == TDI_NONO )
	{
	}

	if ( m_phase == TDI_TURN )
	{
		angle = m_startAngle+PI*progress;
		m_object->SetAngleY(0, angle);
	}

	if ( m_phase == TDI_BACK )
	{
		pos = m_goalPos + (m_startPos-m_goalPos)*progress;
		pos.y = m_startPos.y;

		if ( progress < 0.80f )
		{
			pos.y += DIVE_HEIGHT;
		}
		else if ( progress < 0.95f )
		{
			progress = (progress-0.80f)/0.15f;
			pos.y += DIVE_HEIGHT*(1.0f-progress);
		}
		else
		{
			pos.y += 0.0f;
		}

		m_object->SetPosition(0, pos);
	}

	if ( m_phase == TDI_END )
	{
		pos = m_startPos + (m_oupsPos-m_startPos)*progress;
		pos.y = m_startPos.y;

		if ( progress < 0.05f )
		{
			pos.y += 0.0f;
		}
		else if ( progress < 0.20f )
		{
			progress = (progress-0.05f)/0.15f;
			pos.y += DIVE_HEIGHT*progress;
		}
		else
		{
			pos.y += DIVE_HEIGHT;
		}

		m_object->SetPosition(0, pos);
	}

	if ( m_phase == TDI_OUPS )
	{
	}

	if ( m_phase == TDI_DIVE )
	{
		pos = m_oupsPos + (m_divePos-m_oupsPos)*progress;
		pos.y += -powf(progress, 2.0f)*40.0f;
		m_object->SetPosition(0, pos);

		angle = progress*PI;
		m_object->SetAngleZ(0, -angle);

		if ( pos.y < m_level+4.0f && m_bPlouf )
		{
			m_bPlouf = FALSE;

			pos.y = m_level-1.0f;
			dim.x = 8.0f;  // hauteur
			dim.y = 4.0f;  // diamètre
			duration = 1.4f;
			m_particule->CreateParticule(pos, D3DVECTOR(0.0f, 0.0f, 0.0f), dim, PARTIPLOUF0, duration, 0.0f);

			for ( i=0 ; i<5 ; i++ )
			{
				pos = m_divePos;
				pos.x += (Rand()-0.5f)*8.0f;
				pos.z += (Rand()-0.5f)*8.0f;
				pos.y = m_level-1.0f;
				force = 1.0f+Rand()*1.0f;
				dim.x = 1.0f*force;  // hauteur
				dim.y = 0.5f*force;  // diamètre
				duration = 0.8f+Rand()*1.0f;
				m_particule->CreateParticule(pos, D3DVECTOR(0.0f, 0.0f, 0.0f), dim, PARTIPLOUF0, duration, 0.0f);

				pos = m_divePos;
				pos.x += (Rand()-0.5f)*12.0f;
				pos.z += (Rand()-0.5f)*12.0f;
				pos.y = m_level+2.0f;
				dim.x = 4.0f+Rand()*4.0f;
				dim.y = dim.x;
				duration = 2.0f+Rand()*2.0f;
				m_particule->CreateParticule(pos, D3DVECTOR(0.0f, 0.0f, 0.0f), dim, PARTIFLIC, duration, 0.0f);
			}

			m_sound->Play(SOUND_PLOUF, m_divePos);
		}
	}

	if ( m_phase == TDI_WAIT )
	{
		pos = m_divePos + (m_oupsPos-m_divePos)*progress;
		pos.y = -40.0f;
		m_object->SetPosition(0, pos);
	}

	if ( m_phase == TDI_UP )
	{
		pos = m_oupsPos;
		pos.y += -40.0f+powf(progress, 0.5f)*(40.0f-7.2f);
		m_object->SetPosition(0, pos);

		if ( pos.y > m_level+4.0f && m_bPlouf )
		{
			m_bPlouf = FALSE;

			for ( i=0 ; i<5 ; i++ )
			{
				pos = m_oupsPos;
				pos.x += (Rand()-0.5f)*6.0f;
				pos.z += (Rand()-0.5f)*6.0f;
				pos.y = m_level+2.0f;
				dim.x = 4.0f+Rand()*4.0f;
				dim.y = dim.x;
				duration = 2.0f+Rand()*2.0f;
				m_particule->CreateParticule(pos, D3DVECTOR(0.0f, 0.0f, 0.0f), dim, PARTIFLIC, duration, 0.0f);
			}
		}
	}

	if ( m_phase == TDI_OSCIL )
	{
		angle = sinf(progress*PI*6.0f)*progress;
		m_object->SetAngleX(0, angle);

		pos = m_oupsPos;
		pos.y += -7.2f;

		rot.x = sinf(angle)*6.0f;
		rot.y = 0.0f;
		rot = RotatePoint(-(m_object->RetAngleY(0)-PI*1.5f), rot);
		linVib.x = rot.x;
		linVib.z = rot.y;
		linVib.y = 6.0f-cosf(angle)*6.0f;

		m_object->SetPosition(0, pos+linVib);
	}

	if ( m_phase == TDI_JUMP )
	{
		angle = NormAngle(progress*PI*2.0f);
		m_object->SetAngleX(0, angle);

		pos = m_oupsPos + (m_goalPos-m_oupsPos)*progress;
		pos.y += -7.2f+progress*7.2f;

		rot.x = sinf(angle)*6.0f;
		rot.y = 0.0f;
		rot = RotatePoint(-(m_object->RetAngleY(0)-PI*1.5f), rot);
		linVib.x = rot.x;
		linVib.z = rot.y;
		linVib.y = 6.0f-cosf(angle)*6.0f;

		m_object->SetPosition(0, pos+linVib);
	}

	if ( m_phase == TDI_OH )
	{
	}

	return TRUE;
}


// Assigne le but à atteindre.

Error CTaskDive::Start(CObject *dive)
{
	D3DMATRIX*	mat;
	BOOL		bWater;

	if ( dive == 0 || dive->RetLock() )
	{
		m_bError = TRUE;
		return ERR_GENERIC;
	}

	m_dive = dive;
	m_startPos = m_object->RetPosition(0);
	m_startAngle = m_object->RetAngleY(0);

	mat = m_dive->RetWorldMatrix(0);
	m_goalPos = Transform(*mat, D3DVECTOR(16.0f, 0.0f, 0.0f));
	m_goalPos.y = m_startPos.y+DIVE_HEIGHT;
	m_oupsPos = Transform(*mat, D3DVECTOR(20.4f, 0.0f, 0.0f));
	m_oupsPos.y = m_startPos.y+DIVE_HEIGHT;
	m_divePos = Transform(*mat, D3DVECTOR(32.0f, 0.0f, 0.0f));
	m_divePos.y = m_startPos.y+DIVE_HEIGHT;

	m_level = m_water->RetLevel();
	m_bPlouf = TRUE;

	m_dive->SetLock(TRUE);

	bWater = TRUE;
	if ( m_terrain->RetResource(m_goalPos) != TR_SPACE )  bWater = FALSE;
	if ( m_terrain->RetResource(m_divePos) != TR_SPACE )  bWater = FALSE;

	if ( bWater )
	{
		m_phase = TDI_END;
		m_speed = 1.0f/3.0f;
	}
	else
	{
		m_phase = TDI_MARCH;
		m_speed = 1.0f/3.0f;
	}
	m_time = 0.0f;
	m_progress = 0.0f;
	m_bError = FALSE;

	StartAction(MBLUPI_WALK);
	SetLinSpeed(1.0f);

	return ERR_OK;
}

// Indique si l'action est terminée.

Error CTaskDive::IsEnded()
{
	if ( m_engine->RetPause() )  return ERR_CONTINUE;

	if ( m_bError )
	{
		return ERR_STOP;
	}

	if ( m_progress < 1.0f )
	{
		return ERR_CONTINUE;
	}

	// Refuse de plonger.

	if ( m_phase == TDI_MARCH )
	{
		m_sound->Play(SOUND_BLUPIpeur, m_goalPos);
		StartAction(MBLUPI_GOAL1);
		SetLinSpeed(0.0f);
		m_phase = TDI_TRY;
		m_progress = 0.0f;
		m_speed = 1.0f/2.0f;
		return ERR_CONTINUE;
	}

	if ( m_phase == TDI_TRY )
	{
		StartAction(MBLUPI_ERROR);
		SetLinSpeed(0.0f);
		m_phase = TDI_NONO;
		m_progress = 0.0f;
		m_speed = 1.0f/1.0f;
		return ERR_CONTINUE;
	}

	if ( m_phase == TDI_NONO )
	{
		StartAction(MBLUPI_TURN);
		SetLinSpeed(0.0f);
		m_phase = TDI_TURN;
		m_progress = 0.0f;
		m_speed = 1.0f/1.0f;
		return ERR_CONTINUE;
	}

	if ( m_phase == TDI_TURN )
	{
		StartAction(MBLUPI_WALK);
		SetLinSpeed(1.0f);
		m_phase = TDI_BACK;
		m_progress = 0.0f;
		m_speed = 1.0f/3.0f;
		return ERR_CONTINUE;
	}

	if ( m_phase == TDI_BACK )
	{
		StartAction(MBLUPI_WAIT);
		SetLinSpeed(0.0f);
		m_dive->SetLock(FALSE);
		return ERR_STOP;
	}

	// Plonge pour de vrai.

	if ( m_phase == TDI_END )
	{
		m_sound->Play(SOUND_BLUPIpeur, m_oupsPos);
		StartAction(MBLUPI_OUPS, 1.0f);
		SetLinSpeed(0.0f);
		m_phase = TDI_OUPS;
		m_progress = 0.0f;
		m_speed = 1.0f/1.0f;
		return ERR_CONTINUE;
	}

	if ( m_phase == TDI_OUPS )
	{
		m_phase = TDI_DIVE;
		m_progress = 0.0f;
		m_speed = 1.0f/1.0f;
		return ERR_CONTINUE;
	}

	if ( m_phase == TDI_DIVE )
	{
		StartAction(MBLUPI_WAIT);
		SetLinSpeed(0.0f);
		m_object->SetAngleY(0, m_startAngle+PI);  // demi-tour
		m_object->SetAngleZ(0, 0.0f);
		m_phase = TDI_WAIT;
		m_progress = 0.0f;
		m_speed = 1.0f/3.0f;
		return ERR_CONTINUE;
	}

	if ( m_phase == TDI_WAIT )
	{
		m_sound->Play(SOUND_BLUP, m_oupsPos);
		StartAction(MBLUPI_GOAL2);
		SetLinSpeed(0.0f);
		m_phase = TDI_UP;
		m_progress = 0.0f;
		m_speed = 1.0f/0.8f;
		m_bPlouf = TRUE;
		return ERR_CONTINUE;
	}

	if ( m_phase == TDI_UP )
	{
		m_sound->Play(SOUND_BLUPIhic, m_oupsPos);
		m_phase = TDI_OSCIL;
		m_progress = 0.0f;
		m_speed = 1.0f/3.0f;
		return ERR_CONTINUE;
	}

	if ( m_phase == TDI_OSCIL )
	{
		m_sound->Play(SOUND_BLUPIoups, m_oupsPos);
		StartAction(MBLUPI_OUPS, 0.5f);
		SetLinSpeed(0.0f);
		m_phase = TDI_JUMP;
		m_progress = 0.0f;
		m_speed = 1.0f/1.0f;
		return ERR_CONTINUE;
	}

	if ( m_phase == TDI_JUMP )
	{
		m_sound->Play(SOUND_BLUPIohhh, m_oupsPos);
		StartAction(MBLUPI_AMAZE);
		SetLinSpeed(0.0f);
		m_phase = TDI_OH;
		m_progress = 0.0f;
		m_speed = 1.0f/2.0f;
		return ERR_CONTINUE;
	}

	if ( m_phase == TDI_OH )
	{
		StartAction(MBLUPI_WAIT);
		StartAction(MBLUPI_WALK);
		SetLinSpeed(1.0f);
		m_phase = TDI_BACK;
		m_progress = 0.0f;
		m_speed = 1.0f/3.0f;
		return ERR_CONTINUE;
	}

	return ERR_STOP;
}


// Démarre une action pour Blupi.

void CTaskDive::StartAction(int action, float speed)
{
	CMotion*	motion;

	motion = m_object->RetMotion();
	if ( motion == 0 )  return;

	motion->SetAction(action, speed);
}

// Spécifie la vitesse de Blupi.

void CTaskDive::SetLinSpeed(float speed)
{
	CMotion*	motion;

	motion = m_object->RetMotion();
	if ( motion == 0 )  return;

	m_motion->SetActionLinSpeed(speed);
}

