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

#include "graphics/opengl/gl33device.h"

#include "common/config.h"

#include "common/config_file.h"
#include "common/image.h"
#include "common/logger.h"
#include "common/make_unique.h"

#include "graphics/core/light.h"

#include "graphics/engine/engine.h"

#include "graphics/opengl/glframebuffer.h"
#include "graphics/opengl/gl33renderers.h"
#include "graphics/opengl/gl33objectrenderer.h"
#include "graphics/opengl/gl33particlerenderer.h"

#include "math/geometry.h"


#include <SDL.h>
#include <physfs.h>

#include <cassert>

#include <glm/gtc/type_ptr.hpp>


// Graphics module namespace
namespace Gfx
{

CGL33VertexBuffer::CGL33VertexBuffer(PrimitiveType type, size_t size)
    : CVertexBuffer(type, size)
{
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_data.size() * sizeof(Vertex3D), nullptr, GL_STATIC_DRAW);

    // Vertex coordinate
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, position)));

    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, normal)));

    // Color
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, color)));

    // Texture coordinate 0
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, uv)));

    // Texture coordinate 1
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), reinterpret_cast<void*>(offsetof(Vertex3D, uv2)));
}

CGL33VertexBuffer::~CGL33VertexBuffer()
{
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
}

void CGL33VertexBuffer::Update()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_data.size() * sizeof(Vertex3D), m_data.data());
}

CGL33Device::CGL33Device(const DeviceConfig &config)
    : m_config(config)
{}


CGL33Device::~CGL33Device()
{
}

void CGL33Device::DebugHook()
{
    /* This function is only called here, so it can be used
     * as a breakpoint when debugging using gDEBugger */
    glColor3i(0, 0, 0);
}

void CGL33Device::DebugLights()
{
    Gfx::ColorHSV color(0.0, 1.0, 1.0);

    glLineWidth(3.0f);
    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);
    glDisable(GL_BLEND);

    glm::mat4 saveWorldMat = m_worldMat;
    glm::mat4 identity = glm::mat4(1.0f);
    SetTransform(TRANSFORM_WORLD, identity);

    for (int i = 0; i < static_cast<int>( m_lights.size() ); ++i)
    {
        color.h = static_cast<float>(i) / static_cast<float>(m_lights.size());
        if (m_lightsEnabled[i])
        {
            const Light& l = m_lights[i];
            if (l.type == LIGHT_DIRECTIONAL)
            {
                Gfx::VertexCol v[2];
                v[0].coord = -glm::normalize(l.direction) * 100.0f + glm::vec3(0.0f, 0.0f, 1.0f) * float(i);
                v[0].color = HSV2RGB(color);
                v[1].coord =  glm::normalize(l.direction) * 100.0f + glm::vec3(0.0f, 0.0f, 1.0f) * float(i);
                v[1].color = HSV2RGB(color);
                while (v[0].coord.y < 60.0f && v[0].coord.y < 60.0f)
                {
                    v[0].coord.y += 10.0f;
                    v[1].coord.y += 10.0f;
                }
                DrawPrimitive(PrimitiveType::LINES, v, 2);

                v[0].coord = v[1].coord + glm::normalize(v[0].coord - v[1].coord) * 50.0f;

                glLineWidth(10.0f);
                DrawPrimitive(PrimitiveType::LINES, v, 2);
                glLineWidth(3.0f);
            }
            else  if (l.type == LIGHT_POINT)
            {
                Gfx::VertexCol v[8];
                for (int i = 0; i < 8; ++i)
                    v[i].color = HSV2RGB(color);

                v[0].coord = l.position + glm::vec3(-1.0f, -1.0f, -1.0f) * 4.0f;
                v[1].coord = l.position + glm::vec3( 1.0f, -1.0f, -1.0f) * 4.0f;
                v[2].coord = l.position + glm::vec3( 1.0f,  1.0f, -1.0f) * 4.0f;
                v[3].coord = l.position + glm::vec3(-1.0f,  1.0f, -1.0f) * 4.0f;
                v[4].coord = l.position + glm::vec3(-1.0f, -1.0f, -1.0f) * 4.0f;
                DrawPrimitive(PrimitiveType::LINE_STRIP, v, 5);

                v[0].coord = l.position + glm::vec3(-1.0f, -1.0f,  1.0f) * 4.0f;
                v[1].coord = l.position + glm::vec3( 1.0f, -1.0f,  1.0f) * 4.0f;
                v[2].coord = l.position + glm::vec3( 1.0f,  1.0f,  1.0f) * 4.0f;
                v[3].coord = l.position + glm::vec3(-1.0f,  1.0f,  1.0f) * 4.0f;
                v[4].coord = l.position + glm::vec3(-1.0f, -1.0f,  1.0f) * 4.0f;
                DrawPrimitive(PrimitiveType::LINE_STRIP, v, 5);

                v[0].coord = l.position + glm::vec3(-1.0f, -1.0f, -1.0f) * 4.0f;
                v[1].coord = l.position + glm::vec3(-1.0f, -1.0f,  1.0f) * 4.0f;
                v[2].coord = l.position + glm::vec3( 1.0f, -1.0f, -1.0f) * 4.0f;
                v[3].coord = l.position + glm::vec3( 1.0f, -1.0f,  1.0f) * 4.0f;
                v[4].coord = l.position + glm::vec3( 1.0f,  1.0f, -1.0f) * 4.0f;
                v[5].coord = l.position + glm::vec3( 1.0f,  1.0f,  1.0f) * 4.0f;
                v[6].coord = l.position + glm::vec3(-1.0f,  1.0f, -1.0f) * 4.0f;
                v[7].coord = l.position + glm::vec3(-1.0f,  1.0f,  1.0f) * 4.0f;
                DrawPrimitive(PrimitiveType::LINES, v, 8);
            }
            else if (l.type == LIGHT_SPOT)
            {
                Gfx::VertexCol v[5];
                for (int i = 0; i < 5; ++i)
                    v[i].color = HSV2RGB(color);

                v[0].coord = l.position + glm::vec3(-1.0f,  0.0f, -1.0f) * 4.0f;
                v[1].coord = l.position + glm::vec3( 1.0f,  0.0f, -1.0f) * 4.0f;
                v[2].coord = l.position + glm::vec3( 1.0f,  0.0f,  1.0f) * 4.0f;
                v[3].coord = l.position + glm::vec3(-1.0f,  0.0f,  1.0f) * 4.0f;
                v[4].coord = l.position + glm::vec3(-1.0f,  0.0f, -1.0f) * 4.0f;
                DrawPrimitive(PrimitiveType::LINE_STRIP, v, 5);

                v[0].coord = l.position;
                v[1].coord = l.position + glm::normalize(l.direction) * 100.0f;
                glEnable(GL_LINE_STIPPLE);
                glLineStipple(3.0, 0xFF);
                DrawPrimitive(PrimitiveType::LINES, v, 2);
                glDisable(GL_LINE_STIPPLE);
            }
        }
    }

    glLineWidth(1.0f);
    glEnable(GL_LIGHTING);
    glDepthMask(GL_TRUE);
    glEnable(GL_BLEND);

    SetTransform(TRANSFORM_WORLD, saveWorldMat);
}

