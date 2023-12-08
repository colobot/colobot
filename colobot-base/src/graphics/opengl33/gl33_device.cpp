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

#include "graphics/opengl33/gl33_device.h"

#include "graphics/opengl33/gl33_object_renderer.h"
#include "graphics/opengl33/gl33_particle_renderer.h"
#include "graphics/opengl33/gl33_shadow_renderer.h"
#include "graphics/opengl33/gl33_terrain_renderer.h"
#include "graphics/opengl33/gl33_ui_renderer.h"
#include "graphics/opengl33/glframebuffer.h"

#include "common/config.h"

#include "common/config_file.h"
#include "common/image.h"
#include "common/logger.h"
#include "common/version.h"

#include "graphics/core/light.h"
#include "graphics/core/material.h"
#include "graphics/core/transparency.h"

#include "graphics/engine/engine.h"

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

std::string CGL33Device::GetName()
{
    return std::string("OpenGL 3.3");
}

bool CGL33Device::Create()
{
    GetLogger()->Info("Creating CDevice - OpenGL 3.3");

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
        GetLogger()->Error("Unsupported OpenGL version: %%.%%", glMajor, glMinor);
        GetLogger()->Error("OpenGL 3.2 or newer is required to use this engine.");
        m_errorMessage = "It seems your graphics card does not support OpenGL 3.2.";
        m_errorMessage += "Please make sure you have appropriate hardware and newest drivers installed.";
        m_errorMessage += "(OpenGL 3.2 is roughly equivalent to Direct3D 10)";
        m_errorMessage += GetHardwareInfo();
        return false;
    }
    else if (glVersion < 33)
    {
        GetLogger()->Warn("Partially supported OpenGL version: %%.%%", glMajor, glMinor);
        GetLogger()->Warn("You may experience problems while running the game on this engine.");
        GetLogger()->Warn("OpenGL 3.3 or newer is recommended.");
    }
    else
    {
        const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
        const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));

        GetLogger()->Info("OpenGL %%", version);
        GetLogger()->Info("%%", renderer);
    }

    // Detect support of anisotropic filtering
    m_capabilities.anisotropySupported = AreExtensionsSupported("GL_EXT_texture_filter_anisotropic");
    if (m_capabilities.anisotropySupported)
    {
        // Obtain maximum anisotropy level available
        float level;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &level);
        m_capabilities.maxAnisotropy = static_cast<int>(level);

        GetLogger()->Info("Anisotropic filtering available");
        GetLogger()->Info("Maximum anisotropy: %%", m_capabilities.maxAnisotropy);
    }
    else
    {
        GetLogger()->Info("Anisotropic filtering not available");
    }

    m_capabilities.multisamplingSupported = true;
    glGetIntegerv(GL_MAX_SAMPLES, &m_capabilities.maxSamples);
    GetLogger()->Info("Multisampling supported, max samples: %%", m_capabilities.maxSamples);

    // Set just to be sure
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glViewport(0, 0, m_config.size.x, m_config.size.y);

    // this is set in shader
    m_capabilities.maxLights = 4;

    int maxTextures = 0;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextures);
    GetLogger()->Info("Maximum texture image units: %%", maxTextures);

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_capabilities.maxTextureSize);
    GetLogger()->Info("Maximum texture size: %%", m_capabilities.maxTextureSize);

    m_capabilities.multitexturingSupported = true;
    m_capabilities.maxTextures = maxTextures;

    m_capabilities.shadowMappingSupported = true;

    m_capabilities.framebufferSupported = true;
    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &m_capabilities.maxRenderbufferSize);
    GetLogger()->Info("Maximum renderbuffer size: %%", m_capabilities.maxRenderbufferSize);

    m_uiRenderer = std::make_unique<CGL33UIRenderer>(this);
    m_terrainRenderer = std::make_unique<CGL33TerrainRenderer>(this);
    m_objectRenderer = std::make_unique<CGL33ObjectRenderer>(this);
    m_particleRenderer = std::make_unique<CGL33ParticleRenderer>(this);
    m_shadowRenderer = std::make_unique<CGL33ShadowRenderer>(this);

    // create default framebuffer object
    FramebufferParams framebufferParams;

    framebufferParams.width = m_config.size.x;
    framebufferParams.height = m_config.size.y;
    framebufferParams.depth = m_config.depthSize;

    m_framebuffers["default"] = std::make_unique<CDefaultFramebuffer>(framebufferParams);

    GetLogger()->Info("CDevice created successfully");

    return true;
}

