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

// cloud.cpp

#include "graphics/engine/cloud.h"

#include "common/iman.h"
#include "graphics/engine/engine.h"
#include "graphics/engine/terrain.h"
#include "math/geometry.h"


const int CLOUD_LINE_PREALLOCATE_COUNT = 100;

const int DIMEXPAND = 4;    // extension of the dimensions


Gfx::CCloud::CCloud(CInstanceManager* iMan, Gfx::CEngine* engine)
{
    m_iMan = iMan;
    m_iMan->AddInstance(CLASS_CLOUD, this);

    m_engine = engine;
    m_terrain = nullptr;

    m_level = 0.0f;
    m_wind  = Math::Vector(0.0f, 0.0f, 0.0f);
    m_subdiv = 8;
    m_enable = true;

    m_lines.reserve(CLOUD_LINE_PREALLOCATE_COUNT);
}

Gfx::CCloud::~CCloud()
{
    m_iMan = nullptr;
    m_engine = nullptr;
    m_terrain = nullptr;
}

bool Gfx::CCloud::EventProcess(const Event &event)
{
    if ( event.type == EVENT_FRAME )
        return EventFrame(event);

    return true;
}

bool Gfx::CCloud::EventFrame(const Event &event)
{
    if (m_engine->GetPause()) return true;

    m_time += event.rTime;

    if (m_level == 0.0f) return true;

    if (m_time - m_lastTest < 0.2f) return true;

    m_lastTest = m_time;

    return true;
}

void Gfx::CCloud::AdjustLevel(Math::Vector &pos, Math::Vector &eye, float deep,
                              Math::Point &uv1, Math::Point &uv2)
{
    uv1.x = (pos.x+20000.0f)/1280.0f;
    uv1.y = (pos.z+20000.0f)/1280.0f;
    uv1.x -= m_time*(m_wind.x/100.0f);
    uv1.y -= m_time*(m_wind.z/100.0f);

    uv2.x = 0.0f;
    uv2.y = 0.0f;

    float dist = Math::DistanceProjected(pos, eye);
    float factor = powf(dist/deep, 2.0f);
    pos.y -= m_level*factor*10.0f;
}

