// water.cpp

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
#include "particule.h"
#include "terrain.h"
#include "object.h"
#include "sound.h"
#include "water.h"




// Converts a FLOAT to a DWORD for use in SetRenderState() calls.

inline DWORD F2DW( FLOAT f )
{
	return *((DWORD*)&f);
}



// Constructeur du terrain.

CWater::CWater(CInstanceManager* iMan, CD3DEngine* engine)
{
	m_iMan = iMan;
	m_iMan->AddInstance(CLASS_WATER, this);

	m_engine = engine;
	m_terrain = 0;
	m_particule = 0;
	m_sound = 0;

	m_type[0] = WATER_NULL;
	m_type[1] = WATER_NULL;
	m_level = -12.0f;
	m_bDraw = TRUE;
	m_meteo = METEO_NORM;
	m_color = 0xffffffff;
	m_filename[0] = 0;

	m_pick = 0.0f;
	m_tension = 0.0f;
	m_shadowForce   = 1.0f;
	m_farColorLight = 0x00ffffff;  // blanc
	m_farColorDark  = 0x00ffffff;  // blanc
	m_farStart      = 0.4f;
	m_farEnd        = 0.8f;
	m_bBold         = FALSE;

	WaterPickFlush();
}

// Destructeur du terrain.

CWater::~CWater()
{
}


BOOL CWater::EventProcess(const Event &event)
{
	if ( event.event == EVENT_FRAME )
	{
		return EventFrame(event);
	}

	if ( event.event == EVENT_KEYDOWN )
	{
#if 0
		if ( event.param == 'M' )
		{
			SetLevel(m_level+1.0f);
		}
		if ( event.param == 'D' )
		{
			SetLevel(m_level-1.0f);
		}
		if ( event.param == 'H' )
		{
			m_bDraw = !m_bDraw;
		}
		if ( event.param == 'C' )
		{
				 if ( m_color == 0xffffffff )  m_color = 0xcccccccc;
			else if ( m_color == 0xcccccccc )  m_color = 0x88888888;
			else if ( m_color == 0x88888888 )  m_color = 0x44444444;
			else if ( m_color == 0x44444444 )  m_color = 0x00000000;
			else if ( m_color == 0x00000000 )  m_color = 0xffffffff;
		}
		if ( event.param == 'Q' )
		{
			int		i;
			i = (m_color>>24);
			i += 0x44;
			i &= 0xff;
			i = (i<<24);
			m_color &= 0x00ffffff;
			m_color |= i;
		}
		if ( event.param == 'W' )
		{
			int		i;
			i = (m_color>>16);
			i += 0x44;
			i &= 0xff;
			i = (i<<16);
			m_color &= 0xff00ffff;
			m_color |= i;
		}
		if ( event.param == 'E' )
		{
			int		i;
			i = (m_color>>8);
			i += 0x44;
			i &= 0xff;
			i = (i<<8);
			m_color &= 0xffff00ff;
			m_color |= i;
		}
		if ( event.param == 'R' )
		{
			int		i;
			i = m_color;
			i += 0x44;
			i &= 0xff;
			m_color &= 0xffffff00;
			m_color |= i;
		}
#endif
	}
	return TRUE;
}

// Fait évoluer l'eau.

BOOL CWater::EventFrame(const Event &event)
{
	if ( m_engine->RetPause() )  return TRUE;

	m_time += event.rTime;

	if ( m_type[0] == WATER_NULL )  return TRUE;

	if ( m_meteo != METEO_NORM )
	{
		LavaFrame(event.rTime);
	}
	if ( m_pick != 0.0f )
	{
		WaterPickFrame(event.rTime);
	}
	return TRUE;
}

// Fait évoluer les jets de vapeur sur la lave.

