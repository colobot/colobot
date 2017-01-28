// terrain.cpp

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
#include "modfile.h"
#include "particule.h"
#include "terrain.h"




// Constructeur du terrain.

CTerrain::CTerrain(CInstanceManager* iMan)
{
	m_iMan = iMan;
	m_iMan->AddInstance(CLASS_TERRAIN, this);

	m_engine = (CD3DEngine*)m_iMan->SearchInstance(CLASS_ENGINE);
	m_particule = (CParticule*)m_iMan->SearchInstance(CLASS_PARTICULE);

	m_resources = 0;
	m_shadows = 0;
	m_lockZone = 0;
	m_objRank = -1;
	m_nbTiles = 20;
	m_nbTiles2 = m_nbTiles/2;
	m_dimTile = 8.0f;
	m_texName1[0] = 0;
	m_texName2[0] = 0;
	m_idWall1[0] = 1;
	m_idWall1[1] = 8;
	m_idWall2[0] = 1;
	m_idWall2[1] = 9;
	m_idWall3[0] = 1;
	m_idWall3[1] = 10;
	m_idFlat[0] = 1;
	m_idFlat[1] = 0;
	m_idHole[0] = 1;
	m_idHole[1] = 11;
	m_idGround[0] = 1;
	m_idGround[1] = 11;
	m_depth = 1;
	m_slope = 0.0f;
	m_model = 0;
	m_modelRv = 0.0f;
	m_modelRh = 0.0f;
	m_baseRv = 0.0f;
	m_baseRh = 0.0f;
	m_bSmooth = TRUE;
	m_generation = 0;
	m_lockZoneDebug = FALSE;
	SignMarkFlush();
}

// Destructeur du terrain.

CTerrain::~CTerrain()
{
	free(m_resources);
	free(m_shadows);
	free(m_lockZone);
}


// Génère un nouveau terrain.

BOOL CTerrain::Generate(int nbTiles, float dimTile)
{
	m_nbTiles = nbTiles;
	m_nbTiles2 = nbTiles/2;
	m_dimTile = dimTile;

	return TRUE;
}


int CTerrain::RetNbTiles()
{
	return m_nbTiles;
}

float CTerrain::RetDimTile()
{
	return m_dimTile;
}

float CTerrain::RetDim()
{
	return m_nbTiles*m_dimTile;
}

// Retourne un numéro de génération du terrain. Chaque fois
// que le relief change (caisse incrustée dans un trou par
// exemple), ce numéro change.

int CTerrain::RetGeneration()
{
	return m_generation;
}


// Copie une série d'identificateurs.

void CopyID(int *dst, int *src, int def)
{
	int		i;

	dst[0] = *src++;
	for ( i=0 ; i<dst[0] ; i++ )
	{
		dst[i+1] = *src++;
	}

	if ( dst[0] == 0 )
	{
		dst[0] = 1;
		dst[1] = def;
	}
}

// Initialise les noms des textures à utiliser pour le terrain.

BOOL CTerrain::InitTextures(char *texName1, char *texName2,
							int *idWall1, int *idWall2, int *idWall3,
							int *idFlat, int *idHole, int *idGround,
							int depth,
							float slope, int model, float rv, float rh,
							float bv, float bh, BOOL bSmooth)
{
	int		i;

	strcpy(m_texName1, texName1);
	strcpy(m_texName2, texName2);

	CopyID(m_idWall1,  idWall1,   8);  // paroi 0..-8 si trou -8
	CopyID(m_idWall2,  idWall2,   9);  // paroi 0..-8 si trou infini
	CopyID(m_idWall3,  idWall3,  10);  // paroi -8..-16
	CopyID(m_idFlat,   idFlat,    0);  // sol plat 0
	CopyID(m_idHole,   idHole,   11);  // sol plat -8
	CopyID(m_idGround, idGround, 11);  // sol terrain 3D

	m_depth = depth;
	m_slope = slope;
	m_model = model;
	m_modelRv = rv;
	m_modelRh = rh;
	m_baseRv = bv;
	m_baseRh = bh;
	m_bSmooth = bSmooth;
	m_generation = 0;

	for ( i=0 ; i<8 ; i++ )
	{
		m_hardness[i] = 0.5f;
	}

	SignMarkFlush();

	return TRUE;
}


// Initialise les duretés du terrain.

void CTerrain::SetHardness(float *hardness)
{
	int		i;

	for ( i=0 ; i<8 ; i++ )
	{
		m_hardness[i] = hardness[i];
	}
}


// Initialise le terrain avec de l'eau partout et un bord TR_HOLE autour.

BOOL CTerrain::ResInit(BOOL bEmpty)
{
	TerrainRes	res;
	int			x, y;

	free(m_resources);
	m_resources = (char*)malloc(m_nbTiles*m_nbTiles*2);

	for ( y=0 ; y<m_nbTiles ; y++ )
	{
		for ( x=0 ; x<m_nbTiles ; x++ )
		{
			res = TR_SPACE;

			if ( !bEmpty )
			{
				if ( x <= 0 || x >= m_nbTiles-1 ||
					 y <= 0 || y >= m_nbTiles-1 )  res = TR_HOLE;
			}

			SetResource(x,y, res, TRUE);
		}
	}

	free(m_shadows);
	m_shadows = (char*)malloc(m_nbTiles*m_nbTiles);
	ZeroMemory(m_shadows, m_nbTiles*m_nbTiles);

	free(m_lockZone);
	m_lockZone = (char*)malloc(m_nbTiles*m_nbTiles*2);
	ZeroMemory(m_lockZone, m_nbTiles*m_nbTiles*2);

	return TRUE;
}


// Retourne la longueur pour GetResource/PutResource.

int CTerrain::LenResource()
{
	return m_nbTiles*m_nbTiles*2;
}

void CTerrain::GetResource(char *buffer)
{
	memcpy(buffer, m_resources, m_nbTiles*m_nbTiles*2);
}

void CTerrain::PutResource(char *buffer)
{
	memcpy(m_resources, buffer, m_nbTiles*m_nbTiles*2);
}

// Remet la copie des ressources.

void CTerrain::RestoreResourceCopy()
{
	memcpy(m_resources, m_resources+m_nbTiles*m_nbTiles, m_nbTiles*m_nbTiles);
}

// Retourne le type du sol.

TerrainRes CTerrain::RetResource(int x, int y, BOOL bCopy)
{
	if ( m_resources == 0 )  return TR_SOLID0;

	if ( x < 0 || x >= m_nbTiles ||
		 y < 0 || y >= m_nbTiles )  return TR_SPACE;

	if ( bCopy )
	{
		return (TerrainRes)m_resources[m_nbTiles*m_nbTiles+x+m_nbTiles*y];
	}
	else
	{
		return (TerrainRes)m_resources[x+m_nbTiles*y];
	}
}

// Retourne le type du sol.

TerrainRes CTerrain::RetResource(const D3DVECTOR &p, BOOL bCopy)
{
	D3DVECTOR	pos;
	int			x, y;

	pos = Grid(p, m_dimTile);
	x = (int)(pos.x/m_dimTile) + m_nbTiles2;
	y = (int)(pos.z/m_dimTile) + m_nbTiles2;

	return RetResource(x, y, bCopy);
}

