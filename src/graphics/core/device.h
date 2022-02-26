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

/**
 * \file graphics/core/device.h
 * \brief Abstract graphics device - CDevice class and related structs/enums
 */

#pragma once

#include "graphics/core/color.h"
#include "graphics/core/texture.h"
#include "graphics/core/vertex.h"

#include <glm/glm.hpp>

#include <memory>
#include <string>
#include <vector>


class CImage;
struct ImageData;


// Graphics module namespace
namespace Gfx
{

class CFramebuffer;
class CUIRenderer;
class CTerrainRenderer;
class CShadowRenderer;
class CObjectRenderer;
class CParticleRenderer;

struct FramebufferParams;
struct Light;
struct Material;
struct Vertex;
struct VertexCol;
struct Vertex3D;

enum class CullFace : unsigned char;
enum class TransparencyMode : unsigned char;

/**
 * \struct DeviceConfig
 * \brief General config for graphics device
 *
 * These settings are common window options set by SDL.
 */
struct DeviceConfig
{
    //! Screen size
    glm::ivec2 size = { 800, 600 };
    //! Bits per pixel
    int bpp = 32;
    //! Full screen
    bool fullScreen = false;
    //! Resizeable window
    bool resizeable = true;
    //! Double buffering
    bool doubleBuf = true;
    //! No window frame (also set with full screen)
    bool noFrame = false;

    //! Size of red channel in bits
    int redSize = 8;
    //! Size of green channel in bits
    int greenSize = 8;
    //! Size of blue channel in bits
    int blueSize = 8;
    //! Size of alpha channel in bits
    int alphaSize = 8;
    //! Color depth in bits
    int depthSize = 24;
    //! Stencil depth in bits
    int stencilSize = 8;

    //! Force hardware acceleration (video mode set will fail on lack of hw accel)
    bool hardwareAccel = true;

    //! Loads the default values
    void LoadDefault()
    {
        *this = DeviceConfig();
    }
};

/**
* \struct DeviceCapabilities
* \brief This structs contains various capabilities of graphics device
*/
struct DeviceCapabilities
{
    bool multitexturingSupported = false;
    int maxTextures = 1;
    int maxTextureSize = 1024;

    int maxLights = 8;

    bool shadowMappingSupported = false;

    bool framebufferSupported = false;
    int maxRenderbufferSize = 0;

    bool anisotropySupported = false;
    int maxAnisotropy = 1;

    bool multisamplingSupported = false;
    int maxSamples = 1;
};

/**
 * \enum CompFunc
 * \brief Type of function used to compare values
 */
enum CompFunc
{
    COMP_FUNC_NEVER,
    COMP_FUNC_LESS,
    COMP_FUNC_EQUAL,
    COMP_FUNC_NOTEQUAL,
    COMP_FUNC_LEQUAL,
    COMP_FUNC_GREATER,
    COMP_FUNC_GEQUAL,
    COMP_FUNC_ALWAYS
};

/**
 * \enum FillMode
 * \brief Polygon fill mode
 */
enum class FillMode : unsigned char
{
    //! Draw only points
    POINT,
    //! Draw only lines
    LINES,
    //! Draw full polygons
    POLY
};

/**
 * \enum PrimitiveType
 * \brief Type of primitive to render
 */
enum class PrimitiveType : unsigned char
{
    POINTS,
    LINES,
    LINE_STRIP,
    LINE_LOOP,
    TRIANGLES,
    TRIANGLE_STRIP,
    TRIANGLE_FAN
};

/**
 * \enum FrustumPlane
 * \brief Planes of frustum space
 *
 * Bitset of flags - can be OR'd together.
 */
enum FrustumPlane
{
    FRUSTUM_PLANE_LEFT   = 0x01,
    FRUSTUM_PLANE_RIGHT  = 0x02,
    FRUSTUM_PLANE_TOP    = 0x04,
    FRUSTUM_PLANE_BOTTOM = 0x08,
    FRUSTUM_PLANE_FRONT  = 0x10,
    FRUSTUM_PLANE_BACK   = 0x20,
    FRUSTUM_PLANE_ALL = FRUSTUM_PLANE_LEFT   | FRUSTUM_PLANE_RIGHT  |
                        FRUSTUM_PLANE_TOP    | FRUSTUM_PLANE_BOTTOM |
                        FRUSTUM_PLANE_FRONT  | FRUSTUM_PLANE_BACK
};

/**
 * \enum RenderTarget
 * \brief Render targets for rendering to textures
 */
enum RenderTarget
{
    RENDER_TARGET_COLOR,
    RENDER_TARGET_DEPTH,
    RENDER_TARGET_STENCIL
};

class CFrameBufferPixels
{
public:
    virtual ~CFrameBufferPixels() {}

    virtual void* GetPixelsData() = 0;
};

class CVertexBuffer
{
protected:
    PrimitiveType m_type;
    std::vector<Vertex3D> m_data;

public:
    CVertexBuffer(PrimitiveType type, size_t size)
        : m_type(type), m_data(size, Vertex3D{})
    {

    }

    virtual ~CVertexBuffer()
    {

    }

    virtual void Update() = 0;

    PrimitiveType GetType() const
    {
        return m_type;
    }

    void SetType(PrimitiveType type)
    {
        m_type = type;
    }

    size_t Size() const
    {
        return m_data.size();
    }

    void Resize(size_t size)
    {
        m_data.resize(size);
    }

    Vertex3D& operator[](size_t index)
    {
        return m_data[index];
    }

    const Vertex3D& operator[](size_t index) const
    {
        return m_data[index];
    }

