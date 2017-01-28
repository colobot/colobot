// button.cpp

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
#include "button.h"



#define DELAY1		0.4f
#define DELAY2		0.1f



// Constructeur de l'objet.

CButton::CButton(CInstanceManager* iMan) : CControl(iMan)
{
	CControl::CControl(iMan);

	m_bCapture = FALSE;
	m_bImmediat = FALSE;
	m_bRepeat = FALSE;
	m_repeat = 0.0f;
}

// Destructeur de l'objet.

CButton::~CButton()
{
	CControl::~CControl();
}


// Crée un nouveau bouton.

BOOL CButton::Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
	if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

	CControl::Create(pos, dim, icon, eventMsg);

	if ( icon == -1 )
	{
		char	name[100];
		char*	p;

		GetResource(RES_EVENT, eventMsg, name);
		p = strchr(name, '\\');
		if ( p != 0 )  *p = 0;
		SetName(name);
	}

	return TRUE;
}


// Gestion d'un événement.

BOOL CButton::EventProcess(const Event &event)
{
	if ( (m_state & STATE_VISIBLE) == 0 )  return TRUE;
	if ( m_state & STATE_DEAD )  return TRUE;

	CControl::EventProcess(event);

	if ( event.event == EVENT_FRAME )
	{
		if ( m_bRepeat && m_repeat != 0.0f )
		{
			m_repeat -= event.rTime;
			if ( m_repeat <= 0.0f )
			{
				m_repeat = DELAY2;

				Event newEvent = event;
				newEvent.event = m_eventMsg;
				m_event->AddEvent(newEvent);
				return FALSE;
			}
		}
	}

	if ( event.event == EVENT_LBUTTONDOWN &&
		 (m_state & STATE_VISIBLE)        &&
		 (m_state & STATE_ENABLE)         )
	{
		if ( CControl::Detect(event.pos) )
		{
			m_bCapture = TRUE;
			m_repeat = DELAY1;

			if ( m_bImmediat || m_bRepeat )
			{
				Event newEvent = event;
				newEvent.event = m_eventMsg;
				m_event->AddEvent(newEvent);
			}
			return FALSE;
		}
	}

	if ( event.event == EVENT_MOUSEMOVE && m_bCapture )
	{
	}

	if ( event.event == EVENT_LBUTTONUP && m_bCapture )
	{
		if ( CControl::Detect(event.pos) )
		{
			if ( !m_bImmediat && !m_bRepeat )
			{
				Event newEvent = event;
				newEvent.event = m_eventMsg;
				m_event->AddEvent(newEvent);
			}
		}

		m_bCapture = FALSE;
		m_repeat = 0.0f;
	}

	if ( m_bFocus &&
		 (m_state & STATE_VISIBLE) &&
		 (m_state & STATE_ENABLE)  &&
		 event.event == EVENT_KEYDOWN &&
		 (event.param == VK_RETURN ||
		  event.param == VK_BUTTON1) )
	{
		Event newEvent = event;
		newEvent.event = m_eventMsg;
		m_event->AddEvent(newEvent);
	}

	return TRUE;
}


// Dessine le bouton.

void CButton::Draw()
{
	FPOINT	pos, dim;

	if ( (m_state & STATE_VISIBLE) == 0 )  return;

	if ( m_state & STATE_WARNING )  // hachures jaunes-noires ?
	{
		pos.x = m_pos.x-(4.0f/640.0f);
		pos.y = m_pos.y-(4.0f/480.0f);
		dim.x = m_dim.x+(8.0f/640.0f);
		dim.y = m_dim.y+(8.0f/480.0f);
		if ( m_state & STATE_SHADOW )
		{
			DrawShadow(pos, dim);
		}
		DrawWarning(pos, dim);
	}

	if ( m_state & STATE_SHADOW )
	{
		DrawShadow(m_pos, m_dim);
	}
	if ( m_bFocus )
	{
		DrawFocus(m_pos, m_dim);
	}

	CControl::Draw();
}


// Gestion du mode immédiat, qui envoie l'événement "pressé"
// avant que le bouton de la souris soit relâché.

void CButton::SetImmediat(BOOL bImmediat)
{
	m_bImmediat = bImmediat;
}

BOOL CButton::RetImmediat()
{
	return m_bImmediat;
}


// Gestion du mode "répétition automatique", lorsque le bouton
// de la souris est maintenu pressé.

void CButton::SetRepeat(BOOL bRepeat)
{
	m_bRepeat = bRepeat;
}

BOOL CButton::RetRepeat()
{
	return m_bRepeat;
}

