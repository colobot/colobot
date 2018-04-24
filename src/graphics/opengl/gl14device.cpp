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


#include "graphics/opengl/gl14device.h"

#include "common/config.h"

#include "common/image.h"
#include "common/logger.h"

#include "graphics/core/light.h"

#include "graphics/engine/engine.h"

#include "graphics/opengl/glframebuffer.h"

#include "math/geometry.h"


#include <SDL.h>

#include <cassert>


// Graphics module namespace
namespace Gfx
{

CGL14Device::CGL14Device(const DeviceConfig &config)
    : m_config(config)
{}

CGL14Device::~CGL14Device()
{
}

void CGL14Device::DebugHook()
{
    /* This function is only called here, so it can be used
     * as a breakpoint when debugging using gDEBugger */
    glColor3i(0, 0, 0);
}

void CGL14Device::DebugLights()
{
    Gfx::ColorHSV color(0.0, 1.0, 1.0);

    glLineWidth(3.0f);
    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);
    glDisable(GL_BLEND);

    Math::Matrix saveWorldMat = m_worldMat;
    m_worldMat.LoadIdentity();
    UpdateModelviewMatrix();

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
    m_worldMat = saveWorldMat;
    UpdateModelviewMatrix();
}

std::string CGL14Device::GetName()
{
    return std::string("OpenGL 1.4");
}

bool CGL14Device::Create()
{
    GetLogger()->Info("Creating CDevice - OpenGL 1.4\n");

    if (!InitializeGLEW())
    {
        m_errorMessage = "An error occurred while initializing GLEW.";
        return false;
    }

    // Extract OpenGL version
    int glMajor = 1, glMinor = 1;
    int glVersion = GetOpenGLVersion(glMajor, glMinor);

    if (glVersion < 13)
    {
        GetLogger()->Error("Unsupported OpenGL version: %d.%d\n", glMajor, glMinor);
        GetLogger()->Error("OpenGL 1.3 or newer is required to use this engine.\n");
        m_errorMessage = "It seems your graphics card does not support OpenGL 1.3.\n";
        m_errorMessage += "Please make sure you have appropriate hardware and newest drivers installed.\n\n";
        m_errorMessage += GetHardwareInfo();
        return false;
    }

    const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));

    GetLogger()->Info("OpenGL %s\n", version);
    GetLogger()->Info("%s\n", renderer);

    // Detect Shadow mapping support
    if (glVersion >= 14)     // Core depth texture+shadow, OpenGL 1.4+
    {
        m_shadowMappingSupport = SMS_CORE;
        m_capabilities.shadowMappingSupported = true;
        GetLogger()->Info("Shadow mapping available (core)\n");
    }
    else if (glewIsSupported("GL_ARB_depth_texture GL_ARB_shadow"))  // ARB depth texture + shadow
    {
        m_shadowMappingSupport = SMS_ARB;
        m_capabilities.shadowMappingSupported = true;
        GetLogger()->Info("Shadow mapping available (ARB)\n");
    }
    else       // No Shadow mapping
    {
        m_shadowMappingSupport = SMS_NONE;
        m_capabilities.shadowMappingSupported = false;
        GetLogger()->Info("Shadow mapping not available\n");
    }

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
        m_capabilities.multisamplingSupported = false;
        GetLogger()->Info("Multisampling not supported\n");
    }

    // check for glMultiDrawArrays()
    if (glVersion >= 14)
        m_multiDrawArrays = true;

    GetLogger()->Info("Auto-detecting VBO support\n");

    // detecting VBO ARB extension
    bool vboARB = glewIsSupported("GL_ARB_vertex_buffer_object");

    // VBO is core OpenGL feature since 1.5
    // everything below 1.5 means no VBO support
    if (glVersion >= 15)
    {
        GetLogger()->Info("Core VBO supported\n", glMajor, glMinor);
        m_vertexBufferType = VBT_VBO_CORE;

        // Set function pointers
        m_glGenBuffers = glGenBuffers;
        m_glDeleteBuffers = glDeleteBuffers;
        m_glBindBuffer = glBindBuffer;
        m_glBufferData = glBufferData;
        m_glBufferSubData = glBufferSubData;
    }
    else if (vboARB)     // VBO ARB extension available
    {
        GetLogger()->Info("ARB VBO supported\n");
        m_vertexBufferType = VBT_VBO_ARB;

        // Set function pointers
        m_glGenBuffers = glGenBuffersARB;
        m_glDeleteBuffers = glDeleteBuffersARB;
        m_glBindBuffer = glBindBufferARB;
        m_glBufferData = glBufferDataARB;
        m_glBufferSubData = glBufferSubDataARB;
    }
    else                // no VBO support
    {
        GetLogger()->Info("VBO not supported\n");
        m_vertexBufferType = VBT_DISPLAY_LIST;
    }

    // This is mostly done in all modern hardware by default
    // DirectX doesn't even allow the option to turn off perspective correction anymore
    // So turn it on permanently
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    // To avoid problems with scaling & lighting
    glEnable(GL_RESCALE_NORMAL);
    //glEnable(GL_NORMALIZE);        // this needs some testing

    // Minimal depth bias to avoid Z-fighting
    //SetDepthBias(0.001f);
    glAlphaFunc(GL_GREATER, 0.1f);

    // Set just to be sure
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glViewport(0, 0, m_config.size.x, m_config.size.y);

    int numLights = 0;
    glGetIntegerv(GL_MAX_LIGHTS, &numLights);

    m_capabilities.maxLights = numLights;

    m_lights        = std::vector<Light>(numLights, Light());
    m_lightsEnabled = std::vector<bool> (numLights, false);

    int maxTextures = 0;
    glGetIntegerv(GL_MAX_TEXTURE_UNITS, &maxTextures);
    GetLogger()->Info("Maximum texture units: %d\n", maxTextures);

    m_capabilities.multitexturingSupported = true;
    m_capabilities.maxTextures = maxTextures;

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_capabilities.maxTextureSize);
    GetLogger()->Info("Maximum texture size: %d\n", m_capabilities.maxTextureSize);

    m_currentTextures    = std::vector<Texture>           (maxTextures, Texture());
    m_texturesEnabled    = std::vector<bool>              (maxTextures, false);
    m_textureStageParams = std::vector<TextureStageParams>(maxTextures, TextureStageParams());
    m_remap              = std::vector<int>               (maxTextures, 0);

    // default mapping
    for (int i = 0; i < maxTextures; i++)
        m_remap[i] = i;

    // special remapping for quality shadows
    if (maxTextures >= 4)
    {
        m_remap[0] = 2;
        m_remap[1] = 3;
        m_remap[2] = 0;
        m_remap[3] = 1;

        m_shadowQuality = true;
        GetLogger()->Debug("Using quality shadows\n");
    }
    else
    {
        m_shadowQuality = false;
        GetLogger()->Debug("Using simple shadows\n");
    }

    // create white texture
    glGenTextures(1, &m_whiteTexture);
    glBindTexture(GL_TEXTURE_2D, m_whiteTexture);
    int color = 0xFFFFFFFF;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &color);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    // create default framebuffer object
    FramebufferParams framebufferParams;

    framebufferParams.width = m_config.size.x;
    framebufferParams.height = m_config.size.y;
    framebufferParams.depth = m_config.depthSize;

    m_framebuffers["default"] = MakeUnique<CDefaultFramebuffer>(framebufferParams);

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

    GetLogger()->Info("CDevice created successfully\n");

    return true;
}

