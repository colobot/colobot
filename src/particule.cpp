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

// particule.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "d3dmath.h"
#include "d3dtextr.h"
#include "d3dengine.h"
#include "d3dutil.h"
#include "language.h"
#include "iman.h"
#include "math3d.h"
#include "event.h"
#include "object.h"
#include "physics.h"
#include "auto.h"
#include "robotmain.h"
#include "terrain.h"
#include "sound.h"
#include "water.h"
#include "particule.h"



#define FOG_HSUP    10.0f
#define FOG_HINF    100.0f




// Check if an object can be destroyed, but is not an enemy.

BOOL IsSoft(ObjectType type)
{
    return ( type == OBJECT_HUMAN    ||
             type == OBJECT_MOBILEfa ||
             type == OBJECT_MOBILEta ||
             type == OBJECT_MOBILEwa ||
             type == OBJECT_MOBILEia ||
             type == OBJECT_MOBILEfc ||
             type == OBJECT_MOBILEtc ||
             type == OBJECT_MOBILEwc ||
             type == OBJECT_MOBILEic ||
             type == OBJECT_MOBILEfi ||
             type == OBJECT_MOBILEti ||
             type == OBJECT_MOBILEwi ||
             type == OBJECT_MOBILEii ||
             type == OBJECT_MOBILEfs ||
             type == OBJECT_MOBILEts ||
             type == OBJECT_MOBILEws ||
             type == OBJECT_MOBILEis ||
             type == OBJECT_MOBILErt ||
             type == OBJECT_MOBILErc ||
             type == OBJECT_MOBILErr ||
             type == OBJECT_MOBILErs ||
             type == OBJECT_MOBILEsa ||
             type == OBJECT_MOBILEft ||
             type == OBJECT_MOBILEtt ||
             type == OBJECT_MOBILEwt ||
             type == OBJECT_MOBILEit ||
             type == OBJECT_MOBILEdr ||  // robot?
             type == OBJECT_METAL    ||
             type == OBJECT_POWER    ||
             type == OBJECT_ATOMIC   ||  // cargo?
             type == OBJECT_DERRICK  ||
             type == OBJECT_STATION  ||
             type == OBJECT_FACTORY  ||
             type == OBJECT_REPAIR   ||
             type == OBJECT_DESTROYER||
             type == OBJECT_CONVERT  ||
             type == OBJECT_TOWER    ||
             type == OBJECT_RESEARCH ||
             type == OBJECT_RADAR    ||
             type == OBJECT_INFO     ||
             type == OBJECT_ENERGY   ||
             type == OBJECT_LABO     ||
             type == OBJECT_NUCLEAR  ||
             type == OBJECT_PARA     );  // building?
}

// Check if an object is a destroyable enemy.

BOOL IsAlien(ObjectType type)
{
    return ( type == OBJECT_ANT      ||
             type == OBJECT_SPIDER   ||
             type == OBJECT_BEE      ||
             type == OBJECT_WORM     ||
             type == OBJECT_MOTHER   ||
             type == OBJECT_NEST     ||
             type == OBJECT_BULLET   ||
             type == OBJECT_EGG      ||
             type == OBJECT_MOBILEtg ||
             type == OBJECT_TEEN28   ||
             type == OBJECT_TEEN31   );
}

// Returns the damping factor for friendly fire.

float RetDecay(ObjectType type)
{
    if ( IsSoft(type) )  return 0.2f;
    return 1.0f;
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


// Removes all particles.

void CParticule::FlushParticule()
{
    int     i, j;

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

    m_wheelTraceTotal = 0;
    m_wheelTraceIndex = 0;

    for ( i=0 ; i<SH_MAX ; i++ )
    {
        m_bFrameUpdate[i] = TRUE;
    }

    m_fogTotal = 0;
    m_exploGunCounter = 0;
}

// Removes all particles of a sheet.

void CParticule::FlushParticule(int sheet)
{
    int     i;

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

    if ( sheet == SH_WORLD )
    {
        m_wheelTraceTotal = 0;
        m_wheelTraceIndex = 0;
    }
}


// Builds file name of the effect.
//  effectNN.tga, with NN = number

void NameParticule(char *buffer, int num)
{
         if ( num == 1 )  strcpy(buffer, "effect00.tga");
    else if ( num == 2 )  strcpy(buffer, "effect01.tga");
    else if ( num == 3 )  strcpy(buffer, "effect02.tga");
#if _POLISH
    else if ( num == 4 )  strcpy(buffer, "textp.tga");
#else
    else if ( num == 4 )  strcpy(buffer, "text.tga");
#endif
    else                  strcpy(buffer, "xxx.tga");
}


// Creates a new particle.
// Returns the channel of the particle created or -1 on error.

int CParticule::CreateParticule(D3DVECTOR pos, D3DVECTOR speed, FPOINT dim,
                                ParticuleType type,
                                float duration, float mass,
                                float windSensitivity, int sheet)
{
//? float   dist;
    int     i, j, t;

    if ( m_main == 0 )
    {
        m_main = (CRobotMain*)m_iMan->SearchInstance(CLASS_MAIN);
    }

#if 0
    if ( sheet == SH_WORLD       &&
         type != PARTISELY       &&
         type != PARTISELR       &&
         type != PARTIGUN1       &&
         type != PARTIGUN2       &&
         type != PARTIGUN3       &&
         type != PARTIGUN4       &&
         type != PARTIQUARTZ     &&
         !m_main->RetMovieLock() )
    {
        dist = Length(pos, m_engine->RetEyePt());
        if ( dist > 300.0f )  return -1;
    }
#endif

    t = -1;
    if ( type == PARTIEXPLOT   ||
         type == PARTIEXPLOO   ||
         type == PARTIMOTOR    ||
         type == PARTIBLITZ    ||
         type == PARTICRASH    ||
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
         type == PARTIQUEUE    ||
         type == PARTIORGANIC1 ||
         type == PARTIORGANIC2 ||
         type == PARTIFLAME    ||
         type == PARTIBUBBLE   ||
         type == PARTIERROR    ||
         type == PARTIWARNING  ||
         type == PARTIINFO     ||
         type == PARTISPHERE1  ||
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
         type == PARTILENS1    ||
         type == PARTILENS2    ||
         type == PARTILENS3    ||
         type == PARTILENS4    ||
         type == PARTIGFLAT    ||
         type == PARTIDROP     ||
         type == PARTIWATER    ||
         type == PARTILIMIT1   ||
         type == PARTILIMIT2   ||
         type == PARTILIMIT3   ||
         type == PARTILIMIT4   ||
         type == PARTIEXPLOG1  ||
         type == PARTIEXPLOG2  )
    {
        t = 1;  // effect00
    }
    if ( type == PARTIGLINT   ||
         type == PARTIGLINTb  ||
         type == PARTIGLINTr  ||
         type == PARTITOTO    ||
         type == PARTISELY    ||
         type == PARTISELR    ||
         type == PARTIQUARTZ  ||
         type == PARTIGUNDEL  ||
         type == PARTICONTROL ||
         type == PARTISHOW    ||
         type == PARTICHOC    ||
         type == PARTIFOG4    ||
         type == PARTIFOG5    ||
         type == PARTIFOG6    ||
         type == PARTIFOG7    )
    {
        t = 2;  // effect01
    }
    if ( type == PARTIGUN1    ||
         type == PARTIFLIC    ||
         type == PARTISPHERE0 ||
         type == PARTISPHERE3 ||
         type == PARTIFOG0    ||
         type == PARTIFOG1    ||
         type == PARTIFOG2    ||
         type == PARTIFOG3    )
    {
        t = 3;  // effect02
    }
    if ( type == PARTISMOKE1  ||
         type == PARTISMOKE2  ||
         type == PARTISMOKE3  ||
         type == PARTIBLOOD   ||
         type == PARTIBLOODM  ||
         type == PARTIVIRUS1  ||
         type == PARTIVIRUS2  ||
         type == PARTIVIRUS3  ||
         type == PARTIVIRUS4  ||
         type == PARTIVIRUS5  ||
         type == PARTIVIRUS6  ||
         type == PARTIVIRUS7  ||
         type == PARTIVIRUS8  ||
         type == PARTIVIRUS9  ||
         type == PARTIVIRUS10 )
    {
        t = 4;  // text (D3DSTATETTw)
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
            m_particule[i].windSensitivity = windSensitivity;
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
            m_particule[i].phaseTime = 0.0f;
            m_particule[i].testTime  = 0.0f;
            m_particule[i].objLink   = 0;
            m_particule[i].objFather = 0;
            m_particule[i].trackRank = -1;

            m_totalInterface[t][sheet] ++;

            if ( type == PARTIEXPLOT ||
                 type == PARTIEXPLOO )
            {
                m_particule[i].angle = Rand()*PI*2.0f;
            }

            if ( type == PARTIGUN1 ||
                 type == PARTIGUN4 )
            {
                m_particule[i].testTime = 1.0f;  // impact immediately
            }

            if ( type >= PARTIFOG0 &&
                 type <= PARTIFOG9 )
            {
                if ( m_fogTotal < MAXPARTIFOG )
                m_fog[m_fogTotal++] = i;
            }

            return i | ((m_particule[i].uniqueStamp&0xffff)<<16);
        }
    }

    return -1;
}

// Creates a new triangular particle (debris).
// Returns the channel of the particle created or -1 on error.

