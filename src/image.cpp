// image.cpp

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
#include "image.h"




// Constructeur de l'objet.

CImage::CImage(CInstanceManager* iMan) : CControl(iMan)
{
	CControl::CControl(iMan);

	m_filename[0] = 0;
}

// Destructeur de l'objet.

CImage::~CImage()
{
	if ( m_filename[0] != 0 )
	{
		m_engine->FreeTexture(m_filename);
	}

	CControl::~CControl();
}


// Crée un nouveau bouton.

BOOL CImage::Create(FPOINT pos, FPOINT dim, int icon, EventMsg eventMsg)
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


// Spécifie le nom de l'image à afficher.

void CImage::SetFilenameImage(char *name)
{
	if ( m_filename[0] != 0 )
	{
		m_engine->FreeTexture(m_filename);
	}

	strcpy(m_filename, name);
}

char* CImage::RetFilenameImage()
{
	return m_filename;
}


// Gestion d'un événement.

BOOL CImage::EventProcess(const Event &event)
{
	return TRUE;
}


// Dessine le bouton.

void CImage::Draw()
{
	FPOINT		uv1,uv2, corner, pos, dim;
	float		dp;

	if ( (m_state & STATE_VISIBLE) == 0 )  return;

	if ( m_state & STATE_SHADOW )
	{
		DrawShadow(m_pos, m_dim);
	}

	dp = 0.5f/256.0f;

	if ( m_icon == 0 )  // cadre en creux ?
	{
		m_engine->SetTexture("button1.tga");
		m_engine->SetState(D3DSTATENORMAL);
		uv1.x =  64.0f/256.0f;
		uv1.y =   0.0f/256.0f;  // u-v texture
		uv2.x =  96.0f/256.0f;
		uv2.y =  32.0f/256.0f;
		uv1.x += dp;
		uv1.y += dp;
		uv2.x -= dp;
		uv2.y -= dp;
		corner.x = 10.0f/640.0f;
		corner.y = 10.0f/480.0f;
		DrawIcon(m_pos, m_dim, uv1, uv2, corner, 8.0f/256.0f);
	}

	if ( m_filename[0] != 0 )  // affiche une image ?
	{
		m_engine->LoadTexture(m_filename);
		m_engine->SetTexture(m_filename);
		m_engine->SetState(D3DSTATENORMAL);
		pos = m_pos;
		dim = m_dim;
		pos.x += 4.0f/640.0f;
		pos.y += 4.0f/480.0f;
		dim.x -= 8.0f/640.0f;
		dim.y -= 8.0f/480.0f;
		uv1.x = 0.0f;
		uv1.y = 0.0f;
		uv2.x = 1.0f;
		uv2.y = 1.0f;
		DrawIcon(pos, dim, uv1, uv2);
	}
}