void CWater::LavaFrame(float rTime)
{
	D3DVECTOR	eye, lookat, dir, perp, pos, speed;
	FPOINT		dim;
	float		distance, shift, level;
	int			i;

	if ( m_particule == 0 )
	{
		m_particule = (CParticule*)m_iMan->SearchInstance(CLASS_PARTICULE);
	}

	for ( i=0 ; i<MAXWATVAPOR ; i++ )
	{
		VaporFrame(i, rTime);
	}

	if ( m_time-m_lastLava >= 0.1f )
	{
		eye    = m_engine->RetEyePt();
		lookat = m_engine->RetLookatPt();

		if ( eye.y > 100.0f )  return;

		distance = Rand()*200.0f;
		shift = (Rand()-0.5f)*200.0f;

		dir = Normalize(lookat-eye);
		pos = eye + dir*distance;

		perp.x = -dir.z;
		perp.y =  dir.y;
		perp.z =  dir.x;
		pos = pos + perp*shift;

		if ( !m_terrain->ValidPosition(pos, 8.0f) )  return;

		if ( m_meteo != METEO_SNOW &&  // pas neige ?
			 m_meteo != METEO_RAIN )   // pas pluie ?
		{
			level = m_terrain->RetFloorLevel(pos);
			if ( level >= m_level )  return;
		}

		pos.y = m_level;

		if ( m_meteo == METEO_LAVA &&  // lave rouge ?
			 m_engine->RetSetup(ST_AMBIANCE) != 0 )
		{
			dim.x = 2.0f+2.0f*Rand();  // hauteur
			dim.y = 1.5f+1.5f*Rand();  // diamètre
			m_particule->CreateParticule(pos, D3DVECTOR(0.0f, 0.0f, 0.0f), dim, PARTIPLOUF0, 1.4f, 0.0f);

			level = Rand();
			if ( level < 0.8f )
			{
				if ( VaporCreate(PARTIFIRE, pos, 0.02f+Rand()*0.06f) )
				{
					m_lastLava = m_time;
				}
			}
			else if ( level < 0.9f )
			{
				if ( VaporCreate(PARTIFLAME, pos, 0.5f+Rand()*3.0f) )
				{
					m_lastLava = m_time;
				}
			}
			else
			{
				if ( VaporCreate(PARTIVAPOR, pos, 0.2f+Rand()*2.0f) )
				{
					m_lastLava = m_time;
				}
			}
		}

		if ( m_meteo == METEO_ORGA &&  // lave organique ?
			 m_engine->RetSetup(ST_AMBIANCE) != 0 )
		{
			dim.x = 1.5f+1.5f*Rand();  // hauteur
			dim.y = 0.1f+0.1f*Rand();  // diamètre
			m_particule->CreateParticule(pos, D3DVECTOR(0.0f, 0.0f, 0.0f), dim, PARTIPLOUF0, 1.4f, 0.0f);

			level = Rand();
			if ( level < 0.2f )
			{
				if ( VaporCreate(PARTIROOT, pos, 0.2f+Rand()*2.0f) )
				{
					m_lastLava = m_time;
				}
			}
		}

		if ( m_meteo == METEO_SNOW &&  // neige ?
			 m_engine->RetSetup(ST_METEO) != 0 )
		{
			pos.y = 40.0f;
			speed.x = 0.0f;
			speed.z = 0.0f;
			speed.y = -8.0f;
			dim.x = 0.5f;
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTISNOW, 6.0f);
		}

		if ( m_meteo == METEO_RAIN &&  // pluie ?
			 m_engine->RetSetup(ST_METEO) != 0 )
		{
			pos.y = 40.0f;
			speed.x = 0.0f;
			speed.z = 0.0f;
			speed.y = -30.0f;
			dim.x = 3.0f;
			dim.y = dim.x;
			m_particule->CreateParticule(pos, speed, dim, PARTIRAIN, 2.0f);
		}
	}
}

// Supprime tous les jets de vapeur.

void CWater::VaporFlush()
{
	int		i;

	for ( i=0 ; i<MAXWATVAPOR ; i++ )
	{
		m_vapor[i].bUsed = FALSE;
	}
}

// Crée un nouveau jet de vapeur.

BOOL CWater::VaporCreate(ParticuleType type, D3DVECTOR pos, float delay)
{
	int		i;

	for ( i=0 ; i<MAXWATVAPOR ; i++ )
	{
		if ( !m_vapor[i].bUsed )
		{
			m_vapor[i].bUsed = TRUE;
			m_vapor[i].type  = type;
			m_vapor[i].pos   = pos;
			m_vapor[i].delay = delay;
			m_vapor[i].time  = 0.0f;
			m_vapor[i].last  = 0.0f;

			if ( m_vapor[i].type == PARTIFIRE )
			{
				m_sound->Play(SOUND_BLUP, pos, 1.0f, 1.0f-Rand()*0.5f);
			}
			if ( m_vapor[i].type == PARTIFLAME )
			{
//?				m_sound->Play(SOUND_SWIM, pos, 1.0f, 1.0f-Rand()*0.5f);
			}
			if ( m_vapor[i].type == PARTIVAPOR )
			{
				m_sound->Play(SOUND_PSHHH, pos, 0.3f, 2.0f);
			}

			return TRUE;
		}
	}
	return FALSE;
}

// Fait évoluer un jet de vapeur,

