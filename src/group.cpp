// group.cpp

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
#include "group.h"




// Constructeur de l'objet.

CGroup::CGroup(CInstanceManager* iMan) : CControl(iMan)
{
	CControl::CControl(iMan);
}

// Destructeur de l'objet.

CGroup::~CGroup()
{
	CControl::~CControl();
}


// Crée un nouveau bouton.

BOOL CGroup::Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
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

BOOL CGroup::EventProcess(const Event &event)
{
	return TRUE;
}


// Dessine le bouton.

void CGroup::Draw()
{
	FPOINT		uv1,uv2, corner, pos, dim;
	float		dp;

	if ( (m_state & STATE_VISIBLE) == 0 )  return;

	if ( m_state & STATE_SHADOW )
	{
		DrawShadow(m_pos, m_dim);
	}

	dp = 0.5f/256.0f;

	if ( m_icon == 0 )  // gris uni ?
	{
		m_engine->SetTexture("button1.tga");
		m_engine->SetState(D3DSTATENORMAL);
		uv1.x = 144.0f/256.0f;
		uv1.y =  16.0f/256.0f;
		uv2.x = 160.0f/256.0f;
		uv2.y =  32.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		DrawIcon(m_pos, m_dim, uv1, uv2);
	}
	if ( m_icon == 1 )  // orange uni opaque ?
	{
		m_engine->SetTexture("button1.tga");
		m_engine->SetState(D3DSTATENORMAL);
		uv1.x =   8.0f/256.0f;
		uv1.y =  48.0f/256.0f;
		uv2.x =  16.0f/256.0f;
		uv2.y =  64.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		DrawIcon(m_pos, m_dim, uv1, uv2);
	}
	if ( m_icon == 2 )  // dégradé orange -> transparent ?
	{
		m_engine->SetTexture("button1.tga");
		m_engine->SetState(D3DSTATETTw);
		uv1.x =  16.0f/256.0f;
		uv1.y =  48.0f/256.0f;
		uv2.x =  24.0f/256.0f;
		uv2.y =  64.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		DrawIcon(m_pos, m_dim, uv1, uv2);
	}
	if ( m_icon == 3 )  // dégradé transparent -> gris ?
	{
		m_engine->SetTexture("button1.tga");
		m_engine->SetState(D3DSTATETTw);
		uv1.x =  24.0f/256.0f;
		uv1.y =  32.0f/256.0f;
		uv2.x =  32.0f/256.0f;
		uv2.y =  48.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		DrawIcon(m_pos, m_dim, uv1, uv2);
	}
	if ( m_icon == 4 )  // coin bleu dégradé ?
	{
		m_engine->SetTexture("button1.tga");
		m_engine->SetState(D3DSTATETTw);
		uv1.x = 192.0f/256.0f;
		uv1.y =  64.0f/256.0f;
		uv2.x = 224.0f/256.0f;
		uv2.y =  96.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		DrawIcon(m_pos, m_dim, uv1, uv2);
	}
	if ( m_icon == 5 )  // coin orange dégradé ?
	{
		m_engine->SetTexture("button1.tga");
		m_engine->SetState(D3DSTATETTw);
		uv1.x = 224.0f/256.0f;
		uv1.y =  64.0f/256.0f;
		uv2.x = 256.0f/256.0f;
		uv2.y =  96.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		DrawIcon(m_pos, m_dim, uv1, uv2);
	}
	if ( m_icon == 6 )
	{
		m_engine->SetTexture("button1.tga");
		m_engine->SetState(D3DSTATENORMAL);
		uv1.x =   4.0f/256.0f;
		uv1.y =   4.0f/256.0f;
		uv2.x =  28.0f/256.0f;
		uv2.y =  28.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		DrawIcon(m_pos, m_dim, uv1, uv2);
	}
	if ( m_icon == 7 )
	{
		m_engine->SetTexture("button1.tga");
		m_engine->SetState(D3DSTATENORMAL);
		uv1.x =  64.0f/256.0f;
		uv1.y =   0.0f/256.0f;
		uv2.x =  96.0f/256.0f;
		uv2.y =  32.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		corner.x = 6.0f/640.0f;
		corner.y = 6.0f/480.0f;
		DrawIcon(m_pos, m_dim, uv1, uv2, corner, 8.0f/256.0f);
	}

	if ( m_icon >= 8 && m_icon <= 11 )  // cadre CDisplayText ?
	{
		m_engine->SetTexture("button1.tga");
		m_engine->SetState(D3DSTATENORMAL);
		pos.x = m_pos.x-4.0f/640.0f;
		pos.y = m_pos.y-4.0f/480.0f;
		dim.x = m_dim.x+8.0f/640.0f;
		dim.y = m_dim.y+8.0f/480.0f;
		uv1.x =  64.0f/256.0f;
		uv1.y =   0.0f/256.0f;
		uv2.x =  96.0f/256.0f;
		uv2.y =  32.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		corner.x = 4.0f/640.0f;
		corner.y = 4.0f/480.0f;
		DrawIcon(pos, dim, uv1, uv2, 4.0f/256.0f);
	}
	if ( m_icon == 8 )
	{
		m_engine->SetTexture("button1.tga");
		m_engine->SetState(D3DSTATETTb);
		uv1.x = 160.0f/256.0f;  // vert
		uv1.y =  16.0f/256.0f;
		uv2.x = 176.0f/256.0f;
		uv2.y =  32.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		DrawIcon(m_pos, m_dim, uv1, uv2, 8.0f/256.0f);
	}
	if ( m_icon == 9 )
	{
		m_engine->SetTexture("button1.tga");
		m_engine->SetState(D3DSTATETTb);
		uv1.x = 176.0f/256.0f;  // rouge
		uv1.y =  16.0f/256.0f;
		uv2.x = 192.0f/256.0f;
		uv2.y =  32.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		DrawIcon(m_pos, m_dim, uv1, uv2, 8.0f/256.0f);
	}
	if ( m_icon == 10 )
	{
		m_engine->SetTexture("button1.tga");
		m_engine->SetState(D3DSTATETTb);
		uv1.x = 192.0f/256.0f;  // bleu
		uv1.y =  16.0f/256.0f;
		uv2.x = 208.0f/256.0f;
		uv2.y =  32.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		DrawIcon(m_pos, m_dim, uv1, uv2, 8.0f/256.0f);
	}
	if ( m_icon == 11 )
	{
		m_engine->SetTexture("button1.tga");
		m_engine->SetState(D3DSTATETTb);
		uv1.x = 208.0f/256.0f;  // orange
		uv1.y =  16.0f/256.0f;
		uv2.x = 224.0f/256.0f;
		uv2.y =  32.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		DrawIcon(m_pos, m_dim, uv1, uv2, 8.0f/256.0f);
	}

	if ( m_icon == 17 )
	{
	}
	if ( m_icon == 18 )  // flèche > pour SatCom ?
	{
		m_engine->SetTexture("button1.tga");
		m_engine->SetState(D3DSTATETTw);
		uv1.x =   0.0f/256.0f;  // >
		uv1.y = 192.0f/256.0f;
		uv2.x =  32.0f/256.0f;
		uv2.y = 224.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		DrawIcon(m_pos, m_dim, uv1, uv2);
	}
	if ( m_icon == 19 )  // sigle SatCom ?
	{
		m_engine->SetTexture("button1.tga");
		m_engine->SetState(D3DSTATETTw);
		uv1.x = 224.0f/256.0f;  // sigle SatCom
		uv1.y = 224.0f/256.0f;
		uv2.x = 256.0f/256.0f;
		uv2.y = 256.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		DrawIcon(m_pos, m_dim, uv1, uv2);
	}
	if ( m_icon == 20 )  // fond fleu uni ?
	{
		m_engine->SetTexture("button1.tga");
		m_engine->SetState(D3DSTATETTw);
		uv1.x = 224.0f/256.0f;
		uv1.y =  32.0f/256.0f;
		uv2.x = 256.0f/256.0f;
		uv2.y =  64.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		DrawIcon(m_pos, m_dim, uv1, uv2);
	}
	if ( m_icon == 21 )  // sigle stand-by ?
	{
	}
	if ( m_icon == 22 )  // indicateur de pause ?
	{
		m_engine->SetTexture("button1.tga");
		m_engine->SetState(D3DSTATENORMAL);
		pos.x = m_pos.x-10.0f/640.0f;
		pos.y = m_pos.y-10.0f/480.0f;
		dim.x = m_dim.x+20.0f/640.0f;
		dim.y = m_dim.y+20.0f/480.0f;
		uv1.x =  32.0f/256.0f;
		uv1.y =  64.0f/256.0f;
		uv2.x =  64.0f/256.0f;
		uv2.y =  96.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		corner.x = 4.0f/640.0f;
		corner.y = 4.0f/480.0f;
		DrawIcon(pos, dim, uv1, uv2, 4.0f/256.0f);

		m_engine->SetState(D3DSTATETTw);
		uv1.x = 160.0f/256.0f;
		uv1.y =  64.0f/256.0f;
		uv2.x = 192.0f/256.0f;
		uv2.y =  96.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		DrawIcon(m_pos, m_dim, uv1, uv2);
	}

	if ( m_icon == 23 )  // bandes pour écran win ?
	{
		m_engine->SetTexture("button1.tga");
		m_engine->SetState(D3DSTATETTb);
		uv1.x = 128.0f/256.0f;  // gris foncé
		uv1.y =  16.0f/256.0f;
		uv2.x = 144.0f/256.0f;
		uv2.y =  32.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		DrawIcon(m_pos, m_dim, uv1, uv2);
	}
	if ( m_icon >= 24 && m_icon <= 26 )
	{
		m_engine->SetTexture("button1.tga");
		m_engine->SetState(D3DSTATETTw);
		uv1.x = 144.0f/256.0f;  // gris clair
		uv1.y =  16.0f/256.0f;
		uv2.x = 160.0f/256.0f;
		uv2.y =  32.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		DrawIcon(m_pos, m_dim, uv1, uv2);

		pos.x = m_pos.x+m_dim.x*0.2f;
		pos.y = m_pos.y+m_dim.y*0.2f;
		dim.x = m_dim.x*0.6f;
		dim.y = m_dim.y*0.6f;

		if ( m_icon == 24 )  // chrono ?
		{
			uv1.x =  96.0f/256.0f;
			uv1.y = 224.0f/256.0f;
			uv2.x = 128.0f/256.0f;
			uv2.y = 256.0f/256.0f;
		}
		if ( m_icon == 25 )  // pesetas ?
		{
			uv1.x = 128.0f/256.0f;
			uv1.y = 224.0f/256.0f;
			uv2.x = 160.0f/256.0f;
			uv2.y = 256.0f/256.0f;
		}
		if ( m_icon == 26 )  // vainqueur ?
		{
			uv1.x = 160.0f/256.0f;
			uv1.y = 224.0f/256.0f;
			uv2.x = 192.0f/256.0f;
			uv2.y = 256.0f/256.0f;
		}
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		DrawIcon(pos, dim, uv1, uv2);
	}
	if ( m_icon == 27 )
	{
		m_engine->SetTexture("button1.tga");
		m_engine->SetState(D3DSTATENORMAL);
		uv1.x = 208.0f/256.0f;
		uv1.y =  16.0f/256.0f;
		uv2.x = 224.0f/256.0f;
		uv2.y =  32.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		corner.x = 10.0f/640.0f;
		corner.y = 10.0f/480.0f;
		DrawIcon(m_pos, m_dim, uv1, uv2, corner, 5.0f/256.0f);
	}
	if ( m_icon == 28 )
	{
		m_engine->SetTexture("button1.tga");
		m_engine->SetState(D3DSTATENORMAL);
		uv1.x =  64.0f/256.0f;
		uv1.y =   0.0f/256.0f;
		uv2.x =  96.0f/256.0f;
		uv2.y =  32.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		corner.x = 4.0f/640.0f;
		corner.y = 4.0f/480.0f;
		DrawIcon(m_pos, m_dim, uv1, uv2, corner, 4.0f/256.0f);

		m_engine->SetTexture("button1.tga");
		m_engine->SetState(D3DSTATETTb);
		pos.x = m_pos.x+4.0f/640.0f;
		pos.y = m_pos.y+4.0f/480.0f;
		dim.x = m_dim.x-8.0f/640.0f;
		dim.y = m_dim.y-8.0f/480.0f;
		uv1.x =   0.0f/256.0f;
		uv1.y =  32.0f/256.0f;
		uv2.x =   8.0f/256.0f;
		uv2.y =  64.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		DrawIcon(pos, dim, uv1, uv2);
	}
	if ( m_icon == 29 )
	{
		m_engine->SetTexture("button1.tga");
		m_engine->SetState(D3DSTATETTb);
		uv1.x = 176.0f/256.0f;  // rouge
		uv1.y =  16.0f/256.0f;
		uv2.x = 192.0f/256.0f;
		uv2.y =  32.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		DrawIcon(m_pos, m_dim, uv1, uv2);
	}
	if ( m_icon == 30 )
	{
		m_engine->SetTexture("button1.tga");
		m_engine->SetState(D3DSTATETTb);
		uv1.x = 192.0f/256.0f;  // bleu
		uv1.y =  16.0f/256.0f;
		uv2.x = 208.0f/256.0f;
		uv2.y =  32.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		DrawIcon(m_pos, m_dim, uv1, uv2);
	}
}


