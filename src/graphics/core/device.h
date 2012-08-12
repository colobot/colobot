// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
// * Copyright (C) 2012, Polish Portal of Colobot (PPC)
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program. If not, see  http://www.gnu.org/licenses/.

/**
 * \file graphics/core/device.h
 * \brief Abstract graphics device - Gfx::CDevice class and related structs/enums
 */

#pragma once


#include "graphics/core/color.h"
#include "graphics/core/light.h"
#include "graphics/core/material.h"
#include "graphics/core/texture.h"
#include "graphics/core/vertex.h"
#include "math/intpoint.h"
#include "math/matrix.h"

#include <string>


class CImage;
struct ImageData;


namespace Gfx {

/**
  \struct DeviceConfig
  \brief General config for graphics device

  These settings are common window options set by SDL.
*/
struct DeviceConfig
{
    //! Screen size
    Math::IntPoint size;
    //! Bits per pixel
    int bpp;
    //! Full screen
    bool fullScreen;
    //! Resizeable window
    bool resizeable;
    //! Double buffering
    bool doubleBuf;
    //! No window frame (also set with full screen)
    bool noFrame;

    //! Constructor calls LoadDefault()
    DeviceConfig() { LoadDefault(); }

    //! Loads the default values
    inline void LoadDefault()
    {
        size = Math::IntPoint(800, 600);
        bpp = 32;
        fullScreen = false;
        resizeable = false;
        doubleBuf = true;
        noFrame = false;
    }
};


/**
  \enum TransformType
  \brief Type of transformation in rendering pipeline

  These correspond to DirectX's three transformation matrices. */
enum TransformType
{
    TRANSFORM_WORLD,
    TRANSFORM_VIEW,
    TRANSFORM_PROJECTION
};

/**
  \enum RenderState
  \brief Render states that can be enabled/disabled */
enum RenderState
{
    RENDER_STATE_LIGHTING,
    RENDER_STATE_TEXTURING,
    RENDER_STATE_BLENDING,
    RENDER_STATE_FOG,
    RENDER_STATE_DEPTH_TEST,
    RENDER_STATE_DEPTH_WRITE,
    RENDER_STATE_ALPHA_TEST,
    RENDER_STATE_CULLING,
    RENDER_STATE_DITHERING
};

/**
  \enum CompFunc
  \brief Type of function used to compare values */
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
  \enum BlendFunc
  \brief Type of blending function */
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
  \enum FogMode
  \brief Type of fog calculation function */
enum FogMode
{
    FOG_LINEAR,
    FOG_EXP,
    FOG_EXP2
};

/**
  \enum CullMode
  \brief Culling mode for polygons */
enum CullMode
{
    //! Cull clockwise faces
    CULL_CW,
    //! Cull counter-clockwise faces
    CULL_CCW
};

/**
  \enum ShadeModel
  \brief Shade model used in rendering */
enum ShadeModel
{
    SHADE_FLAT,
    SHADE_SMOOTH
};

/**
  \enum FillMode
  \brief Polygon fill mode */
enum FillMode
{
    //! Draw only points
    FILL_POINT,
    //! Draw only lines
    FILL_LINES,
    //! Draw full polygons
    FILL_FILL
};

/**
  \enum PrimitiveType
  \brief Type of primitive to render */
enum PrimitiveType
{
    PRIMITIVE_POINTS,
    PRIMITIVE_LINES,
    PRIMITIVE_LINE_STRIP,
    PRIMITIVE_TRIANGLES,
    PRIMITIVE_TRIANGLE_STRIP
};

/**
  \enum IntersectPlane
  \brief Intersection plane of projection volume

  These flags can be OR'd together. */
enum IntersectPlane
{
    INTERSECT_PLANE_LEFT   = 0x01,
    INTERSECT_PLANE_RIGHT  = 0x02,
    INTERSECT_PLANE_TOP    = 0x04,
    INTERSECT_PLANE_BOTTOM = 0x08,
    INTERSECT_PLANE_FRONT  = 0x10,
    INTERSECT_PLANE_BACK   = 0x20,
    INTERSECT_PLANE_ALL = INTERSECT_PLANE_LEFT   | INTERSECT_PLANE_RIGHT  |
                          INTERSECT_PLANE_TOP    | INTERSECT_PLANE_BOTTOM |
                          INTERSECT_PLANE_FRONT  | INTERSECT_PLANE_BACK
};

/*

Notes for rewriting DirectX code:

>> SetRenderState() translates to many functions depending on param

D3DRENDERSTATE_ALPHABLENDENABLE    -> SetRenderState() with RENDER_STATE_BLENDING
D3DRENDERSTATE_ALPHAFUNC           -> SetAlphaTestFunc() func
D3DRENDERSTATE_ALPHAREF            -> SetAlphaTestFunc() ref
D3DRENDERSTATE_ALPHATESTENABLE     -> SetRenderState() with RENDER_STATE_ALPHA_TEST
D3DRENDERSTATE_AMBIENT             -> SetGlobalAmbient()
D3DRENDERSTATE_CULLMODE            -> SetCullMode()
D3DRENDERSTATE_DESTBLEND           -> SetBlendFunc() dest blending func
D3DRENDERSTATE_DITHERENABLE        -> SetRenderState() with RENDER_STATE_DITHERING
D3DRENDERSTATE_FILLMODE            -> SetFillMode()
D3DRENDERSTATE_FOGCOLOR            -> SetFogParams()
D3DRENDERSTATE_FOGENABLE           -> SetRenderState() with RENDER_STATE_FOG
D3DRENDERSTATE_FOGEND              -> SetFogParams()
D3DRENDERSTATE_FOGSTART            -> SetFogParams()
D3DRENDERSTATE_FOGVERTEXMODE       -> SetFogParams() fog model
D3DRENDERSTATE_LIGHTING            -> SetRenderState() with RENDER_STATE_LIGHTING
D3DRENDERSTATE_SHADEMODE           -> SetShadeModel()
D3DRENDERSTATE_SPECULARENABLE      -> doesn't matter (always enabled)
D3DRENDERSTATE_SRCBLEND            -> SetBlendFunc() src blending func
D3DRENDERSTATE_TEXTUREFACTOR       -> SetTextureFactor()
D3DRENDERSTATE_ZBIAS               -> SetDepthBias()
D3DRENDERSTATE_ZENABLE             -> SetRenderState() with RENDER_STATE_DEPTH_TEST
D3DRENDERSTATE_ZFUNC               -> SetDepthTestFunc()
D3DRENDERSTATE_ZWRITEENABLE        -> SetRenderState() with RENDER_STATE_DEPTH_WRITE


>> SetTextureStageState() translates to SetTextureParams() or CreateTexture() for some params

Params from enum in struct TextureCreateParams or TextureParams
  D3DTSS_ADDRESS       -> Gfx::TexWrapMode wrapS, wrapT
  D3DTSS_ALPHAARG1     -> Gfx::TexMixArgument alphaArg1
  D3DTSS_ALPHAARG2     -> Gfx::TexMixArgument alphaArg2
  D3DTSS_ALPHAOP       -> Gfx::TexMixOperation alphaOperation
  D3DTSS_COLORARG1     -> Gfx::TexMixArgument colorArg1
  D3DTSS_COLORARG2     -> Gfx::TexMixArgument colorArg2
  D3DTSS_COLOROP       -> Gfx::TexMixOperation colorOperation
  D3DTSS_MAGFILTER     -> Gfx::TexMagFilter magFilter
  D3DTSS_MINFILTER     -> Gfx::TexMinFilter minFilter
  D3DTSS_TEXCOORDINDEX -> doesn't matter (texture coords are set explicitly by glMultiTexCoordARB*)

Note that D3DTSS_ALPHAOP or D3DTSS_COLOROP set to D3DTOP_DISABLE must translate to disabling the whole texture stage.
In DirectX, you shouldn't mix enabling one and disabling the other.
Also, if previous stage is disabled in DirectX, the later ones are disabled, too. In OpenGL, that is not the case.

*/

/**
  \class CDevice
  \brief Abstract interface of graphics device

  It is based on DIRECT3DDEVICE class from DirectX to make it easier to port existing code.
  It encapsulates the general graphics device state and provides a common interface
  to graphics-specific functions which will be used throughout the program,
  both in CEngine class and in UI classes. Note that it doesn't contain all functions from DirectX,
  only those that were used in old code.

 */
class CDevice
{
public:
    virtual ~CDevice() {}