void CWater::VaporFrame(int i, float rTime)
{
	D3DVECTOR	pos, speed;
	FPOINT		dim;
	float		progress;
	int			j;

	m_vapor[i].time += rTime;

	if ( m_sound == 0 )
	{
		m_sound = (CSound*)m_iMan->SearchInstance(CLASS_SOUND);
	}

	if ( m_vapor[i].time <= m_vapor[i].delay )
	{
		progress = m_vapor[i].time/m_vapor[i].delay;

		if ( m_time-m_vapor[i].last >= 0.02f )
		{
			m_vapor[i].last = m_time;

			if ( m_vapor[i].type == PARTIFIRE )
			{
				for ( j=0 ; j<10 ; j++ )
				{
					pos = m_vapor[i].pos;
					pos.x += (Rand()-0.5f)*2.0f;
					pos.z += (Rand()-0.5f)*2.0f;
					pos.y -= 1.0f;
					speed.x = (Rand()-0.5f)*6.0f;
					speed.z = (Rand()-0.5f)*6.0f;
					speed.y = 8.0f+Rand()*5.0f;
					dim.x = Rand()*1.5f+1.5f;
					dim.y = dim.x;
					m_particule->CreateParticule(pos, speed, dim, PARTILAVA, 2.0f, 10.0f);
				}
			}
			else if ( m_vapor[i].type == PARTIFLAME )
			{
				pos = m_vapor[i].pos;
				pos.x += (Rand()-0.5f)*8.0f;
				pos.z += (Rand()-0.5f)*8.0f;
				pos.y -= 2.0f;
				speed.x = (Rand()-0.5f)*2.0f;
				speed.z = (Rand()-0.5f)*2.0f;
				speed.y = 4.0f+Rand()*4.0f;
				dim.x = Rand()*2.0f+2.0f;
				dim.y = dim.x;
				m_particule->CreateParticule(pos, speed, dim, PARTIFLAME);
			}
			else if ( m_vapor[i].type == PARTIVAPOR )
			{
				pos = m_vapor[i].pos;
				pos.x += (Rand()-0.5f)*4.0f;
				pos.z += (Rand()-0.5f)*4.0f;
				pos.y -= 2.0f;
				speed.x = (Rand()-0.5f)*2.0f;
				speed.z = (Rand()-0.5f)*2.0f;
				speed.y = 8.0f+Rand()*8.0f;
				dim.x = Rand()*1.0f+1.0f;
				dim.y = dim.x;
				m_particule->CreateParticule(pos, speed, dim, PARTIVAPOR);
			}
			else if ( m_vapor[i].type == PARTIROOT )
			{
				pos = m_vapor[i].pos;
				pos.x += (Rand()-0.5f)*2.0f;
				pos.z += (Rand()-0.5f)*2.0f;
				pos.y -= 2.0f;
				speed.x = (Rand()-0.5f)*2.0f;
				speed.z = (Rand()-0.5f)*2.0f;
				speed.y = (6.0f+Rand()*6.0f)*(1.0f-progress);
				dim.x = (Rand()*0.5f+0.5f)*(1.0f-progress);
				dim.y = dim.x;
				m_particule->CreateParticule(pos, speed, dim, PARTIROOT, 2.0f);
			}
		}
	}
	else
	{
		m_vapor[i].bUsed = FALSE;
	}
}

// Remise à zéro de l'évolution des picks.

void CWater::WaterPickFlush()
{
	int		i;

	for ( i=0 ; i<MAXWATERPICK ; i++ )
	{
		m_waterPick[i].progress = 1.0f;
		m_waterPick[i].speed = 1.0f;
		m_waterPick[i].height = 0.0f;
		m_waterPick[i].radius = 0.0f;
		m_waterPick[i].pos = D3DVECTOR(0.0f, 0.0f, 0.0f);
	}
}

// Indique si un pick ne chevauche aucun autre.

BOOL CWater::WaterPickCheck(int rank)
{
	float	dist;
	int		i;

	for ( i=0 ; i<MAXWATERPICK ; i++ )
	{
		if ( i == rank )  continue;

		dist = Length2d(m_waterPick[i].pos, m_waterPick[rank].pos);
		if ( dist < m_waterPick[i].radius+m_waterPick[rank].radius )  return FALSE;
	}
	return TRUE;
}

// Evolution des picks.

void CWater::WaterPickFrame(float rTime)
{
	int		i, j;

	for ( i=0 ; i<MAXWATERPICK ; i++ )
	{
		if ( m_waterPick[i].progress >= 1.0f )
		{
			for ( j=0 ; j<50 ; j++ )
			{
				m_waterPick[i].progress = 0.0f;
				m_waterPick[i].height = m_pick;
				m_waterPick[i].radius = 12.0f+Rand()*20.0f;
				m_waterPick[i].speed = m_waterPick[i].radius/3.0f;
				m_waterPick[i].pos.x = (Rand()-0.5f)*m_nbTiles*m_dimTile;
				m_waterPick[i].pos.z = (Rand()-0.5f)*m_nbTiles*m_dimTile;
				m_waterPick[i].pos.y = 0.0f;
				if ( WaterPickCheck(i) )  break;
			}
			if ( j >= 50 )  m_waterPick[i].height = 0.0f;
		}

		m_waterPick[i].progress += rTime*(1.0f/m_waterPick[i].speed);
		if ( m_waterPick[i].progress >= 1.0f )  m_waterPick[i].progress = 1.0f;
	}
}


// Ajuste la position et la normale, pour imiter des reflets
// sur une étendue d'eau au repos.

