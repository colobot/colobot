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

// water.cpp

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "d3dengine.h"
#include "d3dmath.h"
#include "d3dutil.h"
#include "event.h"
#include "misc.h"
#include "iman.h"
#include "math3d.h"
#include "particule.h"
#include "terrain.h"
#include "object.h"
#include "sound.h"
#include "water.h"




// Constructor of the terrain.

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
    m_level = 0.0f;
    m_bDraw = TRUE;
    m_bLava = FALSE;
    m_color = 0xffffffff;
    m_subdiv = 4;
    m_filename[0] = 0;
}

// Destructor of the terrain.

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
        if ( event.param == 'S' )
        {
                 if ( m_subdiv == 1 )  m_subdiv = 2;
            else if ( m_subdiv == 2 )  m_subdiv = 4;
            else if ( m_subdiv == 4 )  m_subdiv = 8;
            else if ( m_subdiv == 8 )  m_subdiv = 1;
            SetLevel(m_level);
        }
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
            int     i;
            i = (m_color>>24);
            i += 0x44;
            i &= 0xff;
            i = (i<<24);
            m_color &= 0x00ffffff;
            m_color |= i;
        }
        if ( event.param == 'W' )
        {
            int     i;
            i = (m_color>>16);
            i += 0x44;
            i &= 0xff;
            i = (i<<16);
            m_color &= 0xff00ffff;
            m_color |= i;
        }
        if ( event.param == 'E' )
        {
            int     i;
            i = (m_color>>8);
            i += 0x44;
            i &= 0xff;
            i = (i<<8);
            m_color &= 0xffff00ff;
            m_color |= i;
        }
        if ( event.param == 'R' )
        {
            int     i;
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

// Makes water evolve.

BOOL CWater::EventFrame(const Event &event)
{
    if ( m_engine->RetPause() )  return TRUE;

    m_time += event.rTime;

    if ( m_type[0] == WATER_NULL )  return TRUE;

    if ( m_bLava )
    {
        LavaFrame(event.rTime);
    }
    return TRUE;
}

// Makes evolve the steam jets on the lava.

void CWater::LavaFrame(float rTime)
{
    D3DVECTOR   eye, lookat, dir, perp, pos;
    float       distance, shift, level;
    int         i;

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

        distance = Rand()*200.0f;
        shift = (Rand()-0.5f)*200.0f;

        dir = Normalize(lookat-eye);
        pos = eye + dir*distance;

        perp.x = -dir.z;
        perp.y =  dir.y;
        perp.z =  dir.x;
        pos = pos + perp*shift;

        level = m_terrain->RetFloorLevel(pos, TRUE);
        if ( level < m_level )
        {
            pos.y = m_level;

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
    }
}

// Removes all the steam jets.

void CWater::VaporFlush()
{
    int     i;

    for ( i=0 ; i<MAXWATVAPOR ; i++ )
    {
        m_vapor[i].bUsed = FALSE;
    }
}

// Creates a new steam.

BOOL CWater::VaporCreate(ParticuleType type, D3DVECTOR pos, float delay)
{
    int     i;

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
//?             m_sound->Play(SOUND_SWIM, pos, 1.0f, 1.0f-Rand()*0.5f);
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

// Makes evolve a steam jet,

void CWater::VaporFrame(int i, float rTime)
{
    D3DVECTOR   pos, speed;
    FPOINT      dim;
    int         j;

    m_vapor[i].time += rTime;

    if ( m_sound == 0 )
    {
        m_sound = (CSound*)m_iMan->SearchInstance(CLASS_SOUND);
    }

    if ( m_vapor[i].time <= m_vapor[i].delay )
    {
        if ( m_time-m_vapor[i].last >= m_engine->ParticuleAdapt(0.02f) )
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
                    m_particule->CreateParticule(pos, speed, dim, PARTIERROR, 2.0f, 10.0f);
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
            else
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
        }
    }
    else
    {
        m_vapor[i].bUsed = FALSE;
    }
}


// Adjusts the position to normal, to imitate reflections on an expanse of water at rest.

void CWater::AdjustLevel(D3DVECTOR &pos, D3DVECTOR &norm,
                         FPOINT &uv1, FPOINT &uv2)
{
#if 0
    float       t1, t2;

    uv1.x = (pos.x+10000.0f)/40.0f;
    uv1.y = (pos.z+10000.0f)/40.0f;

    t1 = m_time*1.5f + pos.x*0.1f * pos.z*0.2f;
    pos.y += sinf(t1)*m_eddy.y;

    t1 = m_time*0.6f + pos.x*0.1f * pos.z*0.2f;
    t2 = m_time*0.7f + pos.x*0.3f * pos.z*0.4f;
    pos.x += sinf(t1)*m_eddy.x;
    pos.z += sinf(t2)*m_eddy.z;

//? uv2.x = (pos.x+10000.0f)/40.0f+0.3f;
//? uv2.y = (pos.z+10000.0f)/40.0f+0.4f;
    uv2.x = (pos.x+10000.0f)/20.0f;
    uv2.y = (pos.z+10000.0f)/20.0f;

    t1 = m_time*0.7f + pos.x*5.5f + pos.z*5.6f;
    t2 = m_time*0.8f + pos.x*5.7f + pos.z*5.8f;
    norm = D3DVECTOR(sinf(t1)*m_glint, 1.0f, sinf(t2)*m_glint);
#else
    float       t1, t2;

    t1 = m_time*1.5f + pos.x*0.1f * pos.z*0.2f;
    pos.y += sinf(t1)*m_eddy.y;

    t1 = m_time*1.5f;
    uv1.x = (pos.x+10000.0f)/40.0f+sinf(t1)*m_eddy.x*0.02f;
    uv1.y = (pos.z+10000.0f)/40.0f-cosf(t1)*m_eddy.z*0.02f;
    uv2.x = (pos.x+10010.0f)/20.0f+cosf(-t1)*m_eddy.x*0.02f;
    uv2.y = (pos.z+10010.0f)/20.0f-sinf(-t1)*m_eddy.z*0.02f;

    t1 = m_time*0.50f + pos.x*2.1f + pos.z*1.1f;
    t2 = m_time*0.75f + pos.x*2.0f + pos.z*1.0f;
    norm = D3DVECTOR(sinf(t1)*m_glint, 1.0f, sinf(t2)*m_glint);
#endif
}

// Draw the back surface of the water.
// This surface prevents to see the sky (background) underwater!

void CWater::DrawBack()
{
    LPDIRECT3DDEVICE7 device;
    D3DVERTEX2      vertex[4];      // 2 triangles
    D3DMATERIAL7    material;
    D3DMATRIX       matrix;
    D3DVECTOR       eye, lookat, n, p, p1, p2;
    FPOINT          uv1, uv2;
    float           deep, dist;

    if ( !m_bDraw )  return;
    if ( m_type[0] == WATER_NULL )  return;
    if ( m_lineUsed == 0 )  return;

    eye = m_engine->RetEyePt();
    lookat = m_engine->RetLookatPt();

    ZeroMemory( &material, sizeof(D3DMATERIAL7) );
    material.diffuse = m_diffuse;
    material.ambient = m_ambient;
    m_engine->SetMaterial(material);

    m_engine->SetTexture("", 0);

    device = m_engine->RetD3DDevice();
    device->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE);
    device->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE);
    device->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
    m_engine->SetState(D3DSTATENORMAL);

    deep = m_engine->RetDeepView(0);
    m_engine->SetDeepView(deep*2.0f, 0);
    m_engine->SetFocus(m_engine->RetFocus());
    m_engine->UpdateMatProj();  // twice the depth of view

    D3DUtil_SetIdentityMatrix(matrix);
    device->SetTransform(D3DTRANSFORMSTATE_WORLD, &matrix);

    p.x = eye.x;
    p.z = eye.z;
    dist = Length2d(eye, lookat);
    p.x = (lookat.x-eye.x)*deep*1.0f/dist + eye.x;
    p.z = (lookat.z-eye.z)*deep*1.0f/dist + eye.z;

    p1.x =  (lookat.z-eye.z)*deep*2.0f/dist + p.x;
    p1.z = -(lookat.x-eye.x)*deep*2.0f/dist + p.z;
    p2.x = -(lookat.z-eye.z)*deep*2.0f/dist + p.x;
    p2.z =  (lookat.x-eye.x)*deep*2.0f/dist + p.z;

    p1.y = -50.0f;
    p2.y = m_level;

    n.x = (lookat.x-eye.x)/dist;
    n.z = (lookat.z-eye.z)/dist;
    n.y = 0.0f;

    uv1.x = uv1.y = 0.0f;
    uv2.x = uv2.y = 0.0f;

    vertex[0] = D3DVERTEX2(D3DVECTOR(p1.x, p2.y, p1.z), n, uv1.x,uv2.y);
    vertex[1] = D3DVERTEX2(D3DVECTOR(p1.x, p1.y, p1.z), n, uv1.x,uv1.y);
    vertex[2] = D3DVERTEX2(D3DVECTOR(p2.x, p2.y, p2.z), n, uv2.x,uv2.y);
    vertex[3] = D3DVERTEX2(D3DVECTOR(p2.x, p1.y, p2.z), n, uv2.x,uv1.y);

    device->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 4, NULL);
    m_engine->AddStatisticTriangle(2);

    m_engine->SetDeepView(deep, 0);
    m_engine->SetFocus(m_engine->RetFocus());
    m_engine->UpdateMatProj();  // gives the initial depth of view

    device->SetRenderState(D3DRENDERSTATE_LIGHTING, TRUE);
    device->SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE);
    device->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
}

