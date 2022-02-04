/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2021, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "graphics/opengl/gl33objectrenderer.h"

#include "graphics/opengl/gl33device.h"
#include "graphics/opengl/glutil.h"

#include "graphics/core/vertex.h"

#include "common/logger.h"

#include <GL/glew.h>

#include <glm/ext.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace Gfx;

CGL33ObjectRenderer::CGL33ObjectRenderer(CGL33Device* device)
    :m_device(device)
{
    GetLogger()->Info("Creating CGL33ObjectRenderer\n");


    std::string preamble = LoadSource("shaders/gl33/preamble.glsl");
    std::string shadowSource = LoadSource("shaders/gl33/shadow.glsl");
    std::string vsSource = LoadSource("shaders/gl33/object_vs.glsl");
    std::string fsSource = LoadSource("shaders/gl33/object_fs.glsl");

    GLint vsShader = CreateShader(GL_VERTEX_SHADER, { preamble, shadowSource, vsSource });
    if (vsShader == 0)
    {
        GetLogger()->Error("Cound not create vertex shader from file 'object_vs.glsl'\n");
        return;
    }

    GLint fsShader = CreateShader(GL_FRAGMENT_SHADER, { preamble, shadowSource, fsSource });
    if (fsShader == 0)
    {
        GetLogger()->Error("Cound not create fragment shader from file 'object_fs.glsl'\n");
        return;
    }

    m_program = LinkProgram({ vsShader, fsShader });

    glDeleteShader(vsShader);
    glDeleteShader(fsShader);

    glUseProgram(m_program);

    // Setup uniforms
    glm::mat4 identity(1.0f);

    m_projectionMatrix = glGetUniformLocation(m_program, "uni_ProjectionMatrix");
    m_viewMatrix = glGetUniformLocation(m_program, "uni_ViewMatrix");
    m_cameraMatrix = glGetUniformLocation(m_program, "uni_CameraMatrix");
    m_shadowMatrix = glGetUniformLocation(m_program, "uni_ShadowMatrix");
    m_modelMatrix = glGetUniformLocation(m_program, "uni_ModelMatrix");
    m_normalMatrix = glGetUniformLocation(m_program, "uni_NormalMatrix");
    m_lighting = glGetUniformLocation(m_program, "uni_Lighting");
    m_lightPosition = glGetUniformLocation(m_program, "uni_LightPosition");
    m_lightIntensity = glGetUniformLocation(m_program, "uni_LightIntensity");
    m_lightColor = glGetUniformLocation(m_program, "uni_LightColor");
    m_fogRange = glGetUniformLocation(m_program, "uni_FogRange");
    m_fogColor = glGetUniformLocation(m_program, "uni_FogColor");
    m_color = glGetUniformLocation(m_program, "uni_Color");
    m_primaryEnabled = glGetUniformLocation(m_program, "uni_PrimaryEnabled");
    m_triplanarMode = glGetUniformLocation(m_program, "uni_TriplanarMode");
    m_triplanarScale = glGetUniformLocation(m_program, "uni_TriplanarScale");
    m_dirty = glGetUniformLocation(m_program, "uni_Dirty");
    m_alphaScissor = glGetUniformLocation(m_program, "uni_AlphaScissor");
    m_uvOffset = glGetUniformLocation(m_program, "uni_UVOffset");
    m_uvScale = glGetUniformLocation(m_program, "uni_UVScale");

    m_shadowRegions = glGetUniformLocation(m_program, "uni_ShadowRegions");

    GLchar name[64];

    for (int i = 0; i < 4; i++)
    {
        sprintf(name, "uni_ShadowParam[%d].transform", i);
        m_shadows[i].transform = glGetUniformLocation(m_program, name);

        sprintf(name, "uni_ShadowParam[%d].uv_offset", i);
        m_shadows[i].offset = glGetUniformLocation(m_program, name);

        sprintf(name, "uni_ShadowParam[%d].uv_scale", i);
        m_shadows[i].scale = glGetUniformLocation(m_program, name);
    }

    // Set texture units
    auto texture = glGetUniformLocation(m_program, "uni_PrimaryTexture");
    glUniform1i(texture, 10);

    texture = glGetUniformLocation(m_program, "uni_SecondaryTexture");
    glUniform1i(texture, 11);

    texture = glGetUniformLocation(m_program, "uni_ShadowMap");
    glUniform1i(texture, 12);

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
    glBufferData(GL_COPY_WRITE_BUFFER, 8 * sizeof(Vertex3D), nullptr, GL_STREAM_DRAW);

    glGenVertexArrays(1, &m_bufferVAO);
    glBindVertexArray(m_bufferVAO);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);

    GetLogger()->Info("CGL33ObjectRenderer created successfully\n");
}