void CGL14Device::Destroy()
{
    // delete framebuffers
    for (auto& framebuffer : m_framebuffers)
        framebuffer.second->Destroy();

    m_framebuffers.clear();

    // Delete the remaining textures
    // Should not be strictly necessary, but just in case
    DestroyAllTextures();
    glDeleteTextures(1, &m_whiteTexture);
    m_whiteTexture = 0;

    m_lights.clear();
    m_lightsEnabled.clear();

    m_currentTextures.clear();
    m_texturesEnabled.clear();
    m_textureStageParams.clear();
}

void CGL14Device::ConfigChanged(const DeviceConfig& newConfig)
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

void CGL14Device::BeginScene()
{
    Clear();

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(m_projectionMat.Array());

    UpdateModelviewMatrix();
}

void CGL14Device::EndScene()
{
#ifdef DEV_BUILD
    int count = ClearGLErrors();

    if (count > 0)
        GetLogger()->Debug("OpenGL errors detected: %d\n", count);
#endif
}

void CGL14Device::Clear()
{
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void CGL14Device::SetRenderMode(RenderMode mode)
{
    // nothing is done
}

void CGL14Device::SetTransform(TransformType type, const Math::Matrix &matrix)
{
    if      (type == TRANSFORM_WORLD)
    {
        m_worldMat = matrix;
        UpdateModelviewMatrix();

        m_combinedMatrixOutdated = true;
    }
    else if (type == TRANSFORM_VIEW)
    {
        m_viewMat = matrix;
        UpdateModelviewMatrix();

        m_combinedMatrixOutdated = true;
    }
    else if (type == TRANSFORM_PROJECTION)
    {
        m_projectionMat = matrix;
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(m_projectionMat.Array());

        m_combinedMatrixOutdated = true;
    }
    else if (type == TRANSFORM_SHADOW)
    {
        m_shadowMatrix = matrix;

        glActiveTexture(GL_TEXTURE0 + m_remap[2]);
        glMatrixMode(GL_TEXTURE);
        glLoadMatrixf(m_shadowMatrix.Array());
    }
    else
    {
        assert(false);
    }
}

void CGL14Device::UpdateModelviewMatrix()
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glScalef(1.0f, 1.0f, -1.0f);
    glMultMatrixf(m_viewMat.Array());
    glMultMatrixf(m_worldMat.Array());
    glGetFloatv(GL_MODELVIEW_MATRIX, m_modelviewMat.Array());

    if (m_lighting)
    {
        UpdateLightPositions();
    }
}

void CGL14Device::SetMaterial(const Material &material)
{
    m_material = material;

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,  m_material.ambient.Array());
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_material.diffuse.Array());
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_material.specular.Array());
}

int CGL14Device::GetMaxLightCount()
{
    return m_lights.size();
}

void CGL14Device::SetLight(int index, const Light &light)
{
    assert(index >= 0);
    assert(index < static_cast<int>( m_lights.size() ));

    m_lights[index] = light;

    // Indexing from GL_LIGHT0 should always work
    glLightfv(GL_LIGHT0 + index, GL_AMBIENT,  const_cast<GLfloat*>(light.ambient.Array()));
    glLightfv(GL_LIGHT0 + index, GL_DIFFUSE,  const_cast<GLfloat*>(light.diffuse.Array()));
    glLightfv(GL_LIGHT0 + index, GL_SPECULAR, const_cast<GLfloat*>(light.specular.Array()));

    glLightf(GL_LIGHT0 + index, GL_CONSTANT_ATTENUATION,  light.attenuation0);
    glLightf(GL_LIGHT0 + index, GL_LINEAR_ATTENUATION,    light.attenuation1);
    glLightf(GL_LIGHT0 + index, GL_QUADRATIC_ATTENUATION, light.attenuation2);

    if (light.type == LIGHT_SPOT)
    {
        glLightf(GL_LIGHT0 + index, GL_SPOT_CUTOFF, light.spotAngle * Math::RAD_TO_DEG);
        glLightf(GL_LIGHT0 + index, GL_SPOT_EXPONENT, light.spotIntensity);
    }
    else
    {
        glLightf(GL_LIGHT0 + index, GL_SPOT_CUTOFF, 180.0f);
    }

    UpdateLightPosition(index);
}

void CGL14Device::UpdateLightPosition(int index)
{
    assert(index >= 0);
    assert(index < static_cast<int>( m_lights.size() ));

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    Light &light = m_lights[index];

    if (light.type == LIGHT_POINT)
    {
        glLoadIdentity();
        glScalef(1.0f, 1.0f, -1.0f);
        glMultMatrixf(m_viewMat.Array());

        GLfloat position[4] = { light.position.x, light.position.y, light.position.z, 1.0f };
        glLightfv(GL_LIGHT0 + index, GL_POSITION, position);
    }
    else
    {
        glLoadIdentity();
        glScalef(1.0f, 1.0f, -1.0f);
        Math::Matrix mat = m_viewMat;
        mat.Set(1, 4, 0.0f);
        mat.Set(2, 4, 0.0f);
        mat.Set(3, 4, 0.0f);
        glMultMatrixf(mat.Array());

        if (light.type == LIGHT_SPOT)
        {
            GLfloat direction[4] = { -light.direction.x, -light.direction.y, -light.direction.z, 1.0f };
            glLightfv(GL_LIGHT0 + index, GL_SPOT_DIRECTION, direction);
        }
        else if (light.type == LIGHT_DIRECTIONAL)
        {
            GLfloat position[4] = { -light.direction.x, -light.direction.y, -light.direction.z, 0.0f };
            glLightfv(GL_LIGHT0 + index, GL_POSITION, position);
        }
    }

    glPopMatrix();
}

