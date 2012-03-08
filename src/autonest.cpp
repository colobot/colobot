// autonest.cpp

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
#include "cmdtoken.h"
#include "auto.h"
#include "autonest.h"




// Constructeur de l'objet.

CAutoNest::CAutoNest(CInstanceManager* iMan, CObject* object)
					 : CAuto(iMan, object)
{
	CAuto::CAuto(iMan, object);

	Init();
}

// Destructeur de l'objet.

CAutoNest::~CAutoNest()
{
	CAuto::~CAuto();
}


// Détruit l'objet.

void CAutoNest::DeleteObject(BOOL bAll)
{
	CObject*	fret;

	if ( !bAll )
	{
		fret = SearchFret();
		if ( fret != 0 )
		{
			fret->DeleteObject();
			delete fret;
		}
	}

	CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoNest::Init()
{
	D3DVECTOR	pos;

	m_phase    = ANP_WAIT;
	m_progress = 0.0f;
	m_speed    = 1.0f/4.0f;

	m_time     = 0.0f;
	m_lastParticule = 0.0f;

	pos = m_object->RetPosition(0);
	m_terrain->MoveOnFloor(pos);
	m_fretPos = pos;
}


// Gestion d'un événement.

BOOL CAutoNest::EventProcess(const Event &event)
{
	CObject*	fret;

	CAuto::EventProcess(event);

	if ( m_engine->RetPause() )  return TRUE;
	if ( event.event != EVENT_FRAME )  return TRUE;

	m_progress += event.rTime*m_speed;

	if ( m_phase == ANP_WAIT )
	{
		if ( m_progress >= 1.0f )
		{
			if ( !SearchFree(m_fretPos) )
			{
				m_phase    = ANP_WAIT;
				m_progress = 0.0f;
				m_speed    = 1.0f/4.0f;
			}
			else
			{
				CreateFret(m_fretPos, 0.0f, OBJECT_BULLET);
				m_phase    = ANP_BIRTH;
				m_progress = 0.0f;
				m_speed    = 1.0f/5.0f;
			}
		}
	}

	if ( m_phase == ANP_BIRTH )
	{
		fret = SearchFret();

		if ( m_progress < 1.0f )
		{
			if ( fret != 0 )
			{
				fret->SetZoom(0, m_progress);
			}
		}
		else
		{
			if ( fret != 0 )
			{
				fret->SetZoom(0, 1.0f);
				fret->SetLock(FALSE);
			}

			m_phase    = ANP_WAIT;
			m_progress = 0.0f;
			m_speed    = 1.0f/5.0f;
		}
	}

	return TRUE;
}


// Cherche si un emplacement est libre.

BOOL CAutoNest::SearchFree(D3DVECTOR pos)
{
	CObject*	pObj;
	D3DVECTOR	sPos;
	ObjectType	type;
	float		sRadius, distance;
	int			i, j;

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		type = pObj->RetType();
		if ( type == OBJECT_NEST )  continue;

		j = 0;
		while ( pObj->GetCrashSphere(j++, sPos, sRadius) )
		{
			distance = Length(sPos, pos);
			distance -= sRadius;
			if ( distance < 2.0f )  return FALSE;  // emplacement occupé
		}
	}

	return TRUE;  // emplacement libre
}

// Crée un objet transportable.

void CAutoNest::CreateFret(D3DVECTOR pos, float angle, ObjectType type)
{
	CObject*	fret;

	fret = new CObject(m_iMan);
	if ( !fret->CreateResource(pos, angle, type) )
	{
		delete fret;
		return;
	}
	fret->SetLock(TRUE);  // pas utilisable
	fret->SetZoom(0, 0.0f);
}

// Cherche le boullet en cours de fabrication.

CObject* CAutoNest::SearchFret()
{
	CObject*	pObj;
	D3DVECTOR	oPos;
	ObjectType	type;
	int			i;

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		if ( !pObj->RetLock() )  continue;

		type = pObj->RetType();
		if ( type != OBJECT_BULLET )  continue;

		oPos = pObj->RetPosition(0);
		if ( oPos.x == m_fretPos.x &&
			 oPos.z == m_fretPos.z )
		{
			return pObj;
		}
	}

	return 0;
}


// Retourne une erreur liée à l'état de l'automate.

Error CAutoNest::RetError()
{
	return ERR_OK;
}


// Sauve tous les paramètres de l'automate.

BOOL CAutoNest::Write(char *line)
{
	D3DVECTOR	pos;
	char		name[100];

	if ( m_phase == ANP_WAIT )  return FALSE;

	sprintf(name, " aExist=%d", 1);
	strcat(line, name);

	CAuto::Write(line);

	sprintf(name, " aPhase=%d", m_phase);
	strcat(line, name);

	sprintf(name, " aProgress=%.2f", m_progress);
	strcat(line, name);

	sprintf(name, " aSpeed=%.2f", m_speed);
	strcat(line, name);

	return TRUE;
}

// Restitue tous les paramètres de l'automate.

BOOL CAutoNest::Read(char *line)
{
	D3DVECTOR	pos;

	if ( OpInt(line, "aExist", 0) == 0 )  return FALSE;

	CAuto::Read(line);

	m_phase = (AutoNestPhase)OpInt(line, "aPhase", ANP_WAIT);
	m_progress = OpFloat(line, "aProgress", 0.0f);
	m_speed = OpFloat(line, "aSpeed", 1.0f);

	m_lastParticule = 0.0f;

	return TRUE;
}


