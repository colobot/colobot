/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2014, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsiteс.ch; http://colobot.info; http://github.com/colobot
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
 \enum VertexBufferType
 \brief Specifies type of vertex buffer to use
 */
enum VertexBufferType
{
   VBT_DISPLAY_LIST,    //! use display lists
   VBT_VBO_CORE,        //! use core OpenGL 1.5 VBOs
   VBT_VBO_ARB          //! use ARB extension VBOs
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

    virtual void DebugHook() override;
    virtual void DebugLights() override;

    virtual bool Create() override;
    virtual void Destroy() override;

    void ConfigChanged(const GLDeviceConfig &newConfig);

    void SetUseVbo(bool useVbo);
    void SetVertexBufferType(VertexBufferType type);

    virtual void BeginScene() override;
    virtual void EndScene() override;

    virtual void Clear() override;

    virtual void SetTransform(TransformType type, const Math::Matrix &matrix) override;

    virtual void SetMaterial(const Material &material) override;

    virtual int GetMaxLightCount() override;
    virtual void SetLight(int index, const Light &light) override;
    virtual void SetLightEnabled(int index, bool enabled) override;

    virtual Texture CreateTexture(CImage *image, const TextureCreateParams &params) override;
    virtual Texture CreateTexture(ImageData *data, const TextureCreateParams &params) override;
    virtual void DestroyTexture(const Texture &texture) override;
    virtual void DestroyAllTextures() override;

    virtual int GetMaxTextureStageCount() override;
    virtual void SetTexture(int index, const Texture &texture) override;
    virtual void SetTexture(int index, unsigned int textureId) override;
    virtual void SetTextureEnabled(int index, bool enabled) override;

    virtual void SetTextureStageParams(int index, const TextureStageParams &params) override;

    virtual void SetTextureStageWrap(int index, Gfx::TexWrapMode wrapS, Gfx::TexWrapMode wrapT) override;

    virtual void DrawPrimitive(PrimitiveType type, const Vertex *vertices    , int vertexCount,
                               Color color = Color(1.0f, 1.0f, 1.0f, 1.0f)) override;
    virtual void DrawPrimitive(PrimitiveType type, const VertexTex2 *vertices, int vertexCount,
                               Color color = Color(1.0f, 1.0f, 1.0f, 1.0f)) override;
    virtual void DrawPrimitive(PrimitiveType type, const VertexCol *vertices , int vertexCount) override;

    virtual unsigned int CreateStaticBuffer(PrimitiveType primitiveType, const Vertex* vertices, int vertexCount) override;
    virtual unsigned int CreateStaticBuffer(PrimitiveType primitiveType, const VertexTex2* vertices, int vertexCount) override;
    virtual unsigned int CreateStaticBuffer(PrimitiveType primitiveType, const VertexCol* vertices, int vertexCount) override;
    virtual void UpdateStaticBuffer(unsigned int bufferId, PrimitiveType primitiveType, const Vertex* vertices, int vertexCount) override;
    virtual void UpdateStaticBuffer(unsigned int bufferId, PrimitiveType primitiveType, const VertexTex2* vertices, int vertexCount) override;
    virtual void UpdateStaticBuffer(unsigned int bufferId, PrimitiveType primitiveType, const VertexCol* vertices, int vertexCount) override;
    virtual void DrawStaticBuffer(unsigned int bufferId) override;
    virtual void DestroyStaticBuffer(unsigned int bufferId) override;

    virtual int ComputeSphereVisibility(const Math::Vector &center, float radius) override;

    virtual void SetRenderState(RenderState state, bool enabled) override;

    virtual void SetDepthTestFunc(CompFunc func) override;

    virtual void SetDepthBias(float factor) override;

    virtual void SetAlphaTestFunc(CompFunc func, float refValue) override;

    virtual void SetBlendFunc(BlendFunc srcBlend, BlendFunc dstBlend) override;

    virtual void SetClearColor(const Color &color) override;

    virtual void SetGlobalAmbient(const Color &color) override;

    virtual void SetFogParams(FogMode mode, const Color &color, float start, float end, float density) override;

    virtual void SetCullMode(CullMode mode) override;

    virtual void SetShadeModel(ShadeModel model) override;

    virtual void SetFillMode(FillMode mode)  override;

    virtual void* GetFrameBufferPixels() const override;

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
    //! Which vertex buffer type to use
    VertexBufferType m_vertexBufferType;
    //! Map of saved VBO objects
    std::map<unsigned int, VboObjectInfo> m_vboObjects;
    //! Last ID of VBO object
    unsigned int m_lastVboId;
};


} // namespace Gfx