// Change le type du sol.

BOOL CTerrain::SetResource(int x, int y, TerrainRes res, BOOL bCopy)
{
	if ( m_resources == 0 )  return FALSE;

	m_resources[x+m_nbTiles*y] = res;

	if ( bCopy )
	{
		m_resources[m_nbTiles*m_nbTiles+x+m_nbTiles*y] = res;
	}

	m_generation ++;

	return TRUE;
}

// Change le type du sol.

BOOL CTerrain::SetResource(const D3DVECTOR &p, TerrainRes res, BOOL bCopy)
{
	D3DVECTOR	pos;
	int			x, y;

	pos = Grid(p, m_dimTile);
	x = (int)(pos.x/m_dimTile) + m_nbTiles2;
	y = (int)(pos.z/m_dimTile) + m_nbTiles2;

	if ( m_resources == 0 )  return FALSE;

	if ( x < 0 || x >= m_nbTiles ||
		 y < 0 || y >= m_nbTiles )  return FALSE;

	m_resources[x+m_nbTiles*y] = res;

	if ( bCopy )
	{
		m_resources[m_nbTiles*m_nbTiles+x+m_nbTiles*y] = res;
	}

	m_generation ++;

	return TRUE;
}

// Indique s'il est possible de passer.

BOOL CTerrain::IsSolid(TerrainRes res)
{
	return ( res == TR_SOLID0 ||
			 res == TR_SOLID1 ||
			 res == TR_SOLID2 ||
			 res == TR_SOLID3 ||
			 res == TR_SOLID4 ||
			 res == TR_SOLID5 ||
			 res == TR_SOLID6 ||
			 res == TR_SOLID7 ||
			 res == TR_BOX    );
}

// Indique s'il est possible de passer.

BOOL CTerrain::IsSolid(int x, int y)
{
	return IsSolid(RetResource(x, y));
}

// Indique s'il est possible de passer.

BOOL CTerrain::IsSolid(const D3DVECTOR &p)
{
	return IsSolid(RetResource(p));
}


// Retourne la dureté du terrain.

float CTerrain::RetHardness(int x, int y)
{
	TerrainRes	res;
	
	res = RetResource(x, y);
	if ( res == TR_SOLID0 )  return m_hardness[0];
	if ( res == TR_SOLID1 )  return m_hardness[1];
	if ( res == TR_SOLID2 )  return m_hardness[2];
	if ( res == TR_SOLID3 )  return m_hardness[3];
	if ( res == TR_SOLID4 )  return m_hardness[4];
	if ( res == TR_SOLID5 )  return m_hardness[5];
	if ( res == TR_SOLID6 )  return m_hardness[6];
	if ( res == TR_SOLID7 )  return m_hardness[7];
	if ( res == TR_BOX    )  return 0.8f;
	if ( res == TR_LIFT   )  return 1.0f;

	return 0.5f;
}

// Retourne la dureté du terrain.

float CTerrain::RetHardness(const D3DVECTOR &p)
{
	D3DVECTOR	pos;
	int			x, y;

	pos = Grid(p, m_dimTile);
	x = (int)(pos.x/m_dimTile) + m_nbTiles2;
	y = (int)(pos.z/m_dimTile) + m_nbTiles2;

	return RetHardness(x, y);
}


// Initialise les ombres pour l'eau.
// Suppose un éclairage venant du sud-est (x<0, z>0).

BOOL CTerrain::InitShadows()
{
	TerrainRes	res;
	int			x, y, weight;

	if ( m_shadows == 0 )  return FALSE;

	for ( y=0 ; y<m_nbTiles ; y++ )
	{
		for ( x=0 ; x<m_nbTiles ; x++ )
		{
			weight = 0;

			res = RetResource(x+0, y+0);
			if ( res != TR_SPACE )  weight += 3;

			res = RetResource(x+1, y+0);
			if ( res != TR_SPACE )  weight += 3;

			res = RetResource(x+1, y-1);
			if ( res != TR_SPACE )  weight += 3;

			res = RetResource(x+0, y-1);
			if ( res != TR_SPACE )  weight += 3;

			res = RetResource(x-1, y-1);
			if ( res != TR_SPACE )  weight += 1;

			res = RetResource(x-1, y+0);
			if ( res != TR_SPACE )  weight += 1;

			res = RetResource(x-1, y+1);
			if ( res != TR_SPACE )  weight += 1;

			res = RetResource(x+0, y+1);
			if ( res != TR_SPACE )  weight += 1;

			res = RetResource(x+1, y+1);
			if ( res != TR_SPACE )  weight += 1;

			m_shadows[x+m_nbTiles*y] = weight;
		}
	}

	return TRUE;
}

// Retourne l'ombre pour l'eau.

float CTerrain::RetShadows(int x, int y)
{
	if ( m_shadows == 0 )  return 0.0f;

	if ( x < 0 || x >= m_nbTiles ||
		 y < 0 || y >= m_nbTiles )  return 0.0f;

	return m_shadows[x+m_nbTiles*y];
}

// Retourne l'ombre pour l'eau.

float CTerrain::RetShadows(const D3DVECTOR &p)
{
	D3DVECTOR	pos;
	int			x, y;

	pos = Grid(p, m_dimTile);
	x = (int)(pos.x/m_dimTile) + m_nbTiles2;
	y = (int)(pos.z/m_dimTile) + m_nbTiles2;

	return RetShadows(x, y);
}


// Retourne la longueur pour GetLockZone/PutLockZone.

int CTerrain::LenLockZone()
{
	return m_nbTiles*m_nbTiles*2;
}

void CTerrain::GetLockZone(char *buffer)
{
	memcpy(buffer, m_lockZone, m_nbTiles*m_nbTiles*2);
}

void CTerrain::PutLockZone(char *buffer)
{
	memcpy(m_lockZone, buffer, m_nbTiles*m_nbTiles*2);
}

// Bloque ou débloque une zone.

void CTerrain::SetLockZone(int x, int y, LockZone type, BOOL bInit)
{
	if ( m_lockZone == 0 )  return;

	if ( x < 0 || x >= m_nbTiles ||
		 y < 0 || y >= m_nbTiles )  return;

	if ( bInit )
	{
		m_lockZone[x+m_nbTiles*y] = type;
		m_lockZone[x+m_nbTiles*y+m_nbTiles*m_nbTiles] = type;
	}
	else
	{
		if ( type == LZ_FREE )
		{
			type = (LockZone)m_lockZone[x+m_nbTiles*y+m_nbTiles*m_nbTiles];
		}
		m_lockZone[x+m_nbTiles*y] = type;
	}

	if ( m_lockZoneDebug )
	{
		D3DVECTOR p;
		p.x = (x-m_nbTiles2)*m_dimTile;
		p.z = (y-m_nbTiles2)*m_dimTile;
		p.y = 0.2f;
		m_engine->ShowLockZone(p, (type!=LZ_FREE && type!=LZ_MAX1X));
	}
}

