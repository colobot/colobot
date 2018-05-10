/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
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


#include "graphics/opengl/gl21device.h"

#include "common/config.h"

#include "common/config_file.h"
#include "common/image.h"
#include "common/logger.h"

#include "graphics/core/light.h"

#include "graphics/engine/engine.h"

#include "graphics/opengl/glframebuffer.h"

#include "math/geometry.h"


#include <SDL.h>
#include <physfs.h>

#include <cmath>
#include <cassert>


// Graphics module namespace
namespace Gfx
{

CGL21Device::CGL21Device(const DeviceConfig &config)
    : m_config(config)
{}


CGL21Device::~CGL21Device()
{
}

void CGL21Device::DebugHook()
{
    /* This function is only called here, so it can be used
     * as a breakpoint when debugging using gDEBugger */
    glColor3i(0, 0, 0);
}

void CGL21Device::DebugLights()
{
    Gfx::ColorHSV color(0.0, 1.0, 1.0);

    glLineWidth(3.0f);
    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);
    glDisable(GL_BLEND);

    Math::Matrix saveWorldMat = m_worldMat;
    m_worldMat.LoadIdentity();
    SetTransform(TRANSFORM_WORLD, m_worldMat);

    for (int i = 0; i < static_cast<int>( m_lights.size() ); ++i)
    {
        color.h = static_cast<float>(i) / static_cast<float>(m_lights.size());
        if (m_lightsEnabled[i])
        {
            const Light& l = m_lights[i];
            if (l.type == LIGHT_DIRECTIONAL)
            {
                Gfx::VertexCol v[2];
                v[0].coord = -Math::Normalize(l.direction) * 100.0f + Math::Vector(0.0f, 0.0f, 1.0f) * i;
                v[0].color = HSV2RGB(color);
                v[1].coord =  Math::Normalize(l.direction) * 100.0f + Math::Vector(0.0f, 0.0f, 1.0f) * i;
                v[1].color = HSV2RGB(color);
                while (v[0].coord.y < 60.0f && v[0].coord.y < 60.0f)
                {
                    v[0].coord.y += 10.0f;
                    v[1].coord.y += 10.0f;
                }
                DrawPrimitive(PRIMITIVE_LINES, v, 2);

                v[0].coord = v[1].coord + Math::Normalize(v[0].coord - v[1].coord) * 50.0f;

                glLineWidth(10.0f);
                DrawPrimitive(PRIMITIVE_LINES, v, 2);
                glLineWidth(3.0f);
            }
            else  if (l.type == LIGHT_POINT)
            {
                Gfx::VertexCol v[8];
                for (int i = 0; i < 8; ++i)
                    v[i].color = HSV2RGB(color);

                v[0].coord = l.position + Math::Vector(-1.0f, -1.0f, -1.0f) * 4.0f;
                v[1].coord = l.position + Math::Vector( 1.0f, -1.0f, -1.0f) * 4.0f;
                v[2].coord = l.position + Math::Vector( 1.0f,  1.0f, -1.0f) * 4.0f;
                v[3].coord = l.position + Math::Vector(-1.0f,  1.0f, -1.0f) * 4.0f;
                v[4].coord = l.position + Math::Vector(-1.0f, -1.0f, -1.0f) * 4.0f;
                DrawPrimitive(PRIMITIVE_LINE_STRIP, v, 5);

                v[0].coord = l.position + Math::Vector(-1.0f, -1.0f,  1.0f) * 4.0f;
                v[1].coord = l.position + Math::Vector( 1.0f, -1.0f,  1.0f) * 4.0f;
                v[2].coord = l.position + Math::Vector( 1.0f,  1.0f,  1.0f) * 4.0f;
                v[3].coord = l.position + Math::Vector(-1.0f,  1.0f,  1.0f) * 4.0f;
                v[4].coord = l.position + Math::Vector(-1.0f, -1.0f,  1.0f) * 4.0f;
                DrawPrimitive(PRIMITIVE_LINE_STRIP, v, 5);

                v[0].coord = l.position + Math::Vector(-1.0f, -1.0f, -1.0f) * 4.0f;
                v[1].coord = l.position + Math::Vector(-1.0f, -1.0f,  1.0f) * 4.0f;
                v[2].coord = l.position + Math::Vector( 1.0f, -1.0f, -1.0f) * 4.0f;
                v[3].coord = l.position + Math::Vector( 1.0f, -1.0f,  1.0f) * 4.0f;
                v[4].coord = l.position + Math::Vector( 1.0f,  1.0f, -1.0f) * 4.0f;
                v[5].coord = l.position + Math::Vector( 1.0f,  1.0f,  1.0f) * 4.0f;
                v[6].coord = l.position + Math::Vector(-1.0f,  1.0f, -1.0f) * 4.0f;
                v[7].coord = l.position + Math::Vector(-1.0f,  1.0f,  1.0f) * 4.0f;
                DrawPrimitive(PRIMITIVE_LINES, v, 8);
            }
            else if (l.type == LIGHT_SPOT)
            {
                Gfx::VertexCol v[5];
                for (int i = 0; i < 5; ++i)
                    v[i].color = HSV2RGB(color);

                v[0].coord = l.position + Math::Vector(-1.0f,  0.0f, -1.0f) * 4.0f;
                v[1].coord = l.position + Math::Vector( 1.0f,  0.0f, -1.0f) * 4.0f;
                v[2].coord = l.position + Math::Vector( 1.0f,  0.0f,  1.0f) * 4.0f;
                v[3].coord = l.position + Math::Vector(-1.0f,  0.0f,  1.0f) * 4.0f;
                v[4].coord = l.position + Math::Vector(-1.0f,  0.0f, -1.0f) * 4.0f;
                DrawPrimitive(PRIMITIVE_LINE_STRIP, v, 5);

                v[0].coord = l.position;
                v[1].coord = l.position + Math::Normalize(l.direction) * 100.0f;
                glEnable(GL_LINE_STIPPLE);
                glLineStipple(3.0, 0xFF);
                DrawPrimitive(PRIMITIVE_LINES, v, 2);
                glDisable(GL_LINE_STIPPLE);
            }
        }
    }

    glLineWidth(1.0f);
    glEnable(GL_LIGHTING);
    glDepthMask(GL_TRUE);
    glEnable(GL_BLEND);

    SetTransform(TRANSFORM_WORLD, saveWorldMat);
    m_worldMat = saveWorldMat;
}

std::string CGL21Device::GetName()
{
    return std::string("OpenGL 2.1");
}

