// menu.cpp

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
#include "group.h"
#include "menu.h"





// Constructeur de l'objet.

CMenu::CMenu(CInstanceManager* iMan) : CControl(iMan)
{
	int		i;

	CControl::CControl(iMan);

	m_bCapture = FALSE;
	m_select = -1;
	m_total = 0;

	for ( i=0 ; i<20 ; i++ )
	{
		m_subIcon[i] = -1;
		m_buttons[i] = 0;
		m_texture[i][0] = 0;
	}
	m_group = 0;
}

// Destructeur de l'objet.

CMenu::~CMenu()
{
	DeleteSubMenu();
	CControl::~CControl();
}


// Crée un nouveau menu.

BOOL CMenu::Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
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

BOOL CMenu::EventProcess(const Event &event)
{
	if ( (m_state & STATE_VISIBLE) == 0 )  return TRUE;
	if ( m_state & STATE_DEAD )  return TRUE;

	CControl::EventProcess(event);

	if ( event.event == EVENT_LBUTTONDOWN &&
		 (m_state & STATE_VISIBLE)        &&
		 (m_state & STATE_ENABLE)         )
	{
		if ( CControl::Detect(event.pos) )
		{
			CreateSubMenu();
			m_bCapture = TRUE;
			return FALSE;
		}
	}

	if ( event.event == EVENT_MOUSEMOVE && m_bCapture )
	{
		DetectSubMenu(event.pos);
	}

	if ( event.event == EVENT_LBUTTONUP && m_bCapture )
	{
//?		if ( CControl::Detect(event.pos) )
		{
			Event newEvent = event;
			newEvent.event = m_eventMsg;
			m_event->AddEvent(newEvent);
		}

		DeleteSubMenu();
		m_bCapture = FALSE;
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

void CMenu::Draw()
{
	FPOINT	pos, dim;
	int		i;

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

	if ( m_group != 0 )
	{
		m_group->Draw();
	}

	for ( i=0 ; i<m_total ; i++ )
	{
		if ( m_buttons[i] != 0 )
		{
			m_buttons[i]->Draw();
		}
	}

	CControl::Draw();
}


// Crée les boutons du sous-menu.

void CMenu::CreateSubMenu()
{
	FPOINT		pos, dim;
	int			i;

	pos.x = m_pos.x+m_dim.x;
	pos.y = m_pos.y-5.0f/480.0f;
	dim.x = m_dim.x*m_total+10.0f/640.0f;
	dim.y = m_dim.y+10.0f/480.0f;
	m_group = new CGroup(m_iMan);
	m_group->Create(pos, dim, 31, EVENT_NULL);

	pos = m_pos;
	dim = m_dim;
	pos.x += 5.0f/640.0f;
	for ( i=0 ; i<m_total ; i++ )
	{
		pos.x += dim.x;
		m_buttons[i] = new CButton(m_iMan);
		m_buttons[i]->Create(pos, dim, m_subIcon[i], (EventMsg)(EVENT_MENU0+i));
		m_buttons[i]->SetTexture(m_texture[i], m_uv1[i], m_uv2[i]);

		if ( i == m_select )
		{
			m_buttons[i]->SetState(STATE_PASSED, TRUE);
		}
	}

	m_initSelect = m_select;
	DetectSubMenu(m_pos);
}

// Met en évidence le sous-bouton survolé par la souris.

void CMenu::DetectSubMenu(FPOINT mouse)
{
	FPOINT	pos, dim;
	BOOL	bCheck;
	int		i, select;

	select = m_initSelect;
	bCheck = TRUE;
	for ( i=0 ; i<m_total ; i++ )
	{
		if ( m_buttons[i] == 0 )  continue;

		pos = m_buttons[i]->RetPos();
		dim = m_buttons[i]->RetDim();

		if ( mouse.x >= pos.x       &&
			 mouse.x <= pos.x+dim.x &&
			 mouse.y >= pos.y       &&
			 mouse.y <= pos.y+dim.y )
		{
			m_buttons[i]->SetState(STATE_CHECK, TRUE);
			select = i;
			m_select = i;
			bCheck = FALSE;
		}
		else
		{
			m_buttons[i]->SetState(STATE_CHECK, FALSE);
		}
	}

	SetIcon(m_subIcon[select]);
	SetTexture(m_texture[select], m_uv1[select], m_uv2[select]);

	SetState(STATE_CHECK, bCheck);
	SetState(STATE_HILIGHT, FALSE);
	SetState(STATE_PRESS, FALSE);
}

// Détruit les boutons du sous-menu.

void CMenu::DeleteSubMenu()
{
	int		i;

	if ( m_group != 0 )
	{
		delete m_group;
		m_group = 0;
	}

	for ( i=0 ; i<m_total ; i++ )
	{
		if ( m_buttons[i] == 0 )  continue;
		delete m_buttons[i];
		m_buttons[i] = 0;
	}
}


// Choix du sous-menu sélectionné.

void CMenu::SetSelect(int select)
{
	m_select = select;
}

int CMenu::RetSelect()
{
	return m_select;
}


// Choix du nombre de case.

void CMenu::SetSubTotal(int total)
{
	m_total = total;
}

int CMenu::RetSubTotal()
{
	return m_total;
}


// Choix des icônes du sous-menu.

void CMenu::SetSubIcon(int rank, int icon)
{
	if ( rank < 0 || rank >= 20 )  return;
	m_subIcon[rank] = icon;
}

int CMenu::RetSubIcon(int rank)
{
	if ( rank < 0 || rank >= 20 )  return -1;
	return m_subIcon[rank];
}

void CMenu::SetSubTexture(int rank, char *filename, FPOINT uv1, FPOINT uv2)
{
	if ( rank < 0 || rank >= 20 )  return;
	strcpy(m_texture[rank], filename);
	m_uv1[rank] = uv1;
	m_uv2[rank] = uv2;
}