// Bloque ou débloque une zone.

void CTerrain::SetLockZone(const D3DVECTOR &pos, LockZone type, BOOL bInit)
{
	D3DVECTOR	p;
	int			x, y;

	if ( m_lockZone == 0 )  return;

	p = Grid(pos, m_dimTile);
	x = (int)(p.x/m_dimTile) + m_nbTiles2;
	y = (int)(p.z/m_dimTile) + m_nbTiles2;

	if ( x < 0 || x >= m_nbTiles ||
		 y < 0 || y >= m_nbTiles )  return;

	if ( bInit )
	{
		m_lockZone[x+m_nbTiles*y] = type;
		m_lockZone[x+m_nbTiles*y+m_nbTiles*m_nbTiles] = type;
	}
	else
	{
		if ( type == LZ_FREE )
		{
			type = (LockZone)m_lockZone[x+m_nbTiles*y+m_nbTiles*m_nbTiles];
		}
		m_lockZone[x+m_nbTiles*y] = type;
	}

	if ( m_lockZoneDebug )
	{
		p.y = 0.2f;
		m_engine->ShowLockZone(p, (type!=LZ_FREE && type!=LZ_MAX1X));
	}
}

// Indique si une zone est bloquée.

LockZone CTerrain::RetLockZone(int x, int y, BOOL bInit)
{
	if ( x < 0 || x >= m_nbTiles ||
		 y < 0 || y >= m_nbTiles )  return LZ_FREE;

	if ( bInit )
	{
		if ( m_lockZone[x+m_nbTiles*y+m_nbTiles*m_nbTiles] != LZ_FREE )
		{
			return (LockZone)m_lockZone[x+m_nbTiles*y+m_nbTiles*m_nbTiles];
		}
	}
	return (LockZone)m_lockZone[x+m_nbTiles*y];
}

// Indique si une zone est bloquée.

LockZone CTerrain::RetLockZone(const D3DVECTOR &pos, BOOL bInit)
{
	D3DVECTOR	p;
	int			x, y;

	if ( m_lockZone == 0 )  return LZ_FREE;

	p = Grid(pos, m_dimTile);
	x = (int)(p.x/m_dimTile) + m_nbTiles2;
	y = (int)(p.z/m_dimTile) + m_nbTiles2;

	if ( x < 0 || x >= m_nbTiles ||
		 y < 0 || y >= m_nbTiles )  return LZ_FREE;

	if ( bInit )
	{
		if ( m_lockZone[x+m_nbTiles*y+m_nbTiles*m_nbTiles] != LZ_FREE )
		{
			return (LockZone)m_lockZone[x+m_nbTiles*y+m_nbTiles*m_nbTiles];
		}
	}
	return (LockZone)m_lockZone[x+m_nbTiles*y];
}

// Indique si une zone est bloquée.

BOOL CTerrain::IsLockZone(int x, int y)
{
	int		lz;

	if ( x < 0 || x >= m_nbTiles ||
		 y < 0 || y >= m_nbTiles )  return TRUE;

	lz = m_lockZone[x+m_nbTiles*y];
	return ( lz != LZ_FREE  &&
			 lz != LZ_MAX1X );
}

// Indique si une zone est bloquée.

BOOL CTerrain::IsLockZone(const D3DVECTOR &pos)
{
	D3DVECTOR	p;
	int			x, y, lz;

	if ( m_lockZone == 0 )  return TRUE;

	p = Grid(pos, m_dimTile);
	x = (int)(p.x/m_dimTile) + m_nbTiles2;
	y = (int)(p.z/m_dimTile) + m_nbTiles2;

	if ( x < 0 || x >= m_nbTiles ||
		 y < 0 || y >= m_nbTiles )  return TRUE;

	lz = m_lockZone[x+m_nbTiles*y];
	return ( lz != LZ_FREE  &&
			 lz != LZ_MAX1X );
}

// Indique si une zone est bloquée par un objet carré.
// Un objet rond est considéré comme non bloquant.

BOOL CTerrain::IsLockZoneSquare(int x, int y)
{
	int		lz;

	if ( x < 0 || x >= m_nbTiles ||
		 y < 0 || y >= m_nbTiles )  return TRUE;

	lz = m_lockZone[x+m_nbTiles*y];
	return ( lz != LZ_FREE  &&
			 lz != LZ_MAX1X &&
			 lz != LZ_FIXo  &&
			 lz != LZ_MINE  &&
			 lz != LZ_FIOLE &&
			 lz != LZ_BLUPI &&
			 lz != LZ_BOXo  );
}

// Indique si une zone est bloquée par un objet carré.
// Un objet rond est considéré comme non bloquant.

BOOL CTerrain::IsLockZoneSquare(const D3DVECTOR &pos)
{
	D3DVECTOR	p;
	int			x, y, lz;

	if ( m_lockZone == 0 )  return TRUE;

	p = Grid(pos, m_dimTile);
	x = (int)(p.x/m_dimTile) + m_nbTiles2;
	y = (int)(p.z/m_dimTile) + m_nbTiles2;

	if ( x < 0 || x >= m_nbTiles ||
		 y < 0 || y >= m_nbTiles )  return TRUE;

	lz = m_lockZone[x+m_nbTiles*y];
	return ( lz != LZ_FREE  &&
			 lz != LZ_MAX1X &&
			 lz != LZ_FIXo  &&
			 lz != LZ_MINE  &&
			 lz != LZ_FIOLE &&
			 lz != LZ_BLUPI &&
			 lz != LZ_BOXo  );
}


// Efface toutes les marques.

void CTerrain::SignMarkFlush()
{
	int		i;

	for ( i=0 ; i<MAXSIGNMARK ; i++ )
	{
		m_signMark[i].bUsed = FALSE;
	}
}

// Crée une nouvelle marque.

BOOL CTerrain::SignMarkCreate(D3DVECTOR pos, float angle, ParticuleType type)
{
	int		i;

	for ( i=0 ; i<MAXSIGNMARK ; i++ )
	{
		if ( !m_signMark[i].bUsed )
		{
			m_signMark[i].bUsed = TRUE;
			m_signMark[i].type  = type;
			m_signMark[i].pos   = pos;
			m_signMark[i].angle = angle;
			m_signMark[i].channel = m_particule->CreateParticule(pos, D3DVECTOR(0.0f, 0.0f, 0.0f), FPOINT(3.0f, 3.0f), type);

			if ( m_signMark[i].channel == -1 )
			{
				m_signMark[i].bUsed = FALSE;
				return FALSE;
			}
			else
			{
				m_particule->SetAngle(m_signMark[i].channel, angle);
			}
			return TRUE;
		}
	}

	return FALSE;
}

// Supprime ume marque.

BOOL CTerrain::SignMarkDelete(D3DVECTOR pos)
{
	int		i;

	for ( i=0 ; i<MAXSIGNMARK ; i++ )
	{
		if ( m_signMark[i].bUsed )
		{
			if ( m_signMark[i].pos.x == pos.x &&
				 m_signMark[i].pos.z == pos.z )
			{
				m_particule->DeleteParticule(m_signMark[i].channel);
				m_signMark[i].bUsed = FALSE;
				return TRUE;
			}
		}
	}

	return FALSE;
}

