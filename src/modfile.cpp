// modfile.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "D3DEngine.h"
#include "D3DMath.h"
#include "language.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "math3d.h"
#include "modfile.h"



#define MAX_VERTICES	2000



// Constructeur de l'objet.

CModFile::CModFile(CInstanceManager* iMan)
{
	m_iMan = iMan;

	m_engine = (CD3DEngine*)m_iMan->SearchInstance(CLASS_ENGINE);

	m_triangleUsed = 0;
	m_triangleTable = (ModelTriangle*)malloc(sizeof(ModelTriangle)*MAX_VERTICES);
	ZeroMemory(m_triangleTable, sizeof(ModelTriangle)*MAX_VERTICES);
}

// Destructeur de l'objet.

CModFile::~CModFile()
{
	free(m_triangleTable);
}




// Crée un triangle dans la structure interne.

BOOL CModFile::CreateTriangle(D3DVECTOR p1, D3DVECTOR p2, D3DVECTOR p3,
							  float min, float max)
{
	D3DVECTOR	n;
	int			i;

	if ( p1.x == p2.x && p1.y == p2.y && p1.z == p2.z &&
		 p1.x == p3.x && p1.y == p3.y && p1.z == p3.z )  return TRUE;

	if ( m_triangleUsed >= MAX_VERTICES )
	{
		OutputDebugString("ERROR: CreateTriangle::Too many triangles\n");
		return FALSE;
	}

	i = m_triangleUsed++;

	ZeroMemory(&m_triangleTable[i], sizeof(ModelTriangle));

	m_triangleTable[i].bUsed = TRUE;
	m_triangleTable[i].bSelect = FALSE;

	n = ComputeNormal(p3, p2, p1);
	m_triangleTable[i].p1 = D3DVERTEX2( p1, n);
	m_triangleTable[i].p2 = D3DVERTEX2( p2, n);
	m_triangleTable[i].p3 = D3DVERTEX2( p3, n);

	m_triangleTable[i].material.diffuse.r = 1.0f;
	m_triangleTable[i].material.diffuse.g = 1.0f;
	m_triangleTable[i].material.diffuse.b = 1.0f;  // blanc
	m_triangleTable[i].material.ambient.r = 0.5f;
	m_triangleTable[i].material.ambient.g = 0.5f;
	m_triangleTable[i].material.ambient.b = 0.5f;

	m_triangleTable[i].min = min;
	m_triangleTable[i].max = max;

	return TRUE;
}

// Lit un fichier DXF.

