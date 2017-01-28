// particule.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "D3DMath.h"
#include "D3DTextr.h"
#include "D3DEngine.h"
#include "iman.h"
#include "math3d.h"
#include "event.h"
#include "object.h"
#include "auto.h"
#include "robotmain.h"
#include "terrain.h"
#include "sound.h"
#include "water.h"
#include "particule.h"



#define FOG_HSUP	10.0f
#define FOG_HINF	100.0f




// Vérifie si un objet est destructible, mais pas un ennemi.

BOOL IsSoft(ObjectType type)
{
	return ( type == OBJECT_MARK     ||
			 type == OBJECT_METAL    ||
			 type == OBJECT_BARREL   ||
			 type == OBJECT_BARRELa  ||
			 type == OBJECT_ATOMIC   );
}

// Vérifie si un objet est un ennemi destructible.

BOOL IsAlien(ObjectType type)
{
	return ( type == OBJECT_BULLET   ||
			 type == OBJECT_MOBILEtg );
}



// Application constructor.

CParticule::CParticule(CInstanceManager *iMan, CD3DEngine* engine)
{
	m_iMan = iMan;
	m_iMan->AddInstance(CLASS_PARTICULE, this);

	m_pD3DDevice = 0;
	m_engine = engine;
	m_main = 0;
	m_terrain = 0;
	m_water = 0;
	m_sound = 0;
	m_uniqueStamp = 0;
	m_exploGunCounter = 0;
	m_lastTimeGunDel = 0.0f;
	m_absTime = 0.0f;
	m_sunbeamIntensity = 1.0f;
	m_bGoto = FALSE;

	FlushParticule();
}

// Application destructor. Free memory.

CParticule::~CParticule()
{
	m_iMan->DeleteInstance(CLASS_PARTICULE, this);
}


void CParticule::SetD3DDevice(LPDIRECT3DDEVICE7 device)
{
	m_pD3DDevice = device;
}


// Supprime toutes les particules.

void CParticule::FlushParticule()
{
	int		i, j;

	for ( i=0 ; i<MAXPARTICULE*MAXPARTITYPE ; i++ )
	{
		m_particule[i].bUsed = FALSE;
	}

	for ( i=0 ; i<MAXPARTITYPE ; i++ )
	{
		for ( j=0 ; j<SH_MAX ; j++ )
		{
			m_totalInterface[i][j] = 0;
		}
	}

	for ( i=0 ; i<MAXTRACK ; i++ )
	{
		m_track[i].bUsed = FALSE;
	}

	for ( i=0 ; i<MAXSUNBEAM ; i++ )
	{
		m_sunbeam[i].bUsed = FALSE;
	}

	m_wheelTraceTotal = 0;
	m_wheelTraceIndex = 0;

	for ( i=0 ; i<SH_MAX ; i++ )
	{
		m_bFrameUpdate[i] = TRUE;
	}

	m_fogTotal = 0;
	m_exploGunCounter = 0;
	m_sunbeamIntensity = 1.0f;
	m_bGoto = FALSE;
}

// Supprime toutes les particules d'une feuille.

void CParticule::FlushParticule(int sheet)
{
	int		i;

	for ( i=0 ; i<MAXPARTICULE*MAXPARTITYPE ; i++ )
	{
		if ( !m_particule[i].bUsed )  continue;
		if ( m_particule[i].sheet != sheet )  continue;

		m_particule[i].bUsed = FALSE;
	}

	for ( i=0 ; i<MAXPARTITYPE ; i++ )
	{
		m_totalInterface[i][sheet] = 0;
	}

	for ( i=0 ; i<MAXTRACK ; i++ )
	{
		m_track[i].bUsed = FALSE;
	}

	for ( i=0 ; i<MAXSUNBEAM ; i++ )
	{
		m_sunbeam[i].bUsed = FALSE;
	}

	if ( sheet == SH_WORLD )
	{
		m_wheelTraceTotal = 0;
		m_wheelTraceIndex = 0;
	}
}


// Construit le nom de fichier de l'effet.
//	effectNN.tga, avec NN = numéro

void NameParticule(char *buffer, int num)
{
	     if ( num == 1 )  strcpy(buffer, "effect00.tga");
	else if ( num == 2 )  strcpy(buffer, "effect01.tga");
	else if ( num == 3 )  strcpy(buffer, "effect02.tga");
	else if ( num == 4 )  strcpy(buffer, "effect03.tga");
	else                  strcpy(buffer, "xxx.tga");
}


// Créé une nouvelle particule.
// Retourne le canal de la particule crée ou -1 en cas d'erreur.

int CParticule::CreateParticule(D3DVECTOR pos, D3DVECTOR speed, FPOINT dim,
								ParticuleType type,
								float duration, float mass, int sheet)
{
	float	level;
	int		i, j, t;

	if ( m_main == 0 )
	{
		m_main = (CRobotMain*)m_iMan->SearchInstance(CLASS_MAIN);
	}
	if ( m_terrain == 0 )
	{
		m_terrain = (CTerrain*)m_iMan->SearchInstance(CLASS_TERRAIN);
	}
	if ( m_water == 0 )
	{
		m_water = (CWater*)m_iMan->SearchInstance(CLASS_WATER);
	}

	t = -1;
	if ( type == PARTIEXPLOT   ||
		 type == PARTIEXPLOA   ||
		 type == PARTIEXPLOO   ||
		 type == PARTIMOTOR    ||
		 type == PARTIBLITZ    ||
		 type == PARTIBLITZb   ||
		 type == PARTICRASH    ||
		 type == PARTIDUST1    ||
		 type == PARTIDUST2    ||
		 type == PARTIVAPOR    ||
		 type == PARTIGAS      ||
		 type == PARTIBASE     ||
		 type == PARTIFIRE     ||
		 type == PARTIFIREZ    ||
		 type == PARTIBLUE     ||
		 type == PARTIROOT     ||
		 type == PARTIRECOVER  ||
		 type == PARTIEJECT    ||
		 type == PARTISCRAPS   ||
		 type == PARTIGUN2     ||
		 type == PARTIGUN3     ||
		 type == PARTIGUN4     ||
		 type == PARTIORGANIC1 ||
		 type == PARTIORGANIC2 ||
		 type == PARTIFLAME    ||
		 type == PARTIBUBBLE   ||
		 type == PARTILAVA     ||
		 type == PARTISPHERE2  ||
		 type == PARTISPHERE4  ||
		 type == PARTISPHERE5  ||
		 type == PARTISPHERE6  ||
		 type == PARTIPLOUF0   ||
		 type == PARTITRACK1   ||
		 type == PARTITRACK2   ||
		 type == PARTITRACK3   ||
		 type == PARTITRACK4   ||
		 type == PARTITRACK5   ||
		 type == PARTITRACK6   ||
		 type == PARTITRACK7   ||
		 type == PARTITRACK8   ||
		 type == PARTITRACK9   ||
		 type == PARTITRACK10  ||
		 type == PARTITRACK11  ||
		 type == PARTITRACK12  ||
		 type == PARTITRACK13  ||
		 type == PARTILENS1    ||
		 type == PARTILENS2    ||
		 type == PARTILENS3    ||
		 type == PARTILENS4    ||
		 type == PARTIDROP     ||
		 type == PARTIWATER    ||
		 type == PARTIEXPLOG1  ||
		 type == PARTIEXPLOG2  ||
		 type == PARTILOCKZONE ||
		 type == PARTIGRIDb    ||
		 type == PARTIGRIDt    ||
		 type == PARTIGRIDw    ||
		 type == PARTIGRIDs    ||
		 type == PARTIGRIDe    ||
		 type == PARTISNOW     ||
		 type == PARTIRAIN     ||
		 type == PARTIFLIC     )
	{
		t = 1;  // effect00
	}
	if ( type == PARTIGLINT   ||
		 type == PARTIGLINTb  ||
		 type == PARTIGLINTr  ||
		 type == PARTISELY    ||
		 type == PARTISELR    ||
		 type == PARTIQUARTZ  ||
		 type == PARTIGUNDEL  ||
		 type == PARTICONTROL ||
		 type == PARTISPHERE1 ||
		 type == PARTICHOC    ||
		 type == PARTIPOWER   ||
		 type == PARTIGLU     ||
		 type == PARTIFOG4    ||
		 type == PARTIFOG5    ||
		 type == PARTIFOG6    ||
		 type == PARTIFOG7    ||
		 type == PARTIBIGO    ||
		 type == PARTIBIGT    )
	{
		t = 2;  // effect01
	}
	if ( type == PARTIGUN1       ||
		 type == PARTISPHERE0    ||
		 type == PARTISPHERE3    ||
		 type == PARTIFOG0       ||
		 type == PARTIFOG1       ||
		 type == PARTIFOG2       ||
		 type == PARTIFOG3       ||
		 type == PARTITERSPTIRE  ||
		 type == PARTISIGNMARK0  ||
		 type == PARTISIGNMARK1  ||
		 type == PARTISIGNMARK2  ||
		 type == PARTISIGNMARK3  ||
		 type == PARTISIGNMARK4  ||
		 type == PARTISIGNMARK5  ||
		 type == PARTISIGNMARK6  ||
		 type == PARTISIGNMARK7  ||
		 type == PARTISIGNMARK8  ||
		 type == PARTISIGNMARK9  ||
		 type == PARTISIGNMARK10 ||
		 type == PARTISIGNMARK11 )
	{
		t = 3;  // effect02
	}
	if ( type == PARTISMOKE1    ||
		 type == PARTISMOKE2    ||
		 type == PARTISMOKE3    ||
		 type == PARTIWHEEL     ||
		 type == PARTITRACE1    ||
		 type == PARTITRACE2    ||
		 type == PARTITRACE3    ||
		 type == PARTITRACE4    ||
		 type == PARTITRACE5    ||
		 type == PARTITRACE6    ||
		 type == PARTITRACE7    ||
		 type == PARTITRACE8    )
	{
		t = 4;  // effect03
	}
	if ( t >= MAXPARTITYPE )  return -1;
	if ( t == -1 )  return -1;

	for ( j=0 ; j<MAXPARTICULE ; j++ )
	{
		i = MAXPARTICULE*t+j;

		if ( !m_particule[i].bUsed )
		{
			ZeroMemory(&m_particule[i], sizeof(Particule));
			m_particule[i].bUsed     = TRUE;
			m_particule[i].bRay      = FALSE;
			m_particule[i].uniqueStamp = m_uniqueStamp++;
			m_particule[i].sheet     = sheet;
			m_particule[i].mass      = mass;
			m_particule[i].duration  = duration;
			m_particule[i].pos       = pos;
			m_particule[i].goal      = pos;
			m_particule[i].speed     = speed;
			m_particule[i].dim       = dim;
			m_particule[i].zoom      = 1.0f;
			m_particule[i].angle     = 0.0f;
			m_particule[i].intensity = 1.0f;
			m_particule[i].type      = type;
			m_particule[i].phase     = PARPHSTART;
			m_particule[i].texSup.x  = 0.0f;
			m_particule[i].texSup.y  = 0.0f;
			m_particule[i].texInf.x  = 0.0f;
			m_particule[i].texInf.y  = 0.0f;
			m_particule[i].time      = 0.0f;
			m_particule[i].objFather = 0;
			m_particule[i].trackRank = -1;

			m_totalInterface[t][sheet] ++;

			if ( type == PARTIFLIC )
			{
				m_particule[i].zoom = 0.0f;
			}

			if ( type == PARTIEXPLOT ||
				 type == PARTIEXPLOA ||
				 type == PARTIEXPLOO )
			{
				m_particule[i].angle = Rand()*PI*2.0f;
			}

			if ( type >= PARTIFOG0 &&
				 type <= PARTIFOG9 )
			{
				if ( m_fogTotal < MAXPARTIFOG )
				m_fog[m_fogTotal++] = i;
			}

			if ( type == PARTIDUST1 ||
				 type == PARTIDUST2 )
			{
				m_particule[i].angle = Rand()*PI*2.0f;
			}

			if ( type == PARTISNOW ||
				 type == PARTIRAIN )
			{
				if ( m_terrain->IsLockZone(pos) )
				{
					level = NAN;
				}
				else
				{
					level = m_terrain->RetFloorLevel(pos);
					if ( level < m_water->RetLevel() )
					{
						level = m_water->RetLevel();
					}
				}
				m_particule[i].ground = level;
			}

			return i | ((m_particule[i].uniqueStamp&0xffff)<<16);
		}
	}

	return -1;
}

// Créé une nouvelle particule triangulaire (débris).
// Retourne le canal de la particule crée ou -1 en cas d'erreur.

int CParticule::CreateFrag(D3DVECTOR pos, D3DVECTOR speed,
						   D3DTriangle *triangle,
						   ParticuleType type,
						   float duration, float mass, int sheet)
{
	int			i, j, t;

	t = 0;
	for ( j=0 ; j<MAXPARTICULE ; j++ )
	{
		i = MAXPARTICULE*t+j;

		if ( !m_particule[i].bUsed )
		{
			ZeroMemory(&m_particule[i], sizeof(Particule));
			m_particule[i].bUsed     = TRUE;
			m_particule[i].bRay      = FALSE;
			m_particule[i].uniqueStamp = m_uniqueStamp++;
			m_particule[i].sheet     = sheet;
			m_particule[i].mass      = mass;
			m_particule[i].duration  = duration;
			m_particule[i].pos       = pos;
			m_particule[i].goal      = pos;
			m_particule[i].speed     = speed;
			m_particule[i].zoom      = 1.0f;
			m_particule[i].angle     = 0.0f;
			m_particule[i].intensity = 1.0f;
			m_particule[i].type      = type;
			m_particule[i].phase     = PARPHSTART;
			m_particule[i].texSup.x  = 0.0f;
			m_particule[i].texSup.y  = 0.0f;
			m_particule[i].texInf.x  = 0.0f;
			m_particule[i].texInf.y  = 0.0f;
			m_particule[i].time      = 0.0f;
			m_particule[i].objFather = 0;
			m_particule[i].trackRank = -1;
			m_triangle[i] = *triangle;

			m_totalInterface[t][sheet] ++;

			return i | ((m_particule[i].uniqueStamp&0xffff)<<16);
		}
	}

	return -1;
}

// Créé une nouvelle particule étant une partie d'objet.
// Retourne le canal de la particule crée ou -1 en cas d'erreur.

