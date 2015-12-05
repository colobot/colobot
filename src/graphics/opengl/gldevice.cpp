/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2015, Daniel Roux, EPSITEC SA & TerranovaTeam
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


#include "graphics/opengl/gldevice.h"

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

CGLDevice::CGLDevice(const DeviceConfig &config)
    : m_config(config)
{}

CGLDevice::~CGLDevice()
{
}

void CGLDevice::DebugHook()
{
    /* This function is only called here, so it can be used
     * as a breakpoint when debugging using gDEBugger */
    glColor3i(0, 0, 0);
}

void CGLDevice::DebugLights()
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

bool CGLDevice::Create()
{
    GetLogger()->Info("Creating CDevice - OpenGL 1.4\n");

    static bool glewInited = false;

    if (!glewInited)
    {
        glewInited = true;

        glewExperimental = GL_TRUE;

        if (glewInit() != GLEW_OK)
        {
            GetLogger()->Error("GLEW initialization failed\n");
            return false;
        }

        // Extract OpenGL version
        const char *version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
        sscanf(version, "%d.%d", &m_glMajor, &m_glMinor);
        GetLogger()->Info("OpenGL %d.%d\n", m_glMajor, m_glMinor);

        // Detect multitexture support
        m_multitextureAvailable = glewIsSupported("GL_ARB_multitexture GL_ARB_texture_env_combine");
        if (!m_multitextureAvailable)
            GetLogger()->Warn("GLEW reports multitexturing not supported - graphics quality will be degraded!\n");

        // Detect Shadow mapping support
        if (m_glMajor > 1 || m_glMinor >= 4)     // Core depth texture+shadow, OpenGL 1.4+
        {
            m_shadowMappingSupport = SMS_CORE;
            GetLogger()->Info("Shadow mapping available (core)\n");
        }
        else if (glewIsSupported("GL_ARB_depth_texture GL_ARB_shadow"))  // ARB depth texture + shadow
        {
            m_shadowMappingSupport = SMS_ARB;
            GetLogger()->Info("Shadow mapping available (ARB)\n");
        }
        else       // No Shadow mapping
        {
            m_shadowMappingSupport = SMS_NONE;
            GetLogger()->Info("Shadow mapping not available\n");
        }

        m_shadowAmbientSupported = glewIsSupported("GL_ARB_shadow_ambient");
        if (m_shadowAmbientSupported)
            GetLogger()->Info("Shadow ambient supported\n");

        // Detect support of anisotropic filtering
        m_anisotropyAvailable = glewIsSupported("GL_EXT_texture_filter_anisotropic");
        if(m_anisotropyAvailable)
        {
            // Obtain maximum anisotropy level available
            float level;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &level);
            m_maxAnisotropy = static_cast<int>(level);

            GetLogger()->Info("Anisotropic filtering available\n");
            GetLogger()->Info("Maximum anisotropy: %d\n", m_maxAnisotropy);
        }
        else
        {
            GetLogger()->Info("Anisotropic filtering not available\n");
        }

        // Read maximum sample count for MSAA
        if(glewIsSupported("GL_ARB_multisample"))
        {
            glGetIntegerv(GL_MAX_SAMPLES_EXT, &m_maxSamples);
            GetLogger()->Info("Multisampling supported, max samples: %d\n", m_maxSamples);
        }
        else
        {
            GetLogger()->Info("Multisampling not supported\n");
        }

        GetLogger()->Info("Auto-detecting VBO support\n");

        // detecting VBO ARB extension
        bool vboARB = glewIsSupported("GL_ARB_vertex_buffer_object");

        // VBO is core OpenGL feature since 1.5
        // everything below 1.5 means no VBO support
        if (m_glMajor > 1 || m_glMinor > 4)
        {
            GetLogger()->Info("Core VBO supported\n", m_glMajor, m_glMinor);
            m_vertexBufferType = VBT_VBO_CORE;
        }
        else if(vboARB)     // VBO ARB extension available
        {
            GetLogger()->Info("ARB VBO supported\n");
            m_vertexBufferType = VBT_VBO_ARB;
        }
        else                // no VBO support
        {
            GetLogger()->Info("VBO not supported\n");
            m_vertexBufferType = VBT_DISPLAY_LIST;
        }
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

    m_lights        = std::vector<Light>(numLights, Light());
    m_lightsEnabled = std::vector<bool> (numLights, false);

    int maxTextures = 0;
    glGetIntegerv(GL_MAX_TEXTURE_UNITS, &maxTextures);
    GetLogger()->Info("Maximum texture units: %d\n", maxTextures);

    m_currentTextures    = std::vector<Texture>           (maxTextures, Texture());
    m_texturesEnabled    = std::vector<bool>              (maxTextures, false);
    m_textureStageParams = std::vector<TextureStageParams>(maxTextures, TextureStageParams());

    // create default framebuffer object
    FramebufferParams framebufferParams;

    framebufferParams.width = m_config.size.x;
    framebufferParams.height = m_config.size.y;
    framebufferParams.depth = m_config.depthSize;

    m_framebuffers["default"] = MakeUnique<CDefaultFramebuffer>(framebufferParams);

    m_framebufferSupport = DetectFramebufferSupport();
    if (m_framebufferSupport != FBS_NONE)
        GetLogger()->Debug("Framebuffer supported\n");

    GetLogger()->Info("CDevice created successfully\n");

    return true;
}

