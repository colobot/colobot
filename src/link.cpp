// link.cpp

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
#include "link.h"



#define DX		(3.0f/640.0f)
#define DY		(3.0f/480.0f)



// Constructeur de l'objet.

CLink::CLink(CInstanceManager* iMan) : CControl(iMan)
{
	CControl::CControl(iMan);

	m_src.x = NAN;
}

// Destructeur de l'objet.

CLink::~CLink()
{
	CControl::~CControl();
}


// Crée un nouveau bouton.

BOOL CLink::Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
	if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

	CControl::Create(pos, dim, icon, eventMsg);
	return TRUE;
}


// Gestion d'un événement.

BOOL CLink::EventProcess(const Event &event)
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

void CLink::Draw()
{
	FPOINT			pos, dim, uv1, uv2;
	float			dp, h, l;

	if ( (m_state & STATE_VISIBLE) == 0 )  return;
	if ( m_src.x == NAN )  return;

	dp = 0.5f/256.0f;

	m_engine->SetTexture("button1.tga");
	m_engine->SetState(D3DSTATENORMAL);

	if ( m_bGreen )
	{
		uv1.x = 160.0f/256.0f;  // vert
		uv1.y =  16.0f/256.0f;
		uv2.x = 176.0f/256.0f;
		uv2.y =  32.0f/256.0f;
	}
	else
	{
		uv1.x = 176.0f/256.0f;  // rouge
		uv1.y =  16.0f/256.0f;
		uv2.x = 192.0f/256.0f;
		uv2.y =  32.0f/256.0f;
	}
	uv1.x += dp;
	uv1.y += dp;
	uv2.x -= dp;
	uv2.y -= dp;

	h = m_src.y-m_dst.y-DY/2.0f-10.0f/480.0f;
	l = m_dst.x-m_src.x+DX/2.0f-8.0f/640.0f;

	pos.x = m_src.x;
	pos.y = m_src.y-DY/2.0f;
	dim.x = 8.0f/640.0f;
	dim.y = DY;
	DrawIcon(pos, dim, uv1, uv2);

	pos.x = m_src.x+8.0f/640.0f-DX;
	pos.y = m_src.y-DY/2.0f-h;
	dim.x = DX;
	dim.y = h;
	DrawIcon(pos, dim, uv1, uv2);

	if ( l > 0.0f )
	{
		pos.x = m_src.x+8.0f/640.0f;
		pos.y = m_src.y-DY/2.0f-h;
		dim.x = l;
		dim.y = DY;
		DrawIcon(pos, dim, uv1, uv2);
	}
	else
	{
		pos.x = m_dst.x-DX/2.0f;
		pos.y = m_src.y-DY/2.0f-h;
		dim.x = -l;
		dim.y = DY;
		DrawIcon(pos, dim, uv1, uv2);
	}

	pos.x = m_dst.x-DX/2.0f;
	pos.y = m_dst.y;
	dim.x = DX;
	dim.y = 10.0f/480.0f;
	DrawIcon(pos, dim, uv1, uv2);
}


void CLink::SetPoints(FPOINT src, FPOINT dst, BOOL bGreen)
{
	m_src = src;
	m_dst = dst;
	m_bGreen = bGreen;
}