bool CGL21Device::Create()
{
    GetLogger()->Info("Creating CDevice - OpenGL 2.1\n");

    if (!InitializeGLEW())
    {
        m_errorMessage = "An error occurred while initializing GLEW.";
        return false;
    }

    // Extract OpenGL version
    int glMajor, glMinor;
    int glVersion = GetOpenGLVersion(glMajor, glMinor);

    if (glVersion < 20)
    {
        GetLogger()->Error("Unsupported OpenGL version: %d.%d\n", glMajor, glMinor);
        GetLogger()->Error("OpenGL 2.0 or newer is required to use this engine.\n");
        m_errorMessage = "It seems your graphics card does not support OpenGL 2.0.\n";
        m_errorMessage += "Please make sure you have appropriate hardware and newest drivers installed.\n";
        m_errorMessage += "(OpenGL 2.0 is roughly equivalent to Direct3D 9)\n\n";
        m_errorMessage += GetHardwareInfo();
        return false;
    }

    const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));

    GetLogger()->Info("OpenGL %s\n", version);
    GetLogger()->Info("%s\n", renderer);

    // Detect support of anisotropic filtering
    m_capabilities.anisotropySupported = glewIsSupported("GL_EXT_texture_filter_anisotropic");
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

    // Read maximum sample count for MSAA
    if(glewIsSupported("GL_EXT_framebuffer_multisample"))
    {
        m_capabilities.multisamplingSupported = true;

        glGetIntegerv(GL_MAX_SAMPLES_EXT, &m_capabilities.maxSamples);
        GetLogger()->Info("Multisampling supported, max samples: %d\n", m_capabilities.maxSamples);
    }
    else
    {
        GetLogger()->Info("Multisampling not supported\n");
    }

    m_capabilities.shadowMappingSupported = true;

    // This is mostly done in all modern hardware by default
    // DirectX doesn't even allow the option to turn off perspective correction anymore
    // So turn it on permanently
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    // Set just to be sure
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glViewport(0, 0, m_config.size.x, m_config.size.y);

    // this is set in shader
    int numLights = 4;

    m_lights        = std::vector<Light>(numLights, Light());
    m_lightsEnabled = std::vector<bool> (numLights, false);

    m_capabilities.maxLights = numLights;

    int maxTextures = 0;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextures);
    GetLogger()->Info("Maximum texture image units: %d\n", maxTextures);

    m_capabilities.multitexturingSupported = true;
    m_capabilities.maxTextures = maxTextures;

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_capabilities.maxTextureSize);
    GetLogger()->Info("Maximum texture size: %d\n", m_capabilities.maxTextureSize);

    m_framebufferSupport = DetectFramebufferSupport();
    if (m_framebufferSupport == FBS_ARB)
    {
        m_capabilities.framebufferSupported = true;
        GetLogger()->Info("Framebuffer supported (ARB)\n");

        glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &m_capabilities.maxRenderbufferSize);
        GetLogger()->Info("Maximum renderbuffer size: %d\n", m_capabilities.maxRenderbufferSize);
    }
    else if (m_framebufferSupport == FBS_EXT)
    {
        m_capabilities.framebufferSupported = true;
        GetLogger()->Info("Framebuffer supported (EXT)\n");

        glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE_EXT, &m_capabilities.maxRenderbufferSize);
        GetLogger()->Info("Maximum renderbuffer size: %d\n", m_capabilities.maxRenderbufferSize);
    }
    else
    {
        m_capabilities.framebufferSupported = false;
        GetLogger()->Info("Framebuffer not supported\n");
    }

    m_currentTextures    = std::vector<Texture>           (maxTextures, Texture());
    m_texturesEnabled    = std::vector<bool>              (maxTextures, false);
    m_textureStageParams = std::vector<TextureStageParams>(maxTextures, TextureStageParams());

    // Create shader program for normal rendering
    GLint shaders[2];
    char filename[128];

    strcpy(filename, "shaders/gl21/vs_normal.glsl");
    shaders[0] = LoadShader(GL_VERTEX_SHADER, filename);
    if (shaders[0] == 0)
    {
        m_errorMessage = GetLastShaderError();
        GetLogger()->Error("Cound not create vertex shader from file '%s'\n", filename);
        return false;
    }

    strcpy(filename, "shaders/gl21/fs_normal.glsl");
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

    // Create shader program for interface rendering
    strcpy(filename, "shaders/gl21/vs_interface.glsl");
    shaders[0] = LoadShader(GL_VERTEX_SHADER, filename);
    if (shaders[0] == 0)
    {
        m_errorMessage = GetLastShaderError();
        GetLogger()->Error("Cound not create vertex shader from file '%s'\n", filename);
        return false;
    }

    strcpy(filename, "shaders/gl21/fs_interface.glsl");
    shaders[1] = LoadShader(GL_FRAGMENT_SHADER, filename);
    if (shaders[1] == 0)
    {
        m_errorMessage = GetLastShaderError();
        GetLogger()->Error("Cound not compile fragment shader from file '%s'\n", filename);
        return false;
    }

    m_interfaceProgram = LinkProgram(2, shaders);
    if (m_interfaceProgram == 0)
    {
        m_errorMessage = GetLastShaderError();
        GetLogger()->Error("Cound not link shader program for interface rendering\n");
        return false;
    }

    glDeleteShader(shaders[0]);
    glDeleteShader(shaders[1]);

    // Create shader program for shadow rendering
    strcpy(filename, "shaders/gl21/vs_shadow.glsl");
    shaders[0] = LoadShader(GL_VERTEX_SHADER, filename);
    if (shaders[0] == 0)
    {
        m_errorMessage = GetLastShaderError();
        GetLogger()->Error("Cound not create vertex shader from file '%s'\n", filename);
        return false;
    }

    strcpy(filename, "shaders/gl21/fs_shadow.glsl");
    shaders[1] = LoadShader(GL_FRAGMENT_SHADER, filename);
    if (shaders[1] == 0)
    {
        m_errorMessage = GetLastShaderError();
        GetLogger()->Error("Cound not compile fragment shader from file '%s'\n", filename);
        return false;
    }

    m_shadowProgram = LinkProgram(2, shaders);
    if (m_shadowProgram == 0)
    {
        m_errorMessage = GetLastShaderError();
        GetLogger()->Error("Cound not link shader program for shadow rendering\n");
        return false;
    }

    glDeleteShader(shaders[0]);
    glDeleteShader(shaders[1]);

    // Obtain uniform locations from normal rendering program and initialize them
    {
        UniformLocations &uni = m_uniforms[0];

        uni.projectionMatrix = glGetUniformLocation(m_normalProgram, "uni_ProjectionMatrix");
        uni.viewMatrix = glGetUniformLocation(m_normalProgram, "uni_ViewMatrix");
        uni.modelMatrix = glGetUniformLocation(m_normalProgram, "uni_ModelMatrix");
        uni.normalMatrix = glGetUniformLocation(m_normalProgram, "uni_NormalMatrix");
        uni.shadowMatrix = glGetUniformLocation(m_normalProgram, "uni_ShadowMatrix");

        uni.primaryTexture = glGetUniformLocation(m_normalProgram, "uni_PrimaryTexture");
        uni.secondaryTexture = glGetUniformLocation(m_normalProgram, "uni_SecondaryTexture");
        uni.shadowTexture = glGetUniformLocation(m_normalProgram, "uni_ShadowTexture");

        for (int i = 0; i < 3; i++)
        {
            char name[64];
            sprintf(name, "uni_TextureEnabled[%d]", i);
            uni.textureEnabled[i] = glGetUniformLocation(m_normalProgram, name);
        }

        uni.alphaTestEnabled = glGetUniformLocation(m_normalProgram, "uni_AlphaTestEnabled");
        uni.alphaReference = glGetUniformLocation(m_normalProgram, "uni_AlphaReference");

        uni.fogEnabled = glGetUniformLocation(m_normalProgram, "uni_FogEnabled");
        uni.fogRange = glGetUniformLocation(m_normalProgram, "uni_FogRange");
        uni.fogColor = glGetUniformLocation(m_normalProgram, "uni_FogColor");

        uni.shadowColor = glGetUniformLocation(m_normalProgram, "uni_ShadowColor");
        uni.lightCount = glGetUniformLocation(m_normalProgram, "uni_LightCount");

        uni.ambientColor = glGetUniformLocation(m_normalProgram, "uni_Material.ambient");
        uni.diffuseColor = glGetUniformLocation(m_normalProgram, "uni_Material.diffuse");
        uni.specularColor = glGetUniformLocation(m_normalProgram, "uni_Material.specular");

        GLchar name[64];
        for (int i = 0; i < 8; i++)
        {
            sprintf(name, "uni_Light[%d].Position", i);
            uni.lights[i].position = glGetUniformLocation(m_normalProgram, name);

            sprintf(name, "uni_Light[%d].Ambient", i);
            uni.lights[i].ambient = glGetUniformLocation(m_normalProgram, name);

            sprintf(name, "uni_Light[%d].Diffuse", i);
            uni.lights[i].diffuse = glGetUniformLocation(m_normalProgram, name);

            sprintf(name, "uni_Light[%d].Specular", i);
            uni.lights[i].specular = glGetUniformLocation(m_normalProgram, name);
        }

        // Set default uniform values
        Math::Matrix matrix;
        matrix.LoadIdentity();

        glUseProgram(m_normalProgram);

        glUniformMatrix4fv(uni.projectionMatrix, 1, GL_FALSE, matrix.Array());
        glUniformMatrix4fv(uni.viewMatrix, 1, GL_FALSE, matrix.Array());
        glUniformMatrix4fv(uni.modelMatrix, 1, GL_FALSE, matrix.Array());
        glUniformMatrix4fv(uni.normalMatrix, 1, GL_FALSE, matrix.Array());
        glUniformMatrix4fv(uni.shadowMatrix, 1, GL_FALSE, matrix.Array());

        glUniform1i(uni.primaryTexture, 0);
        glUniform1i(uni.secondaryTexture, 1);
        glUniform1i(uni.shadowTexture, 2);

        for (int i = 0; i < 3; i++)
            glUniform1i(uni.textureEnabled[i], 0);

        glUniform1i(uni.alphaTestEnabled, 0);
        glUniform1f(uni.alphaReference, 0.5f);

        glUniform1i(uni.fogEnabled, 0);
        glUniform2f(uni.fogRange, 100.0f, 200.0f);
        glUniform4f(uni.fogColor, 0.8f, 0.8f, 0.8f, 1.0f);

        glUniform1f(uni.shadowColor, 0.5f);

        glUniform1i(uni.lightCount, 0);
    }

    // Obtain uniform locations from interface rendering program and initialize them
    {
        UniformLocations &uni = m_uniforms[1];

        uni.projectionMatrix = glGetUniformLocation(m_interfaceProgram, "uni_ProjectionMatrix");
        uni.viewMatrix = glGetUniformLocation(m_interfaceProgram, "uni_ViewMatrix");
        uni.modelMatrix = glGetUniformLocation(m_interfaceProgram, "uni_ModelMatrix");

        uni.primaryTexture = glGetUniformLocation(m_interfaceProgram, "uni_PrimaryTexture");

        uni.textureEnabled[0] = glGetUniformLocation(m_interfaceProgram, "uni_TextureEnabled");
        uni.textureEnabled[1] = -1;
        uni.textureEnabled[2] = -1;

        // Set default uniform values
        Math::Matrix matrix;
        matrix.LoadIdentity();

        glUseProgram(m_interfaceProgram);

        glUniformMatrix4fv(uni.projectionMatrix, 1, GL_FALSE, matrix.Array());
        glUniformMatrix4fv(uni.viewMatrix, 1, GL_FALSE, matrix.Array());
        glUniformMatrix4fv(uni.modelMatrix, 1, GL_FALSE, matrix.Array());

        glUniform1i(uni.primaryTexture, 0);

        glUniform1i(uni.textureEnabled[0], 0);
    }

    // Obtain uniform locations from shadow rendering program and initialize them
    {
        UniformLocations &uni = m_uniforms[2];

        uni.projectionMatrix = glGetUniformLocation(m_shadowProgram, "uni_ProjectionMatrix");
        uni.viewMatrix = glGetUniformLocation(m_shadowProgram, "uni_ViewMatrix");
        uni.modelMatrix = glGetUniformLocation(m_shadowProgram, "uni_ModelMatrix");

        uni.primaryTexture = glGetUniformLocation(m_shadowProgram, "uni_PrimaryTexture");

        uni.textureEnabled[0] = glGetUniformLocation(m_shadowProgram, "uni_TextureEnabled");
        uni.textureEnabled[1] = -1;
        uni.textureEnabled[2] = -1;

        uni.alphaTestEnabled = glGetUniformLocation(m_shadowProgram, "uni_AlphaTestEnabled");
        uni.alphaReference = glGetUniformLocation(m_shadowProgram, "uni_AlphaReference");

        // Set default uniform values
        Math::Matrix matrix;
        matrix.LoadIdentity();

        glUseProgram(m_shadowProgram);

        glUniformMatrix4fv(uni.projectionMatrix, 1, GL_FALSE, matrix.Array());
        glUniformMatrix4fv(uni.viewMatrix, 1, GL_FALSE, matrix.Array());
        glUniformMatrix4fv(uni.modelMatrix, 1, GL_FALSE, matrix.Array());

        glUniform1i(uni.primaryTexture, 0);

        glUniform1i(uni.textureEnabled[0], 0);

        glUniform1i(uni.alphaTestEnabled, 0);
        glUniform1f(uni.alphaReference, 0.5f);
    }

    glUseProgram(m_normalProgram);
    glEnable(GL_VERTEX_PROGRAM_TWO_SIDE);

    // create default framebuffer object
    FramebufferParams framebufferParams;

    framebufferParams.width = m_config.size.x;
    framebufferParams.height = m_config.size.y;
    framebufferParams.depth = m_config.depthSize;

    m_framebuffers["default"] = MakeUnique<CDefaultFramebuffer>(framebufferParams);

    GetLogger()->Info("CDevice created successfully\n");

    return true;
}