void CGL14Device::UpdateLightPositions()
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    // update spotlights and directional lights
    glLoadIdentity();
    glScalef(1.0f, 1.0f, -1.0f);
    Math::Matrix mat = m_viewMat;
    mat.Set(1, 4, 0.0f);
    mat.Set(2, 4, 0.0f);
    mat.Set(3, 4, 0.0f);
    glMultMatrixf(mat.Array());

    int lightIndex = 0;

    for (const Light &light : m_lights)
    {
        if (m_lightsEnabled[lightIndex])
        {
            if (light.type == LIGHT_SPOT)
            {
                GLfloat direction[4] = { -light.direction.x, -light.direction.y, -light.direction.z, 1.0f };
                glLightfv(GL_LIGHT0 + lightIndex, GL_SPOT_DIRECTION, direction);
            }
            else if (light.type == LIGHT_DIRECTIONAL)
            {
                GLfloat position[4] = { -light.direction.x, -light.direction.y, -light.direction.z, 0.0f };
                glLightfv(GL_LIGHT0 + lightIndex, GL_POSITION, position);
            }
        }

        lightIndex++;
    }

    // update point lights
    glLoadIdentity();
    glScalef(1.0f, 1.0f, -1.0f);
    glMultMatrixf(m_viewMat.Array());

    lightIndex = 0;

    for (const Light &light : m_lights)
    {
        if (m_lightsEnabled[lightIndex])
        {
            if (light.type == LIGHT_POINT)
            {
                GLfloat position[4] = { light.position.x, light.position.y, light.position.z, 1.0f };
                glLightfv(GL_LIGHT0 + lightIndex, GL_POSITION, position);
            }
        }

        lightIndex++;
    }

    glPopMatrix();
}

void CGL14Device::SetLightEnabled(int index, bool enabled)
{
    assert(index >= 0);
    assert(index < static_cast<int>( m_lights.size() ));

    m_lightsEnabled[index] = enabled;

    if (enabled)
        glEnable(GL_LIGHT0 + index);
    else
        glDisable(GL_LIGHT0 + index);
}

/** If image is invalid, returns invalid texture.
    Otherwise, returns pointer to new Texture struct.
    This struct must not be deleted in other way than through DeleteTexture() */
Texture CGL14Device::CreateTexture(CImage *image, const TextureCreateParams &params)
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

Texture CGL14Device::CreateTexture(ImageData *data, const TextureCreateParams &params)
{
    Texture result;

    result.size.x = data->surface->w;
    result.size.y = data->surface->h;

    if (!Math::IsPowerOfTwo(result.size.x) || !Math::IsPowerOfTwo(result.size.y))
        GetLogger()->Warn("Creating non-power-of-2 texture (%dx%d)!\n", result.size.x, result.size.y);

    result.originalSize = result.size;

    // Use & enable 1st texture stage
    glActiveTexture(GL_TEXTURE0 + m_remap[0]);

    glEnable(GL_TEXTURE_2D);

    glGenTextures(1, &result.id);
    glBindTexture(GL_TEXTURE_2D, result.id);

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

Texture CGL14Device::CreateDepthTexture(int width, int height, int depth)
{
    Texture result;

    if (m_shadowMappingSupport == SMS_NONE)
    {
        result.id = 0;
        return result;
    }

    result.alpha = false;
    result.size.x = width;
    result.size.y = height;

    // Use & enable 1st texture stage
    glActiveTexture(GL_TEXTURE0 + m_remap[0]);

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

void CGL14Device::UpdateTexture(const Texture& texture, Math::IntPoint offset, ImageData* data, TexImgFormat format)
{
    // Use & enable 1st texture stage
    glActiveTexture(GL_TEXTURE0 + m_remap[0]);

    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, texture.id);

    PreparedTextureData texData = PrepareTextureData(data, format);

    glPixelStorei(GL_UNPACK_ROW_LENGTH, texData.actualSurface->pitch / texData.actualSurface->format->BytesPerPixel);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexSubImage2D(GL_TEXTURE_2D, 0, offset.x, offset.y, texData.actualSurface->w, texData.actualSurface->h,
                    texData.sourceFormat, GL_UNSIGNED_BYTE, texData.actualSurface->pixels);

    SDL_FreeSurface(texData.convertedSurface);
}

void CGL14Device::DestroyTexture(const Texture &texture)
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

void CGL14Device::DestroyAllTextures()
{
    // Unbind all texture stages
    for (int index = 0; index < static_cast<int>( m_currentTextures.size() ); ++index)
        SetTexture(index, Texture());

    for (auto it = m_allTextures.begin(); it != m_allTextures.end(); ++it)
        glDeleteTextures(1, &(*it).id);

    m_allTextures.clear();

    // recreate white texture
    glActiveTexture(GL_TEXTURE0 + m_remap[0]);

    glDeleteTextures(1, &m_whiteTexture);
    glGenTextures(1, &m_whiteTexture);
    glBindTexture(GL_TEXTURE_2D, m_whiteTexture);
    int color = 0xFFFFFFFF;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &color);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, m_currentTextures[0].id);
}

int CGL14Device::GetMaxTextureStageCount()
{
    return m_currentTextures.size();
}

/**
  If \a texture is invalid, unbinds the given texture.
  If valid, binds the texture and enables the given texture stage.
  The setting is remembered, even if texturing is disabled at the moment. */
