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

  Corresponds directly to DirectX's transformation types. Listed are only the used types. */
enum TransformType
{
    TRANSFORM_WORLD,
    TRANSFORM_VIEW,
    TRANSFORM_PROJECTION
};

/**
  \enum RenderState
  \brief Render states that can be enabled/disabled

  Corresponds to DirectX's render states. Listed are only the used modes.

  TODO: replace with functions in CDevice */
enum RenderState
{
    RENDER_STATE_ALPHABLENDENABLE,
    RENDER_STATE_ALPHAFUNC,
    RENDER_STATE_ALPHAREF,
    RENDER_STATE_ALPHATESTENABLE,
    RENDER_STATE_AMBIENT,
    RENDER_STATE_CULLMODE,
    RENDER_STATE_DESTBLEND,
    RENDER_STATE_DITHERENABLE,
    RENDER_STATE_FILLMODE,
    RENDER_STATE_FOGCOLOR,
    RENDER_STATE_FOGENABLE,
    RENDER_STATE_FOGEND,
    RENDER_STATE_FOGSTART,
    RENDER_STATE_FOGVERTEXMODE,
    RENDER_STATE_LIGHTING,
    RENDER_STATE_SHADEMODE,
    RENDER_STATE_SPECULARENABLE,
    RENDER_STATE_SRCBLEND,
    RENDER_STATE_TEXTUREFACTOR,
    RENDER_STATE_WRAP,
    RENDER_STATE_ZBIAS,
    RENDER_STATE_ZENABLE,
    RENDER_STATE_ZFUNC,
    RENDER_STATE_ZWRITEENABLE
};

/**
  \enum PrimitiveType
  \brief Type of primitive to render

  Only these two types are used. */
enum PrimitiveType
{
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
    //! Initializes the device, setting the initial state
    virtual void Initialize() = 0;
    //! Destroys the device, releasing every acquired resource
    virtual void Destroy() = 0;

    // TODO: documentation

    virtual void BeginScene() = 0;
    virtual void EndScene() = 0;

    virtual void Clear() = 0;

    virtual void SetTransform(TransformType type, const Math::Matrix &matrix) = 0;
    virtual const Math::Matrix& GetTransform(TransformType type) = 0;
    virtual void MultiplyTransform(TransformType type, const Math::Matrix &matrix) = 0;

    virtual void SetMaterial(const Gfx::Material &material) = 0;
    virtual const Gfx::Material& GetMaterial() = 0;

    virtual int GetMaxLightCount() = 0;
    virtual void SetLight(int index, const Gfx::Light &light) = 0;
    virtual const Gfx::Light& GetLight(int index) = 0;
    virtual void SetLightEnabled(int index, bool enabled) = 0;
    virtual bool GetLightEnabled(int index) = 0;

    virtual int GetMaxTextureCount() = 0;
    virtual const Gfx::Texture& GetTexture(int index) = 0;
    virtual void SetTexture(int index, const Gfx::Texture &texture) = 0;

    // TODO:
    // virtual void GetTextureStageState() = 0;
    // virtual void SetTextureStageState() = 0;

    virtual void SetRenderState(Gfx::RenderState state, bool enabled) = 0;
    virtual bool GetRenderState(Gfx::RenderState state) = 0;

    // TODO:
    // virtual void ComputeSphereVisibility() = 0;

    virtual void DrawPrimitive(PrimitiveType, Vertex *vertices, int vertexCount) = 0;
    virtual void DrawPrimitive(PrimitiveType, VertexTex2 *vertices, int vertexCount) = 0;
};

}; // namespace Gfx
