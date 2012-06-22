// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.

// planet.cpp


#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "common/struct.h"
#include "math/const.h"
#include "old/d3dengine.h"
#include "old/d3dmath.h"
#include "common/event.h"
#include "common/misc.h"
#include "common/iman.h"
#include "old/math3d.h"
#include "old/planet.h"




// Constructor of the terrain.

CPlanet::CPlanet(CInstanceManager* iMan, CD3DEngine* engine)
{
	m_iMan = iMan;
	m_iMan->AddInstance(CLASS_PLANET, this);

	m_engine = engine;
	Flush();

}

// Destructor of the terrain.

CPlanet::~CPlanet()
{
}


// Removes all the planets.

void CPlanet::Flush()
{
	int		i, j;

	for ( j=0 ; j<2 ; j++ )
	{
		for ( i=0 ; i<MAXPLANET ; i++ )
		{
			m_planet[j][i].bUsed = false;
		}
	}

	m_bPlanetExist = false;
	m_mode = 0;
	m_time = 0.0f;
}


// Management of an event.

bool CPlanet::EventProcess(const Event &event)
{
	if ( event.event == EVENT_FRAME )
	{
		return EventFrame(event);
	}
	return true;
}

// Makes the planets evolve.

bool CPlanet::EventFrame(const Event &event)
{
	float		a;
	int			i;

	if ( m_engine->RetPause() )  return true;

	m_time += event.rTime;

	for ( i=0 ; i<MAXPLANET ; i++ )
	{
		if ( !m_planet[m_mode][i].bUsed )  continue;

		a = m_time*m_planet[m_mode][i].speed;
		if ( a < 0.0f )
		{
			a += Math::PI*1000.0f;
		}
		m_planet[m_mode][i].angle.x = a+m_planet[m_mode][i].start.x;
		m_planet[m_mode][i].angle.y = sinf(a)*sinf(m_planet[m_mode][i].dir)+m_planet[m_mode][i].start.y;
	}

	return true;
}


// Load all the textures for the planets.

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

// Draws all the planets.

void CPlanet::Draw()
{
	LPDIRECT3DDEVICE7 device;
	D3DVERTEX2	vertex[4];	// 2 triangles
	Math::Vector	n;
	Math::Point		p1, p2;
	float		eyeDirH, eyeDirV, dp, u1, u2, v1, v2, a;
	int			i;

	device = m_engine->RetD3DDevice();
	eyeDirH = m_engine->RetEyeDirH();
	eyeDirV = m_engine->RetEyeDirV();

	n = Math::Vector(0.0f, 0.0f, -1.0f);  // normal
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
		p1.x = Math::Mod(a, Math::PI*2.0f)-0.5f;

		a = eyeDirV + m_planet[m_mode][i].angle.y;
		p1.y = 0.4f+(Math::Mod(a+Math::PI, Math::PI*2.0f)-Math::PI)*(2.0f/Math::PI);

		p1.x -= m_planet[m_mode][i].dim/2.0f*0.75f;
		p1.y -= m_planet[m_mode][i].dim/2.0f;
		p2.x = p1.x+m_planet[m_mode][i].dim*0.75f;
		p2.y = p1.y+m_planet[m_mode][i].dim;

		u1 = m_planet[m_mode][i].uv1.x + dp;
		v1 = m_planet[m_mode][i].uv1.y + dp;
		u2 = m_planet[m_mode][i].uv2.x - dp;
		v2 = m_planet[m_mode][i].uv2.y - dp;

		vertex[0] = D3DVERTEX2(Math::Vector(p1.x, p1.y, 0.0f), n, u1,v2);
		vertex[1] = D3DVERTEX2(Math::Vector(p1.x, p2.y, 0.0f), n, u1,v1);
		vertex[2] = D3DVERTEX2(Math::Vector(p2.x, p1.y, 0.0f), n, u2,v2);
		vertex[3] = D3DVERTEX2(Math::Vector(p2.x, p2.y, 0.0f), n, u2,v1);

		device->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 4, NULL);
		m_engine->AddStatisticTriangle(2);
	}
}


// Creates a new planet.

bool CPlanet::Create(int mode, Math::Point start, float dim, float speed,
					 float dir, char *name, Math::Point uv1, Math::Point uv2)
{
	int		i;

	if ( mode < 0 )  mode = 0;
	if ( mode > 1 )  mode = 1;

	for ( i=0 ; i<MAXPLANET ; i++ )
	{
		if ( m_planet[mode][i].bUsed )  continue;

		m_planet[mode][i].bUsed = true;
		m_planet[mode][i].start = start;
		m_planet[mode][i].angle = start;
		m_planet[mode][i].dim   = dim;
		m_planet[mode][i].speed = speed;
		m_planet[mode][i].dir   = dir;

		strcpy(m_planet[mode][i].name, name);
		m_planet[mode][i].uv1   = uv1;
		m_planet[mode][i].uv2   = uv2;

		m_planet[mode][i].bTGA = ( strstr(m_planet[mode][i].name, "planet") != 0 );

		m_bPlanetExist = true;
		return true;
	}

	return false;
}

// Indicates if there is at least one planet.

bool CPlanet::PlanetExist()
{
	return m_bPlanetExist;
}


// Choice of mode.

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