    //! Provides a hook to debug graphics code (implementation-specific)
    virtual void DebugHook() = 0;

    //! Initializes the device, setting the initial state
    virtual bool Create() = 0;
    //! Destroys the device, releasing every acquired resource
    virtual void Destroy() = 0;

    //! Returns the last encountered error
    virtual std::string GetError() = 0;

    //! Begins drawing the 3D scene
    virtual void BeginScene() = 0;
    //! Ends drawing the 3D scene
    virtual void EndScene() = 0;

    //! Clears the screen to blank
    virtual void Clear() = 0;

    //! Sets the transform matrix of given type
    virtual void SetTransform(TransformType type, const Math::Matrix &matrix) = 0;
    //! Returns the current transform matrix of given type
    virtual const Math::Matrix& GetTransform(TransformType type) = 0;
    //! Multiplies the current transform matrix of given type by given matrix
    virtual void MultiplyTransform(TransformType type, const Math::Matrix &matrix) = 0;

    //! Sets the current material
    virtual void SetMaterial(const Gfx::Material &material) = 0;
    //! Returns the current material
    virtual const Gfx::Material& GetMaterial() = 0;

    //! Returns the maximum number of lights available
    virtual int GetMaxLightCount() = 0;
    //! Sets the light at given index
    virtual void SetLight(int index, const Gfx::Light &light) = 0;
    //! Returns the current light at given index
    virtual const Gfx::Light& GetLight(int index) = 0;
    //! Enables/disables the light at given index
    virtual void SetLightEnabled(int index, bool enabled) = 0;
    //! Returns the current enable state of light at given index
    virtual bool GetLightEnabled(int index) = 0;