CGL33ObjectRenderer::~CGL33ObjectRenderer()
{
    glDeleteProgram(m_program);
    glDeleteTextures(1, &m_whiteTexture);
    glDeleteBuffers(1, &m_bufferVBO);
    glDeleteVertexArrays(1, &m_bufferVAO);
}

void CGL33ObjectRenderer::CGL33ObjectRenderer::Begin()
{
    glUseProgram(m_program);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D, m_whiteTexture);

    glActiveTexture(GL_TEXTURE11);
    glBindTexture(GL_TEXTURE_2D, m_whiteTexture);

    glActiveTexture(GL_TEXTURE12);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_primaryTexture = 0;
    m_secondaryTexture = 0;
    m_shadowMap = 0;

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);   // Colobot issue: faces are reversed

    SetUVTransform({ 0.0f, 0.0f }, { 1.0f, 1.0f });
}

void CGL33ObjectRenderer::CGL33ObjectRenderer::End()
{
    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE11);
    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE12);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_primaryTexture = 0;
    m_secondaryTexture = 0;
    m_shadowMap = 0;

    glDepthMask(GL_TRUE);
}

void CGL33ObjectRenderer::SetProjectionMatrix(const glm::mat4& matrix)
{
    glUniformMatrix4fv(m_projectionMatrix, 1, GL_FALSE, value_ptr(matrix));
}

void CGL33ObjectRenderer::SetViewMatrix(const glm::mat4& matrix)
{
    glm::mat4 scale(1.0f);
    scale[2][2] = -1.0f;

    auto viewMatrix = scale * matrix;
    auto cameraMatrix = glm::inverse(viewMatrix);

    glUniformMatrix4fv(m_viewMatrix, 1, GL_FALSE, value_ptr(viewMatrix));
    glUniformMatrix4fv(m_cameraMatrix, 1, GL_FALSE, value_ptr(cameraMatrix));
}

void CGL33ObjectRenderer::SetModelMatrix(const glm::mat4& matrix)
{
    auto normalMatrix = glm::transpose(glm::inverse(glm::mat3(matrix)));

    glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, value_ptr(matrix));
    glUniformMatrix3fv(m_normalMatrix, 1, GL_FALSE, value_ptr(normalMatrix));
}

void CGL33ObjectRenderer::SetColor(const glm::vec4& color)
{
    glUniform4fv(m_color, 1, glm::value_ptr(color));
}

void CGL33ObjectRenderer::SetPrimaryTexture(const Texture& texture)
{
    if (m_primaryTexture == texture.id) return;

    m_primaryTexture = texture.id;

    glActiveTexture(GL_TEXTURE10);

    if (texture.id == 0)
        glBindTexture(GL_TEXTURE_2D, m_whiteTexture);
    else
        glBindTexture(GL_TEXTURE_2D, texture.id);
}

void CGL33ObjectRenderer::SetSecondaryTexture(const Texture& texture)
{
    if (m_secondaryTexture == texture.id) return;

    m_secondaryTexture = texture.id;

    glActiveTexture(GL_TEXTURE11);

    if (texture.id == 0)
        glBindTexture(GL_TEXTURE_2D, m_whiteTexture);
    else
        glBindTexture(GL_TEXTURE_2D, texture.id);
}

void CGL33ObjectRenderer::SetShadowMap(const Texture& texture)
{
    if (m_shadowMap == texture.id) return;

    m_shadowMap = texture.id;

    glActiveTexture(GL_TEXTURE12);

    if (texture.id == 0)
        glBindTexture(GL_TEXTURE_2D, m_whiteTexture);
    else
        glBindTexture(GL_TEXTURE_2D, texture.id);
}

void CGL33ObjectRenderer::SetLighting(bool enabled)
{
    glUniform1i(m_lighting, enabled ? 1 : 0);
}

void CGL33ObjectRenderer::SetLight(const glm::vec4& position, const float& intensity, const glm::vec3& color)
{
    glUniform4fv(m_lightPosition, 1, glm::value_ptr(position));
    glUniform1f(m_lightIntensity, intensity);
    glUniform3fv(m_lightColor, 1, glm::value_ptr(color));
}