BOOL CModFile::ReadDXF(char *filename, float min, float max)
{
	FILE*		file = NULL;
	char		line[100];
	int			command, rankSommet, nbSommet, nbFace;
	D3DVECTOR	table[MAX_VERTICES];
	BOOL		bWaitNbSommet;
	BOOL		bWaitNbFace;
	BOOL		bWaitSommetX;
	BOOL		bWaitSommetY;
	BOOL		bWaitSommetZ;
	BOOL		bWaitFaceX;
	BOOL		bWaitFaceY;
	BOOL		bWaitFaceZ;
	float		x,y,z;
	int			p1,p2,p3;

	file = fopen(filename, "r");
	if ( file == NULL )  return FALSE;

	m_triangleUsed = 0;

	rankSommet = 0;
	bWaitNbSommet = FALSE;
	bWaitNbFace   = FALSE;
	bWaitSommetX  = FALSE;
	bWaitSommetY  = FALSE;
	bWaitSommetZ  = FALSE;
	bWaitFaceX    = FALSE;
	bWaitFaceY    = FALSE;
	bWaitFaceZ    = FALSE;

	while ( fgets(line, 100, file) != NULL )
	{
		sscanf(line, "%d", &command);
		if ( fgets(line, 100, file) == NULL )  break;

		if ( command == 66 )
		{
			bWaitNbSommet = TRUE;
		}

		if ( command == 71 && bWaitNbSommet )
		{
			bWaitNbSommet = FALSE;
			sscanf(line, "%d", &nbSommet);
			if ( nbSommet > MAX_VERTICES )  nbSommet = MAX_VERTICES;
			rankSommet = 0;
			bWaitNbFace = TRUE;

//?			sprintf(s, "Waiting for %d sommets\n", nbSommet);
//?			OutputDebugString(s);
		}

		if ( command == 72 && bWaitNbFace )
		{
			bWaitNbFace = FALSE;
			sscanf(line, "%d", &nbFace);
			bWaitSommetX = TRUE;

//?			sprintf(s, "Waiting for %d faces\n", nbFace);
//?			OutputDebugString(s);
		}

		if ( command == 10 && bWaitSommetX )
		{
			bWaitSommetX = FALSE;
			sscanf(line, "%f", &x);
			bWaitSommetY = TRUE;
		}

		if ( command == 20 && bWaitSommetY )
		{
			bWaitSommetY = FALSE;
			sscanf(line, "%f", &y);
			bWaitSommetZ = TRUE;
		}

		if ( command == 30 && bWaitSommetZ )
		{
			bWaitSommetZ = FALSE;
			sscanf(line, "%f", &z);

			nbSommet --;
			if ( nbSommet >= 0 )
			{
				D3DVECTOR p(x,z,y);  // permutation de Y et Z !
				table[rankSommet++] = p;
				bWaitSommetX = TRUE;

//?				sprintf(s, "Sommet[%d]=%f;%f;%f\n", rankSommet, p.x,p.y,p.z);
//?				OutputDebugString(s);
			}
			else
			{
				bWaitFaceX = TRUE;
			}
		}

		if ( command == 71 && bWaitFaceX )
		{
			bWaitFaceX = FALSE;
			sscanf(line, "%d", &p1);
			if ( p1 < 0 )  p1 = -p1;
			bWaitFaceY = TRUE;
		}

		if ( command == 72 && bWaitFaceY )
		{
			bWaitFaceY = FALSE;
			sscanf(line, "%d", &p2);
			if ( p2 < 0 )  p2 = -p2;
			bWaitFaceZ = TRUE;
		}

		if ( command == 73 && bWaitFaceZ )
		{
			bWaitFaceZ = FALSE;
			sscanf(line, "%d", &p3);
			if ( p3 < 0 )  p3 = -p3;

			nbFace --;
			if ( nbFace >= 0 )
			{
				CreateTriangle( table[p3-1], table[p2-1], table[p1-1], min,max );
				bWaitFaceX = TRUE;

//?				sprintf(s, "Face=%d;%d;%d\n", p1,p2,p3);
//?				OutputDebugString(s);
			}
		}

	}

	fclose(file);
	return TRUE;
}



typedef struct
{
	int		rev;
	int		vers;
	int		total;
	int		reserve[10];
}
InfoMOD;



// Lit un fichier MOD.

BOOL CModFile::AddModel(char *filename, int first, BOOL bEdit, BOOL bMeta)
{
	FILE*		file;
	InfoMOD		info;
	float		limit[2];
	int			i, nb, err;
	char*		p;

	if ( m_engine->RetDebugMode() )
	{
		bMeta = FALSE;
	}

	if ( bMeta )
	{
		p = strchr(filename, '\\');
		if ( p == 0 )
		{
			err = g_metafile.Open("blupimania2.dat", filename, "b");
		}
		else
		{
			err = g_metafile.Open("blupimania2.dat", p+1, "b");
		}
		if ( err != 0 )  bMeta = FALSE;
	}
	if ( !bMeta )
	{
		file = fopen(filename, "rb");
		if ( file == NULL )  return FALSE;
	}

	if ( bMeta )
	{
		g_metafile.Read(&info, sizeof(InfoMOD));
	}
	else
	{
		fread(&info, sizeof(InfoMOD), 1, file);
	}
	nb = info.total;
	m_triangleUsed += nb;

	if ( bMeta )
	{
		g_metafile.Read(m_triangleTable+first, sizeof(ModelTriangle)*nb);
	}
	else
	{
		fread(m_triangleTable+first, sizeof(ModelTriangle), nb, file);
	}

#if 0
	if ( bEdit )
	{
		for ( i=first ; i<m_triangleUsed ; i++ )
		{
			if ( strcmp(m_triangleTable[i].texName, "bot2.tga") == 0 )
			{
				strcpy(m_triangleTable[i].texName, "blupi.tga");
			}
		}
	}
#endif

	if ( !bEdit )
	{
		limit[0] = m_engine->RetLimitLOD(0);  // frontière AB selon config
		limit[1] = m_engine->RetLimitLOD(1);  // frontière BC selon config

		// Frontières standard -> config.
		for ( i=first ; i<m_triangleUsed ; i++ )
		{
			if ( m_triangleTable[i].min == 0.0f &&
				 m_triangleTable[i].max == 100.0f )  // résolution A ?
			{
				m_triangleTable[i].max = limit[0];
			}
			else if ( m_triangleTable[i].min == 100.0f &&
					  m_triangleTable[i].max == 200.0f )  // résolution B ?
			{
				m_triangleTable[i].min = limit[0];
				m_triangleTable[i].max = limit[1];
			}
			else if ( m_triangleTable[i].min == 200.0f &&
					  m_triangleTable[i].max == 1000000.0f )  // résolution C ?
			{
				m_triangleTable[i].min = limit[1];
			}
		}
	}

	if ( bMeta )
	{
		g_metafile.Close();
	}
	else
	{
		fclose(file);
	}
	return TRUE;
}

