// map.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "D3DEngine.h"
#include "event.h"
#include "math3d.h"
#include "terrain.h"
#include "water.h"
#include "object.h"
#include "event.h"
#include "misc.h"
#include "robotmain.h"
#include "iman.h"
#include "map.h"




// Constructeur de l'objet.

CMap::CMap(CInstanceManager* iMan) : CControl(iMan)
{
	CControl::CControl(iMan);

	m_main    = (CRobotMain*)m_iMan->SearchInstance(CLASS_MAIN);
	m_terrain = (CTerrain*)m_iMan->SearchInstance(CLASS_TERRAIN);
	m_water   = (CWater*)m_iMan->SearchInstance(CLASS_WATER);

	m_bEnable  = TRUE;
	m_time     = 0.0f;
	m_zoom     = 2.0f;
	m_offset.x = 0.0f;
	m_offset.y = 0.0f;

	m_floorColor.r = 1.00f;
	m_floorColor.g = 0.50f;
	m_floorColor.b = 0.00f;  // orange

	m_waterColor.r = 0.00f;
	m_waterColor.g = 0.80f;
	m_waterColor.b = 1.00f;  // bleu

	m_half = m_terrain->RetMosaic()*m_terrain->RetBrick()*m_terrain->RetSize()/2.0f;

	m_hiliteRank = -1;
	FlushObject();
}

// Destructeur de l'objet.

CMap::~CMap()
{
	CControl::~CControl();
}


// Crée un nouveau bouton.

BOOL CMap::Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
{
	if ( eventMsg == EVENT_NULL )  eventMsg = GetUniqueEventMsg();

	CControl::Create(pos, dim, icon, eventMsg);
	return TRUE;
}


// Choix du facteur de zoom de la carte.

void CMap::SetZoom(float value)
{
	m_zoom = value;
}

float CMap::RetZoom()
{
	return m_zoom;
}


// Active ou désactive la carte.

void CMap::SetEnable(BOOL bEnable)
{
	m_bEnable = bEnable;
	SetState(STATE_DEAD, !bEnable);
}

BOOL CMap::RetEnable()
{
	return m_bEnable;
}


// Choix de la couleur du sol.

void CMap::SetFloorColor(D3DCOLORVALUE color)
{
	m_floorColor = color;
}

// Choix de la couleur de l'eau.

void CMap::SetWaterColor(D3DCOLORVALUE color)
{
	m_waterColor = color;
}


// Gestion d'un événement.

BOOL CMap::EventProcess(const Event &event)
{
	BOOL	bInMap;

	if ( (m_state & STATE_VISIBLE) == 0 )  return TRUE;

	CControl::EventProcess(event);

	if ( event.event == EVENT_FRAME )
	{
		m_time += event.rTime;
	}

	if ( event.event == EVENT_MOUSEMOVE && Detect(event.pos) )
	{
		m_engine->SetMouseType(D3DMOUSENORM);
		if ( DetectObject(event.pos, bInMap) != 0 )
		{
			m_engine->SetMouseType(D3DMOUSEHAND);
		}
	}

	if ( event.event == EVENT_LBUTTONDOWN )
	{
		if ( CControl::Detect(event.pos) )
		{
			SelectObject(event.pos);
			return FALSE;
		}
	}

	return TRUE;
}

// Ajuste l'offset pour ne pas dépasser la carte.

FPOINT CMap::AdjustOffset(FPOINT offset)
{
	float	limit;

	limit = m_half - m_half/m_zoom;
	if ( offset.x < -limit )  offset.x = -limit;
	if ( offset.x >  limit )  offset.x =  limit;
	if ( offset.y < -limit )  offset.y = -limit;
	if ( offset.y >  limit )  offset.y =  limit;

	return offset;
}

// Indique l'objet survolé par la souris.

void CMap::SetHilite(CObject* pObj)
{
	int		i;

	m_hiliteRank = -1;
	if ( pObj == 0 )  return;

	for ( i=0 ; i<MAPMAXOBJECT ; i++ )
	{
		if ( !m_map[i].bUsed )  continue;

		if ( m_map[i].object == pObj )
		{
			m_hiliteRank = i;
			break;
		}
	}
}

// Détecte un objet dans la carte.