// Indique si toutes les marques sont visibles.

void CTerrain::SignMarkShow(BOOL bShow)
{
	int		i;

	for ( i=0 ; i<MAXSIGNMARK ; i++ )
	{
		if ( m_signMark[i].bUsed )
		{
			m_particule->SetZoom(m_signMark[i].channel, bShow?1.0f:0.0f);
		}
	}
}

// Retourne une marque.

BOOL CTerrain::SignMarkGet(D3DVECTOR pos, float &angle, ParticuleType &type)
{
	int		i;

	for ( i=0 ; i<MAXSIGNMARK ; i++ )
	{
		if ( m_signMark[i].bUsed )
		{
			if ( m_signMark[i].pos.x == pos.x &&
				 m_signMark[i].pos.z == pos.z )
			{
				angle = m_signMark[i].angle;
				type  = m_signMark[i].type;
				return TRUE;
			}
		}
	}

	return FALSE;
}

// Retourne une marque.

BOOL CTerrain::SignMarkGet(int i, D3DVECTOR &pos, float &angle, ParticuleType &type)
{
	if ( i < 0 || i >= MAXSIGNMARK )  return FALSE;
	if ( !m_signMark[i].bUsed )  return FALSE;

	pos   = m_signMark[i].pos;
	angle = m_signMark[i].angle;
	type  = m_signMark[i].type;
	return TRUE;
}


// Gestion du mode de debug.

void CTerrain::SetDebugLockZone(BOOL bShow)
{
	m_lockZoneDebug = bShow;
}

BOOL CTerrain::RetDebugLockZone()
{
	return m_lockZoneDebug;
}


// Adapte un vertex pour le terrain.

void CTerrain::AdapteTileVertex(D3DVERTEX2 &v)
{
	D3DVECTOR	pos;

	pos.x = v.x+4.0f;
	pos.z = v.z+4.0f;
	if ( RetResource(pos) == TR_LIFT )  return;

	pos.x = v.x-4.0f;
	pos.z = v.z+4.0f;
	if ( RetResource(pos) == TR_LIFT )  return;

	pos.x = v.x+4.0f;
	pos.z = v.z-4.0f;
	if ( RetResource(pos) == TR_LIFT )  return;

	pos.x = v.x-4.0f;
	pos.z = v.z-4.0f;
	if ( RetResource(pos) == TR_LIFT )  return;

	PseudoRandom(v, m_baseRv, m_baseRh);
}

// Crée une tuile du terrain.

void CTerrain::CreateTile(int objRank, int x, int y, float level,
						  float tu, float tv)
{
	D3DVERTEX2		vertex[4], iv;
	D3DMATERIAL7	mat;
	D3DVECTOR		center;
	float			dp;

	ZeroMemory( &mat, sizeof(D3DMATERIAL7) );
	mat.diffuse.r = 1.0f;
	mat.diffuse.g = 1.0f;
	mat.diffuse.b = 1.0f;
	mat.ambient.r = 0.5f;
	mat.ambient.g = 0.5f;
	mat.ambient.b = 0.5f;

	center.x = m_dimTile*(x-m_nbTiles2);
	center.z = m_dimTile*(y-m_nbTiles2);
	center.y = level;

	vertex[0].x = center.x-m_dimTile/2.0f;
	vertex[0].z = center.z-m_dimTile/2.0f;
	vertex[0].y = center.y;

	vertex[1].x = center.x-m_dimTile/2.0f;
	vertex[1].z = center.z+m_dimTile/2.0f;
	vertex[1].y = center.y;

	vertex[2].x = center.x+m_dimTile/2.0f;
	vertex[2].z = center.z-m_dimTile/2.0f;
	vertex[2].y = center.y;

	vertex[3].x = center.x+m_dimTile/2.0f;
	vertex[3].z = center.z+m_dimTile/2.0f;
	vertex[3].y = center.y;

	if ( m_baseRv != 0.0f || m_baseRh != 0.0f )
	{
		AdapteTileVertex(vertex[0]);
		AdapteTileVertex(vertex[1]);
		AdapteTileVertex(vertex[2]);
		AdapteTileVertex(vertex[3]);
	}

	dp = 0.5f/256.0f;
	vertex[0].tu = tu+0.00f+dp;
	vertex[0].tv = tv+0.25f-dp;
	vertex[1].tu = tu+0.00f+dp;
	vertex[1].tv = tv+0.00f+dp;
	vertex[2].tu = tu+0.25f-dp;
	vertex[2].tv = tv+0.25f-dp;
	vertex[3].tu = tu+0.25f-dp;
	vertex[3].tv = tv+0.00f+dp;

	vertex[0].tu2 = ((float)(x+0)/m_nbTiles);
	vertex[0].tv2 = ((float)(y+0)/m_nbTiles);
	vertex[1].tu2 = ((float)(x+0)/m_nbTiles);
	vertex[1].tv2 = ((float)(y+1)/m_nbTiles);
	vertex[2].tu2 = ((float)(x+1)/m_nbTiles);
	vertex[2].tv2 = ((float)(y+0)/m_nbTiles);
	vertex[3].tu2 = ((float)(x+1)/m_nbTiles);
	vertex[3].tv2 = ((float)(y+1)/m_nbTiles);

	vertex[0].nx = 0.0f;
	vertex[0].ny = 1.0f;
	vertex[0].nz = 0.0f;
	vertex[1].nx = 0.0f;
	vertex[1].ny = 1.0f;
	vertex[1].nz = 0.0f;
	vertex[2].nx = 0.0f;
	vertex[2].ny = 1.0f;
	vertex[2].nz = 0.0f;
	vertex[3].nx = 0.0f;
	vertex[3].ny = 1.0f;
	vertex[3].nz = 0.0f;

	if ( level == -8.0f )
	{
		// Met des ombres sur le terrain de niveau -8, en supposant
		// un soleil placé au sud-est.
		vertex[0].ny /= RetShadowPound(x-0.5f, y-0.5f);
		vertex[1].ny /= RetShadowPound(x-0.5f, y+0.5f);
		vertex[2].ny /= RetShadowPound(x+0.5f, y-0.5f);
		vertex[3].ny /= RetShadowPound(x+0.5f, y+0.5f);
	}

	if ( m_texName2[0] == 0 )
	{
		m_engine->AddSurface(objRank, vertex, 4, mat, D3DSTATENORMAL,
							 m_texName1, m_texName2, 0.0f, 10000.0f, FALSE);
	}
	else
	{
		m_engine->AddSurface(objRank, vertex, 4, mat, D3DSTATEDUALb,
							 m_texName1, m_texName2, 0.0f, 10000.0f, FALSE);
	}
}

// Crée une paroi verticale du terrain.

