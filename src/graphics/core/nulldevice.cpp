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


#include "graphics/core/nulldevice.h"

#include "common/config.h"
#include "common/logger.h"

#include "math/geometry.h"


// Graphics module namespace
namespace Gfx {

CNullDevice::CNullDevice()
{
    m_matrix = Math::Matrix();
    m_material = Material();
    m_light = Light();
}

CNullDevice::~CNullDevice()
{
}

void CNullDevice::DebugHook()
{
}

void CNullDevice::DebugLights()
{
}

bool CNullDevice::Create()
{
    return true;
}

void CNullDevice::Destroy()
{
}

void CNullDevice::BeginScene()
{
}

void CNullDevice::EndScene()
{
}

void CNullDevice::Clear()
{
}

void CNullDevice::SetTransform(TransformType type, const Math::Matrix &matrix)
{
}

const Math::Matrix& CNullDevice::GetTransform(TransformType type)
{
    return m_matrix;
}

void CNullDevice::MultiplyTransform(TransformType type, const Math::Matrix &matrix)
{
}

void CNullDevice::SetMaterial(const Material &material)
{
}

const Material& CNullDevice::GetMaterial()
{
    return m_material;
}

int CNullDevice::GetMaxLightCount()
{
    return 99;
}

void CNullDevice::SetLight(int index, const Light &light)
{
}

const Light& CNullDevice::GetLight(int index)
{
    return m_light;
}

void CNullDevice::SetLightEnabled(int index, bool enabled)
{
}

bool CNullDevice::GetLightEnabled(int index)
{
    return false;
}

Texture CNullDevice::CreateTexture(CImage *image, const TextureCreateParams &params)
{
    Texture tex;
    tex.id = 1; // tex.id = 0 => invalid texture
    return tex;
}

Texture CNullDevice::CreateTexture(ImageData *data, const TextureCreateParams &params)
{
    Texture tex;
    tex.id = 1; // tex.id = 0 => invalid texture
    return tex;
}

void CNullDevice::DestroyTexture(const Texture &texture)
{
}

void CNullDevice::DestroyAllTextures()
{
}

int CNullDevice::GetMaxTextureStageCount()
{
    return 0;
}

void CNullDevice::SetTexture(int index, const Texture &texture)
{
}

void CNullDevice::SetTexture(int index, unsigned int textureId)
{
}

Texture CNullDevice::GetTexture(int index)
{
    return Texture();
}

void CNullDevice::SetTextureEnabled(int index, bool enabled)
{
}

bool CNullDevice::GetTextureEnabled(int index)
{
    return false;
}

void CNullDevice::SetTextureStageParams(int index, const TextureStageParams &params)
{
}

void CNullDevice::SetTextureStageWrap(int index, TexWrapMode wrapS, TexWrapMode wrapT)
{
}

TextureStageParams CNullDevice::GetTextureStageParams(int index)
{
    return TextureStageParams();
}

void CNullDevice::DrawPrimitive(PrimitiveType type, const Vertex *vertices, int vertexCount,
                              Color color)
{
}

void CNullDevice::DrawPrimitive(PrimitiveType type, const VertexTex2 *vertices, int vertexCount,
                              Color color)
{
}

void CNullDevice::DrawPrimitive(PrimitiveType type, const VertexCol *vertices, int vertexCount)
{
}

unsigned int CNullDevice::CreateStaticBuffer(PrimitiveType primitiveType, const Vertex* vertices, int vertexCount)
{
    return 0;
}

unsigned int CNullDevice::CreateStaticBuffer(PrimitiveType primitiveType, const VertexTex2* vertices, int vertexCount)
{
    return 0;
}

unsigned int CNullDevice::CreateStaticBuffer(PrimitiveType primitiveType, const VertexCol* vertices, int vertexCount)
{
    return 0;
}

void CNullDevice::UpdateStaticBuffer(unsigned int bufferId, PrimitiveType primitiveType, const Vertex* vertices, int vertexCount)
{
}

void CNullDevice::UpdateStaticBuffer(unsigned int bufferId, PrimitiveType primitiveType, const VertexTex2* vertices, int vertexCount)
{
}

void CNullDevice::UpdateStaticBuffer(unsigned int bufferId, PrimitiveType primitiveType, const VertexCol* vertices, int vertexCount)
{
}

void CNullDevice::DrawStaticBuffer(unsigned int bufferId)
{
}

void CNullDevice::DestroyStaticBuffer(unsigned int bufferId)
{
}

int CNullDevice::ComputeSphereVisibility(const Math::Vector &center, float radius)
{
    return 0;
}

void CNullDevice::SetRenderState(RenderState state, bool enabled)
{
}

bool CNullDevice::GetRenderState(RenderState state)
{
    return false;
}

void CNullDevice::SetDepthTestFunc(CompFunc func)
{
}

CompFunc CNullDevice::GetDepthTestFunc()
{
    return COMP_FUNC_NEVER;
}

void CNullDevice::SetDepthBias(float factor)
{
}

float CNullDevice::GetDepthBias()
{
    return 0.0f;
}

void CNullDevice::SetAlphaTestFunc(CompFunc func, float refValue)
{
}

void CNullDevice::GetAlphaTestFunc(CompFunc &func, float &refValue)
{

    func = COMP_FUNC_NEVER;
    refValue = 0.0f;
}

void CNullDevice::SetBlendFunc(BlendFunc srcBlend, BlendFunc dstBlend)
{
}

void CNullDevice::GetBlendFunc(BlendFunc &srcBlend, BlendFunc &dstBlend)
{
    srcBlend = BLEND_ZERO;
    dstBlend = BLEND_ZERO;
}

void CNullDevice::SetClearColor(const Color &color)
{
}

Color CNullDevice::GetClearColor()
{
    return Color(0.0f, 0.0f, 0.0f, 0.0f);
}

void CNullDevice::SetGlobalAmbient(const Color &color)
{
}

Color CNullDevice::GetGlobalAmbient()
{
    return Color(0.0f, 0.0f, 0.0f, 0.0f);
}

void CNullDevice::SetFogParams(FogMode mode, const Color &color, float start, float end, float density)
{
}

void CNullDevice::GetFogParams(FogMode &mode, Color &color, float &start, float &end, float &density)
{
    //
}

void CNullDevice::SetCullMode(CullMode mode)
{
}

CullMode CNullDevice::GetCullMode()
{
    return CULL_CW;
}

void CNullDevice::SetShadeModel(ShadeModel model)
{
}

ShadeModel CNullDevice::GetShadeModel()
{
    return SHADE_FLAT;
}

void CNullDevice::SetFillMode(FillMode mode)
{
}

FillMode CNullDevice::GetFillMode()
{
    return FILL_POINT;
}

void* CNullDevice::GetFrameBufferPixels() const
{
    return nullptr;
}


} // namespace Gfx