int CParticule::CreatePart(D3DVECTOR pos, D3DVECTOR speed, FPOINT dim,
						   ParticuleType type,
						   float duration, float mass, float weight,
						   int sheet)
{
	int			i, j, t;

	t = 0;
	for ( j=0 ; j<MAXPARTICULE ; j++ )
	{
		i = MAXPARTICULE*t+j;

		if ( !m_particule[i].bUsed )
		{
			ZeroMemory(&m_particule[i], sizeof(Particule));
			m_particule[i].bUsed     = TRUE;
			m_particule[i].bRay      = FALSE;
			m_particule[i].uniqueStamp = m_uniqueStamp++;
			m_particule[i].sheet     = sheet;
			m_particule[i].mass      = mass;
			m_particule[i].weight    = weight;
			m_particule[i].duration  = duration;
			m_particule[i].pos       = pos;
			m_particule[i].goal      = pos;
			m_particule[i].speed     = speed;
			m_particule[i].dim       = dim;
			m_particule[i].zoom      = 1.0f;
			m_particule[i].angle     = 0.0f;
			m_particule[i].intensity = 1.0f;
			m_particule[i].type      = type;
			m_particule[i].phase     = PARPHSTART;
			m_particule[i].texSup.x  = 0.0f;
			m_particule[i].texSup.y  = 0.0f;
			m_particule[i].texInf.x  = 0.0f;
			m_particule[i].texInf.y  = 0.0f;
			m_particule[i].time      = 0.0f;
			m_particule[i].trackRank = -1;

			m_totalInterface[t][sheet] ++;

			return i | ((m_particule[i].uniqueStamp&0xffff)<<16);
		}
	}

	return -1;
}

// Créé une nouvelle particule linéaire (rayon).
// Retourne le canal de la particule crée ou -1 en cas d'erreur.

int CParticule::CreateRay(D3DVECTOR pos, D3DVECTOR goal,
						  ParticuleType type, FPOINT dim,
						  float duration, int sheet)
{
	int		i, j, t;

	t = -1;
	if ( type == PARTIRAY1 ||
		 type == PARTIRAY2 ||
		 type == PARTIRAY3 ||
		 type == PARTIRAY4 )
	{
		t = 3;  // effect02
	}
	if ( t >= MAXPARTITYPE )  return -1;
	if ( t == -1 )  return -1;

	for ( j=0 ; j<MAXPARTICULE ; j++ )
	{
		i = MAXPARTICULE*t+j;

		if ( !m_particule[i].bUsed )
		{
			ZeroMemory(&m_particule[i], sizeof(Particule));
			m_particule[i].bUsed     = TRUE;
			m_particule[i].bRay      = TRUE;
			m_particule[i].uniqueStamp = m_uniqueStamp++;
			m_particule[i].sheet     = sheet;
			m_particule[i].mass      = 0.0f;
			m_particule[i].duration  = duration;
			m_particule[i].pos       = pos;
			m_particule[i].goal      = goal;
			m_particule[i].speed     = D3DVECTOR(0.0f, 0.0f, 0.0f);
			m_particule[i].dim       = dim;
			m_particule[i].zoom      = 1.0f;
			m_particule[i].angle     = 0.0f;
			m_particule[i].intensity = 1.0f;
			m_particule[i].type      = type;
			m_particule[i].phase     = PARPHSTART;
			m_particule[i].texSup.x  = 0.0f;
			m_particule[i].texSup.y  = 0.0f;
			m_particule[i].texInf.x  = 0.0f;
			m_particule[i].texInf.y  = 0.0f;
			m_particule[i].time      = 0.0f;
			m_particule[i].objFather = 0;
			m_particule[i].trackRank = -1;

			m_totalInterface[t][sheet] ++;

			return i | ((m_particule[i].uniqueStamp&0xffff)<<16);
		}
	}

	return -1;
}

// Crée une particule avec une traînée.
// "length" est la durée de la queue de la traînée (en secondes) !

int CParticule::CreateTrack(D3DVECTOR pos, D3DVECTOR speed, FPOINT dim,
							ParticuleType type, float duration, float mass,
							float length, float width)
{
	int		channel, rank, i;

	// Crée la particule normale.
	channel = CreateParticule(pos, speed, dim, type, duration, mass, 0);
	if ( channel == -1 )  return -1;

	// Cherche une traînée libre.
	for ( i=0 ; i<MAXTRACK ; i++ )
	{
		if ( !m_track[i].bUsed )  // libre ?
		{
			rank = channel;
			if ( !CheckChannel(rank) )  return -1;
			m_particule[rank].trackRank = i;

			m_track[i].bUsed = TRUE;
			m_track[i].step = (length/duration)/MAXTRACKLEN;
			m_track[i].last = 0.0f;
			m_track[i].intensity = 1.0f;
			m_track[i].width = width;
			m_track[i].used = 1;
			m_track[i].head = 0;
			m_track[i].pos[0] = pos;
			break;
		}
	}

	return channel;
}

// Crée une trace de pneu.

void CParticule::CreateWheelTrace(const D3DVECTOR &p1, const D3DVECTOR &p2,
								  const D3DVECTOR &p3, const D3DVECTOR &p4,
								  ParticuleType type)
{
	int		i, max;

	max = MAXWHEELTRACE;
	i = m_wheelTraceIndex++;
	if ( m_wheelTraceIndex > max )  m_wheelTraceIndex = 0;

	m_wheelTrace[i].type = type;
	m_wheelTrace[i].pos[0] = p1;  // ul
	m_wheelTrace[i].pos[1] = p2;  // dl
	m_wheelTrace[i].pos[2] = p3;  // ur
	m_wheelTrace[i].pos[3] = p4;  // dr
	m_wheelTrace[i].startTime = m_absTime;

	if ( m_terrain == 0 )
	{
		m_terrain = (CTerrain*)m_iMan->SearchInstance(CLASS_TERRAIN);
	}

	m_wheelTrace[i].pos[0].y = 0.0f;
	m_wheelTrace[i].pos[0].y += 0.2f;  // juste en dessus du sol

	m_wheelTrace[i].pos[1].y = 0.0f;
	m_wheelTrace[i].pos[1].y += 0.2f;  // juste en dessus du sol

	m_wheelTrace[i].pos[2].y = 0.0f;
	m_wheelTrace[i].pos[2].y += 0.2f;  // juste en dessus du sol

	m_wheelTrace[i].pos[3].y = 0.0f;
	m_wheelTrace[i].pos[3].y += 0.2f;  // juste en dessus du sol

	if ( m_wheelTraceTotal < max )
	{
		m_wheelTraceTotal ++;
	}
	else
	{
		m_wheelTraceTotal = max;
	}
}

// Crée un rayon de solail.

BOOL CParticule::CreateSunbeam(const D3DVECTOR &pos, float dim, float angle,
							   ParticuleType type)
{
	int		i;

	// Cherche une place libre.
	for ( i=0 ; i<MAXSUNBEAM ; i++ )
	{
		if ( !m_sunbeam[i].bUsed )  // libre ?
		{
			m_sunbeam[i].bUsed = TRUE;
			m_sunbeam[i].type  = type;
			m_sunbeam[i].pos   = pos;
			m_sunbeam[i].dim   = dim;
			m_sunbeam[i].angle = angle;
			return TRUE;
		}
	}

	return FALSE;
}

// Crée un effet pour montrer une destination.

void CParticule::CreateGoto(D3DVECTOR pos)
{
	m_bGoto = TRUE;
	m_gotoPos = pos;
	m_gotoProgress = 0.0f;
}


// Check un numéro de canal.
// Adapte le canal pour qu'il puisse être utilisé comme offset
// dans m_particule.

BOOL CParticule::CheckChannel(int &channel)
{
	int		uniqueStamp;

	uniqueStamp = (channel>>16)&0xffff;
	channel &= 0xffff;

	if ( channel < 0 )  return FALSE;
	if ( channel >= MAXPARTICULE*MAXPARTITYPE )  return FALSE;
#if 0
	if ( !m_particule[channel].bUsed )  return FALSE;

	if ( m_particule[channel].uniqueStamp != uniqueStamp )  return FALSE;
#else
	if ( !m_particule[channel].bUsed )
	{
		OutputDebugString("CheckChannel bUsed=FALSE !\n");
		return FALSE;
	}

	if ( m_particule[channel].uniqueStamp != uniqueStamp )
	{
		OutputDebugString("CheckChannel uniqueStamp !\n");
		return FALSE;
	}
#endif

	return TRUE;
}

// Supprime une particule d'après son rang.

void CParticule::DeleteRank(int rank)
{
	int		i;

	if ( m_totalInterface[rank/MAXPARTICULE][m_particule[rank].sheet] > 0 )
	{
		m_totalInterface[rank/MAXPARTICULE][m_particule[rank].sheet] --;
	}

	i = m_particule[rank].trackRank;
	if ( i != -1 )  // traînée associée ?
	{
		m_track[i].bUsed = FALSE;  // libère la traînée
	}

	m_particule[rank].bUsed = FALSE;
}

// Supprime toutes les particules d'un type donné.

void CParticule::DeleteParticule(ParticuleType type)
{
	int		i;

	for ( i=0 ; i<MAXPARTICULE*MAXPARTITYPE ; i++ )
	{
		if ( !m_particule[i].bUsed )  continue;
		if ( m_particule[i].type != type )  continue;

		DeleteRank(i);
	}
}

// Supprime une particule d'après son canal.

void CParticule::DeleteParticule(int channel)
{
	int		i;

	if ( !CheckChannel(channel) )  return;

	if ( m_totalInterface[channel/MAXPARTICULE][m_particule[channel].sheet] > 0 )
	{
		m_totalInterface[channel/MAXPARTICULE][m_particule[channel].sheet] --;
	}

	i = m_particule[channel].trackRank;
	if ( i != -1 )  // traînée associée ?
	{
		m_track[i].bUsed = FALSE;  // libère la traînée
	}

	m_particule[channel].bUsed = FALSE;
}


// Spécifie l'objet père qui a créé la particule.

BOOL CParticule::SetObjectFather(int channel, CObject *object)
{
	if ( !CheckChannel(channel) )  return FALSE;
	m_particule[channel].objFather = object;
	return TRUE;
}

BOOL CParticule::SetPosition(int channel, D3DVECTOR pos)
{
	if ( !CheckChannel(channel) )  return FALSE;
	m_particule[channel].pos = pos;
	return TRUE;
}

BOOL CParticule::SetGoal(int channel, D3DVECTOR pos)
{
	if ( !CheckChannel(channel) )  return FALSE;
	m_particule[channel].goal = pos;
	return TRUE;
}

BOOL CParticule::SetDimension(int channel, FPOINT dim)
{
	if ( !CheckChannel(channel) )  return FALSE;
	m_particule[channel].dim = dim;
	return TRUE;
}

BOOL CParticule::SetZoom(int channel, float zoom)
{
	if ( !CheckChannel(channel) )  return FALSE;
	m_particule[channel].zoom = zoom;
	return TRUE;
}

BOOL CParticule::SetAngle(int channel, float angle)
{
	if ( !CheckChannel(channel) )  return FALSE;
	m_particule[channel].angle = angle;
	return TRUE;
}

BOOL CParticule::SetIntensity(int channel, float intensity)
{
	if ( !CheckChannel(channel) )  return FALSE;
	m_particule[channel].intensity = intensity;
	return TRUE;
}

BOOL CParticule::SetParam(int channel, D3DVECTOR pos, FPOINT dim, float zoom,
						  float angle, float intensity)
{
	if ( !CheckChannel(channel) )  return FALSE;
	m_particule[channel].pos       = pos;
	m_particule[channel].dim       = dim;
	m_particule[channel].zoom      = zoom;
	m_particule[channel].angle     = angle;
	m_particule[channel].intensity = intensity;
	return TRUE;
}

// Retourne la position de la particule.

BOOL CParticule::GetPosition(int channel, D3DVECTOR &pos)
{
	if ( !CheckChannel(channel) )  return FALSE;
	pos = m_particule[channel].pos;
	return TRUE;
}


// Indique l'intensité générale des rayons de soleil.

void CParticule::SetSunbeamIntensity(float intensity)
{
	m_sunbeamIntensity = intensity;
}


// Indique si une feuille évolue ou non.

void CParticule::SetFrameUpdate(int sheet, BOOL bUpdate)
{
	m_bFrameUpdate[sheet] = bUpdate;
}

// Fait évoluer toutes les particules.

