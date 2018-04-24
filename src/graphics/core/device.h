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

/**
 * \file graphics/core/device.h
 * \brief Abstract graphics device - CDevice class and related structs/enums
 */

#pragma once

#include "graphics/core/color.h"
#include "graphics/core/texture.h"
#include "graphics/core/vertex.h"

#include "math/intpoint.h"

#include <memory>
#include <string>


class CImage;
struct ImageData;

namespace Math
{
struct Matrix;
struct Vector;
} // namespace Math


// Graphics module namespace
namespace Gfx
{

class CFramebuffer;
struct FramebufferParams;
struct Light;
struct Material;
struct Vertex;
struct VertexCol;
struct VertexTex2;

/**
 * \struct DeviceConfig
 * \brief General config for graphics device
 *
 * These settings are common window options set by SDL.
 */
struct DeviceConfig
{
    //! Screen size
    Math::IntPoint size = Math::IntPoint(800, 600);
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
 * \enum TextureUnit
 * \brief Texture unit values for binding textures
 *
 * These enums should be used for indexing textures instead of raw integers.
 */
enum TextureUnit
{
    TEXTURE_PRIMARY = 0,
    TEXTURE_SECONDARY = 1,
    TEXTURE_SHADOW = 2,
};

/**
 * \enum TransformType
 * \brief Type of transformation in rendering pipeline
 */
enum TransformType
{
    TRANSFORM_WORLD,
    TRANSFORM_VIEW,
    TRANSFORM_PROJECTION,
    TRANSFORM_SHADOW
};

/**
 * \enum RenderState
 * \brief Render states that can be enabled/disabled
 */
enum RenderState
{
    RENDER_STATE_LIGHTING,
    RENDER_STATE_BLENDING,
    RENDER_STATE_FOG,
    RENDER_STATE_DEPTH_TEST,
    RENDER_STATE_DEPTH_WRITE,
    RENDER_STATE_ALPHA_TEST,
    RENDER_STATE_CULLING,
    RENDER_STATE_DEPTH_BIAS,
    RENDER_STATE_SHADOW_MAPPING,
};

/**
* \enum RenderMode
* \brief Render modes the graphics device can be in
*/
enum RenderMode
{
    RENDER_MODE_NORMAL,
    RENDER_MODE_INTERFACE,
    RENDER_MODE_SHADOW,
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
 * \enum BlendFunc
 * \brief Type of blending function
 */
enum BlendFunc
{
    BLEND_ZERO,
    BLEND_ONE,
    BLEND_SRC_COLOR,
    BLEND_INV_SRC_COLOR,
    BLEND_DST_COLOR,
    BLEND_INV_DST_COLOR,
    BLEND_SRC_ALPHA,
    BLEND_INV_SRC_ALPHA,
    BLEND_DST_ALPHA,
    BLEND_INV_DST_ALPHA,
    BLEND_SRC_ALPHA_SATURATE
};

/**
 * \enum FogMode
 * \brief Type of fog calculation function
 */
enum FogMode
{
    FOG_LINEAR,
    FOG_EXP,
    FOG_EXP2
};

/**
 * \enum CullMode
 * \brief Culling mode for polygons
 */
enum CullMode
{
    //! Cull clockwise faces
    CULL_CW,
    //! Cull counter-clockwise faces
    CULL_CCW
};

/**
 * \enum ShadeModel
 * \brief Shade model used in rendering
 */
enum ShadeModel
{
    SHADE_FLAT,
    SHADE_SMOOTH
};

/**
 * \enum FillMode
 * \brief Polygon fill mode
 */
enum FillMode
{
    //! Draw only points
    FILL_POINT,
    //! Draw only lines
    FILL_LINES,
    //! Draw full polygons
    FILL_POLY
};

/**
 * \enum PrimitiveType
 * \brief Type of primitive to render
 */
enum PrimitiveType
{
    PRIMITIVE_POINTS,
    PRIMITIVE_LINES,
    PRIMITIVE_LINE_STRIP,
    PRIMITIVE_LINE_LOOP,
    PRIMITIVE_TRIANGLES,
    PRIMITIVE_TRIANGLE_STRIP,
    PRIMITIVE_TRIANGLE_FAN
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