    //! Creates a texture from image; the image can be safely removed after that
    virtual Gfx::Texture CreateTexture(CImage *image, const Gfx::TextureCreateParams &params) = 0;
    //! Creates a texture from raw image data; image data can be freed after that
    virtual Gfx::Texture CreateTexture(ImageData *data, const Gfx::TextureCreateParams &params) = 0;
    //! Deletes a given texture, freeing it from video memory
    virtual void DestroyTexture(const Gfx::Texture &texture) = 0;
    //! Deletes all textures created so far
    virtual void DestroyAllTextures() = 0;

    //! Returns the maximum number of multitexture stages
    virtual int GetMaxTextureCount() = 0;
    //! Sets the texture at given texture stage
    virtual void SetTexture(int index, const Gfx::Texture &texture) = 0;
    //! Sets the texture image by ID at given texture stage
    virtual void SetTexture(int index, unsigned int textureId) = 0;
    //! Returns the (multi)texture at given index
    virtual Gfx::Texture GetTexture(int index) = 0;
    //! Enables/disables the given texture stage
    virtual void SetTextureEnabled(int index, bool enabled) = 0;
    //! Returns the current enable state of given texture stage
    virtual bool GetTextureEnabled(int index) = 0;

    //! Sets the params for texture stage with given index
    virtual void SetTextureStageParams(int index, const Gfx::TextureStageParams &params) = 0;
    //! Returns the current params of texture stage with given index
    virtual Gfx::TextureStageParams GetTextureStageParams(int index) = 0;