void CParticule::FrameParticule(float rTime)
{
	D3DVECTOR	eye, pos, speed;
	FPOINT		ts, ti, dim;
	BOOL		bPause;
	float		progress, dp, h, amplitude, duration;
	int			i, j, r;

	if ( m_main == 0 )
	{
		m_main = (CRobotMain*)m_iMan->SearchInstance(CLASS_MAIN);
	}

	bPause = m_engine->RetPause();

	if ( m_terrain == 0 )
	{
		m_terrain = (CTerrain*)m_iMan->SearchInstance(CLASS_TERRAIN);
	}
	if ( m_water == 0 )
	{
		m_water = (CWater*)m_iMan->SearchInstance(CLASS_WATER);
	}

	if ( !bPause )
	{
		m_lastTimeGunDel += rTime;
		m_absTime += rTime;
	}

	if ( m_bGoto )
	{
		m_gotoProgress += rTime;
		if ( m_gotoProgress >= 1.0f )
		{
			m_bGoto = FALSE;
		}
	}

	eye = m_engine->RetEyePt();

	for ( i=0 ; i<MAXPARTICULE*MAXPARTITYPE ; i++ )
	{
		if ( !m_particule[i].bUsed )  continue;
		if ( !m_bFrameUpdate[m_particule[i].sheet] )  continue;

		if ( m_particule[i].type != PARTIQUARTZ )
		{
			m_particule[i].pos += m_particule[i].speed*rTime;
		}

		progress = m_particule[i].time/m_particule[i].duration;

		// Gère les particules avec masse qui rebondissent.
		if ( m_particule[i].mass != 0.0f        &&
			 m_particule[i].type != PARTIQUARTZ )
		{
			m_particule[i].speed.y -= m_particule[i].mass*rTime;

			if ( m_particule[i].type == PARTIFRAG )
			{
				if ( m_particule[i].sheet == SH_INTERFACE )
				{
					h = 0.0f;
				}
				else
				{
					h = m_terrain->RetFloorLevel(m_particule[i].pos);
				}
				h += m_particule[i].dim.y*0.75f;
				if ( m_particule[i].pos.y < h )  // choc avec le sol ?
				{
					DeleteRank(i);
					continue;
				}
			}
			else
			{
				if ( m_particule[i].sheet == SH_INTERFACE )
				{
					h = 0.0f;
				}
				else
				{
					h = m_terrain->RetFloorLevel(m_particule[i].pos);
				}
				h += m_particule[i].dim.y*0.75f;
				if ( m_particule[i].pos.y < h )  // choc avec le sol ?
				{
					if ( m_particule[i].type == PARTIPART &&
						 m_particule[i].weight > 5.0f &&  // assez lourd ?
						 m_particule[i].bounce < 3 )
					{
						amplitude = m_particule[i].weight*0.1f;
						amplitude *= 1.0f-0.3f*m_particule[i].bounce;
						if ( amplitude > 1.0f )  amplitude = 1.0f;
						if ( amplitude > 0.0f )
						{
							Play((Sound)(SOUND_FALLo1+rand()%2), m_particule[i].pos, amplitude, 1.0f);
						}
					}

					if ( m_particule[i].bounce < 3 )
					{
						m_particule[i].pos.y = h;
						m_particule[i].speed.y *= -0.4f;
						m_particule[i].speed.x *=  0.4f;
						m_particule[i].speed.z *=  0.4f;
						m_particule[i].bounce ++;  // un choc de plus
					}
					else	// disparaît après 3 rebonds ?
					{
						if ( m_particule[i].pos.y < h-20.0f ||
							 m_particule[i].time >= 20.0f   )
						{
							DeleteRank(i);
							continue;
						}
					}
				}
			}
		}

		// Gère la traînée associée.
		r = m_particule[i].trackRank;
		if ( r != -1 )  // traînée existe ?
		{
			if ( TrackMove(r, m_particule[i].pos, progress) )
			{
				DeleteRank(i);
				continue;
			}

			m_track[r].bDrawParticule = (progress < 1.0f);
		}

		if ( m_particule[i].type == PARTITRACK1 )  // explosion technique ?
		{
			m_particule[i].zoom = 1.0f-(m_particule[i].time-m_particule[i].duration);

			ts.x = 0.375f;
			ts.y = 0.000f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTITRACK2 )  // jet vert ?
		{
			m_particule[i].zoom = 1.0f-(m_particule[i].time-m_particule[i].duration);

			ts.x = 0.625f;
			ts.y = 0.000f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTITRACK3 )  // araignée ?
		{
			m_particule[i].zoom = 1.0f-(m_particule[i].time-m_particule[i].duration);

			ts.x = 0.500f;
			ts.y = 0.750f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTITRACK4 )  // explosion organique ?
		{
//?			m_particule[i].zoom = 1.0f-(m_particule[i].time-m_particule[i].duration);
			m_particule[i].zoom = 1.0f-progress;

			ts.x = 0.500f;
			ts.y = 0.750f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTITRACK5 )  // derrick ?
		{
			m_particule[i].zoom = 1.0f-(m_particule[i].time-m_particule[i].duration);

			ts.x = 0.750f;
			ts.y = 0.000f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTITRACK6 )  // reset in/out ?
		{
			ts.x = 0.0f;
			ts.y = 0.0f;
			ti.x = 0.0f;
			ti.y = 0.0f;
		}

		if ( m_particule[i].type == PARTITRACK7  ||  // win-1 ?
			 m_particule[i].type == PARTITRACK8  ||  // win-2 ?
			 m_particule[i].type == PARTITRACK9  ||  // win-3 ?
			 m_particule[i].type == PARTITRACK10 )   // win-4 ?
		{
			m_particule[i].zoom = 1.0f-(m_particule[i].time-m_particule[i].duration);

			ts.x = 0.25f*(m_particule[i].type-PARTITRACK7);
			ts.y = 0.25f;
			ti.x = ts.x+0.25f;
			ti.y = ts.y+0.25f;
		}

		if ( m_particule[i].type == PARTITRACK11 )  // tir phazer ?
		{
			m_particule[i].goal = m_particule[i].pos;
			m_particule[i].zoom = 1.0f-(m_particule[i].time-m_particule[i].duration);

			ts.x = 0.375f;
			ts.y = 0.000f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTITRACK12 )  // traînée réacteur ?
		{
			m_particule[i].zoom = 1.0f;

			ts.x = 0.375f;
			ts.y = 0.000f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTITRACK13 )  // jet d'eau ?
		{
			ts.x = 0.0f;
			ts.y = 0.0f;
			ti.x = 0.0f;
			ti.y = 0.0f;
		}

		if ( m_particule[i].type == PARTIMOTOR )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			m_particule[i].zoom = 1.0f-progress;
			m_particule[i].intensity = 1.0f-progress;

			ts.x = 0.000f;
			ts.y = 0.750f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTIBLITZ )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			m_particule[i].zoom = 1.0f-progress;
			m_particule[i].angle = Rand()*PI*2.0f;

			ts.x = 0.125f;
			ts.y = 0.750f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTIBLITZb )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			m_particule[i].zoom = 1.0f-progress;
			m_particule[i].angle = Rand()*PI*2.0f;

			ts.x = 0.375f;
			ts.y = 0.875f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTICRASH )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			if ( progress < 0.25f )
			{
				m_particule[i].zoom = progress/0.25f;
				m_particule[i].intensity = 0.7f;
			}
			else
			{
				m_particule[i].intensity = 1.0f-(progress-0.25f)/0.75f;
				m_particule[i].intensity *= 0.7f;
			}

			ts.x = 0.000f;
			ts.y = 0.750f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTIDUST1 )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			if ( progress < 0.25f )
			{
				m_particule[i].zoom = progress/0.25f;
				m_particule[i].intensity = 0.5f;
			}
			else
			{
				m_particule[i].intensity = 1.0f-(progress-0.25f)/0.75f;
				m_particule[i].intensity *= 0.5f;
			}

			ts.x = 0.625f;
			ts.y = 0.125f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTIDUST2 )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			m_particule[i].intensity = 1.0f-progress;
			m_particule[i].intensity *= 0.5f;

			ts.x = 0.625f;
			ts.y = 0.125f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTIVAPOR )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			m_particule[i].intensity = 1.0f-progress;
			m_particule[i].zoom = 1.0f+progress*3.0f;

			ts.x = 0.000f;
			ts.y = 0.750f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTIGAS )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			m_particule[i].zoom = 1.0f-progress;

			ts.x = 0.375f;
			ts.y = 0.750f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTIBASE )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			m_particule[i].zoom = 1.0f+progress*7.0f;
			m_particule[i].intensity = powf(1.0f-progress, 3.0f);

			ts.x = 0.375f;
			ts.y = 0.750f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTIFIRE  ||
			 m_particule[i].type == PARTIFIREZ )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			if ( m_particule[i].type == PARTIFIRE )
			{
				m_particule[i].zoom = 1.0f-progress;
			}
			else
			{
				m_particule[i].zoom = progress;
			}

			ts.x = 0.500f;
			ts.y = 0.750f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTIFLIC )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			m_particule[i].zoom = 0.1f+progress;
			m_particule[i].intensity = 1.0f-progress;

			ts.x = 0.25f;
			ts.y = 0.50f;
			ti.x = ts.x+0.25f;
			ti.y = ts.y+0.25f;
		}

		if ( m_particule[i].type == PARTIPOWER )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

//?			m_particule[i].zoom = 0.1f+progress;
//?			m_particule[i].intensity = 1.0f-progress;
			m_particule[i].intensity = 1.0f-progress;

			ts.x = 0.00f;
			ts.y = 0.75f;
			if ( rand()%2 == 0 )  ts.x += 0.25f;
			ti.x = ts.x+0.25f;
			ti.y = ts.y+0.25f;
			if ( rand()%2 == 0 )  Swap(ts.x, ti.x);
			if ( rand()%2 == 0 )  Swap(ts.y, ti.y);
		}

		if ( m_particule[i].type == PARTIGLU )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

//?			m_particule[i].zoom = 0.1f+progress;
//?			m_particule[i].intensity = 1.0f-progress;
			m_particule[i].intensity = 1.0f-progress;

			ts.x = 0.50f;
			ts.y = 0.00f;
			ti.x = ts.x+0.25f;
			ti.y = ts.y+0.25f;
			if ( rand()%2 == 0 )  Swap(ts.x, ti.x);
			if ( rand()%2 == 0 )  Swap(ts.y, ti.y);
		}

		if ( m_particule[i].type == PARTICHOC )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			m_particule[i].zoom = 0.1f+progress;
			m_particule[i].intensity = 1.0f-progress;

			ts.x = 0.50f;
			ts.y = 0.50f;
			ti.x = ts.x+0.25f;
			ti.y = ts.y+0.25f;
		}

		if ( m_particule[i].type == PARTITERSPTIRE )
		{
			m_particule[i].zoom = 1.0f;
			m_particule[i].intensity = 1.0f;

			ts.x = 0.00f;
			ts.y = 0.50f;
			ti.x = ts.x+0.25f;
			ti.y = ts.y+0.25f;
		}

		if ( m_particule[i].type == PARTILOCKZONE )
		{
			m_particule[i].zoom = 1.0f;
			m_particule[i].intensity = 1.0f;

			ts.x = 0.875f;
			ts.y = 0.625f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTIGRIDb )
		{
			m_particule[i].zoom = 1.0f;
			m_particule[i].intensity = 1.0f;

			ts.x = 0.000f;
			ts.y = 0.500f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}
		if ( m_particule[i].type == PARTIGRIDt )
		{
			m_particule[i].zoom = 1.0f;
			m_particule[i].intensity = 1.0f;

			ts.x = 0.125f;
			ts.y = 0.500f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}
		if ( m_particule[i].type == PARTIGRIDw )
		{
			m_particule[i].zoom = 1.0f;
			m_particule[i].intensity = 1.0f;

			ts.x = 0.125f;
			ts.y = 0.625f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}
		if ( m_particule[i].type == PARTIGRIDs )
		{
			m_particule[i].zoom = 1.0f;
			m_particule[i].intensity = 1.0f;

			ts.x = 0.000f;
			ts.y = 0.625f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}
		if ( m_particule[i].type == PARTIGRIDe )
		{
			m_particule[i].zoom = 1.0f;
			m_particule[i].intensity = 1.0f;

			ts.x = 0.875f;
			ts.y = 0.500f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTISIGNMARK0 )
		{
			m_particule[i].intensity = 1.0f;

			ts.x = 0.250f;
			ts.y = 0.500f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}
		if ( m_particule[i].type == PARTISIGNMARK1 )
		{
			m_particule[i].intensity = 1.0f;

			ts.x = 0.250f;
			ts.y = 0.625f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}
		if ( m_particule[i].type == PARTISIGNMARK2 )
		{
			m_particule[i].intensity = 1.0f;

			ts.x = 0.375f;
			ts.y = 0.500f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}
		if ( m_particule[i].type == PARTISIGNMARK3 )
		{
			m_particule[i].intensity = 1.0f;

			ts.x = 0.375f;
			ts.y = 0.625f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}
		if ( m_particule[i].type == PARTISIGNMARK4 )
		{
			m_particule[i].intensity = 1.0f;

			ts.x = 0.500f;
			ts.y = 0.500f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}
		if ( m_particule[i].type == PARTISIGNMARK5 )
		{
			m_particule[i].intensity = 1.0f;

			ts.x = 0.500f;
			ts.y = 0.625f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}
		if ( m_particule[i].type == PARTISIGNMARK6 )
		{
			m_particule[i].intensity = 1.0f;

			ts.x = 0.625f;
			ts.y = 0.500f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}
		if ( m_particule[i].type == PARTISIGNMARK7 )
		{
			m_particule[i].intensity = 1.0f;

			ts.x = 0.625f;
			ts.y = 0.625f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}
		if ( m_particule[i].type == PARTISIGNMARK8 )
		{
			m_particule[i].intensity = 1.0f;

			ts.x = 0.750f;
			ts.y = 0.500f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}
		if ( m_particule[i].type == PARTISIGNMARK9 )
		{
			m_particule[i].intensity = 1.0f;

			ts.x = 0.750f;
			ts.y = 0.625f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}
		if ( m_particule[i].type == PARTISIGNMARK10 )
		{
			m_particule[i].intensity = 1.0f;

			ts.x = 0.875f;
			ts.y = 0.500f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}
		if ( m_particule[i].type == PARTISIGNMARK11 )
		{
			m_particule[i].intensity = 1.0f;

			ts.x = 0.875f;
			ts.y = 0.625f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTIFOG0 )
		{
			m_particule[i].zoom = progress;
			m_particule[i].intensity = 0.3f+sinf(progress)*0.15f;
			m_particule[i].angle += rTime*0.05f;

			ts.x = 0.25f;
			ts.y = 0.75f;
			ti.x = ts.x+0.25f;
			ti.y = ts.y+0.25f;
		}
		if ( m_particule[i].type == PARTIFOG1 )
		{
			m_particule[i].zoom = progress;
			m_particule[i].intensity = 0.3f+sinf(progress)*0.15f;
			m_particule[i].angle -= rTime*0.07f;

			ts.x = 0.25f;
			ts.y = 0.75f;
			ti.x = ts.x+0.25f;
			ti.y = ts.y+0.25f;
		}

		if ( m_particule[i].type == PARTIFOG2 )
		{
			m_particule[i].zoom = progress;
			m_particule[i].intensity = 0.6f+sinf(progress)*0.15f;
			m_particule[i].angle += rTime*0.05f;

			ts.x = 0.75f;
			ts.y = 0.75f;
			ti.x = ts.x+0.25f;
			ti.y = ts.y+0.25f;
		}
		if ( m_particule[i].type == PARTIFOG3 )
		{
			m_particule[i].zoom = progress;
			m_particule[i].intensity = 0.6f+sinf(progress)*0.15f;
			m_particule[i].angle -= rTime*0.07f;

			ts.x = 0.75f;
			ts.y = 0.75f;
			ti.x = ts.x+0.25f;
			ti.y = ts.y+0.25f;
		}

		if ( m_particule[i].type == PARTIFOG4 )
		{
			m_particule[i].zoom = progress;
			m_particule[i].intensity = 0.5f+sinf(progress)*0.2f;
			m_particule[i].angle += rTime*0.05f;

			ts.x = 0.00f;
			ts.y = 0.25f;
			ti.x = ts.x+0.25f;
			ti.y = ts.y+0.25f;
		}
		if ( m_particule[i].type == PARTIFOG5 )
		{
			m_particule[i].zoom = progress;
			m_particule[i].intensity = 0.5f+sinf(progress)*0.2f;
			m_particule[i].angle -= rTime*0.07f;

			ts.x = 0.00f;
			ts.y = 0.25f;
			ti.x = ts.x+0.25f;
			ti.y = ts.y+0.25f;
		}

		if ( m_particule[i].type == PARTIFOG6 )
		{
			m_particule[i].zoom = progress;
			m_particule[i].intensity = 0.5f+sinf(progress)*0.2f;
			m_particule[i].angle += rTime*0.05f;

			ts.x = 0.50f;
			ts.y = 0.25f;
			ti.x = ts.x+0.25f;
			ti.y = ts.y+0.25f;
		}
		if ( m_particule[i].type == PARTIFOG7 )
		{
			m_particule[i].zoom = progress;
			m_particule[i].intensity = 0.5f+sinf(progress)*0.2f;
			m_particule[i].angle -= rTime*0.07f;

			ts.x = 0.50f;
			ts.y = 0.25f;
			ti.x = ts.x+0.25f;
			ti.y = ts.y+0.25f;
		}

		// Diminue l'intensité si la caméra est presque à la même
		// hauteur (nappe de brouillard à hauteur des yeux).
		if ( m_particule[i].type >= PARTIFOG0 &&
			 m_particule[i].type <= PARTIFOG9 )
		{
			h = 10.0f;

			if ( m_particule[i].pos.y >= eye.y   &&
				 m_particule[i].pos.y <  eye.y+h )
			{
				m_particule[i].intensity *= (m_particule[i].pos.y-eye.y)/h;
			}
			if ( m_particule[i].pos.y >  eye.y-h &&
				 m_particule[i].pos.y <  eye.y   )
			{
				m_particule[i].intensity *= (eye.y-m_particule[i].pos.y)/h;
			}
		}

		if ( m_particule[i].type == PARTIEXPLOT ||
			 m_particule[i].type == PARTIEXPLOA )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			m_particule[i].zoom = 1.0f-progress/2.0f;
			m_particule[i].intensity = 1.0f-progress;

			if ( m_particule[i].type == PARTIEXPLOT )
			{
				ts.x = 0.750f;  // orange
				ts.y = 0.750f;
			}
			else
			{
				ts.x = 0.625f;  // bleu
				ts.y = 0.875f;
			}
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTIEXPLOO )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

