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

#include "graphics/opengl33/gl33_object_renderer.h"

#include "graphics/opengl33/gl33_device.h"
#include "graphics/opengl33/glutil.h"

#include "graphics/core/material.h"
#include "graphics/core/transparency.h"
#include "graphics/core/vertex.h"

#include "common/logger.h"

#include <GL/glew.h>

#include <glm/ext.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace Gfx;

CGL33ObjectRenderer::CGL33ObjectRenderer(CGL33Device* device)
    :m_device(device)
{
    GetLogger()->Info("Creating CGL33ObjectRenderer");

    std::string preamble = LoadSource("shaders/gl33/preamble.glsl");
    std::string shadowSource = LoadSource("shaders/gl33/shadow.glsl");
    std::string lightingSource = LoadSource("shaders/gl33/lighting.glsl");
    std::string vsSource = LoadSource("shaders/gl33/object_vs.glsl");
    std::string fsSource = LoadSource("shaders/gl33/object_fs.glsl");

    GLint vsShader = CreateShader(GL_VERTEX_SHADER, { preamble, lightingSource, shadowSource, vsSource });
    if (vsShader == 0)
    {
        GetLogger()->Error("Cound not create vertex shader from file 'object_vs.glsl'");
        return;
    }

    GLint fsShader = CreateShader(GL_FRAGMENT_SHADER, { preamble, lightingSource, shadowSource, fsSource });
    if (fsShader == 0)
    {
        GetLogger()->Error("Cound not create fragment shader from file 'object_fs.glsl'");
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
    m_shadowMatrix = glGetUniformLocation(m_program, "uni_ShadowMatrix");
    m_modelMatrix = glGetUniformLocation(m_program, "uni_ModelMatrix");
    m_normalMatrix = glGetUniformLocation(m_program, "uni_NormalMatrix");

    m_lighting = glGetUniformLocation(m_program, "uni_Lighting");
    m_cameraPosition = glGetUniformLocation(m_program, "uni_CameraPosition");
    m_lightPosition = glGetUniformLocation(m_program, "uni_LightPosition");
    m_lightIntensity = glGetUniformLocation(m_program, "uni_LightIntensity");
    m_lightColor = glGetUniformLocation(m_program, "uni_LightColor");

    m_skyColor = glGetUniformLocation(m_program, "uni_SkyColor");
    m_skyIntensity = glGetUniformLocation(m_program, "uni_SkyIntensity");

    m_fogRange = glGetUniformLocation(m_program, "uni_FogRange");
    m_fogColor = glGetUniformLocation(m_program, "uni_FogColor");

    m_albedoColor = glGetUniformLocation(m_program, "uni_AlbedoColor");
    m_emissiveColor = glGetUniformLocation(m_program, "uni_EmissiveColor");
    m_roughness = glGetUniformLocation(m_program, "uni_Roughness");
    m_metalness = glGetUniformLocation(m_program, "uni_Metalness");
    m_aoStrength = glGetUniformLocation(m_program, "uni_AOStrength");

    m_triplanarMode = glGetUniformLocation(m_program, "uni_TriplanarMode");
    m_triplanarScale = glGetUniformLocation(m_program, "uni_TriplanarScale");
    m_alphaScissor = glGetUniformLocation(m_program, "uni_AlphaScissor");

    m_recolor = glGetUniformLocation(m_program, "uni_Recolor");
    m_recolorFrom = glGetUniformLocation(m_program, "uni_RecolorFrom");
    m_recolorTo = glGetUniformLocation(m_program, "uni_RecolorTo");
    m_recolorThreshold = glGetUniformLocation(m_program, "uni_RecolorThreshold");

    m_uvOffset = glGetUniformLocation(m_program, "uni_UVOffset");
    m_uvScale = glGetUniformLocation(m_program, "uni_UVScale");

    m_shadowRegions = glGetUniformLocation(m_program, "uni_ShadowRegions");

    std::array<GLchar, 256> name;

    for (int i = 0; i < 4; i++)
    {
        snprintf(name.data(), name.size(), "uni_ShadowParam[%d].transform", i);
        m_shadows[i].transform = glGetUniformLocation(m_program, name.data());

        snprintf(name.data(), name.size(), "uni_ShadowParam[%d].uv_offset", i);
        m_shadows[i].offset = glGetUniformLocation(m_program, name.data());

        snprintf(name.data(), name.size(), "uni_ShadowParam[%d].uv_scale", i);
        m_shadows[i].scale = glGetUniformLocation(m_program, name.data());
    }

    // Set texture units
    auto texture = glGetUniformLocation(m_program, "uni_AlbedoTexture");
    glUniform1i(texture, m_albedoIndex);

    texture = glGetUniformLocation(m_program, "uni_DetailTexture");
    glUniform1i(texture, m_detailIndex);

    texture = glGetUniformLocation(m_program, "uni_EmissiveTexture");
    glUniform1i(texture, m_emissiveIndex);

    texture = glGetUniformLocation(m_program, "uni_MaterialTexture");
    glUniform1i(texture, m_materialIndex);

    texture = glGetUniformLocation(m_program, "uni_ShadowMap");
    glUniform1i(texture, m_shadowIndex);

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
    glBindTexture(GL_TEXTURE_2D, 0);

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

    GetLogger()->Info("CGL33ObjectRenderer created successfully");
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

    glActiveTexture(GL_TEXTURE0 + m_albedoIndex);
    glBindTexture(GL_TEXTURE_2D, m_whiteTexture);

    glActiveTexture(GL_TEXTURE0 + m_detailIndex);
    glBindTexture(GL_TEXTURE_2D, m_whiteTexture);

    glActiveTexture(GL_TEXTURE0 + m_emissiveIndex);
    glBindTexture(GL_TEXTURE_2D, m_whiteTexture);

    glActiveTexture(GL_TEXTURE0 + m_materialIndex);
    glBindTexture(GL_TEXTURE_2D, m_whiteTexture);

    glActiveTexture(GL_TEXTURE0 + m_shadowIndex);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_albedoTexture = 0;
    m_detailTexture = 0;
    m_emissiveTexture = 0;
    m_materialTexture = 0;
    m_shadowMap = 0;

    m_device->SetDepthTest(true);
    m_device->SetDepthMask(true);
    m_device->SetTransparency(TransparencyMode::NONE);
    m_device->SetCullFace(CullFace::BACK);

    SetUVTransform({ 0.0f, 0.0f }, { 1.0f, 1.0f });
    SetAlphaScissor(0.0f);
    SetFog(1e+6f, 1e+6, {});
    SetEmissiveColor({ 0, 0, 0, 0 });
    SetAlbedoColor({ 1, 1, 1, 1 });
    SetMaterialParams(1.0, 0.0, 0.0);
    SetRecolor(false);
}

void CGL33ObjectRenderer::CGL33ObjectRenderer::End()
{
    glActiveTexture(GL_TEXTURE0 + m_albedoIndex);
    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE0 + m_detailIndex);
    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE0 + m_emissiveIndex);
    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE0 + m_materialIndex);
    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE0 + m_shadowIndex);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_albedoTexture = 0;
    m_detailTexture = 0;
    m_emissiveTexture = 0;
    m_materialTexture = 0;
    m_shadowMap = 0;
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
    auto cameraPos = cameraMatrix[3];

    glUniformMatrix4fv(m_viewMatrix, 1, GL_FALSE, value_ptr(viewMatrix));
    glUniform3f(m_cameraPosition, cameraPos.x, cameraPos.y, cameraPos.z);
}

