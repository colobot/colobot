// control.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "D3DEngine.h"
#include "restext.h"
#include "math3d.h"
#include "event.h"
#include "misc.h"
#include "robotmain.h"
#include "particule.h"
#include "misc.h"
#include "iman.h"
#include "text.h"
#include "sound.h"
#include "control.h"




// Constructeur de l'objet.

CControl::CControl(CInstanceManager* iMan)
{
	m_iMan = iMan;

	m_engine      = (CD3DEngine*)m_iMan->SearchInstance(CLASS_ENGINE);
	m_event       = (CEvent*)m_iMan->SearchInstance(CLASS_EVENT);
	m_main        = (CRobotMain*)m_iMan->SearchInstance(CLASS_MAIN);
	m_particule   = (CParticule*)m_iMan->SearchInstance(CLASS_PARTICULE);
	m_sound       = (CSound*)m_iMan->SearchInstance(CLASS_SOUND);
	m_eventMsg    = EVENT_NULL;
	m_state       = STATE_ENABLE|STATE_VISIBLE|STATE_GLINT;
	m_fontSize    = SMALLFONT;
	m_fontStretch = NORMSTRETCH;
	m_fontType    = FONT_COLOBOT;
	m_justif      = 0;
	m_name[0]     = 0;
	m_tooltip[0]  = 0;
	m_texture[0]  = 0;
	m_bFocus      = FALSE;
	m_bCapture    = FALSE;
	m_tabOrder    = -1;
	m_time        = 0.0f;

	m_bGlint        = FALSE;
	m_glintCorner1  = FPOINT(0.0f, 0.0f);
	m_glintCorner2  = FPOINT(0.0f, 0.0f);
	m_glintProgress = 999.0f;
	m_glintMouse    = FPOINT(0.0f, 0.0f);
}

// Destructeur de l'objet.

CControl::~CControl()
{
}


// Crée un nouveau bouton.
//	pos: [0..1]

BOOL CControl::Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
	char	text[200];
	char*	p;

	if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

	m_pos = pos;
	m_dim = dim;
	m_icon = icon;
	m_eventMsg = eventMsg;

	pos.x = m_pos.x;
	pos.y = m_pos.y+m_dim.y;
	GlintCreate(pos);

	GetResource(RES_EVENT, m_eventMsg, text);
	p = strchr(text, '\\');
	if ( p == 0 )
	{
		if ( icon != -1 )
		{
			strcpy(m_tooltip, text);
		}
	}
	else
	{
		strcpy(m_tooltip, p+1);  // texte après "\\"
	}

	return TRUE;
}


void CControl::SetPos(FPOINT pos)
{
	m_pos = pos;

	pos.x = m_pos.x;
	pos.y = m_pos.y+m_dim.y;
	GlintCreate(pos);
}

FPOINT CControl::RetPos()
{
	return m_pos;
}

void CControl::SetDim(FPOINT dim)
{
	FPOINT	pos;

	m_dim = dim;

	pos.x = m_pos.x;
	pos.y = m_pos.y+m_dim.y;
	GlintCreate(pos);
}

FPOINT CControl::RetDim()
{
	return m_dim;
}


// Modifie un attribut d'état.

BOOL CControl::SetState(int state, BOOL bState)
{
	if ( bState )  m_state |= state;
	else           m_state &= ~state;
	return TRUE;
}

// Met un attribut d'état.

BOOL CControl::SetState(int state)
{
	m_state |= state;
	return TRUE;
}

// Enlève un attribut d'état.

BOOL CControl::ClearState(int state)
{
	m_state &= ~state;
	return TRUE;
}

// Teste un attribut d'état.

BOOL CControl::TestState(int state)
{
	return (m_state & state) ? TRUE:FALSE;
}

// Retourne tous les attributs d'état.

int CControl::RetState()
{
	return m_state;
}


// Gestion de l'icône.

void CControl::SetIcon(int icon)
{
	m_icon = icon;
}

