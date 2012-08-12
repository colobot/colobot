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

/**
 * \file graphics/opengl/gldevice.h
 * \brief OpenGL implementation - Gfx::CGLDevice class
 */

#pragma once


#include "graphics/core/device.h"

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
    CGLDevice(const Gfx::GLDeviceConfig &config);
    virtual ~CGLDevice();

    virtual void DebugHook();

    virtual std::string GetError();

    virtual bool Create();
    virtual void Destroy();

    void ConfigChanged(const Gfx::GLDeviceConfig &newConfig);

    virtual void BeginScene();
    virtual void EndScene();

    virtual void Clear();

    virtual void SetTransform(Gfx::TransformType type, const Math::Matrix &matrix);
    virtual const Math::Matrix& GetTransform(Gfx::TransformType type);
    virtual void MultiplyTransform(Gfx::TransformType type, const Math::Matrix &matrix);

    virtual void SetMaterial(const Gfx::Material &material);
    virtual const Gfx::Material& GetMaterial();

    virtual int GetMaxLightCount();
    virtual void SetLight(int index, const Gfx::Light &light);
    virtual const Gfx::Light& GetLight(int index);
    virtual void SetLightEnabled(int index, bool enabled);
    virtual bool GetLightEnabled(int index);

    virtual Gfx::Texture CreateTexture(CImage *image, const Gfx::TextureCreateParams &params);
    virtual Gfx::Texture CreateTexture(ImageData *data, const Gfx::TextureCreateParams &params);
    virtual void DestroyTexture(const Gfx::Texture &texture);
    virtual void DestroyAllTextures();

    virtual int GetMaxTextureCount();
    virtual void SetTexture(int index, const Gfx::Texture &texture);
    virtual void SetTexture(int index, unsigned int textureId);
    virtual Gfx::Texture GetTexture(int index);
    virtual void SetTextureEnabled(int index, bool enabled);
    virtual bool GetTextureEnabled(int index);

    virtual void SetTextureStageParams(int index, const Gfx::TextureStageParams &params);
    virtual Gfx::TextureStageParams GetTextureStageParams(int index);

    virtual void SetTextureFactor(const Gfx::Color &color);
    virtual Gfx::Color GetTextureFactor();

    virtual void DrawPrimitive(Gfx::PrimitiveType type, const Gfx::Vertex     *vertices, int vertexCount);
    virtual void DrawPrimitive(Gfx::PrimitiveType type, const Gfx::VertexCol  *vertices, int vertexCount);
    virtual void DrawPrimitive(Gfx::PrimitiveType type, const Gfx::VertexTex2 *vertices, int vertexCount);

    virtual int ComputeSphereVisibility(const Math::Vector &center, float radius);

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

    virtual void SetClearColor(const Gfx::Color &color);
    virtual Gfx::Color GetClearColor();

    virtual void SetGlobalAmbient(const Gfx::Color &color);
    virtual Gfx::Color GetGlobalAmbient();

    virtual void SetFogParams(Gfx::FogMode mode, const Gfx::Color &color, float start, float end, float density);
    virtual void GetFogParams(Gfx::FogMode &mode, Gfx::Color &color, float &start, float &end, float &density);

    virtual void SetCullMode(Gfx::CullMode mode);
    virtual Gfx::CullMode GetCullMode();

    virtual void SetShadeModel(Gfx::ShadeModel model);
    virtual Gfx::ShadeModel GetShadeModel();

    virtual void SetFillMode(Gfx::FillMode mode) ;
    virtual Gfx::FillMode GetFillMode();

private:
    //! Updates internal modelview matrix
    void UpdateModelviewMatrix();
    //! Updates position for given light based on transformation matrices
    void UpdateLightPosition(int index);

private:
    //! Current config
    Gfx::GLDeviceConfig m_config;
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

    //! Whether lighting is enabled
    bool m_lighting;
    //! Current lights
    std::vector<Gfx::Light> m_lights;
    //! Current lights enable status
    std::vector<bool> m_lightsEnabled;

    //! Whether texturing is enabled in general
    bool m_texturing;
    //! Current textures; \c NULL value means unassigned
    std::vector<Gfx::Texture> m_currentTextures;
    //! Current texture stages enable status
    std::vector<bool> m_texturesEnabled;
    //! Current texture params
    std::vector<Gfx::TextureStageParams> m_textureStageParams;

    //! Set of all created textures
    std::set<Gfx::Texture> m_allTextures;
};

}; // namespace Gfx