//?			m_particule[i].zoom = 1.0f-progress/2.0f;
			m_particule[i].zoom = 1.0f+progress*2.0f;
			m_particule[i].intensity = 1.0f-progress;

			ts.x = 0.875f;  // vert
			ts.y = 0.750f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTIEXPLOG1 )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			m_particule[i].intensity = 1.0f-progress;

			ts.x = 0.375f;
			ts.y = 0.000f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}
		if ( m_particule[i].type == PARTIEXPLOG2 )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			m_particule[i].intensity = 1.0f-progress;

			ts.x = 0.625f;
			ts.y = 0.000f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTIFLAME )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			m_particule[i].zoom = 1.0f-progress/2.0f;
			if ( progress < 0.5f )
			{
				m_particule[i].intensity = progress/0.5f;
			}
			else
			{
				m_particule[i].intensity = 2.0f-progress/0.5f;
			}

			ts.x = 0.750f;
			ts.y = 0.750f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTIBUBBLE )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			m_particule[i].zoom = 1.0f-progress/2.0f;
			m_particule[i].intensity = 1.0f-progress;

			ts.x = 0.125f;
			ts.y = 0.875f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTISMOKE1 ||
			 m_particule[i].type == PARTISMOKE2 ||
			 m_particule[i].type == PARTISMOKE3 )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			if ( progress < 0.25f )
			{
				m_particule[i].zoom = progress/0.25f;
			}
			else
			{
				m_particule[i].intensity = 1.0f-(progress-0.25f)/0.75f;
			}

			ts.x = 0.00f+0.25f*(m_particule[i].type-PARTISMOKE1);
			ts.y = 0.75f;
			ti.x = ts.x+0.25f;
			ti.y = ts.y+0.25f;
		}

		if ( m_particule[i].type == PARTIWHEEL )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

//?			m_particule[i].intensity = 1.0f-progress;
			if ( progress < 0.50f )
			{
				m_particule[i].intensity = 1.0f;
			}
			else
			{
				m_particule[i].intensity = 1.0f-(progress-0.50f)/0.50f;
			}

			ts.x = 0.00f;
			ts.y = 0.75f;
			ti.x = ts.x+0.25f;
			ti.y = ts.y+0.25f;
		}

		if ( m_particule[i].type == PARTIBIGO )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			m_particule[i].intensity = 1.0f-progress;

			ts.x = 0.50f;
			ts.y = 0.00f;
			ti.x = ts.x+0.25f;
			ti.y = ts.y+0.25f;
		}

		if ( m_particule[i].type == PARTIBIGT )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			m_particule[i].intensity = 1.0f-progress;

			ts.x = 0.25f;
			ts.y = 0.00f;
			ti.x = ts.x+0.25f;
			ti.y = ts.y+0.25f;
		}

		if ( m_particule[i].type == PARTIBLUE )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			m_particule[i].zoom = 1.0f-progress;

			ts.x = 0.625f;
			ts.y = 0.750f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTISNOW )
		{
			if ( progress >= 1.0f ||
				 m_particule[i].pos.y < m_particule[i].ground )
			{
				DeleteRank(i);
				continue;
			}

			ts.x = 0.000f;
			ts.y = 0.125f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTIRAIN )
		{
			if ( m_particule[i].pos.y < m_particule[i].ground &&
				 m_engine->RetSetup(ST_AMBIANCE) != 0 &&
				 Length2d(m_particule[i].pos, eye) < 40.0f )
			{
				r = rand()%4;
				if ( r == 0 || m_particule[i].ground >= -8.0f )
				{
					pos = m_particule[i].pos;
					pos.y = m_particule[i].ground;
					dim.x = 1.5f;  // hauteur
					dim.y = 0.3f;  // diamètre
					CreateParticule(pos, D3DVECTOR(0.0f, 0.0f, 0.0f), dim, PARTIPLOUF0, 1.4f, 0.0f);
				}
				else
				{
					for ( j=0 ; j<4 ; j++ )
					{
						pos = m_particule[i].pos;
						pos.x += (Rand()-0.5f)*4.0f;
						pos.z += (Rand()-0.5f)*4.0f;
						pos.y = m_water->RetLevel()+1.0f;
						dim.x = 2.0f+Rand()*2.0f;
						dim.y = dim.x;
						duration = 2.0f+Rand()*2.0f;
						CreateParticule(pos, D3DVECTOR(0.0f, 0.0f, 0.0f), dim, PARTIFLIC, duration, 0.0f);
					}
				}

				if ( m_particule[i].ground >= -8.0f )  // sur le sol ?
				{
					Play((Sound)(SOUND_FLIC2+rand()%2), m_particule[i].pos, 0.4f, 0.8f+Rand()*0.4f);
				}
				else	// sur l'eau ?
				{
					Play((Sound)(SOUND_FLIC1+rand()%2), m_particule[i].pos, 0.4f, 0.8f+Rand()*0.4f);
				}
			}

			if ( progress >= 1.0f ||
				 m_particule[i].pos.y < m_particule[i].ground )
			{
				DeleteRank(i);
				continue;
			}

			ts.x = 0.750f;
			ts.y = 0.625f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTIROOT )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

#if 1
			m_particule[i].zoom = 1.0f+progress*3.0f;
			m_particule[i].intensity = 1.0f-progress;
#else
			if ( progress < 0.25f )
			{
				m_particule[i].zoom = progress/0.25f;
			}
			else
			{
				m_particule[i].intensity = 1.0f-(progress-0.25f)/0.75f;
			}
#endif

			ts.x = 0.000f;
			ts.y = 0.000f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTIRECOVER )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			if ( progress < 0.25f )
			{
				m_particule[i].zoom = progress/0.25f;
			}
			else
			{
				m_particule[i].intensity = 1.0f-(progress-0.25f)/0.75f;
			}

			ts.x = 0.875f;
			ts.y = 0.000f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTIEJECT )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			m_particule[i].zoom = 1.0f+powf(progress, 2.0f)*5.0f;
			m_particule[i].intensity = 1.0f-progress;

			ts.x = 0.625f;
			ts.y = 0.875f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTISCRAPS )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			m_particule[i].zoom = 1.0f-progress;

			ts.x = 0.625f;
			ts.y = 0.875f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTIFRAG )
		{
			m_particule[i].angle += rTime*PI*0.5f;

			ts.x = 0.0f;
			ts.y = 0.0f;
			ti.x = 0.0f;
			ti.y = 0.0f;
		}

		if ( m_particule[i].type == PARTIPART )
		{
			ts.x = 0.0f;
			ts.y = 0.0f;
			ti.x = 0.0f;
			ti.y = 0.0f;
		}

		if ( m_particule[i].type == PARTIORGANIC1 )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
#if 0
				pos      = m_particule[i].pos;
				dim.x    = m_particule[i].dim.x/4.0f;
				dim.y    = dim.x;
				duration = m_particule[i].duration;
				mass     = m_particule[i].mass;
				total = (int)(10.0f*m_engine->RetParticuleDensity());
				for ( i=0 ; i<total ; i++ )
				{
					speed.x = (Rand()-0.5f)*20.0f;
					speed.y = (Rand()-0.5f)*20.0f;
					speed.z = (Rand()-0.5f)*20.0f;
					CreateParticule(pos, speed, dim, PARTIORGANIC2, duration, mass);
				}
				total = (int)(5.0f*m_engine->RetParticuleDensity());
				for ( i=0 ; i<total ; i++ )
				{
					speed.x = (Rand()-0.5f)*20.0f;
					speed.y = (Rand()-0.5f)*20.0f;
					speed.z = (Rand()-0.5f)*20.0f;
					duration *= Rand()+0.8f;
					CreateTrack(pos, speed, dim, PARTITRACK4, duration, mass, duration*0.2f, dim.x*2.0f);
				}
#endif
				continue;
			}

//?			m_particule[i].zoom = (m_particule[i].time-m_particule[i].duration);
			m_particule[i].zoom = 1.0f-progress;

			ts.x = 0.000f;
			ts.y = 0.875f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTIORGANIC2 )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			m_particule[i].zoom = 1.0f-(m_particule[i].time-m_particule[i].duration);

			ts.x = 0.000f;
			ts.y = 0.875f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTIGLINT )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			if ( progress > 0.5f )
			{
//?				m_particule[i].zoom = 1.0f-(m_particule[i].time-m_particule[i].duration/2.0f);
				m_particule[i].zoom = 1.0f-(progress-0.5f)*2.0f;
			}
			m_particule[i].angle = m_particule[i].time*PI;

			ts.x = 0.75f;
			ts.y = 0.25f;
			ti.x = ts.x+0.25f;
			ti.y = ts.y+0.25f;
		}

		if ( m_particule[i].type == PARTIGLINTb )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			if ( progress > 0.5f )
			{
				m_particule[i].zoom = 1.0f-(progress-0.5f)*2.0f;
			}
			m_particule[i].angle = m_particule[i].time*PI;

			ts.x = 0.75f;
			ts.y = 0.50f;
			ti.x = ts.x+0.25f;
			ti.y = ts.y+0.25f;
		}

		if ( m_particule[i].type == PARTIGLINTr )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			if ( progress > 0.5f )
			{
				m_particule[i].zoom = 1.0f-(progress-0.5f)*2.0f;
			}
			m_particule[i].angle = m_particule[i].time*PI;

			ts.x = 0.75f;
			ts.y = 0.00f;
			ti.x = ts.x+0.25f;
			ti.y = ts.y+0.25f;
		}

		if ( m_particule[i].type >= PARTILENS1 &&
			 m_particule[i].type <= PARTILENS4 )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			if ( progress < 0.5f )
			{
				m_particule[i].zoom = progress*2.0f;
			}
			else
			{
				m_particule[i].intensity = 1.0f-(progress-0.5f)*2.0f;
			}
//?			m_particule[i].angle = m_particule[i].time*PI;

			ts.x = 0.25f*(m_particule[i].type-PARTILENS1);
			ts.y = 0.25f;
			ti.x = ts.x+0.25f;
			ti.y = ts.y+0.25f;
		}

		if ( m_particule[i].type == PARTICONTROL )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			if ( progress < 0.3f )
			{
				m_particule[i].zoom = progress/0.3f;
			}
			else
			{
				m_particule[i].zoom = 1.0f;
				m_particule[i].intensity = 1.0f-(progress-0.3f)/0.7f;
			}

			ts.x = 0.00f;
			ts.y = 0.00f;
			ti.x = ts.x+0.25f;
			ti.y = ts.y+0.25f;
		}

		if ( m_particule[i].type == PARTIGUNDEL )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			if ( progress > 0.5f )
			{
				m_particule[i].zoom = 1.0f-(m_particule[i].time-m_particule[i].duration/2.0f);
			}
			m_particule[i].angle = m_particule[i].time*PI;

			ts.x = 0.75f;
			ts.y = 0.50f;
			ti.x = ts.x+0.25f;
			ti.y = ts.y+0.25f;
		}

		if ( m_particule[i].type == PARTIQUARTZ )
		{
			if ( progress >= 1.0f )
			{
				m_particule[i].time = 0.0f;
				m_particule[i].duration = 0.5f+Rand()*2.0f;
				m_particule[i].pos.x = m_particule[i].speed.x + (Rand()-0.5f)*m_particule[i].mass;
				m_particule[i].pos.y = m_particule[i].speed.y + (Rand()-0.5f)*m_particule[i].mass;
				m_particule[i].pos.z = m_particule[i].speed.z + (Rand()-0.5f)*m_particule[i].mass;
				m_particule[i].dim.x = 0.5f+Rand()*1.5f;
				m_particule[i].dim.y = m_particule[i].dim.x;
				progress = 0.0f;
			}

			if ( progress < 0.2f )
			{
				m_particule[i].zoom = progress/0.2f;
				m_particule[i].intensity = 1.0f;
			}
			else
			{
				m_particule[i].zoom = 1.0f;
				m_particule[i].intensity = 1.0f-(progress-0.2f)/0.8f;
			}

			ts.x = 0.25f;
			ts.y = 0.25f;
			ti.x = ts.x+0.25f;
			ti.y = ts.y+0.25f;
		}

		if ( m_particule[i].type == PARTILAVA )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			m_particule[i].zoom = progress*1.0f;
			m_particule[i].intensity = 1.0f-progress;

			ts.x = 0.500f;
			ts.y = 0.875f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTISELY )
		{
			ts.x = 0.75f;
			ts.y = 0.25f;
			ti.x = ts.x+0.25f;
			ti.y = ts.y+0.25f;
		}
		if ( m_particule[i].type == PARTISELR )
		{
			ts.x = 0.75f;
			ts.y = 0.00f;
			ti.x = ts.x+0.25f;
			ti.y = ts.y+0.25f;
		}

		if ( m_particule[i].type == PARTISPHERE0 )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			m_particule[i].zoom = progress*m_particule[i].dim.x;