int CControl::RetIcon()
{
	return m_icon;
}


// Spécifie une texture spéciale.

void CControl::SetTexture(char *filename, FPOINT uv1, FPOINT uv2)
{
	strcpy(m_texture, filename);
	m_uv1 = uv1;
	m_uv2 = uv2;
}


// Gestion du nom du bouton.

void CControl::SetName(char* name, BOOL bTooltip)
{
	char*	p;

	if ( bTooltip )
	{
		p = strchr(name, '\\');
		if ( p == 0 )
		{
			strncpy(m_name, name, 100);
			m_name[100-1] = 0;
		}
		else
		{
			char	buffer[100];

			strncpy(m_tooltip, p+1, 100);  // texte après "\\"
			m_tooltip[100-1] = 0;

			strncpy(buffer, name, 100);
			buffer[100-1] = 0;
			p = strchr(buffer, '\\');
			if ( p != 0 )  *p = 0;
			strncpy(m_name, buffer, 100);
			m_name[100-1] = 0;
		}
	}
	else
	{
		strncpy(m_name, name, 100);
		m_name[100-1] = 0;
	}
}

char* CControl::RetName()
{
	return m_name;
}


// Gestion du mode de justification (-1,0,1).

void CControl::SetJustif(int mode)
{
	m_justif = mode;
}

int CControl::RetJustif()
{
	return m_justif;
}


// Gestion de la taille de la fonte.

void CControl::SetFontSize(float size)
{
	m_fontSize = size;
}

float CControl::RetFontSize()
{
	return m_fontSize;
}


// Gestion du stretch de la fonte.

void CControl::SetFontStretch(float stretch)
{
	m_fontStretch = stretch;
}

float CControl::RetFontStretch()
{
	return m_fontStretch;
}


// Choix de la fonte.

void CControl::SetFontType(FontType font)
{
	m_fontType = font;
}

FontType CControl::RetFontType()
{
	return m_fontType;
}


// Spécifie le tooltip.

BOOL CControl::SetTooltip(char* name)
{
	strcpy(m_tooltip, name);
	return TRUE;
}

BOOL CControl::GetTooltip(FPOINT pos, char* name)
{
	if ( m_state & STATE_DEMO )
	{
		if ( !Detect(pos) )  return FALSE;
		GetResource(RES_TEXT, RT_DEMO, name);
		return TRUE;
	}

	if ( m_tooltip[0] == 0 )  return FALSE;
	if ( (m_state & STATE_VISIBLE) == 0 )  return FALSE;
	if ( (m_state & STATE_ENABLE) == 0 )  return FALSE;
	if ( m_state & STATE_DEAD )  return FALSE;
	if ( !Detect(pos) )  return FALSE;

	strcpy(name, m_tooltip);
	return TRUE;
}


// Gestion de qui a la focus.

void CControl::SetFocus(BOOL bFocus)
{
	m_bFocus = bFocus;
}

BOOL CControl::RetFocus()
{
	return m_bFocus;
}


// Gestion de l'ordre du bouton.

void CControl::SetTabOrder(int rank)
{
	m_tabOrder = rank;
}

int CControl::RetTabOrder()
{
	return m_tabOrder;
}


// Retourne l'événement associé au contrôle.

EventMsg CControl::RetEventMsg()
{
	return m_eventMsg;
}


// Gestion d'un événement.

