// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "common/struct.h"
#include "graphics/d3d/d3dengine.h"
#include "math/old/d3dmath.h"
#include "common/event.h"
#include "common/misc.h"
#include "common/iman.h"
#include "math/old/math3d.h"
#include "graphics/common/particule.h"
#include "graphics/common/light.h"
#include "graphics/common/terrain.h"
#include "graphics/common/camera.h"
#include "object/object.h"
#include "ui/interface.h"
#include "ui/button.h"
#include "ui/window.h"
#include "sound/sound.h"
#include "ui/displaytext.h"
#include "script/cmdtoken.h"
#include "object/auto/auto.h"
#include "object/auto/autoconvert.h"




// Object's constructor.

CAutoConvert::CAutoConvert(CInstanceManager* iMan, CObject* object)
						   : CAuto(iMan, object)
{
	Init();
	m_phase = ACP_STOP;
	m_bResetDelete = false;
	m_soundChannel = -1;
}

// Object's destructor.

CAutoConvert::~CAutoConvert()
{
}


// Destroys the object.

void CAutoConvert::DeleteObject(bool bAll)
{
	CObject*	fret;

	if ( !bAll )
	{
		fret = SearchStone(OBJECT_STONE);
		if ( fret != 0 )
		{
			fret->DeleteObject();  // destroy the stone
			delete fret;
		}
	}

	if ( m_soundChannel != -1 )
	{
		m_sound->FlushEnvelope(m_soundChannel);
		m_sound->AddEnvelope(m_soundChannel, 0.0f, 1.0f, 1.0f, SOPER_STOP);
		m_soundChannel = -1;
	}

	CAuto::DeleteObject(bAll);
}


// Initialize the object.

void CAutoConvert::Init()
{
	m_phase = ACP_WAIT;
	m_progress = 0.0f;
	m_speed    = 1.0f/2.0f;
	m_time = 0.0f;
	m_timeVirus = 0.0f;
	m_lastParticule = 0.0f;

	CAuto::Init();
}


// Management of an event.