void CTerrain::CreateWall(int objRank, int x, int y, float level,
						  float tu, float tv, const D3DVECTOR &n,
						  float dx0, float dz0, float dx2, float dz2)
{
	D3DVERTEX2		vertex[4];
	D3DMATERIAL7	mat;
	D3DVECTOR		center, nn;
	float			dp;

	ZeroMemory( &mat, sizeof(D3DMATERIAL7) );
	mat.diffuse.r = 1.0f;
	mat.diffuse.g = 1.0f;
	mat.diffuse.b = 1.0f;
	mat.ambient.r = 0.5f;
	mat.ambient.g = 0.5f;
	mat.ambient.b = 0.5f;

	center.x = m_dimTile*(x-m_nbTiles2);
	center.z = m_dimTile*(y-m_nbTiles2);
	center.y = level;

	if ( n.x > 0.0f )  // à droite ?
	{
		vertex[0].x = center.x+m_dimTile/2.0f+dx0;
		vertex[0].z = center.z-m_dimTile/2.0f+dz0;
		vertex[0].y = center.y-m_dimTile;

		vertex[1].x = center.x+m_dimTile/2.0f;
		vertex[1].z = center.z-m_dimTile/2.0f;
		vertex[1].y = center.y;

		vertex[2].x = center.x+m_dimTile/2.0f+dx2;
		vertex[2].z = center.z+m_dimTile/2.0f+dz2;
		vertex[2].y = center.y-m_dimTile;

		vertex[3].x = center.x+m_dimTile/2.0f;
		vertex[3].z = center.z+m_dimTile/2.0f;
		vertex[3].y = center.y;
	}
	else if ( n.x < 0.0f )  // à gauche ?
	{
		vertex[0].x = center.x-m_dimTile/2.0f+dx0;
		vertex[0].z = center.z+m_dimTile/2.0f+dz0;
		vertex[0].y = center.y-m_dimTile;

		vertex[1].x = center.x-m_dimTile/2.0f;
		vertex[1].z = center.z+m_dimTile/2.0f;
		vertex[1].y = center.y;

		vertex[2].x = center.x-m_dimTile/2.0f+dx2;
		vertex[2].z = center.z-m_dimTile/2.0f+dz2;
		vertex[2].y = center.y-m_dimTile;

		vertex[3].x = center.x-m_dimTile/2.0f;
		vertex[3].z = center.z-m_dimTile/2.0f;
		vertex[3].y = center.y;
	}
	else if ( n.z > 0.0f )  // en haut ?
	{
		vertex[0].x = center.x+m_dimTile/2.0f+dx0;
		vertex[0].z = center.z+m_dimTile/2.0f+dz0;
		vertex[0].y = center.y-m_dimTile;

		vertex[1].x = center.x+m_dimTile/2.0f;
		vertex[1].z = center.z+m_dimTile/2.0f;
		vertex[1].y = center.y;

		vertex[2].x = center.x-m_dimTile/2.0f+dx2;
		vertex[2].z = center.z+m_dimTile/2.0f+dz2;
		vertex[2].y = center.y-m_dimTile;

		vertex[3].x = center.x-m_dimTile/2.0f;
		vertex[3].z = center.z+m_dimTile/2.0f;
		vertex[3].y = center.y;
	}
	else	// en bas ?
	{
		vertex[0].x = center.x-m_dimTile/2.0f+dx0;
		vertex[0].z = center.z-m_dimTile/2.0f+dz0;
		vertex[0].y = center.y-m_dimTile;

		vertex[1].x = center.x-m_dimTile/2.0f;
		vertex[1].z = center.z-m_dimTile/2.0f;
		vertex[1].y = center.y;

		vertex[2].x = center.x+m_dimTile/2.0f+dx2;
		vertex[2].z = center.z-m_dimTile/2.0f+dz2;
		vertex[2].y = center.y-m_dimTile;

		vertex[3].x = center.x+m_dimTile/2.0f;
		vertex[3].z = center.z-m_dimTile/2.0f;
		vertex[3].y = center.y;
	}

	if ( m_baseRv != 0.0f || m_baseRh != 0.0f )
	{
		AdapteTileVertex(vertex[0]);
		AdapteTileVertex(vertex[1]);
		AdapteTileVertex(vertex[2]);
		AdapteTileVertex(vertex[3]);
	}

	dp = 0.5f/256.0f;
	vertex[0].tu = tu+0.00f+dp;
	vertex[0].tv = tv+0.25f-dp;
	vertex[1].tu = tu+0.00f+dp;
	vertex[1].tv = tv+0.00f+dp;
	vertex[2].tu = tu+0.25f-dp;
	vertex[2].tv = tv+0.25f-dp;
	vertex[3].tu = tu+0.25f-dp;
	vertex[3].tv = tv+0.00f+dp;

	vertex[0].tu2 = 0.0f;
	vertex[0].tv2 = 0.0f;
	vertex[1].tu2 = 0.0f;
	vertex[1].tv2 = 0.0f;
	vertex[2].tu2 = 0.0f;
	vertex[2].tv2 = 0.0f;
	vertex[3].tu2 = 0.0f;
	vertex[3].tv2 = 0.0f;

	nn = ComputeNormal(D3DVECTOR(vertex[0].x, vertex[0].y, vertex[0].z),
					   D3DVECTOR(vertex[2].x, vertex[2].y, vertex[2].z),
					   D3DVECTOR(vertex[1].x, vertex[1].y, vertex[1].z));

	vertex[0].nx = nn.x;
	vertex[0].ny = nn.y;
	vertex[0].nz = nn.z;
	vertex[1].nx = nn.x;
	vertex[1].ny = nn.y;
	vertex[1].nz = nn.z;
	vertex[2].nx = nn.x;
	vertex[2].ny = nn.y;
	vertex[2].nz = nn.z;
	vertex[3].nx = nn.x;
	vertex[3].ny = nn.y;
	vertex[3].nz = nn.z;

	m_engine->AddSurface(objRank, vertex, 4, mat, D3DSTATENORMAL,
						 m_texName1, "", 0.0f, 10000.0f, FALSE);
}

// Calcule les coordonnées u/v d'une texture d'après un identificateur
// compris entre 0 et 15.

void CTerrain::ComputeUV(int *id, float &tu, float &tv)
{
	int		i;

	i = (rand()%id[0])+1;
	tu = 0.25f*(id[i]%4);
	tv = 0.25f*(id[i]/4);
}

// Retourne le poids pour l'ombre, en supposant un soleil placé
// au sud-.est.

float CTerrain::RetShadowPound(float x, float y)
{
	int		flags;

	static int table[16] = {1,4,16,16,4,4,16,16,2,2,4,4,2,2,4,16};

	flags = 0;
	if ( IsSolid(RetResource((int)(x+0.5f), (int)(y+0.5f))) )  flags |= (1<<0);
	if ( IsSolid(RetResource((int)(x+0.5f), (int)(y-0.5f))) )  flags |= (1<<1);
	if ( IsSolid(RetResource((int)(x-0.5f), (int)(y-0.5f))) )  flags |= (1<<2);
	if ( IsSolid(RetResource((int)(x-0.5f), (int)(y+0.5f))) )  flags |= (1<<3);

	return (float)table[flags];
}