void CGL14Device::SetTexture(int index, const Texture &texture)
{
    assert(index >= 0 && index < static_cast<int>( m_currentTextures.size() ));

    bool same = m_currentTextures[index].id == texture.id;

    m_currentTextures[index] = texture; // remember the new value

    if (same)
        return; // nothing to do

    glActiveTexture(GL_TEXTURE0 + m_remap[index]);

    glBindTexture(GL_TEXTURE_2D, texture.id);

    // Params need to be updated for the new bound texture
    UpdateTextureParams(index);
}

void CGL14Device::SetTexture(int index, unsigned int textureId)
{
    assert(index >= 0 && index < static_cast<int>(m_currentTextures.size()));

    if (m_currentTextures[index].id == textureId)
        return; // nothing to do

    m_currentTextures[index].id = textureId;

    glActiveTexture(GL_TEXTURE0 + m_remap[index]);

    glBindTexture(GL_TEXTURE_2D, textureId);

    // Params need to be updated for the new bound texture
    UpdateTextureParams(index);
}

void CGL14Device::SetTextureEnabled(int index, bool enabled)
{
    assert(index >= 0 && index < static_cast<int>(m_currentTextures.size()));

    bool same = m_texturesEnabled[index] == enabled;

    m_texturesEnabled[index] = enabled;

    if (same)
        return; // nothing to do

    glActiveTexture(GL_TEXTURE0 + m_remap[index]);

    if (enabled)
        glEnable(GL_TEXTURE_2D);
    else
        glDisable(GL_TEXTURE_2D);
}

/**
  Sets the texture parameters for the given texture stage.
  If the given texture was not set (bound) yet, nothing happens.
  The settings are remembered, even if texturing is disabled at the moment. */
void CGL14Device::SetTextureStageParams(int index, const TextureStageParams &params)
{
    assert(index >= 0 && index < static_cast<int>(m_currentTextures.size()));

    // Remember the settings
    m_textureStageParams[index] = params;

    UpdateTextureParams(index);
}

void CGL14Device::UpdateTextureParams(int index)
{
    assert(index >= 0 && index < static_cast<int>(m_currentTextures.size()));

    // Don't actually do anything if texture not set
    if (! m_currentTextures[index].Valid())
        return;

    const TextureStageParams &params = m_textureStageParams[index];

    glActiveTexture(GL_TEXTURE0 + m_remap[index]);

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

    glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, params.factor.Array());

    // To save some trouble
    if ( (params.colorOperation == TEX_MIX_OPER_DEFAULT) &&
         (params.alphaOperation == TEX_MIX_OPER_DEFAULT) )
    {
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        goto after_tex_operations;
    }

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

    // Only these modes of getting color & alpha are used
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);

    // Color operation

    if (params.colorOperation == TEX_MIX_OPER_DEFAULT)
    {
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
        goto after_tex_color;
    }
    else if (params.colorOperation == TEX_MIX_OPER_REPLACE)
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
    else if (params.colorOperation == TEX_MIX_OPER_MODULATE)
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
    else if (params.colorOperation == TEX_MIX_OPER_ADD)
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_ADD);
    else if (params.colorOperation == TEX_MIX_OPER_SUBTRACT)
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_SUBTRACT);
    else  assert(false);

    // Color arg1
    if (params.colorArg1 == TEX_MIX_ARG_TEXTURE)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
    else if (params.colorArg1 == TEX_MIX_ARG_TEXTURE_0)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE0);
    else if (params.colorArg1 == TEX_MIX_ARG_TEXTURE_1)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE1);
    else if (params.colorArg1 == TEX_MIX_ARG_TEXTURE_2)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE2);
    else if (params.colorArg1 == TEX_MIX_ARG_TEXTURE_3)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE3);
    else if (params.colorArg1 == TEX_MIX_ARG_COMPUTED_COLOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
    else if (params.colorArg1 == TEX_MIX_ARG_SRC_COLOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PRIMARY_COLOR);
    else if (params.colorArg1 == TEX_MIX_ARG_FACTOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_CONSTANT);
    else  assert(false);

    // Color arg2
    if (params.colorArg2 == TEX_MIX_ARG_TEXTURE)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
    else if (params.colorArg2 == TEX_MIX_ARG_TEXTURE_0)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE0);
    else if (params.colorArg2 == TEX_MIX_ARG_TEXTURE_1)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE1);
    else if (params.colorArg2 == TEX_MIX_ARG_TEXTURE_2)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE2);
    else if (params.colorArg2 == TEX_MIX_ARG_TEXTURE_3)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE3);
    else if (params.colorArg2 == TEX_MIX_ARG_COMPUTED_COLOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PREVIOUS);
    else if (params.colorArg2 == TEX_MIX_ARG_SRC_COLOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PRIMARY_COLOR);
    else if (params.colorArg2 == TEX_MIX_ARG_FACTOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_CONSTANT);
    else  assert(false);


after_tex_color:

    // Alpha operation
    if (params.alphaOperation == TEX_MIX_OPER_DEFAULT)
    {
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PREVIOUS);
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE);
        goto after_tex_operations;
    }
    else if (params.alphaOperation == TEX_MIX_OPER_REPLACE)
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
    else if (params.alphaOperation == TEX_MIX_OPER_MODULATE)
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
    else if (params.alphaOperation == TEX_MIX_OPER_ADD)
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_ADD);
    else if (params.alphaOperation == TEX_MIX_OPER_SUBTRACT)
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_SUBTRACT);
    else  assert(false);

    // Alpha arg1
    if (params.alphaArg1 == TEX_MIX_ARG_TEXTURE)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE);
    else if (params.alphaArg1 == TEX_MIX_ARG_TEXTURE_0)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE0);
    else if (params.alphaArg1 == TEX_MIX_ARG_TEXTURE_1)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE1);
    else if (params.alphaArg1 == TEX_MIX_ARG_TEXTURE_2)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE2);
    else if (params.alphaArg1 == TEX_MIX_ARG_TEXTURE_3)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE3);
    else if (params.alphaArg1 == TEX_MIX_ARG_COMPUTED_COLOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PREVIOUS);
    else if (params.alphaArg1 == TEX_MIX_ARG_SRC_COLOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
    else if (params.alphaArg1 == TEX_MIX_ARG_FACTOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_CONSTANT);
    else  assert(false);

    // Alpha arg2
    if (params.alphaArg2 == TEX_MIX_ARG_TEXTURE)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE);
    else if (params.alphaArg2 == TEX_MIX_ARG_TEXTURE_0)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE0);
    else if (params.alphaArg2 == TEX_MIX_ARG_TEXTURE_1)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE1);
    else if (params.alphaArg2 == TEX_MIX_ARG_TEXTURE_2)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE2);
    else if (params.alphaArg2 == TEX_MIX_ARG_TEXTURE_3)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE3);
    else if (params.alphaArg2 == TEX_MIX_ARG_COMPUTED_COLOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_PREVIOUS);
    else if (params.alphaArg2 == TEX_MIX_ARG_SRC_COLOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_PRIMARY_COLOR);
    else if (params.alphaArg2 == TEX_MIX_ARG_FACTOR)
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_CONSTANT);
    else  assert(false);