void CGL21Device::Destroy()
{
    // Delete the remaining textures
    // Should not be strictly necessary, but just in case
    glUseProgram(0);
    glDeleteProgram(m_normalProgram);
    glDeleteProgram(m_interfaceProgram);
    glDeleteProgram(m_shadowProgram);

    // delete framebuffers
    for (auto& framebuffer : m_framebuffers)
        framebuffer.second->Destroy();

    m_framebuffers.clear();

    DestroyAllTextures();

    m_lights.clear();
    m_lightsEnabled.clear();

    m_currentTextures.clear();
    m_texturesEnabled.clear();
    m_textureStageParams.clear();
}

void CGL21Device::ConfigChanged(const DeviceConfig& newConfig)
{
    m_config = newConfig;

    // Reset state
    m_lighting = false;
    m_updateLights = true;

    glViewport(0, 0, m_config.size.x, m_config.size.y);

    // create default framebuffer object
    FramebufferParams framebufferParams;

    framebufferParams.width = m_config.size.x;
    framebufferParams.height = m_config.size.y;
    framebufferParams.depth = m_config.depthSize;

    m_framebuffers["default"] = MakeUnique<CDefaultFramebuffer>(framebufferParams);
}

void CGL21Device::BeginScene()
{
    Clear();
}

void CGL21Device::EndScene()
{
#ifdef DEV_BUILD
    CheckGLErrors();
#endif
}