// Lit un fichier MOD.

BOOL CModFile::ReadModel(char *filename, BOOL bEdit, BOOL bMeta)
{
	m_triangleUsed = 0;
	return AddModel(filename, 0, bEdit, bMeta);
}


// Ecrit un fichier MOD.

BOOL CModFile::WriteModel(char *filename)
{
	FILE*		file;
	InfoMOD		info;

	if ( m_triangleUsed == 0 )  return FALSE;

	file = fopen(filename, "wb");
	if ( file == NULL )  return FALSE;

	ZeroMemory(&info, sizeof(InfoMOD));
	info.rev   = 1;
	info.vers  = 2;
	info.total = m_triangleUsed;
	fwrite(&info, sizeof(InfoMOD), 1, file);

	fwrite(m_triangleTable, sizeof(ModelTriangle), m_triangleUsed, file);

	fclose(file);
	return TRUE;
}


// Crée l'objet dans le moteur 3D.

BOOL CModFile::CreateEngineObject(int objRank, int addState)
{
	char	texName1[20];
	char	texName2[20];
	int		texNum, i, state;

	for ( i=0 ; i<m_triangleUsed ; i++ )
	{
		if ( !m_triangleTable[i].bUsed )  continue;

		state = m_triangleTable[i].state;
		strcpy(texName1, m_triangleTable[i].texName);
		texName2[0] = 0;

		m_engine->ReplaceTexDo(texName1);

		if ( strcmp(texName1, "plant.tga") == 0 )
		{
			state |= D3DSTATEALPHA;
		}

		if ( m_triangleTable[i].texNum2 != 0 )
		{
			if ( m_triangleTable[i].texNum2 == 1 )
			{
				texNum = m_engine->RetSecondTexture();
			}
			else
			{
				texNum = m_triangleTable[i].texNum2;
			}

			if ( texNum >= 1 && texNum <= 10 )
			{
				state |= D3DSTATEDUALb;
			}
			if ( texNum >= 11 && texNum <= 20 )
			{
				state |= D3DSTATEDUALw;
			}
			sprintf(texName2, "dirty%.2d.tga", texNum);
		}

		m_engine->AddTriangle(objRank, &m_triangleTable[i].p1, 3,
							  m_triangleTable[i].material,
							  state+addState,
							  texName1, texName2,
							  m_triangleTable[i].min,
							  m_triangleTable[i].max, FALSE);
	}
	return TRUE;
}


// Effectue un miroir selon Z.

void CModFile::Mirror()
{
	D3DVERTEX2	t;
	int			i;

	for ( i=0 ; i<m_triangleUsed ; i++ )
	{
		t                     = m_triangleTable[i].p1;
		m_triangleTable[i].p1 = m_triangleTable[i].p2;
		m_triangleTable[i].p2 = t;

		m_triangleTable[i].p1.z = -m_triangleTable[i].p1.z;
		m_triangleTable[i].p2.z = -m_triangleTable[i].p2.z;
		m_triangleTable[i].p3.z = -m_triangleTable[i].p3.z;

		m_triangleTable[i].p1.nz = -m_triangleTable[i].p1.nz;
		m_triangleTable[i].p2.nz = -m_triangleTable[i].p2.nz;
		m_triangleTable[i].p3.nz = -m_triangleTable[i].p3.nz;
	}
}

