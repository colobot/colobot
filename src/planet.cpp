// planet.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "D3DEngine.h"
#include "D3DMath.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "math3d.h"
#include "planet.h"




// Constructeur du terrain.

CPlanet::CPlanet(CInstanceManager* iMan, CD3DEngine* engine)
{
	m_iMan = iMan;
	m_iMan->AddInstance(CLASS_PLANET, this);

	m_engine = engine;
	Flush();

}

// Destructeur du terrain.

CPlanet::~CPlanet()
{
}


// Supprime toutes les planètes.

void CPlanet::Flush()
{
	int		i, j;

	for ( j=0 ; j<2 ; j++ )
	{
		for ( i=0 ; i<MAXPLANET ; i++ )
		{
			m_planet[j][i].bUsed = FALSE;
		}
	}

	m_bPlanetExist = FALSE;
	m_mode = 0;
	m_time = 0.0f;
}


// Gestion d'un événement.

BOOL CPlanet::EventProcess(const Event &event)
{
	if ( event.event == EVENT_FRAME )
	{
		return EventFrame(event);
	}
	return TRUE;
}

// Fait évoluer les planètes.

BOOL CPlanet::EventFrame(const Event &event)
{
	float		a;
	int			i;

	if ( m_engine->RetPause() )  return TRUE;

	m_time += event.rTime;

	for ( i=0 ; i<MAXPLANET ; i++ )
	{
		if ( !m_planet[m_mode][i].bUsed )  continue;

		a = m_time*m_planet[m_mode][i].speed;
		if ( a < 0.0f )
		{
			a += PI*1000.0f;
		}
		m_planet[m_mode][i].angle.x = a+m_planet[m_mode][i].start.x;
		m_planet[m_mode][i].angle.y = sinf(a)*sinf(m_planet[m_mode][i].dir)+m_planet[m_mode][i].start.y;
	}

	return TRUE;
}


// Charge toutes les textures pour les planètes.

void CPlanet::LoadTexture()
{
	int		i, j;

	for ( j=0 ; j<2 ; j++ )
	{
		for ( i=0 ; i<MAXPLANET ; i++ )
		{
			if ( !m_planet[j][i].bUsed )  continue;

			m_engine->LoadTexture(m_planet[j][i].name);
		}
	}
}

// Dessine toutes les planètes.

void CPlanet::Draw()
{
	LPDIRECT3DDEVICE7 device;
	D3DVERTEX2	vertex[4];	// 2 triangles
	D3DVECTOR	n;
	FPOINT		p1, p2;
	float		eyeDirH, eyeDirV, dp, u1, u2, v1, v2, a;
	int			i;

	device = m_engine->RetD3DDevice();
	eyeDirH = m_engine->RetEyeDirH();
	eyeDirV = m_engine->RetEyeDirV();

	n = D3DVECTOR(0.0f, 0.0f, -1.0f);  // normale
	dp = 0.5f/256.0f;

	for ( i=0 ; i<MAXPLANET ; i++ )
	{
		if ( !m_planet[m_mode][i].bUsed )  continue;

		m_engine->SetTexture(m_planet[m_mode][i].name);

		if ( m_planet[m_mode][i].bTGA )
		{
			m_engine->SetState(D3DSTATEWRAP|D3DSTATEALPHA);
		}
		else
		{
			m_engine->SetState(D3DSTATEWRAP|D3DSTATETTb);
		}

		a = eyeDirH + m_planet[m_mode][i].angle.x;
		p1.x = Mod(a, PI*2.0f)-0.5f;

		a = eyeDirV + m_planet[m_mode][i].angle.y;
		p1.y = 0.4f+(Mod(a+PI, PI*2.0f)-PI)*(2.0f/PI);

		p1.x -= m_planet[m_mode][i].dim/2.0f*0.75f;
		p1.y -= m_planet[m_mode][i].dim/2.0f;
		p2.x = p1.x+m_planet[m_mode][i].dim*0.75f;
		p2.y = p1.y+m_planet[m_mode][i].dim;

		u1 = m_planet[m_mode][i].uv1.x + dp;
		v1 = m_planet[m_mode][i].uv1.y + dp;
		u2 = m_planet[m_mode][i].uv2.x - dp;
		v2 = m_planet[m_mode][i].uv2.y - dp;

		vertex[0] = D3DVERTEX2(D3DVECTOR(p1.x, p1.y, 0.0f), n, u1,v2);
		vertex[1] = D3DVERTEX2(D3DVECTOR(p1.x, p2.y, 0.0f), n, u1,v1);
		vertex[2] = D3DVERTEX2(D3DVECTOR(p2.x, p1.y, 0.0f), n, u2,v2);
		vertex[3] = D3DVERTEX2(D3DVECTOR(p2.x, p2.y, 0.0f), n, u2,v1);

		device->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 4, NULL);
		m_engine->AddStatisticTriangle(2);
	}
}


// Crée une nouvelle planète.

BOOL CPlanet::Create(int mode, FPOINT start, float dim, float speed,
					 float dir, char *name, FPOINT uv1, FPOINT uv2)
{
	int		i;

	if ( mode < 0 )  mode = 0;
	if ( mode > 1 )  mode = 1;

	for ( i=0 ; i<MAXPLANET ; i++ )
	{
		if ( m_planet[mode][i].bUsed )  continue;

		m_planet[mode][i].bUsed = TRUE;
		m_planet[mode][i].start = start;
		m_planet[mode][i].angle = start;
		m_planet[mode][i].dim   = dim;
		m_planet[mode][i].speed = speed;
		m_planet[mode][i].dir   = dir;

		strcpy(m_planet[mode][i].name, name);
		m_planet[mode][i].uv1   = uv1;
		m_planet[mode][i].uv2   = uv2;

		m_planet[mode][i].bTGA = ( strstr(m_planet[mode][i].name, "planet") != 0 );

		m_bPlanetExist = TRUE;
		return TRUE;
	}

	return FALSE;
}

// Indique s'il existe au moins une planète.

BOOL CPlanet::PlanetExist()
{
	return m_bPlanetExist;
}


// Choix du mode.

void CPlanet::SetMode(int mode)
{
	if ( mode < 0 )  mode = 0;
	if ( mode > 1 )  mode = 1;
	m_mode = mode;
}

int CPlanet::RetMode()
{
	return m_mode;
}