std::string CGL33Device::GetName()
{
    return std::string("OpenGL 3.3");
}

bool CGL33Device::Create()
{
    GetLogger()->Info("Creating CDevice - OpenGL 3.3\n");

    if (!InitializeGLEW())
    {
        m_errorMessage = "An error occurred while initializing GLEW.";
        return false;
    }

    // Extract OpenGL version
    int glMajor, glMinor;
    int glVersion = GetOpenGLVersion(glMajor, glMinor);

    if (glVersion < 32)
    {
        GetLogger()->Error("Unsupported OpenGL version: %d.%d\n", glMajor, glMinor);
        GetLogger()->Error("OpenGL 3.2 or newer is required to use this engine.\n");
        m_errorMessage = "It seems your graphics card does not support OpenGL 3.2.\n";
        m_errorMessage += "Please make sure you have appropriate hardware and newest drivers installed.\n";
        m_errorMessage += "(OpenGL 3.2 is roughly equivalent to Direct3D 10)\n\n";
        m_errorMessage += GetHardwareInfo();
        return false;
    }
    else if (glVersion < 33)
    {
        GetLogger()->Warn("Partially supported OpenGL version: %d.%d\n", glMajor, glMinor);
        GetLogger()->Warn("You may experience problems while running the game on this engine.\n");
        GetLogger()->Warn("OpenGL 3.3 or newer is recommended.\n");
    }
    else
    {
        const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
        const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));

        GetLogger()->Info("OpenGL %s\n", version);
        GetLogger()->Info("%s\n", renderer);
    }

    // Detect support of anisotropic filtering
    m_capabilities.anisotropySupported = AreExtensionsSupported("GL_EXT_texture_filter_anisotropic");
    if (m_capabilities.anisotropySupported)
    {
        // Obtain maximum anisotropy level available
        float level;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &level);
        m_capabilities.maxAnisotropy = static_cast<int>(level);

        GetLogger()->Info("Anisotropic filtering available\n");
        GetLogger()->Info("Maximum anisotropy: %d\n", m_capabilities.maxAnisotropy);
    }
    else
    {
        GetLogger()->Info("Anisotropic filtering not available\n");
    }

    m_capabilities.multisamplingSupported = true;
    glGetIntegerv(GL_MAX_SAMPLES, &m_capabilities.maxSamples);
    GetLogger()->Info("Multisampling supported, max samples: %d\n", m_capabilities.maxSamples);

    // Set just to be sure
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glViewport(0, 0, m_config.size.x, m_config.size.y);

    // this is set in shader
    m_capabilities.maxLights = 4;

    m_lights           = std::vector<Light>(m_capabilities.maxLights, Light());
    m_lightsEnabled    = std::vector<bool>(m_capabilities.maxLights, false);

    int maxTextures = 0;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextures);
    GetLogger()->Info("Maximum texture image units: %d\n", maxTextures);

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_capabilities.maxTextureSize);
    GetLogger()->Info("Maximum texture size: %d\n", m_capabilities.maxTextureSize);

    m_capabilities.multitexturingSupported = true;
    m_capabilities.maxTextures = maxTextures;

    m_currentTextures    = std::vector<Texture>           (maxTextures, Texture());
    m_texturesEnabled    = std::vector<bool>              (maxTextures, false);
    m_textureStageParams = std::vector<TextureStageParams>(maxTextures, TextureStageParams());

    m_capabilities.shadowMappingSupported = true;

    m_capabilities.framebufferSupported = true;
    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &m_capabilities.maxRenderbufferSize);
    GetLogger()->Info("Maximum renderbuffer size: %d\n", m_capabilities.maxRenderbufferSize);

    // Create shader program for normal rendering
    GLint shaders[2];
    char filename[64];

    strcpy(filename, "shaders/gl33/vs_normal.glsl");
    shaders[0] = LoadShader(GL_VERTEX_SHADER, filename);
    if (shaders[0] == 0)
    {
        m_errorMessage = GetLastShaderError();
        GetLogger()->Error("Cound not create vertex shader from file '%s'\n", filename);
        return false;
    }

    strcpy(filename, "shaders/gl33/fs_normal.glsl");
    shaders[1] = LoadShader(GL_FRAGMENT_SHADER, filename);
    if (shaders[1] == 0)
    {
        m_errorMessage = GetLastShaderError();
        GetLogger()->Error("Cound not create fragment shader from file '%s'\n", filename);
        return false;
    }

    m_normalProgram = LinkProgram(2, shaders);
    if (m_normalProgram == 0)
    {
        m_errorMessage = GetLastShaderError();
        GetLogger()->Error("Cound not link shader program for normal rendering\n");
        return false;
    }

    glDeleteShader(shaders[0]);
    glDeleteShader(shaders[1]);

    // Obtain uniform locations
    glUseProgram(m_normalProgram);

    {
        UniformLocations &uni = m_uniforms;

        uni.projectionMatrix = glGetUniformLocation(m_normalProgram, "uni_ProjectionMatrix");
        uni.viewMatrix = glGetUniformLocation(m_normalProgram, "uni_ViewMatrix");
        uni.modelMatrix = glGetUniformLocation(m_normalProgram, "uni_ModelMatrix");
        uni.normalMatrix = glGetUniformLocation(m_normalProgram, "uni_NormalMatrix");
        uni.shadowMatrix = glGetUniformLocation(m_normalProgram, "uni_ShadowMatrix");
        uni.cameraPosition = glGetUniformLocation(m_normalProgram, "uni_CameraPosition");

        uni.primaryTexture = glGetUniformLocation(m_normalProgram, "uni_PrimaryTexture");
        uni.secondaryTexture = glGetUniformLocation(m_normalProgram, "uni_SecondaryTexture");
        uni.shadowTexture = glGetUniformLocation(m_normalProgram, "uni_ShadowTexture");

        uni.textureEnabled[0] = glGetUniformLocation(m_normalProgram, "uni_PrimaryTextureEnabled");
        uni.textureEnabled[1] = glGetUniformLocation(m_normalProgram, "uni_SecondaryTextureEnabled");
        uni.textureEnabled[2] = glGetUniformLocation(m_normalProgram, "uni_ShadowTextureEnabled");

        uni.fogEnabled = glGetUniformLocation(m_normalProgram, "uni_FogEnabled");
        uni.fogRange = glGetUniformLocation(m_normalProgram, "uni_FogRange");
        uni.fogColor = glGetUniformLocation(m_normalProgram, "uni_FogColor");

        uni.alphaTestEnabled = glGetUniformLocation(m_normalProgram, "uni_AlphaTestEnabled");
        uni.alphaReference = glGetUniformLocation(m_normalProgram, "uni_AlphaReference");

        uni.shadowColor = glGetUniformLocation(m_normalProgram, "uni_ShadowColor");

        uni.lightCount = glGetUniformLocation(m_normalProgram, "uni_LightCount");

        uni.ambientColor = glGetUniformLocation(m_normalProgram, "uni_AmbientColor");
        uni.diffuseColor = glGetUniformLocation(m_normalProgram, "uni_DiffuseColor");
        uni.specularColor = glGetUniformLocation(m_normalProgram, "uni_SpecularColor");

        GLchar name[64];
        for (int i = 0; i < m_capabilities.maxLights; i++)
        {
            LightLocations &light = uni.lights[i];

            sprintf(name, "uni_Light[%d].Enabled", i);
            light.enabled = glGetUniformLocation(m_normalProgram, name);

            sprintf(name, "uni_Light[%d].Position", i);
            light.position = glGetUniformLocation(m_normalProgram, name);

            sprintf(name, "uni_Light[%d].Ambient", i);
            light.ambient = glGetUniformLocation(m_normalProgram, name);

            sprintf(name, "uni_Light[%d].Diffuse", i);
            light.diffuse = glGetUniformLocation(m_normalProgram, name);

            sprintf(name, "uni_Light[%d].Specular", i);
            light.specular = glGetUniformLocation(m_normalProgram, name);

            sprintf(name, "uni_Light[%d].Attenuation", i);
            light.attenuation = glGetUniformLocation(m_normalProgram, name);
        }

        // Set default uniform values
        glm::mat4 matrix = glm::mat4(1.0f);

        glUniformMatrix4fv(uni.projectionMatrix, 1, GL_FALSE, glm::value_ptr(matrix));
        glUniformMatrix4fv(uni.viewMatrix, 1, GL_FALSE, glm::value_ptr(matrix));
        glUniformMatrix4fv(uni.modelMatrix, 1, GL_FALSE, glm::value_ptr(matrix));
        glUniformMatrix4fv(uni.normalMatrix, 1, GL_FALSE, glm::value_ptr(matrix));
        glUniformMatrix4fv(uni.shadowMatrix, 1, GL_FALSE, glm::value_ptr(matrix));
        glUniform3f(uni.cameraPosition, 0.0f, 0.0f, 0.0f);

        glUniform1i(uni.primaryTexture, 0);
        glUniform1i(uni.secondaryTexture, 1);
        glUniform1i(uni.shadowTexture, 2);

        glUniform1i(uni.textureEnabled[0], 0);
        glUniform1i(uni.textureEnabled[1], 0);
        glUniform1i(uni.textureEnabled[2], 0);

        glUniform4f(uni.ambientColor, 0.4f, 0.4f, 0.4f, 1.0f);
        glUniform4f(uni.diffuseColor, 0.8f, 0.8f, 0.8f, 1.0f);
        glUniform4f(uni.specularColor, 0.3f, 0.3f, 0.3f, 1.0f);

        glUniform1i(uni.fogEnabled, 0);
        glUniform2f(uni.fogRange, 100.0f, 200.0f);
        glUniform4f(uni.fogColor, 0.8f, 0.8f, 0.8f, 1.0f);

        glUniform1f(uni.shadowColor, 0.5f);

        glUniform1i(uni.alphaTestEnabled, 0);
        glUniform1f(uni.alphaReference, 0.5f);

        glUniform1i(uni.lightCount, 0);
    }

    m_uiRenderer = std::make_unique<CGL33UIRenderer>(this);
    m_terrainRenderer = std::make_unique<CGL33TerrainRenderer>(this);
    m_objectRenderer = std::make_unique<CGL33ObjectRenderer>(this);
    m_particleRenderer = std::make_unique<CGL33ParticleRenderer>(this);
    m_shadowRenderer = std::make_unique<CGL33ShadowRenderer>(this);

    glUseProgram(m_normalProgram);

    // create default framebuffer object
    FramebufferParams framebufferParams;

    framebufferParams.width = m_config.size.x;
    framebufferParams.height = m_config.size.y;
    framebufferParams.depth = m_config.depthSize;

    m_framebuffers["default"] = MakeUnique<CDefaultFramebuffer>(framebufferParams);

    // create dynamic buffer
    glGenVertexArrays(1, &m_dynamicBuffer.vao);

    m_dynamicBuffer.size = 4 * 1024 * 1024;
    m_dynamicBuffer.offset = 0;

    glGenBuffers(1, &m_dynamicBuffer.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_dynamicBuffer.vbo);
    glBufferData(GL_ARRAY_BUFFER, m_dynamicBuffer.size, nullptr, GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_vboMemory += m_dynamicBuffer.size;

    GetLogger()->Info("CDevice created successfully\n");

    return true;
}