// Crée un joli bord sous forme d'un objet.

void CTerrain::CreateBorder(CModFile *pModFile, int objRank,
							float angle, int x, int y, char *type)
{
	D3DVECTOR	pos;
	char		text[50];
	int			rank;

	pos.x = (x-m_nbTiles2)*m_dimTile;
	pos.z = (y-m_nbTiles2)*m_dimTile;
	pos.y = -16.0f;

	sprintf(text, "objects\\terrain%d%s.mod", m_model, type);
	pModFile->ReadModel(text);
	pModFile->Rotate(angle);

	if ( m_bSmooth)  pModFile->TerrainNormalAdjust();

	// Met des ombres sur le terrain de niveau -8, en supposant
	// un soleil placé au sud-est.
	pModFile->TerrainNormalShadow(D3DVECTOR( 4.0f, 0.0f,  4.0f), RetShadowPound(x+0.5f, y+0.5f));
	pModFile->TerrainNormalShadow(D3DVECTOR( 4.0f, 0.0f, -4.0f), RetShadowPound(x+0.5f, y-0.5f));
	pModFile->TerrainNormalShadow(D3DVECTOR(-4.0f, 0.0f, -4.0f), RetShadowPound(x-0.5f, y-0.5f));
	pModFile->TerrainNormalShadow(D3DVECTOR(-4.0f, 0.0f,  4.0f), RetShadowPound(x-0.5f, y+0.5f));

	rank = (rand()%m_idGround[0])+1;
	rank = m_idGround[rank];
	pModFile->TerrainTexture(m_texName1, rank);

	pModFile->Translate(pos);
	pModFile->TerrainRandomize(m_modelRv, m_modelRh);
	pModFile->CreateEngineObject(objRank);
}

// Crée tous les objets du terrain dans le moteur 3D.

