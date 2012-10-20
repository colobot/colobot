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


#include "graphics/engine/water.h"

#include "common/iman.h"
#include "common/logger.h"

#include "graphics/core/device.h"
#include "graphics/engine/engine.h"
#include "graphics/engine/terrain.h"

#include "math/geometry.h"

#include "object/object.h"

#include "sound/sound.h"


// Graphics module namespace
namespace Gfx {


const int WATERLINE_PREALLOCATE_COUNT = 500;

// TODO: remove the limit?
const int VAPOR_SIZE = 10;


CWater::CWater(CInstanceManager* iMan, CEngine* engine)
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
    m_color = Color(1.0f, 1.0f, 1.0f, 1.0f);
    m_subdiv = 4;

    m_lines.reserve(WATERLINE_PREALLOCATE_COUNT);

    std::vector<WaterVapor>(VAPOR_SIZE).swap(m_vapors);
}

CWater::~CWater()
{
    m_iMan = nullptr;
    m_engine = nullptr;
    m_terrain = nullptr;
    m_particule = nullptr;
    m_sound = nullptr;
}


bool CWater::EventProcess(const Event &event)
{
    if (event.type == EVENT_FRAME)
        return EventFrame(event);

    return true;
}

bool CWater::EventFrame(const Event &event)
{
    if (m_engine->GetPause())  return true;

    m_time += event.rTime;

    if (m_type[0] == WATER_NULL)  return true;

    if (m_lava)
        LavaFrame(event.rTime);

    return true;
}

void CWater::LavaFrame(float rTime)
{
    if (m_particule == nullptr)
        m_particule = static_cast<CParticle*>( m_iMan->SearchInstance(CLASS_PARTICULE) );

    for (int i = 0; i < static_cast<int>( m_vapors.size() ); i++)
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
                if ( VaporCreate(PARTIFIRE, pos, 0.02f+Math::Rand()*0.06f) )
                    m_lastLava = m_time;
            }
            else if (level < 0.9f)
            {
                if ( VaporCreate(PARTIFLAME, pos, 0.5f+Math::Rand()*3.0f) )
                    m_lastLava = m_time;
            }
            else
            {
                if ( VaporCreate(PARTIVAPOR, pos, 0.2f+Math::Rand()*2.0f) )
                    m_lastLava = m_time;
            }
        }
    }
}

void CWater::VaporFlush()
{
    m_vapors.clear();
}

bool CWater::VaporCreate(ParticleType type, Math::Vector pos, float delay)
{
    for (int i = 0; i < static_cast<int>( m_vapors.size() ); i++)
    {
        if (! m_vapors[i].used)
        {
            m_vapors[i].used = true;
            m_vapors[i].type  = type;
            m_vapors[i].pos   = pos;
            m_vapors[i].delay = delay;
            m_vapors[i].time  = 0.0f;
            m_vapors[i].last  = 0.0f;

            if (m_vapors[i].type == PARTIFIRE)
                m_sound->Play(SOUND_BLUP, pos, 1.0f, 1.0f-Math::Rand()*0.5f);

            if (m_vapors[i].type == PARTIVAPOR)
                m_sound->Play(SOUND_PSHHH, pos, 0.3f, 2.0f);

            return true;
        }
    }

    return false;
}