void CGL33Device::Destroy()
{
    // delete shader program
    glUseProgram(0);
    glDeleteProgram(m_normalProgram);

    // delete framebuffers
    for (auto& framebuffer : m_framebuffers)
        framebuffer.second->Destroy();

    m_framebuffers.clear();

    // Delete the remaining textures
    // Should not be strictly necessary, but just in case
    DestroyAllTextures();

    // delete dynamic buffer
    glDeleteVertexArrays(1, &m_dynamicBuffer.vao);
    glDeleteBuffers(1, &m_dynamicBuffer.vbo);

    m_vboMemory -= m_dynamicBuffer.size;

    m_lights.clear();
    m_lightsEnabled.clear();

    m_currentTextures.clear();
    m_texturesEnabled.clear();
    m_textureStageParams.clear();

    for (auto buffer : m_buffers)
        delete buffer;

    m_buffers.clear();

    m_uiRenderer = nullptr;
    m_terrainRenderer = nullptr;
    m_objectRenderer = nullptr;
    m_particleRenderer = nullptr;
    m_shadowRenderer = nullptr;
}

void CGL33Device::ConfigChanged(const DeviceConfig& newConfig)
{
    m_config = newConfig;

    // Reset state
    m_lighting = false;

    glViewport(0, 0, m_config.size.x, m_config.size.y);

    // create default framebuffer object
    FramebufferParams framebufferParams;

    framebufferParams.width = m_config.size.x;
    framebufferParams.height = m_config.size.y;
    framebufferParams.depth = m_config.depthSize;

    m_framebuffers["default"] = MakeUnique<CDefaultFramebuffer>(framebufferParams);
}

