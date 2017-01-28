// key.cpp

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
#include "sound.h"
#include "text.h"
#include "key.h"




// COnstruit le nom d'une touche.

void GetKeyName(char *name, int key)
{
	if ( !GetResource(RES_KEY, key, name) )
	{
		if ( (key >= '0' && key <= '9') ||
			 (key >= 'A' && key <= 'Z') ||
			 (key >= 'a' && key <= 'z') )
		{
			name[0] = key;
			name[1] = 0;
		}
		else
		{
			sprintf(name, "Code %d", key);
		}
	}
}




// Constructeur de l'objet.

CKey::CKey(CInstanceManager* iMan) : CControl(iMan)
{
	CControl::CControl(iMan);

	m_key[0] = 0;
	m_key[1] = 0;
	m_bCatch = FALSE;
}

// Destructeur de l'objet.

CKey::~CKey()
{
	CControl::~CControl();
}


// Crée un nouveau bouton.

BOOL CKey::Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
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

BOOL CKey::EventProcess(const Event &event)
{
	if ( m_state & STATE_DEAD )  return TRUE;

	CControl::EventProcess(event);

	if ( event.event == EVENT_LBUTTONDOWN )
	{
		if ( Detect(event.pos) )
		{
			m_bCatch = TRUE;
		}
		else
		{
			m_bCatch = FALSE;
		}
	}

	if ( event.event == EVENT_KEYDOWN && m_bCatch )
	{
		m_bCatch = FALSE;

		if ( TestKey(event.param) )  // impossible ?
		{
			m_sound->Play(SOUND_TZOING);
		}
		else
		{
			if ( event.param == m_key[0] ||
				 event.param == m_key[1] )
			{
				m_key[0] = event.param;
				m_key[1] = 0;
			}
			else
			{
				m_key[1] = m_key[0];
				m_key[0] = event.param;
			}
			m_sound->Play(SOUND_CLICK);

			Event newEvent = event;
			newEvent.event = m_eventMsg;
			m_event->AddEvent(newEvent);
		}
		return FALSE;
	}

	return TRUE;
}


// Cherche si une touche est déjà utilisée.

BOOL CKey::TestKey(int key)
{
	int		i, j;

	if ( key == VK_PAUSE    ||
		 key == VK_SNAPSHOT )  return TRUE;  // touche bloquée

	for ( i=0 ; i<20 ; i++ )
	{
		for ( j=0 ; j<2 ; j++ )
		{
			if ( key == m_engine->RetKey(i, j) )  // touche utilisée ?
			{
				m_engine->SetKey(i, j, 0);  // plus rien !
			}
		}

		if ( m_engine->RetKey(i, 0) == 0 )  // première option libre ?
		{
			m_engine->SetKey(i, 0, m_engine->RetKey(i, 1));  // shift
			m_engine->SetKey(i, 1, 0);
		}
	}

	return FALSE;  // pas utilisée
}


// Dessine le bouton.

void CKey::Draw()
{
	FPOINT		iDim, pos;
	float		zoomExt, zoomInt, h;
	int			icon;
	char		text[100];

	if ( (m_state & STATE_VISIBLE) == 0 )  return;

	iDim = m_dim;
	m_dim.x = 200.0f/640.0f;

	if ( m_state & STATE_SHADOW )
	{
		DrawShadow(m_pos, m_dim);
	}

	m_engine->SetTexture("button1.tga");
//?	m_engine->SetState(D3DSTATENORMAL);
	m_engine->SetState(D3DSTATETTb);

	zoomExt = 1.00f;
	zoomInt = 0.95f;

	icon = 2;
	if ( m_key[0] == 0 &&
		 m_key[1] == 0 )  // pas de raccourci ?
	{
		icon = 3;
	}
	if ( m_state & STATE_HILIGHT )
	{
		icon = 1;
	}
	if ( m_state & STATE_CHECK )
	{
		icon = 0;
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
	if ( m_bCatch )
	{
		icon = 23;
	}
	DrawPart(icon, zoomExt, 8.0f/256.0f);  // dessine le bouton

	h = m_engine->RetText()->RetHeight(m_fontSize, m_fontType)/2.0f;

	GetKeyName(text, m_key[0]);
	if ( m_key[1] != 0 )
	{
		GetResource(RES_TEXT, RT_KEY_OR, text+strlen(text));
		GetKeyName(text+strlen(text), m_key[1]);
	}

	pos.x = m_pos.x+m_dim.x*0.5f;
	pos.y = m_pos.y+m_dim.y*0.5f;
	pos.y -= h;
	m_engine->RetText()->DrawText(text, pos, m_dim.x, 0, m_fontSize, m_fontStretch, m_fontType, 0);

	m_dim = iDim;

	if ( m_state & STATE_DEAD )  return;

	// Dessine le nom.
	pos.x = m_pos.x+(214.0f/640.0f);
	pos.y = m_pos.y+m_dim.y*0.5f;
	pos.y -= h;
	m_engine->RetText()->DrawText(m_name, pos, m_dim.x, 1, m_fontSize, m_fontStretch, m_fontType, 0);
}



void CKey::SetKey(int option, int key)
{
	if ( option < 0 ||
		 option > 1 )  return;

	m_key[option] = key;
}

int CKey::RetKey(int option)
{
	if ( option < 0 ||
		 option > 1 )  return 0;

	return m_key[option];
}

