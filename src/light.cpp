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
// * along with this program. If not, see .

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "D3DEngine.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "math3d.h"
#include "light.h"





// Initialise une progression.

void ProgInit(LightProg &p, float value)
{
	p.starting = value;
	p.ending   = value;
	p.current  = value;
	p.progress = 0.0f;
	p.speed    = 100.0f;
}

// Fait �voluer une progression.

void ProgFrame(LightProg &p, float rTime)
{
	if ( p.speed < 100.0f )
	{
		if ( p.progress < 1.0f )
		{
			p.progress += p.speed*rTime;
			if ( p.progress > 1.0f )
			{
				p.progress = 1.0f;
			}
		}

		p.current = (p.ending-p.starting)*p.progress + p.starting;
	}
	else
	{
		p.current = p.ending;
	}
}

// Change la valeur courante.

void ProgSet(LightProg &p, float value)
{
	p.starting = p.current;
	p.ending   = value;
	p.progress = 0.0f;
}





// Constructeur de l'objet.

CLight::CLight(CInstanceManager* iMan, CD3DEngine* engine)
{
	m_iMan = iMan;
	m_iMan->AddInstance(CLASS_LIGHT, this);

	m_pD3DDevice = 0;
	m_engine = engine;

	m_lightUsed = 0;
	m_lightTable = (Light*)malloc(sizeof(Light)*D3DMAXLIGHT);
	ZeroMemory(m_lightTable, sizeof(Light)*D3DMAXLIGHT);

	m_time = 0.0f;
}

// Destructeur de l'objet.

CLight::~CLight()
{
	free(m_lightTable);
	m_iMan->DeleteInstance(CLASS_LIGHT, this);
}


void CLight::SetD3DDevice(LPDIRECT3DDEVICE7 device)
{
	m_pD3DDevice = device;
}


// Supprime toutes les lumi�res.

void CLight::FlushLight()
{
	int		i;

	for ( i=0 ; i<D3DMAXLIGHT ; i++ )
	{
		m_lightTable[i].bUsed = FALSE;
		m_pD3DDevice->LightEnable(i, FALSE);
	}
	m_lightUsed = 0;
}


// Cr�e une nouvelle lumi�re. Retourne son rang ou -1 en cas d'erreur.

int CLight::CreateLight()
{
	int		i;

	for ( i=0 ; i<D3DMAXLIGHT ; i++ )
	{
		if ( m_lightTable[i].bUsed == FALSE )
		{
			ZeroMemory(&m_lightTable[i], sizeof(Light));
			m_lightTable[i].bUsed   = TRUE;
			m_lightTable[i].bEnable = TRUE;

			m_lightTable[i].incluType = TYPENULL;
			m_lightTable[i].excluType = TYPENULL;

			m_lightTable[i].light.dltType       = D3DLIGHT_DIRECTIONAL;
			m_lightTable[i].light.dcvDiffuse.r  =   0.5f;
			m_lightTable[i].light.dcvDiffuse.g  =   0.5f;
			m_lightTable[i].light.dcvDiffuse.b  =   0.5f;  // blanc
			m_lightTable[i].light.dvPosition.x  =-100.0f;
			m_lightTable[i].light.dvPosition.y  = 100.0f;
			m_lightTable[i].light.dvPosition.z  =-100.0f;
			m_lightTable[i].light.dvDirection.x =   1.0f;
			m_lightTable[i].light.dvDirection.y =  -1.0f;
			m_lightTable[i].light.dvDirection.z =   1.0f;

			ProgInit(m_lightTable[i].intensity,  1.0f);  // maximum
			ProgInit(m_lightTable[i].colorRed,   0.5f);
			ProgInit(m_lightTable[i].colorGreen, 0.5f);
			ProgInit(m_lightTable[i].colorBlue,  0.5f);  // gris

			if ( m_lightUsed < i+1 )
			{
				m_lightUsed = i+1;
			}
			return i;
		}
	}
	return -1;
}

// Supprime une lumi�re.

BOOL CLight::DeleteLight(int lightRank)
{
	int		i;

	if ( lightRank < 0 || lightRank >= D3DMAXLIGHT )  return FALSE;

	m_lightTable[lightRank].bUsed = FALSE;
	m_pD3DDevice->LightEnable(lightRank, FALSE);

	m_lightUsed = 0;
	for ( i=0 ; i<D3DMAXLIGHT ; i++ )
	{
		if ( m_lightTable[i].bUsed == TRUE )
		{
			m_lightUsed = i+1;
		}
	}

	return TRUE;
}


// Sp�cifie une lumi�re.

BOOL CLight::SetLight(int lightRank, const D3DLIGHT7 &light)
{
	if ( lightRank < 0 || lightRank >= D3DMAXLIGHT )  return FALSE;

	m_lightTable[lightRank].light = light;

	ProgInit(m_lightTable[lightRank].colorRed,   m_lightTable[lightRank].light.dcvDiffuse.r);
	ProgInit(m_lightTable[lightRank].colorGreen, m_lightTable[lightRank].light.dcvDiffuse.g);
	ProgInit(m_lightTable[lightRank].colorBlue,  m_lightTable[lightRank].light.dcvDiffuse.b);

	return TRUE;
}