BOOL CTerrain::CreateObjects()
{
	CModFile*		pModFile;
	D3DMATRIX		transform;
	TerrainRes		res, dir[8];
	float			tu, tv;
	float			slope, dx0, dz0, dx2, dz2;
	int				objRank, x, y, id[2], i;

	InitShadows();

	objRank = m_engine->CreateObject();
	m_engine->SetObjectType(objRank, TYPETERRAIN);  // c'est un terrain
	m_objRank = objRank;

	slope = m_slope;

	for ( y=0 ; y<m_nbTiles ; y++ )
	{
		for ( x=0 ; x<m_nbTiles ; x++ )
		{
			res = RetResource(x, y);
			if ( IsSolid(res) )
			{
				if ( res == TR_SOLID0 )
				{
					ComputeUV(m_idFlat, tu, tv);
				}
				else
				{
					id[0] = 1;
					if ( res == TR_SOLID1 )  id[1] = 1;
					if ( res == TR_SOLID2 )  id[1] = 2;
					if ( res == TR_SOLID3 )  id[1] = 3;
					if ( res == TR_SOLID4 )  id[1] = 4;
					if ( res == TR_SOLID5 )  id[1] = 5;
					if ( res == TR_SOLID6 )  id[1] = 6;
					if ( res == TR_SOLID7 )  id[1] = 7;
					ComputeUV(id, tu, tv);
				}
				CreateTile(objRank, x, y, 0.0f, tu, tv);

				dir[0] = RetResource(x+0, y+1);  // haut
				dir[1] = RetResource(x+1, y+0);  // droite
				dir[2] = RetResource(x+0, y-1);  // bas
				dir[3] = RetResource(x-1, y+0);  // gauche

				if ( !IsSolid(dir[0]) )  // haut
				{
					if ( dir[0] == TR_SPACE ||
						 dir[0] == TR_LIFT  )
					{
						ComputeUV(m_idWall2, tu, tv);
						CreateWall(objRank, x, y, 0.0f, tu, tv, D3DVECTOR(0.0f, 0.0f, 1.0f), 0.0f, 0.0f, 0.0f, 0.0f);
					}
					else
					{
						ComputeUV(m_idWall1, tu, tv);
						CreateWall(objRank, x, y, 0.0f, tu, tv, D3DVECTOR(0.0f, 0.0f, 1.0f), 0.0f, 0.0f, 0.0f, 0.0f);
					}
				}

				if ( !IsSolid(dir[1]) )  // droite
				{
					if ( dir[1] == TR_SPACE ||
						 dir[1] == TR_LIFT  )
					{
						ComputeUV(m_idWall2, tu, tv);
						CreateWall(objRank, x, y, 0.0f, tu, tv, D3DVECTOR(1.0f, 0.0f, 0.0f), 0.0f, 0.0f, 0.0f, 0.0f);
					}
					else
					{
						ComputeUV(m_idWall1, tu, tv);
						CreateWall(objRank, x, y, 0.0f, tu, tv, D3DVECTOR(1.0f, 0.0f, 0.0f), 0.0f, 0.0f, 0.0f, 0.0f);
					}
				}

				if ( !IsSolid(dir[2]) )  // bas
				{
					if ( dir[2] == TR_SPACE ||
						 dir[2] == TR_LIFT  )
					{
						ComputeUV(m_idWall2, tu, tv);
						CreateWall(objRank, x, y, 0.0f, tu, tv, D3DVECTOR(0.0f, 0.0f, -1.0f), 0.0f, 0.0f, 0.0f, 0.0f);
					}
					else
					{
						ComputeUV(m_idWall1, tu, tv);
						CreateWall(objRank, x, y, 0.0f, tu, tv, D3DVECTOR(0.0f, 0.0f, -1.0f), 0.0f, 0.0f, 0.0f, 0.0f);
					}
				}

				if ( !IsSolid(dir[3]) )  // gauche
				{
					if ( dir[3] == TR_SPACE ||
						 dir[3] == TR_LIFT  )
					{
						ComputeUV(m_idWall2, tu, tv);
						CreateWall(objRank, x, y, 0.0f, tu, tv, D3DVECTOR(-1.0f, 0.0f, 0.0f), 0.0f, 0.0f, 0.0f, 0.0f);
					}
					else
					{
						ComputeUV(m_idWall1, tu, tv);
						CreateWall(objRank, x, y, 0.0f, tu, tv, D3DVECTOR(-1.0f, 0.0f, 0.0f), 0.0f, 0.0f, 0.0f, 0.0f);
					}
				}
			}

			if ( res == TR_HOLE && m_model == 0 )
			{
				ComputeUV(m_idHole, tu, tv);
				CreateTile(objRank, x, y, -8.0f, tu, tv);
			}

			if ( res != TR_SPACE && m_model == 0 )
			{
				dir[0] = RetResource(x+0, y+1);  // haut
				dir[1] = RetResource(x+1, y+0);  // droite
				dir[2] = RetResource(x+0, y-1);  // bas
				dir[3] = RetResource(x-1, y+0);  // gauche

				if ( dir[0] == TR_SPACE ||
					 dir[0] == TR_LIFT  )
				{
					dx0 = (dir[1]==TR_SPACE)? slope:0.0f;
					dz0 = (dir[0]==TR_SPACE)? slope:0.0f;
					dx2 = (dir[3]==TR_SPACE)?-slope:0.0f;
					dz2 = (dir[0]==TR_SPACE)? slope:0.0f;
					ComputeUV(m_idWall3, tu, tv);
					for ( i=0 ; i<m_depth ; i++ )
					{
						CreateWall(objRank, x, y, -8.0f*(i+1), tu, tv, D3DVECTOR(0.0f, 0.0f, 1.0f), dx0, dz0, dx2, dz2);
					}
				}

				if ( dir[1] == TR_SPACE ||
					 dir[1] == TR_LIFT  )
				{
					dx0 = (dir[1]==TR_SPACE)? slope:0.0f;
					dz0 = (dir[2]==TR_SPACE)?-slope:0.0f;
					dx2 = (dir[1]==TR_SPACE)? slope:0.0f;
					dz2 = (dir[0]==TR_SPACE)? slope:0.0f;
					ComputeUV(m_idWall3, tu, tv);
					for ( i=0 ; i<m_depth ; i++ )
					{
						CreateWall(objRank, x, y, -8.0f*(i+1), tu, tv, D3DVECTOR(1.0f, 0.0f, 0.0f), dx0, dz0, dx2, dz2);
					}
				}

				if ( dir[2] == TR_SPACE ||
					 dir[2] == TR_LIFT  )
				{
					dx0 = (dir[3]==TR_SPACE)?-slope:0.0f;
					dz0 = (dir[2]==TR_SPACE)?-slope:0.0f;
					dx2 = (dir[1]==TR_SPACE)? slope:0.0f;
					dz2 = (dir[2]==TR_SPACE)?-slope:0.0f;
					ComputeUV(m_idWall3, tu, tv);
					for ( i=0 ; i<m_depth ; i++ )
					{
						CreateWall(objRank, x, y, -8.0f*(i+1), tu, tv, D3DVECTOR(0.0f, 0.0f, -1.0f), dx0, dz0, dx2, dz2);
					}
				}

				if ( dir[3] == TR_SPACE ||
					 dir[3] == TR_LIFT  )
				{
					dx0 = (dir[3]==TR_SPACE)?-slope:0.0f;
					dz0 = (dir[0]==TR_SPACE)? slope:0.0f;
					dx2 = (dir[3]==TR_SPACE)?-slope:0.0f;
					dz2 = (dir[2]==TR_SPACE)?-slope:0.0f;
					ComputeUV(m_idWall3, tu, tv);
					for ( i=0 ; i<m_depth ; i++ )
					{
						CreateWall(objRank, x, y, -8.0f*(i+1), tu, tv, D3DVECTOR(-1.0f, 0.0f, 0.0f), dx0, dz0, dx2, dz2);
					}
				}
			}
		}
	}

	if ( m_model != 0 )
	{
		pModFile = new CModFile(m_iMan);

		for ( y=-1 ; y<m_nbTiles+1 ; y++ )
		{
			for ( x=-1 ; x<m_nbTiles+1 ; x++ )
			{
				res = RetResource(x, y);
				if ( res == TR_SPACE )
				{
					dir[0] = RetResource(x+0, y+1);  // up
					dir[1] = RetResource(x+1, y+1);  // 
					dir[2] = RetResource(x+1, y+0);  // right
					dir[3] = RetResource(x+1, y-1);  // 
					dir[4] = RetResource(x+0, y-1);  // down
					dir[5] = RetResource(x-1, y-1);  // 
					dir[6] = RetResource(x-1, y+0);  // left
					dir[7] = RetResource(x-1, y+1);  // 

					// Crée les points (.).
					if ( dir[3] != TR_SPACE &&
						 dir[2] == TR_SPACE &&
						 dir[4] == TR_SPACE )
					{
						CreateBorder(pModFile, objRank, PI*0.0f, x, y, "d");
					}

					if ( dir[1] != TR_SPACE &&
						 dir[0] == TR_SPACE &&
						 dir[2] == TR_SPACE )
					{
						CreateBorder(pModFile, objRank, PI*0.5f, x, y, "d");
					}

					if ( dir[7] != TR_SPACE &&
						 dir[0] == TR_SPACE &&
						 dir[6] == TR_SPACE )
					{
						CreateBorder(pModFile, objRank, PI*1.0f, x, y, "d");
					}

					if ( dir[5] != TR_SPACE &&
						 dir[4] == TR_SPACE &&
						 dir[6] == TR_SPACE )
					{
						CreateBorder(pModFile, objRank, PI*1.5f, x, y, "d");
					}

					// Crée les lignes (I).
					if ( dir[4] != TR_SPACE &&
						 dir[2] == TR_SPACE &&
						 dir[6] == TR_SPACE )
					{
						CreateBorder(pModFile, objRank, PI*0.0f, x, y, "i");
					}

					if ( dir[2] != TR_SPACE &&
						 dir[0] == TR_SPACE &&
						 dir[4] == TR_SPACE )
					{
						CreateBorder(pModFile, objRank, PI*0.5f, x, y, "i");
					}

					if ( dir[0] != TR_SPACE &&
						 dir[2] == TR_SPACE &&
						 dir[6] == TR_SPACE )
					{
						CreateBorder(pModFile, objRank, PI*1.0f, x, y, "i");
					}

					if ( dir[6] != TR_SPACE &&
						 dir[0] == TR_SPACE &&
						 dir[4] == TR_SPACE )
					{
						CreateBorder(pModFile, objRank, PI*1.5f, x, y, "i");
					}

					// Crée les coins (L).
					if ( dir[2] != TR_SPACE &&
						 dir[4] != TR_SPACE &&
						 dir[0] == TR_SPACE &&
						 dir[6] == TR_SPACE )
					{
						CreateBorder(pModFile, objRank, PI*0.0f, x, y, "l");
					}

					if ( dir[0] != TR_SPACE &&
						 dir[2] != TR_SPACE &&
						 dir[4] == TR_SPACE &&
						 dir[6] == TR_SPACE )
					{
						CreateBorder(pModFile, objRank, PI*0.5f, x, y, "l");
					}

					if ( dir[0] != TR_SPACE &&
						 dir[6] != TR_SPACE &&
						 dir[2] == TR_SPACE &&
						 dir[4] == TR_SPACE )
					{
						CreateBorder(pModFile, objRank, PI*1.0f, x, y, "l");
					}

					if ( dir[4] != TR_SPACE &&
						 dir[6] != TR_SPACE &&
						 dir[0] == TR_SPACE &&
						 dir[2] == TR_SPACE )
					{
						CreateBorder(pModFile, objRank, PI*1.5f, x, y, "l");
					}

					// Crée les coins (U).
					if ( dir[2] != TR_SPACE &&
						 dir[4] != TR_SPACE &&
						 dir[6] != TR_SPACE &&
						 dir[0] == TR_SPACE )
					{
						CreateBorder(pModFile, objRank, PI*0.0f, x, y, "u");
					}

					if ( dir[0] != TR_SPACE &&
						 dir[2] != TR_SPACE &&
						 dir[4] != TR_SPACE &&
						 dir[6] == TR_SPACE )
					{
						CreateBorder(pModFile, objRank, PI*0.5f, x, y, "u");
					}

					if ( dir[0] != TR_SPACE &&
						 dir[2] != TR_SPACE &&
						 dir[6] != TR_SPACE &&
						 dir[4] == TR_SPACE )
					{
						CreateBorder(pModFile, objRank, PI*1.0f, x, y, "u");
					}

					if ( dir[0] != TR_SPACE &&
						 dir[4] != TR_SPACE &&
						 dir[6] != TR_SPACE &&
						 dir[2] == TR_SPACE )
					{
						CreateBorder(pModFile, objRank, PI*1.5f, x, y, "u");
					}

					// Crée le trou (O).
					if ( dir[0] != TR_SPACE &&
						 dir[2] != TR_SPACE &&
						 dir[4] != TR_SPACE &&
						 dir[6] != TR_SPACE )
					{
						CreateBorder(pModFile, objRank, PI*0.0f, x, y, "o");
					}
				}
				if ( res != TR_SPACE )
				{
					CreateBorder(pModFile, objRank, PI*0.0f, x, y, "f");
				}
			}
		}

		delete pModFile;
	}

	D3DUtil_SetIdentityMatrix(transform);
	m_engine->SetObjectTransform(objRank, transform);

	return TRUE;
}