after_tex_operations: ;
}

void CGL14Device::EnableShadows()
{
    // already enabled
    if (m_shadowMapping) return;

    // shadow map unit
    glActiveTexture(GL_TEXTURE0 + m_remap[2]);
    glEnable(GL_TEXTURE_2D);

    glMatrixMode(GL_TEXTURE);
    glLoadMatrixf(m_shadowMatrix.Array());

    // enable texture coordinate generation
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_GEN_R);
    glEnable(GL_TEXTURE_GEN_Q);

    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);

    float plane1[] = { 1.0f, 0.0f, 0.0f, 0.0f };
    float plane2[] = { 0.0f, 1.0f, 0.0f, 0.0f };
    float plane3[] = { 0.0f, 0.0f, 1.0f, 0.0f };
    float plane4[] = { 0.0f, 0.0f, 0.0f, 1.0f };

    glTexGenfv(GL_S, GL_EYE_PLANE, plane1);
    glTexGenfv(GL_T, GL_EYE_PLANE, plane2);
    glTexGenfv(GL_R, GL_EYE_PLANE, plane3);
    glTexGenfv(GL_Q, GL_EYE_PLANE, plane4);

    // simple shadows
    if (!m_shadowQuality)
    {
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }
    // quality shadows
    else
    {
        // texture environment settings
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

        float half[] = { 0.5f, 0.5f, 0.5f, 1.0f };
        glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, half);

        // color = 0.5 * (1.0 - shadow)
        //       = 0.5 for shadow = 0.0
        //       = 0.0 for shadow = 1.0
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);

        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_ONE_MINUS_SRC_COLOR);

        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_CONSTANT);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

        // alpha = previous
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);

        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PREVIOUS);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);


        // combine unit
        glActiveTexture(GL_TEXTURE0 + m_remap[3]);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, m_whiteTexture);

        // texture enviromnent settings
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

        // color = (1.0 - previous) * primary color
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);

        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_ONE_MINUS_SRC_COLOR);

        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PRIMARY_COLOR);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

        // alpha = primary color
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);

        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
    }

    m_shadowMapping = true;
}

void CGL14Device::DisableShadows()
{
    // already disabled
    if (!m_shadowMapping) return;

    glActiveTexture(GL_TEXTURE0 + m_remap[2]);
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_GEN_R);
    glDisable(GL_TEXTURE_GEN_Q);

    // quality shadows
    if (m_shadowQuality)
    {
        glActiveTexture(GL_TEXTURE0 + m_remap[3]);
        glDisable(GL_TEXTURE_2D);
    }

    m_shadowMapping = false;
}

void CGL14Device::SetTextureStageWrap(int index, TexWrapMode wrapS, TexWrapMode wrapT)
{
    assert(index >= 0 && index < static_cast<int>( m_currentTextures.size() ));

    // Remember the settings
    m_textureStageParams[index].wrapS = wrapS;
    m_textureStageParams[index].wrapT = wrapT;

    // Don't actually do anything if texture not set
    if (! m_currentTextures[index].Valid())
        return;

    glActiveTexture(GL_TEXTURE0 + m_remap[index]);

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

void CGL14Device::DrawPrimitive(PrimitiveType type, const Vertex *vertices, int vertexCount,
                              Color color)
{
    Vertex* vs = const_cast<Vertex*>(vertices);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex), reinterpret_cast<GLfloat*>(&vs[0].coord));

    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, sizeof(Vertex), reinterpret_cast<GLfloat*>(&vs[0].normal));

    glClientActiveTexture(GL_TEXTURE0 + m_remap[0]);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), reinterpret_cast<GLfloat*>(&vs[0].texCoord));

    glColor4fv(color.Array());

    glDrawArrays(TranslateGfxPrimitive(type), 0, vertexCount);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY); // GL_TEXTURE0
}

void CGL14Device::DrawPrimitive(PrimitiveType type, const VertexTex2 *vertices, int vertexCount,
                              Color color)
{
    VertexTex2* vs = const_cast<VertexTex2*>(vertices);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(VertexTex2), reinterpret_cast<GLfloat*>(&vs[0].coord));

    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, sizeof(VertexTex2), reinterpret_cast<GLfloat*>(&vs[0].normal));

    glClientActiveTexture(GL_TEXTURE0 + m_remap[0]);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof(VertexTex2), reinterpret_cast<GLfloat*>(&vs[0].texCoord));

    glClientActiveTexture(GL_TEXTURE0 + m_remap[1]);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof(VertexTex2), reinterpret_cast<GLfloat*>(&vs[0].texCoord2));

    glColor4fv(color.Array());

    glDrawArrays(TranslateGfxPrimitive(type), 0, vertexCount);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY); // GL_TEXTURE1

    glClientActiveTexture(GL_TEXTURE0 + m_remap[0]);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void CGL14Device::DrawPrimitive(PrimitiveType type, const VertexCol *vertices, int vertexCount)
{
    VertexCol* vs = const_cast<VertexCol*>(vertices);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(VertexCol), reinterpret_cast<GLfloat*>(&vs[0].coord));

    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(4, GL_FLOAT, sizeof(VertexCol), reinterpret_cast<GLfloat*>(&vs[0].color));

    glDrawArrays(TranslateGfxPrimitive(type), 0, vertexCount);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