int CParticule::CreateFrag(D3DVECTOR pos, D3DVECTOR speed,
                           D3DTriangle *triangle,
                           ParticuleType type,
                           float duration, float mass,
                           float windSensitivity, int sheet)
{
    D3DVECTOR   p1, p2, p3, n;
    float       l1, l2, l3, dx, dy;
    int         i, j, t;

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
            m_particule[i].windSensitivity = windSensitivity;
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
            m_particule[i].phaseTime = 0.0f;
            m_particule[i].testTime  = 0.0f;
            m_particule[i].objLink   = 0;
            m_particule[i].objFather = 0;
            m_particule[i].trackRank = -1;
            m_triangle[i] = *triangle;

            m_totalInterface[t][sheet] ++;

            p1.x = m_triangle[i].triangle[0].x;
            p1.y = m_triangle[i].triangle[0].y;
            p1.z = m_triangle[i].triangle[0].z;

            p2.x = m_triangle[i].triangle[1].x;
            p2.y = m_triangle[i].triangle[1].y;
            p2.z = m_triangle[i].triangle[1].z;

            p3.x = m_triangle[i].triangle[2].x;
            p3.y = m_triangle[i].triangle[2].y;
            p3.z = m_triangle[i].triangle[2].z;

            l1 = Length(p1, p2);
            l2 = Length(p2, p3);
            l3 = Length(p3, p1);
            dx = Abs(Min(l1, l2, l3))*0.5f;
            dy = Abs(Max(l1, l2, l3))*0.5f;
            p1 = D3DVECTOR(-dx,  dy, 0.0f);
            p2 = D3DVECTOR( dx,  dy, 0.0f);
            p3 = D3DVECTOR(-dx, -dy, 0.0f);

            m_triangle[i].triangle[0].x = p1.x;
            m_triangle[i].triangle[0].y = p1.y;
            m_triangle[i].triangle[0].z = p1.z;

            m_triangle[i].triangle[1].x = p2.x;
            m_triangle[i].triangle[1].y = p2.y;
            m_triangle[i].triangle[1].z = p2.z;

            m_triangle[i].triangle[2].x = p3.x;
            m_triangle[i].triangle[2].y = p3.y;
            m_triangle[i].triangle[2].z = p3.z;

            n = D3DVECTOR(0.0f, 0.0f, -1.0f);

            m_triangle[i].triangle[0].nx = n.x;
            m_triangle[i].triangle[0].ny = n.y;
            m_triangle[i].triangle[0].nz = n.z;

            m_triangle[i].triangle[1].nx = n.x;
            m_triangle[i].triangle[1].ny = n.y;
            m_triangle[i].triangle[1].nz = n.z;

            m_triangle[i].triangle[2].nx = n.x;
            m_triangle[i].triangle[2].ny = n.y;
            m_triangle[i].triangle[2].nz = n.z;

            if ( type == PARTIFRAG )
            {
                m_particule[i].angle = Rand()*PI*2.0f;
            }
            return i | ((m_particule[i].uniqueStamp&0xffff)<<16);
        }
    }

    return -1;
}

// Creates a new particle being a part of object.
// Returns the channel of the particle created or -1 on error.

int CParticule::CreatePart(D3DVECTOR pos, D3DVECTOR speed,
                           ParticuleType type,
                           float duration, float mass, float weight,
                           float windSensitivity, int sheet)
{
    int         i, j, t;

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
            m_particule[i].windSensitivity = windSensitivity;
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
            m_particule[i].phaseTime = 0.0f;
            m_particule[i].testTime  = 0.0f;
            m_particule[i].trackRank = -1;

            m_totalInterface[t][sheet] ++;

            return i | ((m_particule[i].uniqueStamp&0xffff)<<16);
        }
    }

    return -1;
}

// Creates a new linear particle (radius).
// Returns the channel of the particle created or -1 on error.

int CParticule::CreateRay(D3DVECTOR pos, D3DVECTOR goal,
                          ParticuleType type, FPOINT dim,
                          float duration, int sheet)
{
    int     i, j, t;

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
            m_particule[i].windSensitivity = 0.0f;
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
            m_particule[i].phaseTime = 0.0f;
            m_particule[i].testTime  = 0.0f;
            m_particule[i].objLink   = 0;
            m_particule[i].objFather = 0;
            m_particule[i].trackRank = -1;

            m_totalInterface[t][sheet] ++;

            return i | ((m_particule[i].uniqueStamp&0xffff)<<16);
        }
    }

    return -1;
}

// Creates a particle with a trail.
// "length" is the length of the tail of drag (in seconds)!

