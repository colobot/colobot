/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsitec.ch; http://colobot.info; http://github.com/colobot
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://gnu.org/licenses
 */


#include "graphics/engine/cloud.h"

#include "graphics/core/material.h"
#include "graphics/core/renderers.h"
#include "graphics/core/transparency.h"

#include "graphics/engine/engine.h"
#include "graphics/engine/terrain.h"

#include "level/robotmain.h"

#include "math/geometry.h"
#include "common/stringutils.h"

// Graphics module namespace
namespace Gfx
{

namespace
{
const int CLOUD_LINE_PREALLOCATE_COUNT = 100;

//! Extension of the bricks dimensions
const int CLOUD_SIZE_EXPAND = 4;
} // anonymous namespace


CCloud::CCloud(CEngine* engine)
    : m_engine(engine)
{
    m_lines.reserve(CLOUD_LINE_PREALLOCATE_COUNT);
}

CCloud::~CCloud()
{
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

void CCloud::AdjustLevel(glm::vec3& pos, glm::vec3& eye, float deep,
                         glm::vec2& uv1, glm::vec2& uv2)
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

    std::vector<Vertex3D> vertices((m_brickCount+2)*2, Vertex3D());

    float iDeep = m_engine->GetDeepView();
    float deep = (m_brickCount*m_brickSize)/2.0f;
    m_engine->SetDeepView(deep);
    m_engine->SetFocus(m_engine->GetFocus());

    float fogStart = deep*0.15f;
    float fogEnd   = deep*0.24f;

    auto renderer = m_engine->GetObjectRenderer();
    renderer->Begin();

    auto fogColor = m_engine->GetFogColor(m_engine->GetRankView());

    renderer->SetFog(fogStart, fogEnd, { fogColor.r, fogColor.g, fogColor.b });

    renderer->SetProjectionMatrix(m_engine->GetMatProj());
    renderer->SetViewMatrix(m_engine->GetMatView());

    auto texture = m_engine->LoadTexture(m_fileName);
    renderer->SetAlbedoTexture(texture);

    renderer->SetTransparency(TransparencyMode::BLACK);
    renderer->SetDepthMask(false);

    glm::mat4 matrix = glm::mat4(1.0f);
    renderer->SetModelMatrix(matrix);

    float size = m_brickSize/2.0f;
    glm::vec3 eye = m_engine->GetEyePt();
    glm::vec3 n = glm::vec3(0.0f, -1.0f, 0.0f);

    // Draws all the lines
    for (int i = 0; i < static_cast<int>( m_lines.size() ); i++)
    {
        glm::vec3 pos{};
        pos.y = m_level;
        pos.z = m_lines[i].pz;
        pos.x = m_lines[i].px1;

        int vertexIndex = 0;

        glm::vec3 p{};
        glm::vec2 uv1, uv2;

        glm::u8vec4 white(255);

        p.x = pos.x-size;
        p.z = pos.z+size;
        p.y = pos.y;
        AdjustLevel(p, eye, deep, uv1, uv2);
        vertices[vertexIndex++] = Vertex3D{ p, white, uv1, uv2, n };

        p.x = pos.x-size;
        p.z = pos.z-size;
        p.y = pos.y;
        AdjustLevel(p, eye, deep, uv1, uv2);
        vertices[vertexIndex++] = Vertex3D{ p, white, uv1, uv2, n };

        for (int j = 0; j < m_lines[i].len; j++)
        {
            p.x = pos.x+size;
            p.z = pos.z+size;
            p.y = pos.y;
            AdjustLevel(p, eye, deep, uv1, uv2);
            vertices[vertexIndex++] = Vertex3D{ p, white, uv1, uv2, n };

            p.x = pos.x+size;
            p.z = pos.z-size;
            p.y = pos.y;
            AdjustLevel(p, eye, deep, uv1, uv2);
            vertices[vertexIndex++] = Vertex3D{ p, white, uv1, uv2, n };

            pos.x += size*2.0f;
        }

        renderer->DrawPrimitive(PrimitiveType::TRIANGLE_STRIP, vertexIndex, vertices.data());
        m_engine->AddStatisticTriangle(vertexIndex - 2);
    }

    renderer->End();

    m_engine->SetDeepView(iDeep);
    m_engine->SetFocus(m_engine->GetFocus());
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

void CCloud::Create(const std::filesystem::path& fileName,
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