// Draws the flat surface of the water.

void CWater::DrawSurf()
{
    LPDIRECT3DDEVICE7 device;
    D3DVERTEX2*     vertex;     // triangles
    D3DMATERIAL7    material;
    D3DMATRIX       matrix;
    D3DVECTOR       eye, lookat, n, pos, p;
    FPOINT          uv1, uv2;
    BOOL            bUnder;
    DWORD           flags;
    float           deep, size, sizez, radius;
    int             rankview, i, j, u;

    if ( !m_bDraw )  return;
    if ( m_type[0] == WATER_NULL )  return;
    if ( m_lineUsed == 0 )  return;

    vertex = (D3DVERTEX2*)malloc(sizeof(D3DVERTEX2)*(m_brick+2)*2);

    eye = m_engine->RetEyePt();
    lookat = m_engine->RetLookatPt();

    rankview = m_engine->RetRankView();
    bUnder = ( rankview == 1);

    device = m_engine->RetD3DDevice();
//? device->SetRenderState(D3DRENDERSTATE_AMBIENT, 0xffffffff);
//? device->SetRenderState(D3DRENDERSTATE_LIGHTING, TRUE);
//? device->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE);
    device->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);

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

    size = m_size/2.0f;
    if ( bUnder )  sizez = -size;
    else           sizez =  size;

    // Draws all the lines.
    deep = m_engine->RetDeepView(0)*1.5f;

    for ( i=0 ; i<m_lineUsed ; i++ )
    {
        pos.y = m_level;
        pos.z = m_line[i].pz;
        pos.x = m_line[i].px1;

        // Visible line?
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


// Indicates if there is water in a given position.

BOOL CWater::RetWater(int x, int y)
{
    D3DVECTOR   pos;
    float       size, offset, level;
    int         dx, dy;

    x *= m_subdiv;
    y *= m_subdiv;

    size = m_size/m_subdiv;
    offset = m_brick*m_size/2.0f;

    for ( dy=0 ; dy<=m_subdiv ; dy++ )
    {
        for ( dx=0 ; dx<=m_subdiv ; dx++ )
        {
            pos.x = (x+dx)*size - offset;
            pos.z = (y+dy)*size - offset;
            pos.y = 0.0f;
            level = m_terrain->RetFloorLevel(pos, TRUE);
            if ( level < m_level+m_eddy.y )  return TRUE;
        }
    }
    return FALSE;
}

// Updates the positions, relative to the ground.

BOOL CWater::CreateLine(int x, int y, int len)
{
    float   offset;

    m_line[m_lineUsed].x   = x;
    m_line[m_lineUsed].y   = y;
    m_line[m_lineUsed].len = len;

    offset = m_brick*m_size/2.0f - m_size/2.0f;

    m_line[m_lineUsed].px1 = m_size* m_line[m_lineUsed].x - offset;
    m_line[m_lineUsed].px2 = m_size*(m_line[m_lineUsed].x+m_line[m_lineUsed].len) - offset;
    m_line[m_lineUsed].pz  = m_size* m_line[m_lineUsed].y - offset;

    m_lineUsed ++;

    return ( m_lineUsed < MAXWATERLINE );
}

// Creates all expanses of water.

BOOL CWater::Create(WaterType type1, WaterType type2, const char *filename,
                    D3DCOLORVALUE diffuse, D3DCOLORVALUE ambient,
                    float level, float glint, D3DVECTOR eddy)
{
    int         x, y, len;

    m_type[0]  = type1;
    m_type[1]  = type2;
    m_diffuse  = diffuse;
    m_ambient  = ambient;
    m_level    = level;
    m_glint    = glint;
    m_eddy     = eddy;
    m_time     = 0.0f;
    m_lastLava = 0.0f;
    strcpy(m_filename, filename);

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
    m_brick = m_terrain->RetBrick()*m_terrain->RetMosaic();
    m_size  = m_terrain->RetSize();

    m_brick /= m_subdiv;
    m_size  *= m_subdiv;

    if ( m_type[0] == WATER_NULL )  return TRUE;

    m_lineUsed = 0;
    for ( y=0 ; y<m_brick ; y++ )
    {
        len = 0;
        for ( x=0 ; x<m_brick ; x++ )
        {
            if ( RetWater(x,y) )  // water here?
            {
                len ++;
                if ( len >= 5 )
                {
                    if ( !CreateLine(x-len+1, y, len) )  return FALSE;
                    len = 0;
                }
            }
            else    // dry?
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

// Removes all the water.

void CWater::Flush()
{
    m_type[0] = WATER_NULL;
    m_type[1] = WATER_NULL;
    m_level = 0.0f;
    m_bLava = FALSE;
}


// Changes the level of the water.

BOOL CWater::SetLevel(float level)
{
    m_level = level;

    return Create(m_type[0], m_type[1], m_filename, m_diffuse, m_ambient,
                  m_level, m_glint, m_eddy);
}

// Returns the current level of water.

float CWater::RetLevel()
{
    return m_level;
}

// Returns the current level of water for a given object.

float CWater::RetLevel(CObject* object)
{
    ObjectType  type;

    type = object->RetType();

    if ( type == OBJECT_HUMAN ||
         type == OBJECT_TECH  )
    {
        return m_level-3.0f;
    }

    if ( type == OBJECT_MOBILEfa ||
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
         type == OBJECT_MOBILEtg ||
         type == OBJECT_MOBILEft ||
         type == OBJECT_MOBILEtt ||
         type == OBJECT_MOBILEwt ||
         type == OBJECT_MOBILEit ||
         type == OBJECT_MOBILEdr )
    {
        return m_level-2.0f;
    }

    return m_level;
}


// Management of the mode of lava/water.

void CWater::SetLava(BOOL bLava)
{
    m_bLava = bLava;
}

BOOL CWater::RetLava()
{
    return m_bLava;
}


// Adjusts the eye of the camera, not to be in the water.

void CWater::AdjustEye(D3DVECTOR &eye)
{
    if ( m_bLava )
    {
        if ( eye.y < m_level+2.0f )
        {
            eye.y = m_level+2.0f;  // never under the lava
        }
    }
    else
    {
        if ( eye.y >= m_level-2.0f &&
             eye.y <= m_level+2.0f )  // close to the surface?
        {
            eye.y = m_level+2.0f;  // bam, well above
        }
    }
}