BOOL CControl::EventProcess(const Event &event)
{
	if ( m_state & STATE_DEAD )  return TRUE;

	if ( event.event == EVENT_FRAME )
	{
		m_time += event.rTime;
		if ( m_bGlint )  GlintFrame(event);
	}

	if ( event.event == EVENT_MOUSEMOVE )
	{
		m_glintMouse = event.pos;

		if ( Detect(event.pos) )
		{
			if ( (m_state & STATE_VISIBLE) &&
				 (m_state & STATE_ENABLE ) )
			{
				m_engine->SetMouseType(D3DMOUSEHAND);
			}
			SetState(STATE_HILIGHT);
		}
		else
		{
			ClearState(STATE_HILIGHT);
		}
	}

	if ( event.event == EVENT_LBUTTONDOWN )
	{
		if ( Detect(event.pos) )
		{
			m_bCapture = TRUE;
			SetState(STATE_PRESS);
		}
	}

	if ( event.event == EVENT_MOUSEMOVE && m_bCapture )
	{
		if ( Detect(event.pos) )
		{
			SetState(STATE_PRESS);
		}
		else
		{
			ClearState(STATE_PRESS);
		}
	}

	if ( event.event == EVENT_LBUTTONUP && m_bCapture )
	{
		m_bCapture = FALSE;
		ClearState(STATE_PRESS);
	}

	return TRUE;
}


// Supprime le reflet.

void CControl::GlintDelete()
{
	m_bGlint = FALSE;
}

// Crée un reflet pour ce bouton.

void CControl::GlintCreate(FPOINT ref, BOOL bLeft, BOOL bUp)
{
	float	offset;

	offset = 8.0f/640.0f;
	if ( offset > m_dim.x/4.0f)  offset = m_dim.x/4.0f;

	if ( bLeft )
	{
		m_glintCorner1.x = ref.x;
		m_glintCorner2.x = ref.x+offset;
	}
	else
	{
		m_glintCorner1.x = ref.x-offset;
		m_glintCorner2.x = ref.x;
	}

	offset = 8.0f/480.0f;
	if ( offset > m_dim.y/4.0f)  offset = m_dim.y/4.0f;

	if ( bUp )
	{
		m_glintCorner1.y = ref.y-offset;
		m_glintCorner2.y = ref.y;
	}
	else
	{
		m_glintCorner1.y = ref.y;
		m_glintCorner2.y = ref.y+offset;
	}

	m_bGlint = TRUE;
}

// Gestion du reflet.

void CControl::GlintFrame(const Event &event)
{
	D3DVECTOR	pos, speed;
	FPOINT		dim;

	if ( (m_state & STATE_GLINT  ) == 0 ||
		 (m_state & STATE_ENABLE ) == 0 ||
		 (m_state & STATE_VISIBLE) == 0 )  return;

	m_glintProgress += event.rTime;

	if ( m_glintProgress >= 2.0f && Detect(m_glintMouse) )
	{
		pos.x = m_glintCorner1.x + (m_glintCorner2.x-m_glintCorner1.x)*Rand();
		pos.y = m_glintCorner1.y + (m_glintCorner2.y-m_glintCorner1.y)*Rand();
		pos.z = 0.0f;
		speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
		dim.x = ((15.0f+Rand()*15.0f)/640.0f);
		dim.y = dim.x/0.75f;
		m_particule->CreateParticule(pos, speed, dim, PARTICONTROL,
									 1.0f, 0.0f, SH_INTERFACE);

		m_glintProgress = 0.0f;
	}
}


// Dessine le bouton.