void CWater::AdjustLevel(D3DVECTOR &pos, D3DVECTOR &norm,
						 FPOINT &uv1, FPOINT &uv2)
{
#if 0
	float		t1, t2;

	t1 = m_time*1.5f + pos.x*0.2f * pos.z*0.4f;
	pos.y += sinf(t1)*m_eddy.y;

	t1 = m_time*1.5f;
	uv1.x = (pos.x+10000.0f)/40.0f+sinf(t1)*m_eddy.x*0.02f;
	uv1.y = (pos.z+10000.0f)/40.0f-cosf(t1)*m_eddy.z*0.02f;
	uv2.x = (pos.x+10010.0f)/20.0f+cosf(-t1)*m_eddy.x*0.02f;
	uv2.y = (pos.z+10010.0f)/20.0f-sinf(-t1)*m_eddy.z*0.02f;

	t1 = m_time*0.50f + pos.x*2.1f + pos.z*1.1f;
	t2 = m_time*0.75f + pos.x*2.0f + pos.z*1.0f;
	norm = D3DVECTOR(sinf(t1)*m_glint, 1.0f, sinf(t2)*m_glint);
#else
	D3DVECTOR	p;
	float		t1, t2, weight, dist, progress, add;
	int			i;

	t1 = m_time*1.5f + pos.x*0.1f * pos.z*0.2f;
	pos.y += sinf(t1)*m_eddy.y;

	if ( m_pick != 0.0f )
	{
		for ( i=0 ; i<MAXWATERPICK ; i++ )
		{
			dist = Length2d(pos, m_waterPick[i].pos);
			if ( dist < m_waterPick[i].radius )
			{
				dist = cosf(dist/m_waterPick[i].radius*(PI*0.5f));
				progress = sinf(m_waterPick[i].progress*PI*4.0f);
				progress *= 1.0f-m_waterPick[i].progress;
				pos.y += progress*dist*m_waterPick[i].height*0.8f;

				dist = cosf(dist/m_waterPick[i].radius*(PI*1.5f));
				progress = sinf(m_waterPick[i].progress*PI*2.0f);
				progress *= 1.0f-m_waterPick[i].progress;
				pos.y += progress*dist*m_waterPick[i].height*0.6f;
			}
		}
	}

	if ( m_tension != 0.0f )
	{
		add = 0.0f;
		p = pos;
		p.x += 4.0f;
		if ( m_terrain->RetFloorLevel(p) >= m_level+m_eddy.y )  add = 1.0f;
		p = pos;
		p.x -= 4.0f;
		if ( m_terrain->RetFloorLevel(p) >= m_level+m_eddy.y )  add = 1.0f;
		p = pos;
		p.z += 4.0f;
		if ( m_terrain->RetFloorLevel(p) >= m_level+m_eddy.y )  add = 1.0f;
		p = pos;
		p.z -= 4.0f;
		if ( m_terrain->RetFloorLevel(p) >= m_level+m_eddy.y )  add = 1.0f;
		pos.y += add*m_tension;
	}

	t1 = m_time*1.5f;

	uv1.x = (pos.x+10000.0f)/40.0f;
	uv1.x += sinf(t1)*m_eddy.x*0.02f;
	uv1.x += sinf(t1+pos.x*m_vortex.x)*m_vortex.y;
	uv1.x += cosf(t1+pos.z*m_vortex.z)*m_vortex.y;

	uv1.y = (pos.z+10000.0f)/40.0f;
	uv1.y -= cosf(t1)*m_eddy.z*0.02f;
	uv1.y += cosf(t1+pos.x*m_vortex.x)*m_vortex.y;
	uv1.y += sinf(t1+pos.z*m_vortex.z)*m_vortex.y;

	uv2.x = (pos.x+10010.0f)/20.0f;
	uv2.x += cosf(-t1)*m_eddy.x*0.02f;
	uv2.x += sinf(-t1-pos.x*m_vortex.x)*m_vortex.y;
	uv2.x += cosf(-t1-pos.z*m_vortex.z)*m_vortex.y;

	uv2.y = (pos.z+10010.0f)/20.0f;
	uv2.y -= sinf(-t1)*m_eddy.z*0.02f;
	uv2.y += cosf(-t1-pos.x*m_vortex.x)*m_vortex.y;
	uv2.y += sinf(-t1-pos.z*m_vortex.z)*m_vortex.y;

	t1 = m_time*0.50f + pos.x*2.1f + pos.z*1.1f;
	t2 = m_time*0.75f + pos.x*2.0f + pos.z*1.0f;
	norm = D3DVECTOR(sinf(t1)*m_glint, 1.0f, sinf(t2)*m_glint);

	// Plus foncé si proche d'une rive. Suppose un éclairage venant
	// du sud-est (x<0, z>0).
	if ( m_shadowForce != 0.0f )
	{
#if 1
		weight = m_terrain->RetShadows(pos);
		if ( m_shadowForce > 0.0f )
		{
			norm *= powf(0.1f, weight*m_shadowForce);
		}
		else
		{
			norm *= (1.0f+weight)*(-m_shadowForce);
		}
#else
		weight = 0.0f;
		p = pos;
		p.x += 12.0f;
		if ( m_terrain->RetFloorLevel(p) >= m_level+m_eddy.y )  weight += 3.0f;
		p = pos;
		p.x -= 12.0f;
		if ( m_terrain->RetFloorLevel(p) >= m_level+m_eddy.y )  weight += 1.0f;
		p = pos;
		p.z += 12.0f;
		if ( m_terrain->RetFloorLevel(p) >= m_level+m_eddy.y )  weight += 3.0f;
		p = pos;
		p.z -= 12.0f;
		if ( m_terrain->RetFloorLevel(p) >= m_level+m_eddy.y )  weight += 1.0f;
		norm *= powf(0.1f, weight*m_shadowForce);
#endif
	}
#endif
}