void CGLDevice::Destroy()
{
    // delete framebuffers
    for (auto& framebuffer : m_framebuffers)
        framebuffer.second->Destroy();

    m_framebuffers.clear();

    // Delete the remaining textures
    // Should not be strictly necessary, but just in case
    DestroyAllTextures();

    m_lights.clear();
    m_lightsEnabled.clear();

    m_currentTextures.clear();
    m_texturesEnabled.clear();
    m_textureStageParams.clear();
}

void CGLDevice::ConfigChanged(const DeviceConfig& newConfig)
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

void CGLDevice::BeginScene()
{
    Clear();

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(m_projectionMat.Array());

    UpdateModelviewMatrix();
}

void CGLDevice::EndScene()
{
}

void CGLDevice::Clear()
{
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void CGLDevice::SetTransform(TransformType type, const Math::Matrix &matrix)
{
    if      (type == TRANSFORM_WORLD)
    {
        m_worldMat = matrix;
        UpdateModelviewMatrix();
    }
    else if (type == TRANSFORM_VIEW)
    {
        m_viewMat = matrix;
        UpdateModelviewMatrix();
    }
    else if (type == TRANSFORM_PROJECTION)
    {
        m_projectionMat = matrix;
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(m_projectionMat.Array());
    }
    else if (type == TRANSFORM_SHADOW)
    {
        if (!m_multitextureAvailable)
            return;

        Math::Matrix temp = matrix;
        glActiveTexture(GL_TEXTURE2);
        glMatrixMode(GL_TEXTURE);
        glLoadMatrixf(temp.Array());
    }
    else
    {
        assert(false);
    }
}

void CGLDevice::UpdateModelviewMatrix()
{
    m_modelviewMat = Math::MultiplyMatrices(m_viewMat, m_worldMat);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glScalef(1.0f, 1.0f, -1.0f);
    glMultMatrixf(m_modelviewMat.Array());

    if (m_lighting)
    {
        for (int index = 0; index < static_cast<int>( m_lights.size() ); ++index)
            UpdateLightPosition(index);
    }
}

void CGLDevice::SetMaterial(const Material &material)
{
    m_material = material;

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,  m_material.ambient.Array());
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_material.diffuse.Array());
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_material.specular.Array());
}

int CGLDevice::GetMaxLightCount()
{
    return m_lights.size();
}

void CGLDevice::SetLight(int index, const Light &light)
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

void CGLDevice::UpdateLightPosition(int index)
{
    assert(index >= 0);
    assert(index < static_cast<int>( m_lights.size() ));

    glMatrixMode(GL_MODELVIEW);

    glPushMatrix();

    glLoadIdentity();
    glScalef(1.0f, 1.0f, -1.0f);
    Math::Matrix mat = m_viewMat;
    mat.Set(1, 4, 0.0f);
    mat.Set(2, 4, 0.0f);
    mat.Set(3, 4, 0.0f);
    glMultMatrixf(mat.Array());

    if (m_lights[index].type == LIGHT_SPOT)
    {
        GLfloat direction[4] = { -m_lights[index].direction.x, -m_lights[index].direction.y, -m_lights[index].direction.z, 1.0f };
        glLightfv(GL_LIGHT0 + index, GL_SPOT_DIRECTION, direction);
    }

    if (m_lights[index].type == LIGHT_DIRECTIONAL)
    {
        GLfloat position[4] = { -m_lights[index].direction.x, -m_lights[index].direction.y, -m_lights[index].direction.z, 0.0f };
        glLightfv(GL_LIGHT0 + index, GL_POSITION, position);
    }
    else
    {
        glLoadIdentity();
        glScalef(1.0f, 1.0f, -1.0f);
        glMultMatrixf(m_viewMat.Array());

        GLfloat position[4] = { m_lights[index].position.x, m_lights[index].position.y, m_lights[index].position.z, 1.0f };
        glLightfv(GL_LIGHT0 + index, GL_POSITION, position);
    }

    glPopMatrix();
}