void CControl::Draw()
{
	FPOINT		pos, dim;
	float		zoomExt, zoomInt;
	int			icon, i, state;

	if ( (m_state & STATE_VISIBLE) == 0 )  return;

	m_engine->SetTexture("button1.tga");
//?	m_engine->SetState(D3DSTATENORMAL);

	state = D3DSTATETTb;
	zoomExt = 1.00f;
	zoomInt = 0.95f;

	if ( m_icon >= 128 )
	{
		zoomInt = 0.80f;
	}

	icon = 2;
	if ( m_state & STATE_SIMPLY )
	{
		icon = 15;
	}
	if ( m_state & STATE_CARD )
	{
		icon = 26;
	}
	if ( m_state & STATE_HILIGHT )
	{
		icon = 1;
	}
	if ( m_state & STATE_CHECK )
	{
		if ( m_state & STATE_CARD )
		{
			icon = 27;
		}
		else
		{
			icon = 0;
		}
		state = D3DSTATENORMAL;
	}
	if ( m_state & STATE_TODO )
	{
		icon = 17;
		state = D3DSTATENORMAL;
	}
	if ( m_state & STATE_PASSED )
	{
		icon = 18;
		state = D3DSTATENORMAL;
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
		icon = 7;
	}
	if ( (m_state & STATE_FLASH) && Mod(m_time, 0.4f) < 0.2f )
	{
		icon = 0;
	}
	if ( m_state & STATE_WARNING )  // hachures jaunes-noires ?
	{
		if ( icon == 2 )  icon = 1;
	}
	m_engine->SetState(state);

	if ( m_name[0] == 0 )  // bouton sans nom ?
	{
		DrawPart(icon, zoomExt, 8.0f/256.0f);

		if ( m_state & STATE_DEAD )  return;

		if ( m_texture[0] == 0 )
		{
			i = m_icon;
			if ( i >= 192 )
			{
				i -= 192;
				m_engine->SetTexture("text.tga");
				m_engine->SetState(D3DSTATETTw);
			}
			else if ( i >= 128 )
			{
				i -= 128;
				m_engine->SetTexture("button3.tga");
				m_engine->SetState(D3DSTATETTw);
			}
			else if ( i >= 64 )
			{
				i -= 64;
				m_engine->SetTexture("button2.tga");
				m_engine->SetState(D3DSTATETTw);
			}
			else
			{
				m_engine->SetState(D3DSTATETTw);
			}

			pos = m_pos;
			dim = m_dim;
			pos.x += dim.x/2.0f;
			pos.y += dim.y/2.0f;
			if ( dim.x > dim.y*0.75f )  dim.x = dim.y*0.75f;
			else                        dim.y = dim.x/0.75f;
			pos.x -= dim.x/2.0f;
			pos.y -= dim.y/2.0f;
			DrawPart(pos, dim, i, zoomInt, 0.0f);
		}
		else
		{
			m_engine->SetTexture(m_texture);
			m_engine->SetState(D3DSTATENORMAL);
			pos = m_pos;
			dim = m_dim;
			pos.x +=  6.0f/640.0f;
			pos.y +=  6.0f/480.0f;
			dim.x -= 12.0f/640.0f;
			dim.y -= 12.0f/480.0f;
			DrawIcon(pos, dim, m_uv1, m_uv2);
		}

		if ( (m_state & STATE_ENABLE) == 0 )
		{
			m_engine->SetTexture("button1.tga");
			m_engine->SetState(D3DSTATETTb);
			DrawPart(icon, zoomExt, 8.0f/256.0f);  // blanc par-dessus
		}
	}
	else	// bouton avec nom ?
	{
		DrawPart(icon, 1.0f, 8.0f/256.0f);

		if ( m_state & STATE_DEAD )  return;

		if ( m_justif < 0 )
		{
			pos.x = m_pos.x+m_dim.x-m_dim.y*0.5f;
			pos.y = m_pos.y+m_dim.y*0.5f;
			pos.y -= m_engine->RetText()->RetHeight(m_fontSize, m_fontType)/2;
			m_engine->RetText()->DrawText(m_name, pos, m_dim.x, m_justif, m_fontSize, m_fontStretch, m_fontType, 0);
		}
		else if ( m_justif > 0 )
		{
			pos.x = m_pos.x+m_dim.y*0.5f;
			pos.y = m_pos.y+m_dim.y*0.5f;
			pos.y -= m_engine->RetText()->RetHeight(m_fontSize, m_fontType)/2.0f;
			m_engine->RetText()->DrawText(m_name, pos, m_dim.x, m_justif, m_fontSize, m_fontStretch, m_fontType, 0);
		}
		else
		{
			pos.x = m_pos.x+m_dim.x*0.5f;
			pos.y = m_pos.y+m_dim.y*0.5f;
			pos.y -= m_engine->RetText()->RetHeight(m_fontSize, m_fontType)/2.0f;
			m_engine->RetText()->DrawText(m_name, pos, m_dim.x, m_justif, m_fontSize, m_fontStretch, m_fontType, 0);
		}

		if ( (m_state & STATE_ENABLE) == 0 )
		{
			m_engine->SetTexture("button1.tga");
			m_engine->SetState(D3DSTATETTb);
			DrawPart(icon, 1.0f, 8.0f/256.0f);  // blanc par-dessus
		}
	}
}