void CGL21Device::Clear()
{
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void CGL21Device::SetRenderMode(RenderMode mode)
{
    switch (mode)
    {
    case RENDER_MODE_NORMAL:
        glUseProgram(m_normalProgram);
        m_mode = 0;
        break;
    case RENDER_MODE_INTERFACE:
        glUseProgram(m_interfaceProgram);
        m_mode = 1;
        break;
    case RENDER_MODE_SHADOW:
        glUseProgram(m_shadowProgram);
        m_mode = 2;
        break;
    default:
        assert(false);
    }

    for (int i = 0; i < 3; i++)
    {
        glUniform1i(m_uniforms[m_mode].textureEnabled[i], m_texturesEnabled[i]);
    }
}

void CGL21Device::SetTransform(TransformType type, const Math::Matrix &matrix)
{
    if (type == TRANSFORM_WORLD)
    {
        m_worldMat = matrix;

        glUniformMatrix4fv(m_uniforms[m_mode].modelMatrix, 1, GL_FALSE, m_worldMat.Array());

        m_modelviewMat = Math::MultiplyMatrices(m_viewMat, m_worldMat);
        m_combinedMatrix = Math::MultiplyMatrices(m_projectionMat, m_modelviewMat);

        // normal transform
        Math::Matrix normalMat = matrix;

        if (fabs(normalMat.Det()) > 1e-6)
            normalMat = normalMat.Inverse();

        glUniformMatrix4fv(m_uniforms[m_mode].normalMatrix, 1, GL_TRUE, normalMat.Array());
    }
    else if (type == TRANSFORM_VIEW)
    {
        Math::Matrix scale;
        scale.Set(3, 3, -1.0f);
        m_viewMat = Math::MultiplyMatrices(scale, matrix);

        m_modelviewMat = Math::MultiplyMatrices(m_viewMat, m_worldMat);
        m_combinedMatrix = Math::MultiplyMatrices(m_projectionMat, m_modelviewMat);

        glUniformMatrix4fv(m_uniforms[m_mode].viewMatrix, 1, GL_FALSE, m_viewMat.Array());
    }
    else if (type == TRANSFORM_PROJECTION)
    {
        m_projectionMat = matrix;

        m_combinedMatrix = Math::MultiplyMatrices(m_projectionMat, m_modelviewMat);

        glUniformMatrix4fv(m_uniforms[m_mode].projectionMatrix, 1, GL_FALSE, m_projectionMat.Array());
    }
    else if (type == TRANSFORM_SHADOW)
    {
        Math::Matrix temp = matrix;
        glUniformMatrix4fv(m_uniforms[m_mode].shadowMatrix, 1, GL_FALSE, temp.Array());
    }
    else
    {
        assert(false);
    }
}

void CGL21Device::SetMaterial(const Material &material)
{
    m_material = material;

    glUniform4fv(m_uniforms[m_mode].ambientColor, 1, m_material.ambient.Array());
    glUniform4fv(m_uniforms[m_mode].diffuseColor, 1, m_material.diffuse.Array());
    glUniform4fv(m_uniforms[m_mode].specularColor, 1, m_material.specular.Array());
}

int CGL21Device::GetMaxLightCount()
{
    return m_lights.size();
}

void CGL21Device::SetLight(int index, const Light &light)
{
    assert(index >= 0);
    assert(index < static_cast<int>( m_lights.size() ));

    m_lights[index] = light;

    m_updateLights = true;
}

void CGL21Device::SetLightEnabled(int index, bool enabled)
{
    assert(index >= 0);
    assert(index < static_cast<int>( m_lights.size() ));

    m_lightsEnabled[index] = enabled;

    m_updateLights = true;
}

/** If image is invalid, returns invalid texture.
    Otherwise, returns pointer to new Texture struct.
    This struct must not be deleted in other way than through DeleteTexture() */
Texture CGL21Device::CreateTexture(CImage *image, const TextureCreateParams &params)
{
    ImageData *data = image->GetData();
    if (data == nullptr)
    {
        GetLogger()->Error("Invalid texture data\n");
        return Texture(); // invalid texture
    }

    Math::IntPoint originalSize = image->GetSize();

    if (params.padToNearestPowerOfTwo)
        image->PadToNearestPowerOfTwo();

    Texture tex = CreateTexture(data, params);
    tex.originalSize = originalSize;

    return tex;
}

Texture CGL21Device::CreateTexture(ImageData *data, const TextureCreateParams &params)
{
    Texture result;

    result.size.x = data->surface->w;
    result.size.y = data->surface->h;

    if (!Math::IsPowerOfTwo(result.size.x) || !Math::IsPowerOfTwo(result.size.y))
        GetLogger()->Warn("Creating non-power-of-2 texture (%dx%d)!\n", result.size.x, result.size.y);

    result.originalSize = result.size;

    glGenTextures(1, &result.id);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, result.id);

    glEnable(GL_TEXTURE_2D);

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
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    }
    else
    {
        // Has to be set to 0 because no mipmaps are generated
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
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

    SDL_FreeSurface(texData.convertedSurface);

    m_allTextures.insert(result);

    // Restore the previous state of 1st stage
    glBindTexture(GL_TEXTURE_2D, m_currentTextures[0].id);

    if (! m_texturesEnabled[0])
        glDisable(GL_TEXTURE_2D);

    return result;
}

Texture CGL21Device::CreateDepthTexture(int width, int height, int depth)
{
    Texture result;

    result.alpha = false;
    result.size.x = width;
    result.size.y = height;

    // Use & enable 1st texture stage
    glActiveTexture(GL_TEXTURE0);

    glGenTextures(1, &result.id);
    glBindTexture(GL_TEXTURE_2D, result.id);

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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    float color[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

    glBindTexture(GL_TEXTURE_2D, m_currentTextures[0].id);

    return result;
}

void CGL21Device::UpdateTexture(const Texture& texture, Math::IntPoint offset, ImageData* data, TexImgFormat format)
{
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, texture.id);

    PreparedTextureData texData = PrepareTextureData(data, format);

    glPixelStorei(GL_UNPACK_ROW_LENGTH, texData.actualSurface->pitch / texData.actualSurface->format->BytesPerPixel);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexSubImage2D(GL_TEXTURE_2D, 0, offset.x, offset.y, texData.actualSurface->w, texData.actualSurface->h,
                    texData.sourceFormat, GL_UNSIGNED_BYTE, texData.actualSurface->pixels);

    SDL_FreeSurface(texData.convertedSurface);
}

void CGL21Device::DestroyTexture(const Texture &texture)
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