CObject* CMap::DetectObject(FPOINT pos, BOOL &bInMap)
{
	float		dist, min;
	int			i, best;

	bInMap = FALSE;
	if ( pos.x < m_pos.x         ||
		 pos.y < m_pos.y         ||
		 pos.x > m_pos.x+m_dim.x ||
		 pos.y > m_pos.y+m_dim.y )  return 0;

	bInMap = TRUE;

	pos.x = (pos.x-m_pos.x)/m_dim.x*256.0f;
	pos.y = (pos.y-m_pos.y)/m_dim.y*256.0f;  // 0..256
	pos.x = (pos.x-128.0f)*m_half/(m_zoom*128.0f)+m_offset.x;
	pos.y = (pos.y-128.0f)*m_half/(m_zoom*128.0f)+m_offset.y;

	min = 10000.0f;
	best = -1;
	for ( i=MAPMAXOBJECT-1 ; i>=0 ; i-- )
	{
		if ( !m_map[i].bUsed  )  continue;
		if ( m_map[i].color == MAPCOLOR_BBOX  && !m_bRadar )  continue;
		if ( m_map[i].color == MAPCOLOR_ALIEN && !m_bRadar )  continue;

		dist = Length(m_map[i].pos.x-pos.x, m_map[i].pos.y-pos.y);
		if ( dist > m_half/m_zoom*8.0f/100.0f )  continue;  // trop loin ?
		if ( dist < min )
		{
			min = dist;
			best = i;
		}
	}
	if ( best == -1 )  return 0;
	return m_map[best].object;
}

// Sélectionne un objet.

void CMap::SelectObject(FPOINT pos)
{
	CObject		*pObj;
	BOOL		bInMap;

	pObj = DetectObject(pos, bInMap);
	if ( pObj != 0 )
	{
		m_main->SelectObject(pObj);
	}
}


// Dessine la carte.

void CMap::Draw()
{
	FPOINT		uv1, uv2;
	int			i;

	if ( (m_state & STATE_VISIBLE) == 0 )  return;

	CControl::Draw();  // dessine le fond (bouton)

	if ( !m_bEnable )  return;

	if ( m_map[MAPMAXOBJECT-1].bUsed )
	{
		m_offset = AdjustOffset(m_map[MAPMAXOBJECT-1].pos);
	}

	m_engine->SetTexture("map.tga");
	m_engine->SetState(D3DSTATENORMAL);
	uv1.x = 0.5f+(m_offset.x-(m_half/m_zoom))/(m_half*2.0f);
	uv1.y = 0.5f-(m_offset.y+(m_half/m_zoom))/(m_half*2.0f);
	uv2.x = 0.5f+(m_offset.x+(m_half/m_zoom))/(m_half*2.0f);
	uv2.y = 0.5f-(m_offset.y-(m_half/m_zoom))/(m_half*2.0f);
	DrawVertex(uv1, uv2, 0.97f);  // dessine la carte

	i = MAPMAXOBJECT-1;
	if ( m_map[i].bUsed )  // sélection :
	{
		DrawFocus(m_map[i].pos, m_map[i].dir, m_map[i].type, m_map[i].color);
	}

	for ( i=0 ; i<m_totalFix ; i++ )  // objets fixes :
	{
		if ( i == m_hiliteRank )  continue;
		DrawObject(m_map[i].pos, m_map[i].dir, m_map[i].type, m_map[i].color, FALSE, FALSE);
	}

	for ( i=MAPMAXOBJECT-2 ; i>m_totalMove ; i-- )  // objets mobiles :
	{
		if ( i == m_hiliteRank )  continue;
		DrawObject(m_map[i].pos, m_map[i].dir, m_map[i].type, m_map[i].color, FALSE, FALSE);
	}

	i = MAPMAXOBJECT-1;
	if ( m_map[i].bUsed && i != m_hiliteRank )  // sélection :
	{
		DrawObject(m_map[i].pos, m_map[i].dir, m_map[i].type, m_map[i].color, TRUE, FALSE);
	}

	if ( m_hiliteRank != -1 && m_map[m_hiliteRank].bUsed )
	{
		i = m_hiliteRank;
		DrawObject(m_map[i].pos, m_map[i].dir, m_map[i].type, m_map[i].color, FALSE, TRUE);
		DrawHilite(m_map[i].pos);
	}
}

