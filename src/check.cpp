// check.cpp

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
#include "restext.h"
#include "text.h"
#include "check.h"




// Constructeur de l'objet.

CCheck::CCheck(CInstanceManager* iMan) : CControl(iMan)
{
	CControl::CControl(iMan);
}

// Destructeur de l'objet.

CCheck::~CCheck()
{
	CControl::~CControl();
}


// Crée un nouveau bouton.

BOOL CCheck::Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
	char	name[100];
	char*	p;

	if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

	CControl::Create(pos, dim, icon, eventMsg);

	GetResource(RES_EVENT, eventMsg, name);
	p = strchr(name, '\\');
	if ( p != 0 )  *p = 0;
	SetName(name);

	return TRUE;
}


// Gestion d'un événement.

BOOL CCheck::EventProcess(const Event &event)
{
	if ( m_state & STATE_DEAD )  return TRUE;

	CControl::EventProcess(event);

	if ( event.event == EVENT_LBUTTONDOWN &&
		 (m_state & STATE_VISIBLE)        &&
		 (m_state & STATE_ENABLE)         )
	{
		if ( CControl::Detect(event.pos) )
		{
			Event newEvent = event;
			newEvent.event = m_eventMsg;
			m_event->AddEvent(newEvent);
			return FALSE;
		}
	}

	return TRUE;
}


// Dessine le bouton.

void CCheck::Draw()
{
	FPOINT		iDim, pos;
	float		zoomExt, zoomInt;
	int			icon;

	if ( (m_state & STATE_VISIBLE) == 0 )  return;

	iDim = m_dim;
	m_dim.x = m_dim.y*0.75f;  // carré

	if ( m_state & STATE_SHADOW )
	{
		DrawShadow(m_pos, m_dim);
	}

	m_engine->SetTexture("button1.tga");
	m_engine->SetState(D3DSTATENORMAL);

	zoomExt = 1.00f;
	zoomInt = 0.95f;

	icon = 2;
	if ( m_state & STATE_DEFAULT )
	{
		DrawPart(23, 1.3f, 0.0f);

		zoomExt *= 1.15f;
		zoomInt *= 1.15f;
	}
	if ( m_state & STATE_HILIGHT )
	{
		icon = 1;
	}
	if ( m_state & STATE_PRESS )
	{
		icon = 3;
		zoomInt *= 0.9f;
	}
	if ( (m_state & STATE_ENABLE) == 0 )
	{
		icon = 7;
	}
	if ( m_state & STATE_DEAD )
	{
		icon = 17;
	}
	DrawPart(icon, zoomExt, 0.0f);  // dessine le bouton

	if ( (m_state & STATE_DEAD) == 0 )
	{
		m_engine->SetState(D3DSTATETTw);

		if ( m_state & STATE_CHECK )
		{
			icon = 16;  // vu
			DrawPart(icon, zoomInt, 0.0f);  // dessine l'icône
		}
	}

	m_dim = iDim;

	if ( m_state & STATE_DEAD )  return;

	// Dessine le nom.
	pos.x = m_pos.x+m_dim.y/0.9f;
	pos.y = m_pos.y+m_dim.y*0.50f;
	pos.y -= m_engine->RetText()->RetHeight(m_fontSize, m_fontType)/2.0f;
	m_engine->RetText()->DrawText(m_name, pos, m_dim.x, 1, m_fontSize, m_fontStretch, m_fontType, 0);
}


