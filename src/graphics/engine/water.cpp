// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// * Copyright (C) 2012, Polish Portal of Colobot (PPC)
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

#include "graphics/engine/water.h"

#include "common/iman.h"
#include "graphics/engine/engine.h"
#include "graphics/engine/terrain.h"
#include "sound/sound.h"


const int WATERLINE_PREALLOCATE_COUNT = 500;

// TODO: remove the limit?
const int VAPOR_SIZE = 10;


Gfx::CWater::CWater(CInstanceManager* iMan, Gfx::CEngine* engine)
{
    m_iMan = iMan;
    m_iMan->AddInstance(CLASS_WATER, this);

    m_engine = engine;
    m_terrain = nullptr;
    m_particule = nullptr;
    m_sound = nullptr;

    m_type[0] = WATER_NULL;
    m_type[1] = WATER_NULL;
    m_level = 0.0f;
    m_draw = true;
    m_lava = false;
    m_color = 0xffffffff;
    m_subdiv = 4;

    m_line.reserve(WATERLINE_PREALLOCATE_COUNT);

    std::vector<Gfx::WaterVapor>(VAPOR_SIZE).swap(m_vapor);
}

Gfx::CWater::~CWater()
{
    m_iMan = nullptr;
    m_engine = nullptr;
    m_terrain = nullptr;
    m_particule = nullptr;
    m_sound = nullptr;
}


bool Gfx::CWater::EventProcess(const Event &event)
{
    if (event.type == EVENT_FRAME)
        return EventFrame(event);

    return true;
}

bool Gfx::CWater::EventFrame(const Event &event)
{
    if (m_engine->GetPause())  return true;

    m_time += event.rTime;

    if (m_type[0] == WATER_NULL)  return true;

    if (m_lava)
        LavaFrame(event.rTime);

    return true;
}

void Gfx::CWater::LavaFrame(float rTime)
{
    if (m_particule == nullptr)
        m_particule = static_cast<Gfx::CParticle*>( m_iMan->SearchInstance(CLASS_PARTICULE) );

    for (int i = 0; i < static_cast<int>( m_vapor.size() ); i++)
        VaporFrame(i, rTime);

    if (m_time - m_lastLava >= 0.1f)
    {
        Math::Vector eye    = m_engine->GetEyePt();
        Math::Vector lookat = m_engine->GetLookatPt();

        float distance = Math::Rand()*200.0f;
        float shift = (Math::Rand()-0.5f)*200.0f;

        Math::Vector dir = Normalize(lookat-eye);
        Math::Vector pos = eye + dir*distance;

        Math::Vector perp;
        perp.x = -dir.z;
        perp.y =  dir.y;
        perp.z =  dir.x;
        pos = pos + perp*shift;

        float level = m_terrain->GetFloorLevel(pos, true);
        if (level < m_level)
        {
            pos.y = m_level;

            level = Math::Rand();
            if (level < 0.8f)
            {
                if ( VaporCreate(Gfx::PARTIFIRE, pos, 0.02f+Math::Rand()*0.06f) )
                    m_lastLava = m_time;
            }
            else if (level < 0.9f)
            {
                if ( VaporCreate(Gfx::PARTIFLAME, pos, 0.5f+Math::Rand()*3.0f) )
                    m_lastLava = m_time;
            }
            else
            {
                if ( VaporCreate(Gfx::PARTIVAPOR, pos, 0.2f+Math::Rand()*2.0f) )
                    m_lastLava = m_time;
            }
        }
    }
}

void Gfx::CWater::VaporFlush()
{
    m_vapor.clear();
}

bool Gfx::CWater::VaporCreate(Gfx::ParticleType type, Math::Vector pos, float delay)
{
    for (int i = 0; i < static_cast<int>( m_vapor.size() ); i++)
    {
        if (! m_vapor[i].used)
        {
            m_vapor[i].used = true;
            m_vapor[i].type  = type;
            m_vapor[i].pos   = pos;
            m_vapor[i].delay = delay;
            m_vapor[i].time  = 0.0f;
            m_vapor[i].last  = 0.0f;

            if (m_vapor[i].type == PARTIFIRE)
                m_sound->Play(SOUND_BLUP, pos, 1.0f, 1.0f-Math::Rand()*0.5f);

            if (m_vapor[i].type == PARTIVAPOR)
                m_sound->Play(SOUND_PSHHH, pos, 0.3f, 2.0f);

            return true;
        }
    }

    return false;
}