void CGL21Device::DestroyAllTextures()
{
    // Unbind all texture stages
    for (int index = 0; index < static_cast<int>( m_currentTextures.size() ); ++index)
        SetTexture(index, Texture());

    for (auto it = m_allTextures.begin(); it != m_allTextures.end(); ++it)
        glDeleteTextures(1, &(*it).id);

    m_allTextures.clear();
}

int CGL21Device::GetMaxTextureStageCount()
{
    return m_currentTextures.size();
}

/**
  If \a texture is invalid, unbinds the given texture.
  If valid, binds the texture and enables the given texture stage.
  The setting is remembered, even if texturing is disabled at the moment. */
void CGL21Device::SetTexture(int index, const Texture &texture)
{
    assert(index >= 0 && index < static_cast<int>( m_currentTextures.size() ));

    bool same = m_currentTextures[index].id == texture.id;

    m_currentTextures[index] = texture; // remember the new value

    if (same)
        return; // nothing to do

    glActiveTexture(GL_TEXTURE0 + index);

    glBindTexture(GL_TEXTURE_2D, texture.id);

    // Params need to be updated for the new bound texture
    UpdateTextureState(index);
    UpdateTextureParams(index);
}

void CGL21Device::SetTexture(int index, unsigned int textureId)
{
    assert(index >= 0 && index < static_cast<int>( m_currentTextures.size() ));

    if (m_currentTextures[index].id == textureId)
        return; // nothing to do

    m_currentTextures[index].id = textureId;

    glActiveTexture(GL_TEXTURE0 + index);

    glBindTexture(GL_TEXTURE_2D, textureId);

    // Params need to be updated for the new bound texture
    UpdateTextureState(index);
    UpdateTextureParams(index);
}

void CGL21Device::SetTextureEnabled(int index, bool enabled)
{
    assert(index >= 0 && index < static_cast<int>( m_currentTextures.size() ));

    bool same = m_texturesEnabled[index] == enabled;

    m_texturesEnabled[index] = enabled;

    if (same)
        return; // nothing to do

    UpdateTextureState(index);
}

void CGL21Device::UpdateTextureState(int index)
{
    bool enabled = m_texturesEnabled[index] && (m_currentTextures[index].id != 0);
    glUniform1i(m_uniforms[m_mode].textureEnabled[index], enabled ? 1 : 0);
}

void CGL21Device::UpdateLights()
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
            LightLocations &uni = m_uniforms[m_mode].lights[index];

            glUniform4fv(uni.ambient, 1, light.ambient.Array());
            glUniform4fv(uni.diffuse, 1, light.diffuse.Array());
            glUniform4fv(uni.specular, 1, light.specular.Array());

            glUniform4f(uni.position, -light.direction.x, -light.direction.y, -light.direction.z, 0.0f);

            index++;
        }

        glUniform1i(m_uniforms[m_mode].lightCount, index);
    }
    // Lighting disabled
    else
    {
        glUniform1i(m_uniforms[m_mode].lightCount, 0);
    }
}

inline void CGL21Device::BindVBO(GLuint vbo)
{
    if (m_currentVBO == vbo) return;

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    m_currentVBO = vbo;
}

inline void CGL21Device::BindTexture(int index, GLuint texture)
{
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, texture);
}

/**
  Sets the texture parameters for the given texture stage.
  If the given texture was not set (bound) yet, nothing happens.
  The settings are remembered, even if texturing is disabled at the moment. */
void CGL21Device::SetTextureStageParams(int index, const TextureStageParams &params)
{
    assert(index >= 0 && index < static_cast<int>( m_currentTextures.size() ));

    // Remember the settings
    m_textureStageParams[index] = params;

    UpdateTextureParams(index);
}

void CGL21Device::UpdateTextureParams(int index)
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

void CGL21Device::SetTextureStageWrap(int index, TexWrapMode wrapS, TexWrapMode wrapT)
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

void CGL21Device::DrawPrimitive(PrimitiveType type, const Vertex *vertices, int vertexCount,
                              Color color)
{
    if (m_updateLights) UpdateLights();

    BindVBO(0);

    Vertex* vs = const_cast<Vertex*>(vertices);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex), reinterpret_cast<GLfloat*>(&vs[0].coord));

    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, sizeof(Vertex), reinterpret_cast<GLfloat*>(&vs[0].normal));

    glClientActiveTexture(GL_TEXTURE0);

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), reinterpret_cast<GLfloat*>(&vs[0].texCoord));

    glColor4fv(color.Array());

    glDrawArrays(TranslateGfxPrimitive(type), 0, vertexCount);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY); // GL_TEXTURE0
}

void CGL21Device::DrawPrimitive(PrimitiveType type, const VertexTex2 *vertices, int vertexCount,
                              Color color)
{
    if (m_updateLights) UpdateLights();

    BindVBO(0);

    VertexTex2* vs = const_cast<VertexTex2*>(vertices);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(VertexTex2), reinterpret_cast<GLfloat*>(&vs[0].coord));

    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, sizeof(VertexTex2), reinterpret_cast<GLfloat*>(&vs[0].normal));

    glClientActiveTexture(GL_TEXTURE0);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof(VertexTex2), reinterpret_cast<GLfloat*>(&vs[0].texCoord));

    glClientActiveTexture(GL_TEXTURE1);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof(VertexTex2), reinterpret_cast<GLfloat*>(&vs[0].texCoord2));

    glColor4fv(color.Array());

    glDrawArrays(TranslateGfxPrimitive(type), 0, vertexCount);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY); // GL_TEXTURE1

    glClientActiveTexture(GL_TEXTURE0);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void CGL21Device::DrawPrimitive(PrimitiveType type, const VertexCol *vertices, int vertexCount)
{
    if (m_updateLights) UpdateLights();

    BindVBO(0);

    VertexCol* vs = const_cast<VertexCol*>(vertices);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(VertexCol), reinterpret_cast<GLfloat*>(&vs[0].coord));

    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(4, GL_FLOAT, sizeof(VertexCol), reinterpret_cast<GLfloat*>(&vs[0].color));

    glDrawArrays(TranslateGfxPrimitive(type), 0, vertexCount);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