// Dessine la surface du fond de l'eau.

void CWater::DrawBack()
{
	LPDIRECT3DDEVICE7 device;
	D3DVERTEX2		vertex[10];		// 8 triangles
	D3DMATERIAL7	material;
	D3DMATRIX		matrix;
	D3DVECTOR		eye, lookat, n, p1, p2, pos, dim;
	FPOINT			uv1, uv2;
	float			deep, dist;
	int				x, z;

	if ( !m_bDraw )  return;
	if ( m_type[0] == WATER_NULL )  return;
	if ( m_lineUsed == 0 )  return;

	eye = m_engine->RetEyePt();
	lookat = m_engine->RetLookatPt();

	ZeroMemory( &material, sizeof(D3DMATERIAL7) );
//?	material.diffuse.r = 0.0f;
//?	material.diffuse.g = 0.0f;
//?	material.diffuse.b = 0.0f;
//?	material.ambient.r = 0.0f;
//?	material.ambient.g = 0.0f;
//?	material.ambient.b = 0.0f;
	m_engine->SetMaterial(material);

	m_engine->SetTexture("", 0);

	p1.x = -m_nbTiles*m_dimTile/2.0f;
	p1.z = -m_nbTiles*m_dimTile/2.0f;
	p1.y = -16.0f;

	p2.x = m_nbTiles*m_dimTile/2.0f;
	p2.z = m_nbTiles*m_dimTile/2.0f;
	p2.y = -16.0f;

	dist = 0.0f;
	dist = Max(dist, Length(eye, D3DVECTOR(p1.x, p1.y, p1.z)));
	dist = Max(dist, Length(eye, D3DVECTOR(p1.x, p1.y, p2.z)));
	dist = Max(dist, Length(eye, D3DVECTOR(p2.x, p1.y, p1.z)));
	dist = Max(dist, Length(eye, D3DVECTOR(p2.x, p1.y, p2.z)));

	device = m_engine->RetD3DDevice();
	device->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	device->SetRenderState(D3DRENDERSTATE_LIGHTING, TRUE);
#if 1
	device->SetRenderState(D3DRENDERSTATE_FOGENABLE, TRUE);
	device->SetRenderState(D3DRENDERSTATE_FOGCOLOR, 0x00ffffff);
	device->SetRenderState(D3DRENDERSTATE_FOGVERTEXMODE, D3DFOG_LINEAR);
	device->SetRenderState(D3DRENDERSTATE_FOGSTART, F2DW(dist*0.25f));
	device->SetRenderState(D3DRENDERSTATE_FOGEND,   F2DW(dist));
#endif
	m_engine->SetState(D3DSTATENORMAL);

	deep = m_engine->RetDeepView(0);
	m_engine->SetDeepView(dist, 0);
	m_engine->SetFocus(m_engine->RetFocus());
	m_engine->UpdateMatProj();  // diminue la profondeur de vue

	D3DUtil_SetIdentityMatrix(matrix);
	device->SetTransform(D3DTRANSFORMSTATE_WORLD, &matrix);

	n.x = 0.0f;
	n.z = 0.0f;
	n.y = 0.001f;

	uv1.x = uv1.y = 0.0f;
	uv2.x = uv2.y = 0.0f;

	dim = (p2-p1)/4;
	pos.z = p1.z;
	for ( z=0 ; z<4 ; z++ )
	{
		pos.x = p1.x;
		vertex[0] = D3DVERTEX2(D3DVECTOR(pos.x, p1.y, pos.z), n, uv1.x,uv2.y);
		vertex[1] = D3DVERTEX2(D3DVECTOR(pos.x, p1.y, pos.z+dim.z), n, uv1.x,uv1.y);
		for ( x=0 ; x<4 ; x++ )
		{
			pos.x += dim.x;
			vertex[2+x*2] = D3DVERTEX2(D3DVECTOR(pos.x, p1.y, pos.z), n, uv1.x,uv2.y);
			vertex[3+x*2] = D3DVERTEX2(D3DVECTOR(pos.x, p1.y, pos.z+dim.z), n, uv1.x,uv1.y);
		}
		device->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 10, NULL);
		m_engine->AddStatisticTriangle(8);
		pos.z += dim.z;
	}

	m_engine->SetDeepView(deep, 0);
	m_engine->SetFocus(m_engine->RetFocus());
	m_engine->UpdateMatProj();  // remet profondeur de vue initiale
}

// Dessine la surface plane de l'eau.

