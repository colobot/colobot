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
 * \file graphics/opengl/gl33device.h
 * \brief OpenGL 3.3 implementation - CGL33Device class
 */

#pragma once

#include "graphics/core/device.h"

#include "graphics/core/material.h"

#include "graphics/opengl/glframebuffer.h"
#include "graphics/opengl/glutil.h"

#include "math/matrix.h"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>


// Graphics module namespace
namespace Gfx
{

//! Struct for dynamic buffers
struct DynamicBuffer
{
    //! Auxiliary VAO for rendering primitives with DrawPrimitive*
    GLuint vao = 0;
    //! Dynamic buffer
    GLuint vbo = 0;
    //! Dynamic buffer size
    unsigned int size = 0;
    //! Dynamic buffer offset
    unsigned int offset = 0;
};

/**
  \class CGL33Device
  \brief Implementation of CDevice interface in OpenGL 3.3

  Provides the concrete implementation of 3D device in OpenGL.

  This class should be initialized (by calling Initialize() ) only after
  setting the video mode by CApplication, once the OpenGL context is defined.
  Because of that, CGLDeviceConfig is outside the CDevice class and must be set
  in CApplication.
*/
class CGL33Device : public CDevice
{
public:
    CGL33Device(const DeviceConfig &config);
    virtual ~CGL33Device();

    void DebugHook() override;
    void DebugLights() override;

    std::string GetName() override;

    bool Create() override;
    void Destroy() override;

    void ConfigChanged(const DeviceConfig &newConfig) override;

    void BeginScene() override;
    void EndScene() override;

    void Clear() override;

    void SetRenderMode(RenderMode mode) override;

    void SetTransform(TransformType type, const Math::Matrix &matrix) override;

    void SetMaterial(const Material &material) override;

    int GetMaxLightCount() override;
    void SetLight(int index, const Light &light) override;
    void SetLightEnabled(int index, bool enabled) override;

    Texture CreateTexture(CImage *image, const TextureCreateParams &params) override;
    Texture CreateTexture(ImageData *data, const TextureCreateParams &params) override;
    Texture CreateDepthTexture(int width, int height, int depth) override;
    void UpdateTexture(const Texture& texture, Math::IntPoint offset, ImageData* data, TexImgFormat format) override;
    void DestroyTexture(const Texture &texture) override;
    void DestroyAllTextures() override;

    int GetMaxTextureStageCount() override;
    void SetTexture(int index, const Texture &texture) override;
    void SetTexture(int index, unsigned int textureId) override;
    void SetTextureEnabled(int index, bool enabled) override;

    void SetTextureStageParams(int index, const TextureStageParams &params) override;

    void SetTextureStageWrap(int index, Gfx::TexWrapMode wrapS, Gfx::TexWrapMode wrapT) override;

    virtual void DrawPrimitive(PrimitiveType type, const void *vertices,
        int size, const VertexFormat &format, int vertexCount) override;
    virtual void DrawPrimitives(PrimitiveType type, const void *vertices,
        int size, const VertexFormat &format, int first[], int count[], int drawCount) override;

    virtual void DrawPrimitive(PrimitiveType type, const Vertex *vertices    , int vertexCount,
                               Color color = Color(1.0f, 1.0f, 1.0f, 1.0f)) override;
    virtual void DrawPrimitive(PrimitiveType type, const VertexTex2 *vertices, int vertexCount,
                               Color color = Color(1.0f, 1.0f, 1.0f, 1.0f)) override;
    virtual void DrawPrimitive(PrimitiveType type, const VertexCol *vertices , int vertexCount) override;

    virtual void DrawPrimitives(PrimitiveType type, const Vertex *vertices,
        int first[], int count[], int drawCount,
        Color color = Color(1.0f, 1.0f, 1.0f, 1.0f)) override;
    virtual void DrawPrimitives(PrimitiveType type, const VertexTex2 *vertices,
        int first[], int count[], int drawCount,
        Color color = Color(1.0f, 1.0f, 1.0f, 1.0f)) override;
    virtual void DrawPrimitives(PrimitiveType type, const VertexCol *vertices,
        int first[], int count[], int drawCount) override;

    unsigned int CreateStaticBuffer(PrimitiveType primitiveType, const Vertex* vertices, int vertexCount) override;
    unsigned int CreateStaticBuffer(PrimitiveType primitiveType, const VertexTex2* vertices, int vertexCount) override;
    unsigned int CreateStaticBuffer(PrimitiveType primitiveType, const VertexCol* vertices, int vertexCount) override;
    void UpdateStaticBuffer(unsigned int bufferId, PrimitiveType primitiveType, const Vertex* vertices, int vertexCount) override;
    void UpdateStaticBuffer(unsigned int bufferId, PrimitiveType primitiveType, const VertexTex2* vertices, int vertexCount) override;
    void UpdateStaticBuffer(unsigned int bufferId, PrimitiveType primitiveType, const VertexCol* vertices, int vertexCount) override;
    void DrawStaticBuffer(unsigned int bufferId) override;
    void DestroyStaticBuffer(unsigned int bufferId) override;