void CGL33ObjectRenderer::SetShadowParams(int count, const ShadowParam* params)
{
    glUniform1i(m_shadowRegions, count);

    for (int i = 0; i < count; i++)
    {
        glUniformMatrix4fv(m_shadows[i].transform, 1, GL_FALSE, glm::value_ptr(params[i].matrix));
        glUniform2fv(m_shadows[i].offset, 1, glm::value_ptr(params[i].uv_offset));
        glUniform2fv(m_shadows[i].scale, 1, glm::value_ptr(params[i].uv_scale));
    }
}

void CGL33ObjectRenderer::SetFog(float min, float max, const glm::vec3& color)
{
    glUniform2f(m_fogRange, min, max);
    glUniform3f(m_fogColor, color.r, color.g, color.b);
}

void CGL33ObjectRenderer::SetDepthTest(bool enabled)
{
    if (enabled)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
}

void CGL33ObjectRenderer::SetDepthMask(bool enabled)
{
    glDepthMask(enabled ? GL_TRUE : GL_FALSE);
}

void CGL33ObjectRenderer::SetCullMode(bool enabled)
{
    if (enabled)
    {
        glEnable(GL_CULL_FACE);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }
}

void CGL33ObjectRenderer::SetTransparency(TransparencyMode mode)
{
    switch (mode)
    {
    case TransparencyMode::NONE:
        glDisable(GL_BLEND);
        break;
    case TransparencyMode::ALPHA:
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);
        break;
    case TransparencyMode::BLACK:
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
        glBlendEquation(GL_FUNC_ADD);
        break;
    case TransparencyMode::WHITE:
        glEnable(GL_BLEND);
        glBlendFunc(GL_DST_COLOR, GL_ZERO);
        glBlendEquation(GL_FUNC_ADD);
        break;
    }
}

void CGL33ObjectRenderer::SetUVTransform(const glm::vec2& offset, const glm::vec2& scale)
{
    glUniform2fv(m_uvOffset, 1, glm::value_ptr(offset));
    glUniform2fv(m_uvScale, 1, glm::value_ptr(scale));
}

void CGL33ObjectRenderer::SetPrimaryTextureEnabled(bool enabled)
{
    glUniform1f(m_primaryEnabled, enabled ? 1.0f : 0.0f);
}

void CGL33ObjectRenderer::SetTriplanarMode(bool enabled)
{
    glUniform1i(m_triplanarMode, enabled ? 1 : 0);
}

void CGL33ObjectRenderer::SetTriplanarScale(float scale)
{
    glUniform1f(m_triplanarScale, scale);
}

void CGL33ObjectRenderer::SetDirty(float amount)
{
    glUniform1f(m_dirty, amount);
}

void CGL33ObjectRenderer::SetAlphaScissor(float alpha)
{
    glUniform1f(m_alphaScissor, alpha);
}

void CGL33ObjectRenderer::DrawObject(const CVertexBuffer* buffer)
{
    auto b = dynamic_cast<const CGL33VertexBuffer*>(buffer);

    if (b == nullptr) return;

    glBindVertexArray(b->GetVAO());

    glDrawArrays(TranslateGfxPrimitive(b->GetType()), 0, b->Size());
}

void CGL33ObjectRenderer::DrawPrimitive(PrimitiveType type, int count, const Vertex3D* vertices)
{
    DrawPrimitives(type, 1, &count, vertices);
}

void CGL33ObjectRenderer::DrawPrimitives(PrimitiveType type, int drawCount, int count[], const Vertex3D* vertices)
{
    m_first.resize(drawCount);

    GLint offset = 0;

    for (size_t i = 0; i < drawCount; i++)
    {
        m_first[i] = offset;
        offset += count[i];
    }

    glBindVertexArray(m_bufferVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_bufferVBO);

    size_t size = offset * sizeof(Vertex3D);

    // Send new vertices to GPU
    glBindBuffer(GL_ARRAY_BUFFER, m_bufferVBO);
    glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, vertices);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D),
        reinterpret_cast<void*>(offsetof(Vertex3D, position)));

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D),
        reinterpret_cast<void*>(offsetof(Vertex3D, normal)));

    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex3D),
        reinterpret_cast<void*>(offsetof(Vertex3D, color)));

    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3D),
        reinterpret_cast<void*>(offsetof(Vertex3D, uv)));

    glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3D),
        reinterpret_cast<void*>(offsetof(Vertex3D, uv2)));

    glMultiDrawArrays(TranslateGfxPrimitive(type), m_first.data(), count, drawCount);
}