    void SetData(const Vertex3D* data, size_t offset, size_t count)
    {
        std::copy(data, data + count, m_data.data() + offset);
    }

    auto Data()
    {
        return m_data.data();
    }

    auto Data() const
    {
        return m_data.data();
    }

    auto begin()
    {
        return m_data.begin();
    }

    auto end()
    {
        return m_data.end();
    }

    auto begin() const
    {
        return m_data.begin();
    }

    auto end() const
    {
        return m_data.end();
    }
};

/**
 * \class CDevice
 * \brief Abstract interface of graphics device
 *
 * It is based on DIRECT3DDEVICE class from DirectX to make it easier to port existing code.
 * It encapsulates the general graphics device state and provides a common interface
 * to graphics-specific functions which will be used throughout the program,
 * both in CEngine class and in UI classes. Note that it doesn't contain all functions from DirectX,
 * only those that were used in old code.
 *
 */
class CDevice
{
protected:
    std::string m_errorMessage;

    //! Capabilities of this device
    //! Should only be changed by code in concrete device implementation
    DeviceCapabilities m_capabilities;

public:
    virtual ~CDevice() {}

    //! Returns last error message or empty string
    inline std::string GetError()
    {
        return m_errorMessage;
    }

    //! Returns device capabilities
    const DeviceCapabilities& GetCapabilities()
    {
        return m_capabilities;
    }

    //! Returns a name of this device
    virtual std::string GetName() = 0;

    //! Initializes the device, setting the initial state
    virtual bool Create() = 0;
    //! Destroys the device, releasing every acquired resource
    virtual void Destroy() = 0;

    //! Changes configuration
    virtual void ConfigChanged(const DeviceConfig &newConfig) = 0;

    //! Begins drawing the 3D scene
    virtual void BeginScene() = 0;
    //! Ends drawing the 3D scene
    virtual void EndScene() = 0;

    //! Clears the screen to blank
    virtual void Clear() = 0;

    //! Returns UI renderer
    virtual CUIRenderer* GetUIRenderer() = 0;
    //! Returns terrain renderer
    virtual CTerrainRenderer* GetTerrainRenderer() = 0;
    //! Returns object renderer
    virtual CObjectRenderer* GetObjectRenderer() = 0;
    //! Returns particle renderer
    virtual CParticleRenderer* GetParticleRenderer() = 0;
    //! Returns shadow renderer
    virtual CShadowRenderer* GetShadowRenderer() = 0;

    //! Creates a texture from image; the image can be safely removed after that
    virtual Texture CreateTexture(CImage *image, const TextureCreateParams &params) = 0;
    //! Creates a texture from raw image data; image data can be freed after that
    virtual Texture CreateTexture(ImageData *data, const TextureCreateParams &params) = 0;
    //! Creates a depth texture with specific dimensions and depth
    virtual Texture CreateDepthTexture(int width, int height, int depth) = 0;
    //! Updates a part of texture from raw image data
    virtual void UpdateTexture(const Texture& texture, const glm::ivec2& offset, ImageData* data, TexImgFormat format) = 0;
    //! Deletes a given texture, freeing it from video memory
    virtual void DestroyTexture(const Texture &texture) = 0;
    //! Deletes all textures created so far
    virtual void DestroyAllTextures() = 0;

    virtual CVertexBuffer* CreateVertexBuffer(PrimitiveType primitiveType, const Vertex3D* vertices, int vertexCount) = 0;
    virtual void DestroyVertexBuffer(CVertexBuffer*) = 0;

    //! Changes rendering viewport
    virtual void SetViewport(int x, int y, int width, int height) = 0;

    //! Sets depth test
    virtual void SetDepthTest(bool enabled) = 0;
    //! Sets depth mask
    virtual void SetDepthMask(bool enabled) = 0;

    //! Sets which faces to cull
    virtual void SetCullFace(CullFace mode) = 0;

    //! Sets transparency mode
    virtual void SetTransparency(TransparencyMode mode) = 0;

    //! Sets the color mask
    virtual void SetColorMask(bool red, bool green, bool blue, bool alpha) = 0;

    //! Sets the clear color
    virtual void SetClearColor(const Color &color) = 0;

    //! Copies content of framebuffer to texture
    virtual void CopyFramebufferToTexture(Texture& texture, int xOffset, int yOffset, int x, int y, int width, int height) = 0;

    //! Returns the pixels of the entire screen
    virtual std::unique_ptr<CFrameBufferPixels> GetFrameBufferPixels() const = 0;

    //! Returns framebuffer with given name or nullptr if it doesn't exist
    virtual CFramebuffer* GetFramebuffer(std::string name) = 0;

    //! Creates new framebuffer with given name or nullptr if it's not possible
    virtual CFramebuffer* CreateFramebuffer(std::string name, const FramebufferParams& params) = 0;

    //! Deletes framebuffer
    virtual void DeleteFramebuffer(std::string name) = 0;

    //! Checks if anisotropy is supported
    virtual bool IsAnisotropySupported() = 0;

    //! Returns max anisotropy level supported
    virtual int GetMaxAnisotropyLevel() = 0;

    //! Returns max samples supported
    virtual int GetMaxSamples() = 0;

    //! Checks if shadow mapping is supported
    virtual bool IsShadowMappingSupported() = 0;

    //! Returns max texture size supported
    virtual int GetMaxTextureSize() = 0;

    //! Checks if framebuffers are supported
    virtual bool IsFramebufferSupported() = 0;
};


} // namespace Gfx