bool CAutoConvert::EventProcess(const Event &event)
{
	CObject*	fret;
	D3DVECTOR	pos, speed;
	FPOINT		dim, c, p;
	float		angle;

	CAuto::EventProcess(event);

	if ( m_engine->RetPause() )  return true;
	if ( event.event != EVENT_FRAME )  return true;

	m_progress += event.rTime*m_speed;
	m_timeVirus -= event.rTime;

	if ( m_object->RetVirusMode() )  // contaminated by a virus?
	{
		if ( m_timeVirus <= 0.0f )
		{
			m_timeVirus = 0.1f+Rand()*0.3f;

			angle = (Rand()-0.5f)*0.3f;
			m_object->SetAngleY(1, angle);
			m_object->SetAngleY(2, angle);
			m_object->SetAngleY(3, angle+PI);

			m_object->SetAngleX(2, -PI*0.35f*(0.8f+Rand()*0.2f));
			m_object->SetAngleX(3, -PI*0.35f*(0.8f+Rand()*0.2f));
		}
		return true;
	}

	EventProgress(event.rTime);

	if ( m_phase == ACP_STOP )  return true;

	if ( m_phase == ACP_WAIT )
	{
		if ( m_progress >= 1.0f )
		{
			fret = SearchStone(OBJECT_STONE);  // Has stone transformed?
			if ( fret == 0 || SearchVehicle() )
			{
				m_phase    = ACP_WAIT;  // still waiting ...
				m_progress = 0.0f;
				m_speed    = 1.0f/2.0f;
			}
			else
			{
				fret->SetLock(true);  // stone usable

				SetBusy(true);
				InitProgressTotal(3.0f+10.0f+1.5f);
				UpdateInterface();

				m_sound->Play(SOUND_OPEN, m_object->RetPosition(0), 1.0f, 1.0f);
				m_bSoundClose = false;

				m_phase    = ACP_CLOSE;
				m_progress = 0.0f;
				m_speed    = 1.0f/3.0f;
			}
		}
	}

	if ( m_phase == ACP_CLOSE )
	{
		if ( m_progress < 1.0f )
		{
			if ( m_progress >= 0.8f && !m_bSoundClose )
			{
				m_bSoundClose = true;
				m_sound->Play(SOUND_CLOSE, m_object->RetPosition(0), 1.0f, 0.8f);
			}
			angle = -PI*0.35f*(1.0f-Bounce(m_progress, 0.85f, 0.05f));
			m_object->SetAngleX(2, angle);
			m_object->SetAngleX(3, angle);
		}
		else
		{
			m_object->SetAngleX(2, 0.0f);
			m_object->SetAngleX(3, 0.0f);

			m_soundChannel = m_sound->Play(SOUND_CONVERT, m_object->RetPosition(0), 0.0f, 0.25f, true);
			m_sound->AddEnvelope(m_soundChannel, 1.0f, 0.25f, 0.5f, SOPER_CONTINUE);
			m_sound->AddEnvelope(m_soundChannel, 1.0f, 1.00f, 4.5f, SOPER_CONTINUE);
			m_sound->AddEnvelope(m_soundChannel, 1.0f, 0.25f, 4.5f, SOPER_CONTINUE);
			m_sound->AddEnvelope(m_soundChannel, 0.0f, 0.25f, 0.5f, SOPER_STOP);

			m_phase    = ACP_ROTATE;
			m_progress = 0.0f;
			m_speed    = 1.0f/10.0f;
		}
	}

	if ( m_phase == ACP_ROTATE )
	{
		if ( m_progress < 1.0f )
		{
			if ( m_progress < 0.5f )
			{
				angle = powf((m_progress*2.0f)*5.0f, 2.0f);  // accelerates
			}
			else
			{
				angle = -powf((2.0f-m_progress*2.0f)*5.0f, 2.0f);  // slows
			}
			m_object->SetAngleY(1, angle);
			m_object->SetAngleY(2, angle);
			m_object->SetAngleY(3, angle+PI);

			if ( m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
			{
				m_lastParticule = m_time;

				pos = m_object->RetPosition(0);
				c.x = pos.x;
				c.y = pos.z;
				p.x = c.x;
				p.y = c.y+6.0f;
				p = RotatePoint(c, Rand()*PI*2.0f, p);
				pos.x = p.x;
				pos.z = p.y;
				pos.y += 1.0f;
				speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
				dim.x = Rand()*2.0f+1.0f;
				dim.y = dim.x;
				m_particule->CreateParticule(pos, speed, dim, PARTIGAS, 1.0f, 0.0f, 0.0f);
			}
		}
		else
		{
			m_object->SetAngleY(1, 0.0f);
			m_object->SetAngleY(2, 0.0f);
			m_object->SetAngleY(3, PI);

			fret = SearchStone(OBJECT_STONE);
			if ( fret != 0 )
			{
				m_bResetDelete = ( fret->RetResetCap() != RESET_NONE );
				fret->DeleteObject();  // destroy the stone
				delete fret;
			}

			CreateMetal();  // Create the metal
			m_sound->Play(SOUND_OPEN, m_object->RetPosition(0), 1.0f, 1.5f);

			m_phase    = ACP_OPEN;
			m_progress = 0.0f;
			m_speed    = 1.0f/1.5f;
		}
	}

	if ( m_phase == ACP_OPEN )
	{
		if ( m_progress < 1.0f )
		{
			angle = -PI*0.35f*Bounce(m_progress, 0.7f, 0.2f);
			m_object->SetAngleX(2, angle);
			m_object->SetAngleX(3, angle);

			if ( m_progress < 0.9f &&
				 m_lastParticule+m_engine->ParticuleAdapt(0.05f) <= m_time )
			{
				m_lastParticule = m_time;

				pos = m_object->RetPosition(0);
				pos.x += (Rand()-0.5f)*6.0f;
				pos.z += (Rand()-0.5f)*6.0f;
				pos.y += Rand()*4.0f;
				speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
				dim.x = Rand()*4.0f+3.0f;
				dim.y = dim.x;
				m_particule->CreateParticule(pos, speed, dim, PARTIBLUE, 1.0f, 0.0f, 0.0f);
			}
		}
		else
		{
			m_soundChannel = -1;
			m_object->SetAngleX(2, -PI*0.35f);
			m_object->SetAngleX(3, -PI*0.35f);

			SetBusy(false);
			UpdateInterface();

			m_phase    = ACP_WAIT;
			m_progress = 0.0f;
			m_speed    = 1.0f/2.0f;
		}
	}

	return true;
}

// Returns an error due the state of the automation.

Error CAutoConvert::RetError()
{
	if ( m_object->RetVirusMode() )
	{
		return ERR_BAT_VIRUS;
	}

	if ( m_phase == ACP_WAIT )  return ERR_CONVERT_EMPTY;
	return ERR_OK;
}

// Cancels the current transformation.

bool CAutoConvert::Abort()
{
	if ( m_soundChannel != -1 )
	{
		m_sound->FlushEnvelope(m_soundChannel);
		m_sound->AddEnvelope(m_soundChannel, 0.0f, 1.0f, 1.0f, SOPER_STOP);
		m_soundChannel = -1;
	}

	m_object->SetAngleY(1, 0.0f);
	m_object->SetAngleY(2, 0.0f);
	m_object->SetAngleY(3, PI);
	m_object->SetAngleX(2, -PI*0.35f);
	m_object->SetAngleX(3, -PI*0.35f);

	m_phase    = ACP_WAIT;
	m_progress = 0.0f;
	m_speed    = 1.0f/2.0f;

	SetBusy(false);
	UpdateInterface();

	return true;
}


// Creates all the interface when the object is selected.

bool CAutoConvert::CreateInterface(bool bSelect)
{
	CWindow*	pw;
	FPOINT		pos, ddim;
	float		ox, oy, sx, sy;

	CAuto::CreateInterface(bSelect);

	if ( !bSelect )  return true;

	pw = (CWindow*)m_interface->SearchControl(EVENT_WINDOW0);
	if ( pw == 0 )  return false;

	ox = 3.0f/640.0f;
	oy = 3.0f/480.0f;
	sx = 33.0f/640.0f;
	sy = 33.0f/480.0f;

	pos.x = ox+sx*0.0f;
	pos.y = oy+sy*0;
	ddim.x = 66.0f/640.0f;
	ddim.y = 66.0f/480.0f;
	pw->CreateGroup(pos, ddim, 103, EVENT_OBJECT_TYPE);

	return true;
}


// Saves all parameters of the controller.

bool CAutoConvert::Write(char *line)
{
	char	name[100];

	if ( m_phase == ACP_STOP ||
		 m_phase == ACP_WAIT )  return false;

	sprintf(name, " aExist=%d", 1);
	strcat(line, name);

	CAuto::Write(line);

	sprintf(name, " aPhase=%d", m_phase);
	strcat(line, name);

	sprintf(name, " aProgress=%.2f", m_progress);
	strcat(line, name);

	sprintf(name, " aSpeed=%.2f", m_speed);
	strcat(line, name);

	return true;
}

// Restores all parameters of the controller.

bool CAutoConvert::Read(char *line)
{
	if ( OpInt(line, "aExist", 0) == 0 )  return false;

	CAuto::Read(line);

	m_phase = (AutoConvertPhase)OpInt(line, "aPhase", ACP_WAIT);
	m_progress = OpFloat(line, "aProgress", 0.0f);
	m_speed = OpFloat(line, "aSpeed", 1.0f);

	m_lastParticule = 0.0f;

	return true;
}


// Searches for the object before or during processing.

CObject* CAutoConvert::SearchStone(ObjectType type)
{
	CObject*	pObj;
	D3DVECTOR	cPos, oPos;
	ObjectType	oType;
	float		dist;
	int			i;

	cPos = m_object->RetPosition(0);

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		oType = pObj->RetType();
		if ( oType != type )  continue;
		if ( pObj->RetTruck() != 0 )  continue;

		oPos = pObj->RetPosition(0);
		dist = Length(oPos, cPos);

		if ( dist <= 5.0f )  return pObj;
	}

	return 0;
}

