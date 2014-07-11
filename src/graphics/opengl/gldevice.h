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
 * \brief OpenGL implementation - CGLDevice class
 */

#pragma once


#include "graphics/core/device.h"

#include <string>
#include <vector>
#include <set>
#include <map>


// Graphics module namespace
namespace Gfx {

/**
 \enum VBOMode
 \brief VBO autodetect/override
 */
enum VBOMode
{
    VBO_MODE_ENABLE,  //! < override: enable
    VBO_MODE_DISABLE, //! < override: disable
    VBO_MODE_AUTO     //! < autodetect
};

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

    //! VBO override/autodetect
    VBOMode vboMode;

    //! Constructor calls LoadDefaults()
    GLDeviceConfig();

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
class CGLDevice : public CDevice
{
public:
    CGLDevice(const GLDeviceConfig &config);
    virtual ~CGLDevice();

    virtual void DebugHook();
    virtual void DebugLights();

    virtual bool Create();
    virtual void Destroy();

    void ConfigChanged(const GLDeviceConfig &newConfig);

    void SetUseVbo(bool useVbo);
    bool GetUseVbo();

    virtual void BeginScene();
    virtual void EndScene();

    virtual void Clear();

    virtual void SetTransform(TransformType type, const Math::Matrix &matrix);
    virtual const Math::Matrix& GetTransform(TransformType type);
    virtual void MultiplyTransform(TransformType type, const Math::Matrix &matrix);

    virtual void SetMaterial(const Material &material);
    virtual const Material& GetMaterial();

    virtual int GetMaxLightCount();
    virtual void SetLight(int index, const Light &light);
    virtual const Light& GetLight(int index);
    virtual void SetLightEnabled(int index, bool enabled);
    virtual bool GetLightEnabled(int index);

    virtual Texture CreateTexture(CImage *image, const TextureCreateParams &params);
    virtual Texture CreateTexture(ImageData *data, const TextureCreateParams &params);
    virtual void DestroyTexture(const Texture &texture);
    virtual void DestroyAllTextures();

    virtual int GetMaxTextureStageCount();
    virtual void SetTexture(int index, const Texture &texture);
    virtual void SetTexture(int index, unsigned int textureId);
    virtual Texture GetTexture(int index);
    virtual void SetTextureEnabled(int index, bool enabled);
    virtual bool GetTextureEnabled(int index);

    virtual void SetTextureStageParams(int index, const TextureStageParams &params);
    virtual TextureStageParams GetTextureStageParams(int index);

    virtual void SetTextureStageWrap(int index, Gfx::TexWrapMode wrapS, Gfx::TexWrapMode wrapT);

    virtual void DrawPrimitive(PrimitiveType type, const Vertex *vertices    , int vertexCount,
                               Color color = Color(1.0f, 1.0f, 1.0f, 1.0f));
    virtual void DrawPrimitive(PrimitiveType type, const VertexTex2 *vertices, int vertexCount,
                               Color color = Color(1.0f, 1.0f, 1.0f, 1.0f));
    virtual void DrawPrimitive(PrimitiveType type, const VertexCol *vertices , int vertexCount);

    virtual unsigned int CreateStaticBuffer(PrimitiveType primitiveType, const Vertex* vertices, int vertexCount);
    virtual unsigned int CreateStaticBuffer(PrimitiveType primitiveType, const VertexTex2* vertices, int vertexCount);
    virtual unsigned int CreateStaticBuffer(PrimitiveType primitiveType, const VertexCol* vertices, int vertexCount);
    virtual void UpdateStaticBuffer(unsigned int bufferId, PrimitiveType primitiveType, const Vertex* vertices, int vertexCount);
    virtual void UpdateStaticBuffer(unsigned int bufferId, PrimitiveType primitiveType, const VertexTex2* vertices, int vertexCount);
    virtual void UpdateStaticBuffer(unsigned int bufferId, PrimitiveType primitiveType, const VertexCol* vertices, int vertexCount);
    virtual void DrawStaticBuffer(unsigned int bufferId);
    virtual void DestroyStaticBuffer(unsigned int bufferId);

    virtual int ComputeSphereVisibility(const Math::Vector &center, float radius);

    virtual void SetRenderState(RenderState state, bool enabled);
    virtual bool GetRenderState(RenderState state);

    virtual void SetDepthTestFunc(CompFunc func);
    virtual CompFunc GetDepthTestFunc();

    virtual void SetDepthBias(float factor);
    virtual float GetDepthBias();

    virtual void SetAlphaTestFunc(CompFunc func, float refValue);
    virtual void GetAlphaTestFunc(CompFunc &func, float &refValue);

    virtual void SetBlendFunc(BlendFunc srcBlend, BlendFunc dstBlend);
    virtual void GetBlendFunc(BlendFunc &srcBlend, BlendFunc &dstBlend);

    virtual void SetClearColor(const Color &color);
    virtual Color GetClearColor();

    virtual void SetGlobalAmbient(const Color &color);
    virtual Color GetGlobalAmbient();

    virtual void SetFogParams(FogMode mode, const Color &color, float start, float end, float density);
    virtual void GetFogParams(FogMode &mode, Color &color, float &start, float &end, float &density);

    virtual void SetCullMode(CullMode mode);
    virtual CullMode GetCullMode();

    virtual void SetShadeModel(ShadeModel model);
    virtual ShadeModel GetShadeModel();

    virtual void SetFillMode(FillMode mode) ;
    virtual FillMode GetFillMode();

    virtual void* GetFrameBufferPixels()const;

private:
    //! Updates internal modelview matrix
    void UpdateModelviewMatrix();
    //! Updates position for given light based on transformation matrices
    void UpdateLightPosition(int index);
    //! Updates the texture params for given texture stage
    void UpdateTextureParams(int index);

private:
    //! Current config
    GLDeviceConfig m_config;

    //! Current world matrix
    Math::Matrix m_worldMat;
    //! Current view matrix
    Math::Matrix m_viewMat;
    //! OpenGL modelview matrix = world matrix * view matrix
    Math::Matrix m_modelviewMat;
    //! Current projection matrix
    Math::Matrix m_projectionMat;

    //! The current material
    Material m_material;

    //! Whether lighting is enabled
    bool m_lighting;
    //! Current lights
    std::vector<Light> m_lights;
    //! Current lights enable status
    std::vector<bool> m_lightsEnabled;

    //! Current textures; \c NULL value means unassigned
    std::vector<Texture> m_currentTextures;
    //! Current texture stages enable status
    std::vector<bool> m_texturesEnabled;
    //! Current texture params
    std::vector<TextureStageParams> m_textureStageParams;

    //! Set of all created textures
    std::set<Texture> m_allTextures;

    //! Type of vertex structure
    enum VertexType
    {
        VERTEX_TYPE_NORMAL,
        VERTEX_TYPE_TEX2,
        VERTEX_TYPE_COL,
    };

    //! Info about static VBO buffers
    struct VboObjectInfo
    {
        PrimitiveType primitiveType;
        unsigned int bufferId;
        VertexType vertexType;
        int vertexCount;
    };

    //! Whether to use multitexturing
    bool m_multitextureAvailable;
    //! Whether to use VBOs or display lists
    bool m_vboAvailable;
    //! Map of saved VBO objects
    std::map<unsigned int, VboObjectInfo> m_vboObjects;
    //! Last ID of VBO object
    unsigned int m_lastVboId;
};


} // namespace Gfx

