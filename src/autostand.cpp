// autostand.cpp

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
#include "sound.h"
#include "auto.h"
#include "autostand.h"



// Constructeur de l'objet.

CAutoStand::CAutoStand(CInstanceManager* iMan, CObject* object)
						 : CAuto(iMan, object)
{
	CAuto::CAuto(iMan, object);

	Init();
}

// Destructeur de l'objet.

CAutoStand::~CAutoStand()
{
	CAuto::~CAuto();
}


// Détruit l'objet.

void CAutoStand::DeleteObject(BOOL bAll)
{
	if ( m_channelSound != -1 )
	{
		m_sound->FlushEnvelope(m_channelSound);
		m_sound->AddEnvelope(m_channelSound, 0.0f, 1.0f, 2.0f, SOPER_STOP);
		m_channelSound = -1;
	}

	CAuto::DeleteObject(bAll);
}


// Initialise l'objet.

void CAutoStand::Init()
{
	m_time = 0.0f;

	m_progress = 0.0f;
	m_speed    = 1.0f/2.0f;
	m_channelSound = -1;
	m_totalPerso = 0;
}


// Démarre l'objet.

void CAutoStand::Start(int param)
{
	D3DMATRIX*	mat;
	CObject*	object;
	ObjectType	type;
	D3DVECTOR	pos;
	BOOL		occupied[8*4];
	float		value, angle, percent;
	int			i, r, x, y;

	value = m_object->RetCmdLine(0);
	if ( value == NAN )
	{
		m_totalPerso = 0;
	}
	else
	{
		m_totalPerso = (int)value;
		percent = m_engine->RetGadgetQuantity();
		m_totalPerso = (int)(percent*m_totalPerso);
	}
	if ( m_totalPerso > 8*4 )  m_totalPerso = 8*4;

	for ( i=0 ; i<8*4 ; i++ )
	{
		occupied[i] = FALSE;
	}

	mat = m_object->RetWorldMatrix(0);
	angle = m_object->RetAngleY(0);
	for ( i=0 ; i<m_totalPerso ; i++ )
	{
		r = rand()%4;
		if ( r == 0 )  type = OBJECT_BOT1;
		if ( r == 1 )  type = OBJECT_BOT2;
		if ( r == 2 )  type = OBJECT_BOT3;
		if ( r == 3 )  type = OBJECT_BOT4;

		while ( TRUE )
		{
			int rank[16] = {0,0,0,0,0,0,0,0,1,1,1,1,1,2,2,3};
			x = rank[rand()%16];  // rempli mieux les premiers rangs
			pos.x = 15.0f-x*10.0f;
			pos.y = 4.0f+x*4.0f;

			y = rand()%8;
			pos.z = y*10.0f-40.0f+(Rand()-0.5f)*6.0f;

			if ( occupied[x*8+y] == FALSE )
			{
				occupied[x*8+y] = TRUE;
				break;
			}
		}
		pos = Transform(*mat, pos);

		object = new CObject(m_iMan);
		if ( !object->CreateBot(pos, angle, 1.0f, type, TRUE) )
		{
			delete object;
		}
		else
		{
			pos.y += object->RetCharacter()->height;
			object->SetPosition(0, pos);
		}
	}
}


// Gestion d'un événement.

BOOL CAutoStand::EventProcess(const Event &event)
{
	CAuto::EventProcess(event);

	if ( m_engine->RetPause() )  return TRUE;

	m_progress += event.rTime*m_speed;
	SoundFrame(event.rTime);

	return TRUE;
}

// Gestion du son.

void CAutoStand::SoundFrame(float rTime)
{
	D3DVECTOR	pos, look;
	float		dist;

	if ( m_totalPerso == 0 )  return;

	pos = m_object->RetPosition(0);
	look = m_engine->RetLookatPt();
	dist = Length(pos, look);

	if ( dist >= 400.0f )
	{
		if ( m_channelSound != -1 )
		{
			m_sound->FlushEnvelope(m_channelSound);
			m_sound->AddEnvelope(m_channelSound, 0.0f, 1.0f, 2.0f, SOPER_STOP);
			m_channelSound = -1;
		}
	}
	else
	{
		if ( m_channelSound == -1 )
		{
			m_channelSound = m_sound->Play(SOUND_RESEARCH, pos, 0.0f, 1.0f, TRUE);
			m_sound->AddEnvelope(m_channelSound, 1.0f, 1.0f, 2.0f, SOPER_CONTINUE);
			m_sound->AddEnvelope(m_channelSound, 1.0f, 1.0f, 1.0f, SOPER_LOOP);
		}
	}
}



// Stoppe l'automate.

BOOL CAutoStand::Abort()
{
	return TRUE;
}


// Retourne une erreur liée à l'état de l'automate.

Error CAutoStand::RetError()
{
	return ERR_OK;
}