int CParticule::CreateTrack(D3DVECTOR pos, D3DVECTOR speed, FPOINT dim,
                            ParticuleType type, float duration, float mass,
                            float length, float width)
{
    int     channel, rank, i;

    // Creates the normal particle.
    channel = CreateParticule(pos, speed, dim, type, duration, mass, 0.0f, 0);
    if ( channel == -1 )  return -1;

    // Seeks a streak free.
    for ( i=0 ; i<MAXTRACK ; i++ )
    {
        if ( !m_track[i].bUsed )  // free?
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

// Creates a tire mark.

void CParticule::CreateWheelTrace(const D3DVECTOR &p1, const D3DVECTOR &p2,
                                  const D3DVECTOR &p3, const D3DVECTOR &p4,
                                  ParticuleType type)
{
    int     i, max;

//? max = (int)(m_engine->RetWheelTraceQuantity()*MAXWHEELTRACE);
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

    m_terrain->MoveOnFloor(m_wheelTrace[i].pos[0]);
    m_wheelTrace[i].pos[0].y += 0.2f;  // just above the ground

    m_terrain->MoveOnFloor(m_wheelTrace[i].pos[1]);
    m_wheelTrace[i].pos[1].y += 0.2f;  // just above the ground

    m_terrain->MoveOnFloor(m_wheelTrace[i].pos[2]);
    m_wheelTrace[i].pos[2].y += 0.2f;  // just above the ground

    m_terrain->MoveOnFloor(m_wheelTrace[i].pos[3]);
    m_wheelTrace[i].pos[3].y += 0.2f;  // just above the ground

    if ( m_wheelTraceTotal < max )
    {
        m_wheelTraceTotal ++;
    }
    else
    {
        m_wheelTraceTotal = max;
    }
}


// Check a channel number.
// Adapts the channel so it can be used as an offset in m_particule.

BOOL CParticule::CheckChannel(int &channel)
{
    int     uniqueStamp;

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

// Removes a particle after his rank.

void CParticule::DeleteRank(int rank)
{
    int     i;

    if ( m_totalInterface[rank/MAXPARTICULE][m_particule[rank].sheet] > 0 )
    {
        m_totalInterface[rank/MAXPARTICULE][m_particule[rank].sheet] --;
    }

    i = m_particule[rank].trackRank;
    if ( i != -1 )  // drag associated?
    {
        m_track[i].bUsed = FALSE;  // frees the drag
    }

    m_particule[rank].bUsed = FALSE;
}

// Removes all particles of a given type.

void CParticule::DeleteParticule(ParticuleType type)
{
    int     i;

    for ( i=0 ; i<MAXPARTICULE*MAXPARTITYPE ; i++ )
    {
        if ( !m_particule[i].bUsed )  continue;
        if ( m_particule[i].type != type )  continue;

        DeleteRank(i);
    }
}

// Removes all particles of a given channel.

void CParticule::DeleteParticule(int channel)
{
    int     i;

    if ( !CheckChannel(channel) )  return;

    if ( m_totalInterface[channel/MAXPARTICULE][m_particule[channel].sheet] > 0 )
    {
        m_totalInterface[channel/MAXPARTICULE][m_particule[channel].sheet] --;
    }

    i = m_particule[channel].trackRank;
    if ( i != -1 )  // drag associated?
    {
        m_track[i].bUsed = FALSE;  // frees the drag
    }

    m_particule[channel].bUsed = FALSE;
}


// Specifies the object to which the particle is bound.

void CParticule::SetObjectLink(int channel, CObject *object)
{
    if ( !CheckChannel(channel) )  return;
    m_particule[channel].objLink = object;
}

// Specifies the parent object that created the particle.

void CParticule::SetObjectFather(int channel, CObject *object)
{
    if ( !CheckChannel(channel) )  return;
    m_particule[channel].objFather = object;
}

void CParticule::SetPosition(int channel, D3DVECTOR pos)
{
    if ( !CheckChannel(channel) )  return;
    m_particule[channel].pos = pos;
}

void CParticule::SetDimension(int channel, FPOINT dim)
{
    if ( !CheckChannel(channel) )  return;
    m_particule[channel].dim = dim;
}

void CParticule::SetZoom(int channel, float zoom)
{
    if ( !CheckChannel(channel) )  return;
    m_particule[channel].zoom = zoom;
}

void CParticule::SetAngle(int channel, float angle)
{
    if ( !CheckChannel(channel) )  return;
    m_particule[channel].angle = angle;
}

void CParticule::SetIntensity(int channel, float intensity)
{
    if ( !CheckChannel(channel) )  return;
    m_particule[channel].intensity = intensity;
}

void CParticule::SetParam(int channel, D3DVECTOR pos, FPOINT dim, float zoom,
                          float angle, float intensity)
{
    if ( !CheckChannel(channel) )  return;
    m_particule[channel].pos       = pos;
    m_particule[channel].dim       = dim;
    m_particule[channel].zoom      = zoom;
    m_particule[channel].angle     = angle;
    m_particule[channel].intensity = intensity;
}

void CParticule::SetPhase(int channel, ParticulePhase phase, float duration)
{
    if ( !CheckChannel(channel) )  return;
    m_particule[channel].phase = phase;
    m_particule[channel].duration = duration;
    m_particule[channel].phaseTime = m_particule[channel].time;
}

// Returns the position of the particle.

BOOL CParticule::GetPosition(int channel, D3DVECTOR &pos)
{
    if ( !CheckChannel(channel) )  return FALSE;
    pos = m_particule[channel].pos;
    return TRUE;
}


// Indicates whether a sheet evolves or not.

void CParticule::SetFrameUpdate(int sheet, BOOL bUpdate)
{
    m_bFrameUpdate[sheet] = bUpdate;
}

// Makes evolve all the particles.

void CParticule::FrameParticule(float rTime)
{
    CObject*    object;
    D3DVECTOR   eye, pos, speed, wind;
    FPOINT      ts, ti, dim;
    BOOL        bPause;
    float       progress, dp, h, duration, mass, amplitude;
    int         i, j, r, total;

    if ( m_main == 0 )
    {
        m_main = (CRobotMain*)m_iMan->SearchInstance(CLASS_MAIN);
    }

    bPause = ( m_engine->RetPause() && !m_main->RetInfoLock() );

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

    wind = m_terrain->RetWind();
    eye = m_engine->RetEyePt();

    for ( i=0 ; i<MAXPARTICULE*MAXPARTITYPE ; i++ )
    {
        if ( !m_particule[i].bUsed )  continue;
        if ( !m_bFrameUpdate[m_particule[i].sheet] )  continue;

        if ( m_particule[i].type != PARTISHOW )
        {
            if ( bPause && m_particule[i].sheet != SH_INTERFACE )  continue;
        }

        if ( m_particule[i].type != PARTIQUARTZ )
        {
            m_particule[i].pos += m_particule[i].speed*rTime;
        }

        if ( m_particule[i].sheet == SH_WORLD )
        {
            h = rTime*m_particule[i].windSensitivity*Rand()*2.0f;
            m_particule[i].pos += wind*h;
        }

        progress = (m_particule[i].time-m_particule[i].phaseTime)/m_particule[i].duration;

        // Manages the particles with mass that bounce.
        if ( m_particule[i].mass != 0.0f        &&
             m_particule[i].type != PARTIQUARTZ )
        {
            m_particule[i].speed.y -= m_particule[i].mass*rTime;

            if ( m_particule[i].sheet == SH_INTERFACE )
            {
                h = 0.0f;
            }
            else
            {
                h = m_terrain->RetFloorLevel(m_particule[i].pos, TRUE);
            }
            h += m_particule[i].dim.y*0.75f;
            if ( m_particule[i].pos.y < h )  // impact with the ground?
            {
                if ( m_particule[i].type == PARTIPART &&
                     m_particule[i].weight > 3.0f &&  // heavy enough?
                     m_particule[i].bounce < 3 )
                {
                    amplitude = m_particule[i].weight*0.1f;
                    amplitude *= 1.0f-0.3f*m_particule[i].bounce;
                    if ( amplitude > 1.0f )  amplitude = 1.0f;
                    if ( amplitude > 0.0f )
                    {
                        Play(SOUND_BOUM, m_particule[i].pos, amplitude);
                    }
                }

                if ( m_particule[i].bounce < 3 )
                {
                    m_particule[i].pos.y = h;
                    m_particule[i].speed.y *= -0.4f;
                    m_particule[i].speed.x *=  0.4f;
                    m_particule[i].speed.z *=  0.4f;
                    m_particule[i].bounce ++;  // more impact
                }
                else    // disappears after 3 bounces?
                {
                    if ( m_particule[i].pos.y < h-10.0f ||
                         m_particule[i].time >= 20.0f   )
                    {
                        DeleteRank(i);
                        continue;
                    }
                }
            }
        }

        // Manages drag associated.
        r = m_particule[i].trackRank;
        if ( r != -1 )  // drag exists?
        {
            if ( TrackMove(r, m_particule[i].pos, progress) )
            {
                DeleteRank(i);
                continue;
            }

            m_track[r].bDrawParticule = (progress < 1.0f);
        }

        if ( m_particule[i].type == PARTITRACK1 )  // explosion technique?
        {
            m_particule[i].zoom = 1.0f-(m_particule[i].time-m_particule[i].duration);

            ts.x = 0.375f;
            ts.y = 0.000f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if ( m_particule[i].type == PARTITRACK2 )  // spray blue?
        {
            m_particule[i].zoom = 1.0f-(m_particule[i].time-m_particule[i].duration);

            ts.x = 0.500f;
            ts.y = 0.000f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if ( m_particule[i].type == PARTITRACK3 )  // spider?
        {
            m_particule[i].zoom = 1.0f-(m_particule[i].time-m_particule[i].duration);

            ts.x = 0.500f;
            ts.y = 0.750f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if ( m_particule[i].type == PARTITRACK4 )  // insect explosion?
        {
            m_particule[i].zoom = 1.0f-(m_particule[i].time-m_particule[i].duration);

            ts.x = 0.625f;
            ts.y = 0.000f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if ( m_particule[i].type == PARTITRACK5 )  // derrick?
        {
            m_particule[i].zoom = 1.0f-(m_particule[i].time-m_particule[i].duration);

            ts.x = 0.750f;
            ts.y = 0.000f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if ( m_particule[i].type == PARTITRACK6 )  // reset in/out?
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

        if ( m_particule[i].type == PARTITRACK11 )  // phazer shot?
        {
            object = SearchObjectGun(m_particule[i].goal, m_particule[i].pos, m_particule[i].type, m_particule[i].objFather);
            m_particule[i].goal = m_particule[i].pos;
            if ( object != 0 )
            {
                if ( object->RetType() == OBJECT_MOTHER )
                {
                    object->ExploObject(EXPLO_BOUM, 0.1f);
                }
                else
                {
                    object->ExploObject(EXPLO_BOUM, 0.0f, RetDecay(object->RetType()));
                }
            }

            m_particule[i].zoom = 1.0f-(m_particule[i].time-m_particule[i].duration);

            ts.x = 0.375f;
            ts.y = 0.000f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if ( m_particule[i].type == PARTITRACK12 )  // drag reactor?
        {
            m_particule[i].zoom = 1.0f;

            ts.x = 0.375f;
            ts.y = 0.000f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
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

        if ( m_particule[i].type == PARTICRASH )
        {
            if ( progress >= 1.0f )
            {
                DeleteRank(i);
                continue;
            }

//?         m_particule[i].intensity = 1.0f-progress;
            if ( progress < 0.25f )
            {
                m_particule[i].zoom = progress/0.25f;
            }
            else
            {
                m_particule[i].intensity = 1.0f-(progress-0.25f)/0.75f;
            }

//?         ts.x = 0.250f;
            ts.x = 0.000f;
//?         ts.x = 0.375f;
            ts.y = 0.750f;
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

        if ( m_particule[i].type == PARTIGUN1 )  // fireball shot?
        {
            if ( progress >= 1.0f )
            {
                DeleteRank(i);
                continue;
            }

            if ( m_particule[i].testTime >= 0.1f )
            {
                m_particule[i].testTime = 0.0f;

                if ( m_terrain->RetFloorHeight(m_particule[i].pos, TRUE) < -2.0f )
                {
                    m_exploGunCounter ++;

                    if ( m_exploGunCounter%2 == 0 )
                    {
                        pos = m_particule[i].goal;
                        m_terrain->MoveOnFloor(pos, TRUE);
                        speed.x = 0.0f;
                        speed.z = 0.0f;
                        speed.y = 0.0f;
                        dim.x = Rand()*6.0f+6.0f;
                        dim.y = dim.x;
                        duration = Rand()*1.0f+1.0f;
                        mass = 0.0f;
                        CreateParticule(pos, speed, dim, PARTIEXPLOG1, duration, mass, 1.0f);

                        pos.y += 1.0f;
                        total = (int)(2.0f*m_engine->RetParticuleDensity());
                        for ( j=0 ; j<total ; j++ )
                        {
                            speed.x = (Rand()-0.5f)*20.0f;
                            speed.z = (Rand()-0.5f)*20.0f;
                            speed.y = Rand()*20.0f;
                            dim.x = 1.0f;
                            dim.y = dim.x;
                            duration = Rand()*1.0f+1.0f;
                            mass = Rand()*10.0f+15.0f;
                            CreateParticule(pos, speed, dim, PARTIEXPLOG1, duration, mass, 1.0f);
                        }
                    }

                    if ( m_exploGunCounter%4 == 0 )
                    {
                        Play(SOUND_EXPLOg1, pos, 0.5f);
                    }

                    DeleteRank(i);
                    continue;
                }

                object = SearchObjectGun(m_particule[i].goal, m_particule[i].pos, m_particule[i].type, m_particule[i].objFather);
                m_particule[i].goal = m_particule[i].pos;
                if ( object != 0 )
                {
                    object->ExploObject(EXPLO_BURN, 0.0f, RetDecay(object->RetType()));

                    m_exploGunCounter ++;

                    if ( m_exploGunCounter%2 == 0 )
                    {
                        pos = m_particule[i].pos;
                        speed.x = 0.0f;
                        speed.z = 0.0f;
                        speed.y = 0.0f;
                        dim.x = Rand()*6.0f+6.0f;
                        dim.y = dim.x;
                        duration = Rand()*1.0f+1.0f;
                        mass = 0.0f;
                        CreateParticule(pos, speed, dim, PARTIEXPLOG1, duration, mass, 1.0f);

                        pos.y += 1.0f;
                        total = (int)(2.0f*m_engine->RetParticuleDensity());
                        for ( j=0 ; j<total ; j++ )
                        {
                            speed.x = (Rand()-0.5f)*20.0f;
                            speed.z = (Rand()-0.5f)*20.0f;
                            speed.y = Rand()*20.0f;
                            dim.x = 1.0f;
                            dim.y = dim.x;
                            duration = Rand()*1.0f+1.0f;
                            mass = Rand()*10.0f+15.0f;
                            CreateParticule(pos, speed, dim, PARTIEXPLOG1, duration, mass, 1.0f);
                        }
                    }

                    if ( m_exploGunCounter%4 == 0 )
                    {
                        Play(SOUND_EXPLOg1, pos, 0.5f);
                    }

                    DeleteRank(i);
                    continue;
                }
            }

            m_particule[i].angle -= rTime*PI*8.0f;
            m_particule[i].zoom = 1.0f-progress;

            ts.x = 0.00f;
            ts.y = 0.50f;
            ti.x = ts.x+0.25f;
            ti.y = ts.y+0.25f;
        }

        if ( m_particule[i].type == PARTIGUN2 )  // ant shot?
        {
            if ( progress >= 1.0f )
            {
                DeleteRank(i);
                continue;
            }

            if ( m_particule[i].testTime >= 0.2f )
            {
                m_particule[i].testTime = 0.0f;
                object = SearchObjectGun(m_particule[i].goal, m_particule[i].pos, m_particule[i].type, m_particule[i].objFather);
                m_particule[i].goal = m_particule[i].pos;
                if ( object != 0 )
                {
                    if ( object->RetShieldRadius() > 0.0f )  // protected by shield?
                    {
                        CreateParticule(m_particule[i].pos, D3DVECTOR(0.0f, 0.0f, 0.0f), FPOINT(6.0f, 6.0f), PARTIGUNDEL, 2.0f);
                        if ( m_lastTimeGunDel > 0.2f )
                        {
                            m_lastTimeGunDel = 0.0f;
                            Play(SOUND_GUNDEL, m_particule[i].pos, 1.0f);
                        }
                        DeleteRank(i);
                        continue;
                    }
                    else
                    {
                        if ( object->RetType() != OBJECT_HUMAN )
                        {
                            Play(SOUND_TOUCH, m_particule[i].pos, 1.0f);
                        }
                        object->ExploObject(EXPLO_BOUM, 0.0f);  // starts explosion
                    }
                }
            }

            m_particule[i].angle = Rand()*PI*2.0f;
            m_particule[i].zoom = 1.0f-progress;

            ts.x = 0.125f;
            ts.y = 0.875f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if ( m_particule[i].type == PARTIGUN3 )  // spider suicides?
        {
            if ( progress >= 1.0f )
            {
                DeleteRank(i);
                continue;
            }

            if ( m_particule[i].testTime >= 0.2f )
            {
                m_particule[i].testTime = 0.0f;
                object = SearchObjectGun(m_particule[i].goal, m_particule[i].pos, m_particule[i].type, m_particule[i].objFather);
                m_particule[i].goal = m_particule[i].pos;
                if ( object != 0 )
                {
                    if ( object->RetShieldRadius() > 0.0f )
                    {
                        CreateParticule(m_particule[i].pos, D3DVECTOR(0.0f, 0.0f, 0.0f), FPOINT(6.0f, 6.0f), PARTIGUNDEL, 2.0f);
                        if ( m_lastTimeGunDel > 0.2f )
                        {
                            m_lastTimeGunDel = 0.0f;
                            Play(SOUND_GUNDEL, m_particule[i].pos, 1.0f);
                        }
                        DeleteRank(i);
                        continue;
                    }
                    else
                    {
                        object->ExploObject(EXPLO_BURN, 1.0f);  // starts explosion
                    }
                }
            }

//?         ts.x = 0.875f;
//?         ts.y = 0.750f;
            ts.x = 0.500f;
            ts.y = 0.750f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if ( m_particule[i].type == PARTIGUN4 )  // orgaball shot?
        {
            if ( progress >= 1.0f )
            {
                DeleteRank(i);
                continue;
            }

            if ( m_particule[i].testTime >= 0.1f )
            {
                m_particule[i].testTime = 0.0f;

                if ( m_terrain->RetFloorHeight(m_particule[i].pos, TRUE) < -2.0f )
                {
                    m_exploGunCounter ++;

                    if ( m_exploGunCounter%2 == 0 )
                    {
                        pos = m_particule[i].goal;
                        m_terrain->MoveOnFloor(pos, TRUE);
                        speed.x = 0.0f;
                        speed.z = 0.0f;
                        speed.y = 0.0f;
                        dim.x = Rand()*4.0f+2.0f;
                        dim.y = dim.x;
                        duration = Rand()*0.7f+0.7f;
                        mass = 0.0f;
                        CreateParticule(pos, speed, dim, PARTIEXPLOG2, duration, mass, 1.0f);
                    }

                    if ( m_exploGunCounter%4 == 0 )
                    {
                        Play(SOUND_EXPLOg2, pos, 0.5f);
                    }

                    DeleteRank(i);
                    continue;
                }

                object = SearchObjectGun(m_particule[i].goal, m_particule[i].pos, m_particule[i].type, m_particule[i].objFather);
                m_particule[i].goal = m_particule[i].pos;
                if ( object != 0 )
                {
                    object->ExploObject(EXPLO_BOUM, 0.0f, RetDecay(object->RetType()));

                    m_exploGunCounter ++;

                    if ( m_exploGunCounter%2 == 0 )
                    {
                        pos = m_particule[i].pos;
                        speed.x = 0.0f;
                        speed.z = 0.0f;
                        speed.y = 0.0f;
                        dim.x = Rand()*4.0f+2.0f;
                        dim.y = dim.x;
                        duration = Rand()*0.7f+0.7f;
                        mass = 0.0f;
                        CreateParticule(pos, speed, dim, PARTIEXPLOG2, duration, mass, 1.0f);
                    }

                    if ( m_exploGunCounter%4 == 0 )
                    {
                        Play(SOUND_EXPLOg2, pos, 0.5f);
                    }

                    DeleteRank(i);
                    continue;
                }
            }

            m_particule[i].angle = Rand()*PI*2.0f;
            m_particule[i].zoom = 1.0f-progress;

            ts.x = 0.125f;
            ts.y = 0.875f;
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

            ts.x = 0.00f;
            ts.y = 0.75f;
            ti.x = ts.x+0.25f;
            ti.y = ts.y+0.25f;
        }

        if ( m_particule[i].type == PARTISHOW )
        {
            if ( progress >= 1.0f )
            {
                DeleteRank(i);
                continue;
            }

            if ( progress < 0.5f )  m_particule[i].intensity = progress/0.5f;
            else                    m_particule[i].intensity = 2.0f-progress/0.5f;
            m_particule[i].zoom = 1.0f-progress*0.8f;
            m_particule[i].angle -= rTime*PI*0.5f;

            ts.x = 0.50f;
            ts.y = 0.00f;
            ti.x = ts.x+0.25f;
            ti.y = ts.y+0.25f;
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

        if ( m_particule[i].type == PARTIGFLAT )
        {
            if ( progress >= 1.0f )
            {
                DeleteRank(i);
                continue;
            }

            m_particule[i].zoom = 0.1f+progress;
            m_particule[i].intensity = 1.0f-progress;
            m_particule[i].angle -= rTime*PI*2.0f;

            ts.x = 0.00f;
            ts.y = 0.50f;
            ti.x = ts.x+0.25f;
            ti.y = ts.y+0.25f;
        }

        if ( m_particule[i].type == PARTILIMIT1 )
        {
            if ( progress >= 1.0f )
            {
                DeleteRank(i);
                continue;
            }

            m_particule[i].zoom = 1.0f;
            m_particule[i].intensity = 1.0f;

            ts.x = 0.000f;
            ts.y = 0.125f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }
        if ( m_particule[i].type == PARTILIMIT2 )
        {
            if ( progress >= 1.0f )
            {
                DeleteRank(i);
                continue;
            }

            m_particule[i].zoom = 1.0f;
            m_particule[i].intensity = 1.0f;

            ts.x = 0.375f;
            ts.y = 0.125f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }
        if ( m_particule[i].type == PARTILIMIT3 )
        {
            if ( progress >= 1.0f )
            {
                DeleteRank(i);
                continue;
            }

            m_particule[i].zoom = 1.0f;
            m_particule[i].intensity = 1.0f;

            ts.x = 0.500f;
            ts.y = 0.125f;
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

        // Decreases the intensity if the camera
        // is almost at the same height (fog was eye level).
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
             m_particule[i].type == PARTIEXPLOO )
        {
            if ( progress >= 1.0f )
            {
                DeleteRank(i);
                continue;
            }

            m_particule[i].zoom = 1.0f-progress/2.0f;
            m_particule[i].intensity = 1.0f-progress;

            if ( m_particule[i].type == PARTIEXPLOT )  ts.x = 0.750f;
            else                                       ts.x = 0.875f;
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
            if ( progress >= 1.0f ||
                 m_particule[i].pos.y >= m_water->RetLevel() )
            {
                DeleteRank(i);
                continue;
            }

            m_particule[i].zoom = 1.0f-progress/2.0f;
            m_particule[i].intensity = 1.0f-progress;

            ts.x = 0.250f;
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

            ts.x = 0.500f+0.125f*(m_particule[i].type-PARTISMOKE1);
            ts.y = 0.750f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if ( m_particule[i].type == PARTIBLOOD )
        {
            if ( progress >= 1.0f )
            {
                DeleteRank(i);
                continue;
            }

            m_particule[i].intensity = 1.0f-progress;

            ts.x = 0.750f+(rand()%2)*0.125f;
            ts.y = 0.875f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if ( m_particule[i].type == PARTIBLOODM )
        {
            if ( progress >= 1.0f )
            {
                DeleteRank(i);
                continue;
            }

            m_particule[i].intensity = 1.0f-progress;

            ts.x = 0.875f;
            ts.y = 0.750f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if ( m_particule[i].type == PARTIVIRUS1  ||
             m_particule[i].type == PARTIVIRUS2  ||
             m_particule[i].type == PARTIVIRUS3  ||
             m_particule[i].type == PARTIVIRUS4  ||
             m_particule[i].type == PARTIVIRUS5  ||
             m_particule[i].type == PARTIVIRUS6  ||
             m_particule[i].type == PARTIVIRUS7  ||
             m_particule[i].type == PARTIVIRUS8  ||
             m_particule[i].type == PARTIVIRUS9  ||
             m_particule[i].type == PARTIVIRUS10 )
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
            m_particule[i].angle += rTime*PI*1.0f;

            if ( m_particule[i].type == PARTIVIRUS1 )  // A ?
            {
                ts.x =   0.0f/256.0f;  ts.y =  19.0f/256.0f;
                ti.x =  10.0f/256.0f;  ti.y =  30.0f/256.0f;
            }
            if ( m_particule[i].type == PARTIVIRUS2 )  // C ?
            {
                ts.x =  19.0f/256.0f;  ts.y =  19.0f/256.0f;
                ti.x =  28.0f/256.0f;  ti.y =  30.0f/256.0f;
            }
            if ( m_particule[i].type == PARTIVIRUS3 )  // E ?
            {
                ts.x =  36.0f/256.0f;  ts.y =  19.0f/256.0f;
                ti.x =  45.0f/256.0f;  ti.y =  30.0f/256.0f;
            }
            if ( m_particule[i].type == PARTIVIRUS4 )  // N ?
            {
                ts.x = 110.0f/256.0f;  ts.y =  19.0f/256.0f;
                ti.x = 120.0f/256.0f;  ti.y =  30.0f/256.0f;
            }
            if ( m_particule[i].type == PARTIVIRUS5 )  // R ?
            {
                ts.x = 148.0f/256.0f;  ts.y =  19.0f/256.0f;
                ti.x = 158.0f/256.0f;  ti.y =  30.0f/256.0f;
            }
            if ( m_particule[i].type == PARTIVIRUS6 )  // T ?
            {
                ts.x = 166.0f/256.0f;  ts.y =  19.0f/256.0f;
                ti.x = 175.0f/256.0f;  ti.y =  30.0f/256.0f;
            }
            if ( m_particule[i].type == PARTIVIRUS7 )  // 0 ?
            {
                ts.x =  90.0f/256.0f;  ts.y =   2.0f/256.0f;
                ti.x =  98.0f/256.0f;  ti.y =  13.0f/256.0f;
            }
            if ( m_particule[i].type == PARTIVIRUS8 )  // 2 ?
            {
                ts.x = 103.0f/256.0f;  ts.y =   2.0f/256.0f;
                ti.x = 111.0f/256.0f;  ti.y =  13.0f/256.0f;
            }
            if ( m_particule[i].type == PARTIVIRUS9 )  // 5 ?
            {
                ts.x = 125.0f/256.0f;  ts.y =   2.0f/256.0f;
                ti.x = 132.0f/256.0f;  ti.y =  13.0f/256.0f;
            }
            if ( m_particule[i].type == PARTIVIRUS10 )  // 9 ?
            {
                ts.x = 153.0f/256.0f;  ts.y =   2.0f/256.0f;
                ti.x = 161.0f/256.0f;  ti.y =  13.0f/256.0f;
            }
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

        if ( m_particule[i].type == PARTIROOT )
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

        if ( m_particule[i].type == PARTIQUEUE )
        {
            if ( m_particule[i].testTime >= 0.05f )
            {
                m_particule[i].testTime = 0.0f;

                D3DVECTOR   pos, speed;
                FPOINT      dim;

                pos = m_particule[i].pos;
//?             speed = -m_particule[i].speed*0.5f;
                speed = D3DVECTOR(0.0f, 0.0f, 0.0f);
                dim.x = 1.0f*(Rand()*0.8f+0.6f);
                dim.y = dim.x;
                CreateParticule(pos, speed, dim, PARTIGAS, 0.5f);
            }

            ts.x = 0.375f;
            ts.y = 0.750f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if ( m_particule[i].type == PARTIORGANIC1 )
        {
            if ( progress >= 1.0f )
            {
                DeleteRank(i);

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
                continue;
            }

            m_particule[i].zoom = (m_particule[i].time-m_particule[i].duration);

            ts.x = 0.125f;
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

            ts.x = 0.125f;
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
//?             m_particule[i].zoom = 1.0f-(m_particule[i].time-m_particule[i].duration/2.0f);
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
//?         m_particule[i].angle = m_particule[i].time*PI;

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

        if ( m_particule[i].type == PARTITOTO )
        {
            if ( progress >= 1.0f )
            {
                DeleteRank(i);
                continue;
            }

            m_particule[i].zoom = 1.0f-progress;
            if ( progress < 0.15f )
            {
                m_particule[i].intensity = progress/0.15f;
            }
            else
            {
                m_particule[i].intensity = 1.0f-(progress-0.15f)/0.85f;
            }
            m_particule[i].intensity *= 0.5f;

            ts.x = 0.25f;
            ts.y = 0.50f;
            ti.x = ts.x+0.25f;
            ti.y = ts.y+0.25f;
        }

        if ( m_particule[i].type == PARTIERROR )
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

        if ( m_particule[i].type == PARTIWARNING )
        {
            if ( progress >= 1.0f )
            {
                DeleteRank(i);
                continue;
            }

            m_particule[i].zoom = progress*1.0f;
            m_particule[i].intensity = 1.0f-progress;

            ts.x = 0.875f;
            ts.y = 0.875f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
        }

        if ( m_particule[i].type == PARTIINFO )
        {
            if ( progress >= 1.0f )
            {
                DeleteRank(i);
                continue;
            }

            m_particule[i].zoom = progress*1.0f;
            m_particule[i].intensity = 1.0f-progress;

            ts.x = 0.750f;
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
//?         m_particule[i].intensity = 1.0f-progress;
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

            if ( progress < 0.30f )
            {
                m_particule[i].intensity = progress/0.30f;
            }
            else
            {
                m_particule[i].intensity = 1.0f-(progress-0.30f)/0.70f;
            }
            m_particule[i].zoom = progress*m_particule[i].dim.x;
            m_particule[i].angle = m_particule[i].time*PI*2.0f;

            ts.x = 0.000f;
            ts.y = 0.000f;
            ti.x = ts.x+0.125f;
            ti.y = ts.y+0.125f;
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
//?         m_particule[i].intensity = 1.0f;
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

            if ( m_particule[i].testTime >= 0.2f )
            {
                m_particule[i].testTime = 0.0f;
                object = SearchObjectRay(m_particule[i].pos, m_particule[i].goal,
                                         m_particule[i].type, m_particule[i].objFather);
                if ( object != 0 )
                {
                    object->ExploObject(EXPLO_BOUM, 0.0f);
                }
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
        m_particule[i].testTime += rTime;
    }
}


// Moves a drag.
// Returns true if the drag is finished.

BOOL CParticule::TrackMove(int i, D3DVECTOR pos, float progress)
{
    D3DVECTOR   last;
    int         h, hh;

    if ( i < 0 || i >= MAXTRACK )  return TRUE;
    if ( m_track[i].bUsed == FALSE )  return TRUE;

    if ( progress < 1.0f )  // particle exists?
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

//?     m_track[i].intensity = 1.0f;
        m_track[i].intensity = 1.0f-progress;
    }
    else    // mort lente de la tra�n�e ?
    {
//?     m_track[i].intensity = 1.0f-(progress-1.0f)/(m_track[i].step*MAXTRACKLEN);
        m_track[i].intensity = 0.0f;
    }

    return (m_track[i].intensity <= 0.0f);
}

// Draws a drag.

void CParticule::TrackDraw(int i, ParticuleType type)
{
    D3DVERTEX2  vertex[4];  // 2 triangles
    D3DVECTOR   corner[4], p1, p2, p, n, eye;
    D3DMATRIX   matrix;
    FPOINT      texInf, texSup, rot;
    float       lTotal, f1, f2, a;
    int         counter, h;

    // Calculates the total length memorized.
    lTotal = 0.0f;
    h = m_track[i].head;
    for ( counter=0 ; counter<m_track[i].used-1 ; counter++ )
    {
        lTotal += m_track[i].len[h];
        h --;  if ( h < 0 )  h = MAXTRACKLEN-1;
    }

    D3DUtil_SetIdentityMatrix(matrix);
    m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &matrix);

    if ( type == PARTITRACK1 )  // explosion technique?
    {
        texInf.x = 64.5f/256.0f;
        texInf.y = 21.0f/256.0f;
        texSup.x = 95.5f/256.0f;
        texSup.y = 22.0f/256.0f;  // orange
    }
    if ( type == PARTITRACK2 )  // blue spray?
    {
        texInf.x = 64.5f/256.0f;
        texInf.y = 13.0f/256.0f;
        texSup.x = 95.5f/256.0f;
        texSup.y = 14.0f/256.0f;  // blue
    }
    if ( type == PARTITRACK3 )  // spider?
    {
        texInf.x = 64.5f/256.0f;
        texInf.y =  5.0f/256.0f;
        texSup.x = 95.5f/256.0f;
        texSup.y =  6.0f/256.0f;  // brown
    }
    if ( type == PARTITRACK4 )  // insect explosion?
    {
        texInf.x = 64.5f/256.0f;
        texInf.y =  9.0f/256.0f;
        texSup.x = 95.5f/256.0f;
        texSup.y = 10.0f/256.0f;  // dark green
    }
    if ( type == PARTITRACK5 )  // derrick?
    {
        texInf.x = 64.5f/256.0f;
        texInf.y = 29.0f/256.0f;
        texSup.x = 95.5f/256.0f;
        texSup.y = 30.0f/256.0f;  // dark brown
    }
    if ( type == PARTITRACK6 )  // reset in/out?
    {
        texInf.x = 64.5f/256.0f;
        texInf.y = 17.0f/256.0f;
        texSup.x = 95.5f/256.0f;
        texSup.y = 18.0f/256.0f;  // cyan
    }
    if ( type == PARTITRACK7 )  // win-1?
    {
        texInf.x = 64.5f/256.0f;
        texInf.y = 41.0f/256.0f;
        texSup.x = 95.5f/256.0f;
        texSup.y = 42.0f/256.0f;  // orange
    }
    if ( type == PARTITRACK8 )  // win-2?
    {
        texInf.x = 64.5f/256.0f;
        texInf.y = 45.0f/256.0f;
        texSup.x = 95.5f/256.0f;
        texSup.y = 46.0f/256.0f;  // yellow
    }
    if ( type == PARTITRACK9 )  // win-3?
    {
        texInf.x = 64.5f/256.0f;
        texInf.y = 49.0f/256.0f;
        texSup.x = 95.5f/256.0f;
        texSup.y = 50.0f/256.0f;  // red
    }
    if ( type == PARTITRACK10 )  // win-4?
    {
        texInf.x = 64.5f/256.0f;
        texInf.y = 53.0f/256.0f;
        texSup.x = 95.5f/256.0f;
        texSup.y = 54.0f/256.0f;  // violet
    }
    if ( type == PARTITRACK11 )  // phazer shot?
    {
        texInf.x = 64.5f/256.0f;
        texInf.y = 21.0f/256.0f;
        texSup.x = 95.5f/256.0f;
        texSup.y = 22.0f/256.0f;  // orange
    }
    if ( type == PARTITRACK12 )  // drag reactor?
    {
        texInf.x = 64.5f/256.0f;
        texInf.y = 21.0f/256.0f;
        texSup.x = 95.5f/256.0f;
        texSup.y = 22.0f/256.0f;  // orange
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

// Draws a triangular particle.

void CParticule::DrawParticuleTriangle(int i)
{
    CObject*        object;
    D3DMATRIX       matrix;
    D3DVECTOR       eye, pos, angle;

    if ( m_particule[i].zoom == 0.0f )  return;

    eye = m_engine->RetEyePt();
    pos = m_particule[i].pos;

    object = m_particule[i].objLink;
    if ( object != 0 )
    {
        pos += object->RetPosition(0);
    }

    angle.x = -RotateAngle(Length2d(pos, eye), pos.y-eye.y);
    angle.y = RotateAngle(pos.z-eye.z, pos.x-eye.x);
    angle.z = m_particule[i].angle;

    MatRotateXZY(matrix, angle);
    matrix._41 = pos.x;
    matrix._42 = pos.y;
    matrix._43 = pos.z;
    m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &matrix);

    m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST, D3DFVF_VERTEX2,
                                m_triangle[i].triangle, 3, NULL);
    m_engine->AddStatisticTriangle(1);
}

// Draw a normal particle.

void CParticule::DrawParticuleNorm(int i)
{
    CObject*        object;
    D3DVERTEX2      vertex[4];  // 2 triangles
    D3DMATRIX       matrix;
    D3DVECTOR       corner[4], eye, pos, n, angle;
    FPOINT          dim;
    float           zoom;

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

        object = m_particule[i].objLink;
        if ( object != 0 )
        {
            pos += object->RetPosition(0);
        }

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

// Draw a particle flat (horizontal).

void CParticule::DrawParticuleFlat(int i)
{
    CObject*        object;
    D3DVERTEX2      vertex[4];  // 2 triangles
    D3DMATRIX       matrix;
    D3DVECTOR       corner[4], pos, n, angle, eye;
    FPOINT          dim;

    if ( m_particule[i].zoom == 0.0f )  return;
    if ( m_particule[i].intensity == 0.0f )  return;

    pos = m_particule[i].pos;

    object = m_particule[i].objLink;
    if ( object != 0 )
    {
        pos += object->RetPosition(0);
    }

    angle.x = PI/2.0f;
    angle.y = 0.0f;
    angle.z = m_particule[i].angle;

#if 0
    if ( m_engine->RetRankView() == 1 )  // underwater?
    {
        angle.x = -PI/2.0f;
        pos.y -= 1.0f;
    }
#else
    if ( m_engine->RetRankView() == 1 )  // underwater?
    {
        pos.y -= 1.0f;
    }

    eye = m_engine->RetEyePt();
    if ( pos.y > eye.y )  // seen from below?
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

// Draw a particle to a flat sheet of fog.

void CParticule::DrawParticuleFog(int i)
{
    CObject*        object;
    D3DVERTEX2      vertex[4];  // 2 triangles
    D3DMATRIX       matrix;
    D3DVECTOR       corner[4], pos, n, angle, eye;
    FPOINT          dim, zoom;

    if ( !m_engine->RetFog() )  return;
    if ( m_particule[i].intensity == 0.0f )  return;

    pos = m_particule[i].pos;

    dim.x = m_particule[i].dim.x;
    dim.y = m_particule[i].dim.y;

    if ( m_particule[i].type == PARTIFOG0 ||
         m_particule[i].type == PARTIFOG2 ||
         m_particule[i].type == PARTIFOG4 ||
         m_particule[i].type == PARTIFOG6 )
    {
//?     pos.x += sinf(m_particule[i].zoom*1.2f)*dim.x*0.1f;
//?     pos.y += cosf(m_particule[i].zoom*1.5f)*dim.y*0.1f;
        zoom.x = 1.0f+sinf(m_particule[i].zoom*2.0f)/6.0f;
        zoom.y = 1.0f+cosf(m_particule[i].zoom*2.7f)/6.0f;
    }
    if ( m_particule[i].type == PARTIFOG1 ||
         m_particule[i].type == PARTIFOG3 ||
         m_particule[i].type == PARTIFOG5 ||
         m_particule[i].type == PARTIFOG7 )
    {
//?     pos.x += sinf(m_particule[i].zoom*1.0f)*dim.x*0.1f;
//?     pos.y += cosf(m_particule[i].zoom*1.3f)*dim.y*0.1f;
        zoom.x = 1.0f+sinf(m_particule[i].zoom*3.0f)/6.0f;
        zoom.y = 1.0f+cosf(m_particule[i].zoom*3.7f)/6.0f;
    }

    dim.x *= zoom.x;
    dim.y *= zoom.y;

    object = m_particule[i].objLink;
    if ( object != 0 )
    {
        pos += object->RetPosition(0);
    }

    angle.x = PI/2.0f;
    angle.y = 0.0f;
    angle.z = m_particule[i].angle;

    if ( m_engine->RetRankView() == 1 )  // underwater?
    {
        pos.y -= 1.0f;
    }

    eye = m_engine->RetEyePt();
    if ( pos.y > eye.y )  // seen from below?
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

// Draw a particle in the form of radius.

void CParticule::DrawParticuleRay(int i)
{
    CObject*        object;
    D3DVERTEX2      vertex[4];  // 2 triangles
    D3DMATRIX       matrix;
    D3DVECTOR       corner[4], eye, pos, goal, n, angle, proj;
    FPOINT          dim, texInf, texSup;
    BOOL            bLeft;
    float           a, len, adv, prop, vario1, vario2;
    int             r, rank, step, first, last;

    if ( m_particule[i].zoom == 0.0f )  return;
    if ( m_particule[i].intensity == 0.0f )  return;

    eye = m_engine->RetEyePt();
    pos = m_particule[i].pos;
    goal = m_particule[i].goal;

    object = m_particule[i].objLink;
    if ( object != 0 )
    {
        pos += object->RetPosition(0);
    }

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

// Draws a spherical particle.

void CParticule::DrawParticuleSphere(int i)
{
    D3DVERTEX2      vertex[2*16*(16+1)];  // triangles
    D3DMATRIX       matrix, rot;
    D3DVECTOR       angle, v0, v1;
    FPOINT          ts, ti;
    float           zoom, deltaRingAngle, deltaSegAngle;
    float           r0,r1, tu0,tv0, tu1,tv1;
    int             j, ring, seg, numRings, numSegments;

    zoom = m_particule[i].zoom;
#if 0
    if ( !m_engine->RetStateColor() && m_particule[i].intensity < 0.5f )
    {
        zoom *= m_particule[i].intensity/0.5f;
    }
#endif

    if ( zoom == 0.0f )  return;

    m_engine->SetState(D3DSTATETTb|D3DSTATE2FACE|D3DSTATEWRAP, RetColor(m_particule[i].intensity));

    D3DUtil_SetIdentityMatrix(matrix);
    matrix._11 = zoom;
    matrix._22 = zoom;
    matrix._33 = zoom;
    matrix._41 = m_particule[i].pos.x;
    matrix._42 = m_particule[i].pos.y;
    matrix._43 = m_particule[i].pos.z;

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
        numRings    = 16;
        numSegments = 16;
    }
    else
    {
        numRings    = 8;
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

// Returns the height depending on the progress.

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

// Draws a cylindrical particle.

void CParticule::DrawParticuleCylinder(int i)
{
    D3DVERTEX2      vertex[2*5*(10+1)];  // triangles
    D3DMATRIX       matrix, rot;
    D3DVECTOR       angle, v0, v1;
    FPOINT          ts, ti;
    float           progress, zoom, diam, deltaSegAngle, h[6], d[6];
    float           r0,r1, tu0,tv0, tu1,tv1, p1, p2, pp;
    int             j, ring, seg, numRings, numSegments;

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
            p1 = progress/0.5f;  // front
            p2 = 0.0f;  // back
        }
        else
        {
            p1 = 1.0f;  // front
            p2 = (progress-0.5f)/0.5f;  // back
            ts.y += (ti.y-ts.y)*p2;
        }
#else
        p1 = progress;  // front
        p2 = powf(progress, 5.0f);  // back
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
        r0   = 1.0f*d[ring+0];  // radius at the base
        r1   = 1.0f*d[ring+1];  // radius at the top
        v0.y = 1.0f*h[ring+0];  // bottom
        v1.y = 1.0f*h[ring+1];  // top

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

//?         tu0 = ((float)seg)/numSegments;
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

// Draws a tire mark.

void CParticule::DrawParticuleWheel(int i)
{
    D3DVECTOR   pos[4], center;
    D3DVERTEX2  vertex[4];  // 2 triangles
    D3DVECTOR   n;
    FPOINT      ts, ti;
    float       dist, dp;

    dist = Length2d(m_engine->RetEyePt(), m_wheelTrace[i].pos[0]);
    if ( dist > 300.0f )  return;

    pos[0] = m_wheelTrace[i].pos[0];
    pos[1] = m_wheelTrace[i].pos[1];
    pos[2] = m_wheelTrace[i].pos[2];
    pos[3] = m_wheelTrace[i].pos[3];

    if ( m_wheelTrace[i].type == PARTITRACE0 )  // white ground track?
    {
        ts.x =   8.0f/256.0f;
        ts.y = 224.0f/256.0f;
    }
    else if ( m_wheelTrace[i].type == PARTITRACE1 )  // black ground track?
    {
        ts.x =   0.0f/256.0f;
        ts.y = 224.0f/256.0f;
    }
    else if ( m_wheelTrace[i].type == PARTITRACE2 )  // gray ground track?
    {
        ts.x =   0.0f/256.0f;
        ts.y = 232.0f/256.0f;
    }
    else if ( m_wheelTrace[i].type == PARTITRACE3 )  // light gray ground track?
    {
        ts.x =   8.0f/256.0f;
        ts.y = 232.0f/256.0f;
    }
    else if ( m_wheelTrace[i].type == PARTITRACE4 )  // red ground track?
    {
        ts.x =  32.0f/256.0f;
        ts.y = 224.0f/256.0f;
    }
    else if ( m_wheelTrace[i].type == PARTITRACE5 )  // pink ground track?
    {
        ts.x =  40.0f/256.0f;
        ts.y = 224.0f/256.0f;
    }
    else if ( m_wheelTrace[i].type == PARTITRACE6 )  // violet ground track?
    {
        ts.x =  32.0f/256.0f;
        ts.y = 232.0f/256.0f;
    }
    else if ( m_wheelTrace[i].type == PARTITRACE7 )  // orange ground track?
    {
        ts.x =  40.0f/256.0f;
        ts.y = 232.0f/256.0f;
    }
    else if ( m_wheelTrace[i].type == PARTITRACE8 )  // yellow ground track?
    {
        ts.x =  16.0f/256.0f;
        ts.y = 224.0f/256.0f;
    }
    else if ( m_wheelTrace[i].type == PARTITRACE9 )  // beige ground track?
    {
        ts.x =  24.0f/256.0f;
        ts.y = 224.0f/256.0f;
    }
    else if ( m_wheelTrace[i].type == PARTITRACE10 )  // brown ground track?
    {
        ts.x =  16.0f/256.0f;
        ts.y = 232.0f/256.0f;
    }
    else if ( m_wheelTrace[i].type == PARTITRACE11 )  // skin ground track?
    {
        ts.x =  24.0f/256.0f;
        ts.y = 232.0f/256.0f;
    }
    else if ( m_wheelTrace[i].type == PARTITRACE12 )  // green ground track?
    {
        ts.x =  48.0f/256.0f;
        ts.y = 224.0f/256.0f;
    }
    else if ( m_wheelTrace[i].type == PARTITRACE13 )  // light green ground track?
    {
        ts.x =  56.0f/256.0f;
        ts.y = 224.0f/256.0f;
    }
    else if ( m_wheelTrace[i].type == PARTITRACE14 )  // blue ground track?
    {
        ts.x =  48.0f/256.0f;
        ts.y = 232.0f/256.0f;
    }
    else if ( m_wheelTrace[i].type == PARTITRACE15 )  // light blue ground track?
    {
        ts.x =  56.0f/256.0f;
        ts.y = 232.0f/256.0f;
    }
    else if ( m_wheelTrace[i].type == PARTITRACE16 )  // black arrow ground track?
    {
        ts.x = 160.0f/256.0f;
        ts.y = 224.0f/256.0f;
    }
    else if ( m_wheelTrace[i].type == PARTITRACE17 )  // red arrow ground track?
    {
        ts.x = 176.0f/256.0f;
        ts.y = 224.0f/256.0f;
    }
    else
    {
        return;
    }

    if ( m_wheelTrace[i].type == PARTITRACE16 ||
         m_wheelTrace[i].type == PARTITRACE17 )
    {
        ti.x = ts.x+16.0f/256.0f;
        ti.y = ts.y+16.0f/256.0f;
    }
    else
    {
        ti.x = ts.x+8.0f/256.0f;
        ti.y = ts.y+8.0f/256.0f;
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

// Draws all the particles.

void CParticule::DrawParticule(int sheet)
{
    D3DMATERIAL7    mat;
    D3DMATRIX       matrix;
    BOOL            bLoadTexture;
    char            name[20];
    int             state, t, i, j, r;

    m_pD3DDevice->SetRenderState(D3DRENDERSTATE_AMBIENT, 0xffffffff);
    m_pD3DDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, TRUE);
//? m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE);
    m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);

    // Draw the basic particles of triangles.
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

    // Draw the particles was calculated based on edge.
    m_pD3DDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE);

    ZeroMemory( &mat, sizeof(D3DMATERIAL7) );
    mat.diffuse.r = 1.0f;
    mat.diffuse.g = 1.0f;
    mat.diffuse.b = 1.0f;  // white
    mat.ambient.r = 0.5f;
    mat.ambient.g = 0.5f;
    mat.ambient.b = 0.5f;
    m_engine->SetMaterial(mat);

    // Draw tire marks.
    if ( m_wheelTraceTotal > 0 && sheet == SH_WORLD )
    {
#if _POLISH
        m_engine->SetTexture("textp.tga");
#else
        m_engine->SetTexture("text.tga");
#endif
        m_engine->SetState(D3DSTATETTw);
//?     m_engine->SetState(D3DSTATENORMAL);
        D3DUtil_SetIdentityMatrix(matrix);
        m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &matrix);
        for ( i=0 ; i<m_wheelTraceTotal ; i++ )
        {
            DrawParticuleWheel(i);
        }
    }

//? for ( t=1 ; t<MAXPARTITYPE ; t++ )
    for ( t=MAXPARTITYPE-1 ; t>=1 ; t-- )  // black behind!
    {
        if ( m_totalInterface[t][sheet] == 0 )  continue;

        bLoadTexture = FALSE;

        if ( t == 4 )  state = D3DSTATETTw;  // text.tga
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
                TrackDraw(r, m_particule[i].type);  // draws the drag
                if ( !m_track[r].bDrawParticule )  continue;
            }

            m_engine->SetState(state, RetColor(m_particule[i].intensity));

            if ( m_particule[i].bRay )  // ray?
            {
                DrawParticuleRay(i);
            }
            else if ( m_particule[i].type == PARTIFLIC  ||  // circle in the water?
                      m_particule[i].type == PARTISHOW  ||
                      m_particule[i].type == PARTICHOC  ||
                      m_particule[i].type == PARTIGFLAT )
            {
                DrawParticuleFlat(i);
            }
            else if ( m_particule[i].type >= PARTIFOG0 &&
                      m_particule[i].type <= PARTIFOG9 )
            {
                DrawParticuleFog(i);
            }
            else if ( m_particule[i].type >= PARTISPHERE0 &&
                      m_particule[i].type <= PARTISPHERE9 )  // sphere?
            {
                DrawParticuleSphere(i);
            }
            else if ( m_particule[i].type >= PARTIPLOUF0 &&
                      m_particule[i].type <= PARTIPLOUF4 )  // cylinder?
            {
                DrawParticuleCylinder(i);
            }
            else    // normal?
            {
                DrawParticuleNorm(i);
            }
        }
    }

//? m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE);
    m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
}


// Seeks if an object collided with a bullet.

CObject* CParticule::SearchObjectGun(D3DVECTOR old, D3DVECTOR pos,
                                     ParticuleType type, CObject *father)
{
    CObject     *pObj, *pBest;
    D3DVECTOR   box1, box2, oPos, p;
    ObjectType  oType;
    BOOL        bShield;
    float       min, oRadius, dist, shieldRadius;
    int         i, j;
    BOOL        bHimself;

    if ( m_main->RetMovieLock() )  return 0;  // current movie?

    bHimself = m_main->RetHimselfDamage();

    min = 5.0f;
    if ( type == PARTIGUN2 ) min = 2.0f;  // shooting insect?
    if ( type == PARTIGUN3 ) min = 3.0f;  // suiciding spider?

    box1 = old;
    box2 = pos;
    if ( box1.x > box2.x )  Swap(box1.x, box2.x);  // box1 < box2
    if ( box1.y > box2.y )  Swap(box1.y, box2.y);
    if ( box1.z > box2.z )  Swap(box1.z, box2.z);
    box1.x -= min;
    box1.y -= min;
    box1.z -= min;
    box2.x += min;
    box2.y += min;
    box2.z += min;

    pBest = 0;
    bShield = FALSE;
    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        if ( !pObj->RetActif() )  continue;  // inactive?
        if ( pObj == father )  continue;

        oType = pObj->RetType();

        if ( oType == OBJECT_TOTO )  continue;

        if ( type == PARTIGUN1 )  // fireball shooting?
        {
            if ( oType == OBJECT_MOTHER )  continue;
            if ( bHimself )  // damage is oneself?
            {
                if ( !IsAlien(oType) &&
                     !IsSoft(oType)  )  continue;
            }
            else    // damage only to enemies?
            {
                if ( !IsAlien(oType) )  continue;
            }
        }
        else if ( type == PARTIGUN2 )  // shooting insect?
        {
            if ( !IsSoft(oType) )  continue;
        }
        else if ( type == PARTIGUN3 )  // suiciding spider?
        {
            if ( !IsSoft(oType) )  continue;
        }
        else if ( type == PARTIGUN4 )  // orgaball shooting?
        {
            if ( oType == OBJECT_MOTHER )  continue;
            if ( bHimself )  // damage is oneself?
            {
                if ( !IsAlien(oType) &&
                     !IsSoft(oType)  )  continue;
            }
            else    // damage only to enemies?
            {
                if ( !IsAlien(oType) )  continue;
            }
        }
        else if ( type == PARTITRACK11 )  // phazer shooting?
        {
            if ( bHimself )  // damage is oneself?
            {
                if ( !IsAlien(oType) &&
                     !IsSoft(oType)  )  continue;
            }
            else    // damage only to enemies?
            {
                if ( !IsAlien(oType) )  continue;
            }
        }
        else
        {
            continue;
        }

        oPos = pObj->RetPosition(0);

        if ( type == PARTIGUN2 ||  // shooting insect?
             type == PARTIGUN3 )   // suiciding spider?
        {
            // Test if the ball is entered into the sphere of a shield.
            shieldRadius = pObj->RetShieldRadius();
            if ( shieldRadius > 0.0f )
            {
                dist = Length(oPos, pos);
                if ( dist <= shieldRadius )
                {
                    pBest = pObj;
                    bShield = TRUE;
                }
            }
        }
        if ( bShield )  continue;

        // Test the center of the object, which is necessary for objects
        // that have no sphere in the center (station).
        dist = Length(oPos, pos)-4.0f;
        if ( dist < min )
        {
            pBest = pObj;
        }

        // Test with all spheres of the object.
        j = 0;
        while ( pObj->GetCrashSphere(j++, oPos, oRadius) )
        {
            if ( oPos.x+oRadius < box1.x || oPos.x-oRadius > box2.x ||  // outside the box?
                 oPos.y+oRadius < box1.y || oPos.y-oRadius > box2.y ||
                 oPos.z+oRadius < box1.z || oPos.z-oRadius > box2.z )  continue;

            p = Projection(old, pos, oPos);
            dist = Length(p, oPos)-oRadius;
            if ( dist < min )
            {
                pBest = pObj;
            }
        }
    }

    return pBest;
}

// Seeks if an object collided with a ray.

CObject* CParticule::SearchObjectRay(D3DVECTOR pos, D3DVECTOR goal,
                                     ParticuleType type, CObject *father)
{
    CObject*    pObj;
    D3DVECTOR   box1, box2, oPos, p;
    ObjectType  oType;
    float       min, dist;
    int         i;

    if ( m_main->RetMovieLock() )  return 0;  // current movie?

    min = 10.0f;

    box1 = pos;
    box2 = goal;
    if ( box1.x > box2.x )  Swap(box1.x, box2.x);  // box1 < box2
    if ( box1.y > box2.y )  Swap(box1.y, box2.y);
    if ( box1.z > box2.z )  Swap(box1.z, box2.z);
    box1.x -= min;
    box1.y -= min;
    box1.z -= min;
    box2.x += min;
    box2.y += min;
    box2.z += min;

    for ( i=0 ; i<1000000 ; i++ )
    {
        pObj = (CObject*)m_iMan->SearchInstance(CLASS_OBJECT, i);
        if ( pObj == 0 )  break;

        if ( !pObj->RetActif() )  continue;  // inactive?
        if ( pObj == father )  continue;

        oType = pObj->RetType();

        if ( oType == OBJECT_TOTO )  continue;

        if ( type  == PARTIRAY1       &&
             oType != OBJECT_MOBILEtg &&
             oType != OBJECT_TEEN28   &&
             oType != OBJECT_TEEN31   &&
             oType != OBJECT_ANT      &&
             oType != OBJECT_SPIDER   &&
             oType != OBJECT_BEE      &&
             oType != OBJECT_WORM     &&
             oType != OBJECT_MOTHER   &&
             oType != OBJECT_NEST     )  continue;

        oPos = pObj->RetPosition(0);

        if ( oPos.x < box1.x || oPos.x > box2.x ||  // outside the box?
             oPos.y < box1.y || oPos.y > box2.y ||
             oPos.z < box1.z || oPos.z > box2.z )  continue;

        p = Projection(pos, goal, oPos);
        dist = Length(p, oPos);
        if ( dist < min )  return pObj;
    }

    return 0;
}


// Sounded one.

void CParticule::Play(Sound sound, D3DVECTOR pos, float amplitude)
{
    if ( m_sound == 0 )
    {
        m_sound = (CSound*)m_iMan->SearchInstance(CLASS_SOUND);
    }

    m_sound->Play(sound, pos, amplitude);
}



// Seeks the color if you're in the fog.
// Returns black if you're not in the fog.

D3DCOLORVALUE CParticule::RetFogColor(D3DVECTOR pos)
{
    D3DCOLORVALUE   result, color;
    float           dist, factor;
    int             fog, i;

    result.r = 0.0f;
    result.g = 0.0f;
    result.b = 0.0f;
    result.a = 0.0f;

    for ( fog=0 ; fog<m_fogTotal ; fog++ )
    {
        i = m_fog[fog];  // i = rank of the particle

        if ( pos.y >= m_particule[i].pos.y+FOG_HSUP )  continue;
        if ( pos.y <= m_particule[i].pos.y-FOG_HINF )  continue;

        dist = Length2d(pos, m_particule[i].pos);
        if ( dist >= m_particule[i].dim.x*1.5f )  continue;

        // Calculates the horizontal distance.
        factor = 1.0f-powf(dist/(m_particule[i].dim.x*1.5f), 4.0f);

        // Calculates the vertical distance.
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
             m_particule[i].type == PARTIFOG1 )  // blue?
        {
            color.r = 0.0f;
            color.g = 0.5f;
            color.b = 1.0f;
        }
        else if ( m_particule[i].type == PARTIFOG2 ||
                  m_particule[i].type == PARTIFOG3 )  // red?
        {
            color.r = 2.0f;
            color.g = 1.0f;
            color.b = 0.0f;
        }
        else if ( m_particule[i].type == PARTIFOG4 ||
                  m_particule[i].type == PARTIFOG5 )  // white?
        {
            color.r = 1.0f;
            color.g = 1.0f;
            color.b = 1.0f;
        }
        else if ( m_particule[i].type == PARTIFOG6 ||
                  m_particule[i].type == PARTIFOG7 )  // yellow?
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


// Writes a file. BMP containing all the tire tracks.

BOOL CParticule::WriteWheelTrace(char *filename, int width, int height,
                                 D3DVECTOR dl, D3DVECTOR ur)
{
    HDC             hDC;
    HDC             hDCImage;
    HBITMAP         hb;
    PBITMAPINFO     info;
    HBRUSH          hBrush;
    HPEN            hPen;
    HGDIOBJ         old;
    RECT            rect;
    COLORREF        color;
    FPOINT          pos[4];
    POINT           list[4];
    int             i;

    if ( !m_engine->GetRenderDC(hDC) )  return FALSE;

    hDCImage = CreateCompatibleDC(hDC);
    if ( hDCImage == 0 )
    {
        m_engine->ReleaseRenderDC(hDC);
        return FALSE;
    }

    hb = CreateCompatibleBitmap(hDC, width, height);
    if ( hb == 0 )
    {
        DeleteDC(hDCImage);
        m_engine->ReleaseRenderDC(hDC);
        return FALSE;
    }

    SelectObject(hDCImage, hb);

    rect.left   = 0;
    rect.right  = width;
    rect.top    = 0;
    rect.bottom = height;
    FillRect(hDCImage, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));

    hPen = CreatePen(PS_NULL, 1, 0);
    SelectObject(hDCImage, hPen);

    for ( i=0 ; i<m_wheelTraceTotal ; i++ )
    {
        if ( m_wheelTrace[i].type == PARTITRACE0 )  // black ground track?
        {
            color = RGB(0,0,0);
        }
        else if ( m_wheelTrace[i].type == PARTITRACE1 )  // red ground track?
        {
            color = RGB(255,0,0);
        }
        else if ( m_wheelTrace[i].type == PARTITRACE2 )  // green ground track?
        {
            color = RGB(0,255,0);
        }
        else if ( m_wheelTrace[i].type == PARTITRACE3 )  // blue ground track?
        {
            color = RGB(0,0,255);
        }
        else if ( m_wheelTrace[i].type == PARTITRACE4 )  // cyan ground track?
        {
            color = RGB(0,255,255);
        }
        else if ( m_wheelTrace[i].type == PARTITRACE5 )  // magenta ground track?
        {
            color = RGB(255,0,255);
        }
        else if ( m_wheelTrace[i].type == PARTITRACE6 )  // yellow ground track?
        {
            color = RGB(255,255,0);
        }
        else
        {
            color = RGB(0,0,0);
        }
        hBrush = CreateSolidBrush(color);
        old = SelectObject(hDCImage, hBrush);

        pos[0].x = ((m_wheelTrace[i].pos[0].x-dl.x)*width)/(ur.x-dl.x);
        pos[0].y = ((m_wheelTrace[i].pos[0].z-dl.z)*width)/(ur.z-dl.z);
        pos[1].x = ((m_wheelTrace[i].pos[1].x-dl.x)*width)/(ur.x-dl.x);
        pos[1].y = ((m_wheelTrace[i].pos[1].z-dl.z)*width)/(ur.z-dl.z);
        pos[2].x = ((m_wheelTrace[i].pos[2].x-dl.x)*width)/(ur.x-dl.x);
        pos[2].y = ((m_wheelTrace[i].pos[2].z-dl.z)*width)/(ur.z-dl.z);
        pos[3].x = ((m_wheelTrace[i].pos[3].x-dl.x)*width)/(ur.x-dl.x);
        pos[3].y = ((m_wheelTrace[i].pos[3].z-dl.z)*width)/(ur.z-dl.z);

        list[0].x = (int)pos[0].x;
        list[0].y = (int)pos[0].y;
        list[1].x = (int)pos[1].x;
        list[1].y = (int)pos[1].y;
        list[2].x = (int)pos[3].x;
        list[2].y = (int)pos[3].y;
        list[3].x = (int)pos[2].x;
        list[3].y = (int)pos[2].y;
        Polygon(hDCImage, list, 4);

        if ( old != 0 )  SelectObject(hDCImage, old);
        DeleteObject(hBrush);
    }

    info = m_engine->CreateBitmapInfoStruct(hb);
    if ( info == 0 )
    {
        DeleteObject(hb);
        DeleteDC(hDCImage);
        m_engine->ReleaseRenderDC(hDC);
        return FALSE;
    }

    m_engine->CreateBMPFile(filename, info, hb, hDCImage);

    DeleteObject(hb);
    DeleteDC(hDCImage);
    m_engine->ReleaseRenderDC(hDC);
    return TRUE;
}