void CWater::VaporFrame(int i, float rTime)
{
    m_vapors[i].time += rTime;

    if (m_sound == nullptr)
        m_sound = static_cast<CSoundInterface*>(m_iMan->SearchInstance(CLASS_SOUND));

    if (m_vapors[i].time <= m_vapors[i].delay)
    {
        if (m_time-m_vapors[i].last >= m_engine->ParticleAdapt(0.02f))
        {
            m_vapors[i].last = m_time;

            if (m_vapors[i].type == PARTIFIRE)
            {
                for (int j = 0; j < 10; j++)
                {
                    Math::Vector pos = m_vapors[i].pos;
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
            else if (m_vapors[i].type == PARTIFLAME)
            {
                Math::Vector pos = m_vapors[i].pos;
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
                Math::Vector pos = m_vapors[i].pos;
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
        m_vapors[i].used = false;
    }
}

void CWater::AdjustLevel(Math::Vector &pos, Math::Vector &norm,
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
void CWater::DrawBack()
{
    if (! m_draw) return;
    if (m_type[0] == WATER_NULL) return;
    if (m_lines.empty()) return;

    Math::Vector eye = m_engine->GetEyePt();
    Math::Vector lookat = m_engine->GetLookatPt();

    Material material;
    material.diffuse = m_diffuse;
    material.ambient = m_ambient;
    m_engine->SetMaterial(material);

    CDevice* device = m_engine->GetDevice();

    m_engine->SetState(Gfx::ENG_RSTATE_OPAQUE_COLOR);

    float deep = m_engine->GetDeepView(0);
    m_engine->SetDeepView(deep*2.0f, 0);
    m_engine->SetFocus(m_engine->GetFocus());
    m_engine->UpdateMatProj();  // twice the depth of view

    Math::Matrix matrix;
    matrix.LoadIdentity();
    device->SetTransform(TRANSFORM_WORLD, matrix);

    Math::Vector p;
    p.x = eye.x;
    p.z = eye.z;
    float dist = Math::DistanceProjected(eye, lookat);
    p.x = (lookat.x-eye.x)*deep*1.0f/dist + eye.x;
    p.z = (lookat.z-eye.z)*deep*1.0f/dist + eye.z;

    Math::Vector p1, p2;
    p1.x =  (lookat.z-eye.z)*deep*2.0f/dist + p.x;
    p1.z = -(lookat.x-eye.x)*deep*2.0f/dist + p.z;
    p2.x = -(lookat.z-eye.z)*deep*2.0f/dist + p.x;
    p2.z =  (lookat.x-eye.x)*deep*2.0f/dist + p.z;

    p1.y = -50.0f;
    p2.y = m_level;

    Gfx::Color white = Gfx::Color(1.0f, 1.0f, 1.0f, 0.0f);

    VertexCol vertices[4] =
    {
        VertexCol(Math::Vector(p1.x, p2.y, p1.z), white),
        VertexCol(Math::Vector(p1.x, p1.y, p1.z), white),
        VertexCol(Math::Vector(p2.x, p2.y, p2.z), white),
        VertexCol(Math::Vector(p2.x, p1.y, p2.z), white)
    };

    device->DrawPrimitive(PRIMITIVE_TRIANGLE_STRIP, vertices, 4);
    m_engine->AddStatisticTriangle(2);

    m_engine->SetDeepView(deep, 0);
    m_engine->SetFocus(m_engine->GetFocus());
    m_engine->UpdateMatProj();  // gives the initial depth of view
}

void CWater::DrawSurf()
{
    if (! m_draw) return;
    if (m_type[0] == WATER_NULL) return;
    if (m_lines.empty()) return;

    std::vector<VertexTex2> vertices((m_brickCount+2)*2, VertexTex2());

    Math::Vector eye = m_engine->GetEyePt();

    int rankview = m_engine->GetRankView();
    bool under = ( rankview == 1);

    CDevice* device = m_engine->GetDevice();

    Math::Matrix matrix;
    matrix.LoadIdentity();
    device->SetTransform(TRANSFORM_WORLD, matrix);

    Material material;
    material.diffuse = m_diffuse;
    material.ambient = m_ambient;
    m_engine->SetMaterial(material);

    m_engine->SetTexture(m_fileName, 0);
    m_engine->SetTexture(m_fileName, 1);

    if (m_type[rankview] == WATER_TT)
        m_engine->SetState(ENG_RSTATE_TTEXTURE_BLACK | ENG_RSTATE_DUAL_WHITE | ENG_RSTATE_WRAP, m_color);

    else if (m_type[rankview] == WATER_TO)
        m_engine->SetState(ENG_RSTATE_NORMAL | ENG_RSTATE_DUAL_WHITE | ENG_RSTATE_WRAP);

    else if (m_type[rankview] == WATER_CT)
        m_engine->SetState(ENG_RSTATE_TTEXTURE_BLACK);

    else if (m_type[rankview] == WATER_CO)
        m_engine->SetState(ENG_RSTATE_NORMAL);

    device->SetRenderState(RENDER_STATE_FOG, true);

    float size = m_brickSize/2.0f;
    float sizez = 0.0f;
    if (under) sizez = -size;
    else       sizez =  size;

    // Draws all the lines
    float deep = m_engine->GetDeepView(0)*1.5f;

    for (int i = 0; i < static_cast<int>( m_lines.size() ); i++)
    {
        Math::Vector pos;
        pos.y = m_level;
        pos.z = m_lines[i].pz;
        pos.x = m_lines[i].px1;

        // Visible line?
        Math::Vector p = pos;
        p.x += size*(m_lines[i].len-1);
        float radius = sqrtf(powf(size, 2.0f)+powf(size*m_lines[i].len, 2.0f));
        if (Math::Distance(p, eye) > deep + radius)
            continue;

        if (device->ComputeSphereVisibility(p, radius) != Gfx::FRUSTUM_PLANE_ALL)
            continue;

        int vertexIndex = 0;

        Math::Point uv1, uv2;
        Math::Vector n;

        p.x = pos.x-size;
        p.z = pos.z-sizez;
        p.y = pos.y;
        AdjustLevel(p, n, uv1, uv2);
        if (under) n.y = -n.y;
        vertices[vertexIndex++] = VertexTex2(p, n, uv1, uv2);

        p.x = pos.x-size;
        p.z = pos.z+sizez;
        p.y = pos.y;
        AdjustLevel(p, n, uv1, uv2);
        if (under)  n.y = -n.y;
        vertices[vertexIndex++] = VertexTex2(p, n, uv1, uv2);

        for (int j = 0; j < m_lines[i].len; j++)
        {
            p.x = pos.x+size;
            p.z = pos.z-sizez;
            p.y = pos.y;
            AdjustLevel(p, n, uv1, uv2);
            if (under)  n.y = -n.y;
            vertices[vertexIndex++] = VertexTex2(p, n, uv1, uv2);

            p.x = pos.x+size;
            p.z = pos.z+sizez;
            p.y = pos.y;
            AdjustLevel(p, n, uv1, uv2);
            if (under)  n.y = -n.y;
            vertices[vertexIndex++] = VertexTex2(p, n, uv1, uv2);

            pos.x += size*2.0f;
        }

        device->DrawPrimitive(PRIMITIVE_TRIANGLE_STRIP, &vertices[0], vertexIndex);
        m_engine->AddStatisticTriangle(vertexIndex - 2);
    }
}

bool CWater::GetWater(int x, int y)
{
    x *= m_subdiv;
    y *= m_subdiv;

    float size = m_brickSize/m_subdiv;
    float offset = m_brickCount*m_brickSize/2.0f;

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

void CWater::CreateLine(int x, int y, int len)
{
    WaterLine line;

    line.x   = x;
    line.y   = y;
    line.len = len;

    float  offset = m_brickCount*m_brickSize/2.0f - m_brickSize/2.0f;

    line.px1 = m_brickSize* line.x - offset;
    line.px2 = m_brickSize*(line.x+line.len) - offset;
    line.pz  = m_brickSize* line.y - offset;

    m_lines.push_back(line);
}

void CWater::Create(WaterType type1, WaterType type2, const std::string& fileName,
                    Color diffuse, Color ambient,
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

    m_brickCount = m_terrain->GetBrickCount()*m_terrain->GetMosaicCount();
    m_brickSize  = m_terrain->GetBrickSize();

    m_brickCount /= m_subdiv;
    m_brickSize  *= m_subdiv;

    if (m_type[0] == WATER_NULL)
        return;

    m_lines.clear();

    for (int y = 0; y < m_brickCount; y++)
    {
        int len = 0;
        for (int x = 0; x < m_brickCount; x++)
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
            CreateLine(m_brickCount - len, y, len);
    }
}

void CWater::Flush()
{
    m_type[0] = WATER_NULL;
    m_type[1] = WATER_NULL;
    m_level = 0.0f;
    m_lava = false;
}

void CWater::SetLevel(float level)
{
    m_level = level;

    Create(m_type[0], m_type[1], m_fileName, m_diffuse, m_ambient,
           m_level, m_glint, m_eddy);
}

float CWater::GetLevel()
{
    return m_level;
}

float CWater::GetLevel(CObject* object)
{
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

    return m_level;
}

void CWater::SetLava(bool lava)
{
    m_lava = lava;
}

bool CWater::GetLava()
{
    return m_lava;
}

void CWater::AdjustEye(Math::Vector &eye)
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


} // namespace Gfx