//?			m_particule[i].intensity = 1.0f-progress;
			if ( progress < 0.65f )
			{
				m_particule[i].intensity = progress/0.65f;
			}
			else
			{
				m_particule[i].intensity = 1.0f-(progress-0.65f)/0.35f;
			}
			m_particule[i].intensity *= 0.5f;

			ts.x = 0.50f;
			ts.y = 0.75f;
			ti.x = ts.x+0.25f;
			ti.y = ts.y+0.25f;
		}

		if ( m_particule[i].type == PARTISPHERE1 )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			if ( progress < 0.25f )
			{
				m_particule[i].intensity = progress/0.25f;
			}
			else if ( progress < 0.75f )
			{
				m_particule[i].intensity = 1.0f;
			}
			else
			{
				m_particule[i].intensity = 1.0f-(progress-0.75f)/0.25f;
			}
			m_particule[i].zoom = m_particule[i].dim.x;
//?			m_particule[i].angle = m_particule[i].time*PI*8.0f;
			m_particule[i].angle = Rand()*PI*2.0f;

			ts.x = 0.00f;
			ts.y = 0.75f;
			ti.x = ts.x+0.50f;
			ti.y = ts.y+0.25f;
		}

		if ( m_particule[i].type == PARTISPHERE2 )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			if ( progress < 0.20f )
			{
				m_particule[i].intensity = 1.0f;
			}
			else
			{
				m_particule[i].intensity = 1.0f-(progress-0.20f)/0.80f;
			}
			m_particule[i].zoom = progress*m_particule[i].dim.x;
			m_particule[i].angle = m_particule[i].time*PI*2.0f;

			ts.x = 0.125f;
			ts.y = 0.000f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTISPHERE3 )
		{
			if ( m_particule[i].phase == PARPHEND &&
				 progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			if ( m_particule[i].phase == PARPHSTART )
			{
				m_particule[i].intensity = progress;
				if ( m_particule[i].intensity > 1.0f )
				{
					m_particule[i].intensity = 1.0f;
				}
			}

			if ( m_particule[i].phase == PARPHEND )
			{
				m_particule[i].intensity = 1.0f-progress;
			}

			m_particule[i].zoom = m_particule[i].dim.x;
			m_particule[i].angle = m_particule[i].time*PI*0.2f;

			ts.x = 0.25f;
			ts.y = 0.75f;
			ti.x = ts.x+0.25f;
			ti.y = ts.y+0.25f;
		}

		if ( m_particule[i].type == PARTISPHERE4 )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			m_particule[i].zoom = progress*m_particule[i].dim.x;
			if ( progress < 0.65f )
			{
				m_particule[i].intensity = progress/0.65f;
			}
			else
			{
				m_particule[i].intensity = 1.0f-(progress-0.65f)/0.35f;
			}
			m_particule[i].intensity *= 0.5f;

			ts.x = 0.125f;
			ts.y = 0.000f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTISPHERE5 )
		{
			m_particule[i].intensity = 0.7f+sinf(progress)*0.3f;
			m_particule[i].zoom = m_particule[i].dim.x*(1.0f+sinf(progress*0.7f)*0.01f);
			m_particule[i].angle = m_particule[i].time*PI*0.2f;

			ts.x = 0.25f;
			ts.y = 0.50f;
			ti.x = ts.x+0.25f;
			ti.y = ts.y+0.25f;
		}

		if ( m_particule[i].type == PARTISPHERE6 )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			m_particule[i].zoom = (1.0f-progress)*m_particule[i].dim.x;
			m_particule[i].intensity = progress*0.5f;

			ts.x = 0.125f;
			ts.y = 0.000f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTIPLOUF0 )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			m_particule[i].zoom = progress;
#if 0
			if ( progress <= 0.5f )
			{
				m_particule[i].intensity = 1.0f;
			}
			else
			{
				m_particule[i].intensity = 1.0f-(progress-0.5f)/0.5f;
			}
#else
//?			m_particule[i].intensity = 1.0f;
			m_particule[i].intensity = 1.0f-progress;
#endif

			ts.x = 0.50f;
			ts.y = 0.50f;
			ti.x = ts.x+0.25f;
			ti.y = ts.y+0.25f;
		}

		if ( m_particule[i].type == PARTIDROP )
		{
			if ( progress >= 1.0f ||
				 m_particule[i].pos.y < m_water->RetLevel() )
			{
				DeleteRank(i);
				continue;
			}

			m_particule[i].zoom = 1.0f-progress;
			m_particule[i].intensity = 1.0f-progress;

			ts.x = 0.750f;
			ts.y = 0.500f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTIWATER )
		{
			if ( progress >= 1.0f ||
				 m_particule[i].pos.y < m_water->RetLevel() )
			{
				DeleteRank(i);
				continue;
			}

			m_particule[i].intensity = 1.0f-progress;

			ts.x = 0.125f;
			ts.y = 0.125f;
			ti.x = ts.x+0.125f;
			ti.y = ts.y+0.125f;
		}

		if ( m_particule[i].type == PARTIRAY1 )  // rayon tour ?
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			ts.x = 0.00f;
			ts.y = 0.00f;
			ti.x = ts.x+0.25f;
			ti.y = ts.y+0.25f;
		}

		if ( m_particule[i].type == PARTIRAY2 ||
			 m_particule[i].type == PARTIRAY3 )
		{
			if ( progress >= 1.0f )
			{
				DeleteRank(i);
				continue;
			}

			ts.x = 0.00f;
			ts.y = 0.25f;
			ti.x = ts.x+0.25f;
			ti.y = ts.y+0.25f;
		}

		dp = (1.0f/256.0f)/2.0f;
		m_particule[i].texSup.x = ts.x+dp;
		m_particule[i].texSup.y = ts.y+dp;
		m_particule[i].texInf.x = ti.x-dp;
		m_particule[i].texInf.y = ti.y-dp;
		m_particule[i].time     += rTime;
	}
}


// Déplace une traînée.
// Retourne TRUE lorsque la traînée est terminée.

BOOL CParticule::TrackMove(int i, D3DVECTOR pos, float progress)
{
	D3DVECTOR	last;
	int			h, hh;

	if ( i < 0 || i >= MAXTRACK )  return TRUE;
	if ( m_track[i].bUsed == FALSE )  return TRUE;

	if ( progress < 1.0f )  // particule existe ?
	{
		h = m_track[i].head;

		if ( m_track[i].used == 1 ||
			 m_track[i].last+m_track[i].step <= progress )
		{
			m_track[i].last = progress;
			last = m_track[i].pos[h];
			h ++;
			if ( h == MAXTRACKLEN )  h = 0;
			if ( m_track[i].used < MAXTRACKLEN )  m_track[i].used ++;
		}
		else
		{
			hh = h-1;
			if ( hh < 0 )  hh = MAXTRACKLEN-1;
			last = m_track[i].pos[hh];
		}
		m_track[i].pos[h] = pos;
		m_track[i].len[h] = Length(pos, last);

		m_track[i].head = h;

//?		m_track[i].intensity = 1.0f;
		m_track[i].intensity = 1.0f-progress;
	}
	else	// mort lente de la traînée ?
	{
//?		m_track[i].intensity = 1.0f-(progress-1.0f)/(m_track[i].step*MAXTRACKLEN);
		m_track[i].intensity = 0.0f;
	}

	return (m_track[i].intensity <= 0.0f);
}

// Dessine une traînée.

void CParticule::TrackDraw(int i, ParticuleType type)
{
	D3DVERTEX2	vertex[4];	// 2 triangles
	D3DVECTOR	corner[4], p1, p2, p, n, eye;
	D3DMATRIX	matrix;
	FPOINT		texInf, texSup, rot;
	float		lTotal, f1, f2, a;
	int			counter, h;

	// Calcule la longueur totale mémorisée.
	lTotal = 0.0f;
	h = m_track[i].head;
	for ( counter=0 ; counter<m_track[i].used-1 ; counter++ )
	{
		lTotal += m_track[i].len[h];
		h --;  if ( h < 0 )  h = MAXTRACKLEN-1;
	}

	D3DUtil_SetIdentityMatrix(matrix);
	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &matrix);

	if ( type == PARTITRACK1 )  // explosion technique ?
	{
		texInf.x = 64.5f/256.0f;
		texInf.y = 21.0f/256.0f;
		texSup.x = 95.5f/256.0f;
		texSup.y = 22.0f/256.0f;  // orange
	}
	if ( type == PARTITRACK2 )  // jet vert ?
	{
		texInf.x = 64.5f/256.0f;
		texInf.y = 13.0f/256.0f;
		texSup.x = 95.5f/256.0f;
		texSup.y = 14.0f/256.0f;  // vert
	}
	if ( type == PARTITRACK3 )  // araignée ?
	{
		texInf.x = 64.5f/256.0f;
		texInf.y =  5.0f/256.0f;
		texSup.x = 95.5f/256.0f;
		texSup.y =  6.0f/256.0f;  // brun
	}
	if ( type == PARTITRACK4 )  // explosion organique ?
	{
		texInf.x = 64.5f/256.0f;
		texInf.y = 49.0f/256.0f;
		texSup.x = 95.5f/256.0f;
		texSup.y = 50.0f/256.0f;  // rouge
	}
	if ( type == PARTITRACK5 )  // derrick ?
	{
		texInf.x = 64.5f/256.0f;
		texInf.y = 29.0f/256.0f;
		texSup.x = 95.5f/256.0f;
		texSup.y = 30.0f/256.0f;  // brun foncé
	}
	if ( type == PARTITRACK6 )  // reset in/out ?
	{
		texInf.x = 64.5f/256.0f;
		texInf.y = 17.0f/256.0f;
		texSup.x = 95.5f/256.0f;
		texSup.y = 18.0f/256.0f;  // cyan
	}
	if ( type == PARTITRACK7 )  // win-1 ?
	{
		texInf.x = 64.5f/256.0f;
		texInf.y = 41.0f/256.0f;
		texSup.x = 95.5f/256.0f;
		texSup.y = 42.0f/256.0f;  // orange
	}
	if ( type == PARTITRACK8 )  // win-2 ?
	{
		texInf.x = 64.5f/256.0f;
		texInf.y = 45.0f/256.0f;
		texSup.x = 95.5f/256.0f;
		texSup.y = 46.0f/256.0f;  // jaune
	}
	if ( type == PARTITRACK9 )  // win-3 ?
	{
		texInf.x = 64.5f/256.0f;
		texInf.y = 49.0f/256.0f;
		texSup.x = 95.5f/256.0f;
		texSup.y = 50.0f/256.0f;  // rouge
	}
	if ( type == PARTITRACK10 )  // win-4 ?
	{
		texInf.x = 64.5f/256.0f;
		texInf.y = 53.0f/256.0f;
		texSup.x = 95.5f/256.0f;
		texSup.y = 54.0f/256.0f;  // violet
	}
	if ( type == PARTITRACK11 )  // tir phazer ?
	{
		texInf.x = 64.5f/256.0f;
		texInf.y = 21.0f/256.0f;
		texSup.x = 95.5f/256.0f;
		texSup.y = 22.0f/256.0f;  // orange
	}
	if ( type == PARTITRACK12 )  // traînée réacteur ?
	{
		texInf.x = 64.5f/256.0f;
		texInf.y = 21.0f/256.0f;
		texSup.x = 95.5f/256.0f;
		texSup.y = 22.0f/256.0f;  // orange
	}
	if ( type == PARTITRACK13 )  // jet d'eau ?
	{
		texInf.x = 64.5f/256.0f;
		texInf.y = 57.0f/256.0f;
		texSup.x = 95.5f/256.0f;
		texSup.y = 58.0f/256.0f;  // bleu
	}

	h  = m_track[i].head;
	p1 = m_track[i].pos[h];
	f1 = m_track[i].intensity;

	eye = m_engine->RetEyePt();
	a = RotateAngle(eye.x-p1.x, eye.z-p1.z);

	for ( counter=0 ; counter<m_track[i].used-1 ; counter++ )
	{
		f2 = f1-(m_track[i].len[h]/lTotal);
		if ( f2 < 0.0f )  f2 = 0.0f;
		h --;  if ( h < 0 )  h = MAXTRACKLEN-1;
		p2 = m_track[i].pos[h];

		n = Normalize(p1-eye);

		p = p1;
		p.x += f1*m_track[i].width;
		rot = RotatePoint(FPOINT(p1.x, p1.z), a+PI/2.0f, FPOINT(p.x, p.z));
		corner[0].x = rot.x;
		corner[0].y = p1.y;
		corner[0].z = rot.y;
		rot = RotatePoint(FPOINT(p1.x, p1.z), a-PI/2.0f, FPOINT(p.x, p.z));
		corner[1].x = rot.x;
		corner[1].y = p1.y;
		corner[1].z = rot.y;

		p = p2;
		p.x += f2*m_track[i].width;
		rot = RotatePoint(FPOINT(p2.x, p2.z), a+PI/2.0f, FPOINT(p.x, p.z));
		corner[2].x = rot.x;
		corner[2].y = p2.y;
		corner[2].z = rot.y;
		rot = RotatePoint(FPOINT(p2.x, p2.z), a-PI/2.0f, FPOINT(p.x, p.z));
		corner[3].x = rot.x;
		corner[3].y = p2.y;
		corner[3].z = rot.y;

		if ( p2.y < p1.y )
		{
			vertex[0] = D3DVERTEX2(corner[1], n, texSup.x, texSup.y);
			vertex[1] = D3DVERTEX2(corner[0], n, texInf.x, texSup.y);
			vertex[2] = D3DVERTEX2(corner[3], n, texSup.x, texInf.y);
			vertex[3] = D3DVERTEX2(corner[2], n, texInf.x, texInf.y);
		}
		else
		{
			vertex[0] = D3DVERTEX2(corner[0], n, texSup.x, texSup.y);
			vertex[1] = D3DVERTEX2(corner[1], n, texInf.x, texSup.y);
			vertex[2] = D3DVERTEX2(corner[2], n, texSup.x, texInf.y);
			vertex[3] = D3DVERTEX2(corner[3], n, texInf.x, texInf.y);
		}

		m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 4, NULL);
		m_engine->AddStatisticTriangle(2);

		if ( f2 < 0.0f )  break;
		f1 = f2;
		p1 = p2;
	}
}


