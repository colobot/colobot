// shortcut.cpp

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
#include "shortcut.h"




// Constructeur de l'objet.

CShortcut::CShortcut(CInstanceManager* iMan) : CControl(iMan)
{
	CControl::CControl(iMan);
	m_time = 0.0f;
}

// Destructeur de l'objet.

CShortcut::~CShortcut()
{
	CControl::~CControl();
}


// Crée un nouveau bouton.

BOOL CShortcut::Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
	if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

	CControl::Create(pos, dim, icon, eventMsg);
	return TRUE;
}


// Gestion d'un événement.

BOOL CShortcut::EventProcess(const Event &event)
{
	CControl::EventProcess(event);

	if ( event.event == EVENT_FRAME )
	{
		m_time += event.rTime;
	}

	if ( event.event == EVENT_LBUTTONDOWN )
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

void CShortcut::Draw()
{
}

// Dessine le tableau des vertex.

void CShortcut::DrawVertex(int icon, float zoom)
{
	LPDIRECT3DDEVICE7 device;
	D3DVERTEX2	vertex[4];	// 2 triangles
	FPOINT		p1, p2, c;
	D3DVECTOR	n;
	float		u1, u2, v1, v2, dp;

	device = m_engine->RetD3DDevice();

	p1.x = m_pos.x;
	p1.y = m_pos.y;
	p2.x = m_pos.x + m_dim.x;
	p2.y = m_pos.y + m_dim.y;

	c.x = (p1.x+p2.x)/2.0f;
	c.y = (p1.y+p2.y)/2.0f;  // centre

	p1.x = (p1.x-c.x)*zoom + c.x;
	p1.y = (p1.y-c.y)*zoom + c.y;

	p2.x = (p2.x-c.x)*zoom + c.x;
	p2.y = (p2.y-c.y)*zoom + c.y;

	u1 = (32.0f/256.0f)*(icon%8);
	v1 = (32.0f/256.0f)*(icon/8);  // u-v texture
	u2 = (32.0f/256.0f)+u1;
	v2 = (32.0f/256.0f)+v1;

	dp = 0.5f/256.0f;
	u1 += dp;
	v1 += dp;
	u2 -= dp;
	v2 -= dp;

	n = D3DVECTOR(0.0f, 0.0f, -1.0f);  // normale

	vertex[0] = D3DVERTEX2(D3DVECTOR(p1.x, p1.y, 0.0f), n, u1,v2);
	vertex[1] = D3DVERTEX2(D3DVECTOR(p1.x, p2.y, 0.0f), n, u1,v1);
	vertex[2] = D3DVERTEX2(D3DVECTOR(p2.x, p1.y, 0.0f), n, u2,v2);
	vertex[3] = D3DVERTEX2(D3DVECTOR(p2.x, p2.y, 0.0f), n, u2,v1);

	device->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 4, NULL);
	m_engine->AddStatisticTriangle(2);
}