// Effectue une rotation autour de l'axe Y.
// Le centre de la rotation est toujours (0;0;0).

void CModFile::Rotate(float angle)
{
	FPOINT		rot;
	int			i;

	if ( angle == 0.0f )  return;

	for ( i=0 ; i<m_triangleUsed ; i++ )
	{
		rot = RotatePoint(angle, FPOINT(m_triangleTable[i].p1.x,
										m_triangleTable[i].p1.z));
		m_triangleTable[i].p1.x = rot.x;
		m_triangleTable[i].p1.z = rot.y;

		rot = RotatePoint(angle, FPOINT(m_triangleTable[i].p2.x,
										m_triangleTable[i].p2.z));
		m_triangleTable[i].p2.x = rot.x;
		m_triangleTable[i].p2.z = rot.y;

		rot = RotatePoint(angle, FPOINT(m_triangleTable[i].p3.x,
										m_triangleTable[i].p3.z));
		m_triangleTable[i].p3.x = rot.x;
		m_triangleTable[i].p3.z = rot.y;

		rot = RotatePoint(angle, FPOINT(m_triangleTable[i].p1.nx,
										m_triangleTable[i].p1.nz));
		m_triangleTable[i].p1.nx = rot.x;
		m_triangleTable[i].p1.nz = rot.y;

		rot = RotatePoint(angle, FPOINT(m_triangleTable[i].p2.nx,
										m_triangleTable[i].p2.nz));
		m_triangleTable[i].p2.nx = rot.x;
		m_triangleTable[i].p2.nz = rot.y;

		rot = RotatePoint(angle, FPOINT(m_triangleTable[i].p3.nx,
										m_triangleTable[i].p3.nz));
		m_triangleTable[i].p3.nx = rot.x;
		m_triangleTable[i].p3.nz = rot.y;
	}
}

// Effectue une translation.

void CModFile::Translate(const D3DVECTOR &dist)
{
	int			i;

	for ( i=0 ; i<m_triangleUsed ; i++ )
	{
		m_triangleTable[i].p1.x += dist.x;
		m_triangleTable[i].p1.y += dist.y;
		m_triangleTable[i].p1.z += dist.z;

		m_triangleTable[i].p2.x += dist.x;
		m_triangleTable[i].p2.y += dist.y;
		m_triangleTable[i].p2.z += dist.z;

		m_triangleTable[i].p3.x += dist.x;
		m_triangleTable[i].p3.y += dist.y;
		m_triangleTable[i].p3.z += dist.z;
	}
}

// Ajuste les normales pour que le terrain soit lisse.

void CModFile::TerrainNormalAdjust()
{
	int			i;

	for ( i=0 ; i<m_triangleUsed ; i++ )
	{
		if ( Abs(m_triangleTable[i].p1.y-8.0f) < 0.1f )
		{
			m_triangleTable[i].p1.nx = 0.0f;
			m_triangleTable[i].p1.ny = 1.0f;
			m_triangleTable[i].p1.nz = 0.0f;
		}

		if ( Abs(m_triangleTable[i].p2.y-8.0f) < 0.1f )
		{
			m_triangleTable[i].p2.nx = 0.0f;
			m_triangleTable[i].p2.ny = 1.0f;
			m_triangleTable[i].p2.nz = 0.0f;
		}

		if ( Abs(m_triangleTable[i].p3.y-8.0f) < 0.1f )
		{
			m_triangleTable[i].p3.nx = 0.0f;
			m_triangleTable[i].p3.ny = 1.0f;
			m_triangleTable[i].p3.nz = 0.0f;
		}

		// Rend plus foncé les parties basses.
		m_triangleTable[i].p1.ny -= (8.0f-m_triangleTable[i].p1.y)/4.0f;
		m_triangleTable[i].p2.ny -= (8.0f-m_triangleTable[i].p2.y)/4.0f;
		m_triangleTable[i].p3.ny -= (8.0f-m_triangleTable[i].p3.y)/4.0f;
	}
}

// Ajuste les normales pour faire une ombre à un endroit donné.
// Plus le facteur est grand (>1) et plus l'ombre est prononcée.