void CGLDevice::SetLightEnabled(int index, bool enabled)
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
Texture CGLDevice::CreateTexture(CImage *image, const TextureCreateParams &params)
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

Texture CGLDevice::CreateTexture(ImageData *data, const TextureCreateParams &params)
{
    Texture result;

    result.size.x = data->surface->w;
    result.size.y = data->surface->h;

    if (!Math::IsPowerOfTwo(result.size.x) || !Math::IsPowerOfTwo(result.size.y))
        GetLogger()->Warn("Creating non-power-of-2 texture (%dx%d)!\n", result.size.x, result.size.y);

    result.originalSize = result.size;

    // Use & enable 1st texture stage
    if (m_multitextureAvailable)
        glActiveTexture(GL_TEXTURE0);

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
    if (m_anisotropyAvailable)
    {
        float level = Math::Min(m_maxAnisotropy, CEngine::GetInstance().GetTextureAnisotropyLevel());

        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, level);
    }

    bool convert = false;
    GLenum sourceFormat = 0;

    if (params.format == TEX_IMG_RGB)
    {
        sourceFormat = GL_RGB;
        result.alpha = false;
    }
    else if (params.format == TEX_IMG_BGR)
    {
        sourceFormat = GL_BGR;
        result.alpha = false;
    }
    else if (params.format == TEX_IMG_RGBA)
    {
        sourceFormat = GL_RGBA;
        result.alpha = true;
    }
    else if (params.format == TEX_IMG_BGRA)
    {
        sourceFormat = GL_BGRA;
        result.alpha = true;
    }
    else if (params.format == TEX_IMG_AUTO)
    {
        if (data->surface->format->BytesPerPixel == 4)
        {
            if ((data->surface->format->Amask == 0xFF000000) &&
                (data->surface->format->Rmask == 0x00FF0000) &&
                (data->surface->format->Gmask == 0x0000FF00) &&
                (data->surface->format->Bmask == 0x000000FF))
            {
                sourceFormat = GL_BGRA;
                result.alpha = true;
            }
            else if ((data->surface->format->Amask == 0xFF000000) &&
                     (data->surface->format->Bmask == 0x00FF0000) &&
                     (data->surface->format->Gmask == 0x0000FF00) &&
                     (data->surface->format->Rmask == 0x000000FF))
            {
                sourceFormat = GL_RGBA;
                result.alpha = true;
            }
            else
            {
                sourceFormat = GL_RGBA;
                convert = true;
            }
        }
        else if (data->surface->format->BytesPerPixel == 3)
        {
            if ((data->surface->format->Rmask == 0xFF0000) &&
                (data->surface->format->Gmask == 0x00FF00) &&
                (data->surface->format->Bmask == 0x0000FF))
            {
                sourceFormat = GL_BGR;
                result.alpha = false;
            }
            else if ((data->surface->format->Bmask == 0xFF0000) &&
                     (data->surface->format->Gmask == 0x00FF00) &&
                     (data->surface->format->Rmask == 0x0000FF))
            {
                sourceFormat = GL_RGB;
                result.alpha = false;
            }
            else
            {
                sourceFormat = GL_RGBA;
                convert = true;
            }
        }
        else
        {
            GetLogger()->Error("Unknown data surface format");
            assert(false);
        }
    }
    else
        assert(false);

    SDL_Surface* actualSurface = data->surface;
    SDL_Surface* convertedSurface = nullptr;

    if (convert)
    {
        SDL_PixelFormat format;
        format.BytesPerPixel = 4;
        format.BitsPerPixel = 32;
        format.Aloss = format.Bloss = format.Gloss = format.Rloss = 0;
        format.Amask = 0xFF000000;
        format.Ashift = 24;
        format.Bmask = 0x00FF0000;
        format.Bshift = 16;
        format.Gmask = 0x0000FF00;
        format.Gshift = 8;
        format.Rmask = 0x000000FF;
        format.Rshift = 0;
        format.palette = nullptr;
        convertedSurface = SDL_ConvertSurface(data->surface, &format, SDL_SWSURFACE);
        if (convertedSurface != nullptr)
            actualSurface = convertedSurface;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, actualSurface->w, actualSurface->h,
                 0, sourceFormat, GL_UNSIGNED_BYTE, actualSurface->pixels);

    SDL_FreeSurface(convertedSurface);

    m_allTextures.insert(result);

    // Restore the previous state of 1st stage
    glBindTexture(GL_TEXTURE_2D, m_currentTextures[0].id);

    if (! m_texturesEnabled[0])
        glDisable(GL_TEXTURE_2D);

    return result;
}