// Dessine une particule triangulaire.

void CParticule::DrawParticuleTriangle(int i)
{
	D3DMATRIX		matrix;
	D3DVECTOR		pos, angle;

	if ( m_particule[i].zoom == 0.0f )  return;

	pos = m_particule[i].pos;

	switch ( i%6 )
	{
		case 0:
			angle.x = 0.0f;
			angle.y = m_particule[i].angle;
			angle.z = 0.0f;
			break;
		case 1:
			angle.x = 0.0f;
			angle.y = -m_particule[i].angle;
			angle.z = 0.0f;
			break;
		case 2:
			angle.x = m_particule[i].angle;
			angle.y = 0.0f,
			angle.z = 0.0f;
			break;
		case 3:
			angle.x = -m_particule[i].angle;
			angle.y = 0.0f,
			angle.z = 0.0f;
			break;
		case 4:
			angle.x = 0.0f;
			angle.y = 0.0f;
			angle.z = m_particule[i].angle;
			break;
		case 5:
			angle.x = 0.0f;
			angle.y = 0.0f;
			angle.z = -m_particule[i].angle;
			break;
	}

	MatRotateXZY(matrix, angle);
	matrix._41 = pos.x;
	matrix._42 = pos.y;
	matrix._43 = pos.z;
	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &matrix);

	m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST, D3DFVF_VERTEX2,
								m_triangle[i].triangle, 3, NULL);
	m_engine->AddStatisticTriangle(1);
}

// Dessine une particule normale.

void CParticule::DrawParticuleNorm(int i)
{
	D3DVERTEX2		vertex[4];	// 2 triangles
	D3DMATRIX		matrix;
	D3DVECTOR		corner[4], eye, pos, n, angle;
	FPOINT			dim;
	float			zoom;

	zoom = m_particule[i].zoom;
	if ( !m_engine->RetStateColor() && m_particule[i].intensity < 0.5f )
	{
		zoom *= m_particule[i].intensity/0.5f;
	}

	if ( zoom == 0.0f )  return;
	if ( m_particule[i].intensity == 0.0f )  return;

	if ( m_particule[i].sheet == SH_INTERFACE )
	{
		pos = m_particule[i].pos;

		n = D3DVECTOR(0.0f, 0.0f, -1.0f);

		dim.x = m_particule[i].dim.x * zoom;
		dim.y = m_particule[i].dim.y * zoom;

		corner[0].x = pos.x+dim.x;
		corner[0].y = pos.y+dim.y;
		corner[0].z = 0.0f;

		corner[1].x = pos.x-dim.x;
		corner[1].y = pos.y+dim.y;
		corner[1].z = 0.0f;

		corner[2].x = pos.x+dim.x;
		corner[2].y = pos.y-dim.y;
		corner[2].z = 0.0f;

		corner[3].x = pos.x-dim.x;
		corner[3].y = pos.y-dim.y;
		corner[3].z = 0.0f;

		vertex[0] = D3DVERTEX2(corner[1], n, m_particule[i].texSup.x, m_particule[i].texSup.y);
		vertex[1] = D3DVERTEX2(corner[0], n, m_particule[i].texInf.x, m_particule[i].texSup.y);
		vertex[2] = D3DVERTEX2(corner[3], n, m_particule[i].texSup.x, m_particule[i].texInf.y);
		vertex[3] = D3DVERTEX2(corner[2], n, m_particule[i].texInf.x, m_particule[i].texInf.y);

		m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 4, NULL);
		m_engine->AddStatisticTriangle(2);
	}
	else
	{
		eye = m_engine->RetEyePt();
		pos = m_particule[i].pos;

		angle.x = -RotateAngle(Length2d(pos, eye), pos.y-eye.y);
		angle.y = RotateAngle(pos.z-eye.z, pos.x-eye.x);
		angle.z = m_particule[i].angle;

		MatRotateXZY(matrix, angle);
		matrix._41 = pos.x;
		matrix._42 = pos.y;
		matrix._43 = pos.z;
		m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &matrix);

		n = D3DVECTOR(0.0f, 0.0f, -1.0f);

		dim.x = m_particule[i].dim.x * zoom;
		dim.y = m_particule[i].dim.y * zoom;

		corner[0].x =  dim.x;
		corner[0].y =  dim.y;
		corner[0].z =  0.0f;

		corner[1].x = -dim.x;
		corner[1].y =  dim.y;
		corner[1].z =  0.0f;

		corner[2].x =  dim.x;
		corner[2].y = -dim.y;
		corner[2].z =  0.0f;

		corner[3].x = -dim.x;
		corner[3].y = -dim.y;
		corner[3].z =  0.0f;

		vertex[0] = D3DVERTEX2(corner[1], n, m_particule[i].texSup.x, m_particule[i].texSup.y);
		vertex[1] = D3DVERTEX2(corner[0], n, m_particule[i].texInf.x, m_particule[i].texSup.y);
		vertex[2] = D3DVERTEX2(corner[3], n, m_particule[i].texSup.x, m_particule[i].texInf.y);
		vertex[3] = D3DVERTEX2(corner[2], n, m_particule[i].texInf.x, m_particule[i].texInf.y);

		m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 4, NULL);
		m_engine->AddStatisticTriangle(2);
	}
}

// Dessine une particule plate (horizontale).

void CParticule::DrawParticuleFlat(int i)
{
	D3DVERTEX2		vertex[4];	// 2 triangles
	D3DMATRIX		matrix;
	D3DVECTOR		corner[4], pos, n, angle, eye;
	FPOINT			dim;

	if ( m_particule[i].zoom == 0.0f )  return;
	if ( m_particule[i].intensity == 0.0f )  return;

	pos = m_particule[i].pos;

	angle.x = PI/2.0f;
	angle.y = 0.0f;
	angle.z = m_particule[i].angle;

#if 0
	if ( m_engine->RetRankView() == 1 )  // sous l'eau ?
	{
		angle.x = -PI/2.0f;
		pos.y -= 1.0f;
	}
#else
	if ( m_engine->RetRankView() == 1 )  // sous l'eau ?
	{
		pos.y -= 1.0f;
	}

	eye = m_engine->RetEyePt();
	if ( pos.y > eye.y )  // vu par en-dessous ?
	{
		angle.x = -PI/2.0f;
	}
#endif

	MatRotateXZY(matrix, angle);
	matrix._41 = pos.x;
	matrix._42 = pos.y;
	matrix._43 = pos.z;
	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &matrix);

	n = D3DVECTOR(0.0f, 0.0f, -1.0f);

	dim.x = m_particule[i].dim.x * m_particule[i].zoom;
	dim.y = m_particule[i].dim.y * m_particule[i].zoom;

	corner[0].x =  dim.x;
	corner[0].y =  dim.y;
	corner[0].z =  0.0f;

	corner[1].x = -dim.x;
	corner[1].y =  dim.y;
	corner[1].z =  0.0f;

	corner[2].x =  dim.x;
	corner[2].y = -dim.y;
	corner[2].z =  0.0f;

	corner[3].x = -dim.x;
	corner[3].y = -dim.y;
	corner[3].z =  0.0f;

	vertex[0] = D3DVERTEX2(corner[1], n, m_particule[i].texSup.x, m_particule[i].texSup.y);
	vertex[1] = D3DVERTEX2(corner[0], n, m_particule[i].texInf.x, m_particule[i].texSup.y);
	vertex[2] = D3DVERTEX2(corner[3], n, m_particule[i].texSup.x, m_particule[i].texInf.y);
	vertex[3] = D3DVERTEX2(corner[2], n, m_particule[i].texInf.x, m_particule[i].texInf.y);

	m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 4, NULL);
	m_engine->AddStatisticTriangle(2);
}

// Dessine une particule plate pour faire une nappe de brouillard.

void CParticule::DrawParticuleFog(int i)
{
	D3DVERTEX2		vertex[4];	// 2 triangles
	D3DMATRIX		matrix;
	D3DVECTOR		corner[4], pos, n, angle, eye;
	FPOINT			dim, zoom;

	if ( m_particule[i].intensity == 0.0f )  return;

	pos = m_particule[i].pos;

	dim.x = m_particule[i].dim.x;
	dim.y = m_particule[i].dim.y;

	if ( m_particule[i].type == PARTIFOG0 ||
		 m_particule[i].type == PARTIFOG2 ||
		 m_particule[i].type == PARTIFOG4 ||
		 m_particule[i].type == PARTIFOG6 )
	{
//?		pos.x += sinf(m_particule[i].zoom*1.2f)*dim.x*0.1f;
//?		pos.y += cosf(m_particule[i].zoom*1.5f)*dim.y*0.1f;
		zoom.x = 1.0f+sinf(m_particule[i].zoom*2.0f)/6.0f;
		zoom.y = 1.0f+cosf(m_particule[i].zoom*2.7f)/6.0f;
	}
	if ( m_particule[i].type == PARTIFOG1 ||
		 m_particule[i].type == PARTIFOG3 ||
		 m_particule[i].type == PARTIFOG5 ||
		 m_particule[i].type == PARTIFOG7 )
	{
//?		pos.x += sinf(m_particule[i].zoom*1.0f)*dim.x*0.1f;
//?		pos.y += cosf(m_particule[i].zoom*1.3f)*dim.y*0.1f;
		zoom.x = 1.0f+sinf(m_particule[i].zoom*3.0f)/6.0f;
		zoom.y = 1.0f+cosf(m_particule[i].zoom*3.7f)/6.0f;
	}

	dim.x *= zoom.x;
	dim.y *= zoom.y;

	angle.x = PI/2.0f;
	angle.y = 0.0f;
	angle.z = m_particule[i].angle;

	if ( m_engine->RetRankView() == 1 )  // sous l'eau ?
	{
		pos.y -= 1.0f;
	}

	eye = m_engine->RetEyePt();
	if ( pos.y > eye.y )  // vu par en-dessous ?
	{
		angle.x = -PI/2.0f;
	}

	MatRotateXZY(matrix, angle);
	matrix._41 = pos.x;
	matrix._42 = pos.y;
	matrix._43 = pos.z;
	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &matrix);

	n = D3DVECTOR(0.0f, 0.0f, -1.0f);

	corner[0].x =  dim.x;
	corner[0].y =  dim.y;
	corner[0].z =  0.0f;

	corner[1].x = -dim.x;
	corner[1].y =  dim.y;
	corner[1].z =  0.0f;

	corner[2].x =  dim.x;
	corner[2].y = -dim.y;
	corner[2].z =  0.0f;

	corner[3].x = -dim.x;
	corner[3].y = -dim.y;
	corner[3].z =  0.0f;

	vertex[0] = D3DVERTEX2(corner[1], n, m_particule[i].texSup.x, m_particule[i].texSup.y);
	vertex[1] = D3DVERTEX2(corner[0], n, m_particule[i].texInf.x, m_particule[i].texSup.y);
	vertex[2] = D3DVERTEX2(corner[3], n, m_particule[i].texSup.x, m_particule[i].texInf.y);
	vertex[3] = D3DVERTEX2(corner[2], n, m_particule[i].texInf.x, m_particule[i].texInf.y);

	m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 4, NULL);
	m_engine->AddStatisticTriangle(2);
}

// Dessine une particule sous forme de rayon.

void CParticule::DrawParticuleRay(int i)
{
	D3DVERTEX2		vertex[4];	// 2 triangles
	D3DMATRIX		matrix;
	D3DVECTOR		corner[4], eye, pos, goal, n, angle, proj;
	FPOINT			dim, texInf, texSup;
	BOOL			bLeft;
	float			a, len, adv, prop, vario1, vario2;
	int				r, rank, step, first, last;

	if ( m_particule[i].zoom == 0.0f )  return;
	if ( m_particule[i].intensity == 0.0f )  return;

	eye = m_engine->RetEyePt();
	pos = m_particule[i].pos;
	goal = m_particule[i].goal;

	a = RotateAngle(FPOINT(pos.x,pos.z), FPOINT(goal.x,goal.z), FPOINT(eye.x,eye.z));
	bLeft = (a < PI);

	proj = Projection(pos, goal, eye);
	angle.x = -RotateAngle(Length2d(proj, eye), proj.y-eye.y);
	angle.y = RotateAngle(pos.z-goal.z, pos.x-goal.x)+PI/2.0f;
	angle.z = -RotateAngle(Length2d(pos, goal), pos.y-goal.y);
	if ( bLeft )  angle.x = -angle.x;

	MatRotateZXY(matrix, angle);
	matrix._41 = pos.x;
	matrix._42 = pos.y;
	matrix._43 = pos.z;
	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &matrix);

	n = D3DVECTOR(0.0f, 0.0f, bLeft?1.0f:-1.0f);

	dim.x = m_particule[i].dim.x * m_particule[i].zoom;
	dim.y = m_particule[i].dim.y * m_particule[i].zoom;

	if ( bLeft )  dim.y = -dim.y;

	len = Length(pos, goal);
	adv = 0.0f;

	step = (int)(len/(dim.x*2.0f))+1;

	if ( step == 1 )
	{
		vario1 = 1.0f;
		vario2 = 1.0f;
	}
	else
	{
		vario1 = 0.0f;
		vario2 = 2.0f;
	}

	if ( m_particule[i].type == PARTIRAY2 )
	{
		first = 0;
		last  = step;
		vario1 = 0.0f;
		vario2 = 0.0f;
	}
	else if ( m_particule[i].type == PARTIRAY3 )
	{
		if ( m_particule[i].time < m_particule[i].duration*0.40f )
		{
			prop = m_particule[i].time / (m_particule[i].duration*0.40f);
			first = 0;
			last  = (int)(prop*step);
		}
		else if ( m_particule[i].time < m_particule[i].duration*0.60f )
		{
			first = 0;
			last  = step;
		}
		else
		{
			prop = (m_particule[i].time-m_particule[i].duration*0.60f) / (m_particule[i].duration*0.40f);
			first = (int)(prop*step);
			last  = step;
		}
	}
	else
	{
		if ( m_particule[i].time < m_particule[i].duration*0.50f )
		{
			prop = m_particule[i].time / (m_particule[i].duration*0.50f);
			first = 0;
			last  = (int)(prop*step);
		}
		else if ( m_particule[i].time < m_particule[i].duration*0.75f )
		{
			first = 0;
			last  = step;
		}
		else
		{
			prop = (m_particule[i].time-m_particule[i].duration*0.75f) / (m_particule[i].duration*0.25f);
			first = (int)(prop*step);
			last  = step;
		}
	}

	dim.y *= 0.5f;
	corner[0].x = adv;
	corner[2].x = adv;
	corner[0].y =  dim.y;
	corner[2].y = -dim.y;
	corner[0].z = (Rand()-0.5f)*vario1;
	corner[1].z = (Rand()-0.5f)*vario1;
	corner[2].z = (Rand()-0.5f)*vario1;
	corner[3].z = (Rand()-0.5f)*vario1;

	for ( rank=0 ; rank<step ; rank++ )
	{
		corner[1].x = corner[0].x;
		corner[3].x = corner[2].x;
		corner[0].x = adv+dim.x*2.0f+(Rand()-0.5f)*vario2;
		corner[2].x = adv+dim.x*2.0f+(Rand()-0.5f)*vario2;

		corner[1].y = corner[0].y;
		corner[3].y = corner[2].y;
		corner[0].y =  dim.y+(Rand()-0.5f)*vario2;
		corner[2].y = -dim.y+(Rand()-0.5f)*vario2;

		if ( rank >= first && rank <= last )
		{
#if 1
			texInf = m_particule[i].texInf;
			texSup = m_particule[i].texSup;

			r = rand()%16;
			texInf.x += 0.25f*(r/4);
			texSup.x += 0.25f*(r/4);
			if ( r%2 < 1 && adv > 0.0f && m_particule[i].type != PARTIRAY1 )
			{
				Swap(texInf.x, texSup.x);
			}
			if ( r%4 < 2 )
			{
				Swap(texInf.y, texSup.y);
			}
#else
			texInf.x = Mod(texInf.x+0.25f, 1.0f);
			texSup.x = Mod(texSup.x+0.25f, 1.0f);
#endif

			vertex[0] = D3DVERTEX2(corner[1], n, texSup.x, texSup.y);
			vertex[1] = D3DVERTEX2(corner[0], n, texInf.x, texSup.y);
			vertex[2] = D3DVERTEX2(corner[3], n, texSup.x, texInf.y);
			vertex[3] = D3DVERTEX2(corner[2], n, texInf.x, texInf.y);

			m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 4, NULL);
			m_engine->AddStatisticTriangle(2);
		}
		adv += dim.x*2.0f;
	}
}