void CModFile::TerrainNormalShadow(const D3DVECTOR &pos, float factor)
{
	int			i;

	if ( factor == 1.0f )  return;

	for ( i=0 ; i<m_triangleUsed ; i++ )
	{
		if ( Abs(m_triangleTable[i].p1.x-pos.x) < 1.0f &&
			 Abs(m_triangleTable[i].p1.z-pos.z) < 1.0f )
		{
			m_triangleTable[i].p1.ny /= factor;
		}

		if ( Abs(m_triangleTable[i].p2.x-pos.x) < 1.0f &&
			 Abs(m_triangleTable[i].p2.z-pos.z) < 1.0f )
		{
			m_triangleTable[i].p2.ny /= factor;
		}

		if ( Abs(m_triangleTable[i].p3.x-pos.x) < 1.0f &&
			 Abs(m_triangleTable[i].p3.z-pos.z) < 1.0f )
		{
			m_triangleTable[i].p3.ny /= factor;
		}
	}
}

// Modifie pseudo aléatoirement le terrain.

void CModFile::TerrainRandomize(float rv, float rh)
{
	int			i;

	for ( i=0 ; i<m_triangleUsed ; i++ )
	{
		PseudoRandom(m_triangleTable[i].p1, rv, rh);
		PseudoRandom(m_triangleTable[i].p2, rv, rh);
		PseudoRandom(m_triangleTable[i].p3, rv, rh);
	}
}

// Conversion d'une coordonnée X en mapping TU.

float ConvertX2TU(float x, int rank)
{
	x = (x+4.0f)/8.0f;  // 0..1
	x = (((rank%4)*64.0f+0.5f)/256.0f)+x*(63.0f/256.0f);
	return x;
}

// Conversion d'une coordonnée Y en mapping TV.

float ConvertZ2TV(float z, int rank)
{
	z = (z+4.0f)/8.0f;  // 0..1
	z = (((rank/4)*64.0f+0.5f)/256.0f)+z*(63.0f/256.0f);
	return z;
}

// Modifie la texture pour le terrain.

void CModFile::TerrainTexture(char *name, int rank)
{
	int			i;

	for ( i=0 ; i<m_triangleUsed ; i++ )
	{
		strcpy(m_triangleTable[i].texName, name);

		m_triangleTable[i].p1.tu = ConvertX2TU(m_triangleTable[i].p1.x, rank);
		m_triangleTable[i].p1.tv = ConvertZ2TV(m_triangleTable[i].p1.z, rank);

		m_triangleTable[i].p2.tu = ConvertX2TU(m_triangleTable[i].p2.x, rank);
		m_triangleTable[i].p2.tv = ConvertZ2TV(m_triangleTable[i].p2.z, rank);

		m_triangleTable[i].p3.tu = ConvertX2TU(m_triangleTable[i].p3.x, rank);
		m_triangleTable[i].p3.tv = ConvertZ2TV(m_triangleTable[i].p3.z, rank);
	}
}


// Retourne le pointeur à la liste de triangles.

void CModFile::SetTriangleUsed(int total)
{
	m_triangleUsed = total;
}

int CModFile::RetTriangleUsed()
{
	return m_triangleUsed;
}

int CModFile::RetTriangleMax()
{
	return MAX_VERTICES;
}

ModelTriangle* CModFile::RetTriangleList()
{
	return m_triangleTable;
}


// Retourne la hauteur en fonction d'une position (x;-;z);

float CModFile::RetHeight(D3DVECTOR pos)
{
	D3DVECTOR	p1, p2, p3;
	float		limit;
	int			i;

	limit = 5.0f;

	for ( i=0 ; i<m_triangleUsed ; i++ )
	{
		if ( !m_triangleTable[i].bUsed )  continue;

		if ( Abs(pos.x-m_triangleTable[i].p1.x) < limit &&
			 Abs(pos.z-m_triangleTable[i].p1.z) < limit )
		{
			return m_triangleTable[i].p1.y;
		}

		if ( Abs(pos.x-m_triangleTable[i].p2.x) < limit &&
			 Abs(pos.z-m_triangleTable[i].p2.z) < limit )
		{
			return m_triangleTable[i].p2.y;
		}

		if ( Abs(pos.x-m_triangleTable[i].p3.x) < limit &&
			 Abs(pos.z-m_triangleTable[i].p3.z) < limit )
		{
			return m_triangleTable[i].p3.y;
		}
	}

	return 0.0f;
}