void CGL33ObjectRenderer::SetModelMatrix(const glm::mat4& matrix)
{
    auto normalMatrix = glm::transpose(glm::inverse(glm::mat3(matrix)));

    glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, value_ptr(matrix));
    glUniformMatrix3fv(m_normalMatrix, 1, GL_FALSE, value_ptr(normalMatrix));
}

void CGL33ObjectRenderer::SetAlbedoColor(const Color& color)
{
    glUniform4f(m_albedoColor, color.r, color.g, color.b, color.a);
}

void CGL33ObjectRenderer::SetAlbedoTexture(const Texture& texture)
{
    if (m_albedoTexture == texture.id) return;

    m_albedoTexture = texture.id;

    glActiveTexture(GL_TEXTURE0 + m_albedoIndex);

    if (texture.id == 0)
        glBindTexture(GL_TEXTURE_2D, m_whiteTexture);
    else
        glBindTexture(GL_TEXTURE_2D, texture.id);
}

void CGL33ObjectRenderer::SetEmissiveColor(const Color& color)
{
    glUniform3f(m_emissiveColor, color.r, color.g, color.b);
}

void CGL33ObjectRenderer::SetEmissiveTexture(const Texture& texture)
{
    if (m_emissiveTexture == texture.id) return;

    m_emissiveTexture = texture.id;

    glActiveTexture(GL_TEXTURE0 + m_emissiveIndex);

    if (texture.id == 0)
        glBindTexture(GL_TEXTURE_2D, m_whiteTexture);
    else
        glBindTexture(GL_TEXTURE_2D, texture.id);
}

