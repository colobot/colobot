// pesetas.cpp

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
#include "restext.h"
#include "text.h"
#include "pesetas.h"




// Constructeur de l'objet.

CPesetas::CPesetas(CInstanceManager* iMan) : CControl(iMan)
{
	CControl::CControl(iMan);

	m_levelCredit  = 0.0f;
	m_levelMission = 0.0f;
	m_levelNext    = 0.0f;
}

// Destructeur de l'objet.

CPesetas::~CPesetas()
{
	CControl::~CControl();
}


// Crée un nouveau bouton.

BOOL CPesetas::Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
	if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

	CControl::Create(pos, dim, icon, eventMsg);
	return TRUE;
}


// Gestion d'un événement.

BOOL CPesetas::EventProcess(const Event &event)
{
	CControl::EventProcess(event);

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

void CPesetas::Draw()
{
	FPOINT			pos, dim, ppos, ddim, uv1, uv2, corner;
	float			dp, width, x, xx, zoom;
	char			text[100];

	if ( (m_state & STATE_VISIBLE) == 0 )  return;

	dp = 0.5f/256.0f;

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

	corner.x = 10.0f/640.0f;
	corner.y = 10.0f/480.0f;

	DrawIcon(pos, dim, uv1, uv2, corner, 8.0f/256.0f);

	pos.x +=  6.0f/640.0f;
	pos.y +=  6.0f/480.0f;
	dim.x -= 12.0f/640.0f;
	dim.y -= 12.0f/480.0f;

	ppos.y = pos.y+dim.y*0.88f;
	ddim.y = dim.y*0.12f;
	ppos.x = pos.x;
	ddim.x = dim.x*0.10f;
	DrawCell(ppos, ddim, 11, "", 0);
	ppos.x = pos.x+dim.x*0.10f;
	ddim.x = dim.x*0.60f;
	GetResource(RES_TEXT, RT_PLAY_PCREDIT, text);
	DrawCell(ppos, ddim, 1, text, 1);
	ppos.x = pos.x+dim.x*0.70f;
	ddim.x = dim.x*0.30f;
	sprintf(text, "%d ", (int)m_levelCredit);
	DrawCell(ppos, ddim, 1, text, -1);

	ppos.y = pos.y+dim.y*0.76f;
	ddim.y = dim.y*0.12f;
	ppos.x = pos.x;
	ddim.x = dim.x*0.10f;
	DrawCell(ppos, ddim, 12, "", 0);
	ppos.x = pos.x+dim.x*0.10f;
	ddim.x = dim.x*0.60f;
	GetResource(RES_TEXT, RT_PLAY_PMISSION, text);
	DrawCell(ppos, ddim, 2, text, 1);
	ppos.x = pos.x+dim.x*0.70f;
	ddim.x = dim.x*0.30f;
	sprintf(text, "%d ", (int)m_levelMission);
	DrawCell(ppos, ddim, 2, text, -1);

#if _SE
	if ( m_levelNext < 150.0f )
#else
	if ( m_levelNext < 40000.0f )
#endif
	{
		ppos.y = pos.y;
		ddim.y = dim.y*0.12f;
		ppos.x = pos.x;
		ddim.x = dim.x*0.10f;
		DrawCell(ppos, ddim, 13, "", 0);
		ppos.x = pos.x+dim.x*0.10f;
		ddim.x = dim.x*0.60f;
		GetResource(RES_TEXT, RT_PLAY_PNEXT, text);
		DrawCell(ppos, ddim, 3, text, 1);
		ppos.x = pos.x+dim.x*0.70f;
		ddim.x = dim.x*0.30f;
		if ( m_levelNext == 0.0f )  sprintf(text, " ");
		else                        sprintf(text, "%d ", (int)m_levelNext);
		DrawCell(ppos, ddim, 3, text, -1);
	}

	// Dessine la jauge.
	m_engine->SetTexture("button1.tga");

	ppos.x = pos.x+dim.x*0.10f;
	ppos.y = pos.y+dim.y*0.34f;
	ddim.x = dim.x*0.80f;
	ddim.y = dim.y*0.20f;
	DrawShadow(ppos, ddim);
	m_engine->SetState(D3DSTATENORMAL);
	uv1.x =  64.0f/256.0f;
	uv1.y =   0.0f/256.0f;
	uv2.x =  96.0f/256.0f;
	uv2.y =  32.0f/256.0f;
	uv1.x += dp;
	uv1.y += dp;
	uv2.x -= dp;
	uv2.y -= dp;
	corner.x = 10.0f/640.0f;
	corner.y = 10.0f/480.0f;
	DrawIcon(ppos, ddim, uv1, uv2, corner, 8.0f/256.0f);  // fond

	if ( m_levelNext != 0.0f )
	{
		if ( m_levelCredit+m_levelMission <= m_levelNext )
		{
			zoom = 1.0f;
		}
		else
		{
			zoom = m_levelNext/(m_levelCredit+m_levelMission);
		}

		ppos.x += 4.0f/640.0f;
		ppos.y += 4.0f/480.0f;
		ddim.x -= 8.0f/640.0f;
		ddim.y -= 8.0f/480.0f;
		ddim.x *= zoom;
		uv1.x =   0.0f/256.0f;
		uv1.y = 160.0f/256.0f;
		uv2.x =  64.0f/256.0f;
		uv2.y = 176.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		DrawIcon(ppos, ddim, uv1, uv2);  // rouge
		width = ddim.x;

		ddim.x = width*(m_levelCredit/m_levelNext);
		ppos.y += 4.0f/480.0f;
		ddim.y -= 8.0f/480.0f;
		uv1.x =   0.0f/256.0f;
		uv1.y = 128.0f/256.0f;
		uv2.x =  64.0f/256.0f;
		uv2.y = 144.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		DrawIcon(ppos, ddim, uv1, uv2);  // vert

		ppos.x += ddim.x;
		ddim.x = width*(m_levelMission/m_levelNext);
		uv1.x =  64.0f/256.0f;
		uv1.y = 160.0f/256.0f;
		uv2.x = 128.0f/256.0f;
		uv2.y = 176.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		DrawIcon(ppos, ddim, uv1, uv2);  // bleu

		// Dessine les lignes.
		m_engine->SetState(D3DSTATETTw);

		if ( m_levelCredit > 0.0f )
		{
			DrawDot(pos, dim, 0.03f, 0.94f);
			DrawVLine(pos, dim, 0.03f, 0.94f, 0.44f);
			x = 0.10f+4.0f/640.0f/dim.x+width*(m_levelCredit/m_levelNext)/2.0f/dim.x;
			DrawHLine(pos, dim, 0.03f, 0.44f, x);
			DrawDot(pos, dim, x, 0.44f);
			xx = 0.06f;
		}
		else
		{
			xx = 0.05f;
		}

		DrawDot(pos, dim, xx, 0.82f);
		DrawVLine(pos, dim, xx, 0.82f, 0.62f);
		x = 0.10f+4.0f/640.0f/dim.x+width*(m_levelCredit/m_levelNext)/dim.x+width*(m_levelMission/m_levelNext)/2.0f/dim.x;
		DrawHLine(pos, dim, xx, 0.62f, x);
		DrawVLine(pos, dim, x, 0.62f+1.0f/480.0f/dim.y, 0.44f);
		DrawDot(pos, dim, x, 0.44f);

#if _SE
		if ( m_levelNext < 150.0f )
#else
		if ( m_levelNext < 40000.0f )
#endif
		{
			DrawDot(pos, dim, 0.05f, 0.06f);
			DrawVLine(pos, dim, 0.05f, 0.06f, 0.20f);
			x = 0.10f+2.0f/640.0f/dim.x+0.40f*zoom;
			DrawHLine(pos, dim, 0.05f, 0.20f, x);
			DrawVLine(pos, dim, x, 0.20f, 0.36f);
			DrawDot(pos, dim, x, 0.36f);
		}
	}
}

// Dessine une cellule.

void CPesetas::DrawCell(FPOINT pos, FPOINT dim, int icon, char *text, int justif)
{
	FPOINT		ppos, ddim, uv1, uv2;
	float		dp;

	dp = 0.5f/256.0f;

	m_engine->SetTexture("button1.tga");
	if ( icon < 10 )  m_engine->SetState(D3DSTATETTb);
	else              m_engine->SetState(D3DSTATENORMAL);

	ppos.x = pos.x+1.0f/640.0f;
	ppos.y = pos.y+1.0f/480.0f;
	ddim.x = dim.x-2.0f/640.0f;
	ddim.y = dim.y-2.0f/480.0f;

	if ( icon%10 == 0 )  // gris ?
	{
		uv1.x =  68.0f/256.0f;
		uv1.y =   4.0f/256.0f;
		uv2.x =  92.0f/256.0f;
		uv2.y =  28.0f/256.0f;
	}
	if ( icon%10 == 1 )  // vert ?
	{
		uv1.x =   0.0f/256.0f;
		uv1.y = 128.0f/256.0f;
		uv2.x =  64.0f/256.0f;
		uv2.y = 144.0f/256.0f;
	}
	if ( icon%10 == 2 )  // bleu ?
	{
		uv1.x =  64.0f/256.0f;
		uv1.y = 160.0f/256.0f;
		uv2.x = 128.0f/256.0f;
		uv2.y = 176.0f/256.0f;
	}
	if ( icon%10 == 3 )  // rouge ?
	{
		uv1.x =  32.0f/256.0f;
		uv1.y = 160.0f/256.0f;
		uv2.x =  64.0f/256.0f;
		uv2.y = 176.0f/256.0f;
	}
	uv1.x += dp;
	uv1.y += dp;
	uv2.x -= dp;
	uv2.y -= dp;

	DrawIcon(ppos, ddim, uv1, uv2);

	if ( text[0] == 0 )  return;

	if ( justif > 0 )
	{
		ppos.x = pos.x;
		ppos.x += 4.0f/640.0f;
	}
	if ( justif == 0 )
	{
		ppos.x = pos.x+dim.x/2.0f;
	}
	if ( justif < 0 )
	{
		ppos.x = pos.x+dim.x;
		ppos.x -= 4.0f/640.0f;
	}
	ppos.y += 2.0f/480.0f;
	m_engine->RetText()->DrawText(text, ppos, dim.x, justif, m_fontSize, m_fontStretch, m_fontType, 0);
}

// Dessine une ligne horizontale.

void CPesetas::DrawHLine(FPOINT pos, FPOINT dim, float x1, float y1, float x2)
{
	FPOINT		uv1, uv2;
	float		dp;

	dp = 0.5f/256.0f;

	if ( x1 > x2 )  Swap(x1, x2);

	pos.x += dim.x*x1;
	pos.y += dim.y*y1;
	dim.x *= x2-x1;
	dim.y = 1.0f/480.0f;

	uv1.x = 128.0f/256.0f;
	uv1.y =  16.0f/256.0f;
	uv2.x = 144.0f/256.0f;
	uv2.y =  32.0f/256.0f;
	uv1.x += dp;
	uv1.y += dp;
	uv2.x -= dp;
	uv2.y -= dp;

	DrawIcon(pos, dim, uv1, uv2);
}

// Dessine une ligne verticale.

void CPesetas::DrawVLine(FPOINT pos, FPOINT dim, float x1, float y1, float y2)
{
	FPOINT		uv1, uv2;
	float		dp;

	dp = 0.5f/256.0f;

	if ( y1 > y2 )  Swap(y1, y2);

	pos.x += dim.x*x1;
	pos.y += dim.y*y1;
	dim.x = 1.0f/640.0f;
	dim.y *= y2-y1;

	uv1.x = 128.0f/256.0f;
	uv1.y =  16.0f/256.0f;
	uv2.x = 144.0f/256.0f;
	uv2.y =  32.0f/256.0f;
	uv1.x += dp;
	uv1.y += dp;
	uv2.x -= dp;
	uv2.y -= dp;

	DrawIcon(pos, dim, uv1, uv2);
}

// Dessine un gros point.

void CPesetas::DrawDot(FPOINT pos, FPOINT dim, float x, float y)
{
	FPOINT		uv1, uv2;
	float		dp;

	dp = 0.5f/256.0f;

	pos.x += dim.x*x-3.0f/640.0f;
	pos.y += dim.y*y-3.0f/480.0f;
	dim.x = 7.0f/640.0f;
	dim.y = 7.0f/480.0f;

	uv1.x =   8.0f/256.0f;
	uv1.y =  32.0f/256.0f;
	uv2.x =  16.0f/256.0f;
	uv2.y =  40.0f/256.0f;
	uv1.x += dp;
	uv1.y += dp;
	uv2.x -= dp;
	uv2.y -= dp;

	DrawIcon(pos, dim, uv1, uv2);
}



// Gestion des niveaux.

void CPesetas::SetLevelCredit(float level)
{
	m_levelCredit = level;
}

float CPesetas::RetLevelCredit()
{
	return m_levelCredit;
}

void CPesetas::SetLevelMission(float level)
{
	m_levelMission = level;
}

float CPesetas::RetLevelMission()
{
	return m_levelMission;
}

void CPesetas::SetLevelNext(float level)
{
	m_levelNext = level;
}

float CPesetas::RetLevelNext()
{
	return m_levelNext;
}