// Dessine le tableau des vertex.

void CControl::DrawPart(int icon, float zoom, float ex)
{
	DrawPart(m_pos, m_dim, icon, zoom, ex);
}

// Dessine le tableau des vertex.

void CControl::DrawPart(FPOINT pos, FPOINT dim, int icon, float zoom, float ex)
{
	FPOINT		p1, p2, c, uv1, uv2;
	float		dp;

	p1.x = pos.x;
	p1.y = pos.y;
	p2.x = pos.x + dim.x;
	p2.y = pos.y + dim.y;

	if ( (m_state & STATE_CARD ) &&
		 (m_state & STATE_CHECK) )
	{
		p2.y += (2.0f/480.0f);  // un poil plus haut
	}

	c.x = (p1.x+p2.x)/2.0f;
	c.y = (p1.y+p2.y)/2.0f;  // centre

	p1.x = (p1.x-c.x)*zoom + c.x;
	p1.y = (p1.y-c.y)*zoom + c.y;
	p2.x = (p2.x-c.x)*zoom + c.x;
	p2.y = (p2.y-c.y)*zoom + c.y;

	p2.x -= p1.x;
	p2.y -= p1.y;

	uv1.x = (32.0f/256.0f)*(icon%8);
	uv1.y = (32.0f/256.0f)*(icon/8);  // uv texture
	uv2.x = (32.0f/256.0f)+uv1.x;
	uv2.y = (32.0f/256.0f)+uv1.y;

	dp = 0.5f/256.0f;
	uv1.x += dp;
	uv1.y += dp;
	uv2.x -= dp;
	uv2.y -= dp;

	if ( dim.x > 50.0f/640.0f &&
		 dim.y > 30.0f/480.0f )
	{
		DrawIcon(p1, p2, uv1, uv2, FPOINT(8.0f/640.0f, 8.0f/480.0f), ex);
	}
	else
	{
		DrawIcon(p1, p2, uv1, uv2, ex);
	}
}

// Dessine une icône rectangulaire composée de 1 (si ex=0)
// ou 3 morceaux.