void Gfx::CWater::VaporFrame(int i, float rTime)
{
    m_vapor[i].time += rTime;

    if (m_sound == nullptr)
        m_sound = static_cast<CSoundInterface*>(m_iMan->SearchInstance(CLASS_SOUND));

    if (m_vapor[i].time <= m_vapor[i].delay)
    {
        if (m_time-m_vapor[i].last >= m_engine->ParticleAdapt(0.02f))
        {
            m_vapor[i].last = m_time;

            if (m_vapor[i].type == PARTIFIRE)
            {
                for (int j = 0; j < 10; j++)
                {
                    Math::Vector pos = m_vapor[i].pos;
                    pos.x += (Math::Rand()-0.5f)*2.0f;
                    pos.z += (Math::Rand()-0.5f)*2.0f;
                    pos.y -= 1.0f;
                    Math::Vector speed;
                    speed.x = (Math::Rand()-0.5f)*6.0f;
                    speed.z = (Math::Rand()-0.5f)*6.0f;
                    speed.y = 8.0f+Math::Rand()*5.0f;
                    Math::Point dim;
                    dim.x = Math::Rand()*1.5f+1.5f;
                    dim.y = dim.x;
                    m_particule->CreateParticle(pos, speed, dim, PARTIERROR, 2.0f, 10.0f);
                }
            }
            else if (m_vapor[i].type == PARTIFLAME)
            {
                Math::Vector pos = m_vapor[i].pos;
                pos.x += (Math::Rand()-0.5f)*8.0f;
                pos.z += (Math::Rand()-0.5f)*8.0f;
                pos.y -= 2.0f;
                Math::Vector speed;
                speed.x = (Math::Rand()-0.5f)*2.0f;
                speed.z = (Math::Rand()-0.5f)*2.0f;
                speed.y = 4.0f+Math::Rand()*4.0f;
                Math::Point dim;
                dim.x = Math::Rand()*2.0f+2.0f;
                dim.y = dim.x;
                m_particule->CreateParticle(pos, speed, dim, PARTIFLAME);
            }
            else
            {
                Math::Vector pos = m_vapor[i].pos;
                pos.x += (Math::Rand()-0.5f)*4.0f;
                pos.z += (Math::Rand()-0.5f)*4.0f;
                pos.y -= 2.0f;
                Math::Vector speed;
                speed.x = (Math::Rand()-0.5f)*2.0f;
                speed.z = (Math::Rand()-0.5f)*2.0f;
                speed.y = 8.0f+Math::Rand()*8.0f;
                Math::Point dim;
                dim.x = Math::Rand()*1.0f+1.0f;
                dim.y = dim.x;
                m_particule->CreateParticle(pos, speed, dim, PARTIVAPOR);
            }
        }
    }
    else
    {
        m_vapor[i].used = false;
    }
}

void Gfx::CWater::AdjustLevel(Math::Vector &pos, Math::Vector &norm,
                              Math::Point &uv1, Math::Point &uv2)
{
    float t1 = m_time*1.5f + pos.x*0.1f * pos.z*0.2f;
    pos.y += sinf(t1)*m_eddy.y;

    t1 = m_time*1.5f;
    uv1.x = (pos.x+10000.0f)/40.0f+sinf(t1)*m_eddy.x*0.02f;
    uv1.y = (pos.z+10000.0f)/40.0f-cosf(t1)*m_eddy.z*0.02f;
    uv2.x = (pos.x+10010.0f)/20.0f+cosf(-t1)*m_eddy.x*0.02f;
    uv2.y = (pos.z+10010.0f)/20.0f-sinf(-t1)*m_eddy.z*0.02f;

    t1 = m_time*0.50f + pos.x*2.1f + pos.z*1.1f;
    float t2 = m_time*0.75f + pos.x*2.0f + pos.z*1.0f;
    norm = Math::Vector(sinf(t1)*m_glint, 1.0f, sinf(t2)*m_glint);
}

