// gauge.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "D3DEngine.h"
#include "language.h"
#include "math3d.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "gauge.h"




// Constructeur de l'objet.

CGauge::CGauge(CInstanceManager* iMan) : CControl(iMan)
{
	CControl::CControl(iMan);

	m_level = 0.0f;
	m_mode = GM_NORMAL;
}

// Destructeur de l'objet.

CGauge::~CGauge()
{
	CControl::~CControl();
}


// Crée un nouveau bouton.

BOOL CGauge::Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
	if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

	CControl::Create(pos, dim, icon, eventMsg);
	return TRUE;
}


// Gestion d'un événement.

BOOL CGauge::EventProcess(const Event &event)
{
	CControl::EventProcess(event);

	if ( event.event == EVENT_MOUSEMOVE && Detect(event.pos) )
	{
		m_engine->SetMouseType(D3DMOUSENORM);
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


// Dessine la jauge.

void CGauge::Draw()
{
	LPDIRECT3DDEVICE7 device;
	D3DVECTOR		n;
	D3DVERTEX2		vertex[4];	// 2 triangles
	FPOINT			p1, p2, p3, p4, c;
	FPOINT			pos, dim, uv1, uv2, corner;
	float			dp, angle;

	if ( (m_state & STATE_VISIBLE) == 0 )  return;

	device = m_engine->RetD3DDevice();
	dp = 0.5f/256.0f;

	if ( m_mode == GM_NORMAL )
	{
		if ( m_state & STATE_SHADOW )
		{
			DrawShadow(m_pos, m_dim);
		}

		m_engine->SetTexture("button1.tga");
		m_engine->SetState(D3DSTATENORMAL);

		pos = m_pos;
		dim = m_dim;

		uv1.x =  64.0f/256.0f;
		uv1.y =   0.0f/256.0f;
		uv2.x =  96.0f/256.0f;
		uv2.y =  32.0f/256.0f;

		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;

		corner.x = 5.0f/640.0f;
		corner.y = 5.0f/480.0f;

		DrawIcon(pos, dim, uv1, uv2, corner, 8.0f/256.0f);


		pos.x +=  5.0f/640.0f;
		pos.y +=  5.0f/480.0f;
		dim.x -= 10.0f/640.0f;
		dim.y -= 10.0f/480.0f;

		if ( m_dim.x < m_dim.y )  // jauge verticale ?
		{
			uv1.x = (0.0f+m_icon*16.0f)/256.0f;
			uv2.x = uv1.x+16.0f/256.0f;
			uv1.y = 128.0f/256.0f+m_level*(64.0f/256.0f);
			uv2.y = uv1.y+64.0f/256.0f;
		}
		else	// jauge horizontale ?
		{
			uv1.x = 0.0f/256.0f+(1.0f-m_level)*(64.0f/256.0f);
			uv2.x = uv1.x+64.0f/256.0f;
			uv1.y = (128.0f+m_icon*16.0f)/256.0f;
			uv2.y = uv1.y+16.0f/256.0f;
		}

		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;

		DrawIcon(pos, dim, uv1, uv2);
	}

	if ( m_mode == GM_SPEED )
	{
		pos = m_pos;
		dim = m_dim;

#if _FRENCH
		m_engine->SetTexture("board-f.tga");
#endif
#if _ENGLISH
		m_engine->SetTexture("board-e.tga");
#endif
#if _DEUTSCH
		m_engine->SetTexture("board-d.tga");
#endif
#if _ITALIAN
		m_engine->SetTexture("board-f.tga");
#endif
#if _SPANISH
		m_engine->SetTexture("board-f.tga");
#endif
#if _PORTUGUESE
		m_engine->SetTexture("board-p.tga");
#endif
		m_engine->SetState(D3DSTATETTb);
		uv1.x =   0.0f/256.0f;
		uv1.y =   0.0f/256.0f;
		uv2.x = 128.0f/256.0f;
		uv2.y = 128.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		DrawIcon(pos, dim, uv1, uv2);

		m_engine->SetState(D3DSTATETTw);
		uv1.x = 128.0f/256.0f;
		uv1.y =   0.0f/256.0f;
		uv2.x = 256.0f/256.0f;
		uv2.y = 128.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		DrawIcon(pos, dim, uv1, uv2);

		m_engine->SetTexture("button1.tga");
		m_engine->SetState(D3DSTATENORMAL);
		n = D3DVECTOR(0.0f, 0.0f, -1.0f);  // normale

		angle = 150.0f-m_level*150.0f/100.0f;
		angle = angle*PI/180.0f;

		c.x = pos.x+dim.x/2.0f;
		c.y = pos.y+dim.y/2.0f;

		p1.x = c.x-dim.x*0.02f;
		p1.y = c.y+dim.x*0.27f;
		p1 = RotatePoint(c, angle, p1);
		p1.x = c.x+(p1.x-c.x)*(dim.x/dim.y);

		p2.x = c.x-dim.x*0.02f;
		p2.y = c.y+dim.x*0.50f;
		p2 = RotatePoint(c, angle, p2);
		p2.x = c.x+(p2.x-c.x)*(dim.x/dim.y);

		p3.x = c.x+dim.x*0.02f;
		p3.y = c.y+dim.x*0.27f;
		p3 = RotatePoint(c, angle, p3);
		p3.x = c.x+(p3.x-c.x)*(dim.x/dim.y);

		p4.x = c.x+dim.x*0.02f;
		p4.y = c.y+dim.x*0.50f;
		p4 = RotatePoint(c, angle, p4);
		p4.x = c.x+(p4.x-c.x)*(dim.x/dim.y);

		uv1.x =  24.0f/256.0f;
		uv1.y =  48.0f/256.0f;
		uv2.x =  32.0f/256.0f;
		uv2.y =  64.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;

		vertex[0] = D3DVERTEX2(D3DVECTOR(p1.x, p1.y, 0.0f), n, uv1.x,uv2.y);
		vertex[1] = D3DVERTEX2(D3DVECTOR(p2.x, p2.y, 0.0f), n, uv1.x,uv1.y);
		vertex[2] = D3DVERTEX2(D3DVECTOR(p3.x, p3.y, 0.0f), n, uv2.x,uv2.y);
		vertex[3] = D3DVERTEX2(D3DVECTOR(p4.x, p4.y, 0.0f), n, uv2.x,uv1.y);

		device->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 4, NULL);
		m_engine->AddStatisticTriangle(2);
	}

	if ( m_mode == GM_RPM )
	{
		pos = m_pos;
		dim = m_dim;

#if _FRENCH
		m_engine->SetTexture("board-f.tga");
#endif
#if _ENGLISH
		m_engine->SetTexture("board-e.tga");
#endif
#if _DEUTSCH
		m_engine->SetTexture("board-d.tga");
#endif
#if _ITALIAN
		m_engine->SetTexture("board-f.tga");
#endif
#if _SPANISH
		m_engine->SetTexture("board-f.tga");
#endif
#if _PORTUGUESE
		m_engine->SetTexture("board-p.tga");
#endif
		m_engine->SetState(D3DSTATETTb);
		uv1.x =   0.0f/256.0f;
		uv1.y =   0.0f/256.0f;
		uv2.x = 128.0f/256.0f;
		uv2.y = 128.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		DrawIcon(pos, dim, uv1, uv2);

		m_engine->SetState(D3DSTATETTw);
		uv1.x = 128.0f/256.0f;
		uv1.y = 128.0f/256.0f;
		uv2.x = 256.0f/256.0f;
		uv2.y = 256.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		DrawIcon(pos, dim, uv1, uv2);

		m_engine->SetTexture("button1.tga");
		m_engine->SetState(D3DSTATENORMAL);
		n = D3DVECTOR(0.0f, 0.0f, -1.0f);  // normale

		angle = 150.0f-m_level*150.0f/1.0f;
		angle = angle*PI/180.0f;

		c.x = pos.x+dim.x/2.0f;
		c.y = pos.y+dim.y/2.0f;

		p1.x = c.x-dim.x*0.02f;
		p1.y = c.y+dim.x*0.27f;
		p1 = RotatePoint(c, angle, p1);
		p1.x = c.x+(p1.x-c.x)*(dim.x/dim.y);

		p2.x = c.x-dim.x*0.02f;
		p2.y = c.y+dim.x*0.50f;
		p2 = RotatePoint(c, angle, p2);
		p2.x = c.x+(p2.x-c.x)*(dim.x/dim.y);

		p3.x = c.x+dim.x*0.02f;
		p3.y = c.y+dim.x*0.27f;
		p3 = RotatePoint(c, angle, p3);
		p3.x = c.x+(p3.x-c.x)*(dim.x/dim.y);

		p4.x = c.x+dim.x*0.02f;
		p4.y = c.y+dim.x*0.50f;
		p4 = RotatePoint(c, angle, p4);
		p4.x = c.x+(p4.x-c.x)*(dim.x/dim.y);

		uv1.x =  24.0f/256.0f;
		uv1.y =  48.0f/256.0f;
		uv2.x =  32.0f/256.0f;
		uv2.y =  64.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;

		vertex[0] = D3DVERTEX2(D3DVECTOR(p1.x, p1.y, 0.0f), n, uv1.x,uv2.y);
		vertex[1] = D3DVERTEX2(D3DVECTOR(p2.x, p2.y, 0.0f), n, uv1.x,uv1.y);
		vertex[2] = D3DVERTEX2(D3DVECTOR(p3.x, p3.y, 0.0f), n, uv2.x,uv2.y);
		vertex[3] = D3DVERTEX2(D3DVECTOR(p4.x, p4.y, 0.0f), n, uv2.x,uv1.y);

		device->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 4, NULL);
		m_engine->AddStatisticTriangle(2);
	}

	if ( m_mode == GM_COMPASS )
	{
		pos = m_pos;
		dim = m_dim;

#if _FRENCH
		m_engine->SetTexture("board-f.tga");
#endif
#if _ENGLISH
		m_engine->SetTexture("board-e.tga");
#endif
#if _DEUTSCH
		m_engine->SetTexture("board-d.tga");
#endif
#if _ITALIAN
		m_engine->SetTexture("board-f.tga");
#endif
#if _SPANISH
		m_engine->SetTexture("board-f.tga");
#endif
#if _PORTUGUESE
		m_engine->SetTexture("board-p.tga");
#endif
		m_engine->SetState(D3DSTATETTb);
		uv1.x =   0.0f/256.0f;
		uv1.y =   0.0f/256.0f;
		uv2.x = 128.0f/256.0f;
		uv2.y = 128.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		DrawIcon(pos, dim, uv1, uv2);

		m_engine->SetState(D3DSTATETTw);
		uv1.x =   0.0f/256.0f;
		uv1.y = 128.0f/256.0f;
		uv2.x =  64.0f/256.0f;
		uv2.y = 192.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		DrawIcon(pos, dim, uv1, uv2);

		m_engine->SetTexture("button1.tga");
		m_engine->SetState(D3DSTATENORMAL);
		n = D3DVECTOR(0.0f, 0.0f, -1.0f);  // normale

		angle = m_level;

		c.x = pos.x+dim.x/2.0f;
		c.y = pos.y+dim.y/2.0f;

		p1.x = c.x;
		p1.y = c.y+dim.x*0.50f;
		p1 = RotatePoint(c, angle, p1);
		p1.x = c.x+(p1.x-c.x)*(dim.x/dim.y);

		p2.x = c.x+dim.x*0.05f;
		p2.y = c.y+dim.x*0.10f;
		p2 = RotatePoint(c, angle, p2);
		p2.x = c.x+(p2.x-c.x)*(dim.x/dim.y);

		p3.x = c.x-dim.x*0.05f;
		p3.y = c.y+dim.x*0.10f;
		p3 = RotatePoint(c, angle, p3);
		p3.x = c.x+(p3.x-c.x)*(dim.x/dim.y);

		uv1.x =  24.0f/256.0f;
		uv1.y =  48.0f/256.0f;
		uv2.x =  32.0f/256.0f;
		uv2.y =  64.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;

		vertex[0] = D3DVERTEX2(D3DVECTOR(p1.x, p1.y, 0.0f), n, uv1.x,uv1.y);
		vertex[1] = D3DVERTEX2(D3DVECTOR(p2.x, p2.y, 0.0f), n, uv1.x,uv2.y);
		vertex[2] = D3DVERTEX2(D3DVECTOR(p3.x, p3.y, 0.0f), n, uv2.x,uv2.y);

		device->DrawPrimitive(D3DPT_TRIANGLELIST, D3DFVF_VERTEX2, vertex, 3, NULL);
		m_engine->AddStatisticTriangle(1);
	}

	if ( m_mode == GM_LEVEL1 ||
		 m_mode == GM_LEVEL2 )
	{
		pos = m_pos;
		dim = m_dim;

#if _FRENCH
		m_engine->SetTexture("board-f.tga");
#endif
#if _ENGLISH
		m_engine->SetTexture("board-e.tga");
#endif
#if _DEUTSCH
		m_engine->SetTexture("board-d.tga");
#endif
#if _ITALIAN
		m_engine->SetTexture("board-f.tga");
#endif
#if _SPANISH
		m_engine->SetTexture("board-f.tga");
#endif
#if _PORTUGUESE
		m_engine->SetTexture("board-p.tga");
#endif
		m_engine->SetState(D3DSTATETTb);
		uv1.x =   0.0f/256.0f;
		uv1.y =   0.0f/256.0f;
		uv2.x = 128.0f/256.0f;
		uv2.y = 128.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		DrawIcon(pos, dim, uv1, uv2);

		m_engine->SetState(D3DSTATETTw);
		if ( m_mode == GM_LEVEL1 )
		{
			uv1.x =   0.0f/256.0f;
			uv1.y = 192.0f/256.0f;
			uv2.x =  64.0f/256.0f;
			uv2.y = 256.0f/256.0f;
		}
		else
		{
			uv1.x =  64.0f/256.0f;
			uv1.y = 192.0f/256.0f;
			uv2.x = 128.0f/256.0f;
			uv2.y = 256.0f/256.0f;
		}
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		DrawIcon(pos, dim, uv1, uv2);

		m_engine->SetTexture("button1.tga");
		m_engine->SetState(D3DSTATENORMAL);
		n = D3DVECTOR(0.0f, 0.0f, -1.0f);  // normale

		angle = 45.0f-m_level*45.0f/0.5f;
		angle = angle*PI/180.0f;

		c.x = pos.x+dim.x*0.50f;
		c.y = pos.y+dim.y*0.16f;

		p1.x = c.x-dim.x*0.02f;
		p1.y = c.y+dim.x*0.10f;
		p1 = RotatePoint(c, angle, p1);
		p1.x = c.x+(p1.x-c.x)*(dim.x/dim.y);

		p2.x = c.x-dim.x*0.02f;
		p2.y = c.y+dim.x*0.60f;
		p2 = RotatePoint(c, angle, p2);
		p2.x = c.x+(p2.x-c.x)*(dim.x/dim.y);

		p3.x = c.x+dim.x*0.02f;
		p3.y = c.y+dim.x*0.10f;
		p3 = RotatePoint(c, angle, p3);
		p3.x = c.x+(p3.x-c.x)*(dim.x/dim.y);

		p4.x = c.x+dim.x*0.02f;
		p4.y = c.y+dim.x*0.60f;
		p4 = RotatePoint(c, angle, p4);
		p4.x = c.x+(p4.x-c.x)*(dim.x/dim.y);

		uv1.x =  24.0f/256.0f;
		uv1.y =  48.0f/256.0f;
		uv2.x =  32.0f/256.0f;
		uv2.y =  64.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;

		vertex[0] = D3DVERTEX2(D3DVECTOR(p1.x, p1.y, 0.0f), n, uv1.x,uv2.y);
		vertex[1] = D3DVERTEX2(D3DVECTOR(p2.x, p2.y, 0.0f), n, uv1.x,uv1.y);
		vertex[2] = D3DVERTEX2(D3DVECTOR(p3.x, p3.y, 0.0f), n, uv2.x,uv2.y);
		vertex[3] = D3DVERTEX2(D3DVECTOR(p4.x, p4.y, 0.0f), n, uv2.x,uv1.y);

		device->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 4, NULL);
		m_engine->AddStatisticTriangle(2);
	}
}


// Gestion du niveau de la jauge.

void CGauge::SetLevel(float level)
{
	if ( m_mode == GM_SPEED )
	{
		if ( level <   0.0f )  level =   0.0f;
		if ( level > 200.0f )  level = 200.0f;
	}
	else if ( m_mode == GM_RPM )
	{
		if ( level < 0.0f )  level = 0.0f;
		if ( level > 2.0f )  level = 2.0f;
	}
	else if ( m_mode == GM_COMPASS )
	{
		level = NormAngle(level);  // 0..PI*2
	}
	else
	{
		level = Norm(level);  // 0..1
	}

	m_level = level;
}

float CGauge::RetLevel()
{
	return m_level;
}


// Gestion du mode.

void CGauge::SetMode(GaugeMode mode)
{
	FPOINT	pos;

	m_mode = mode;

	if ( m_mode != GM_NORMAL )
	{
		pos.x = m_pos.x+m_dim.x*0.1f;
		pos.y = m_pos.y+m_dim.y*0.9f;
		GlintCreate(pos);
	}
}

GaugeMode CGauge::RetMode()
{
	return m_mode;
}


