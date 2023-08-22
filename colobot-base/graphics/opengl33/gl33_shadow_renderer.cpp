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

#include "graphics/opengl33/gl33_shadow_renderer.h"

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

CGL33ShadowRenderer::CGL33ShadowRenderer(CGL33Device* device)
    : m_device(device)
{
    GetLogger()->Info("Creating CGL33ShadowRenderer\n");

    GLint shaders[2] = {};

    shaders[0] = LoadShader(GL_VERTEX_SHADER, "shaders/gl33/shadow_vs.glsl");
    if (shaders[0] == 0)
    {
        GetLogger()->Error("Cound not create vertex shader from file 'shadow_vs.glsl'\n");
        return;
    }

    shaders[1] = LoadShader(GL_FRAGMENT_SHADER, "shaders/gl33/shadow_fs.glsl");
    if (shaders[1] == 0)
    {
        GetLogger()->Error("Cound not create fragment shader from file 'shadow_fs.glsl'\n");
        return;
    }

    m_program = LinkProgram(2, shaders);
    if (m_program == 0)
    {
        GetLogger()->Error("Cound not link shader program for terrain renderer\n");
        return;
    }

    glDeleteShader(shaders[0]);
    glDeleteShader(shaders[1]);

    glUseProgram(m_program);

    // Setup uniforms
    auto texture = glGetUniformLocation(m_program, "uni_Texture");
    glUniform1i(texture, 0);

    glm::mat4 identity(1.0f);

    m_projectionMatrix = glGetUniformLocation(m_program, "uni_ProjectionMatrix");
    m_viewMatrix = glGetUniformLocation(m_program, "uni_ViewMatrix");
    m_modelMatrix = glGetUniformLocation(m_program, "uni_ModelMatrix");
    m_alphaScissor = glGetUniformLocation(m_program, "uni_AlphaScissor");

    glUseProgram(0);

    glGenFramebuffers(1, &m_framebuffer);

    GetLogger()->Info("CGL33ShadowRenderer created successfully\n");
}

CGL33ShadowRenderer::~CGL33ShadowRenderer()
{
    glDeleteProgram(m_program);

    glDeleteFramebuffers(1, &m_framebuffer);
}

void CGL33ShadowRenderer::Begin()
{
    glViewport(0, 0, m_width, m_height);
    m_device->SetDepthMask(true);
    glClear(GL_DEPTH_BUFFER_BIT);

    glUseProgram(m_program);

    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2.0f, 8.0f);

    m_device->SetColorMask(false, false, false, false);
    m_device->SetDepthTest(true);
    m_device->SetDepthMask(true);

    m_device->SetTransparency(TransparencyMode::NONE);
    m_device->SetCullFace(CullFace::NONE);
}

void CGL33ShadowRenderer::End()
{
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDisable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(0.0f, 0.0f);

    m_device->SetColorMask(true, true, true, true);
}

void CGL33ShadowRenderer::SetProjectionMatrix(const glm::mat4& matrix)
{
    glUniformMatrix4fv(m_projectionMatrix, 1, GL_FALSE, value_ptr(matrix));
}

void CGL33ShadowRenderer::SetViewMatrix(const glm::mat4& matrix)
{
    glm::mat4 scale(1.0f);
    scale[2][2] = -1.0f;

    auto viewMatrix = scale * matrix;

    glUniformMatrix4fv(m_viewMatrix, 1, GL_FALSE, value_ptr(viewMatrix));
}

void CGL33ShadowRenderer::SetModelMatrix(const glm::mat4& matrix)
{
    glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, value_ptr(matrix));
}

void CGL33ShadowRenderer::SetTexture(const Texture& texture)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.id);
}

void CGL33ShadowRenderer::SetShadowMap(const Texture& texture)
{
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture.id, 0);

    m_width = texture.size.x;
    m_height = texture.size.y;

    auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        GetLogger()->Error("Framebuffer incomplete: %d\n", status);
    }
}

void CGL33ShadowRenderer::SetShadowRegion(const glm::vec2& offset, const glm::vec2& scale)
{
    int x = static_cast<int>(m_width * offset.x);
    int y = static_cast<int>(m_height * offset.y);
    int width = static_cast<int>(m_width * scale.x);
    int height = static_cast<int>(m_height * scale.y);

    glViewport(x, y, width, height);
}

void CGL33ShadowRenderer::DrawObject(const CVertexBuffer* buffer, bool transparent)
{
    auto b = dynamic_cast<const CGL33VertexBuffer*>(buffer);

    if (b == nullptr) return;

    glUniform1i(m_alphaScissor, transparent ? 1 : 0);

    glBindVertexArray(b->GetVAO());

    glDrawArrays(TranslateGfxPrimitive(b->GetType()), 0, b->Size());
}