/** This surface prevents to see the sky (background) underwater! */
void Gfx::CWater::DrawBack()
{
    /* TODO!
    LPDIRECT3DDEVICE7 device;
    D3DVERTEX2      vertex[4];      // 2 triangles
    D3DMATERIAL7    material;
    Math::Matrix        matrix;
    Math::Vector        eye, lookat, n, p, p1, p2;
    Math::Point         uv1, uv2;
    float           deep, dist;

    if ( !m_bDraw )  return;
    if ( m_type[0] == WATER_NULL )  return;
    if ( m_lineUsed == 0 )  return;

    eye = m_engine->GetEyePt();
    lookat = m_engine->GetLookatPt();

    ZeroMemory( &material, sizeof(D3DMATERIAL7) );
    material.diffuse = m_diffuse;
    material.ambient = m_ambient;
    m_engine->SetMaterial(material);

    m_engine->SetTexture("", 0);

    device = m_engine->GetD3DDevice();
    device->SetRenderState(D3DRENDERSTATE_LIGHTING, false);
    device->SetRenderState(D3DRENDERSTATE_ZENABLE, false);
    device->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, false);
    m_engine->SetState(D3DSTATENORMAL);

    deep = m_engine->GetDeepView(0);
    m_engine->SetDeepView(deep*2.0f, 0);
    m_engine->SetFocus(m_engine->GetFocus());
    m_engine->UpdateMatProj();  // twice the depth of view

    matrix.LoadIdentity();
    {
        D3DMATRIX mat = MAT_TO_D3DMAT(matrix);
        device->SetTransform(D3DTRANSFORMSTATE_WORLD, &mat);
    }

    p.x = eye.x;
    p.z = eye.z;
    dist = Math::DistanceProjected(eye, lookat);
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

    vertex[0] = D3DVERTEX2(Math::Vector(p1.x, p2.y, p1.z), n, uv1.x,uv2.y);
    vertex[1] = D3DVERTEX2(Math::Vector(p1.x, p1.y, p1.z), n, uv1.x,uv1.y);
    vertex[2] = D3DVERTEX2(Math::Vector(p2.x, p2.y, p2.z), n, uv2.x,uv2.y);
    vertex[3] = D3DVERTEX2(Math::Vector(p2.x, p1.y, p2.z), n, uv2.x,uv1.y);

    device->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, 4, NULL);
    m_engine->AddStatisticTriangle(2);

    m_engine->SetDeepView(deep, 0);
    m_engine->SetFocus(m_engine->GetFocus());
    m_engine->UpdateMatProj();  // gives the initial depth of view

    device->SetRenderState(D3DRENDERSTATE_LIGHTING, true);
    device->SetRenderState(D3DRENDERSTATE_ZENABLE, true);
    device->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, false);*/
}

void Gfx::CWater::DrawSurf()
{
    /* TODO!
    LPDIRECT3DDEVICE7 device;
    D3DVERTEX2*     vertex;     // triangles
    D3DMATERIAL7    material;
    Math::Matrix        matrix;
    Math::Vector        eye, lookat, n, pos, p;
    Math::Point         uv1, uv2;
    bool            bUnder;
    DWORD           flags;
    float           deep, size, sizez, radius;
    int             rankview, i, j, u;

    if (! m_draw) return;
    if (m_type[0] == Gfx::WATER_NULL) return;
    if (m_line.empty()) return;

    vertex = (D3DVERTEX2*)malloc(sizeof(D3DVERTEX2)*(m_brick+2)*2);

    eye = m_engine->GetEyePt();
    lookat = m_engine->GetLookatPt();

    rankview = m_engine->GetRankView();
    bUnder = ( rankview == 1);

    device = m_engine->GetD3DDevice();
    device->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, false);

    matrix.LoadIdentity();
    {
        D3DMATRIX mat = MAT_TO_D3DMAT(matrix);
        device->SetTransform(D3DTRANSFORMSTATE_WORLD, &mat);
    }

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
    device->SetRenderState(D3DRENDERSTATE_FOGENABLE, true);

    size = m_size/2.0f;
    if ( bUnder )  sizez = -size;
    else           sizez =  size;

    // Draws all the lines.
    deep = m_engine->GetDeepView(0)*1.5f;

    for ( i=0 ; i<m_lineUsed ; i++ )
    {
        pos.y = m_level;
        pos.z = m_line[i].pz;
        pos.x = m_line[i].px1;

        // Visible line?
        p = pos;
        p.x += size*(m_line[i].len-1);
        radius = sqrtf(powf(size, 2.0f)+powf(size*m_line[i].len, 2.0f));
        if ( Math::Distance(p, eye) > deep+radius )  continue;

        D3DVECTOR pD3D = VEC_TO_D3DVEC(p);
        device->ComputeSphereVisibility(&pD3D, &radius, 1, 0, &flags);

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

    free(vertex);*/
}