// Calcul un point pour DrawFocus.

FPOINT CMap::MapInter(FPOINT pos, float dir)
{
	FPOINT	p1;
	float	limit;

	p1.x = pos.x+1.0f;
	p1.y = pos.y;
	p1 = RotatePoint(pos, dir, p1);

	p1.x -= pos.x;
	p1.y -= pos.y;

	limit = m_mapPos.x+m_mapDim.x-pos.x;
	if ( p1.x > limit )  // dépasse à droite ?
	{
		p1.y = limit*p1.y/p1.x;
		p1.x = limit;
	}
	limit = m_mapPos.y*0.75f+m_mapDim.y*0.75f-pos.y;
	if ( p1.y > limit )  // dépasse en haut ?
	{
		p1.x = limit*p1.x/p1.y;
		p1.y = limit;
	}
	limit = m_mapPos.x-pos.x;
	if ( p1.x < limit )  // dépasse à gauche ?
	{
		p1.y = limit*p1.y/p1.x;
		p1.x = limit;
	}
	limit = m_mapPos.y*0.75f-pos.y;
	if ( p1.y < limit )  // dépasse en bas ?
	{
		p1.x = limit*p1.x/p1.y;
		p1.y = limit;
	}

	p1.x += pos.x;
	p1.y += pos.y;
	return p1;
}

// Dessine le champ de vision de l'objet sélectionné.

void CMap::DrawFocus(FPOINT pos, float dir, ObjectType type, MapColor color)
{
	FPOINT	p0, p1, p2, uv1, uv2, rel;
	float	aMin, aMax, aOct, focus, a;
	float	limit[5];
	BOOL	bEnding;
	int		quart;

	if ( color != MAPCOLOR_MOVE )  return;

	pos.x = (pos.x-m_offset.x)*(m_zoom*0.5f)/m_half+0.5f;
	pos.y = (pos.y-m_offset.y)*(m_zoom*0.5f)/m_half+0.5f;

	if ( pos.x < 0.0f || pos.x > 1.0f ||
		 pos.y < 0.0f || pos.y > 1.0f )  return;

	rel.x = pos.x*2.0f-1.0f;
	rel.y = pos.y*2.0f-1.0f;  // rel [-1..1]

	pos.x = m_mapPos.x+m_mapDim.x*pos.x;
	pos.y = m_mapPos.y*0.75f+m_mapDim.y*pos.y*0.75f;

	focus = m_engine->RetFocus();
	dir += PI/2.0f;
	aMin = NormAngle(dir-PI/4.0f*focus);
	aMax = NormAngle(dir+PI/4.0f*focus);

	if ( aMin > aMax )
	{
		aMax += PI*2.0f;  // aMax toujours après aMin
	}

	limit[0] = RotateAngle( 1.0f-rel.x,  1.0f-rel.y);  // sup/droite
	limit[1] = RotateAngle(-1.0f-rel.x,  1.0f-rel.y);  // sup/gauche
	limit[2] = RotateAngle(-1.0f-rel.x, -1.0f-rel.y);  // inf/gauche
	limit[3] = RotateAngle( 1.0f-rel.x, -1.0f-rel.y);  // inf/droite
	limit[4] = limit[0]+PI*2.0f;

	a = NormAngle(aMin);
	for ( quart=0 ; quart<4 ; quart++ )
	{
		if ( a >= limit[quart+0] &&
			 a <= limit[quart+1] )  break;
	}
	if ( quart == 4 )  quart = -1;

	uv1.x = 113.0f/256.0f;  // dégradé vert
	uv1.y = 240.5f/256.0f;
	uv2.x = 126.0f/256.0f;
	uv2.y = 255.0f/256.0f;

	m_engine->SetTexture("button2.tga");
	m_engine->SetState(D3DSTATETTw);

	bEnding = FALSE;
	do
	{
		quart ++;
		aOct = limit[quart%4];
		if ( quart >= 4 )  aOct += PI*2.0f;
		if ( aOct >= aMax-CHOUIA )
		{
			aOct = aMax;
			bEnding = TRUE;
		}

		p0 = pos;
		p1 = MapInter(pos, aMin);
		p2 = MapInter(pos, aOct);
		p0.y /= 0.75f;
		p1.y /= 0.75f;
		p2.y /= 0.75f;
		DrawTriangle(p0, p2, p1, uv1, uv2);

		aMin = aOct;
	}
	while ( !bEnding );
}