void CGL33Device::BeginScene()
{
    Clear();

    glUniformMatrix4fv(m_uniforms.projectionMatrix, 1, GL_FALSE, glm::value_ptr(m_projectionMat));
    glUniformMatrix4fv(m_uniforms.viewMatrix, 1, GL_FALSE, glm::value_ptr(m_viewMat));
    glUniformMatrix4fv(m_uniforms.modelMatrix, 1, GL_FALSE, glm::value_ptr(m_worldMat));
}

void CGL33Device::EndScene()
{
    m_uiRenderer->Flush();

#ifdef DEV_BUILD
    CheckGLErrors();
#endif
}

void CGL33Device::Clear()
{
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

CUIRenderer* CGL33Device::GetUIRenderer()
{
    return m_uiRenderer.get();
}

CTerrainRenderer* CGL33Device::GetTerrainRenderer()
{
    return m_terrainRenderer.get();
}

CObjectRenderer* CGL33Device::GetObjectRenderer()
{
    return m_objectRenderer.get();
}

CParticleRenderer* CGL33Device::GetParticleRenderer()
{
    return m_particleRenderer.get();
}

CShadowRenderer* CGL33Device::GetShadowRenderer()
{
    return m_shadowRenderer.get();
}

void CGL33Device::Restore()
{
    m_uiRenderer->Flush();
    glUseProgram(m_normalProgram);


    //UpdateTextureState(0);
    //UpdateTextureState(1);
    //UpdateTextureState(2);
}

void CGL33Device::SetTransform(TransformType type, const glm::mat4 &matrix)
{
    if      (type == TRANSFORM_WORLD)
    {
        m_worldMat = matrix;
        glUniformMatrix4fv(m_uniforms.modelMatrix, 1, GL_FALSE, glm::value_ptr(m_worldMat));

        m_modelviewMat = m_viewMat * m_worldMat;
        m_combinedMatrixOutdated = true;

        // normal transform
        glm::mat4 normalMat = glm::inverse(normalMat);

        glUniformMatrix4fv(m_uniforms.normalMatrix, 1, GL_TRUE, glm::value_ptr(normalMat));
    }
    else if (type == TRANSFORM_VIEW)
    {
        glm::mat4 scale = glm::mat4(1.0f);
        scale[2][2] = -1.0f;
        m_viewMat = scale * matrix;

        m_modelviewMat = m_viewMat * m_worldMat;
        m_combinedMatrixOutdated = true;

        glUniformMatrix4fv(m_uniforms.viewMatrix, 1, GL_FALSE, glm::value_ptr(m_viewMat));

        if (m_uniforms.cameraPosition >= 0)
        {
            glm::vec3 cameraPosition = { 0, 0, 0 };
            cameraPosition = glm::vec3(glm::inverse(m_viewMat) * glm::vec4(cameraPosition, 1.0f));
            glUniform3fv(m_uniforms.cameraPosition, 1, glm::value_ptr(cameraPosition));
        }
    }
    else if (type == TRANSFORM_PROJECTION)
    {
        m_projectionMat = matrix;
        m_combinedMatrixOutdated = true;

        glUniformMatrix4fv(m_uniforms.projectionMatrix, 1, GL_FALSE, glm::value_ptr(m_projectionMat));
    }
    else if (type == TRANSFORM_SHADOW)
    {
        glm::mat4 temp = matrix;
        glUniformMatrix4fv(m_uniforms.shadowMatrix, 1, GL_FALSE, glm::value_ptr(temp));
    }
    else
    {
        assert(false);
    }
}

void CGL33Device::SetMaterial(const Material &material)
{
    m_material = material;

    glUniform4fv(m_uniforms.ambientColor, 1, m_material.ambient.Array());
    glUniform4fv(m_uniforms.diffuseColor, 1, m_material.diffuse.Array());
    glUniform4fv(m_uniforms.specularColor, 1, m_material.specular.Array());
}

int CGL33Device::GetMaxLightCount()
{
    return m_capabilities.maxLights;
}

void CGL33Device::SetLight(int index, const Light &light)
{
    assert(index >= 0);
    assert(index < static_cast<int>( m_lights.size() ));

    m_lights[index] = light;

    m_updateLights = true;
}

void CGL33Device::SetLightEnabled(int index, bool enabled)
{
    assert(index >= 0);
    assert(index < static_cast<int>( m_lights.size() ));

    m_lightsEnabled[index] = enabled;

    m_updateLights = true;
}

/** If image is invalid, returns invalid texture.
    Otherwise, returns pointer to new Texture struct.
    This struct must not be deleted in other way than through DeleteTexture() */
Texture CGL33Device::CreateTexture(CImage *image, const TextureCreateParams &params)
{
    ImageData *data = image->GetData();
    if (data == nullptr)
    {
        GetLogger()->Error("Invalid texture data\n");
        return Texture(); // invalid texture
    }

    glm::ivec2 originalSize = image->GetSize();

    if (params.padToNearestPowerOfTwo)
        image->PadToNearestPowerOfTwo();

    Texture tex = CreateTexture(data, params);
    tex.originalSize = originalSize;

    return tex;
}

Texture CGL33Device::CreateTexture(ImageData *data, const TextureCreateParams &params)
{
    Texture result;

    result.size.x = data->surface->w;
    result.size.y = data->surface->h;

    result.originalSize = result.size;

    glGenTextures(1, &result.id);

    BindTexture(m_freeTexture, result.id);

    // Set texture parameters
    GLint minF = GL_NEAREST, magF = GL_NEAREST;
    int mipmapLevel = 1;

    switch (params.filter)
    {
    case TEX_FILTER_NEAREST:
        minF = GL_NEAREST;
        magF = GL_NEAREST;
        break;
    case TEX_FILTER_BILINEAR:
        minF = GL_LINEAR;
        magF = GL_LINEAR;
        break;
    case TEX_FILTER_TRILINEAR:
        minF = GL_LINEAR_MIPMAP_LINEAR;
        magF = GL_LINEAR;
        mipmapLevel = CEngine::GetInstance().GetTextureMipmapLevel();
        break;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minF);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magF);

    // Set mipmap level and automatic mipmap generation if neccesary
    if (params.mipmap)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipmapLevel - 1);
    }
    else
    {
        // Has to be set to 0 because no mipmaps are generated
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    }

    // Set anisotropy level if available
    if (m_capabilities.anisotropySupported)
    {
        float level = Math::Min(m_capabilities.maxAnisotropy, CEngine::GetInstance().GetTextureAnisotropyLevel());

        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, level);
    }

    PreparedTextureData texData = PrepareTextureData(data, params.format);
    result.alpha = texData.alpha;

    glPixelStorei(GL_UNPACK_ROW_LENGTH, texData.actualSurface->pitch / texData.actualSurface->format->BytesPerPixel);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texData.actualSurface->w, texData.actualSurface->h,
                 0, texData.sourceFormat, GL_UNSIGNED_BYTE, texData.actualSurface->pixels);

    if (params.mipmap)
        glGenerateMipmap(GL_TEXTURE_2D);

    SDL_FreeSurface(texData.convertedSurface);

    m_allTextures.insert(result);

    return result;
}