// Search if a vehicle is too close.

bool CAutoConvert::SearchVehicle()
{
	CObject*	pObj;
	D3DVECTOR	cPos, oPos;
	ObjectType	type;
	float		oRadius, dist;
	int			i;

	cPos = m_object->RetPosition(0);

	for ( i=0 ; i<1000000 ; i++ )
	{
		pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
		if ( pObj == 0 )  break;

		type = pObj->RetType();
		if ( type != OBJECT_HUMAN    &&
			 type != OBJECT_MOBILEfa &&
			 type != OBJECT_MOBILEta &&
			 type != OBJECT_MOBILEwa &&
			 type != OBJECT_MOBILEia &&
			 type != OBJECT_MOBILEfc &&
			 type != OBJECT_MOBILEtc &&
			 type != OBJECT_MOBILEwc &&
			 type != OBJECT_MOBILEic &&
			 type != OBJECT_MOBILEfi &&
			 type != OBJECT_MOBILEti &&
			 type != OBJECT_MOBILEwi &&
			 type != OBJECT_MOBILEii &&
			 type != OBJECT_MOBILEfs &&
			 type != OBJECT_MOBILEts &&
			 type != OBJECT_MOBILEws &&
			 type != OBJECT_MOBILEis &&
			 type != OBJECT_MOBILErt &&
			 type != OBJECT_MOBILErc &&
			 type != OBJECT_MOBILErr &&
			 type != OBJECT_MOBILErs &&
			 type != OBJECT_MOBILEsa &&
			 type != OBJECT_MOBILEtg &&
			 type != OBJECT_MOBILEft &&
			 type != OBJECT_MOBILEtt &&
			 type != OBJECT_MOBILEwt &&
			 type != OBJECT_MOBILEit &&
			 type != OBJECT_MOBILEdr &&
			 type != OBJECT_METAL    &&
			 type != OBJECT_URANIUM  &&
			 type != OBJECT_POWER    &&
			 type != OBJECT_ATOMIC   &&
			 type != OBJECT_BULLET   &&
			 type != OBJECT_BBOX     &&
			 type != OBJECT_TNT      &&
			 type != OBJECT_MOTHER   &&
			 type != OBJECT_ANT      &&
			 type != OBJECT_SPIDER   &&
			 type != OBJECT_BEE      &&
			 type != OBJECT_WORM     )  continue;

		if ( !pObj->GetCrashSphere(0, oPos, oRadius) )  continue;
		dist = Length(oPos, cPos)-oRadius;

		if ( dist < 8.0f )  return true;
	}

	return false;
}

// Creates an object metal.

void CAutoConvert::CreateMetal()
{
	D3DVECTOR		pos;
	float			angle;
	CObject*		fret;

	pos = m_object->RetPosition(0);
	angle = m_object->RetAngleY(0);

	fret = new CObject(m_iMan);
	if ( !fret->CreateResource(pos, angle, OBJECT_METAL) )
	{
		delete fret;
		m_displayText->DisplayError(ERR_TOOMANY, m_object);
		return;
	}

	if ( m_bResetDelete )
	{
		fret->SetResetCap(RESET_DELETE);
	}

	m_displayText->DisplayError(INFO_CONVERT, m_object);
}