void CGL33Device::Destroy()
{
    // delete shader program
    glUseProgram(0);

    // delete framebuffers
    for (auto& framebuffer : m_framebuffers)
        framebuffer.second->Destroy();

    m_framebuffers.clear();

    // Delete the remaining textures
    // Should not be strictly necessary, but just in case
    DestroyAllTextures();

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
    glViewport(0, 0, m_config.size.x, m_config.size.y);

    // create default framebuffer object
    FramebufferParams framebufferParams;

    framebufferParams.width = m_config.size.x;
    framebufferParams.height = m_config.size.y;
    framebufferParams.depth = m_config.depthSize;

    m_framebuffers["default"] = std::make_unique<CDefaultFramebuffer>(framebufferParams);
}

void CGL33Device::BeginScene()
{
    Clear();

    glDisable(GL_BLEND);
    m_transparency = TransparencyMode::NONE;

    glDisable(GL_DEPTH_TEST);
    m_depthTest = false;

    glDepthMask(GL_TRUE);
    m_depthMask = true;

    glFrontFace(GL_CW);   // Colobot issue: faces are reversed
    m_cullFace = CullFace::NONE;
    glDisable(GL_CULL_FACE);
}

void CGL33Device::EndScene()
{
    if constexpr (Version::DEVELOPMENT_BUILD)
        CheckGLErrors();
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

/** If image is invalid, returns invalid texture.
    Otherwise, returns pointer to new Texture struct.
    This struct must not be deleted in other way than through DeleteTexture() */
Texture CGL33Device::CreateTexture(CImage *image, const TextureCreateParams &params)
{
    ImageData *data = image->GetData();
    if (data == nullptr)
    {
        GetLogger()->Error("Invalid texture data");
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

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, result.id);

    // Set texture parameters
    GLint minF = GL_NEAREST, magF = GL_NEAREST;
    int mipmapLevel = 1;

    switch (params.filter)
    {
    case TextureFilter::NEAREST:
        minF = GL_NEAREST;
        magF = GL_NEAREST;
        break;
    case TextureFilter::BILINEAR:
        minF = GL_LINEAR;
        magF = GL_LINEAR;
        break;
    case TextureFilter::TRILINEAR:
        minF = GL_LINEAR_MIPMAP_LINEAR;
        magF = GL_LINEAR;
        mipmapLevel = CEngine::GetInstance().GetTextureMipmapLevel();
        break;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minF);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magF);

    GLint wrap = GL_REPEAT;

    switch (params.wrap)
    {
    case TextureWrapMode::REPEAT:
        wrap = GL_REPEAT;
        break;
    case TextureWrapMode::CLAMP:
        wrap = GL_CLAMP;
        break;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

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

    glActiveTexture(GL_TEXTURE3);
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

void CGL33Device::UpdateTexture(const Texture& texture, const glm::ivec2& offset, ImageData* data, TextureFormat format)
{
    if (texture.id == 0) return;

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, texture.id);

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
    for (int index = 0; index < 32; ++index)
    {
        glActiveTexture(GL_TEXTURE0 + index);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    for (auto it = m_allTextures.begin(); it != m_allTextures.end(); ++it)
        glDeleteTextures(1, &(*it).id);

    m_allTextures.clear();
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

void CGL33Device::SetDepthTest(bool enabled)
{
    if (m_depthTest == enabled) return;

    m_depthTest = enabled;

    if (enabled)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
}

void CGL33Device::SetDepthMask(bool enabled)
{
    if (m_depthMask == enabled) return;

    m_depthMask = enabled;

    glDepthMask(enabled ? GL_TRUE : GL_FALSE);
}

void CGL33Device::SetCullFace(CullFace mode)
{
    if (m_cullFace == mode) return;

    m_cullFace = mode;

    switch (mode)
    {
    case CullFace::NONE:
        glDisable(GL_CULL_FACE);
        break;
    case CullFace::BACK:
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        break;
    case CullFace::FRONT:
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        break;
    case CullFace::BOTH:
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT_AND_BACK);
        break;
    }
}

void CGL33Device::SetTransparency(TransparencyMode mode)
{
    if (m_transparency == mode) return;

    m_transparency = mode;

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

void CGL33Device::SetColorMask(bool red, bool green, bool blue, bool alpha)
{
    glColorMask(red, green, blue, alpha);
}

void CGL33Device::SetClearColor(const Color &color)
{
    glClearColor(color.r, color.g, color.b, color.a);
}

void CGL33Device::CopyFramebufferToTexture(Texture& texture, int xOffset, int yOffset, int x, int y, int width, int height)
{
    if (texture.id == 0) return;

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, texture.id);

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

    auto framebuffer = std::make_unique<CGLFramebuffer>(params);
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