void CGL33ObjectRenderer::SetMaterialParams(float roughness, float metalness, float aoStrength)
{
    glUniform1f(m_roughness, roughness);
    glUniform1f(m_metalness, metalness);
    glUniform1f(m_aoStrength, aoStrength);
}

void CGL33ObjectRenderer::SetMaterialTexture(const Texture& texture)
{
    if (m_materialTexture == texture.id) return;

    m_materialTexture = texture.id;

    glActiveTexture(GL_TEXTURE0 + m_materialIndex);

    if (texture.id == 0)
        glBindTexture(GL_TEXTURE_2D, m_whiteTexture);
    else
        glBindTexture(GL_TEXTURE_2D, texture.id);
}

void CGL33ObjectRenderer::SetDetailTexture(const Texture& texture)
{
    if (m_detailTexture == texture.id) return;

    m_detailTexture = texture.id;

    glActiveTexture(GL_TEXTURE0 + m_detailIndex);

    if (texture.id == 0)
        glBindTexture(GL_TEXTURE_2D, m_whiteTexture);
    else
        glBindTexture(GL_TEXTURE_2D, texture.id);
}

void CGL33ObjectRenderer::SetShadowMap(const Texture& texture)
{
    if (m_shadowMap == texture.id) return;

    m_shadowMap = texture.id;

    glActiveTexture(GL_TEXTURE0 + m_shadowIndex);

    if (texture.id == 0)
        glBindTexture(GL_TEXTURE_2D, 0);
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
    glUniform3f(m_lightColor, color.r, color.g, color.b);
}

void CGL33ObjectRenderer::SetSky(const Color& color, float intensity)
{
    glUniform3f(m_skyColor, color.r, color.g, color.b);
    glUniform1f(m_skyIntensity, intensity);
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
    m_device->SetDepthTest(enabled);
}

void CGL33ObjectRenderer::SetDepthMask(bool enabled)
{
    m_device->SetDepthMask(enabled);
}

void CGL33ObjectRenderer::SetCullFace(CullFace mode)
{
    m_device->SetCullFace(mode);
}

void CGL33ObjectRenderer::SetTransparency(TransparencyMode mode)
{
    m_device->SetTransparency(mode);
}

void CGL33ObjectRenderer::SetUVTransform(const glm::vec2& offset, const glm::vec2& scale)
{
    glUniform2fv(m_uvOffset, 1, glm::value_ptr(offset));
    glUniform2fv(m_uvScale, 1, glm::value_ptr(scale));
}

void CGL33ObjectRenderer::SetTriplanarMode(bool enabled)
{
    glUniform1i(m_triplanarMode, enabled ? 1 : 0);
}

void CGL33ObjectRenderer::SetTriplanarScale(float scale)
{
    glUniform1f(m_triplanarScale, scale);
}

void CGL33ObjectRenderer::SetAlphaScissor(float alpha)
{
    glUniform1f(m_alphaScissor, alpha);
}

void CGL33ObjectRenderer::SetRecolor(bool enabled, const glm::vec3& from, const glm::vec3& to, float threshold)
{
    glUniform1i(m_recolor, enabled ? 1 : 0);

    if (enabled)
    {
        auto fromHSV = RGB2HSV(Color(from.r, from.g, from.b, 1.0));
        auto toHSV = RGB2HSV(Color(to.r, to.g, to.b, 1.0));

        glUniform3f(m_recolorFrom, fromHSV.h, fromHSV.s, fromHSV.v);
        glUniform3f(m_recolorTo, toHSV.h, toHSV.s, toHSV.v);
        glUniform1f(m_recolorThreshold, threshold);
    }
}

void CGL33ObjectRenderer::DrawObject(const CVertexBuffer* buffer)
{
    auto b = dynamic_cast<const CGL33VertexBuffer*>(buffer);

    if (b == nullptr) return;

    glBindVertexArray(b->GetVAO());

    glDrawArrays(TranslateGfxPrimitive(b->GetType()), 0, static_cast<GLsizei>(b->Size()));
}

void CGL33ObjectRenderer::DrawPrimitive(PrimitiveType type, int count, const Vertex3D* vertices)
{
    DrawPrimitives(type, 1, &count, vertices);
}

void CGL33ObjectRenderer::DrawPrimitives(PrimitiveType type, int drawCount, int count[], const Vertex3D* vertices)
{
    m_first.resize(drawCount);

    GLint offset = 0;

    for (int i = 0; i < drawCount; i++)
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