void CControl::DrawIcon(FPOINT pos, FPOINT dim, FPOINT uv1, FPOINT uv2,
						float ex)
{
	LPDIRECT3DDEVICE7 device;
	D3DVERTEX2	vertex[8];	// 6 triangles
	FPOINT		p1, p2, p3, p4;
	D3DVECTOR	n;

	device = m_engine->RetD3DDevice();

	p1.x = pos.x;
	p1.y = pos.y;
	p2.x = pos.x + dim.x;
	p2.y = pos.y + dim.y;

	n = D3DVECTOR(0.0f, 0.0f, -1.0f);  // normale

	if ( ex == 0.0f )  // un seul morceau ?
	{
		vertex[0] = D3DVERTEX2(D3DVECTOR(p1.x, p1.y, 0.0f), n, uv1.x,uv2.y);
		vertex[1] = D3DVERTEX2(D3DVECTOR(p1.x, p2.y, 0.0f), n, uv1.x,uv1.y);
		vertex[2] = D3DVERTEX2(D3DVECTOR(p2.x, p1.y, 0.0f), n, uv2.x,uv2.y);
		vertex[3] = D3DVERTEX2(D3DVECTOR(p2.x, p2.y, 0.0f), n, uv2.x,uv1.y);

		device->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 4, NULL);
		m_engine->AddStatisticTriangle(2);
	}
	else	// 3 morceaux ?
	{
		if ( dim.x >= dim.y )
		{
			p3.x = p1.x + ex*dim.y/(uv2.y-uv1.y);
			p4.x = p2.x - ex*dim.y/(uv2.y-uv1.y);

			vertex[0] = D3DVERTEX2(D3DVECTOR(p1.x, p1.y, 0.0f), n, uv1.x,   uv2.y);
			vertex[1] = D3DVERTEX2(D3DVECTOR(p1.x, p2.y, 0.0f), n, uv1.x,   uv1.y);
			vertex[2] = D3DVERTEX2(D3DVECTOR(p3.x, p1.y, 0.0f), n, uv1.x+ex,uv2.y);
			vertex[3] = D3DVERTEX2(D3DVECTOR(p3.x, p2.y, 0.0f), n, uv1.x+ex,uv1.y);
			vertex[4] = D3DVERTEX2(D3DVECTOR(p4.x, p1.y, 0.0f), n, uv2.x-ex,uv2.y);
			vertex[5] = D3DVERTEX2(D3DVECTOR(p4.x, p2.y, 0.0f), n, uv2.x-ex,uv1.y);
			vertex[6] = D3DVERTEX2(D3DVECTOR(p2.x, p1.y, 0.0f), n, uv2.x,   uv2.y);
			vertex[7] = D3DVERTEX2(D3DVECTOR(p2.x, p2.y, 0.0f), n, uv2.x,   uv1.y);

			device->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 8, NULL);
			m_engine->AddStatisticTriangle(6);
		}
		else
		{
			p3.y = p1.y + ex*dim.x/(uv2.x-uv1.x);
			p4.y = p2.y - ex*dim.x/(uv2.x-uv1.x);

			vertex[0] = D3DVERTEX2(D3DVECTOR(p2.x, p1.y, 0.0f), n, uv2.x,uv2.y   );
			vertex[1] = D3DVERTEX2(D3DVECTOR(p1.x, p1.y, 0.0f), n, uv1.x,uv2.y   );
			vertex[2] = D3DVERTEX2(D3DVECTOR(p2.x, p3.y, 0.0f), n, uv2.x,uv2.y-ex);
			vertex[3] = D3DVERTEX2(D3DVECTOR(p1.x, p3.y, 0.0f), n, uv1.x,uv2.y-ex);
			vertex[4] = D3DVERTEX2(D3DVECTOR(p2.x, p4.y, 0.0f), n, uv2.x,uv1.y+ex);
			vertex[5] = D3DVERTEX2(D3DVECTOR(p1.x, p4.y, 0.0f), n, uv1.x,uv1.y+ex);
			vertex[6] = D3DVERTEX2(D3DVECTOR(p2.x, p2.y, 0.0f), n, uv2.x,uv1.y   );
			vertex[7] = D3DVERTEX2(D3DVECTOR(p1.x, p2.y, 0.0f), n, uv1.x,uv1.y   );

			device->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 8, NULL);
			m_engine->AddStatisticTriangle(6);
		}
	}
}

// Dessine une icône rectangulaire composée de 9 morceaux.