void CGL21Device::DrawPrimitive(PrimitiveType type, const void *vertices,
    int size, const VertexFormat &format, int vertexCount)
{
    if (m_updateLights) UpdateLights();

    BindVBO(0);

    const char *ptr = reinterpret_cast<const char*>(vertices);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(format.vertex.size,
        TransformType(format.vertex.type),
        format.vertex.stride,
        ptr + format.vertex.offset);

    if (format.color.enabled)
    {
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(format.color.size,
            TransformType(format.color.type),
            format.color.stride,
            ptr + format.color.offset);
    }
    else
        glColor4fv(format.color.values);

    if (format.normal.enabled)
    {
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(TransformType(format.normal.type),
            format.normal.stride,
            ptr + format.normal.offset);
    }
    else
        glNormal3fv(format.normal.values);

    glClientActiveTexture(GL_TEXTURE0);
    if (format.tex1.enabled)
    {
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(format.tex1.size,
            TransformType(format.tex1.type),
            format.tex1.stride,
            ptr + format.tex1.offset);
    }
    else
        glTexCoord2fv(format.tex1.values);

    glClientActiveTexture(GL_TEXTURE1);
    if (format.tex2.enabled)
    {
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(format.tex2.size,
            TransformType(format.tex2.type),
            format.tex2.stride,
            ptr + format.tex2.offset);
    }
    else
        glTexCoord2fv(format.tex2.values);

    glDrawArrays(TranslateGfxPrimitive(type), 0, vertexCount);

    glDisableClientState(GL_VERTEX_ARRAY);

    if (format.color.enabled) glDisableClientState(GL_COLOR_ARRAY);
    if (format.normal.enabled) glDisableClientState(GL_NORMAL_ARRAY);

    if (format.tex1.enabled)
    {
        glClientActiveTexture(GL_TEXTURE0);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }

    if (format.tex2.enabled)
    {
        glClientActiveTexture(GL_TEXTURE1);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
}

void CGL21Device::DrawPrimitives(PrimitiveType type, const void *vertices,
    int size, const VertexFormat &format, int first[], int count[], int drawCount)
{
    if (m_updateLights) UpdateLights();

    BindVBO(0);

    const char *ptr = reinterpret_cast<const char*>(vertices);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(format.vertex.size,
        TransformType(format.vertex.type),
        format.vertex.stride,
        ptr + format.vertex.offset);

    if (format.color.enabled)
    {
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(format.color.size,
            TransformType(format.color.type),
            format.color.stride,
            ptr + format.color.offset);
    }
    else
        glColor4fv(format.color.values);

    if (format.normal.enabled)
    {
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(TransformType(format.normal.type),
            format.normal.stride,
            ptr + format.normal.offset);
    }
    else
        glNormal3fv(format.normal.values);

    glClientActiveTexture(GL_TEXTURE0);
    if (format.tex1.enabled)
    {
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(format.tex1.size,
            TransformType(format.tex1.type),
            format.tex1.stride,
            ptr + format.tex1.offset);
    }
    else
        glTexCoord2fv(format.tex1.values);

    glClientActiveTexture(GL_TEXTURE1);
    if (format.tex2.enabled)
    {
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(format.tex2.size,
            TransformType(format.tex2.type),
            format.tex2.stride,
            ptr + format.tex2.offset);
    }
    else
        glTexCoord2fv(format.tex2.values);

    glMultiDrawArrays(TranslateGfxPrimitive(type), first, count, drawCount);

    glDisableClientState(GL_VERTEX_ARRAY);

    if (format.color.enabled) glDisableClientState(GL_COLOR_ARRAY);
    if (format.normal.enabled) glDisableClientState(GL_NORMAL_ARRAY);

    if (format.tex1.enabled)
    {
        glClientActiveTexture(GL_TEXTURE0);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }

    if (format.tex2.enabled)
    {
        glClientActiveTexture(GL_TEXTURE1);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
}

void CGL21Device::DrawPrimitives(PrimitiveType type, const Vertex *vertices,
    int first[], int count[], int drawCount, Color color)
{
    if (m_updateLights) UpdateLights();

    BindVBO(0);

    Vertex* vs = const_cast<Vertex*>(vertices);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex), reinterpret_cast<GLfloat*>(&vs[0].coord));

    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, sizeof(Vertex), reinterpret_cast<GLfloat*>(&vs[0].normal));

    glClientActiveTexture(GL_TEXTURE0);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), reinterpret_cast<GLfloat*>(&vs[0].texCoord));

    glColor4fv(color.Array());

    glMultiDrawArrays(TranslateGfxPrimitive(type), first, count, drawCount);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY); // GL_TEXTURE0
}

void CGL21Device::DrawPrimitives(PrimitiveType type, const VertexTex2 *vertices,
    int first[], int count[], int drawCount, Color color)
{
    if (m_updateLights) UpdateLights();

    BindVBO(0);

    VertexTex2* vs = const_cast<VertexTex2*>(vertices);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(VertexTex2), reinterpret_cast<GLfloat*>(&vs[0].coord));

    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, sizeof(VertexTex2), reinterpret_cast<GLfloat*>(&vs[0].normal));

    glClientActiveTexture(GL_TEXTURE0);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof(VertexTex2), reinterpret_cast<GLfloat*>(&vs[0].texCoord));

    glClientActiveTexture(GL_TEXTURE1);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof(VertexTex2), reinterpret_cast<GLfloat*>(&vs[0].texCoord2));

    glColor4fv(color.Array());

    glMultiDrawArrays(TranslateGfxPrimitive(type), first, count, drawCount);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY); // GL_TEXTURE1

    glClientActiveTexture(GL_TEXTURE0);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void CGL21Device::DrawPrimitives(PrimitiveType type, const VertexCol *vertices,
    int first[], int count[], int drawCount)
{
    if (m_updateLights) UpdateLights();

    BindVBO(0);

    VertexCol* vs = const_cast<VertexCol*>(vertices);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(VertexCol), reinterpret_cast<GLfloat*>(&vs[0].coord));

    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(4, GL_FLOAT, sizeof(VertexCol), reinterpret_cast<GLfloat*>(&vs[0].color));

    glMultiDrawArrays(TranslateGfxPrimitive(type), first, count, drawCount);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

unsigned int CGL21Device::CreateStaticBuffer(PrimitiveType primitiveType, const Vertex* vertices, int vertexCount)
{
    unsigned int id = ++m_lastVboId;

    VboObjectInfo info;
    info.primitiveType = primitiveType;
    info.vertexType = VERTEX_TYPE_NORMAL;
    info.vertexCount = vertexCount;
    info.bufferId = 0;
    info.size = vertexCount * sizeof(Vertex);

    glGenBuffers(1, &info.bufferId);
    BindVBO(info.bufferId);
    glBufferData(GL_ARRAY_BUFFER, info.size, vertices, GL_STATIC_DRAW);

    m_vboObjects[id] = info;

    return id;
}

unsigned int CGL21Device::CreateStaticBuffer(PrimitiveType primitiveType, const VertexTex2* vertices, int vertexCount)
{
    unsigned int id = ++m_lastVboId;

    VboObjectInfo info;
    info.primitiveType = primitiveType;
    info.vertexType = VERTEX_TYPE_TEX2;
    info.vertexCount = vertexCount;
    info.bufferId = 0;
    info.size = vertexCount * sizeof(VertexTex2);

    glGenBuffers(1, &info.bufferId);
    BindVBO(info.bufferId);
    glBufferData(GL_ARRAY_BUFFER, info.size, vertices, GL_STATIC_DRAW);

    m_vboObjects[id] = info;

    return id;
}

unsigned int CGL21Device::CreateStaticBuffer(PrimitiveType primitiveType, const VertexCol* vertices, int vertexCount)
{
    unsigned int id = ++m_lastVboId;

    VboObjectInfo info;
    info.primitiveType = primitiveType;
    info.vertexType = VERTEX_TYPE_COL;
    info.vertexCount = vertexCount;
    info.bufferId = 0;
    info.size = vertexCount * sizeof(VertexCol);

    glGenBuffers(1, &info.bufferId);
    BindVBO(info.bufferId);
    glBufferData(GL_ARRAY_BUFFER, info.size, vertices, GL_STATIC_DRAW);

    m_vboObjects[id] = info;

    return id;
}