void CWater::DrawSurf()
{
	LPDIRECT3DDEVICE7 device;
	D3DVERTEX2*		vertex;		// triangles
	D3DMATERIAL7	material;
	D3DMATRIX		matrix;
	D3DCOLOR		farColor;
	D3DCOLORVALUE	cvLight, cvDark, cv;
	D3DVECTOR		eye, lookat, n, pos, p, p1, p2, oBold;
	FPOINT			uv1, uv2;
	BOOL			bUnder;
	DWORD			flags;
	float			deep, size, sizez, radius, dist, dirV;
	float			angle, intensity, farStart, farEnd;
	int				rankview, i, j, u;

	if ( !m_bDraw )  return;
	if ( m_type[0] == WATER_NULL )  return;
	if ( m_lineUsed == 0 )  return;

	if ( m_type[0] == WATER_TT )
	{
		DrawBack();  // dessine le fond
	}

	vertex = (D3DVERTEX2*)malloc(sizeof(D3DVERTEX2)*(m_nbTiles+2)*2);

	eye = m_engine->RetEyePt();
	lookat = m_engine->RetLookatPt();

	rankview = m_engine->RetRankView();
	bUnder = ( rankview == 1);

	// Calcule la distance pour le brouillard qui fait apparaître
	// l'eau blanche au loin.
	p1.x = -m_nbTiles*m_dimTile/2.0f;
	p1.z = -m_nbTiles*m_dimTile/2.0f;
	p1.y = -8.0f;

	p2.x = m_nbTiles*m_dimTile/2.0f;
	p2.z = m_nbTiles*m_dimTile/2.0f;
	p2.y = -8.0f;

	dist = 0.0f;
	dist = Max(dist, Length(eye, D3DVECTOR(p1.x, p1.y, p1.z)));
	dist = Max(dist, Length(eye, D3DVECTOR(p1.x, p1.y, p2.z)));
	dist = Max(dist, Length(eye, D3DVECTOR(p2.x, p1.y, p1.z)));
	dist = Max(dist, Length(eye, D3DVECTOR(p2.x, p1.y, p2.z)));

	// Plus la caméra est verticale et moins le brouillard est présent.
	dirV = m_engine->RetEyeDirV()/(PI/2.0f);
	dirV = dirV*2.0f;
	if ( dirV < 1.0f )  dirV = 1.0f;
	dist *= dirV;

	// L'intensité est maximale (1.0) si on regarde contre le soleil,
	// c'est-à-dire au sud-est. On est face au soleil lorsque
	// m_eyeDirH = PI*0.75 (135 degrés).
	angle = m_engine->RetEyeDirH();
	angle = NormAngle(angle-PI*0.75f);
	intensity = 0.5f+sinf(PI*0.5f+angle)*0.5f;

	cvLight = RetColor(m_farColorLight);
	cvDark  = RetColor(m_farColorDark);
	cv.r = cvDark.r+(cvLight.r-cvDark.r)*intensity;
	cv.g = cvDark.g+(cvLight.g-cvDark.g)*intensity;
	cv.b = cvDark.b+(cvLight.b-cvDark.b)*intensity;
	farColor = RetColor(cv);

	farStart = m_farStart;
	farEnd   = m_farEnd;
//?	farStart = 1.0f-(1.0f-m_farStart)*(intensity*0.5f+0.5f);
//?	farEnd   = 1.0f-(1.0f-m_farEnd  )*(intensity*0.5f+0.5f);

	device = m_engine->RetD3DDevice();
//?	device->SetRenderState(D3DRENDERSTATE_AMBIENT, 0xffffffff);
	device->SetRenderState(D3DRENDERSTATE_LIGHTING, TRUE);
//?	device->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE);
	device->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	device->SetRenderState(D3DRENDERSTATE_FOGENABLE, TRUE);
	device->SetRenderState(D3DRENDERSTATE_FOGCOLOR, farColor);
	device->SetRenderState(D3DRENDERSTATE_FOGVERTEXMODE, D3DFOG_LINEAR);
	device->SetRenderState(D3DRENDERSTATE_FOGSTART, F2DW(dist*farStart));
	device->SetRenderState(D3DRENDERSTATE_FOGEND,   F2DW(dist*farEnd));

	D3DUtil_SetIdentityMatrix(matrix);
	device->SetTransform(D3DTRANSFORMSTATE_WORLD, &matrix);

	ZeroMemory( &material, sizeof(D3DMATERIAL7) );
	material.diffuse = m_diffuse;
	material.ambient = m_ambient;
	m_engine->SetMaterial(material);

	m_engine->SetTexture(m_filename, 0);
	m_engine->SetTexture(m_filename, 1);

	if ( m_type[rankview] == WATER_TT )
	{
		m_engine->SetState(D3DSTATETTb|D3DSTATEDUALw|D3DSTATEWRAP, m_color);
	}
	if ( m_type[rankview] == WATER_TO )
	{
		m_engine->SetState(D3DSTATENORMAL|D3DSTATEDUALw|D3DSTATEWRAP);
	}
	if ( m_type[rankview] == WATER_CT )
	{
		m_engine->SetState(D3DSTATETTb);
	}
	if ( m_type[rankview] == WATER_CO )
	{
		m_engine->SetState(D3DSTATENORMAL);
	}
	device->SetRenderState(D3DRENDERSTATE_FOGENABLE, TRUE);

	size = m_dimTile/2.0f;
	if ( bUnder )  sizez = -size;
	else           sizez =  size;

	// Dessine toutes les lignes.
	deep = m_engine->RetDeepView(0)*1.5f;

	if ( m_bBold )  oBold = D3DVECTOR(-4.0f, 0.0f, -4.0f);
	else            oBold = D3DVECTOR( 0.0f, 0.0f,  0.0f);

	for ( i=0 ; i<m_lineUsed ; i++ )
	{
		pos.y = m_level;
		pos.z = m_line[i].pz;
		pos.x = m_line[i].px1;
		pos += oBold;

		// Ligne visible ?
		p = pos;
		p.x += size*(m_line[i].len-1);
		radius = sqrtf(powf(size, 2.0f)+powf(size*m_line[i].len, 2.0f));
		if ( Length(p, eye) > deep+radius )  continue;
		device->ComputeSphereVisibility(&p, &radius, 1, 0, &flags);
		if ( flags & D3DSTATUS_CLIPINTERSECTIONALL )  continue;

		u = 0;
		p.x = pos.x-size;
		p.z = pos.z-sizez;
		p.y = pos.y;
		AdjustLevel(p, n, uv1, uv2);
		if ( bUnder )  n.y = -n.y;
		vertex[u++] = D3DVERTEX2(p, n, uv1.x,uv1.y, uv2.x,uv2.y);

		p.x = pos.x-size;
		p.z = pos.z+sizez;
		p.y = pos.y;
		AdjustLevel(p, n, uv1, uv2);
		if ( bUnder )  n.y = -n.y;
		vertex[u++] = D3DVERTEX2(p, n, uv1.x,uv1.y, uv2.x,uv2.y);

		for ( j=0 ; j<m_line[i].len ; j++ )
		{
			p.x = pos.x+size;
			p.z = pos.z-sizez;
			p.y = pos.y;
			AdjustLevel(p, n, uv1, uv2);
			if ( bUnder )  n.y = -n.y;
			vertex[u++] = D3DVERTEX2(p, n, uv1.x,uv1.y, uv2.x,uv2.y);

			p.x = pos.x+size;
			p.z = pos.z+sizez;
			p.y = pos.y;
			AdjustLevel(p, n, uv1, uv2);
			if ( bUnder )  n.y = -n.y;
			vertex[u++] = D3DVERTEX2(p, n, uv1.x,uv1.y, uv2.x,uv2.y);

			pos.x += size*2.0f;
		}

		device->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, u, NULL);
		m_engine->AddStatisticTriangle(u-2);
	}

	free(vertex);
}