Texture CGLDevice::CreateDepthTexture(int width, int height, int depth)
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
    if (m_multitextureAvailable)
        glActiveTexture(GL_TEXTURE0);

    glGenTextures(1, &result.id);
    glBindTexture(GL_TEXTURE_2D, result.id);

    GLuint format = GL_DEPTH_COMPONENT;

    if (m_shadowMappingSupport == SMS_CORE)
    {
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
    }
    else
    {
        switch (depth)
        {
        case 16:
            format = GL_DEPTH_COMPONENT16_ARB;
            break;
        case 24:
            format = GL_DEPTH_COMPONENT24_ARB;
            break;
        case 32:
            format = GL_DEPTH_COMPONENT32_ARB;
            break;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_DEPTH_COMPONENT, GL_INT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL);
    }

    if (m_shadowAmbientSupported)
    {
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FAIL_VALUE_ARB, 0.5f);
    }

    float color[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

    glBindTexture(GL_TEXTURE_2D, m_currentTextures[0].id);

    return result;
}

void CGLDevice::DestroyTexture(const Texture &texture)
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

void CGLDevice::DestroyAllTextures()
{
    // Unbind all texture stages
    for (int index = 0; index < static_cast<int>( m_currentTextures.size() ); ++index)
        SetTexture(index, Texture());

    for (auto it = m_allTextures.begin(); it != m_allTextures.end(); ++it)
        glDeleteTextures(1, &(*it).id);

    m_allTextures.clear();
}

int CGLDevice::GetMaxTextureStageCount()
{
    return m_currentTextures.size();
}

/**
  If \a texture is invalid, unbinds the given texture.
  If valid, binds the texture and enables the given texture stage.
  The setting is remembered, even if texturing is disabled at the moment. */
void CGLDevice::SetTexture(int index, const Texture &texture)
{
    assert(index >= 0 && index < static_cast<int>( m_currentTextures.size() ));

    bool same = m_currentTextures[index].id == texture.id;

    m_currentTextures[index] = texture; // remember the new value

    if (!m_multitextureAvailable && index != 0)
        return;

    if (same)
        return; // nothing to do

    if (m_multitextureAvailable)
        glActiveTexture(GL_TEXTURE0 + index);

    glBindTexture(GL_TEXTURE_2D, texture.id);

    // Params need to be updated for the new bound texture
    UpdateTextureParams(index);
}

void CGLDevice::SetTexture(int index, unsigned int textureId)
{
    assert(index >= 0 && index < static_cast<int>( m_currentTextures.size() ));

    if (m_currentTextures[index].id == textureId)
        return; // nothing to do

    m_currentTextures[index].id = textureId;

    if (!m_multitextureAvailable && index != 0)
        return;

    if (m_multitextureAvailable)
        glActiveTexture(GL_TEXTURE0 + index);

    glBindTexture(GL_TEXTURE_2D, textureId);

    // Params need to be updated for the new bound texture
    UpdateTextureParams(index);
}

void CGLDevice::SetTextureEnabled(int index, bool enabled)
{
    assert(index >= 0 && index < static_cast<int>( m_currentTextures.size() ));

    bool same = m_texturesEnabled[index] == enabled;

    m_texturesEnabled[index] = enabled;

    if (same)
        return; // nothing to do

    if (!m_multitextureAvailable && index != 0)
        return;

    if (m_multitextureAvailable)
        glActiveTexture(GL_TEXTURE0 + index);

    if (enabled)
        glEnable(GL_TEXTURE_2D);
    else
        glDisable(GL_TEXTURE_2D);
}

/**
  Sets the texture parameters for the given texture stage.
  If the given texture was not set (bound) yet, nothing happens.
  The settings are remembered, even if texturing is disabled at the moment. */
void CGLDevice::SetTextureStageParams(int index, const TextureStageParams &params)
{
    assert(index >= 0 && index < static_cast<int>( m_currentTextures.size() ));

    // Remember the settings
    m_textureStageParams[index] = params;

    UpdateTextureParams(index);
}