void Gfx::CCloud::Draw()
{
    /* TODO!
    LPDIRECT3DDEVICE7 device;
    D3DVERTEX2*     vertex;
    Math::Matrix*       matView;
    D3DMATERIAL7    material;
    Math::Matrix        matrix;
    Math::Vector        n, pos, p, eye;
    Math::Point         uv1, uv2;
    float           iDeep, deep, size, fogStart, fogEnd;
    int             i, j, u;

    if ( !m_enable )  return;
    if ( m_level == 0.0f )  return;
    if ( m_linesUsed == 0 )  return;

    vertex = (D3DVERTEX2*)malloc(sizeof(D3DVERTEX2)*(m_brick+2)*2);

    iDeep = m_engine->GetDeepView();
    deep = (m_brick*m_size)/2.0f;
    m_engine->SetDeepView(deep);
    m_engine->SetFocus(m_engine->GetFocus());
    m_engine->UpdateMatProj();  // increases the depth of view

    fogStart = deep*0.15f;
    fogEnd   = deep*0.24f;

    device = m_engine->GetD3DDevice();
    device->SetRenderState(D3DRENDERSTATE_AMBIENT, 0x00000000);
    device->SetRenderState(D3DRENDERSTATE_LIGHTING, false);
    device->SetRenderState(D3DRENDERSTATE_ZENABLE, false);
    device->SetRenderState(D3DRENDERSTATE_FOGENABLE, true);
    device->SetRenderState(D3DRENDERSTATE_FOGSTART, F2DW(fogStart));
    device->SetRenderState(D3DRENDERSTATE_FOGEND,   F2DW(fogEnd));

    matView = m_engine->GetMatView();
    {
      D3DMATRIX mat = MAT_TO_D3DMAT(*matView);
      device->SetTransform(D3DTRANSFORMSTATE_VIEW, &mat);
    }

    ZeroMemory( &material, sizeof(D3DMATERIAL7) );
    material.diffuse = m_diffuse;
    material.ambient = m_ambient;
    m_engine->SetMaterial(material);

    m_engine->SetTexture(m_filename, 0);
    m_engine->SetTexture(m_filename, 1);

    m_engine->SetState(D3DSTATETTb|D3DSTATEFOG|D3DSTATEWRAP);

    matrix.LoadIdentity();
    {
      D3DMATRIX mat = MAT_TO_D3DMAT(matrix);
      device->SetTransform(D3DTRANSFORMSTATE_WORLD, &mat);
    }

    size = m_size/2.0f;
    eye = m_engine->GetEyePt();
    n = Math::Vector(0.0f, -1.0f, 0.0f);

    // Draws all the lines.
    for ( i=0 ; i<m_linesUsed ; i++ )
    {
        pos.y = m_level;
        pos.z = m_lines[i].pz;
        pos.x = m_lines[i].px1;

        u = 0;
        p.x = pos.x-size;
        p.z = pos.z+size;
        p.y = pos.y;
        AdjustLevel(p, eye, deep, uv1, uv2);
        vertex[u++] = D3DVERTEX2(p, n, uv1.x,uv1.y, uv2.x,uv2.y);

        p.x = pos.x-size;
        p.z = pos.z-size;
        p.y = pos.y;
        AdjustLevel(p, eye, deep, uv1, uv2);
        vertex[u++] = D3DVERTEX2(p, n, uv1.x,uv1.y, uv2.x,uv2.y);

        for ( j=0 ; j<m_lines[i].len ; j++ )
        {
            p.x = pos.x+size;
            p.z = pos.z+size;
            p.y = pos.y;
            AdjustLevel(p, eye, deep, uv1, uv2);
            vertex[u++] = D3DVERTEX2(p, n, uv1.x,uv1.y, uv2.x,uv2.y);

            p.x = pos.x+size;
            p.z = pos.z-size;
            p.y = pos.y;
            AdjustLevel(p, eye, deep, uv1, uv2);
            vertex[u++] = D3DVERTEX2(p, n, uv1.x,uv1.y, uv2.x,uv2.y);

            pos.x += size*2.0f;
        }

        device->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX2, vertex, u, NULL);
        m_engine->AddStatisticTriangle(u-2);
    }

    m_engine->SetDeepView(iDeep);
    m_engine->SetFocus(m_engine->GetFocus());
    m_engine->UpdateMatProj();  // gives depth to initial

    free(vertex); */
}

void Gfx::CCloud::CreateLine(int x, int y, int len)
{
    Gfx::CloudLine line;

    line.x   = x;
    line.y   = y;
    line.len = len;

    float offset = m_brick*m_size/2.0f - m_size/2.0f;

    line.px1 = m_size* line.x - offset;
    line.px2 = m_size*(line.x+line.len) - offset;
    line.pz  = m_size* line.y - offset;

    m_lines.push_back(line);
}

void Gfx::CCloud::Create(const std::string& fileName,
                         Gfx::Color diffuse, Gfx::Color ambient,
                         float level)
{
    m_diffuse  = diffuse;
    m_ambient  = ambient;
    m_level    = level;
    m_time     = 0.0f;
    m_lastTest = 0.0f;
    m_fileName = fileName;

    if (! m_fileName.empty())
        m_engine->LoadTexture(m_fileName);

    if (m_terrain == nullptr)
        m_terrain = static_cast<CTerrain*>(m_iMan->SearchInstance(CLASS_TERRAIN));

    m_wind = m_terrain->GetWind();

    m_brick = m_terrain->GetBrick()*m_terrain->GetMosaic()*DIMEXPAND;
    m_size  = m_terrain->GetSize();

    m_brick /= m_subdiv*DIMEXPAND;
    m_size  *= m_subdiv*DIMEXPAND;

    if (m_level == 0.0f)
        return;

    m_lines.clear();
    for (int y = 0; y < m_brick; y++)
        CreateLine(0, y, m_brick);

    return;
}

void Gfx::CCloud::Flush()
{
    m_level = 0.0f;
}


void Gfx::CCloud::SetLevel(float level)
{
    m_level = level;

    Create(m_fileName, m_diffuse, m_ambient, m_level);
}

float Gfx::CCloud::GetLevel()
{
    return m_level;
}

void Gfx::CCloud::SetEnable(bool enable)
{
    m_enable = enable;
}

bool Gfx::CCloud::GetEnable()
{
    return m_enable;
}