// Dessine une particule sphérique.

void CParticule::DrawParticuleSphere(int i)
{
	D3DVERTEX2		vertex[2*16*(16+1)];  // triangles
	D3DMATRIX		matrix, rot;
	D3DVECTOR		pos, angle, v0, v1;
	FPOINT			ts, ti;
	float			zoom, deltaRingAngle, deltaSegAngle;
	float			r0,r1, tu0,tv0, tu1,tv1;
	int				j, ring, seg, numRings, numSegments;

	zoom = m_particule[i].zoom;
#if 0
	if ( !m_engine->RetStateColor() && m_particule[i].intensity < 0.5f )
	{
		zoom *= m_particule[i].intensity/0.5f;
	}
#endif

	if ( zoom == 0.0f )  return;

	m_engine->SetState(D3DSTATETTb|D3DSTATE2FACE|D3DSTATEWRAP, RetColor(m_particule[i].intensity));

	pos = m_particule[i].pos;

	D3DUtil_SetIdentityMatrix(matrix);
	matrix._11 = zoom;
	matrix._22 = zoom;
	matrix._33 = zoom;
	matrix._41 = pos.x;
	matrix._42 = pos.y;
	matrix._43 = pos.z;

	if ( m_particule[i].angle != 0.0f )
	{
		angle.x = m_particule[i].angle*0.4f;
		angle.y = m_particule[i].angle*1.0f;
		angle.z = m_particule[i].angle*0.7f;
		MatRotateZXY(rot, angle);
		matrix = rot*matrix;
	}

	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &matrix);

	ts.x = m_particule[i].texSup.x;
	ts.y = m_particule[i].texSup.y;
	ti.x = m_particule[i].texInf.x;
	ti.y = m_particule[i].texInf.y;

	// Choose a tesselation level.
	if ( m_particule[i].type == PARTISPHERE3 ||
		 m_particule[i].type == PARTISPHERE5 )
	{
		numRings	= 16;
		numSegments = 16;
	}
	else
	{
		numRings	= 8;
		numSegments = 10;
	}

	// Establish constants used in sphere generation.
	deltaRingAngle = PI/numRings;
	deltaSegAngle  = 2.0f*PI/numSegments;

	// Generate the group of rings for the sphere.
	j = 0;
	for ( ring=0 ; ring<numRings ; ring++ )
	{	 
		r0   = sinf((ring+0)*deltaRingAngle);
		r1   = sinf((ring+1)*deltaRingAngle);
		v0.y = cosf((ring+0)*deltaRingAngle);
		v1.y = cosf((ring+1)*deltaRingAngle);

		tv0 = (ring+0)/(float)numRings;
		tv1 = (ring+1)/(float)numRings;
		tv0 = ts.y+(ti.y-ts.y)*tv0;
		tv1 = ts.y+(ti.y-ts.y)*tv1;

		// Generate the group of segments for the current ring.
		for ( seg=0 ; seg<numSegments+1 ; seg++ )
		{
			v0.x = r0*sinf(seg*deltaSegAngle);
			v0.z = r0*cosf(seg*deltaSegAngle);
			v1.x = r1*sinf(seg*deltaSegAngle);
			v1.z = r1*cosf(seg*deltaSegAngle);

			// Add two vertices to the strip which makes up the sphere.
			tu0 = ((float)seg)/numSegments;
			tu0 = ts.x+(ti.x-ts.x)*tu0;
			tu1 = tu0;

			vertex[j++] = D3DVERTEX2(v0,v0, tu0,tv0);
			vertex[j++] = D3DVERTEX2(v1,v1, tu1,tv1);
		}
	}
	
	m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, j, NULL);
	m_engine->AddStatisticTriangle(j);

	m_engine->SetState(D3DSTATETTb, RetColor(m_particule[i].intensity));
}

// Retourne la hauteur en fonction de la progression.

float ProgressCylinder(float progress)
{
	if ( progress < 0.5f )
	{
		return 1.0f - (powf(1.0f-progress*2.0f, 2.0f));
	}
	else
	{
		return 1.0f - (powf(progress*2.0f-1.0f, 2.0f));
	}
}

// Dessine une particule cylindrique.

void CParticule::DrawParticuleCylinder(int i)
{
	D3DVERTEX2		vertex[2*5*(10+1)];  // triangles
	D3DMATRIX		matrix, rot;
	D3DVECTOR		angle, v0, v1;
	FPOINT			ts, ti;
	float			progress, zoom, diam, deltaSegAngle, h[6], d[6];
	float			r0,r1, tu0,tv0, tu1,tv1, p1, p2, pp;
	int				j, ring, seg, numRings, numSegments;

	progress = m_particule[i].zoom;
	zoom = m_particule[i].dim.x;
	diam = m_particule[i].dim.y;
	if ( progress >= 1.0f || zoom == 0.0f )  return;

	m_engine->SetState(D3DSTATETTb|D3DSTATE2FACE|D3DSTATEWRAP, RetColor(m_particule[i].intensity));

	D3DUtil_SetIdentityMatrix(matrix);
	matrix._11 = zoom;
	matrix._22 = zoom;
	matrix._33 = zoom;
	matrix._41 = m_particule[i].pos.x;
	matrix._42 = m_particule[i].pos.y;
	matrix._43 = m_particule[i].pos.z;

	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &matrix);

	ts.x = m_particule[i].texSup.x;
	ts.y = m_particule[i].texSup.y;
	ti.x = m_particule[i].texInf.x;
	ti.y = m_particule[i].texInf.y;

	numRings = 5;
	numSegments = 10;
	deltaSegAngle  = 2.0f*PI/numSegments;

	if ( m_particule[i].type == PARTIPLOUF0 )
	{
#if 0
		if ( progress <= 0.5f )
		{
			p1 = progress/0.5f;  // avant
			p2 = 0.0f;  // arrière
		}
		else
		{
			p1 = 1.0f;  // avant
			p2 = (progress-0.5f)/0.5f;  // arrière
			ts.y += (ti.y-ts.y)*p2;
		}
#else
		p1 = progress;  // avant
		p2 = powf(progress, 5.0f);  // arrière
#endif

		for ( ring=0 ; ring<=numRings ; ring++ )
		{
			pp = p2+(p1-p2)*((float)ring/numRings);
			d[ring] = diam/zoom+pp*2.0f;
			h[ring] = ProgressCylinder(pp);
		}
	}

	j = 0;
	for ( ring=0 ; ring<numRings ; ring++ )
	{
		r0   = 1.0f*d[ring+0];  // rayon à la base
		r1   = 1.0f*d[ring+1];  // rayon en haut
		v0.y = 1.0f*h[ring+0];  // bas
		v1.y = 1.0f*h[ring+1];  // haut

		tv0 = 1.0f-(ring+0)*(1.0f/numRings);
		tv1 = 1.0f-(ring+1)*(1.0f/numRings);
		tv0 = ts.y+(ti.y-ts.y)*tv0;
		tv1 = ts.y+(ti.y-ts.y)*tv1;

		for ( seg=0 ; seg<numSegments+1 ; seg++ )
		{
			v0.x = r0*sinf(seg*deltaSegAngle);
			v0.z = r0*cosf(seg*deltaSegAngle);
			v1.x = r1*sinf(seg*deltaSegAngle);
			v1.z = r1*cosf(seg*deltaSegAngle);

//?			tu0 = ((float)seg)/numSegments;
			tu0 = (seg%2)?0.0f:1.0f;
			tu0 = ts.x+(ti.x-ts.x)*tu0;
			tu1 = tu0;

			vertex[j++] = D3DVERTEX2(v0,v0, tu0,tv0);
			vertex[j++] = D3DVERTEX2(v1,v1, tu1,tv1);
		}
	}
	
	m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, j, NULL);
	m_engine->AddStatisticTriangle(j);

	m_engine->SetState(D3DSTATETTb, RetColor(m_particule[i].intensity));
}

// Dessine une trace de pneu.

void CParticule::DrawParticuleWheel(int i)
{
	D3DVECTOR	pos[4], center;
	D3DVERTEX2	vertex[4];	// 2 triangles
	D3DVECTOR	n;
	FPOINT		ts, ti;
	float		dist, dp, time, zoom;

	dist = Length2d(m_engine->RetEyePt(), m_wheelTrace[i].pos[0]);
	if ( dist > 300.0f )  return;

	pos[0] = m_wheelTrace[i].pos[0];
	pos[1] = m_wheelTrace[i].pos[1];
	pos[2] = m_wheelTrace[i].pos[2];
	pos[3] = m_wheelTrace[i].pos[3];

	if ( m_wheelTrace[i].type == PARTITRACE1 )  // pneu freinage ?
	{
		ts.x = 0.000f;
		ts.y = 0.500f;
		ti.x = ts.x+0.125f;
		ti.y = ts.y+0.250f;
	}
	else if ( m_wheelTrace[i].type == PARTITRACE2 )  // pneu accélération ?
	{
		ts.x = 0.250f;
		ts.y = 0.500f;
		ti.x = ts.x+0.125f;
		ti.y = ts.y+0.250f;
	}
	else if ( m_wheelTrace[i].type == PARTITRACE3 )  // explosion ?
	{
		ts.x = 0.000f;
		ts.y = 0.250f;
		ti.x = ts.x+0.250f;
		ti.y = ts.y+0.250f;
	}
	else if ( m_wheelTrace[i].type == PARTITRACE4 )  // huile ?
	{
		ts.x = 0.250f;
		ts.y = 0.250f;
		ti.x = ts.x+0.250f;
		ti.y = ts.y+0.250f;

		time = m_absTime - m_wheelTrace[i].startTime;
		if ( time < 5.0f )
		{
			zoom = powf(time/5.0f, 0.5f);
			center = (pos[0]+pos[1]+pos[2]+pos[3])/4.0f;
			pos[0] = center+(pos[0]-center)*zoom;
			pos[1] = center+(pos[1]-center)*zoom;
			pos[2] = center+(pos[2]-center)*zoom;
			pos[3] = center+(pos[3]-center)*zoom;
		}
	}
	else if ( m_wheelTrace[i].type == PARTITRACE5 )  // radioactif ?
	{
		ts.x = 0.500f;
		ts.y = 0.250f;
		ti.x = ts.x+0.250f;
		ti.y = ts.y+0.250f;
	}
	else if ( m_wheelTrace[i].type == PARTITRACE6 )  // pneu tout-terrain ?
	{
		ts.x = 0.125f;
		ts.y = 0.500f;
		ti.x = ts.x+0.125f;
		ti.y = ts.y+0.250f;
	}
	else if ( m_wheelTrace[i].type == PARTITRACE7 )  // jante ?
	{
		ts.x = 0.375f;
		ts.y = 0.500f;
		ti.x = ts.x+0.125f;
		ti.y = ts.y+0.250f;
	}
	else if ( m_wheelTrace[i].type == PARTITRACE8 )  // goto ?
	{
	}
	else
	{
		return;
	}

	dp = (1.0f/256.0f)/2.0f;
	ts.x = ts.x+dp;
	ts.y = ts.y+dp;
	ti.x = ti.x-dp;
	ti.y = ti.y-dp;

	n = D3DVECTOR(0.0f, 1.0f, 0.0f);

	vertex[0] = D3DVERTEX2(pos[0], n, ts.x, ts.y);
	vertex[1] = D3DVERTEX2(pos[1], n, ti.x, ts.y);
	vertex[2] = D3DVERTEX2(pos[2], n, ts.x, ti.y);
	vertex[3] = D3DVERTEX2(pos[3], n, ti.x, ti.y);

	m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 4, NULL);
	m_engine->AddStatisticTriangle(2);
}

// Dessine une rayon de soleil.
// Le rayon est le plus visible si on regarde contre le soleil,
// c'est-à-dire au sud-est. On est face au soleil lorsque
// RetEyeDirH() = PI*0.75 (135 degrés).