// Dessine un objet.

void CMap::DrawObject(FPOINT pos, float dir, ObjectType type, MapColor color,
					  BOOL bSelect, BOOL bHilite)
{
	FPOINT		p1, p2, p3, dim, uv1, uv2;
	BOOL		bOut, bUp, bDown, bLeft, bRight;

	pos.x = (pos.x-m_offset.x)*(m_zoom*0.5f)/m_half+0.5f;
	pos.y = (pos.y-m_offset.y)*(m_zoom*0.5f)/m_half+0.5f;

	bOut = bUp = bDown = bLeft = bRight = FALSE;
	if ( pos.x < 0.06f )  { pos.x = 0.02f;  bOut = bLeft  = TRUE; }
	if ( pos.y < 0.06f )  { pos.y = 0.02f;  bOut = bDown  = TRUE; }
	if ( pos.x > 0.94f )  { pos.x = 0.98f;  bOut = bRight = TRUE; }
	if ( pos.y > 0.94f )  { pos.y = 0.98f;  bOut = bUp    = TRUE; }

	pos.x = m_mapPos.x+m_mapDim.x*pos.x;
	pos.y = m_mapPos.y+m_mapDim.y*pos.y;
	dim.x = 2.0f/128.0f*0.75f;
	dim.y = 2.0f/128.0f;

	if ( bOut )  // hors de la carte ?
	{
		if ( color == MAPCOLOR_BBOX  && !m_bRadar )  return;
		if ( color == MAPCOLOR_ALIEN && !m_bRadar )  return;

		if ( Mod(m_time+(pos.x+pos.y)*4.0f, 0.6f) > 0.2f )
		{
			return;  // clignotte
		}

		m_engine->SetTexture("button2.tga");
		m_engine->SetState(D3DSTATETTb);
		if ( bUp )
		{
			uv1.x = 160.5f/256.0f;  // triangle jaune ^
			uv1.y = 240.5f/256.0f;
			uv2.x = 175.0f/256.0f;
			uv2.y = 255.0f/256.0f;
		}
		if ( bDown )
		{
			uv1.x = 160.5f/256.0f;  // triangle jaune v
			uv1.y = 255.0f/256.0f;
			uv2.x = 175.0f/256.0f;
			uv2.y = 240.5f/256.0f;
		}
		if ( bRight )
		{
			uv1.x = 176.5f/256.0f;  // triangle jaune >
			uv1.y = 240.5f/256.0f;
			uv2.x = 191.0f/256.0f;
			uv2.y = 255.0f/256.0f;
		}
		if ( bLeft )
		{
			uv1.x = 191.0f/256.0f;  // triangle jaune <
			uv1.y = 240.5f/256.0f;
			uv2.x = 176.5f/256.0f;
			uv2.y = 255.0f/256.0f;
		}
		pos.x -= dim.x/2.0f;
		pos.y -= dim.y/2.0f;
		DrawIcon(pos, dim, uv1, uv2);
		return;
	}

	if ( bSelect )
	{
		dim.x *= 1.2f+sinf(m_time*8.0f)*0.3f;
		dim.y *= 1.2f+sinf(m_time*8.0f)*0.3f;
	}
	if ( color == MAPCOLOR_BASE ||
		 color == MAPCOLOR_FIX  ||
		 color == MAPCOLOR_MOVE )
	{
		if ( bHilite )
		{
			dim.x *= 2.2f;
			dim.y *= 2.2f;
		}
		else
		{
			dim.x *= 0.6f;
			dim.y *= 0.6f;
		}
	}
	if ( color == MAPCOLOR_ALIEN )
	{
		dim.x *= 1.4f;
		dim.y *= 1.4f;
	}

	if ( color == MAPCOLOR_MOVE && bSelect )
	{
		p1.x = pos.x;
		p1.y = pos.y+dim.y*2.4f;
		p1 = RotatePoint(pos, dir, p1);
		p1.x = pos.x+(p1.x-pos.x)*0.75f;

		p2.x = pos.x+dim.x*1.0f;
		p2.y = pos.y-dim.y*1.6f;
		p2 = RotatePoint(pos, dir, p2);
		p2.x = pos.x+(p2.x-pos.x)*0.75f;

		p3.x = pos.x-dim.x*1.0f;
		p3.y = pos.y-dim.y*1.6f;
		p3 = RotatePoint(pos, dir, p3);
		p3.x = pos.x+(p3.x-pos.x)*0.75f;
	}

	pos.x -= dim.x/2.0f;
	pos.y -= dim.y/2.0f;

	if ( color == MAPCOLOR_BASE ||
		 color == MAPCOLOR_FIX  )
	{
		DrawObjectIcon(pos, dim, color, type, bHilite);
	}

	if ( color == MAPCOLOR_MOVE )
	{
		if ( bSelect )
		{
			m_engine->SetTexture("button2.tga");
			m_engine->SetState(D3DSTATENORMAL);
			uv1.x = 144.5f/256.0f;  // triangle rouge
			uv1.y = 240.5f/256.0f;
			uv2.x = 159.0f/256.0f;
			uv2.y = 255.0f/256.0f;
			DrawTriangle(p1, p2, p3, uv1, uv2);
		}
		DrawObjectIcon(pos, dim, color, type, bHilite);
	}

	if ( color == MAPCOLOR_BBOX )
	{
		if ( m_bRadar )
		{
			m_engine->SetTexture("button2.tga");
			m_engine->SetState(D3DSTATETTw);
			uv1.x =  64.5f/256.0f;  // triangle bleu
			uv1.y = 240.5f/256.0f;
			uv2.x =  79.0f/256.0f;
			uv2.y = 255.0f/256.0f;
			DrawIcon(pos, dim, uv1, uv2);
		}
	}

	if ( color == MAPCOLOR_ALIEN )
	{
		if ( m_bRadar )
		{
			DrawObjectIcon(pos, dim, color, type, TRUE);
		}
	}

	if ( color == MAPCOLOR_WAYPOINTb )
	{
		m_engine->SetTexture("button2.tga");
		m_engine->SetState(D3DSTATETTb);
		uv1.x = 192.5f/256.0f;  // croix bleue
		uv1.y = 240.5f/256.0f;
		uv2.x = 207.0f/256.0f;
		uv2.y = 255.0f/256.0f;
		DrawIcon(pos, dim, uv1, uv2);
	}
	if ( color == MAPCOLOR_WAYPOINTr )
	{
		m_engine->SetTexture("button2.tga");
		m_engine->SetState(D3DSTATETTb);
		uv1.x = 208.5f/256.0f;  // croix rouge
		uv1.y = 240.5f/256.0f;
		uv2.x = 223.0f/256.0f;
		uv2.y = 255.0f/256.0f;
		DrawIcon(pos, dim, uv1, uv2);
	}
	if ( color == MAPCOLOR_WAYPOINTg )
	{
		m_engine->SetTexture("button2.tga");
		m_engine->SetState(D3DSTATETTb);
		uv1.x = 224.5f/256.0f;  // croix verte
		uv1.y = 240.5f/256.0f;
		uv2.x = 239.0f/256.0f;
		uv2.y = 255.0f/256.0f;
		DrawIcon(pos, dim, uv1, uv2);
	}
	if ( color == MAPCOLOR_WAYPOINTy )
	{
		m_engine->SetTexture("button2.tga");
		m_engine->SetState(D3DSTATETTb);
		uv1.x = 240.5f/256.0f;  // croix jaune
		uv1.y = 240.5f/256.0f;
		uv2.x = 255.0f/256.0f;
		uv2.y = 255.0f/256.0f;
		DrawIcon(pos, dim, uv1, uv2);
	}
	if ( color == MAPCOLOR_WAYPOINTv )
	{
		m_engine->SetTexture("button2.tga");
		m_engine->SetState(D3DSTATETTb);
		uv1.x = 192.5f/256.0f;  // croix violette
		uv1.y = 224.5f/256.0f;
		uv2.x = 207.0f/256.0f;
		uv2.y = 239.0f/256.0f;
		DrawIcon(pos, dim, uv1, uv2);
	}
}