void CGL14Device::DrawPrimitive(PrimitiveType type, const void *vertices,
    int size, const VertexFormat &format, int vertexCount)
{
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

    glClientActiveTexture(GL_TEXTURE0 + m_remap[0]);
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

    glClientActiveTexture(GL_TEXTURE0 + m_remap[1]);
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
        glClientActiveTexture(GL_TEXTURE0 + m_remap[0]);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }

    if (format.tex2.enabled)
    {
        glClientActiveTexture(GL_TEXTURE0 + m_remap[1]);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
}

void CGL14Device::DrawPrimitives(PrimitiveType type, const void *vertices,
    int size, const VertexFormat &format, int first[], int count[], int drawCount)
{
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

    glClientActiveTexture(GL_TEXTURE0 + m_remap[0]);
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

    glClientActiveTexture(GL_TEXTURE0 + m_remap[1]);
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

    GLenum t = TranslateGfxPrimitive(type);

    if (m_multiDrawArrays)
    {
        glMultiDrawArrays(t, first, count, drawCount);
    }
    else
    {
        for (int i = 0; i < drawCount; i++)
            glDrawArrays(t, first[i], count[i]);
    }

    glDisableClientState(GL_VERTEX_ARRAY);

    if (format.color.enabled) glDisableClientState(GL_COLOR_ARRAY);
    if (format.normal.enabled) glDisableClientState(GL_NORMAL_ARRAY);

    if (format.tex1.enabled)
    {
        glClientActiveTexture(GL_TEXTURE0 + m_remap[0]);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }

    if (format.tex2.enabled)
    {
        glClientActiveTexture(GL_TEXTURE0 + m_remap[1]);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
}

void CGL14Device::DrawPrimitives(PrimitiveType type, const Vertex *vertices,
    int first[], int count[], int drawCount, Color color)
{
    Vertex* vs = const_cast<Vertex*>(vertices);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex), reinterpret_cast<GLfloat*>(&vs[0].coord));

    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, sizeof(Vertex), reinterpret_cast<GLfloat*>(&vs[0].normal));

    glClientActiveTexture(GL_TEXTURE0 + m_remap[0]);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), reinterpret_cast<GLfloat*>(&vs[0].texCoord));

    glColor4fv(color.Array());

    GLenum t = TranslateGfxPrimitive(type);

    if (m_multiDrawArrays)
    {
        glMultiDrawArrays(t, first, count, drawCount);
    }
    else
    {
        for (int i = 0; i < drawCount; i++)
            glDrawArrays(t, first[i], count[i]);
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY); // GL_TEXTURE0
}

void CGL14Device::DrawPrimitives(PrimitiveType type, const VertexTex2 *vertices,
    int first[], int count[], int drawCount, Color color)
{
    VertexTex2* vs = const_cast<VertexTex2*>(vertices);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(VertexTex2), reinterpret_cast<GLfloat*>(&vs[0].coord));

    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, sizeof(VertexTex2), reinterpret_cast<GLfloat*>(&vs[0].normal));

    glClientActiveTexture(GL_TEXTURE0 + m_remap[0]);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof(VertexTex2), reinterpret_cast<GLfloat*>(&vs[0].texCoord));

    glClientActiveTexture(GL_TEXTURE0 + m_remap[1]);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof(VertexTex2), reinterpret_cast<GLfloat*>(&vs[0].texCoord2));

    glColor4fv(color.Array());

    GLenum t = TranslateGfxPrimitive(type);

    if (m_multiDrawArrays)
    {
        glMultiDrawArrays(t, first, count, drawCount);
    }
    else
    {
        for (int i = 0; i < drawCount; i++)
            glDrawArrays(t, first[i], count[i]);
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY); // GL_TEXTURE1

    glClientActiveTexture(GL_TEXTURE0 + m_remap[0]);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void CGL14Device::DrawPrimitives(PrimitiveType type, const VertexCol *vertices,
    int first[], int count[], int drawCount)
{
    VertexCol* vs = const_cast<VertexCol*>(vertices);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(VertexCol), reinterpret_cast<GLfloat*>(&vs[0].coord));

    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(4, GL_FLOAT, sizeof(VertexCol), reinterpret_cast<GLfloat*>(&vs[0].color));

    GLenum t = TranslateGfxPrimitive(type);

    if (m_multiDrawArrays)
    {
        glMultiDrawArrays(t, first, count, drawCount);
    }
    else
    {
        for (int i = 0; i < drawCount; i++)
            glDrawArrays(t, first[i], count[i]);
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

unsigned int CGL14Device::CreateStaticBuffer(PrimitiveType primitiveType, const Vertex* vertices, int vertexCount)
{
    unsigned int id = 0;
    if (m_vertexBufferType != VBT_DISPLAY_LIST)
    {
        id = ++m_lastVboId;

        VboObjectInfo info;
        info.primitiveType = primitiveType;
        info.vertexType = VERTEX_TYPE_NORMAL;
        info.vertexCount = vertexCount;
        info.bufferId = 0;

        m_glGenBuffers(1, &info.bufferId);
        m_glBindBuffer(GL_ARRAY_BUFFER, info.bufferId);
        m_glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(Vertex), vertices, GL_STATIC_DRAW);
        m_glBindBuffer(GL_ARRAY_BUFFER, 0);

        m_vboObjects[id] = info;
    }
    else
    {
        id = glGenLists(1);

        glNewList(id, GL_COMPILE);

        DrawPrimitive(primitiveType, vertices, vertexCount);

        glEndList();
    }

    return id;
}

unsigned int CGL14Device::CreateStaticBuffer(PrimitiveType primitiveType, const VertexTex2* vertices, int vertexCount)
{
    unsigned int id = 0;
    if (m_vertexBufferType != VBT_DISPLAY_LIST)
    {
        id = ++m_lastVboId;

        VboObjectInfo info;
        info.primitiveType = primitiveType;
        info.vertexType = VERTEX_TYPE_TEX2;
        info.vertexCount = vertexCount;
        info.bufferId = 0;

        m_glGenBuffers(1, &info.bufferId);
        m_glBindBuffer(GL_ARRAY_BUFFER, info.bufferId);
        m_glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(VertexTex2), vertices, GL_STATIC_DRAW);
        m_glBindBuffer(GL_ARRAY_BUFFER, 0);

        m_vboObjects[id] = info;
    }
    else
    {
        id = glGenLists(1);

        glNewList(id, GL_COMPILE);

        DrawPrimitive(primitiveType, vertices, vertexCount);

        glEndList();
    }

    return id;
}