Texture CGL33Device::CreateDepthTexture(int width, int height, int depth)
{
    Texture result;

    result.alpha = false;
    result.size.x = width;
    result.size.y = height;

    glGenTextures(1, &result.id);

    BindTexture(m_freeTexture, result.id);

    GLuint format = GL_DEPTH_COMPONENT;

    switch (depth)
    {
    case 16:
        format = GL_DEPTH_COMPONENT16;
        break;
    case 24:
        format = GL_DEPTH_COMPONENT24;
        break;
    case 32:
        format = GL_DEPTH_COMPONENT32;
        break;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_DEPTH_COMPONENT, GL_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    float color[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

    return result;
}

void CGL33Device::UpdateTexture(const Texture& texture, const glm::ivec2& offset, ImageData* data, TexImgFormat format)
{
    if (texture.id == 0) return;

    BindTexture(m_freeTexture, texture.id);

    PreparedTextureData texData = PrepareTextureData(data, format);

    glPixelStorei(GL_UNPACK_ROW_LENGTH, texData.actualSurface->pitch / texData.actualSurface->format->BytesPerPixel);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexSubImage2D(GL_TEXTURE_2D, 0, offset.x, offset.y, texData.actualSurface->w, texData.actualSurface->h,
                    texData.sourceFormat, GL_UNSIGNED_BYTE, texData.actualSurface->pixels);

    glGenerateMipmap(GL_TEXTURE_2D);

    SDL_FreeSurface(texData.convertedSurface);
}

void CGL33Device::DestroyTexture(const Texture &texture)
{
    // Unbind the texture if in use anywhere
    for (int index = 0; index < static_cast<int>( m_currentTextures.size() ); ++index)
    {
        if (m_currentTextures[index] == texture)
            SetTexture(index, Texture()); // set to invalid texture
    }

    auto it = m_allTextures.find(texture);
    if (it != m_allTextures.end())
    {
        glDeleteTextures(1, &texture.id);
        m_allTextures.erase(it);
    }
}

void CGL33Device::DestroyAllTextures()
{
    // Unbind all texture stages
    for (int index = 0; index < static_cast<int>( m_currentTextures.size() ); ++index)
        SetTexture(index, Texture());

    for (auto it = m_allTextures.begin(); it != m_allTextures.end(); ++it)
        glDeleteTextures(1, &(*it).id);

    m_allTextures.clear();
}

int CGL33Device::GetMaxTextureStageCount()
{
    return m_currentTextures.size();
}

/**
  If \a texture is invalid, unbinds the given texture.
  If valid, binds the texture and enables the given texture stage.
  The setting is remembered, even if texturing is disabled at the moment. */
void CGL33Device::SetTexture(int index, const Texture &texture)
{
    assert(index >= 0 && index < static_cast<int>( m_currentTextures.size() ));

    if (m_currentTextures[index].id == texture.id)
        return;

    BindTexture(index, texture.id);

    m_currentTextures[index] = texture; // remember the new value

    // Params need to be updated for the new bound texture
    UpdateTextureParams(index);
    UpdateTextureState(index);
}

void CGL33Device::SetTexture(int index, unsigned int textureId)
{
    assert(index >= 0 && index < static_cast<int>( m_currentTextures.size() ));

    if (m_currentTextures[index].id == textureId)
        return; // nothing to do

    BindTexture(index, textureId);

    m_currentTextures[index].id = textureId;

    // Params need to be updated for the new bound texture
    UpdateTextureParams(index);
    UpdateTextureState(index);
}

void CGL33Device::SetTextureEnabled(int index, bool enabled)
{
    assert(index >= 0 && index < static_cast<int>( m_currentTextures.size() ));

    if (m_texturesEnabled[index] == enabled)
        return;

    m_texturesEnabled[index] = enabled;

    UpdateTextureState(index);
}

/**
  Sets the texture parameters for the given texture stage.
  If the given texture was not set (bound) yet, nothing happens.
  The settings are remembered, even if texturing is disabled at the moment. */
void CGL33Device::SetTextureStageParams(int index, const TextureStageParams &params)
{
    assert(index >= 0 && index < static_cast<int>( m_currentTextures.size() ));

    // Remember the settings
    m_textureStageParams[index] = params;

    UpdateTextureParams(index);
}

void CGL33Device::UpdateTextureParams(int index)
{
    assert(index >= 0 && index < static_cast<int>( m_currentTextures.size() ));

    // Don't actually do anything if texture not set
    if (! m_currentTextures[index].Valid())
        return;

    const TextureStageParams &params = m_textureStageParams[index];

    glActiveTexture(GL_TEXTURE0 + index);

    if      (params.wrapS == TEX_WRAP_CLAMP)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    else if (params.wrapS == TEX_WRAP_CLAMP_TO_BORDER)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    else if (params.wrapS == TEX_WRAP_REPEAT)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    else  assert(false);

    if      (params.wrapT == TEX_WRAP_CLAMP)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    else if (params.wrapT == TEX_WRAP_CLAMP_TO_BORDER)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    else if (params.wrapT == TEX_WRAP_REPEAT)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    else  assert(false);
}

void CGL33Device::SetTextureStageWrap(int index, TexWrapMode wrapS, TexWrapMode wrapT)
{
    assert(index >= 0 && index < static_cast<int>( m_currentTextures.size() ));

    // Remember the settings
    m_textureStageParams[index].wrapS = wrapS;
    m_textureStageParams[index].wrapT = wrapT;

    // Don't actually do anything if texture not set
    if (! m_currentTextures[index].Valid())
        return;

    glActiveTexture(GL_TEXTURE0 + index);

    if      (wrapS == TEX_WRAP_CLAMP)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    else if (wrapS == TEX_WRAP_CLAMP_TO_BORDER)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    else if (wrapS == TEX_WRAP_REPEAT)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    else  assert(false);

    if      (wrapT == TEX_WRAP_CLAMP)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    else if (wrapT == TEX_WRAP_CLAMP_TO_BORDER)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    else if (wrapT == TEX_WRAP_REPEAT)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    else  assert(false);
}

void CGL33Device::DrawPrimitive(PrimitiveType type, const Vertex *vertices, int vertexCount, Color color)
{
    if (m_updateLights) UpdateLights();

    unsigned int size = vertexCount * sizeof(Vertex);

    DynamicBuffer& buffer = m_dynamicBuffer;

    BindVAO(buffer.vao);
    BindVBO(buffer.vbo);

    unsigned int offset = UploadVertexData(buffer, vertices, size);

    // Vertex coordinate
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<void*>(offset + offsetof(Vertex, coord)));

    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<void*>(offset + offsetof(Vertex, normal)));

    // Color
    glDisableVertexAttribArray(2);
    glVertexAttrib4fv(2, color.Array());

    // Texture coordinate 0
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<void*>(offset + offsetof(Vertex, texCoord)));

    // Texture coordinate 1
    glDisableVertexAttribArray(4);
    glVertexAttrib2f(4, 0.0f, 0.0f);

    glDrawArrays(TranslateGfxPrimitive(type), 0, vertexCount);
}

