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
 * \file graphics/core/device.h
 * \brief Abstract graphics device - CDevice class and related structs/enums
 */

#pragma once


#include "graphics/core/device.h"

// Graphics module namespace
namespace Gfx {

/**
 * \class CNullDevice
 * \brief Device implementation that doesn't render anything
 *
 */
class CNullDevice : public CDevice
{
public:
    CNullDevice();
    virtual ~CNullDevice();
    
    virtual void DebugHook();
    virtual void DebugLights();
    
    virtual bool Create();
    virtual void Destroy();
    
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
    
    virtual void* GetFrameBufferPixels() const;
    
private:
    Math::Matrix m_matrix;
    Material     m_material;
    Light        m_light;
};


} // namespace Gfx