void CGLDevice::SetTextureCoordGeneration(int index, TextureGenerationParams &params)
{
    if (!m_multitextureAvailable && index != 0)
        return;

    if (m_multitextureAvailable)
        glActiveTexture(GL_TEXTURE0 + index);

    for (int i = 0; i < 4; i++)
    {
        GLuint texCoordGen = TranslateTextureCoordinateGen(i);
        GLuint texCoord = TranslateTextureCoordinate(i);

        switch (params.coords[i].mode)
        {
        case TEX_GEN_NONE:
            glDisable(texCoordGen);
            break;
        case TEX_GEN_OBJECT_LINEAR:
            glEnable(texCoordGen);
            glTexGeni(texCoord, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
            glTexGenfv(texCoord, GL_OBJECT_PLANE, params.coords[i].plane);
            break;
        case TEX_GEN_EYE_LINEAR:
            glEnable(texCoordGen);
            glTexGeni(texCoord, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
            glTexGenfv(texCoord, GL_EYE_PLANE, params.coords[i].plane);
            break;
        case TEX_GEN_SPHERE_MAP:
            glEnable(texCoordGen);
            glTexGeni(texCoord, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
            break;
        case TEX_GEN_NORMAL_MAP:
            glEnable(texCoordGen);
            glTexGeni(texCoord, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP);
            break;
        case TEX_GEN_REFLECTION_MAP:
            glEnable(texCoordGen);
            glTexGeni(texCoord, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
            break;
        }
    }
}

void CGLDevice::UpdateTextureParams(int index)
{
    assert(index >= 0 && index < static_cast<int>( m_currentTextures.size() ));

    if (!m_multitextureAvailable && index != 0)
        return;

    // Don't actually do anything if texture not set
    if (! m_currentTextures[index].Valid())
        return;

    const TextureStageParams &params = m_textureStageParams[index];

    if (m_multitextureAvailable)
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

    // Texture env setting is silly without multitexturing
    if (!m_multitextureAvailable)
        return;

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

void CGLDevice::SetTextureStageWrap(int index, TexWrapMode wrapS, TexWrapMode wrapT)
{
    assert(index >= 0 && index < static_cast<int>( m_currentTextures.size() ));

    // Remember the settings
    m_textureStageParams[index].wrapS = wrapS;
    m_textureStageParams[index].wrapT = wrapT;

    // Don't actually do anything if texture not set
    if (! m_currentTextures[index].Valid())
        return;

    if (!m_multitextureAvailable && index != 0)
        return;

    if (m_multitextureAvailable)
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

void CGLDevice::DrawPrimitive(PrimitiveType type, const Vertex *vertices, int vertexCount,
                              Color color)
{
    Vertex* vs = const_cast<Vertex*>(vertices);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex), reinterpret_cast<GLfloat*>(&vs[0].coord));

    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, sizeof(Vertex), reinterpret_cast<GLfloat*>(&vs[0].normal));

    if (m_multitextureAvailable)
        glClientActiveTexture(GL_TEXTURE0);

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), reinterpret_cast<GLfloat*>(&vs[0].texCoord));

    glColor4fv(color.Array());

    glDrawArrays(TranslateGfxPrimitive(type), 0, vertexCount);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY); // GL_TEXTURE0
}

void CGLDevice::DrawPrimitive(PrimitiveType type, const VertexTex2 *vertices, int vertexCount,
                              Color color)
{
    VertexTex2* vs = const_cast<VertexTex2*>(vertices);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(VertexTex2), reinterpret_cast<GLfloat*>(&vs[0].coord));

    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, sizeof(VertexTex2), reinterpret_cast<GLfloat*>(&vs[0].normal));

    if (m_multitextureAvailable)
        glClientActiveTexture(GL_TEXTURE0);

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof(VertexTex2), reinterpret_cast<GLfloat*>(&vs[0].texCoord));

    if (m_multitextureAvailable)
    {
        glClientActiveTexture(GL_TEXTURE1);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, sizeof(VertexTex2), reinterpret_cast<GLfloat*>(&vs[0].texCoord2));
    }

    glColor4fv(color.Array());

    glDrawArrays(TranslateGfxPrimitive(type), 0, vertexCount);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY); // GL_TEXTURE1
    if (m_multitextureAvailable)
    {
        glClientActiveTexture(GL_TEXTURE0);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
}

void CGLDevice::DrawPrimitive(PrimitiveType type, const VertexCol *vertices, int vertexCount)
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