// Dessine l'icône d'un objet.

void CMap::DrawObjectIcon(FPOINT pos, FPOINT dim, MapColor color,
						  ObjectType type, BOOL bHilite)
{
}

// Dessine l'objet survolé par la souris.

void CMap::DrawHilite(FPOINT pos)
{
	FPOINT		dim, uv1, uv2;
	BOOL		bOut, bUp, bDown, bLeft, bRight;

	pos.x = (pos.x-m_offset.x)*(m_zoom*0.5f)/m_half+0.5f;
	pos.y = (pos.y-m_offset.y)*(m_zoom*0.5f)/m_half+0.5f;

	bOut = bUp = bDown = bLeft = bRight = FALSE;
	if ( pos.x < 0.06f )  { pos.x = 0.02f;  bOut = bLeft  = TRUE; }
	if ( pos.y < 0.06f )  { pos.y = 0.02f;  bOut = bDown  = TRUE; }
	if ( pos.x > 0.94f )  { pos.x = 0.98f;  bOut = bRight = TRUE; }
	if ( pos.y > 0.94f )  { pos.y = 0.98f;  bOut = bUp    = TRUE; }

	pos.x = m_mapPos.x+m_mapDim.x*pos.x;
	pos.y = m_mapPos.y+m_mapDim.y*pos.y;
	dim.x = 2.0f/128.0f*0.75f;
	dim.y = 2.0f/128.0f;
	dim.x *= 2.0f+cosf(m_time*8.0f)*0.5f;
	dim.y *= 2.0f+cosf(m_time*8.0f)*0.5f;

	m_engine->SetTexture("button2.tga");
	m_engine->SetState(D3DSTATETTb);
	uv1.x = 160.5f/256.0f;  // hilite
	uv1.y = 224.5f/256.0f;
	uv2.x = 175.0f/256.0f;
	uv2.y = 239.0f/256.0f;
	pos.x -= dim.x/2.0f;
	pos.y -= dim.y/2.0f;
	DrawIcon(pos, dim, uv1, uv2);
}