void CControl::DrawIcon(FPOINT pos, FPOINT dim, FPOINT uv1, FPOINT uv2,
						FPOINT corner, float ex)
{
	LPDIRECT3DDEVICE7 device;
	D3DVERTEX2	vertex[8];	// 6 triangles
	FPOINT		p1, p2, p3, p4;
	D3DVECTOR	n;

	device = m_engine->RetD3DDevice();

	p1.x = pos.x;
	p1.y = pos.y;
	p2.x = pos.x + dim.x;
	p2.y = pos.y + dim.y;

	n = D3DVECTOR(0.0f, 0.0f, -1.0f);  // normale

	if ( corner.x > dim.x/2.0f )  corner.x = dim.x/2.0f;
	if ( corner.y > dim.y/2.0f )  corner.y = dim.y/2.0f;

	p1.x = pos.x;
	p1.y = pos.y;
	p2.x = pos.x + dim.x;
	p2.y = pos.y + dim.y;
	p3.x = p1.x + corner.x;
	p3.y = p1.y + corner.y;
	p4.x = p2.x - corner.x;
	p4.y = p2.y - corner.y;

	// Bande horizontale inférieure.
	vertex[0] = D3DVERTEX2(D3DVECTOR(p1.x, p1.y, 0.0f), n, uv1.x,   uv2.y   );
	vertex[1] = D3DVERTEX2(D3DVECTOR(p1.x, p3.y, 0.0f), n, uv1.x,   uv2.y-ex);
	vertex[2] = D3DVERTEX2(D3DVECTOR(p3.x, p1.y, 0.0f), n, uv1.x+ex,uv2.y   );
	vertex[3] = D3DVERTEX2(D3DVECTOR(p3.x, p3.y, 0.0f), n, uv1.x+ex,uv2.y-ex);
	vertex[4] = D3DVERTEX2(D3DVECTOR(p4.x, p1.y, 0.0f), n, uv2.x-ex,uv2.y   );
	vertex[5] = D3DVERTEX2(D3DVECTOR(p4.x, p3.y, 0.0f), n, uv2.x-ex,uv2.y-ex);
	vertex[6] = D3DVERTEX2(D3DVECTOR(p2.x, p1.y, 0.0f), n, uv2.x,   uv2.y   );
	vertex[7] = D3DVERTEX2(D3DVECTOR(p2.x, p3.y, 0.0f), n, uv2.x,   uv2.y-ex);
	device->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 8, NULL);
	m_engine->AddStatisticTriangle(6);

	// Bande horizontale médiane.
	vertex[0] = D3DVERTEX2(D3DVECTOR(p1.x, p3.y, 0.0f), n, uv1.x,   uv2.y-ex);
	vertex[1] = D3DVERTEX2(D3DVECTOR(p1.x, p4.y, 0.0f), n, uv1.x,   uv1.y+ex);
	vertex[2] = D3DVERTEX2(D3DVECTOR(p3.x, p3.y, 0.0f), n, uv1.x+ex,uv2.y-ex);
	vertex[3] = D3DVERTEX2(D3DVECTOR(p3.x, p4.y, 0.0f), n, uv1.x+ex,uv1.y+ex);
	vertex[4] = D3DVERTEX2(D3DVECTOR(p4.x, p3.y, 0.0f), n, uv2.x-ex,uv2.y-ex);
	vertex[5] = D3DVERTEX2(D3DVECTOR(p4.x, p4.y, 0.0f), n, uv2.x-ex,uv1.y+ex);
	vertex[6] = D3DVERTEX2(D3DVECTOR(p2.x, p3.y, 0.0f), n, uv2.x,   uv2.y-ex);
	vertex[7] = D3DVERTEX2(D3DVECTOR(p2.x, p4.y, 0.0f), n, uv2.x,   uv1.y+ex);
	device->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 8, NULL);
	m_engine->AddStatisticTriangle(6);

	// Bande horizontale supérieure.
	vertex[0] = D3DVERTEX2(D3DVECTOR(p1.x, p4.y, 0.0f), n, uv1.x,   uv1.y+ex);
	vertex[1] = D3DVERTEX2(D3DVECTOR(p1.x, p2.y, 0.0f), n, uv1.x,   uv1.y   );
	vertex[2] = D3DVERTEX2(D3DVECTOR(p3.x, p4.y, 0.0f), n, uv1.x+ex,uv1.y+ex);
	vertex[3] = D3DVERTEX2(D3DVECTOR(p3.x, p2.y, 0.0f), n, uv1.x+ex,uv1.y   );
	vertex[4] = D3DVERTEX2(D3DVECTOR(p4.x, p4.y, 0.0f), n, uv2.x-ex,uv1.y+ex);
	vertex[5] = D3DVERTEX2(D3DVECTOR(p4.x, p2.y, 0.0f), n, uv2.x-ex,uv1.y   );
	vertex[6] = D3DVERTEX2(D3DVECTOR(p2.x, p4.y, 0.0f), n, uv2.x,   uv1.y+ex);
	vertex[7] = D3DVERTEX2(D3DVECTOR(p2.x, p2.y, 0.0f), n, uv2.x,   uv1.y   );
	device->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 8, NULL);
	m_engine->AddStatisticTriangle(6);
}