    //! Provides a hook to debug graphics code (implementation-specific)
    virtual void DebugHook() = 0;

    //! Displays light positions to aid in debuggings
    virtual void DebugLights() = 0;

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

    //! Sets current rendering mode
    virtual void SetRenderMode(RenderMode mode) = 0;

    //! Sets the transform matrix of given type
    virtual void SetTransform(TransformType type, const Math::Matrix &matrix) = 0;

    //! Sets the current material
    virtual void SetMaterial(const Material &material) = 0;

    //! Returns the maximum number of lights available
    virtual int GetMaxLightCount() = 0;
    //! Sets the light at given index
    virtual void SetLight(int index, const Light &light) = 0;
    //! Enables/disables the light at given index
    virtual void SetLightEnabled(int index, bool enabled) = 0;

    //! Creates a texture from image; the image can be safely removed after that
    virtual Texture CreateTexture(CImage *image, const TextureCreateParams &params) = 0;
    //! Creates a texture from raw image data; image data can be freed after that
    virtual Texture CreateTexture(ImageData *data, const TextureCreateParams &params) = 0;
    //! Creates a depth texture with specific dimensions and depth
    virtual Texture CreateDepthTexture(int width, int height, int depth) = 0;
    //! Updates a part of texture from raw image data
    virtual void UpdateTexture(const Texture& texture, Math::IntPoint offset, ImageData* data, TexImgFormat format) = 0;
    //! Deletes a given texture, freeing it from video memory
    virtual void DestroyTexture(const Texture &texture) = 0;
    //! Deletes all textures created so far
    virtual void DestroyAllTextures() = 0;

    //! Returns the maximum number of multitexture stages
    virtual int GetMaxTextureStageCount() = 0;
    //! Sets the texture at given texture stage
    virtual void SetTexture(int index, const Texture &texture) = 0;
    //! Sets the texture image by ID at given texture stage
    virtual void SetTexture(int index, unsigned int textureId) = 0;
    //! Enables/disables the given texture stage
    virtual void SetTextureEnabled(int index, bool enabled) = 0;

    //! Sets the params for texture stage with given index
    virtual void SetTextureStageParams(int index, const TextureStageParams &params) = 0;

    //! Sets only the texture wrap modes (for faster than thru stage params)
    virtual void SetTextureStageWrap(int index, TexWrapMode wrapS, TexWrapMode wrapT) = 0;

    //! Renders primitive composed of generic vertices
    virtual void DrawPrimitive(PrimitiveType type, const void *vertices,
        int size, const VertexFormat &format, int vertexCount) = 0;

    //! Renders multiple primitives composed of generic vertices
    virtual void DrawPrimitives(PrimitiveType type, const void *vertices,
        int size, const VertexFormat &format, int first[], int count[], int drawCount) = 0;

    //! Renders primitive composed of vertices with single texture
    virtual void DrawPrimitive(PrimitiveType type, const Vertex *vertices    , int vertexCount,
                               Color color = Color(1.0f, 1.0f, 1.0f, 1.0f)) = 0;
    //! Renders primitive composed of vertices with multitexturing (2 textures)
    virtual void DrawPrimitive(PrimitiveType type, const VertexTex2 *vertices, int vertexCount,
                               Color color = Color(1.0f, 1.0f, 1.0f, 1.0f)) = 0;
    //! Renders primitive composed of vertices with solid color
    virtual void DrawPrimitive(PrimitiveType type, const VertexCol *vertices , int vertexCount) = 0;

