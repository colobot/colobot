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

#include "graphics/opengl33/gl33_particle_renderer.h"

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

using namespace Gfx;

CGL33ParticleRenderer::CGL33ParticleRenderer(CGL33Device* device)
    : m_device(device)
{
    GetLogger()->Info("Creating CGL33ParticleRenderer");

    std::string preamble = LoadSource("shaders/gl33/preamble.glsl");
    std::string shadowSource = LoadSource("shaders/gl33/shadow.glsl");
    std::string vsSource = LoadSource("shaders/gl33/particle_vs.glsl");
    std::string fsSource = LoadSource("shaders/gl33/particle_fs.glsl");

    GLint vsShader = CreateShader(GL_VERTEX_SHADER, { preamble, shadowSource, vsSource });
    if (vsShader == 0)
    {
        GetLogger()->Error("Cound not create vertex shader from file 'particle_vs.glsl'");
        return;
    }

    GLint fsShader = CreateShader(GL_FRAGMENT_SHADER, { preamble, shadowSource, fsSource });
    if (fsShader == 0)
    {
        GetLogger()->Error("Cound not create fragment shader from file 'particle_fs.glsl'");
        return;
    }

    m_program = LinkProgram({ vsShader, fsShader });

    glDeleteShader(vsShader);
    glDeleteShader(fsShader);

    glUseProgram(m_program);

    // Setup uniforms
    m_projectionMatrix = glGetUniformLocation(m_program, "uni_ProjectionMatrix");
    m_viewMatrix = glGetUniformLocation(m_program, "uni_ViewMatrix");
    m_modelMatrix = glGetUniformLocation(m_program, "uni_ModelMatrix");
    m_fogRange = glGetUniformLocation(m_program, "uni_FogRange");
    m_fogColor = glGetUniformLocation(m_program, "uni_FogColor");
    m_color = glGetUniformLocation(m_program, "uni_Color");
    m_alphaScissor = glGetUniformLocation(m_program, "uni_AlphaScissor");

    // Set texture units
    auto texture = glGetUniformLocation(m_program, "uni_Texture");
    glUniform1i(texture, 10);

    glUniform1f(m_alphaScissor, 0.5f);

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

    // Generic buffer
    glGenBuffers(1, &m_bufferVBO);
    glBindBuffer(GL_COPY_WRITE_BUFFER, m_bufferVBO);
    glBufferData(GL_COPY_WRITE_BUFFER, m_bufferCapacity * sizeof(VertexParticle), nullptr, GL_STREAM_DRAW);
    m_bufferOffset = m_bufferCapacity;

    glGenVertexArrays(1, &m_bufferVAO);
    glBindVertexArray(m_bufferVAO);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    GetLogger()->Info("CGL33ParticleRenderer created successfully");
}

CGL33ParticleRenderer::~CGL33ParticleRenderer()
{
    glDeleteProgram(m_program);
    glDeleteTextures(1, &m_whiteTexture);
}

void CGL33ParticleRenderer::Begin()
{
    glUseProgram(m_program);

    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D, m_whiteTexture);

    m_texture = 0;

    m_device->SetDepthTest(true);
    m_device->SetDepthMask(false);
    m_device->SetCullFace(CullFace::NONE);

    glUniform4f(m_color, 1.0f, 1.0f, 1.0f, 1.0f);

    glBindVertexArray(m_bufferVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_bufferVBO);
}

void CGL33ParticleRenderer::End()
{
    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_texture = 0;

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CGL33ParticleRenderer::SetProjectionMatrix(const glm::mat4& matrix)
{
    glUniformMatrix4fv(m_projectionMatrix, 1, GL_FALSE, value_ptr(matrix));
}

void CGL33ParticleRenderer::SetViewMatrix(const glm::mat4& matrix)
{
    glm::mat4 scale(1.0f);
    scale[2][2] = -1.0f;

    auto viewMatrix = scale * matrix;

    glUniformMatrix4fv(m_viewMatrix, 1, GL_FALSE, value_ptr(viewMatrix));
}

void CGL33ParticleRenderer::SetModelMatrix(const glm::mat4& matrix)
{
    glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, value_ptr(matrix));
}

void CGL33ParticleRenderer::SetColor(const glm::vec4& color)
{
    glUniform4f(m_color, color.r, color.g, color.b, color.a);
}

void CGL33ParticleRenderer::SetTexture(const Texture& texture)
{
    if (m_texture == texture.id) return;

    m_texture = texture.id;

    glActiveTexture(GL_TEXTURE10);

    if (texture.id == 0)
        glBindTexture(GL_TEXTURE_2D, m_whiteTexture);
    else
        glBindTexture(GL_TEXTURE_2D, texture.id);
}

void CGL33ParticleRenderer::SetTransparency(TransparencyMode mode)
{
    m_device->SetTransparency(mode);
}

void CGL33ParticleRenderer::DrawParticle(PrimitiveType type, int count, const VertexParticle* vertices)
{
    GLsizei total = m_bufferOffset + count;

    // Buffer full, orphan
    if (total >= m_bufferCapacity)
    {
        glBufferData(GL_ARRAY_BUFFER, m_bufferCapacity * sizeof(VertexParticle), nullptr, GL_STREAM_DRAW);

        m_bufferOffset = 0;

        // Respecify vertex attributes
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexParticle),
            reinterpret_cast<void*>(offsetof(VertexParticle, position)));

        glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(VertexParticle),
            reinterpret_cast<void*>(offsetof(VertexParticle, color)));

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexParticle),
            reinterpret_cast<void*>(offsetof(VertexParticle, uv)));
    }

    void* ptr = glMapBufferRange(GL_ARRAY_BUFFER,
        m_bufferOffset * sizeof(VertexParticle),
        count * sizeof(VertexParticle),
        GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);

    if (ptr)
    {
        auto buffer = reinterpret_cast<VertexParticle*>(ptr);

        std::copy_n(vertices, count, buffer);

        glUnmapBuffer(GL_ARRAY_BUFFER);
    }
    else
    {
        glBufferSubData(GL_ARRAY_BUFFER,
            m_bufferOffset * sizeof(VertexParticle),
            count * sizeof(VertexParticle),
            vertices);
    }

    glDrawArrays(TranslateGfxPrimitive(type),
        m_bufferOffset,
        count);

    m_bufferOffset += count;
}