// Dessine une icône triangulaire.

void CMap::DrawTriangle(FPOINT p1, FPOINT p2, FPOINT p3, FPOINT uv1, FPOINT uv2)
{
	LPDIRECT3DDEVICE7 device;
	D3DVERTEX2	vertex[3];	// 1 triangle
	D3DVECTOR	n;

	device = m_engine->RetD3DDevice();

	n = D3DVECTOR(0.0f, 0.0f, -1.0f);  // normale

	vertex[0] = D3DVERTEX2(D3DVECTOR(p1.x, p1.y, 0.0f), n, uv1.x,uv1.y);
	vertex[1] = D3DVERTEX2(D3DVECTOR(p2.x, p2.y, 0.0f), n, uv1.x,uv2.y);
	vertex[2] = D3DVERTEX2(D3DVECTOR(p3.x, p3.y, 0.0f), n, uv2.x,uv2.y);

	device->DrawPrimitive(D3DPT_TRIANGLELIST, D3DFVF_VERTEX2, vertex, 3, NULL);
	m_engine->AddStatisticTriangle(1);
}

// Dessine le tableau des vertex.

void CMap::DrawVertex(FPOINT uv1, FPOINT uv2, float zoom)
{
	LPDIRECT3DDEVICE7 device;
	D3DVERTEX2	vertex[4];	// 2 triangles
	FPOINT		p1, p2, c;
	D3DVECTOR	n;

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

	m_mapPos   = p1;
	m_mapDim.x = p2.x-p1.x;
	m_mapDim.y = p2.y-p1.y;

	n = D3DVECTOR(0.0f, 0.0f, -1.0f);  // normale

	vertex[0] = D3DVERTEX2(D3DVECTOR(p1.x, p1.y, 0.0f), n, uv1.x,uv2.y);
	vertex[1] = D3DVERTEX2(D3DVECTOR(p1.x, p2.y, 0.0f), n, uv1.x,uv1.y);
	vertex[2] = D3DVERTEX2(D3DVECTOR(p2.x, p1.y, 0.0f), n, uv2.x,uv2.y);
	vertex[3] = D3DVERTEX2(D3DVECTOR(p2.x, p2.y, 0.0f), n, uv2.x,uv1.y);

	device->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 4, NULL);
	m_engine->AddStatisticTriangle(2);
}