unsigned int CGL14Device::CreateStaticBuffer(PrimitiveType primitiveType, const VertexCol* vertices, int vertexCount)
{
    unsigned int id = 0;
    if (m_vertexBufferType != VBT_DISPLAY_LIST)
    {
        id = ++m_lastVboId;

        VboObjectInfo info;
        info.primitiveType = primitiveType;
        info.vertexType = VERTEX_TYPE_COL;
        info.vertexCount = vertexCount;
        info.bufferId = 0;

        m_glGenBuffers(1, &info.bufferId);
        m_glBindBuffer(GL_ARRAY_BUFFER, info.bufferId);
        m_glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(VertexCol), vertices, GL_STATIC_DRAW);
        m_glBindBuffer(GL_ARRAY_BUFFER, 0);

        m_vboObjects[id] = info;
    }
    else
    {
        id = glGenLists(1);

        glNewList(id, GL_COMPILE);

        DrawPrimitive(primitiveType, vertices, vertexCount);

        glEndList();
    }

    return id;
}

void CGL14Device::UpdateStaticBuffer(unsigned int bufferId, PrimitiveType primitiveType, const Vertex* vertices, int vertexCount)
{
    if (m_vertexBufferType != VBT_DISPLAY_LIST)
    {
        auto it = m_vboObjects.find(bufferId);
        if (it == m_vboObjects.end())
            return;

        VboObjectInfo& info = (*it).second;
        info.primitiveType = primitiveType;
        info.vertexType = VERTEX_TYPE_NORMAL;
        info.vertexCount = vertexCount;

        m_glBindBuffer(GL_ARRAY_BUFFER, info.bufferId);
        m_glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(Vertex), vertices, GL_STATIC_DRAW);
        m_glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    else
    {
        glNewList(bufferId, GL_COMPILE);

        DrawPrimitive(primitiveType, vertices, vertexCount);

        glEndList();
    }
}

void CGL14Device::UpdateStaticBuffer(unsigned int bufferId, PrimitiveType primitiveType, const VertexTex2* vertices, int vertexCount)
{
    if (m_vertexBufferType != VBT_DISPLAY_LIST)
    {
        auto it = m_vboObjects.find(bufferId);
        if (it == m_vboObjects.end())
            return;

        VboObjectInfo& info = (*it).second;
        info.primitiveType = primitiveType;
        info.vertexType = VERTEX_TYPE_TEX2;
        info.vertexCount = vertexCount;

        m_glBindBuffer(GL_ARRAY_BUFFER, info.bufferId);
        m_glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(VertexTex2), vertices, GL_STATIC_DRAW);
        m_glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    else
    {
        glNewList(bufferId, GL_COMPILE);

        DrawPrimitive(primitiveType, vertices, vertexCount);

        glEndList();
    }
}

void CGL14Device::UpdateStaticBuffer(unsigned int bufferId, PrimitiveType primitiveType, const VertexCol* vertices, int vertexCount)
{
    if (m_vertexBufferType != VBT_DISPLAY_LIST)
    {
        auto it = m_vboObjects.find(bufferId);
        if (it == m_vboObjects.end())
            return;

        VboObjectInfo& info = (*it).second;
        info.primitiveType = primitiveType;
        info.vertexType = VERTEX_TYPE_COL;
        info.vertexCount = vertexCount;

        m_glBindBuffer(GL_ARRAY_BUFFER, info.bufferId);
        m_glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(VertexCol), vertices, GL_STATIC_DRAW);
        m_glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    else
    {
        glNewList(bufferId, GL_COMPILE);

        DrawPrimitive(primitiveType, vertices, vertexCount);

        glEndList();
    }
}

void CGL14Device::DrawStaticBuffer(unsigned int bufferId)
{
    if (m_vertexBufferType != VBT_DISPLAY_LIST)
    {
        auto it = m_vboObjects.find(bufferId);
        if (it == m_vboObjects.end())
            return;

        m_glBindBuffer(GL_ARRAY_BUFFER, (*it).second.bufferId);

        if ((*it).second.vertexType == VERTEX_TYPE_NORMAL)
        {
            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(3, GL_FLOAT, sizeof(Vertex), static_cast<char*>(nullptr) + offsetof(Vertex, coord));

            glEnableClientState(GL_NORMAL_ARRAY);
            glNormalPointer(GL_FLOAT, sizeof(Vertex), static_cast<char*>(nullptr) + offsetof(Vertex, normal));

            glClientActiveTexture(GL_TEXTURE0 + m_remap[0]);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), static_cast<char*>(nullptr) + offsetof(Vertex, texCoord));
        }
        else if ((*it).second.vertexType == VERTEX_TYPE_TEX2)
        {
            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(3, GL_FLOAT, sizeof(VertexTex2), static_cast<char*>(nullptr) + offsetof(VertexTex2, coord));

            glEnableClientState(GL_NORMAL_ARRAY);
            glNormalPointer(GL_FLOAT, sizeof(VertexTex2), static_cast<char*>(nullptr) + offsetof(VertexTex2, normal));

            glClientActiveTexture(GL_TEXTURE0 + m_remap[0]);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(2, GL_FLOAT, sizeof(VertexTex2), static_cast<char*>(nullptr) + offsetof(VertexTex2, texCoord));

            glClientActiveTexture(GL_TEXTURE0 + m_remap[1]);
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

            glClientActiveTexture(GL_TEXTURE0 + m_remap[0]);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }
        else if ((*it).second.vertexType == VERTEX_TYPE_COL)
        {
            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_COLOR_ARRAY);
        }

        m_glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    else
    {
        glCallList(bufferId);
    }
}

void CGL14Device::DestroyStaticBuffer(unsigned int bufferId)
{
    if (m_vertexBufferType != VBT_DISPLAY_LIST)
    {
        auto it = m_vboObjects.find(bufferId);
        if (it == m_vboObjects.end())
            return;

        m_glDeleteBuffers(1, &(*it).second.bufferId);

        m_vboObjects.erase(it);
    }
    else
    {
        glDeleteLists(bufferId, 1);
    }
}