// Donne les sp�cifications d'une lumi�re.

BOOL CLight::GetLight(int lightRank, D3DLIGHT7 &light)
{
	if ( lightRank < 0 || lightRank >= D3DMAXLIGHT )  return FALSE;

	light = m_lightTable[lightRank].light;
	return TRUE;
}


// Allume ou �teint une lumi�re.

BOOL CLight::LightEnable(int lightRank, BOOL bEnable)
{
	if ( lightRank < 0 || lightRank >= D3DMAXLIGHT )  return FALSE;

	m_lightTable[lightRank].bEnable = bEnable;
	return TRUE;
}


// Sp�cifie le type (TYPE*) des objets inclus par cette lumi�re.
// Cette lumi�re n'�clairera donc que ce type d'objets.

BOOL CLight::SetLightIncluType(int lightRank, D3DTypeObj type)
{
	if ( lightRank < 0 || lightRank >= D3DMAXLIGHT )  return FALSE;

	m_lightTable[lightRank].incluType = type;
	return TRUE;
}

// Sp�cifie le type (TYPE*) des objets exclus par cette lumi�re.
// Cette lumi�re n'�clairera donc jamais ce type d'objets.

BOOL CLight::SetLightExcluType(int lightRank, D3DTypeObj type)
{
	if ( lightRank < 0 || lightRank >= D3DMAXLIGHT )  return FALSE;

	m_lightTable[lightRank].excluType = type;
	return TRUE;
}


// Gestion de la position de la luni�re.

BOOL CLight::SetLightPos(int lightRank, D3DVECTOR pos)
{
	if ( lightRank < 0 || lightRank >= D3DMAXLIGHT )  return FALSE;

	m_lightTable[lightRank].light.dvPosition = pos;
	return TRUE;
}

D3DVECTOR CLight::RetLightPos(int lightRank)
{
	if ( lightRank < 0 || lightRank >= D3DMAXLIGHT )  return D3DVECTOR(0.0f, 0.0f, 0.0f);

	return m_lightTable[lightRank].light.dvPosition;
}


// Gestion de la direction de la lumi�re.

BOOL CLight::SetLightDir(int lightRank, D3DVECTOR dir)
{
	if ( lightRank < 0 || lightRank >= D3DMAXLIGHT )  return FALSE;

	m_lightTable[lightRank].light.dvDirection = dir;
	return TRUE;
}

D3DVECTOR CLight::RetLightDir(int lightRank)
{
	if ( lightRank < 0 || lightRank >= D3DMAXLIGHT )  return D3DVECTOR(0.0f, 0.0f, 0.0f);

	return m_lightTable[lightRank].light.dvDirection;
}


// Sp�cifie la vitesse de changement.

BOOL CLight::SetLightIntensitySpeed(int lightRank, float speed)
{
	if ( lightRank < 0 || lightRank >= D3DMAXLIGHT )  return FALSE;

	m_lightTable[lightRank].intensity.speed = speed;
	return TRUE;
}

// Gestion de l'intensit� de la lumi�re.

BOOL CLight::SetLightIntensity(int lightRank, float value)
{
	if ( lightRank < 0 || lightRank >= D3DMAXLIGHT )  return FALSE;

	ProgSet(m_lightTable[lightRank].intensity, value);
	return TRUE;
}

float CLight::RetLightIntensity(int lightRank)
{
	if ( lightRank < 0 || lightRank >= D3DMAXLIGHT )  return 0.0f;

	return m_lightTable[lightRank].intensity.current;
}


// Sp�cifie la vitesse de changement.

BOOL CLight::SetLightColorSpeed(int lightRank, float speed)
{
	if ( lightRank < 0 || lightRank >= D3DMAXLIGHT )  return FALSE;

	m_lightTable[lightRank].colorRed.speed   = speed;
	m_lightTable[lightRank].colorGreen.speed = speed;
	m_lightTable[lightRank].colorBlue.speed  = speed;
	return TRUE;
}

// Gestion de la couleur de la lumi�re.

BOOL CLight::SetLightColor(int lightRank, D3DCOLORVALUE color)
{
	if ( lightRank < 0 || lightRank >= D3DMAXLIGHT )  return FALSE;

	ProgSet(m_lightTable[lightRank].colorRed,   color.r);
	ProgSet(m_lightTable[lightRank].colorGreen, color.g);
	ProgSet(m_lightTable[lightRank].colorBlue,  color.b);
	return TRUE;
}

