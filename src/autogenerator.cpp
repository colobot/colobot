// autogenerator.cpp

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
#include "motion.h"
#include "interface.h"
#include "button.h"
#include "window.h"
#include "robotmain.h"
#include "sound.h"
#include "auto.h"
#include "autogenerator.h"



// Constructeur de l'objet.

CAutoGenerator::CAutoGenerator(CInstanceManager* iMan, CObject* object)
						 : CAuto(iMan, object)
{
	CAuto::CAuto(iMan, object);

	Init();
}

// Destructeur de l'objet.

CAutoGenerator::~CAutoGenerator()
{
	CAuto::~CAuto();
}


// Détruit l'objet.

void CAutoGenerator::DeleteObject(BOOL bAll)
{
	CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoGenerator::Init()
{
	m_time = 0.0f;
	m_lastParticule = 0.0f;

	m_totalCreate = 10;
	m_maxCreate = 10;
	m_delay = 10.0f;
	m_programNumber = -1;

	m_phase    = AGEP_WAIT;
	m_progress = 0.0f;
	m_speed    = 1.0f/m_delay;
}


// Démarre l'objet.

void CAutoGenerator::Start(int param)
{
	float		value;

	value = m_object->RetCmdLine(0);
	if ( value == NAN )
	{
		m_totalCreate = 0;
	}
	else
	{
		m_totalCreate = (int)value;
	}

	value = m_object->RetCmdLine(1);
	if ( value == NAN )
	{
		m_maxCreate = 10;
	}
	else
	{
		m_maxCreate = (int)value;
	}

	value = m_object->RetCmdLine(2);
	if ( value == NAN )
	{
		m_delay = 10.0f;
	}
	else
	{
		m_delay = value;
	}

	value = m_object->RetCmdLine(3);
	if ( value == NAN )
	{
		m_programNumber = -1;
	}
	else
	{
		m_programNumber = (int)value;
	}

	m_phase    = AGEP_WAIT;
	m_progress = 0.0f;
	m_speed    = 1.0f/m_delay;
}


// Gestion d'un événement.

BOOL CAutoGenerator::EventProcess(const Event &event)
{
	D3DVECTOR	pos, speed;
	FPOINT		dim, c, p;
	CObject*	pObj;

	CAuto::EventProcess(event);

	if ( m_engine->RetPause() )  return TRUE;

	m_progress += event.rTime*m_speed;

	if ( m_phase == AGEP_WAIT )
	{
		if ( m_progress >= 1.0f )
		{
			pObj = SearchObject();
			if ( pObj != 0 ||  // position occupée ?
				 m_totalCreate == 0 ||  // plus rien à créer ?
				 CountObject(OBJECT_EVIL1) >= m_maxCreate ||  // trop d'objets ?
				 !m_main->IsGameTime() )  // partie terminée ?
			{
				m_phase    = AGEP_WAIT;
				m_progress = 0.0f;
				m_speed    = 1.0f/m_delay;
			}
			else
			{
				if ( !CreateObject(OBJECT_EVIL1) )
				{
					m_phase    = AGEP_WAIT;
					m_progress = 0.0f;
					m_speed    = 1.0f/m_delay;
				}
				else
				{
					m_totalCreate --;

					m_phase    = AGEP_CREATE;
					m_progress = 0.0f;
					m_speed    = 1.0f/5.0f;
				}
			}
		}
	}

	if ( m_phase == AGEP_CREATE )
	{
		pObj = SearchObject(OBJECT_EVIL1);
		if ( pObj == 0 )
		{
			m_phase    = AGEP_WAIT;
			m_progress = 0.0f;
			m_speed    = 1.0f/m_delay;
			return TRUE;
		}

		if ( m_progress < 1.0f )
		{
			pObj->SetZoom(0, m_progress);
			pObj->SetAngleY(0, pObj->RetAngleY(0)+event.rTime*5.0f);

			if ( m_lastParticule+m_engine->ParticuleAdapt(0.10f) <= m_time )
			{
				m_lastParticule = m_time;

				pos = m_object->RetPosition(0);
				c.x = pos.x;
				c.y = pos.z;
				p.x = c.x;
				p.y = c.y+2.0f;
				p = RotatePoint(c, Rand()*PI*2.0f, p);
				pos.x = p.x;
				pos.z = p.y;
				pos.y += 2.5f+Rand()*3.0f;
				speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
				dim.x = Rand()*2.0f+1.0f;
				dim.y = dim.x;
				m_particule->CreateParticule(pos, speed, dim, PARTIEXPLOG2, 1.0f, 0.0f);

				pos = m_object->RetPosition(0);
//?				pos.y += 3.0f;
				speed.x = (Rand()-0.5f)*30.0f;
				speed.z = (Rand()-0.5f)*30.0f;
				speed.y = Rand()*20.0f+10.0f;
				dim.x = Rand()*0.4f+0.4f;
				dim.y = dim.x;
				m_particule->CreateTrack(pos, speed, dim, PARTITRACK4, 2.0f, 50.0f, 1.2f, 1.2f);

				pos = m_object->RetPosition(0);
				pos.y += 20.0f;
				speed.x = (Rand()-0.5f)*1.5f;
				speed.z = (Rand()-0.5f)*1.5f;
				speed.y = -20.0f;
				dim.x = Rand()*2.0f+2.0f;
				dim.y = dim.x;
				m_particule->CreateParticule(pos, speed, dim, PARTIEXPLOG2, 1.0f, 0.0f);

				m_sound->Play(SOUND_ENERGY, m_object->RetPosition(0),
							  1.0f, 1.0f+Rand()*1.5f);
			}
		}
		else
		{
			pObj->SetZoom(0, 1.0f);
			StartAction(pObj, 0);  // attend au repos
			if ( m_programNumber != -1 )
			{
				char	name[100];
				sprintf(name, "auto%.2d.txt", m_programNumber);
				pObj->ReadProgram(0, name);
				pObj->RunProgram(0);
			}

			m_phase    = AGEP_WAIT;
			m_progress = 0.0f;
			m_speed    = 1.0f/m_delay;
		}
	}

	return TRUE;
}


// Stoppe l'automate.

BOOL CAutoGenerator::Abort()
{
	return TRUE;
}


// Retourne une erreur liée à l'état de l'automate.

Error CAutoGenerator::RetError()
{
	return ERR_OK;
}


// Cherche un objet sous le générateur.

CObject* CAutoGenerator::SearchObject()
{
	CObject*	pObj;
	D3DVECTOR	cPos, oPos;
	ObjectType	type;
	float		dist;
	int			i;

	cPos = m_object->RetPosition(0);

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( pObj->RetLock() )  continue;

		type = pObj->RetType();
		if ( type == OBJECT_GENERATOR )  continue;

		oPos = pObj->RetPosition(0);
		dist = Length(oPos, cPos);
		if ( dist <= 3.0f )  return pObj;
	}

	return 0;
}