// Indique s'il y a de l'eau à une position donnée.

BOOL CWater::RetWater(int x, int y)
{
	D3DVECTOR	pos;
	TerrainRes	res;
	float		offset;

	if ( m_nbTiles%2 == 0 )
	{
		offset = m_nbTiles*m_dimTile/2.0f;
	}
	else
	{
		offset = m_nbTiles*m_dimTile/2.0f - m_dimTile/2.0f;
	}

	pos.x = x*m_dimTile - offset;
	pos.z = y*m_dimTile - offset;
	pos.y = 0.0f;

	if ( m_bBold )
	{
		if ( x <= 0 || x >= m_nbTiles ||
			 y <= 0 || y >= m_nbTiles )  return FALSE;

		res = m_terrain->RetResource(pos, TRUE);
		if ( res == TR_SPACE )  return TRUE;
		pos.x -= 8.0f;
		res = m_terrain->RetResource(pos, TRUE);
		if ( res == TR_SPACE )  return TRUE;
		pos.z -= 8.0f;
		res = m_terrain->RetResource(pos, TRUE);
		if ( res == TR_SPACE )  return TRUE;
		pos.x += 8.0f;
		res = m_terrain->RetResource(pos, TRUE);
		if ( res == TR_SPACE )  return TRUE;
		return FALSE;
	}
	else
	{
		res = m_terrain->RetResource(pos, TRUE);
		if ( res == TR_LIFT )  return FALSE;
		if ( res == TR_SPACE )  return TRUE;
		return FALSE;
	}
}

// Met à jour les positions par-rapport au terrain.

BOOL CWater::CreateLine(int x, int y, int len)
{
	float	offset;

	m_line[m_lineUsed].x   = x;
	m_line[m_lineUsed].y   = y;
	m_line[m_lineUsed].len = len;

	if ( m_nbTiles%2 == 0 )
	{
		offset = m_nbTiles*m_dimTile/2.0f;
	}
	else
	{
		offset = m_nbTiles*m_dimTile/2.0f - m_dimTile/2.0f;
	}

	m_line[m_lineUsed].px1 = m_dimTile* m_line[m_lineUsed].x - offset;
	m_line[m_lineUsed].px2 = m_dimTile*(m_line[m_lineUsed].x+m_line[m_lineUsed].len) - offset;
	m_line[m_lineUsed].pz  = m_dimTile* m_line[m_lineUsed].y - offset;

	m_lineUsed ++;

	return ( m_lineUsed < MAXWATERLINE );
}

// Crée toutes les étendues d'eau.

