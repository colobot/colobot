// light.cpp

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

// Fait évoluer une progression.

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


// Supprime toutes les lumières.

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


// Crée une nouvelle lumière. Retourne son rang ou -1 en cas d'erreur.

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

// Supprime une lumière.

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


// Spécifie une lumière.

BOOL CLight::SetLight(int lightRank, const D3DLIGHT7 &light)
{
	if ( lightRank < 0 || lightRank >= D3DMAXLIGHT )  return FALSE;

	m_lightTable[lightRank].light = light;

	ProgInit(m_lightTable[lightRank].colorRed,   m_lightTable[lightRank].light.dcvDiffuse.r);
	ProgInit(m_lightTable[lightRank].colorGreen, m_lightTable[lightRank].light.dcvDiffuse.g);
	ProgInit(m_lightTable[lightRank].colorBlue,  m_lightTable[lightRank].light.dcvDiffuse.b);

	return TRUE;
}

// Donne les spécifications d'une lumière.

BOOL CLight::GetLight(int lightRank, D3DLIGHT7 &light)
{
	if ( lightRank < 0 || lightRank >= D3DMAXLIGHT )  return FALSE;

	light = m_lightTable[lightRank].light;
	return TRUE;
}


// Allume ou éteint une lumière.

BOOL CLight::LightEnable(int lightRank, BOOL bEnable)
{
	if ( lightRank < 0 || lightRank >= D3DMAXLIGHT )  return FALSE;

	m_lightTable[lightRank].bEnable = bEnable;
	return TRUE;
}


// Spécifie le type (TYPE*) des objets inclus par cette lumière.
// Cette lumière n'éclairera donc que ce type d'objets.

BOOL CLight::SetLightIncluType(int lightRank, D3DTypeObj type)
{
	if ( lightRank < 0 || lightRank >= D3DMAXLIGHT )  return FALSE;

	m_lightTable[lightRank].incluType = type;
	return TRUE;
}

// Spécifie le type (TYPE*) des objets exclus par cette lumière.
// Cette lumière n'éclairera donc jamais ce type d'objets.

BOOL CLight::SetLightExcluType(int lightRank, D3DTypeObj type)
{
	if ( lightRank < 0 || lightRank >= D3DMAXLIGHT )  return FALSE;

	m_lightTable[lightRank].excluType = type;
	return TRUE;
}


// Gestion de la position de la lunière.

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


// Gestion de la direction de la lumière.

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


// Spécifie la vitesse de changement.

BOOL CLight::SetLightIntensitySpeed(int lightRank, float speed)
{
	if ( lightRank < 0 || lightRank >= D3DMAXLIGHT )  return FALSE;

	m_lightTable[lightRank].intensity.speed = speed;
	return TRUE;
}

// Gestion de l'intensité de la lumière.

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


// Spécifie la vitesse de changement.

BOOL CLight::SetLightColorSpeed(int lightRank, float speed)
{
	if ( lightRank < 0 || lightRank >= D3DMAXLIGHT )  return FALSE;

	m_lightTable[lightRank].colorRed.speed   = speed;
	m_lightTable[lightRank].colorGreen.speed = speed;
	m_lightTable[lightRank].colorBlue.speed  = speed;
	return TRUE;
}

// Gestion de la couleur de la lumière.

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


// Adapte la couleur de toutes les lumières.

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



// Fait évoluer toutes les lumières.

void CLight::FrameLight(float rTime)
{
	int		i;

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
	}
}


// Met à jour toutes les lumières.

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

// Met à jour les lumières pour un type donné.

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