// Cherche un objet sous le générateur.

CObject* CAutoGenerator::SearchObject(ObjectType type)
{
	CObject*	pObj;
	D3DVECTOR	cPos, oPos;
	float		dist;
	int			i;

	cPos = m_object->RetPosition(0);

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( pObj->RetLock() )  continue;
		if ( pObj->RetType() != type )  continue;

		oPos = pObj->RetPosition(0);
		dist = Length(oPos, cPos);
		if ( dist <= 3.0f )  return pObj;
	}

	return 0;
}

// Compte un objet partout.

int CAutoGenerator::CountObject(ObjectType type)
{
	CObject*	pObj;
	D3DVECTOR	cPos, oPos;
	int			i, total;

	total = 0;
	cPos = m_object->RetPosition(0);

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( pObj->RetLock() )  continue;
		if ( pObj->RetType() != type )  continue;

		total ++;
	}

	return total;
}

// Crée un objet sous le générateur.

BOOL CAutoGenerator::CreateObject(ObjectType type)
{
	D3DVECTOR		pos;
	float			angle;
	CObject*		bot;

	pos = m_object->RetPosition(0);
	angle = m_object->RetAngleY(0);

	bot = new CObject(m_iMan);
	if ( !bot->CreateBot(pos, angle, 1.0f, type, FALSE) )
	{
		delete bot;
		return FALSE;
	}
	bot->SetZoom(0, 0.0f);  // minuscule
	StartAction(bot, -1);  // zoom manuel (pas géré par CMotion)
	return TRUE;
}

// Démarre une action pour un objet.

void CAutoGenerator::StartAction(CObject *pObj, int action)
{
	CMotion*	motion;

	motion = pObj->RetMotion();
	if ( motion == 0 )  return;

	motion->SetAction(action);
}

