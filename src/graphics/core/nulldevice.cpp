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


#include "graphics/core/nulldevice.h"


// Graphics module namespace
namespace Gfx
{

CNullDevice::CNullDevice()
{
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

std::string CNullDevice::GetName()
{
    return std::string("Null Device");
}

bool CNullDevice::Create()
{
    return true;
}

void CNullDevice::Destroy()
{
}

void CNullDevice::ConfigChanged(const DeviceConfig &newConfig)
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

void CNullDevice::SetRenderMode(RenderMode mode)
{
}

void CNullDevice::SetTransform(TransformType type, const Math::Matrix &matrix)
{
}

void CNullDevice::SetMaterial(const Material &material)
{
}

int CNullDevice::GetMaxLightCount()
{
    return 99;
}

void CNullDevice::SetLight(int index, const Light &light)
{
}

void CNullDevice::SetLightEnabled(int index, bool enabled)
{
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

Texture CNullDevice::CreateDepthTexture(int width, int height, int depth)
{
    Texture tex;
    tex.id = 1; // tex.id = 0 => invalid texture
    return tex;
}

void CNullDevice::UpdateTexture(const Texture& texture, Math::IntPoint offset, ImageData* data, TexImgFormat format)
{
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

void CNullDevice::SetTextureEnabled(int index, bool enabled)
{
}

void CNullDevice::SetTextureStageParams(int index, const TextureStageParams &params)
{
}

void CNullDevice::SetTextureStageWrap(int index, TexWrapMode wrapS, TexWrapMode wrapT)
{
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

void CNullDevice::DrawPrimitive(PrimitiveType type, const void *vertices,
    int size, const VertexFormat &format, int vertexCount)
{
}

void CNullDevice::DrawPrimitives(PrimitiveType type, const void *vertices,
    int size, const VertexFormat &format, int first[], int count[], int drawCount)
{
}

void CNullDevice::DrawPrimitives(PrimitiveType type, const Vertex *vertices,
    int first[], int count[], int drawCount, Color color)
{
}

void CNullDevice::DrawPrimitives(PrimitiveType type, const VertexTex2 *vertices,
    int first[], int count[], int drawCount, Color color)
{
}

void CNullDevice::DrawPrimitives(PrimitiveType type, const VertexCol *vertices,
    int first[], int count[], int drawCount)
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

void CNullDevice::SetViewport(int x, int y, int width, int height)
{
}

void CNullDevice::SetRenderState(RenderState state, bool enabled)
{
}

void CNullDevice::SetColorMask(bool red, bool green, bool blue, bool alpha)
{
}

void CNullDevice::SetDepthTestFunc(CompFunc func)
{
}

void CNullDevice::SetDepthBias(float factor, float units)
{
}

void CNullDevice::SetAlphaTestFunc(CompFunc func, float refValue)
{
}

void CNullDevice::SetBlendFunc(BlendFunc srcBlend, BlendFunc dstBlend)
{
}

void CNullDevice::SetClearColor(const Color &color)
{
}

void CNullDevice::SetGlobalAmbient(const Color &color)
{
}

void CNullDevice::SetFogParams(FogMode mode, const Color &color, float start, float end, float density)
{
}

void CNullDevice::SetCullMode(CullMode mode)
{
}

void CNullDevice::SetShadeModel(ShadeModel model)
{
}

void CNullDevice::SetShadowColor(float value)
{
}

void CNullDevice::SetFillMode(FillMode mode)
{
}

void CNullDevice::CopyFramebufferToTexture(Texture& texture, int xOffset, int yOffset, int x, int y, int width, int height)
{
}

std::unique_ptr<CFrameBufferPixels> CNullDevice::GetFrameBufferPixels() const
{
    return nullptr;
}

CFramebuffer* CNullDevice::GetFramebuffer(std::string name)
{
    return nullptr;
}

CFramebuffer* CNullDevice::CreateFramebuffer(std::string name, const FramebufferParams& params)
{
    return nullptr;
}

void CNullDevice::DeleteFramebuffer(std::string name)
{
}

bool CNullDevice::IsAnisotropySupported()
{
    return false;
}

int CNullDevice::GetMaxAnisotropyLevel()
{
    return 1;
}

int CNullDevice::GetMaxSamples()
{
    return 1;
}

bool CNullDevice::IsShadowMappingSupported()
{
    return false;
}

int CNullDevice::GetMaxTextureSize()
{
    return 0;
}

bool CNullDevice::IsFramebufferSupported()
{
    return false;
}

} // namespace Gfx