void CGL21Device::UpdateStaticBuffer(unsigned int bufferId, PrimitiveType primitiveType, const Vertex* vertices, int vertexCount)
{
    auto it = m_vboObjects.find(bufferId);
    if (it == m_vboObjects.end())
        return;

    int newSize = vertexCount * sizeof(Vertex);

    VboObjectInfo& info = (*it).second;
    info.primitiveType = primitiveType;
    info.vertexType = VERTEX_TYPE_NORMAL;
    info.vertexCount = vertexCount;

    BindVBO(info.bufferId);

    if (info.size < newSize)
    {
        glBufferData(GL_ARRAY_BUFFER, newSize, vertices, GL_STATIC_DRAW);
        info.size = newSize;
    }
    else
    {
        glBufferSubData(GL_ARRAY_BUFFER, 0, newSize, vertices);
    }
}

void CGL21Device::UpdateStaticBuffer(unsigned int bufferId, PrimitiveType primitiveType, const VertexTex2* vertices, int vertexCount)
{
    auto it = m_vboObjects.find(bufferId);
    if (it == m_vboObjects.end())
        return;

    VboObjectInfo& info = (*it).second;
    info.primitiveType = primitiveType;
    info.vertexType = VERTEX_TYPE_TEX2;
    info.vertexCount = vertexCount;

    int newSize = vertexCount * sizeof(VertexTex2);

    BindVBO(info.bufferId);

    if (info.size < newSize)
    {
        glBufferData(GL_ARRAY_BUFFER, newSize, vertices, GL_STATIC_DRAW);
        info.size = newSize;
    }
    else
    {
        glBufferSubData(GL_ARRAY_BUFFER, 0, newSize, vertices);
    }
}

void CGL21Device::UpdateStaticBuffer(unsigned int bufferId, PrimitiveType primitiveType, const VertexCol* vertices, int vertexCount)
{
    auto it = m_vboObjects.find(bufferId);
    if (it == m_vboObjects.end())
        return;

    VboObjectInfo& info = (*it).second;
    info.primitiveType = primitiveType;
    info.vertexType = VERTEX_TYPE_COL;
    info.vertexCount = vertexCount;

    int newSize = vertexCount * sizeof(VertexCol);

    BindVBO(info.bufferId);

    if (info.size < newSize)
    {
        glBufferData(GL_ARRAY_BUFFER, newSize, vertices, GL_STATIC_DRAW);
        info.size = newSize;
    }
    else
    {
        glBufferSubData(GL_ARRAY_BUFFER, 0, newSize, vertices);
    }
}

void CGL21Device::DrawStaticBuffer(unsigned int bufferId)
{
    auto it = m_vboObjects.find(bufferId);
    if (it == m_vboObjects.end())
        return;

    if (m_updateLights) UpdateLights();

    BindVBO((*it).second.bufferId);

    if ((*it).second.vertexType == VERTEX_TYPE_NORMAL)
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, sizeof(Vertex), static_cast<char*>(nullptr) + offsetof(Vertex, coord));

        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GL_FLOAT, sizeof(Vertex), static_cast<char*>(nullptr) + offsetof(Vertex, normal));

        glClientActiveTexture(GL_TEXTURE0);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), static_cast<char*>(nullptr) + offsetof(Vertex, texCoord));
    }
    else if ((*it).second.vertexType == VERTEX_TYPE_TEX2)
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, sizeof(VertexTex2), static_cast<char*>(nullptr) + offsetof(VertexTex2, coord));

        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GL_FLOAT, sizeof(VertexTex2), static_cast<char*>(nullptr) + offsetof(VertexTex2, normal));

        glClientActiveTexture(GL_TEXTURE0);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, sizeof(VertexTex2), static_cast<char*>(nullptr) + offsetof(VertexTex2, texCoord));

        glClientActiveTexture(GL_TEXTURE1);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, sizeof(VertexTex2), static_cast<char*>(nullptr) + offsetof(VertexTex2, texCoord2));
    }
    else if ((*it).second.vertexType == VERTEX_TYPE_COL)
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, sizeof(VertexCol), static_cast<char*>(nullptr) + offsetof(VertexCol, coord));

        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(4, GL_FLOAT, sizeof(VertexCol), static_cast<char*>(nullptr) + offsetof(VertexCol, color));
    }

    GLenum mode = TranslateGfxPrimitive((*it).second.primitiveType);
    glDrawArrays(mode, 0, (*it).second.vertexCount);

    if ((*it).second.vertexType == VERTEX_TYPE_NORMAL)
    {
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY); // GL_TEXTURE0
    }
    else if ((*it).second.vertexType == VERTEX_TYPE_TEX2)
    {
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY); // GL_TEXTURE1

        glClientActiveTexture(GL_TEXTURE0);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    else if ((*it).second.vertexType == VERTEX_TYPE_COL)
    {
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
    }
}

void CGL21Device::DestroyStaticBuffer(unsigned int bufferId)
{
    auto it = m_vboObjects.find(bufferId);
    if (it == m_vboObjects.end())
        return;

    if (m_currentVBO == (*it).second.bufferId)
        BindVBO(0);

    glDeleteBuffers(1, &(*it).second.bufferId);

    m_vboObjects.erase(it);
}

/* Based on libwine's implementation */

int CGL21Device::ComputeSphereVisibility(const Math::Vector &center, float radius)
{
    Math::Matrix &m = m_combinedMatrix;

    Math::Vector vec[6];
    float originPlane[6];

    // Left plane
    vec[0].x = m.Get(4, 1) + m.Get(1, 1);
    vec[0].y = m.Get(4, 2) + m.Get(1, 2);
    vec[0].z = m.Get(4, 3) + m.Get(1, 3);
    float l1 = vec[0].Length();
    vec[0].Normalize();
    originPlane[0] = (m.Get(4, 4) + m.Get(1, 4)) / l1;

    // Right plane
    vec[1].x = m.Get(4, 1) - m.Get(1, 1);
    vec[1].y = m.Get(4, 2) - m.Get(1, 2);
    vec[1].z = m.Get(4, 3) - m.Get(1, 3);
    float l2 = vec[1].Length();
    vec[1].Normalize();
    originPlane[1] = (m.Get(4, 4) - m.Get(1, 4)) / l2;

    // Bottom plane
    vec[2].x = m.Get(4, 1) + m.Get(2, 1);
    vec[2].y = m.Get(4, 2) + m.Get(2, 2);
    vec[2].z = m.Get(4, 3) + m.Get(2, 3);
    float l3 = vec[2].Length();
    vec[2].Normalize();
    originPlane[2] = (m.Get(4, 4) + m.Get(2, 4)) / l3;

    // Top plane
    vec[3].x = m.Get(4, 1) - m.Get(2, 1);
    vec[3].y = m.Get(4, 2) - m.Get(2, 2);
    vec[3].z = m.Get(4, 3) - m.Get(2, 3);
    float l4 = vec[3].Length();
    vec[3].Normalize();
    originPlane[3] = (m.Get(4, 4) - m.Get(2, 4)) / l4;

    // Front plane
    vec[4].x = m.Get(4, 1) + m.Get(3, 1);
    vec[4].y = m.Get(4, 2) + m.Get(3, 2);
    vec[4].z = m.Get(4, 3) + m.Get(3, 3);
    float l5 = vec[4].Length();
    vec[4].Normalize();
    originPlane[4] = (m.Get(4, 4) + m.Get(3, 4)) / l5;

    // Back plane
    vec[5].x = m.Get(4, 1) - m.Get(3, 1);
    vec[5].y = m.Get(4, 2) - m.Get(3, 2);
    vec[5].z = m.Get(4, 3) - m.Get(3, 3);
    float l6 = vec[5].Length();
    vec[5].Normalize();
    originPlane[5] = (m.Get(4, 4) - m.Get(3, 4)) / l6;

    int result = 0;

    if (InPlane(vec[0], originPlane[0], center, radius))
        result |= FRUSTUM_PLANE_LEFT;
    if (InPlane(vec[1], originPlane[1], center, radius))
        result |= FRUSTUM_PLANE_RIGHT;
    if (InPlane(vec[2], originPlane[2], center, radius))
        result |= FRUSTUM_PLANE_BOTTOM;
    if (InPlane(vec[3], originPlane[3], center, radius))
        result |= FRUSTUM_PLANE_TOP;
    if (InPlane(vec[4], originPlane[4], center, radius))
        result |= FRUSTUM_PLANE_FRONT;
    if (InPlane(vec[5], originPlane[5], center, radius))
        result |= FRUSTUM_PLANE_BACK;

    return result;
}

