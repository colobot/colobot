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

#pragma once

#include "graphics/core/device.h"

#include "graphics/core/light.h"
#include "graphics/core/material.h"

#include "math/matrix.h"

// Graphics module namespace
namespace Gfx
{

/**
 * \class CNullDevice
 * \brief Device implementation that doesn't render anything
 */
class CNullDevice : public CDevice
{
public:
    CNullDevice();
    virtual ~CNullDevice();

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

    void DrawPrimitive(PrimitiveType type, const void *vertices,
        int size, const VertexFormat &format, int vertexCount) override;
    void DrawPrimitives(PrimitiveType type, const void *vertices,
        int size, const VertexFormat &format, int first[], int count[], int drawCount) override;

    void DrawPrimitive(PrimitiveType type, const Vertex* vertices, int vertexCount, Color color = Color(1.0f, 1.0f, 1.0f, 1.0f)) override;
    void DrawPrimitive(PrimitiveType type, const VertexTex2* vertices, int vertexCount, Color color = Color(1.0f, 1.0f, 1.0f, 1.0f)) override;
    void DrawPrimitive(PrimitiveType type, const VertexCol *vertices, int vertexCount) override;

    void DrawPrimitives(PrimitiveType type, const Vertex *vertices,
        int first[], int count[], int drawCount,
        Color color = Color(1.0f, 1.0f, 1.0f, 1.0f)) override;
    void DrawPrimitives(PrimitiveType type, const VertexTex2 *vertices,
        int first[], int count[], int drawCount,
        Color color = Color(1.0f, 1.0f, 1.0f, 1.0f)) override;
    void DrawPrimitives(PrimitiveType type, const VertexCol *vertices,
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
};


} // namespace Gfx
