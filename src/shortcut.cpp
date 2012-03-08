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
	float		zoom;
	int			icon, mode;

	icon = 0;
	zoom = 0.8f;
	mode = D3DSTATETTw;
	if ( m_state & STATE_HILIGHT )
	{
		icon = 4;
		zoom = 0.9f;
		mode = D3DSTATENORMAL;
	}
	if ( m_state & STATE_CHECK )
	{
		icon = 1;
		zoom = 0.8f;
		mode = D3DSTATENORMAL;
	}
	if ( m_state & STATE_PRESS )
	{
		icon = 1;
		zoom = 1.0f;
		mode = D3DSTATENORMAL;
	}
	if ( m_icon == 6 || m_icon == 7 )  // pause ou film ?
	{
		icon = -1;  // pas de fond
		zoom = 1.0f;
	}

	m_engine->SetTexture("button3.tga");

	if ( icon != -1 )
	{
		m_engine->SetState(mode);
		DrawVertex(icon, 0.95f);
	}

	m_engine->SetState(D3DSTATETTb);
	DrawVertex(m_icon, zoom);

	if ( m_state & STATE_FRAME )
	{
		FPOINT	p1, p2, c, uv1, uv2;
		float	zoom, dp;

		m_engine->SetTexture("button2.tga");
		m_engine->SetState(D3DSTATETTw);

		zoom = 0.9f+sinf(m_time*8.0f)*0.1f;

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

		p2.x -= p1.x;
		p2.y -= p1.y;

		uv1.x = 176.0f/256.0f;
		uv1.y = 224.0f/256.0f;
		uv2.x = 192.0f/256.0f;
		uv2.y = 240.0f/256.0f;

		dp = 0.5f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;

		DrawIcon(p1, p2, uv1, uv2);
	}

	if ( (m_state & STATE_RUN) && Mod(m_time, 0.7f) >= 0.3f )
	{
		FPOINT	uv1, uv2;
		float	dp;

		m_engine->SetTexture("button3.tga");
		m_engine->SetState(D3DSTATETTw);

		uv1.x = 160.0f/256.0f;
		uv1.y =   0.0f/256.0f;
		uv2.x = 192.0f/256.0f;
		uv2.y =  32.0f/256.0f;

		dp = 0.5f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;

		DrawIcon(m_pos, m_dim, uv1, uv2);
	}
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