D3DCOLORVALUE CLight::RetLightColor(int lightRank)
{
	D3DCOLORVALUE	color;

	if ( lightRank < 0 || lightRank >= D3DMAXLIGHT )
	{
		color.r = 0.5f;
		color.g = 0.5f;
		color.b = 0.5f;
		color.a = 0.5f;
		return color;
	}

	color.r = m_lightTable[lightRank].colorRed.current;
	color.g = m_lightTable[lightRank].colorGreen.current;
	color.b = m_lightTable[lightRank].colorBlue.current;
	return color;
}


// Adapte la couleur de toutes les lumi�res.

void CLight::AdaptLightColor(D3DCOLORVALUE color, float factor)
{
	D3DCOLORVALUE	value;
	int				i;

	for ( i=0 ; i<m_lightUsed ; i++ )
	{
		if ( m_lightTable[i].bUsed == FALSE )  continue;

		value.r = m_lightTable[i].colorRed.current;
		value.g = m_lightTable[i].colorGreen.current;
		value.b = m_lightTable[i].colorBlue.current;

		value.r += color.r*factor;
		value.g += color.g*factor;
		value.b += color.b*factor;

		ProgInit(m_lightTable[i].colorRed,   value.r);
		ProgInit(m_lightTable[i].colorGreen, value.g);
		ProgInit(m_lightTable[i].colorBlue,  value.b);
	}

	LightUpdate();
}



// Fait �voluer toutes les lumi�res.

void CLight::FrameLight(float rTime)
{
	D3DVECTOR	dir;
	float		angle;
	int			i;

	if ( m_engine->RetPause() )  return;

	m_time += rTime;

	for ( i=0 ; i<m_lightUsed ; i++ )
	{
		if ( m_lightTable[i].bUsed == FALSE )  continue;

		ProgFrame(m_lightTable[i].intensity,  rTime);
		ProgFrame(m_lightTable[i].colorRed,   rTime);
		ProgFrame(m_lightTable[i].colorGreen, rTime);
		ProgFrame(m_lightTable[i].colorBlue,  rTime);

		if ( m_lightTable[i].incluType == TYPEQUARTZ )
		{
			m_lightTable[i].light.dvDirection.x = sinf((m_time+i*PI*0.5f)*1.0f);
			m_lightTable[i].light.dvDirection.z = cosf((m_time+i*PI*0.5f)*1.1f);
			m_lightTable[i].light.dvDirection.y = -1.0f+cosf((m_time+i*PI*0.5f)*2.7f)*0.5f;
		}

		if ( m_lightTable[i].incluType == TYPEMETAL )
		{
			dir = m_engine->RetEyePt()-m_engine->RetLookatPt();
			angle = RotateAngle(dir.x, dir.z);
			angle += PI*0.5f*i;
			m_lightTable[i].light.dvDirection.x = sinf(angle*2.0f);
			m_lightTable[i].light.dvDirection.z = cosf(angle*2.0f);
		}
	}
}


// Met � jour toutes les lumi�res.

void CLight::LightUpdate()
{
	BOOL	bEnable;
	float	value;
	int		i;

	for ( i=0 ; i<m_lightUsed ; i++ )
	{
		if ( m_lightTable[i].bUsed == FALSE )  continue;

		bEnable = m_lightTable[i].bEnable;
		if ( m_lightTable[i].intensity.current == 0.0f )  bEnable = FALSE;

		if ( bEnable )
		{
			value = m_lightTable[i].colorRed.current * m_lightTable[i].intensity.current;
			m_lightTable[i].light.dcvDiffuse.r = value;

			value = m_lightTable[i].colorGreen.current * m_lightTable[i].intensity.current;
			m_lightTable[i].light.dcvDiffuse.g = value;

			value = m_lightTable[i].colorBlue.current * m_lightTable[i].intensity.current;
			m_lightTable[i].light.dcvDiffuse.b = value;

			m_pD3DDevice->SetLight(i, &m_lightTable[i].light);
			m_pD3DDevice->LightEnable(i, bEnable);
		}
		else
		{
			m_lightTable[i].light.dcvDiffuse.r = 0.0f;
			m_lightTable[i].light.dcvDiffuse.g = 0.0f;
			m_lightTable[i].light.dcvDiffuse.b = 0.0f;

			m_pD3DDevice->LightEnable(i, bEnable);
		}
	}
}

// Met � jour les lumi�res pour un type donn�.

void CLight::LightUpdate(D3DTypeObj type)
{
	BOOL	bEnable;
	int		i;

	for ( i=0 ; i<m_lightUsed ; i++ )
	{
		if ( m_lightTable[i].bUsed == FALSE )  continue;
		if ( m_lightTable[i].bEnable == FALSE )  continue;
		if ( m_lightTable[i].intensity.current == 0.0f )  continue;

		if ( m_lightTable[i].incluType != TYPENULL )
		{
			bEnable = (m_lightTable[i].incluType == type);
			m_pD3DDevice->LightEnable(i, bEnable);
		}

		if ( m_lightTable[i].excluType != TYPENULL )
		{
			bEnable = (m_lightTable[i].excluType != type);
			m_pD3DDevice->LightEnable(i, bEnable);
		}
	}
}