// Met à jour le terrain dans la carte.

void CMap::UpdateTerrain()
{
	D3DCOLORVALUE	color;
	D3DVECTOR		pos;
	float			scale, water, level, intensity;
	int				x, y;

	if ( !m_engine->OpenImage("map.tga") )  return;

	scale = m_terrain->RetScaleRelief();
	water = m_water->RetLevel();
	color.a = 0.0f;

	for ( y=0 ; y<256 ; y++ )
	{
		for ( x=0 ; x<256 ; x++ )
		{
			pos.x =  ((float)x-128.0f)*m_half/128.0f;
			pos.z = -((float)y-128.0f)*m_half/128.0f;
			pos.y = 0.0f;

#if 0
			if ( pos.x >= -m_half && pos.x <= m_half &&
				 pos.z >= -m_half && pos.z <= m_half )
			{
				level = m_terrain->RetFloorLevel(pos, TRUE)/scale;
			}
			else
			{
				level = 1000.0f;
			}
			intensity = Norm(level/256.0f);

			if ( level >= water )  // sur l'eau ?
			{
				color.r = m_floorColor.r + (intensity-0.5f);
				color.g = m_floorColor.g + (intensity-0.5f);
				color.b = m_floorColor.b + (intensity-0.5f);
			}
			else	// sous l'eau ?
			{
				color.r = m_waterColor.r + (intensity-0.5f);
				color.g = m_waterColor.g + (intensity-0.5f);
				color.b = m_waterColor.b + (intensity-0.5f);
			}
#else
			if ( pos.x >= -m_half && pos.x <= m_half &&
				 pos.z >= -m_half && pos.z <= m_half )
			{
				level = m_terrain->RetHardness(pos);
			}
			else
			{
				level = 1.0f;
			}
			intensity = Norm(level);

			if ( level >= water )  // sur l'eau ?
			{
				color.r = m_floorColor.r + (intensity-0.5f);
				color.g = m_floorColor.g + (intensity-0.5f);
				color.b = m_floorColor.b + (intensity-0.5f);
			}
			else	// sous l'eau ?
			{
				color.r = m_waterColor.r + (intensity-0.5f);
				color.g = m_waterColor.g + (intensity-0.5f);
				color.b = m_waterColor.b + (intensity-0.5f);
			}
#endif

			m_engine->SetDot(x, y, color);
		}
	}

	m_engine->CopyImage();  // copie avec juste le terrain dessiné
	m_engine->CloseImage();
}

// Met à jour le terrain dans la carte.

void CMap::UpdateTerrain(int bx, int by, int ex, int ey)
{
	D3DCOLORVALUE	color;
	D3DVECTOR		pos;
	float			scale, water, level, intensity;
	int				x, y;

	if ( !m_engine->OpenImage("map.tga") )  return;
	m_engine->LoadImage();

	scale = m_terrain->RetScaleRelief();
	water = m_water->RetLevel();
	color.a = 0.0f;

	for ( y=by ; y<ey ; y++ )
	{
		for ( x=bx ; x<ex ; x++ )
		{
			pos.x =  ((float)x-128.0f)*m_half/128.0f;
			pos.z = -((float)y-128.0f)*m_half/128.0f;
			pos.y = 0.0f;

			if ( pos.x >= -m_half && pos.x <= m_half &&
				 pos.z >= -m_half && pos.z <= m_half )
			{
				level = m_terrain->RetFloorLevel(pos, TRUE)/scale;
			}
			else
			{
				level = 1000.0f;
			}

			intensity = level/256.0f;
			if ( intensity < 0.0f )  intensity = 0.0f;
			if ( intensity > 1.0f )  intensity = 1.0f;

			if ( level > water )  // sur l'eau ?
			{
				color.r = m_floorColor.r + (intensity-0.5f);
				color.g = m_floorColor.g + (intensity-0.5f);
				color.b = m_floorColor.b + (intensity-0.5f);
			}
			else	// sous l'eau ?
			{
				color.r = m_waterColor.r + (intensity-0.5f);
				color.g = m_waterColor.g + (intensity-0.5f);
				color.b = m_waterColor.b + (intensity-0.5f);
			}

			m_engine->SetDot(x, y, color);
		}
	}

	m_engine->CopyImage();  // copie avec juste le terrain dessiné
	m_engine->CloseImage();
}


