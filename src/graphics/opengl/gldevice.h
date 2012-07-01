// * This file is part of the COLOBOT source code
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

// gldevice.h

#pragma once


#include "graphics/common/device.h"

#include <string>
#include <vector>
#include <set>


namespace Gfx {

/**
 \struct GLDeviceConfig
 \brief Additional config with OpenGL-specific settings */
struct GLDeviceConfig : public DeviceConfig
{
    //! Size of red channel in bits
    int redSize;
    //! Size of green channel in bits
    int greenSize;
    //! Size of blue channel in bits
    int blueSize;
    //! Size of alpha channel in bits
    int alphaSize;
    //! Color depth in bits
    int depthSize;

    //! Force hardware acceleration (video mode set will fail on lack of hw accel)
    bool hardwareAccel;

    //! Constructor calls LoadDefaults()
    GLDeviceConfig() { LoadDefault(); }

    //! Loads the default values
    void LoadDefault();
};

struct GLDevicePrivate;

/**
  \class CGLDevice
  \brief Implementation of CDevice interface in OpenGL

  Provides the concrete implementation of 3D device in OpenGL.

  This class should be initialized (by calling Initialize() ) only after
  setting the video mode by CApplication, once the OpenGL context is defined.
  Because of that, CGLDeviceConfig is outside the CDevice class and must be set
  in CApplication.
*/
class CGLDevice : public Gfx::CDevice
{
public:
    CGLDevice();
    virtual ~CGLDevice();

    virtual bool GetWasInit();
    virtual std::string GetError();

    virtual bool Create();
    virtual void Destroy();

    virtual void BeginScene();
    virtual void EndScene();

    virtual void Clear();

    virtual void SetTransform(Gfx::TransformType type, const Math::Matrix &matrix);
    virtual const Math::Matrix& GetTransform(Gfx::TransformType type);
    virtual void MultiplyTransform(Gfx::TransformType type, const Math::Matrix &matrix);

    virtual void SetMaterial(Gfx::Material &material);
    virtual const Gfx::Material& GetMaterial();

    virtual int GetMaxLightCount();
    virtual void SetLight(int index, Gfx::Light &light);
    virtual const Gfx::Light& GetLight(int index);
    virtual void SetLightEnabled(int index, bool enabled);
    virtual bool GetLightEnabled(int index);

    virtual int GetMaxTextureCount();
        virtual void SetTexture(int index, Gfx::Texture *texture);
    virtual Gfx::Texture* GetTexture(int index);

    virtual void DrawPrimitive(Gfx::PrimitiveType type, Vertex *vertices, int vertexCount);
    virtual void DrawPrimitive(Gfx::PrimitiveType type, Gfx::VertexCol *vertices, int vertexCount);
    virtual void DrawPrimitive(Gfx::PrimitiveType type, VertexTex2 *vertices, int vertexCount);


    virtual void SetRenderState(Gfx::RenderState state, bool enabled);
    virtual bool GetRenderState(Gfx::RenderState state);

    virtual void SetDepthTestFunc(Gfx::CompFunc func);
    virtual Gfx::CompFunc GetDepthTestFunc();

    virtual void SetDepthBias(float factor);
    virtual float GetDepthBias();

    virtual void SetAlphaTestFunc(Gfx::CompFunc func, float refValue);
    virtual void GetAlphaTestFunc(Gfx::CompFunc &func, float &refValue);

    virtual void SetBlendFunc(Gfx::BlendFunc srcBlend, Gfx::BlendFunc dstBlend);
    virtual void GetBlendFunc(Gfx::BlendFunc &srcBlend, Gfx::BlendFunc &dstBlend);

    virtual void SetClearColor(Gfx::Color color);
    virtual Gfx::Color GetClearColor();

    virtual void SetGlobalAmbient(Gfx::Color color);
    virtual Gfx::Color GetGlobalAmbient();

    virtual void SetFogParams(Gfx::FogMode mode, Gfx::Color color, float start, float end, float density);
    virtual void GetFogParams(Gfx::FogMode &mode, Gfx::Color &color, float &start, float &end, float &density);

    virtual void SetCullMode(Gfx::CullMode mode);
    virtual Gfx::CullMode GetCullMode();

    virtual void SetFillMode(Gfx::FillMode mode) ;
    virtual Gfx::FillMode GetFillMode();

private:
    //! Private, OpenGL-specific data
    GLDevicePrivate* m_private;
    //! Was initialized?
    bool m_wasInit;
    //! Last encountered error
    std::string m_error;
    //! Current world matrix
    Math::Matrix m_worldMat;
    //! Current view matrix
    Math::Matrix m_viewMat;
    //! OpenGL modelview matrix = world matrix * view matrix
    Math::Matrix m_modelviewMat;
    //! Current projection matrix
    Math::Matrix m_projectionMat;
    //! The current material
    Gfx::Material m_material;
    //! Current lights
    std::vector<Gfx::Light> m_lights;
    //! Current lights enable status
    std::vector<bool> m_lightsEnabled;
    //! Current textures
    std::vector<Gfx::Texture*> m_textures;
    //! Set of all created textures
    std::set<Gfx::Texture*> m_allTextures;
};

}; // namespace Gfx
