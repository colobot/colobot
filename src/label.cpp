// label.cpp

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
#include "text.h"
#include "label.h"




// Constructeur de l'objet.

CLabel::CLabel(CInstanceManager* iMan) : CControl(iMan)
{
	CControl::CControl(iMan);
}

// Destructeur de l'objet.

CLabel::~CLabel()
{
	CControl::~CControl();
}


// Crée un nouveau bouton.

BOOL CLabel::Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
	if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

	CControl::Create(pos, dim, icon, eventMsg);
	return TRUE;
}


// Gestion d'un événement.

BOOL CLabel::EventProcess(const Event &event)
{
//?	CControl::EventProcess(event);
	return TRUE;
}


// Dessine le bouton.

void CLabel::Draw()
{
	FPOINT	pos;

	if ( (m_state & STATE_VISIBLE) == 0 )  return;

	pos.y = m_pos.y+m_dim.y/2.0f;

	if ( m_justif > 0 )
	{
		pos.x = m_pos.x;
	}
	if ( m_justif == 0 )
	{
		pos.x = m_pos.x+m_dim.x/2.0f;
	}
	if ( m_justif < 0 )
	{
		pos.x = m_pos.x+m_dim.x;
	}
	m_engine->RetText()->DrawText(m_name, pos, m_dim.x, m_justif, m_fontSize, m_fontStretch, m_fontType, 0);
}