BOOL CWater::Init(WaterType type1, WaterType type2, const char *filename,
				  D3DCOLORVALUE diffuse, D3DCOLORVALUE ambient,
				  float level, float glint, D3DVECTOR eddy, D3DVECTOR vortex,
				  float pick, float tension, float shadowForce,
				  D3DCOLOR farColorLight, D3DCOLOR farColorDark,
				  float farStart, float farEnd, Meteo meteo, BOOL bBold)
{
	m_type[0]  = type1;
	m_type[1]  = type2;
	m_diffuse  = diffuse;
	m_ambient  = ambient;
	m_level    = level;
	m_glint    = glint;
	m_eddy     = eddy;
	m_vortex   = vortex;
	m_pick     = pick;
	m_tension  = tension;
	m_shadowForce = shadowForce;
	m_farColorLight = farColorLight;
	m_farColorDark  = farColorDark;
	m_farStart = farStart;
	m_farEnd   = farEnd;
	m_meteo    = meteo;
	m_bBold    = bBold;
	m_time     = 0.0f;
	m_lastLava = 0.0f;
	strcpy(m_filename, filename);

	WaterPickFlush();

	return TRUE;
}

// Crée toutes les étendues d'eau.

BOOL CWater::Create()
{
	int		x, y, len, nbTiles;

	VaporFlush();

	if ( m_filename[0] != 0 )
	{
		m_engine->LoadTexture(m_filename, 0);
		m_engine->LoadTexture(m_filename, 1);
	}

	if ( m_terrain == 0 )
	{
		m_terrain = (CTerrain*)m_iMan->SearchInstance(CLASS_TERRAIN);
	}
	m_nbTiles = m_terrain->RetNbTiles();
	m_dimTile = m_terrain->RetDimTile();

	if ( m_type[0] == WATER_NULL )  return TRUE;

	m_lineUsed = 0;
	nbTiles = m_nbTiles;
	if ( m_bBold )  nbTiles ++;
	for ( y=0 ; y<nbTiles ; y++ )
	{
		len = 0;
		for ( x=0 ; x<nbTiles ; x++ )
		{
			if ( RetWater(x,y) )  // eau ici ?
			{
				len ++;
				if ( len >= 5 )
				{
					if ( !CreateLine(x-len+1, y, len) )  return FALSE;
					len = 0;
				}
			}
			else	// sec ?
			{
				if ( len != 0 )
				{
					if ( !CreateLine(x-len, y, len) )  return FALSE;
					len = 0;
				}
			}
		}
		if ( len != 0 )
		{
			if ( !CreateLine(x-len, y, len) )  return FALSE;
		}
	}
	return TRUE;
}

// Supprime toute l'eau.

void CWater::Flush()
{
	m_type[0] = WATER_NULL;
	m_type[1] = WATER_NULL;
	m_level = 0.0f;
	m_meteo = METEO_NORM;
}


// Modifie le niveau de l'eau.

BOOL CWater::SetLevel(float level)
{
	m_level = level;

	return Create();
}

// Retourne le niveau actuel de l'eau.

float CWater::RetLevel()
{
	return m_level;
}

// Retourne le niveau actuel de l'eau pour un objet donné.

float CWater::RetLevel(CObject* object)
{
	return m_level;
}


// Ajuste l'oeil de la caméra, pour ne pas être entre deux eaux.

void CWater::AdjustEye(D3DVECTOR &eye)
{
	if ( m_level == 0.0f )  return;

	if ( m_meteo != 0 )
	{
		if ( eye.y < m_level+2.0f )
		{
			eye.y = m_level+2.0f;  // jamais sous la lave
		}
	}
	else
	{
		if ( eye.y >= m_level-2.0f &&
			 eye.y <= m_level+2.0f )  // proche de la surface ?
		{
			eye.y = m_level+2.0f;  // paf, bien dessus
		}
	}
}


// Cherche une zone d'eau carrée.

BOOL CWater::SearchArea(D3DVECTOR &pos, float length)
{
	D3DVECTOR	p1, p2;
	float		dim;
	int			i;

	dim = m_terrain->RetDim();

	for ( i=0 ; i<100 ; i++ )
	{
		pos.x = (Rand()-0.5f)*dim;
		pos.z = (Rand()-0.5f)*dim;
		pos.y = 0.0f;
		pos = Grid(pos, 8.0f);

		p1.x = pos.x-length;
		p1.z = pos.z-length;
		p2.x = pos.x+length;
		p2.z = pos.z+length;

		if ( IsWaterRect(p1,p2) )  return TRUE;
	}

	return FALSE;
}

// Cherche si une zone rectangulaire est constituée d'eau.

BOOL CWater::IsWaterRect(const D3DVECTOR &p1, const D3DVECTOR &p2)
{
	D3DVECTOR	pp1, pp2, p;

	pp1.x = Min(p1.x, p2.x)-8.0f;
	pp1.z = Min(p1.z, p2.z)-8.0f;
	pp1.y = 0.0f;

	pp2.x = Max(p1.x, p2.x)+8.0f;
	pp2.z = Max(p1.z, p2.z)+8.0f;
	pp2.y = 0.0f;

	p.y = 0.0f;
	for ( p.z=pp1.z ; p.z<=pp2.z ; p.z+=8.0f )
	{
		for ( p.x=pp1.x ; p.x<=pp2.x ; p.x+=8.0f )
		{
			if ( m_terrain->RetResource(p) != TR_SPACE )  return FALSE;
		}
	}

	return TRUE;
}