unsigned int CGLDevice::CreateStaticBuffer(PrimitiveType primitiveType, const Vertex* vertices, int vertexCount)
{
    unsigned int id = 0;
    if (m_vboAvailable)
    {
        id = ++m_lastVboId;

        VboObjectInfo info;
        info.primitiveType = primitiveType;
        info.vertexType = VERTEX_TYPE_NORMAL;
        info.vertexCount = vertexCount;
        info.bufferId = 0;

        if(m_vertexBufferType == VBT_VBO_CORE)
        {
            glGenBuffers(1, &info.bufferId);
            glBindBuffer(GL_ARRAY_BUFFER, info.bufferId);
            glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(Vertex), vertices, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        else
        {
            glGenBuffersARB(1, &info.bufferId);
            glBindBufferARB(GL_ARRAY_BUFFER_ARB, info.bufferId);
            glBufferDataARB(GL_ARRAY_BUFFER_ARB, vertexCount * sizeof(Vertex), vertices, GL_STATIC_DRAW_ARB);
            glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        }

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

unsigned int CGLDevice::CreateStaticBuffer(PrimitiveType primitiveType, const VertexTex2* vertices, int vertexCount)
{
    unsigned int id = 0;
    if (m_vboAvailable)
    {
        id = ++m_lastVboId;

        VboObjectInfo info;
        info.primitiveType = primitiveType;
        info.vertexType = VERTEX_TYPE_TEX2;
        info.vertexCount = vertexCount;
        info.bufferId = 0;

        if(m_vertexBufferType == VBT_VBO_CORE)
        {
            glGenBuffers(1, &info.bufferId);
            glBindBuffer(GL_ARRAY_BUFFER, info.bufferId);
            glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(VertexTex2), vertices, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        else
        {
            glGenBuffersARB(1, &info.bufferId);
            glBindBufferARB(GL_ARRAY_BUFFER_ARB, info.bufferId);
            glBufferDataARB(GL_ARRAY_BUFFER_ARB, vertexCount * sizeof(VertexTex2), vertices, GL_STATIC_DRAW_ARB);
            glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        }

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

unsigned int CGLDevice::CreateStaticBuffer(PrimitiveType primitiveType, const VertexCol* vertices, int vertexCount)
{
    unsigned int id = 0;
    if (m_vboAvailable)
    {
        id = ++m_lastVboId;

        VboObjectInfo info;
        info.primitiveType = primitiveType;
        info.vertexType = VERTEX_TYPE_COL;
        info.vertexCount = vertexCount;
        info.bufferId = 0;

        if(m_vertexBufferType == VBT_VBO_CORE)
        {
            glGenBuffers(1, &info.bufferId);
            glBindBuffer(GL_ARRAY_BUFFER, info.bufferId);
            glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(VertexCol), vertices, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        else
        {
            glGenBuffersARB(1, &info.bufferId);
            glBindBufferARB(GL_ARRAY_BUFFER_ARB, info.bufferId);
            glBufferDataARB(GL_ARRAY_BUFFER_ARB, vertexCount * sizeof(VertexCol), vertices, GL_STATIC_DRAW_ARB);
            glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        }

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

void CGLDevice::UpdateStaticBuffer(unsigned int bufferId, PrimitiveType primitiveType, const Vertex* vertices, int vertexCount)
{
    if (m_vboAvailable)
    {
        auto it = m_vboObjects.find(bufferId);
        if (it == m_vboObjects.end())
            return;

        VboObjectInfo& info = (*it).second;
        info.primitiveType = primitiveType;
        info.vertexType = VERTEX_TYPE_NORMAL;
        info.vertexCount = vertexCount;

        if(m_vertexBufferType == VBT_VBO_CORE)
        {
            glBindBuffer(GL_ARRAY_BUFFER, info.bufferId);
            glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(Vertex), vertices, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        else
        {
            glBindBufferARB(GL_ARRAY_BUFFER_ARB, info.bufferId);
            glBufferDataARB(GL_ARRAY_BUFFER_ARB, vertexCount * sizeof(Vertex), vertices, GL_STATIC_DRAW_ARB);
            glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        }
    }
    else
    {
        glNewList(bufferId, GL_COMPILE);

        DrawPrimitive(primitiveType, vertices, vertexCount);

        glEndList();
    }
}

void CGLDevice::UpdateStaticBuffer(unsigned int bufferId, PrimitiveType primitiveType, const VertexTex2* vertices, int vertexCount)
{
    if (m_vboAvailable)
    {
        auto it = m_vboObjects.find(bufferId);
        if (it == m_vboObjects.end())
            return;

        VboObjectInfo& info = (*it).second;
        info.primitiveType = primitiveType;
        info.vertexType = VERTEX_TYPE_TEX2;
        info.vertexCount = vertexCount;

        if(m_vertexBufferType == VBT_VBO_CORE)
        {
            glBindBuffer(GL_ARRAY_BUFFER, info.bufferId);
            glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(VertexTex2), vertices, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        else
        {
            glBindBufferARB(GL_ARRAY_BUFFER_ARB, info.bufferId);
            glBufferDataARB(GL_ARRAY_BUFFER_ARB, vertexCount * sizeof(VertexTex2), vertices, GL_STATIC_DRAW_ARB);
            glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        }
    }
    else
    {
        glNewList(bufferId, GL_COMPILE);

        DrawPrimitive(primitiveType, vertices, vertexCount);

        glEndList();
    }
}

void CGLDevice::UpdateStaticBuffer(unsigned int bufferId, PrimitiveType primitiveType, const VertexCol* vertices, int vertexCount)
{
    if (m_vboAvailable)
    {
        auto it = m_vboObjects.find(bufferId);
        if (it == m_vboObjects.end())
            return;

        VboObjectInfo& info = (*it).second;
        info.primitiveType = primitiveType;
        info.vertexType = VERTEX_TYPE_COL;
        info.vertexCount = vertexCount;

        if(m_vertexBufferType == VBT_VBO_CORE)
        {
            glBindBuffer(GL_ARRAY_BUFFER, info.bufferId);
            glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(VertexCol), vertices, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        else
        {
            glBindBufferARB(GL_ARRAY_BUFFER_ARB, info.bufferId);
            glBufferDataARB(GL_ARRAY_BUFFER_ARB, vertexCount * sizeof(VertexCol), vertices, GL_STATIC_DRAW_ARB);
            glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        }
    }
    else
    {
        glNewList(bufferId, GL_COMPILE);

        DrawPrimitive(primitiveType, vertices, vertexCount);

        glEndList();
    }
}

void CGLDevice::DrawStaticBuffer(unsigned int bufferId)
{
    if (m_vboAvailable)
    {
        auto it = m_vboObjects.find(bufferId);
        if (it == m_vboObjects.end())
            return;

        glEnable(GL_VERTEX_ARRAY);

        if(m_vertexBufferType == VBT_VBO_CORE)
            glBindBuffer(GL_ARRAY_BUFFER, (*it).second.bufferId);
        else
            glBindBufferARB(GL_ARRAY_BUFFER_ARB, (*it).second.bufferId);

        if ((*it).second.vertexType == VERTEX_TYPE_NORMAL)
        {
            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(3, GL_FLOAT, sizeof(Vertex), static_cast<char*>(nullptr) + offsetof(Vertex, coord));

            glEnableClientState(GL_NORMAL_ARRAY);
            glNormalPointer(GL_FLOAT, sizeof(Vertex), static_cast<char*>(nullptr) + offsetof(Vertex, normal));

            if (m_multitextureAvailable)
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

            if (m_multitextureAvailable)
                glClientActiveTexture(GL_TEXTURE0);

            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(2, GL_FLOAT, sizeof(VertexTex2), static_cast<char*>(nullptr) + offsetof(VertexTex2, texCoord));

            if (m_multitextureAvailable)
            {
                glClientActiveTexture(GL_TEXTURE1);
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                glTexCoordPointer(2, GL_FLOAT, sizeof(VertexTex2), static_cast<char*>(nullptr) + offsetof(VertexTex2, texCoord2));
            }
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
            if (m_multitextureAvailable)
            {
                glClientActiveTexture(GL_TEXTURE0);
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            }
        }
        else if ((*it).second.vertexType == VERTEX_TYPE_COL)
        {
            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_COLOR_ARRAY);
        }

        if(m_vertexBufferType == VBT_VBO_CORE)
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        else
            glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

        glDisable(GL_VERTEX_ARRAY);
    }
    else
    {
        glCallList(bufferId);
    }
}

void CGLDevice::DestroyStaticBuffer(unsigned int bufferId)
{
    if (m_vboAvailable)
    {
        auto it = m_vboObjects.find(bufferId);
        if (it == m_vboObjects.end())
            return;

        if(m_vertexBufferType == VBT_VBO_CORE)
            glDeleteBuffers(1, &(*it).second.bufferId);
        else
            glDeleteBuffersARB(1, &(*it).second.bufferId);

        m_vboObjects.erase(it);
    }
    else
    {
        glDeleteLists(bufferId, 1);
    }
}

/* Based on libwine's implementation */

int CGLDevice::ComputeSphereVisibility(const Math::Vector &center, float radius)
{
    Math::Matrix m;
    m = Math::MultiplyMatrices(m_worldMat, m);
    m = Math::MultiplyMatrices(m_viewMat, m);
    Math::Matrix sc;
    Math::LoadScaleMatrix(sc, Math::Vector(1.0f, 1.0f, -1.0f));
    m = Math::MultiplyMatrices(sc, m);
    m = Math::MultiplyMatrices(m_projectionMat, m);

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

void CGLDevice::SetViewport(int x, int y, int width, int height)
{
    glViewport(x, y, width, height);
}

void CGLDevice::SetRenderState(RenderState state, bool enabled)
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
            for (int index = 0; index < static_cast<int>( m_lights.size() ); ++index)
                UpdateLightPosition(index);
        }

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

void CGLDevice::SetColorMask(bool red, bool green, bool blue, bool alpha)
{
    glColorMask(red, green, blue, alpha);
}

void CGLDevice::SetDepthTestFunc(CompFunc func)
{
    glDepthFunc(TranslateGfxCompFunc(func));
}

void CGLDevice::SetDepthBias(float factor, float units)
{
    glPolygonOffset(factor, units);
}

void CGLDevice::SetAlphaTestFunc(CompFunc func, float refValue)
{
    glAlphaFunc(TranslateGfxCompFunc(func), refValue);
}

void CGLDevice::SetBlendFunc(BlendFunc srcBlend, BlendFunc dstBlend)
{
    glBlendFunc(TranslateGfxBlendFunc(srcBlend), TranslateGfxBlendFunc(dstBlend));
}

void CGLDevice::SetClearColor(const Color &color)
{
    glClearColor(color.r, color.g, color.b, color.a);
}

void CGLDevice::SetGlobalAmbient(const Color &color)
{
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, color.Array());
}

void CGLDevice::SetFogParams(FogMode mode, const Color &color, float start, float end, float density)
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

void CGLDevice::SetCullMode(CullMode mode)
{
    // Cull clockwise back faces, so front face is the opposite
    // (assuming GL_CULL_FACE is GL_BACK)
    if      (mode == CULL_CW ) glFrontFace(GL_CCW);
    else if (mode == CULL_CCW) glFrontFace(GL_CW);
    else assert(false);
}

void CGLDevice::SetShadeModel(ShadeModel model)
{
    if      (model == SHADE_FLAT)   glShadeModel(GL_FLAT);
    else if (model == SHADE_SMOOTH) glShadeModel(GL_SMOOTH);
    else  assert(false);
}

void CGLDevice::SetShadowColor(float value)
{
    // doesn't do anything because it can't
}

void CGLDevice::SetFillMode(FillMode mode)
{
    if      (mode == FILL_POINT) glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    else if (mode == FILL_LINES) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else if (mode == FILL_POLY)  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else assert(false);
}

void CGLDevice::CopyFramebufferToTexture(Texture& texture, int xOffset, int yOffset, int x, int y, int width, int height)
{
    if (texture.id == 0) return;

    // Use & enable 1st texture stage
    if (m_multitextureAvailable)
        glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, texture.id);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, xOffset, yOffset, x, y, width, height);

    // Restore previous texture
    glBindTexture(GL_TEXTURE_2D, m_currentTextures[0].id);
}

std::unique_ptr<CFrameBufferPixels> CGLDevice::GetFrameBufferPixels() const
{
    return GetGLFrameBufferPixels(m_config.size);
}

CFramebuffer* CGLDevice::GetFramebuffer(std::string name)
{
    auto it = m_framebuffers.find(name);
    if (it == m_framebuffers.end())
        return nullptr;

    return it->second.get();
}

CFramebuffer* CGLDevice::CreateFramebuffer(std::string name, const FramebufferParams& params)
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

    framebuffer->Create();

    CFramebuffer* framebufferPtr = framebuffer.get();
    m_framebuffers[name] = std::move(framebuffer);
    return framebufferPtr;
}

void CGLDevice::DeleteFramebuffer(std::string name)
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

bool CGLDevice::IsAnisotropySupported()
{
    return m_anisotropyAvailable;
}

int CGLDevice::GetMaxAnisotropyLevel()
{
    return m_maxAnisotropy;
}

int CGLDevice::GetMaxSamples()
{
    return m_maxSamples;
}

bool CGLDevice::IsShadowMappingSupported()
{
    return m_shadowMappingSupport != SMS_NONE;
}

int CGLDevice::GetMaxTextureSize()
{
    int value;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &value);
    return value;
}

bool CGLDevice::IsFramebufferSupported()
{
    return m_framebufferSupport != FBS_NONE;
}

} // namespace Gfx