void CParticule::DrawParticuleSunbeam(int i)
{
	D3DVECTOR	pos[4], center;
	D3DVERTEX2	vertex[4];	// 2 triangles
	D3DVECTOR	n;
	FPOINT		ts, ti, rot;
	float		dist, eyeDirH, angle, intensity, dim, dp;

	dist = Length2d(m_engine->RetEyePt(), m_sunbeam[i].pos);
	if ( dist < 30.0f )  return;

	eyeDirH = m_engine->RetEyeDirH();

	angle = NormAngle(eyeDirH+m_sunbeam[i].angle);
	if ( angle <= PI*0.25f || angle >= PI*1.25f )  return;
	angle -= PI*0.25f;
	intensity = sinf(angle);
	intensity *= Norm((dist-30.0f)/30.0f);  // moins visible si proche
	intensity *= m_sunbeamIntensity;
	m_engine->SetState(D3DSTATETTb, RetColor(intensity));

	dim = m_sunbeam[i].dim;

	rot = RotatePoint(eyeDirH-PI/2.0f, dim);
	pos[1].x = m_sunbeam[i].pos.x+rot.x;
	pos[1].y = -16.0f;
	pos[1].z = m_sunbeam[i].pos.z+rot.y;
	pos[3].x = m_sunbeam[i].pos.x+rot.x;
	pos[3].y = 100.0f;
	pos[3].z = m_sunbeam[i].pos.z+rot.y;

	rot = RotatePoint(eyeDirH+PI/2.0f, dim);
	pos[0].x = m_sunbeam[i].pos.x+rot.x;
	pos[0].y = -16.0f;
	pos[0].z = m_sunbeam[i].pos.z+rot.y;
	pos[2].x = m_sunbeam[i].pos.x+rot.x;
	pos[2].y = 100.0f;
	pos[2].z = m_sunbeam[i].pos.z+rot.y;

	if ( m_sunbeam[i].type == PARTISUNBEAM0 )
	{
		ts.x =  96.0f/256.0f;
		ts.y =  32.0f/256.0f;
		ti.x = 128.0f/256.0f;
		ti.y =  64.0f/256.0f;
	}
	else if ( m_sunbeam[i].type == PARTISUNBEAM1 )
	{
		ts.x = 128.0f/256.0f;
		ts.y =  32.0f/256.0f;
		ti.x = 160.0f/256.0f;
		ti.y =  64.0f/256.0f;
	}
	else if ( m_sunbeam[i].type == PARTISUNBEAM2 )
	{
		ts.x =  96.0f/256.0f;
		ts.y = 192.0f/256.0f;
		ti.x = 128.0f/256.0f;
		ti.y = 224.0f/256.0f;
	}
	else if ( m_sunbeam[i].type == PARTISUNBEAM3 )
	{
		ts.x = 128.0f/256.0f;
		ts.y = 192.0f/256.0f;
		ti.x = 160.0f/256.0f;
		ti.y = 224.0f/256.0f;
	}
	else if ( m_sunbeam[i].type == PARTISUNBEAM4 )
	{
		ts.x =  64.0f/256.0f;
		ts.y =   0.0f/256.0f;
		ti.x =  96.0f/256.0f;
		ti.y =   4.0f/256.0f;
	}
	else
	{
		return;
	}

	dp = (1.0f/256.0f)/2.0f;
	ts.x = ts.x+dp;
	ts.y = ts.y+dp;
	ti.x = ti.x-dp;
	ti.y = ti.y-dp;

	rot = RotatePoint(eyeDirH, 1.0f);
	n = D3DVECTOR(rot.x, 0.0f, rot.y);

	vertex[0] = D3DVERTEX2(pos[0], n, ts.x, ti.y);
	vertex[1] = D3DVERTEX2(pos[1], n, ti.x, ti.y);
	vertex[2] = D3DVERTEX2(pos[2], n, ts.x, ts.y);
	vertex[3] = D3DVERTEX2(pos[3], n, ti.x, ts.y);

	m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 4, NULL);
	m_engine->AddStatisticTriangle(2);
}

// Dessine un goto.

void CParticule::DrawParticuleGoto()
{
	D3DVECTOR	pos[4], center;
	D3DVERTEX2	vertex[4];	// 2 triangles
	D3DVECTOR	n;
	FPOINT		ts, ti;
	float		dp, zoom;

	zoom = (1.0f-powf(m_gotoProgress, 4.0f))*4.0f;

	pos[0].x = m_gotoPos.x-zoom;
	pos[0].y = m_gotoPos.y;
	pos[0].z = m_gotoPos.z+zoom;

	pos[1].x = m_gotoPos.x+zoom;
	pos[1].y = m_gotoPos.y;
	pos[1].z = m_gotoPos.z+zoom;

	pos[2].x = m_gotoPos.x-zoom;
	pos[2].y = m_gotoPos.y;
	pos[2].z = m_gotoPos.z-zoom;

	pos[3].x = m_gotoPos.x+zoom;
	pos[3].y = m_gotoPos.y;
	pos[3].z = m_gotoPos.z-zoom;

	n = D3DVECTOR(0.0f, 1.0f, 0.0f);

	dp = (1.0f/256.0f)/2.0f;

	m_engine->SetTexture("effect03.tga");
	m_engine->SetState(D3DSTATETTb);

	ts.x = 0.500f;
	ts.y = 0.000f;
	ti.x = ts.x+0.250f;
	ti.y = ts.y+0.250f;
	ts.x = ts.x+dp;
	ts.y = ts.y+dp;
	ti.x = ti.x-dp;
	ti.y = ti.y-dp;

	vertex[0] = D3DVERTEX2(pos[0], n, ts.x, ts.y);
	vertex[1] = D3DVERTEX2(pos[1], n, ti.x, ts.y);
	vertex[2] = D3DVERTEX2(pos[2], n, ts.x, ti.y);
	vertex[3] = D3DVERTEX2(pos[3], n, ti.x, ti.y);

	m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 4, NULL);
	m_engine->AddStatisticTriangle(2);

	m_engine->SetState(D3DSTATETTw);

	ts.x = 0.750f;
	ts.y = 0.000f;
	ti.x = ts.x+0.250f;
	ti.y = ts.y+0.250f;
	ts.x = ts.x+dp;
	ts.y = ts.y+dp;
	ti.x = ti.x-dp;
	ti.y = ti.y-dp;

	vertex[0] = D3DVERTEX2(pos[0], n, ts.x, ts.y);
	vertex[1] = D3DVERTEX2(pos[1], n, ti.x, ts.y);
	vertex[2] = D3DVERTEX2(pos[2], n, ts.x, ti.y);
	vertex[3] = D3DVERTEX2(pos[3], n, ti.x, ti.y);

	m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 4, NULL);
	m_engine->AddStatisticTriangle(2);
}

// Dessine toutes les particules.

void CParticule::DrawParticule(int sheet)
{
	D3DMATERIAL7	mat;
	D3DMATRIX		matrix;
	BOOL			bLoadTexture;
	char			name[20];
	int				state, t, i, j, r;

	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_AMBIENT, 0xffffffff);
	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, TRUE);
//?	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE);
	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);

	// Dessine les particules à base de triangles.
	if ( m_totalInterface[0][sheet] > 0 )
	{
		for ( i=0 ; i<MAXPARTICULE ; i++ )
		{
			if ( !m_particule[i].bUsed )  continue;
			if ( m_particule[i].sheet != sheet )  continue;
			if ( m_particule[i].type == PARTIPART )  continue;

			m_engine->SetTexture(m_triangle[i].texName1);
			m_engine->SetMaterial(m_triangle[i].material);
			m_engine->SetState(m_triangle[i].state);
			DrawParticuleTriangle(i);
		}
	}

	// Dessines les particules à base de carrés calculés.
	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE);

	ZeroMemory( &mat, sizeof(D3DMATERIAL7) );
	mat.diffuse.r = 1.0f;
	mat.diffuse.g = 1.0f;
	mat.diffuse.b = 1.0f;  // blanc
	mat.ambient.r = 0.5f;
	mat.ambient.g = 0.5f;
	mat.ambient.b = 0.5f;
	m_engine->SetMaterial(mat);

	// Dessine les rayons de soleil.
	if ( sheet == SH_WORLD &&
		 m_engine->RetSetup(ST_SUNBEAM) != 0.0f )
	{
		m_engine->SetTexture("effect00.tga");
		m_engine->SetState(D3DSTATETTb);
		D3DUtil_SetIdentityMatrix(matrix);
		m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &matrix);
		for ( i=0 ; i<MAXSUNBEAM ; i++ )
		{
			if ( !m_sunbeam[i].bUsed )  continue;
			DrawParticuleSunbeam(i);
		}
	}

	// Dessine les traces de pneu.
	if ( m_wheelTraceTotal > 0 && sheet == SH_WORLD )
	{
		m_engine->SetTexture("effect03.tga");
		m_engine->SetState(D3DSTATETTw);
		D3DUtil_SetIdentityMatrix(matrix);
		m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &matrix);
		for ( i=0 ; i<m_wheelTraceTotal ; i++ )
		{
			DrawParticuleWheel(i);
		}
	}

	if ( m_bGoto && sheet == SH_WORLD )
	{
		D3DUtil_SetIdentityMatrix(matrix);
		m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &matrix);
		DrawParticuleGoto();
	}

//?	for ( t=1 ; t<MAXPARTITYPE ; t++ )
	for ( t=MAXPARTITYPE-1 ; t>=1 ; t-- )  // noir derrière !
	{
		if ( m_totalInterface[t][sheet] == 0 )  continue;

		bLoadTexture = FALSE;

		if ( t == 4 )  state = D3DSTATETTw;  // effect03.tga
		else           state = D3DSTATETTb;  // effect[00..02].tga
		m_engine->SetState(state);

		for ( j=0 ; j<MAXPARTICULE ; j++ )
		{
			i = MAXPARTICULE*t+j;
			if ( !m_particule[i].bUsed )  continue;
			if ( m_particule[i].sheet != sheet )  continue;

			if ( !bLoadTexture )
			{
				NameParticule(name, t);
				m_engine->SetTexture(name);
				bLoadTexture = TRUE;
			}

			r = m_particule[i].trackRank;
			if ( r != -1 )
			{
				m_engine->SetState(state);
				TrackDraw(r, m_particule[i].type);  // dessine la traînée
				if ( !m_track[r].bDrawParticule )  continue;
			}

			m_engine->SetState(state, RetColor(m_particule[i].intensity));

			if ( m_particule[i].bRay )  // rayon ?
			{
				DrawParticuleRay(i);
			}
			else if ( m_particule[i].type >= PARTIFLAT )
			{
				DrawParticuleFlat(i);
			}
			else if ( m_particule[i].type >= PARTIFOG0 &&
					  m_particule[i].type <= PARTIFOG9 )
			{
				DrawParticuleFog(i);
			}
			else if ( m_particule[i].type >= PARTISPHERE0 &&
					  m_particule[i].type <= PARTISPHERE9 )  // sphère ?
			{
				DrawParticuleSphere(i);
			}
			else if ( m_particule[i].type >= PARTIPLOUF0 &&
					  m_particule[i].type <= PARTIPLOUF4 )  // cylindre ?
			{
				DrawParticuleCylinder(i);
			}
			else	// normal ?
			{
				DrawParticuleNorm(i);
			}
		}
	}

//?	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE);
	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
}


// Fait entendre un son.

void CParticule::Play(Sound sound, D3DVECTOR pos,
					  float amplitude, float frequency)
{
	if ( m_sound == 0 )
	{
		m_sound = (CSound*)m_iMan->SearchInstance(CLASS_SOUND);
	}

	m_sound->Play(sound, pos, amplitude, frequency);
}



// Cherche la couleur si on est dans le brouillard.
// Retourne noir si on est pas dans le brouillard.

D3DCOLORVALUE CParticule::RetFogColor(D3DVECTOR pos)
{
	D3DCOLORVALUE	result, color;
	float			dist, factor;
	int				fog, i;

	result.r = 0.0f;
	result.g = 0.0f;
	result.b = 0.0f;
	result.a = 0.0f;

	for ( fog=0 ; fog<m_fogTotal ; fog++ )
	{
		i = m_fog[fog];  // i = rang de la particule

		if ( pos.y >= m_particule[i].pos.y+FOG_HSUP )  continue;
		if ( pos.y <= m_particule[i].pos.y-FOG_HINF )  continue;

		dist = Length2d(pos, m_particule[i].pos);
		if ( dist >= m_particule[i].dim.x*1.5f )  continue;

		// Calcule le facteur horizontal.
		factor = 1.0f-powf(dist/(m_particule[i].dim.x*1.5f), 4.0f);

		// Calcule le facteur vertical.
		if ( pos.y > m_particule[i].pos.y )
		{
			factor *= 1.0f-(pos.y-m_particule[i].pos.y)/FOG_HSUP;
		}
		else
		{
			factor *= 1.0f-(m_particule[i].pos.y-pos.y)/FOG_HINF;
		}

		factor *= 0.3f;

		if ( m_particule[i].type == PARTIFOG0 ||
			 m_particule[i].type == PARTIFOG1 )  // bleu ?
		{
			color.r = 0.0f;
			color.g = 0.5f;
			color.b = 1.0f;
		}
		else if ( m_particule[i].type == PARTIFOG2 ||
				  m_particule[i].type == PARTIFOG3 )  // rouge ?
		{
			color.r = 2.0f;
			color.g = 1.0f;
			color.b = 0.0f;
		}
		else if ( m_particule[i].type == PARTIFOG4 ||
				  m_particule[i].type == PARTIFOG5 )  // blanc ?
		{
			color.r = 1.0f;
			color.g = 1.0f;
			color.b = 1.0f;
		}
		else if ( m_particule[i].type == PARTIFOG6 ||
				  m_particule[i].type == PARTIFOG7 )  // jaune ?
		{
			color.r = 0.8f;
			color.g = 1.0f;
			color.b = 0.4f;
		}
		else
		{
			color.r = 0.0f;
			color.g = 0.0f;
			color.b = 0.0f;
		}

		result.r += color.r*factor;
		result.g += color.g*factor;
		result.b += color.b*factor;
	}

	if ( result.r > 0.6f )  result.r = 0.6f;
	if ( result.g > 0.6f )  result.g = 0.6f;
	if ( result.b > 0.6f )  result.b = 0.6f;

	return result;
}


