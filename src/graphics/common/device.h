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

// device.h

#pragma once


#include "graphics/common/color.h"
#include "graphics/common/light.h"
#include "graphics/common/material.h"
#include "graphics/common/texture.h"
#include "graphics/common/vertex.h"
#include "math/matrix.h"

#include <string>


namespace Gfx {

/**
  \struct DeviceConfig
  \brief General config for graphics device

  These settings are common window options set by SDL.
*/
struct DeviceConfig
{
    //! Screen width
    int width;
    //! Screen height
    int height;
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
    void LoadDefault();
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
    //! Cull clockwise side
    CULL_CW,
    //! Cull counter-clockwise side
    CULL_CCW
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
  \brief Type of primitive to render

  Only these two types are used. */
enum PrimitiveType
{
    PRIMITIVE_LINES,
    PRIMITIVE_TRIANGLES,
    PRIMITIVE_TRIANGLE_STRIP
};

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

    //! Initializes the device, setting the initial state
    virtual bool Create() = 0;
    //! Destroys the device, releasing every acquired resource
    virtual void Destroy() = 0;

    //! Returns whether the device has been initialized
    virtual bool GetWasInit() = 0;
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
    virtual void SetMaterial(Gfx::Material &material) = 0;
    //! Returns the current material
    virtual const Gfx::Material& GetMaterial() = 0;

    //! Returns the maximum number of lights available
    virtual int GetMaxLightCount() = 0;
    //! Sets the light at given index
    virtual void SetLight(int index, Gfx::Light &light) = 0;
    //! Returns the current light at given index
    virtual const Gfx::Light& GetLight(int index) = 0;
    //! Enables/disables the light at given index
    virtual void SetLightEnabled(int index, bool enabled) = 0;
    //! Returns the current enable state of light at given index
    virtual bool GetLightEnabled(int index) = 0;

    // TODO:
    // virtual Gfx::Texture* CreateTexture(CImage *image) = 0;
    // virtual void DestroyTexture(Gfx::Texture *texture) = 0;

    //! Returns the maximum number of multitexture units
    virtual int GetMaxTextureCount() = 0;
    //! Sets the (multi)texture at given index
    virtual void SetTexture(int index, Gfx::Texture *texture) = 0;
    //! Returns the (multi)texture at given index
    virtual Gfx::Texture* GetTexture(int index) = 0;

    // TODO:
    // virtual void GetTextureStageState() = 0;
    // virtual void SetTextureStageState() = 0;

    //! Renders primitive composed of vertices with single texture
    virtual void DrawPrimitive(Gfx::PrimitiveType type, Gfx::Vertex *vertices, int vertexCount) = 0;
    //! Renders primitive composed of vertices with color information and single texture
    virtual void DrawPrimitive(Gfx::PrimitiveType type, Gfx::VertexCol *vertices, int vertexCount) = 0;
    //! Renders primitive composed of vertices with multitexturing (2 textures)
    virtual void DrawPrimitive(Gfx::PrimitiveType type, Gfx::VertexTex2 *vertices, int vertexCount) = 0;

    // TODO:
    // virtual void ComputeSphereVisibility() = 0;


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
    virtual void SetClearColor(Gfx::Color color) = 0;
    //! Returns the current clear color
    virtual Gfx::Color GetClearColor() = 0;

    //! Sets the global ambient color
    virtual void SetGlobalAmbient(Gfx::Color color) = 0;
    //! Returns the global ambient color
    virtual Gfx::Color GetGlobalAmbient() = 0;

    //! Sets the fog parameters: mode, color, start distance, end distance and density (for exp models)
    virtual void SetFogParams(Gfx::FogMode mode, Gfx::Color color, float start, float end, float density) = 0;
    //! Returns the current fog parameters: mode, color, start distance, end distance and density (for exp models)
    virtual void GetFogParams(Gfx::FogMode &mode, Gfx::Color &color, float &start, float &end, float &density) = 0;

    //! Sets the current cull mode
    virtual void SetCullMode(Gfx::CullMode mode) = 0;
    //! Returns the current cull mode
    virtual Gfx::CullMode GetCullMode() = 0;

    //! Sets the current fill mode
    virtual void SetFillMode(Gfx::FillMode mode) = 0;
    //! Returns the current fill mode
    virtual Gfx::FillMode GetFillMode() = 0;
};

}; // namespace Gfx