/* Based on libwine's implementation */

int CGL14Device::ComputeSphereVisibility(const Math::Vector &center, float radius)
{
    if (m_combinedMatrixOutdated)
    {
        m_combinedMatrix = Math::MultiplyMatrices(m_projectionMat, m_modelviewMat);
        m_combinedMatrixOutdated = false;
    }

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

void CGL14Device::SetViewport(int x, int y, int width, int height)
{
    glViewport(x, y, width, height);
}

void CGL14Device::SetRenderState(RenderState state, bool enabled)
{
    if (state == RENDER_STATE_DEPTH_WRITE)
    {
        glDepthMask(enabled ? GL_TRUE : GL_FALSE);
        return;
    }
    else if (state == RENDER_STATE_LIGHTING)
    {
        m_lighting = enabled;

        if (enabled)
            glEnable(GL_LIGHTING);
        else
            glDisable(GL_LIGHTING);

        if (enabled)
        {
            UpdateLightPositions();
        }

        return;
    }
    else if (state == RENDER_STATE_SHADOW_MAPPING)
    {
        if (enabled)
            EnableShadows();
        else
            DisableShadows();

        return;
    }

    GLenum flag = 0;

    switch (state)
    {
        case RENDER_STATE_BLENDING:    flag = GL_BLEND; break;
        case RENDER_STATE_FOG:         flag = GL_FOG; break;
        case RENDER_STATE_DEPTH_TEST:  flag = GL_DEPTH_TEST; break;
        case RENDER_STATE_ALPHA_TEST:  flag = GL_ALPHA_TEST; break;
        case RENDER_STATE_CULLING:     flag = GL_CULL_FACE; break;
        case RENDER_STATE_DEPTH_BIAS:  flag = GL_POLYGON_OFFSET_FILL; break;
        default: assert(false); break;
    }

    if (enabled)
        glEnable(flag);
    else
        glDisable(flag);
}

void CGL14Device::SetColorMask(bool red, bool green, bool blue, bool alpha)
{
    glColorMask(red, green, blue, alpha);
}

void CGL14Device::SetDepthTestFunc(CompFunc func)
{
    glDepthFunc(TranslateGfxCompFunc(func));
}

void CGL14Device::SetDepthBias(float factor, float units)
{
    glPolygonOffset(factor, units);
}

void CGL14Device::SetAlphaTestFunc(CompFunc func, float refValue)
{
    glAlphaFunc(TranslateGfxCompFunc(func), refValue);
}

void CGL14Device::SetBlendFunc(BlendFunc srcBlend, BlendFunc dstBlend)
{
    glBlendFunc(TranslateGfxBlendFunc(srcBlend), TranslateGfxBlendFunc(dstBlend));
}

void CGL14Device::SetClearColor(const Color &color)
{
    glClearColor(color.r, color.g, color.b, color.a);
}

void CGL14Device::SetGlobalAmbient(const Color &color)
{
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, color.Array());
}

void CGL14Device::SetFogParams(FogMode mode, const Color &color, float start, float end, float density)
{
    if      (mode == FOG_LINEAR) glFogi(GL_FOG_MODE, GL_LINEAR);
    else if (mode == FOG_EXP)    glFogi(GL_FOG_MODE, GL_EXP);
    else if (mode == FOG_EXP2)   glFogi(GL_FOG_MODE, GL_EXP2);
    else assert(false);

    glFogf(GL_FOG_START,   start);
    glFogf(GL_FOG_END,     end);
    glFogf(GL_FOG_DENSITY, density);
    glFogfv(GL_FOG_COLOR,  color.Array());
}

void CGL14Device::SetCullMode(CullMode mode)
{
    // Cull clockwise back faces, so front face is the opposite
    // (assuming GL_CULL_FACE is GL_BACK)
    if      (mode == CULL_CW ) glFrontFace(GL_CCW);
    else if (mode == CULL_CCW) glFrontFace(GL_CW);
    else assert(false);
}

void CGL14Device::SetShadeModel(ShadeModel model)
{
    if      (model == SHADE_FLAT)   glShadeModel(GL_FLAT);
    else if (model == SHADE_SMOOTH) glShadeModel(GL_SMOOTH);
    else  assert(false);
}

void CGL14Device::SetShadowColor(float value)
{
    // doesn't do anything because it can't
}

void CGL14Device::SetFillMode(FillMode mode)
{
    if      (mode == FILL_POINT) glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    else if (mode == FILL_LINES) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else if (mode == FILL_POLY)  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else assert(false);
}

void CGL14Device::CopyFramebufferToTexture(Texture& texture, int xOffset, int yOffset, int x, int y, int width, int height)
{
    if (texture.id == 0) return;

    // Use & enable 1st texture stage
    glActiveTexture(GL_TEXTURE0 + m_remap[0]);

    glBindTexture(GL_TEXTURE_2D, texture.id);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, xOffset, yOffset, x, y, width, height);

    // Restore previous texture
    glBindTexture(GL_TEXTURE_2D, m_currentTextures[0].id);
}

std::unique_ptr<CFrameBufferPixels> CGL14Device::GetFrameBufferPixels() const
{
    return GetGLFrameBufferPixels(m_config.size);
}

CFramebuffer* CGL14Device::GetFramebuffer(std::string name)
{
    auto it = m_framebuffers.find(name);
    if (it == m_framebuffers.end())
        return nullptr;

    return it->second.get();
}

CFramebuffer* CGL14Device::CreateFramebuffer(std::string name, const FramebufferParams& params)
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

void CGL14Device::DeleteFramebuffer(std::string name)
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

bool CGL14Device::IsAnisotropySupported()
{
    return m_capabilities.anisotropySupported;
}

int CGL14Device::GetMaxAnisotropyLevel()
{
    return m_capabilities.maxAnisotropy;
}

int CGL14Device::GetMaxSamples()
{
    return m_capabilities.maxSamples;
}

bool CGL14Device::IsShadowMappingSupported()
{
    return m_capabilities.shadowMappingSupported;
}

int CGL14Device::GetMaxTextureSize()
{
    return m_capabilities.maxTextureSize;
}

bool CGL14Device::IsFramebufferSupported()
{
    return m_capabilities.framebufferSupported;
}

} // namespace Gfx