// Vide tous les objets.

void CMap::FlushObject()
{
	int		i;

	m_totalFix  = 0;  // index objet fixes
	m_totalMove = MAPMAXOBJECT-2;  // index véhicules mobiles
	m_bRadar = m_main->RetCheatRadar();  // pas de radar

	for ( i=0 ; i<MAPMAXOBJECT ; i++ )
	{
		m_map[i].bUsed = FALSE;
	}
}

// Met à jour un objet dans la carte.

void CMap::UpdateObject(CObject* pObj)
{
	ObjectType		type;
	MapColor		color;
	D3DVECTOR		pos;
	float			dir;

	if ( !m_bEnable )  return;
	if ( m_totalFix >= m_totalMove )  return;  // table pleine ?

	if ( !pObj->RetActif() )  return;
	if ( !pObj->RetSelectable() )  return;
	if ( pObj->RetTruck() != 0 )  return;

	type = pObj->RetType();
	pos  = pObj->RetPosition(0);
	dir  = -(pObj->RetAngleY(0)+PI/2.0f);

	color = MAPCOLOR_NULL;
	if ( type == OBJECT_FACTORY1 ||
		 type == OBJECT_FACTORY2 ||
		 type == OBJECT_FACTORY3 ||
		 type == OBJECT_FACTORY4 ||
		 type == OBJECT_FACTORY5 ||
		 type == OBJECT_TOWER    ||
		 type == OBJECT_NUCLEAR  ||
		 type == OBJECT_PARA     ||
		 type == OBJECT_DOOR1    ||
		 type == OBJECT_DOOR2    ||
		 type == OBJECT_DOOR3    ||
		 type == OBJECT_DOOR4    ||
		 type == OBJECT_DOOR5    ||
		 type == OBJECT_START    ||
		 type == OBJECT_END      )  // objet fixe ?
	{
		color = MAPCOLOR_FIX;
	}
	if ( type == OBJECT_BBOX ||
		 type == OBJECT_KEYa ||
		 type == OBJECT_KEYb ||
		 type == OBJECT_KEYc ||
		 type == OBJECT_KEYd )
	{
		color = MAPCOLOR_BBOX;
	}
	if ( type == OBJECT_HUMAN    ||
		 type == OBJECT_CAR      ||
		 type == OBJECT_MOBILEtg ||
		 type == OBJECT_MOBILEfb ||
		 type == OBJECT_MOBILEob )  // véhicule mobile ?
	{
		color = MAPCOLOR_MOVE;
	}
	if ( type == OBJECT_WAYPOINT )
	{
		color = MAPCOLOR_WAYPOINTb;
	}

	if ( color == MAPCOLOR_NULL )  return;

	if ( pObj->RetSelect() )
	{
		m_map[MAPMAXOBJECT-1].type   = type;
		m_map[MAPMAXOBJECT-1].object = pObj;
		m_map[MAPMAXOBJECT-1].color  = color;
		m_map[MAPMAXOBJECT-1].pos.x  = pos.x;
		m_map[MAPMAXOBJECT-1].pos.y  = pos.z;
		m_map[MAPMAXOBJECT-1].dir    = dir;
		m_map[MAPMAXOBJECT-1].bUsed  = TRUE;
	}
	else
	{
		if ( color == MAPCOLOR_BASE ||
			 color == MAPCOLOR_FIX  )
		{
			m_map[m_totalFix].type   = type;
			m_map[m_totalFix].object = pObj;
			m_map[m_totalFix].color  = color;
			m_map[m_totalFix].pos.x  = pos.x;
			m_map[m_totalFix].pos.y  = pos.z;
			m_map[m_totalFix].dir    = dir;
			m_map[m_totalFix].bUsed  = TRUE;
			m_totalFix ++;
		}
		else
		{
			m_map[m_totalMove].type   = type;
			m_map[m_totalMove].object = pObj;
			m_map[m_totalMove].color  = color;
			m_map[m_totalMove].pos.x  = pos.x;
			m_map[m_totalMove].pos.y  = pos.z;
			m_map[m_totalMove].dir    = dir;
			m_map[m_totalMove].bUsed  = TRUE;
			m_totalMove --;
		}
	}
}