    //! Renders primitives composed of lists of vertices with single texture
    virtual void DrawPrimitives(PrimitiveType type, const Vertex *vertices,
        int first[], int count[], int drawCount,
        Color color = Color(1.0f, 1.0f, 1.0f, 1.0f)) = 0;
    //! Renders primitives composed of lists of vertices with multitexturing (2 textures)
    virtual void DrawPrimitives(PrimitiveType type, const VertexTex2 *vertices,
        int first[], int count[], int drawCount,
        Color color = Color(1.0f, 1.0f, 1.0f, 1.0f)) = 0;
    //! Renders primitives composed of lists of vertices with solid color
    virtual void DrawPrimitives(PrimitiveType type, const VertexCol *vertices,
        int first[], int count[], int drawCount) = 0;

    //! Creates a static buffer composed of given primitives with single texture vertices
    virtual unsigned int CreateStaticBuffer(PrimitiveType primitiveType, const Vertex* vertices, int vertexCount) = 0;

    //! Creates a static buffer composed of given primitives with multitexturing
    virtual unsigned int CreateStaticBuffer(PrimitiveType primitiveType, const VertexTex2* vertices, int vertexCount) = 0;

    //! Creates a static buffer composed of given primitives with solid color
    virtual unsigned int CreateStaticBuffer(PrimitiveType primitiveType, const VertexCol* vertices, int vertexCount) = 0;

    //! Updates the static buffer composed of given primitives with single texture vertices
    virtual void UpdateStaticBuffer(unsigned int bufferId, PrimitiveType primitiveType, const Vertex* vertices, int vertexCount) = 0;

    //! Updates the static buffer composed of given primitives with multitexturing
    virtual void UpdateStaticBuffer(unsigned int bufferId, PrimitiveType primitiveType, const VertexTex2* vertices, int vertexCount) = 0;

    //! Updates the static buffer composed of given primitives with solid color
    virtual void UpdateStaticBuffer(unsigned int bufferId, PrimitiveType primitiveType, const VertexCol* vertices, int vertexCount) = 0;

    //! Draws a static buffer
    virtual void DrawStaticBuffer(unsigned int bufferId) = 0;

    //! Deletes a static buffer
    virtual void DestroyStaticBuffer(unsigned int bufferId) = 0;

    //! Tests whether a sphere is (partially) within the frustum volume
    //! Returns a mask of frustum planes for which the test is positive
    virtual int ComputeSphereVisibility(const Math::Vector &center, float radius) = 0;

    //! Changes rendering viewport
    virtual void SetViewport(int x, int y, int width, int height) = 0;

    //! Enables/disables the given render state
    virtual void SetRenderState(RenderState state, bool enabled) = 0;

    //! Sets the color mask
    virtual void SetColorMask(bool red, bool green, bool blue, bool alpha) = 0;

    //! Sets the function of depth test
    virtual void SetDepthTestFunc(CompFunc func) = 0;

    //! Sets the depth bias (constant value added to Z-coords)
    virtual void SetDepthBias(float factor, float units) = 0;

    //! Sets the alpha test function and reference value
    virtual void SetAlphaTestFunc(CompFunc func, float refValue) = 0;

    //! Sets the blending functions for source and destination operations
    virtual void SetBlendFunc(BlendFunc srcBlend, BlendFunc dstBlend) = 0;

    //! Sets the clear color
    virtual void SetClearColor(const Color &color) = 0;

    //! Sets the global ambient color
    virtual void SetGlobalAmbient(const Color &color) = 0;

    //! Sets the fog parameters: mode, color, start distance, end distance and density (for exp models)
    virtual void SetFogParams(FogMode mode, const Color &color, float start, float end, float density) = 0;

    //! Sets the current cull mode
    virtual void SetCullMode(CullMode mode) = 0;

    //! Sets the shade model
    virtual void SetShadeModel(ShadeModel model) = 0;

    //! Sets shadow color
    virtual void SetShadowColor(float value) = 0;

    //! Sets the current fill mode
    virtual void SetFillMode(FillMode mode) = 0;

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
