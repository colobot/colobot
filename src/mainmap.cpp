// mainmap.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "D3DEngine.h"
#include "D3DMath.h"
#include "global.h"
#include "event.h"
#include "iman.h"
#include "interface.h"
#include "map.h"
#include "slider.h"
#include "scroll.h"
#include "mainmap.h"



#define ZOOM_MIN	1.0f
#define ZOOM_MAX	8.0f



// Constructeur de l'application carte.

CMainMap::CMainMap(CInstanceManager* iMan)
{
	m_iMan = iMan;
	m_iMan->AddInstance(CLASS_MAP, this);

	m_interface = (CInterface*)m_iMan->SearchInstance(CLASS_INTERFACE);
	m_event     = (CEvent*)m_iMan->SearchInstance(CLASS_EVENT);
	m_engine    = (CD3DEngine*)m_iMan->SearchInstance(CLASS_ENGINE);

	m_mapMode = 1;
}

// Destructeur de l'application robot.

CMainMap::~CMainMap()
{
}


// Crée la mini-carte et les boutons correspondants.

void CMainMap::CreateMap()
{
	FPOINT	pos, dim;

	dim.x = 10.0f/640.0f;
	dim.y = 10.0f/480.0f;
	pos.x = 10.0f/640.0f;
	pos.y = 10.0f/480.0f;
	m_interface->CreateMap   (pos, dim, 2, EVENT_OBJECT_MAP);
	m_interface->CreateSlider(pos, dim, 0, EVENT_OBJECT_MAPZOOM);

	DimMap();
}

// Choix des couleurs du sol et de l'eau pour la mini-carte.

void CMainMap::FloorColorMap(D3DCOLORVALUE floor, D3DCOLORVALUE water)
{
	CMap*		pm;

	pm = (CMap*)m_interface->SearchControl(EVENT_OBJECT_MAP);
	if ( pm != 0 )
	{
		pm->SetFloorColor(floor);
		pm->SetWaterColor(water);
	}
}

// Montre ou cache la mini-carte.

void CMainMap::ShowMap(BOOL bShow)
{
	CMap*		pm;
	CSlider*	ps;

	if ( bShow )
	{
		DimMap();
	}
	else
	{
		pm = (CMap*)m_interface->SearchControl(EVENT_OBJECT_MAP);
		if ( pm != 0 )
		{
			pm->ClearState(STATE_VISIBLE);
		}

		ps = (CSlider*)m_interface->SearchControl(EVENT_OBJECT_MAPZOOM);
		if ( ps != 0 )
		{
			ps->ClearState(STATE_VISIBLE);
		}
	}
}

// Dimensions de la mini-carte.

void CMainMap::DimMap()
{
	CMap*		pm;
	CSlider*	ps;
	FPOINT		pos, dim;
	float		value;

	pm = (CMap*)m_interface->SearchControl(EVENT_OBJECT_MAP);
	if ( pm == 0 )  return;

	pm->SetState(STATE_VISIBLE, (m_mapMode != 0));

	dim.x = 100.0f/640.0f;
	dim.y = 100.0f/480.0f;
	pos.x = 540.0f/640.0f;
	pos.y =   0.0f/480.0f;
	pm->SetPos(pos);
	pm->SetDim(dim);

	ps = (CSlider*)m_interface->SearchControl(EVENT_OBJECT_MAPZOOM);
	if ( ps != 0 )
	{
		ps->SetState(STATE_VISIBLE, (m_mapMode != 0));

		dim.x = SCROLL_WIDTH;
		dim.y =  66.0f/480.0f;
		pos.x = 523.0f/640.0f;
		pos.y =   3.0f/480.0f;
		ps->SetPos(pos);
		ps->SetDim(dim);

		value = pm->RetZoom();
		value = (value-ZOOM_MIN)/(ZOOM_MAX-ZOOM_MIN);
		value = powf(value, 0.5f);
		ps->SetVisibleValue(value);
		ps->SetArrowStep(0.2f);
	}
}

// Retourne le zoom actuel de la mini-carte.

float CMainMap::RetZoomMap()
{
	CMap*		pm;
	CSlider*	ps;

	pm = (CMap*)m_interface->SearchControl(EVENT_OBJECT_MAP);
	if ( pm == 0 )  return ZOOM_MIN;

	ps = (CSlider*)m_interface->SearchControl(EVENT_OBJECT_MAPZOOM);
	if ( ps == 0 )  return ZOOM_MIN;

	return pm->RetZoom();
}

// Zoom la mini-carte d'un facteur quelconque.

void CMainMap::ZoomMap(float zoom)
{
	CMap*		pm;
	CSlider*	ps;

	pm = (CMap*)m_interface->SearchControl(EVENT_OBJECT_MAP);
	if ( pm == 0 )  return;

	ps = (CSlider*)m_interface->SearchControl(EVENT_OBJECT_MAPZOOM);
	if ( ps == 0 )  return;

	if ( zoom < ZOOM_MIN )  zoom = ZOOM_MIN;
	if ( zoom > ZOOM_MAX )  zoom = ZOOM_MAX;
	pm->SetZoom(zoom);

	DimMap();
}

// Zoom la mini-carte selon le slider.

void CMainMap::ZoomMap()
{
	CMap*		pm;
	CSlider*	ps;
	float		zoom;

	pm = (CMap*)m_interface->SearchControl(EVENT_OBJECT_MAP);
	if ( pm == 0 )  return;

	ps = (CSlider*)m_interface->SearchControl(EVENT_OBJECT_MAPZOOM);
	if ( ps == 0 )  return;

	zoom = ps->RetVisibleValue();
	zoom = powf(zoom, 2.0f);
	zoom = ZOOM_MIN+zoom*(ZOOM_MAX-ZOOM_MIN);
	pm->SetZoom(zoom);

	DimMap();
}

// Active ou désactive la carte.

void CMainMap::MapEnable(BOOL bEnable)
{
	CMap*		pm;
	CSlider*	ps;

	pm = (CMap*)m_interface->SearchControl(EVENT_OBJECT_MAP);
	if ( pm != 0 )
	{
		pm->SetEnable(bEnable);
	}

	ps = (CSlider*)m_interface->SearchControl(EVENT_OBJECT_MAPZOOM);
	if ( ps != 0 )
	{
		ps->SetState(STATE_ENABLE, bEnable);
	}
}

// Met à jour la mini-carte suite à un changement du terrain.

void CMainMap::UpdateMap()
{
	CMap*	pm;

	pm = (CMap*)m_interface->SearchControl(EVENT_OBJECT_MAP);
	if ( pm != 0 )
	{
		pm->UpdateTerrain();
	}
}

// Indique si la mini-carte est visible.

BOOL CMainMap::RetShowMap()
{
	return ( m_mapMode != 0 );
}


// Détecte l'objet visé dans la mini-carte.

CObject* CMainMap::DetectMap(FPOINT pos, BOOL &bInMap)
{
	CMap*	pm;

	bInMap = FALSE;
	pm = (CMap*)m_interface->SearchControl(EVENT_OBJECT_MAP);
	if ( pm == 0 )  return 0;
	return pm->DetectObject(pos, bInMap);
}


// Indique l'objet survolé par la souris.

void CMainMap::SetHilite(CObject* pObj)
{
	CMap*	pm;

	pm = (CMap*)m_interface->SearchControl(EVENT_OBJECT_MAP);
	if ( pm != 0 )
	{
		pm->SetHilite(pObj);
	}
}