    int ComputeSphereVisibility(const Math::Vector &center, float radius) override;

    void SetViewport(int x, int y, int width, int height) override;

    void SetRenderState(RenderState state, bool enabled) override;

    void SetColorMask(bool red, bool green, bool blue, bool alpha) override;

    void SetDepthTestFunc(CompFunc func) override;

    void SetDepthBias(float factor, float units) override;

    void SetAlphaTestFunc(CompFunc func, float refValue) override;

    void SetBlendFunc(BlendFunc srcBlend, BlendFunc dstBlend) override;

    void SetClearColor(const Color &color) override;

    void SetGlobalAmbient(const Color &color) override;

    void SetFogParams(FogMode mode, const Color &color, float start, float end, float density) override;

    void SetCullMode(CullMode mode) override;

    void SetShadeModel(ShadeModel model) override;

    void SetShadowColor(float value) override;

    void SetFillMode(FillMode mode) override;

    void CopyFramebufferToTexture(Texture& texture, int xOffset, int yOffset, int x, int y, int width, int height) override;

    std::unique_ptr<CFrameBufferPixels> GetFrameBufferPixels() const override;

    CFramebuffer* GetFramebuffer(std::string name) override;

    CFramebuffer* CreateFramebuffer(std::string name, const FramebufferParams& params) override;

    void DeleteFramebuffer(std::string name) override;

    bool IsAnisotropySupported() override;
    int GetMaxAnisotropyLevel() override;

    int GetMaxSamples() override;

    bool IsShadowMappingSupported() override;

    int GetMaxTextureSize() override;

    bool IsFramebufferSupported() override;

private:
    //! Updates the texture params for given texture stage
    void UpdateTextureParams(int index);
    //! Updates texture state
    inline void UpdateTextureState(int index);
    //! Update light parameters
    void UpdateLights();

    //! Binds VBO
    inline void BindVBO(GLuint vbo);
    //! Binds VAO
    inline void BindVAO(GLuint vao);
    //! Binds texture
    inline void BindTexture(int index, GLuint texture);

    //! Uploads data to dynamic buffer and returns offset to it
    unsigned int UploadVertexData(DynamicBuffer& buffer, const void* data, unsigned int size);

    inline void UpdateVertexAttribute(int index, const VertexAttribute &attribute, int offset);

private:
    //! Current config
    DeviceConfig m_config;

    //! Current world matrix
    Math::Matrix m_worldMat;
    //! Current view matrix
    Math::Matrix m_viewMat;
    //! OpenGL modelview matrix = world matrix * view matrix
    Math::Matrix m_modelviewMat;
    //! Current projection matrix
    Math::Matrix m_projectionMat;
    //! Combined world-view-projection matrix
    Math::Matrix m_combinedMatrix;
    //! true means combined matrix is outdated
    bool m_combinedMatrixOutdated = true;

    //! The current material
    Material m_material;

    //! Whether lighting is enabled
    bool m_lighting = false;
    //! true means that light update is needed
    bool m_updateLights = false;
    //! Current lights
    std::vector<Light> m_lights;
    //! Current lights enable status
    std::vector<bool> m_lightsEnabled;

    //! Current textures; \c nullptr value means unassigned
    std::vector<Texture> m_currentTextures;
    //! Current texture stages enable status
    std::vector<bool> m_texturesEnabled;
    //! Current texture params
    std::vector<TextureStageParams> m_textureStageParams;

    //! Set of all created textures
    std::set<Texture> m_allTextures;
    //! Free texture unit
    const int m_freeTexture = 3;

    //! Type of vertex structure
    enum VertexType
    {
        VERTEX_TYPE_NORMAL,
        VERTEX_TYPE_TEX2,
        VERTEX_TYPE_COL,
    };

    //! Info about static VBO buffers
    struct VertexBufferInfo
    {
        PrimitiveType primitiveType = {};
        GLuint vbo = 0;
        GLuint vao = 0;
        VertexType vertexType = {};
        int vertexCount = 0;
        unsigned int size = 0;
    };

    //! Detected capabilities
    //! Map of saved VBO objects
    std::map<unsigned int, VertexBufferInfo> m_vboObjects;
    //! Last ID of VBO object
    unsigned int m_lastVboId = 0;
    //! Currently bound VBO
    GLuint m_currentVBO = 0;
    //! Currently bound VAO
    GLuint m_currentVAO = 0;

    //! Total memory allocated in VBOs
    unsigned long m_vboMemory = 0;

    //! Map of framebuffers
    std::map<std::string, std::unique_ptr<CFramebuffer>> m_framebuffers;

    //! Shader program for normal rendering
    GLuint m_normalProgram = 0;
    //! Shader program for interface rendering
    GLuint m_interfaceProgram = 0;
    //! Shader program for shadow rendering
    GLuint m_shadowProgram = 0;

    DynamicBuffer m_dynamicBuffer;

    //! Current mode
    unsigned int m_mode = 0;
    //! Uniform locations for all modes
    UniformLocations m_uniforms[3];
    //! Uniform locations for current mode
    UniformLocations* m_uni = nullptr;
};

} // namespace Gfx