// Dessine les hachures autour d'un bouton.

void CControl::DrawWarning(FPOINT pos, FPOINT dim)
{
	FPOINT		ppos, ddim, uv1, uv2;
	float		dp;

	dp = 0.5f/256.0f;

	m_engine->SetTexture("button1.tga");
//?	m_engine->SetState(D3DSTATENORMAL);
	m_engine->SetState(D3DSTATETTb);

	ppos.x = pos.x-1.0f/640.0f;
	ppos.y = pos.y-1.0f/480.0f;
	ddim.x = dim.x+2.0f/640.0f;
	ddim.y = dim.y+2.0f/480.0f;

	uv1.x =   0.0f/256.0f;
	uv1.y = 160.0f/256.0f;
	uv2.x =  96.0f/256.0f;
	uv2.y = 192.0f/256.0f;
	uv1.x += dp;
	uv1.y += dp;
	uv2.x -= dp;
	uv2.y -= dp;

	DrawIcon(pos, dim, uv1, uv2);
}

// Dessine l'ombre sous un bouton.

void CControl::DrawShadow(FPOINT pos, FPOINT dim, float deep)
{
	FPOINT		uv1, uv2, corner;
	float		dp;

	dp = 0.5f/256.0f;

	m_engine->SetTexture("button1.tga");
	m_engine->SetState(D3DSTATETTw);

#if 0
	pos.x += deep*0.010f*0.75f;
	pos.y -= deep*0.015f;
	dim.x += deep*0.005f*0.75f;
	dim.y += deep*0.005f;
#else
	pos.y -= deep*0.015f;
	dim.x += deep*0.015f*0.75f;
	dim.y += deep*0.015f;
#endif

	uv1.x =  32.0f/256.0f;
	uv1.y =  32.0f/256.0f;
	uv2.x =  64.0f/256.0f;
	uv2.y =  64.0f/256.0f;

	uv1.x += dp;
	uv1.y += dp;
	uv2.x -= dp;
	uv2.y -= dp;

	corner.x = 10.0f/640.0f;
	corner.y = 10.0f/480.0f;

	DrawIcon(pos, dim, uv1, uv2, corner, 6.0f/256.0f);
}

// Dessine le focus du bouton.

void CControl::DrawFocus(FPOINT pos, FPOINT dim)
{
	FPOINT		uv1, uv2;
	float		dp;

	dp = 0.5f/256.0f;

	m_engine->SetTexture("button1.tga");
//?	m_engine->SetState(D3DSTATENORMAL);
	m_engine->SetState(D3DSTATETTb);

	pos.x -= 4.0f/640.0f;
	pos.y -= 4.0f/480.0f;
	dim.x += 8.0f/640.0f;
	dim.y += 8.0f/480.0f;

	uv1.x = 192.0f/256.0f;
	uv1.y =  16.0f/256.0f;
	uv2.x = 192.0f/256.0f;
	uv2.y =  32.0f/256.0f;

	uv1.x += dp;
	uv1.y += dp;
	uv2.x -= dp;
	uv2.y -= dp;

	DrawIcon(pos, dim, uv1, uv2);
}


// Détecte si une position est dans le bouton.

BOOL CControl::Detect(FPOINT pos)
{
	return ( pos.x >= m_pos.x         &&
			 pos.x <= m_pos.x+m_dim.x &&
			 pos.y >= m_pos.y         &&
			 pos.y <= m_pos.y+m_dim.y );
}