void CGL21Device::SetViewport(int x, int y, int width, int height)
{
    glViewport(x, y, width, height);
}

void CGL21Device::SetRenderState(RenderState state, bool enabled)
{
    if (state == RENDER_STATE_DEPTH_WRITE)
    {
        glDepthMask(enabled ? GL_TRUE : GL_FALSE);
        return;
    }
    else if (state == RENDER_STATE_LIGHTING)
    {
        if (m_lighting == enabled) return;

        m_lighting = enabled;

        m_updateLights = true;

        return;
    }
    else if (state == RENDER_STATE_ALPHA_TEST)
    {
        glUniform1i(m_uniforms[m_mode].alphaTestEnabled, enabled ? 1 : 0);

        return;
    }
    else if (state == RENDER_STATE_FOG)
    {
        glUniform1i(m_uniforms[m_mode].fogEnabled, enabled ? 1 : 0);

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
        case RENDER_STATE_DEPTH_BIAS:  flag = GL_POLYGON_OFFSET_FILL; break;
        default: assert(false); break;
    }

    if (enabled)
        glEnable(flag);
    else
        glDisable(flag);
}

void CGL21Device::SetColorMask(bool red, bool green, bool blue, bool alpha)
{
    glColorMask(red, green, blue, alpha);
}

void CGL21Device::SetDepthTestFunc(CompFunc func)
{
    glDepthFunc(TranslateGfxCompFunc(func));
}

void CGL21Device::SetDepthBias(float factor, float units)
{
    glPolygonOffset(factor, units);
}

void CGL21Device::SetAlphaTestFunc(CompFunc func, float refValue)
{
    glUniform1f(m_uniforms[m_mode].alphaReference, refValue);
}

void CGL21Device::SetBlendFunc(BlendFunc srcBlend, BlendFunc dstBlend)
{
    glBlendFunc(TranslateGfxBlendFunc(srcBlend), TranslateGfxBlendFunc(dstBlend));
}

void CGL21Device::SetClearColor(const Color &color)
{
    glClearColor(color.r, color.g, color.b, color.a);
}

void CGL21Device::SetGlobalAmbient(const Color &color)
{
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, color.Array());
}

void CGL21Device::SetFogParams(FogMode mode, const Color &color, float start, float end, float density)
{
    glUniform2f(m_uniforms[m_mode].fogRange, start, end);
    glUniform4f(m_uniforms[m_mode].fogColor, color.r, color.g, color.b, color.a);

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

void CGL21Device::SetCullMode(CullMode mode)
{
    // Cull clockwise back faces, so front face is the opposite
    // (assuming GL_CULL_FACE is GL_BACK)
    if      (mode == CULL_CW ) glFrontFace(GL_CCW);
    else if (mode == CULL_CCW) glFrontFace(GL_CW);
    else assert(false);
}

void CGL21Device::SetShadeModel(ShadeModel model)
{
    if      (model == SHADE_FLAT)   glShadeModel(GL_FLAT);
    else if (model == SHADE_SMOOTH) glShadeModel(GL_SMOOTH);
    else  assert(false);
}

void CGL21Device::SetShadowColor(float value)
{
    glUniform1f(m_uniforms[m_mode].shadowColor, value);
}

void CGL21Device::SetFillMode(FillMode mode)
{
    if      (mode == FILL_POINT) glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    else if (mode == FILL_LINES) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else if (mode == FILL_POLY)  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else assert(false);
}

void CGL21Device::CopyFramebufferToTexture(Texture& texture, int xOffset, int yOffset, int x, int y, int width, int height)
{
    if (texture.id == 0) return;

    // Use & enable 1st texture stage
    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, texture.id);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, xOffset, yOffset, x, y, width, height);

    // Restore previous texture
    glBindTexture(GL_TEXTURE_2D, m_currentTextures[0].id);
}

std::unique_ptr<CFrameBufferPixels> CGL21Device::GetFrameBufferPixels() const
{
    return GetGLFrameBufferPixels(m_config.size);
}

CFramebuffer* CGL21Device::GetFramebuffer(std::string name)
{
    auto it = m_framebuffers.find(name);
    if (it == m_framebuffers.end())
        return nullptr;

    return it->second.get();
}

CFramebuffer* CGL21Device::CreateFramebuffer(std::string name, const FramebufferParams& params)
{
    // existing framebuffer was found
    if (m_framebuffers.find(name) != m_framebuffers.end())
    {
        return nullptr;
    }

    std::unique_ptr<CFramebuffer> framebuffer;

    if (m_framebufferSupport == FBS_ARB)
        framebuffer = MakeUnique<CGLFramebuffer>(params);
    else if (m_framebufferSupport == FBS_EXT)
        framebuffer = MakeUnique<CGLFramebufferEXT>(params);
    else
        return nullptr;

    if (!framebuffer->Create()) return nullptr;

    CFramebuffer* framebufferPtr = framebuffer.get();
    m_framebuffers[name] = std::move(framebuffer);
    return framebufferPtr;
}

void CGL21Device::DeleteFramebuffer(std::string name)
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

bool CGL21Device::IsAnisotropySupported()
{
    return m_capabilities.anisotropySupported;
}

int CGL21Device::GetMaxAnisotropyLevel()
{
    return m_capabilities.maxAnisotropy;
}

int CGL21Device::GetMaxSamples()
{
    return m_capabilities.maxSamples;
}

bool CGL21Device::IsShadowMappingSupported()
{
    return m_capabilities.shadowMappingSupported;
}

int CGL21Device::GetMaxTextureSize()
{
    return m_capabilities.maxTextureSize;
}

bool CGL21Device::IsFramebufferSupported()
{
    return m_capabilities.framebufferSupported;
}

} // namespace Gfx