void CGL33Device::DrawPrimitive(PrimitiveType type, const VertexCol *vertices, int vertexCount)
{
    if (m_updateLights) UpdateLights();

    unsigned int size = vertexCount * sizeof(VertexCol);

    DynamicBuffer& buffer = m_dynamicBuffer;

    BindVAO(buffer.vao);
    BindVBO(buffer.vbo);

    unsigned int offset = UploadVertexData(buffer, vertices, size);

    // Vertex coordinate
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexCol),
        reinterpret_cast<void*>(offset + offsetof(VertexCol, coord)));

    // Normal
    glDisableVertexAttribArray(1);
    glVertexAttrib3f(1, 0.0f, 0.0f, 1.0f);

    // Color
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexCol),
        reinterpret_cast<void*>(offset + offsetof(VertexCol, color)));

    // Texture coordinate 0
    glDisableVertexAttribArray(3);
    glVertexAttrib2f(3, 0.0f, 0.0f);

    // Texture coordinate 1
    glDisableVertexAttribArray(4);
    glVertexAttrib2f(4, 0.0f, 0.0f);

    glDrawArrays(TranslateGfxPrimitive(type), 0, vertexCount);
}

void CGL33Device::DrawPrimitive(PrimitiveType type, const Vertex3D* vertices, int vertexCount)
{
    if (m_updateLights) UpdateLights();

    unsigned int size = vertexCount * sizeof(Vertex3D);

    DynamicBuffer& buffer = m_dynamicBuffer;

    BindVAO(buffer.vao);
    BindVBO(buffer.vbo);

    unsigned int offset = UploadVertexData(buffer, vertices, size);

    // Vertex coordinate
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D),
        reinterpret_cast<void*>(offset + offsetof(Vertex3D, position)));

    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D),
        reinterpret_cast<void*>(offset + offsetof(Vertex3D, normal)));

    // Color
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex3D),
        reinterpret_cast<void*>(offset + offsetof(Vertex3D, color)));

    // Texture coordinate 0
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3D),
        reinterpret_cast<void*>(offset + offsetof(Vertex3D, uv)));

    // Texture coordinate 1
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3D),
        reinterpret_cast<void*>(offset + offsetof(Vertex3D, uv2)));

    glDrawArrays(TranslateGfxPrimitive(type), 0, vertexCount);
}

