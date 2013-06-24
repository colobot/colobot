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


#include "graphics/engine/cloud.h"

#include "graphics/core/device.h"
#include "graphics/engine/engine.h"
#include "graphics/engine/terrain.h"

#include "object/robotmain.h"

#include "math/geometry.h"


// Graphics module namespace
namespace Gfx {


const int CLOUD_LINE_PREALLOCATE_COUNT = 100;

//! Extension of the bricks dimensions
const int CLOUD_SIZE_EXPAND = 4;


CCloud::CCloud(CEngine* engine)
{
    m_engine = engine;
    m_terrain = nullptr;

    m_level = 0.0f;
    m_wind  = Math::Vector(0.0f, 0.0f, 0.0f);
    m_subdiv = 8;
    m_enabled = true;

    m_lines.reserve(CLOUD_LINE_PREALLOCATE_COUNT);
}

CCloud::~CCloud()
{
    m_engine = nullptr;
    m_terrain = nullptr;
}

bool CCloud::EventProcess(const Event &event)
{
    if ( event.type == EVENT_FRAME )
        return EventFrame(event);

    return true;
}

bool CCloud::EventFrame(const Event &event)
{
    if (m_engine->GetPause()) return true;

    m_time += event.rTime;

    if (m_level == 0.0f) return true;

    if (m_time - m_lastTest < 0.2f) return true;

    m_lastTest = m_time;

    return true;
}

void CCloud::AdjustLevel(Math::Vector& pos, Math::Vector& eye, float deep,
                         Math::Point& uv1, Math::Point& uv2)
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

void CCloud::Draw()
{
    if (! m_enabled) return;
    if (m_level == 0.0f) return;
    if (m_lines.empty()) return;

    std::vector<VertexTex2> vertices((m_brickCount+2)*2, VertexTex2());

    float iDeep = m_engine->GetDeepView();
    float deep = (m_brickCount*m_brickSize)/2.0f;
    m_engine->SetDeepView(deep);
    m_engine->SetFocus(m_engine->GetFocus());
    m_engine->UpdateMatProj();  // increases the depth of view

    float fogStart = deep*0.15f;
    float fogEnd   = deep*0.24f;

    CDevice* device = m_engine->GetDevice();

    // TODO: do this better?
    device->SetFogParams(FOG_LINEAR, m_engine->GetFogColor( m_engine->GetRankView() ),
                        fogStart, fogEnd, 1.0f);

    device->SetTransform(TRANSFORM_VIEW, m_engine->GetMatView());

    Material material;
    material.diffuse = m_diffuse;
    material.ambient = m_ambient;
    m_engine->SetMaterial(material);

    m_engine->SetTexture(m_fileName, 0);
    m_engine->SetTexture(m_fileName, 1);

    m_engine->SetState(ENG_RSTATE_TTEXTURE_BLACK | ENG_RSTATE_FOG | ENG_RSTATE_WRAP);

    Math::Matrix matrix;
    matrix.LoadIdentity();
    device->SetTransform(TRANSFORM_WORLD, matrix);

    float size = m_brickSize/2.0f;
    Math::Vector eye = m_engine->GetEyePt();
    Math::Vector n = Math::Vector(0.0f, -1.0f, 0.0f);

    // Draws all the lines
    for (int i = 0; i < static_cast<int>( m_lines.size() ); i++)
    {
        Math::Vector pos;
        pos.y = m_level;
        pos.z = m_lines[i].pz;
        pos.x = m_lines[i].px1;

        int vertexIndex = 0;

        Math::Vector p;
        Math::Point uv1, uv2;

        p.x = pos.x-size;
        p.z = pos.z+size;
        p.y = pos.y;
        AdjustLevel(p, eye, deep, uv1, uv2);
        vertices[vertexIndex++] = VertexTex2(p, n, uv1, uv2);

        p.x = pos.x-size;
        p.z = pos.z-size;
        p.y = pos.y;
        AdjustLevel(p, eye, deep, uv1, uv2);
        vertices[vertexIndex++] = VertexTex2(p, n, uv1, uv2);

        for (int j = 0; j < m_lines[i].len; j++)
        {
            p.x = pos.x+size;
            p.z = pos.z+size;
            p.y = pos.y;
            AdjustLevel(p, eye, deep, uv1, uv2);
            vertices[vertexIndex++] = VertexTex2(p, n, uv1, uv2);

            p.x = pos.x+size;
            p.z = pos.z-size;
            p.y = pos.y;
            AdjustLevel(p, eye, deep, uv1, uv2);
            vertices[vertexIndex++] = VertexTex2(p, n, uv1, uv2);

            pos.x += size*2.0f;
        }

        device->DrawPrimitive(PRIMITIVE_TRIANGLE_STRIP, &vertices[0], vertexIndex);
        m_engine->AddStatisticTriangle(vertexIndex - 2);
    }

    m_engine->SetDeepView(iDeep);
    m_engine->SetFocus(m_engine->GetFocus());
    m_engine->UpdateMatProj();  // gives depth to initial
}

void CCloud::CreateLine(int x, int y, int len)
{
    CloudLine line;

    line.x   = x;
    line.y   = y;
    line.len = len;

    float offset = m_brickCount*m_brickSize/2.0f - m_brickSize/2.0f;

    line.px1 = m_brickSize* line.x - offset;
    line.px2 = m_brickSize*(line.x+line.len) - offset;
    line.pz  = m_brickSize* line.y - offset;

    m_lines.push_back(line);
}

void CCloud::Create(const std::string& fileName,
                    const Color& diffuse, const Color& ambient,
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
        m_terrain = CRobotMain::GetInstancePointer()->GetTerrain();

    m_wind = m_terrain->GetWind();

    m_brickCount = m_terrain->GetBrickCount()*m_terrain->GetMosaicCount()*CLOUD_SIZE_EXPAND;
    m_brickSize  = m_terrain->GetBrickSize();

    m_brickCount /= m_subdiv*CLOUD_SIZE_EXPAND;
    m_brickSize  *= m_subdiv*CLOUD_SIZE_EXPAND;

    if (m_level == 0.0f)
        return;

    m_lines.clear();
    for (int y = 0; y < m_brickCount; y++)
        CreateLine(0, y, m_brickCount);

    return;
}

void CCloud::Flush()
{
    m_level = 0.0f;
}

void CCloud::SetLevel(float level)
{
    m_level = level;

    Create(m_fileName, m_diffuse, m_ambient, m_level);
}

float CCloud::GetLevel()
{
    return m_level;
}

void CCloud::SetEnabled(bool enabled)
{
    m_enabled = enabled;
}

bool CCloud::GetEnabled()
{
    return m_enabled;
}


} // namespace Gfx