// Supprime tous les objets du terrain dans le moteur 3D.

BOOL CTerrain::DeleteObjects()
{
	m_engine->DeleteObject(m_objRank);
	m_objRank = -1;
	return TRUE;
}


// Retourne le numéro de l'objet terrain.

int CTerrain::RetObjRank()
{
	return m_objRank;
}

// Retourne le modèle de terrain.

int CTerrain::RetModel()
{
	return m_model;
}


// Gestion du vent.

void CTerrain::SetWind(D3DVECTOR speed)
{
	m_wind = speed;
}

D3DVECTOR CTerrain::RetWind()
{
	return m_wind;
}


// Donne la pente exacte du terrain à un endroit donné.

float CTerrain::RetFineSlope(const D3DVECTOR &pos)
{
	return 0.0f;
}

// Donne la pente approximative du terrain à un endroit donné.

float CTerrain::RetCoarseSlope(const D3DVECTOR &pos)
{
	return 0.0f;
}

// Donne le vecteur normal à la position p(x,-,z) du terrain.

BOOL CTerrain::GetNormal(D3DVECTOR &n, const D3DVECTOR &p)
{
	n = D3DVECTOR(0.0f, -1.0f, 0.0f);
	return TRUE;
}

// Retourne la hauteur du sol.

float CTerrain::RetFloorLevel(const D3DVECTOR &p)
{
	TerrainRes	res;

	res = RetResource(p);
	if ( IsSolid(res) )  return 0.0f;
	if ( res == TR_HOLE )  return -m_dimTile;
	return -m_dimTile*4.0f;
}

// Retourne la hauteur jusqu'au sol. Cette hauteur est positive
// lorsqu'on est au-dessus du sol.

float CTerrain::RetFloorHeight(const D3DVECTOR &p)
{
	TerrainRes	res;
	float		h;

	res = RetResource(p);
	h = -m_dimTile*2.0f;
	if ( IsSolid(res) )  h = 0.0f;
	if ( res == TR_HOLE )  h = -m_dimTile;
	return p.y-h;
}

// Modifie la coordonnée "y" du point "p" pour qu'il repose
// sur le sol du terrain.

BOOL CTerrain::MoveOnFloor(D3DVECTOR &p)
{
	TerrainRes	res;
	float		h;

	res = RetResource(p);
	h = -m_dimTile*2.0f;
	if ( IsSolid(res) )  h = 0.0f;
	if ( res == TR_HOLE )  h = -m_dimTile;
	p.y = h;
	return TRUE;
}

// Modifie une coordonnée pour qu'elle soit sur le terrain.
// Retourne FALSE si la coordonnée initiale était trop loin.

BOOL CTerrain::ValidPosition(D3DVECTOR &p, float marging)
{
	BOOL	bOK = TRUE;
	float	limit;

	limit = m_nbTiles*m_dimTile/2.0f - marging;

	if ( p.x < -limit )
	{
		p.x = -limit;
		bOK = FALSE;
	}

	if ( p.z < -limit )
	{
		p.z = -limit;
		bOK = FALSE;
	}

	if ( p.x > limit )
	{
		p.x = limit;
		bOK = FALSE;
	}

	if ( p.z > limit )
	{
		p.z = limit;
		bOK = FALSE;
	}

	return bOK;
}

// Ajuste une position pour qu'elle ne dépasse pas les limites.

void CTerrain::LimitPos(D3DVECTOR &pos)
{
	float		dim;

	dim = (m_nbTiles*m_dimTile)/2.0f;

	if ( pos.x < -dim )  pos.x = -dim;
	if ( pos.x >  dim )  pos.x =  dim;
	if ( pos.z < -dim )  pos.z = -dim;
	if ( pos.z >  dim )  pos.z =  dim;
}


// Détecte où est la souris sur le terrain, au niveau zéro.

BOOL CTerrain::GroundDetect(FPOINT mouse, D3DVECTOR &pos)
{
	D3DVERTEX2	vertex[3];
	D3DVECTOR	center;
	float		min, dist;
	int			x, y;

	center.y = 0.0f;
	vertex[0].y = 0.0f;
	vertex[1].y = 0.0f;
	vertex[2].y = 0.0f;

	min = 1000000.0f;
	for ( y=0 ; y<m_nbTiles ; y++ )
	{
		for ( x=0 ; x<m_nbTiles ; x++ )
		{
			if ( !IsSolid(x,y) )  continue;

			center.x = m_dimTile*(x-m_nbTiles2);
			center.z = m_dimTile*(y-m_nbTiles2);

			vertex[0].x = center.x-m_dimTile/2.0f;
			vertex[0].z = center.z-m_dimTile/2.0f;
			vertex[1].x = center.x-m_dimTile/2.0f;
			vertex[1].z = center.z+m_dimTile/2.0f;
			vertex[2].x = center.x+m_dimTile/2.0f;
			vertex[2].z = center.z-m_dimTile/2.0f;

			if ( m_engine->DetectTriangle(mouse, vertex, m_objRank, dist) &&
				 dist < min )
			{
				min = dist;
				pos = center;
			}

			vertex[0].x = center.x-m_dimTile/2.0f;
			vertex[0].z = center.z+m_dimTile/2.0f;
			vertex[1].x = center.x+m_dimTile/2.0f;
			vertex[1].z = center.z-m_dimTile/2.0f;
			vertex[2].x = center.x+m_dimTile/2.0f;
			vertex[2].z = center.z+m_dimTile/2.0f;

			if ( m_engine->DetectTriangle(mouse, vertex, m_objRank, dist) &&
				 dist < min )
			{
				min = dist;
				pos = center;
			}
		}
	}

	return (min < 1000000.0f);
}