void CGL33Device::DrawPrimitives(PrimitiveType type, const Vertex *vertices,
    int first[], int count[], int drawCount, Color color)
{
    if (m_updateLights) UpdateLights();

    int vertexCount = 0;

    for (int i = 0; i < drawCount; i++)
    {
        int currentCount = first[i] + count[i];

        if (currentCount > vertexCount)
            vertexCount = currentCount;
    }

    unsigned int size = vertexCount * sizeof(Vertex);

    DynamicBuffer& buffer = m_dynamicBuffer;

    BindVAO(buffer.vao);
    BindVBO(buffer.vbo);

    unsigned int offset = UploadVertexData(buffer, vertices, size);

    // Vertex coordinate
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<void*>(offset + offsetof(Vertex, coord)));

    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<void*>(offset + offsetof(Vertex, normal)));

    // Color
    glDisableVertexAttribArray(2);
    glVertexAttrib4fv(2, color.Array());

    // Texture coordinate 0
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<void*>(offset + offsetof(Vertex, texCoord)));

    // Texture coordinate 1
    glDisableVertexAttribArray(4);
    glVertexAttrib2f(4, 0.0f, 0.0f);

    glMultiDrawArrays(TranslateGfxPrimitive(type), first, count, drawCount);
}

void CGL33Device::DrawPrimitives(PrimitiveType type, const VertexCol *vertices,
    int first[], int count[], int drawCount)
{
    if (m_updateLights) UpdateLights();

    int vertexCount = 0;

    for (int i = 0; i < drawCount; i++)
    {
        int currentCount = first[i] + count[i];

        if (currentCount > vertexCount)
            vertexCount = currentCount;
    }

    unsigned int size = vertexCount * sizeof(VertexCol);

    DynamicBuffer& buffer = m_dynamicBuffer;

    BindVAO(buffer.vao);
    BindVBO(buffer.vbo);

    unsigned int offset = UploadVertexData(buffer, vertices, size);

    // Vertex coordinate
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexCol),
        reinterpret_cast<void*>(offset + offsetof(VertexCol, coord)));

    // Normal
    glDisableVertexAttribArray(1);
    glVertexAttrib3f(1, 0.0f, 0.0f, 1.0f);

    // Color
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexCol),
        reinterpret_cast<void*>(offset + offsetof(VertexCol, color)));

    // Texture coordinate 0
    glDisableVertexAttribArray(3);
    glVertexAttrib2f(3, 0.0f, 0.0f);

    // Texture coordinate 1
    glDisableVertexAttribArray(4);
    glVertexAttrib2f(4, 0.0f, 0.0f);

    glMultiDrawArrays(TranslateGfxPrimitive(type), first, count, drawCount);
}

CVertexBuffer* CGL33Device::CreateVertexBuffer(PrimitiveType primitiveType, const Vertex3D* vertices, int vertexCount)
{
    auto buffer = new CGL33VertexBuffer(primitiveType, vertexCount);

    buffer->SetData(vertices, 0, vertexCount);
    buffer->Update();

    m_buffers.insert(buffer);

    return buffer;
}

void CGL33Device::DestroyVertexBuffer(CVertexBuffer* buffer)
{
    if (m_buffers.count(buffer) == 0) return;

    m_buffers.erase(buffer);

    delete buffer;
}

void CGL33Device::SetViewport(int x, int y, int width, int height)
{
    glViewport(x, y, width, height);
}

void CGL33Device::SetRenderState(RenderState state, bool enabled)
{
    if (state == RENDER_STATE_DEPTH_WRITE)
    {
        glDepthMask(enabled ? GL_TRUE : GL_FALSE);
        return;
    }
    else if (state == RENDER_STATE_LIGHTING)
    {
        m_lighting = enabled;

        m_updateLights = true;

        //glUniform1i(m_uniforms.lightingEnabled, enabled ? 1 : 0);

        return;
    }
    else if (state == RENDER_STATE_FOG)
    {
        glUniform1i(m_uniforms.fogEnabled, enabled ? 1 : 0);

        return;
    }
    else if (state == RENDER_STATE_ALPHA_TEST)
    {
        glUniform1i(m_uniforms.alphaTestEnabled, enabled ? 1 : 0);

        return;
    }
    else if (state == RENDER_STATE_SHADOW_MAPPING)
    {
        SetTextureEnabled(TEXTURE_SHADOW, enabled);

        return;
    }

    GLenum flag = 0;

    switch (state)
    {
        case RENDER_STATE_BLENDING:    flag = GL_BLEND; break;
        case RENDER_STATE_DEPTH_TEST:  flag = GL_DEPTH_TEST; break;
        case RENDER_STATE_CULLING:     flag = GL_CULL_FACE; break;
        default: assert(false); break;
    }

    if (enabled)
        glEnable(flag);
    else
        glDisable(flag);
}

void CGL33Device::SetColorMask(bool red, bool green, bool blue, bool alpha)
{
    glColorMask(red, green, blue, alpha);
}

void CGL33Device::SetAlphaTestFunc(CompFunc func, float refValue)
{
    glUniform1f(m_uniforms.alphaReference, refValue);
}

void CGL33Device::SetBlendFunc(BlendFunc srcBlend, BlendFunc dstBlend)
{
    glBlendFunc(TranslateGfxBlendFunc(srcBlend), TranslateGfxBlendFunc(dstBlend));
}

void CGL33Device::SetClearColor(const Color &color)
{
    glClearColor(color.r, color.g, color.b, color.a);
}

void CGL33Device::SetGlobalAmbient(const Color &color)
{
    //glLightModelfv(GL_LIGHT_MODEL_AMBIENT, color.Array());
}

