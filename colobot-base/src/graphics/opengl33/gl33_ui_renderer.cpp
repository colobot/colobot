/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2022, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "graphics/opengl33/gl33_ui_renderer.h"

#include "graphics/opengl33/gl33_device.h"
#include "graphics/opengl33/glutil.h"

#include "graphics/core/material.h"
#include "graphics/core/transparency.h"
#include "graphics/core/vertex.h"

#include "common/logger.h"

#include <GL/glew.h>

#include <glm/ext.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>

#include <array>
#include <vector>

using namespace Gfx;

CGL33UIRenderer::CGL33UIRenderer(CGL33Device* device)
    : m_device(device)
{
    GetLogger()->Info("Creating CGL33UIRenderer");

    GLint shaders[2] = {};

    shaders[0] = LoadShader(GL_VERTEX_SHADER, "shaders/gl33/ui_vs.glsl");
    if (shaders[0] == 0)
    {
        GetLogger()->Error("Cound not create vertex shader from file 'ui_vs.glsl'");
        return;
    }

    shaders[1] = LoadShader(GL_FRAGMENT_SHADER, "shaders/gl33/ui_fs.glsl");
    if (shaders[1] == 0)
    {
        GetLogger()->Error("Cound not create fragment shader from file 'ui_fs.glsl'");
        return;
    }

    m_program = LinkProgram(2, shaders);
    if (m_program == 0)
    {
        GetLogger()->Error("Cound not link shader program for interface renderer");
        return;
    }

    glDeleteShader(shaders[0]);
    glDeleteShader(shaders[1]);

    glUseProgram(m_program);

    // Create uniform buffer
    glGenBuffers(1, &m_uniformBuffer);

    m_uniforms.projectionMatrix = glm::ortho(0.0f, +1.0f, 0.0f, +1.0f);
    m_uniforms.color = { 1.0f, 1.0f, 1.0f, 1.0f };

    m_uniformsDirty = true;

    UpdateUniforms();

    // Bind uniform block to uniform buffer binding
    GLuint blockIndex = glGetUniformBlockIndex(m_program, "Uniforms");

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_uniformBuffer);
    glUniformBlockBinding(m_program, blockIndex, 0);

    // Set texture unit to 8th
    auto texture = glGetUniformLocation(m_program, "uni_Texture");
    glUniform1i(texture, 8);

    // Generic buffer
    glGenBuffers(1, &m_bufferVBO);
    glBindBuffer(GL_COPY_WRITE_BUFFER, m_bufferVBO);
    glBufferData(GL_COPY_WRITE_BUFFER, m_bufferCapacity * sizeof(Vertex2D), nullptr, GL_STREAM_DRAW);
    m_bufferOffset = m_bufferCapacity;

    glGenVertexArrays(1, &m_bufferVAO);
    glBindVertexArray(m_bufferVAO);

    // White texture
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &m_whiteTexture);
    glBindTexture(GL_TEXTURE_2D, m_whiteTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_ONE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_ONE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_ONE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ONE);

    glUseProgram(0);

    GetLogger()->Info("CGL33UIRenderer created successfully");
}

CGL33UIRenderer::~CGL33UIRenderer()
{
    glDeleteProgram(m_program);
    glDeleteTextures(1, &m_whiteTexture);

    glDeleteBuffers(1, &m_bufferVBO);
    glDeleteVertexArrays(1, &m_bufferVAO);
}

void CGL33UIRenderer::SetProjection(float left, float right, float bottom, float top)
{
    m_uniforms.projectionMatrix = glm::ortho(left, right, bottom, top);
    m_uniformsDirty = true;
}

void CGL33UIRenderer::SetTexture(const Texture& texture)
{
    if (m_currentTexture == texture.id) return;

    glActiveTexture(GL_TEXTURE8);

    m_currentTexture = texture.id;

    if (m_currentTexture == 0)
        glBindTexture(GL_TEXTURE_2D, m_whiteTexture);
    else
        glBindTexture(GL_TEXTURE_2D, m_currentTexture);
}

void CGL33UIRenderer::SetColor(const glm::vec4& color)
{
    m_uniforms.color = color;
    m_uniformsDirty = true;
}

void CGL33UIRenderer::SetTransparency(TransparencyMode mode)
{
    m_device->SetTransparency(mode);
}

Vertex2D* CGL33UIRenderer::BeginPrimitive(PrimitiveType type, int count)
{
    return BeginPrimitives(type, 1, &count);
}

Vertex2D* CGL33UIRenderer::BeginPrimitives(PrimitiveType type, int drawCount, const int* counts)
{
    glBindVertexArray(m_bufferVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_bufferVBO);

    m_currentCount = 0;

    for (int i = 0; i < drawCount; i++)
    {
        m_currentCount += counts[i];
    }

    GLuint total = m_bufferOffset + m_currentCount;

    // Buffer full, orphan
    if (total >= m_bufferCapacity)
    {
        glBufferData(GL_ARRAY_BUFFER, m_bufferCapacity * sizeof(Vertex2D), nullptr, GL_STREAM_DRAW);

        m_bufferOffset = 0;

        // Respecify vertex attributes
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D),
            reinterpret_cast<void*>(offsetof(Vertex2D, position)));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D),
            reinterpret_cast<void*>(offsetof(Vertex2D, uv)));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex2D),
            reinterpret_cast<void*>(offsetof(Vertex2D, color)));
    }

    m_first.resize(drawCount);
    m_count.resize(drawCount);

    GLsizei currentOffset = m_bufferOffset;

    for (int i = 0; i < drawCount; i++)
    {
        m_first[i] = currentOffset;
        m_count[i] = counts[i];

        currentOffset += counts[i];
    }

    auto ptr = glMapBufferRange(GL_ARRAY_BUFFER,
        m_bufferOffset * sizeof(Vertex2D),
        m_currentCount * sizeof(Vertex2D),
        GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);

    m_mapped = true;
    m_type = type;
    m_drawCount = drawCount;

    // Mapping failed, use backup buffer
    if (ptr == nullptr)
    {
        m_backup = true;
        m_buffer.resize(m_currentCount);

        return m_buffer.data();
    }
    else
    {
        return reinterpret_cast<Vertex2D*>(ptr);
    }
}

bool CGL33UIRenderer::EndPrimitive()
{
    if (!m_mapped) return false;

    if (m_backup)
    {
        glBufferSubData(GL_ARRAY_BUFFER,
            m_bufferOffset * sizeof(Vertex2D),
            m_currentCount * sizeof(Vertex2D),
            m_buffer.data());
    }
    else
    {
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }

    glUseProgram(m_program);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_uniformBuffer);

    UpdateUniforms();

    m_device->SetDepthTest(false);
    m_device->SetCullFace(CullFace::NONE);

    if (m_drawCount == 1)
        glDrawArrays(TranslateGfxPrimitive(m_type), m_first.front(), m_count.front());
    else
        glMultiDrawArrays(TranslateGfxPrimitive(m_type), m_first.data(), m_count.data(), m_drawCount);

    m_bufferOffset += m_currentCount;

    m_mapped = false;
    m_backup = false;

    return true;
}

void CGL33UIRenderer::UpdateUniforms()
{
    if (!m_uniformsDirty) return;

    glBindBuffer(GL_COPY_WRITE_BUFFER, m_uniformBuffer);
    glBufferData(GL_COPY_WRITE_BUFFER, sizeof(Uniforms), nullptr, GL_STREAM_DRAW);
    glBufferSubData(GL_COPY_WRITE_BUFFER, 0, sizeof(Uniforms), &m_uniforms);
    glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
}