bool Gfx::CWater::GetWater(int x, int y)
{
    x *= m_subdiv;
    y *= m_subdiv;

    float size = m_size/m_subdiv;
    float offset = m_brick*m_size/2.0f;

    for (int dy = 0; dy <= m_subdiv; dy++)
    {
        for (int dx = 0; dx <= m_subdiv; dx++)
        {
            Math::Vector pos;
            pos.x = (x+dx)*size - offset;
            pos.z = (y+dy)*size - offset;
            pos.y = 0.0f;
            float level = m_terrain->GetFloorLevel(pos, true);
            if (level < m_level+m_eddy.y)
                return true;
        }
    }
    return false;
}

void Gfx::CWater::CreateLine(int x, int y, int len)
{
    Gfx::WaterLine line;

    line.x   = x;
    line.y   = y;
    line.len = len;

    float  offset = m_brick*m_size/2.0f - m_size/2.0f;

    line.px1 = m_size* line.x - offset;
    line.px2 = m_size*(line.x+line.len) - offset;
    line.pz  = m_size* line.y - offset;

    m_line.push_back(line);
}

void Gfx::CWater::Create(Gfx::WaterType type1, Gfx::WaterType type2, const std::string& fileName,
                         Gfx::Color diffuse, Gfx::Color ambient,
                         float level, float glint, Math::Vector eddy)
{
    m_type[0]  = type1;
    m_type[1]  = type2;
    m_diffuse  = diffuse;
    m_ambient  = ambient;
    m_level    = level;
    m_glint    = glint;
    m_eddy     = eddy;
    m_time     = 0.0f;
    m_lastLava = 0.0f;
    m_fileName = fileName;

    VaporFlush();

    if (! m_fileName.empty())
        m_engine->LoadTexture(m_fileName);

    if (m_terrain == nullptr)
        m_terrain = static_cast<CTerrain*>(m_iMan->SearchInstance(CLASS_TERRAIN));

    m_brick = m_terrain->GetBrick()*m_terrain->GetMosaic();
    m_size  = m_terrain->GetSize();

    m_brick /= m_subdiv;
    m_size  *= m_subdiv;

    if (m_type[0] == WATER_NULL)
        return;

    m_line.clear();

    for (int y = 0; y < m_brick; y++)
    {
        int len = 0;
        for (int x = 0; x < m_brick; x++)
        {
            if (GetWater(x,y))  // water here?
            {
                len ++;
                if (len >= 5)
                {
                    CreateLine(x-len+1, y, len);
                    len = 0;
                }
            }
            else    // dry?
            {
                if (len != 0)
                {
                    CreateLine(x-len, y, len);
                    len = 0;
                }
            }
        }
        if (len != 0)
            CreateLine(m_brick - len, y, len);
    }
}

void Gfx::CWater::Flush()
{
    m_type[0] = Gfx::WATER_NULL;
    m_type[1] = Gfx::WATER_NULL;
    m_level = 0.0f;
    m_lava = false;
}

void Gfx::CWater::SetLevel(float level)
{
    m_level = level;

    Create(m_type[0], m_type[1], m_fileName, m_diffuse, m_ambient,
           m_level, m_glint, m_eddy);
}

float Gfx::CWater::GetLevel()
{
    return m_level;
}

float Gfx::CWater::GetLevel(CObject* object)
{
    /* TODO!
    ObjectType type = object->GetType();

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
*/
    return m_level;
}

void Gfx::CWater::SetLava(bool lava)
{
    m_lava = lava;
}

bool Gfx::CWater::GetLava()
{
    return m_lava;
}

void Gfx::CWater::AdjustEye(Math::Vector &eye)
{
    if (m_lava)
    {
        if (eye.y < m_level+2.0f)
            eye.y = m_level+2.0f;  // never under the lava
    }
    else
    {
        if (eye.y >= m_level-2.0f &&
            eye.y <= m_level+2.0f)  // close to the surface?
            eye.y = m_level+2.0f;  // bam, well above
    }
}