void CGL33Device::SetFogParams(FogMode mode, const Color &color, float start, float end, float density)
{
    // TODO: reimplement

    glUniform2f(m_uniforms.fogRange, start, end);
    glUniform4f(m_uniforms.fogColor, color.r, color.g, color.b, color.a);

    /*
    if      (mode == FOG_LINEAR) glFogi(GL_FOG_MODE, GL_LINEAR);
    else if (mode == FOG_EXP)    glFogi(GL_FOG_MODE, GL_EXP);
    else if (mode == FOG_EXP2)   glFogi(GL_FOG_MODE, GL_EXP2);
    else assert(false);

    glFogf(GL_FOG_START,   start);
    glFogf(GL_FOG_END,     end);
    glFogf(GL_FOG_DENSITY, density);
    glFogfv(GL_FOG_COLOR,  color.Array());
    // */
}

void CGL33Device::SetCullMode(CullMode mode)
{
    // Cull clockwise back faces, so front face is the opposite
    // (assuming GL_CULL_FACE is GL_BACK)
    if      (mode == CULL_CW ) glFrontFace(GL_CCW);
    else if (mode == CULL_CCW) glFrontFace(GL_CW);
    else assert(false);
}

void CGL33Device::SetShadowColor(float value)
{
    glUniform1f(m_uniforms.shadowColor, value);
}

void CGL33Device::SetFillMode(FillMode mode)
{
    if      (mode == FillMode::POINT) glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    else if (mode == FillMode::LINES) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else if (mode == FillMode::POLY)  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else assert(false);
}

void CGL33Device::CopyFramebufferToTexture(Texture& texture, int xOffset, int yOffset, int x, int y, int width, int height)
{
    if (texture.id == 0) return;

    BindTexture(m_freeTexture, texture.id);

    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, xOffset, yOffset, x, y, width, height);
}

std::unique_ptr<CFrameBufferPixels> CGL33Device::GetFrameBufferPixels() const
{
    return GetGLFrameBufferPixels(m_config.size);
}

CFramebuffer* CGL33Device::GetFramebuffer(std::string name)
{
    auto it = m_framebuffers.find(name);
    if (it == m_framebuffers.end())
        return nullptr;

    return it->second.get();
}

CFramebuffer* CGL33Device::CreateFramebuffer(std::string name, const FramebufferParams& params)
{
    // existing framebuffer was found
    if (m_framebuffers.find(name) != m_framebuffers.end())
    {
        return nullptr;
    }

    auto framebuffer = MakeUnique<CGLFramebuffer>(params);
    if (!framebuffer->Create()) return nullptr;

    CFramebuffer* framebufferPtr = framebuffer.get();
    m_framebuffers[name] = std::move(framebuffer);
    return framebufferPtr;
}

void CGL33Device::DeleteFramebuffer(std::string name)
{
    // can't delete default framebuffer
    if (name == "default") return;

    auto it = m_framebuffers.find(name);
    if (it != m_framebuffers.end())
    {
        it->second->Destroy();
        m_framebuffers.erase(it);
    }
}

inline void CGL33Device::UpdateTextureState(int index)
{
    bool enabled = m_texturesEnabled[index] && (m_currentTextures[index].id != 0);
    glUniform1i(m_uniforms.textureEnabled[index], enabled ? 1 : 0);
}

void CGL33Device::UpdateLights()
{
    m_updateLights = false;

    // If not in normal rendering mode, return immediately
    if (m_mode != 0) return;

    // Lighting enabled
    if (m_lighting)
    {
        int index = 0;

        // Iterate all lights
        for (unsigned int i = 0; i < m_lights.size(); i++)
        {
            // If disabled, ignore and continue
            if (!m_lightsEnabled[i]) continue;

            // If not directional, ignore and continue
            if (m_lights[i].type != LIGHT_DIRECTIONAL) continue;

            Light &light = m_lights[i];
            LightLocations &uni = m_uniforms.lights[index];

            glUniform4fv(uni.ambient, 1, light.ambient.Array());
            glUniform4fv(uni.diffuse, 1, light.diffuse.Array());
            glUniform4fv(uni.specular, 1, light.specular.Array());

            glUniform4f(uni.position, -light.direction.x, -light.direction.y, -light.direction.z, 0.0f);

            index++;
        }

        glUniform1i(m_uniforms.lightCount, index);
    }
    // Lighting disabled
    else
    {
        glUniform1i(m_uniforms.lightCount, 0);
    }
}

inline void CGL33Device::BindVBO(GLuint vbo)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    m_currentVBO = vbo;
}

inline void CGL33Device::BindVAO(GLuint vao)
{
    glBindVertexArray(vao);
    m_currentVAO = vao;
}

inline void CGL33Device::BindTexture(int index, GLuint texture)
{
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, texture);
}

unsigned int CGL33Device::UploadVertexData(DynamicBuffer& buffer, const void* data, unsigned int size)
{
    unsigned int nextOffset = buffer.offset + size;

    // buffer limit exceeded
    // invalidate buffer for the next round of buffer streaming
    if (nextOffset > buffer.size)
    {
        glBufferData(GL_ARRAY_BUFFER, buffer.size, nullptr, GL_STREAM_DRAW);

        buffer.offset = 0;
        nextOffset = size;
    }

    unsigned int currentOffset = buffer.offset;

    // map buffer for unsynchronized copying
    void* ptr = glMapBufferRange(GL_ARRAY_BUFFER, currentOffset, size,
        GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);

    if (ptr != nullptr)
    {
        memcpy(ptr, data, size);

        glUnmapBuffer(GL_ARRAY_BUFFER);
    }
    // mapping failed, we must upload data with glBufferSubData
    else
    {
        GetLogger()->Debug("Buffer mapping failed (offset %d, size %d)\n", currentOffset, size);
        glBufferSubData(GL_ARRAY_BUFFER, currentOffset, size, data);
    }

    buffer.offset = nextOffset;

    return currentOffset;
}

bool CGL33Device::IsAnisotropySupported()
{
    return m_capabilities.anisotropySupported;
}

int CGL33Device::GetMaxAnisotropyLevel()
{
    return m_capabilities.maxAnisotropy;
}

int CGL33Device::GetMaxSamples()
{
    return m_capabilities.maxSamples;
}

bool CGL33Device::IsShadowMappingSupported()
{
    return true;
}

int CGL33Device::GetMaxTextureSize()
{
    return m_capabilities.maxTextureSize;
}

bool CGL33Device::IsFramebufferSupported()
{
    return true;
}

} // namespace Gfx