    //! Sets the texture factor to the given color value
    virtual void SetTextureFactor(const Gfx::Color &color) = 0;
    //! Returns the current texture factor
    virtual Gfx::Color GetTextureFactor() = 0;

    //! Renders primitive composed of vertices with single texture
    virtual void DrawPrimitive(Gfx::PrimitiveType type, const Gfx::Vertex *vertices    , int vertexCount) = 0;
    //! Renders primitive composed of vertices with color information and single texture
    virtual void DrawPrimitive(Gfx::PrimitiveType type, const Gfx::VertexCol *vertices , int vertexCount) = 0;
    //! Renders primitive composed of vertices with multitexturing (2 textures)
    virtual void DrawPrimitive(Gfx::PrimitiveType type, const Gfx::VertexTex2 *vertices, int vertexCount) = 0;

    //! Tests whether a sphere intersects the 6 clipping planes of projection volume
    virtual int ComputeSphereVisibility(const Math::Vector &center, float radius) = 0;

    //! Enables/disables the given render state
    virtual void SetRenderState(Gfx::RenderState state, bool enabled) = 0;
    //! Returns the current setting of given render state
    virtual bool GetRenderState(Gfx::RenderState state) = 0;

    //! Sets the function of depth test
    virtual void SetDepthTestFunc(Gfx::CompFunc func) = 0;
    //! Returns the current function of depth test
    virtual Gfx::CompFunc GetDepthTestFunc() = 0;

    //! Sets the depth bias (constant value added to Z-coords)
    virtual void SetDepthBias(float factor) = 0;
    //! Returns the current depth bias
    virtual float GetDepthBias() = 0;

    //! Sets the alpha test function and reference value
    virtual void SetAlphaTestFunc(Gfx::CompFunc func, float refValue) = 0;
    //! Returns the current alpha test function and reference value
    virtual void GetAlphaTestFunc(Gfx::CompFunc &func, float &refValue) = 0;

    //! Sets the blending functions for source and destination operations
    virtual void SetBlendFunc(Gfx::BlendFunc srcBlend, Gfx::BlendFunc dstBlend) = 0;
    //! Returns the current blending functions for source and destination operations
    virtual void GetBlendFunc(Gfx::BlendFunc &srcBlend, Gfx::BlendFunc &dstBlend) = 0;

    //! Sets the clear color
    virtual void SetClearColor(const Gfx::Color &color) = 0;
    //! Returns the current clear color
    virtual Gfx::Color GetClearColor() = 0;

    //! Sets the global ambient color
    virtual void SetGlobalAmbient(const Gfx::Color &color) = 0;
    //! Returns the global ambient color
    virtual Gfx::Color GetGlobalAmbient() = 0;

    //! Sets the fog parameters: mode, color, start distance, end distance and density (for exp models)
    virtual void SetFogParams(Gfx::FogMode mode, const Gfx::Color &color, float start, float end, float density) = 0;
    //! Returns the current fog parameters: mode, color, start distance, end distance and density (for exp models)
    virtual void GetFogParams(Gfx::FogMode &mode, Gfx::Color &color, float &start, float &end, float &density) = 0;

    //! Sets the current cull mode
    virtual void SetCullMode(Gfx::CullMode mode) = 0;
    //! Returns the current cull mode
    virtual Gfx::CullMode GetCullMode() = 0;

    //! Sets the shade model
    virtual void SetShadeModel(Gfx::ShadeModel model) = 0;
    //! Returns the current shade model
    virtual Gfx::ShadeModel GetShadeModel() = 0;

    //! Sets the current fill mode
    virtual void SetFillMode(Gfx::FillMode mode) = 0;
    //! Returns the current fill mode
    virtual Gfx::FillMode GetFillMode() = 0;
};

}; // namespace Gfx
