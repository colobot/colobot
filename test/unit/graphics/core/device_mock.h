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
#pragma once

#include "graphics/core/device.h"

#include <gmock/gmock.h>

class CDeviceMock : public Gfx::CDevice
{
public:
    CDeviceMock() {}

    MOCK_METHOD0(DebugHook, void());
    MOCK_METHOD0(DebugLights, void());

    MOCK_METHOD0(Create, bool());
    MOCK_METHOD0(Destroy, void());

    MOCK_METHOD0(BeginScene, void());
    MOCK_METHOD0(EndScene, void());

    MOCK_METHOD0(Clear, void());

    MOCK_METHOD2(SetTransform, void(Gfx::TransformType type, const Math::Matrix &matrix));

    MOCK_METHOD1(SetMaterial, void(const Gfx::Material &material));

    MOCK_METHOD0(GetMaxLightCount, int());

    MOCK_METHOD2(SetLight, void(int index, const Gfx::Light &light));
    MOCK_METHOD1(GetLight, const Gfx::Light&(int index));

    MOCK_METHOD2(SetLightEnabled, void(int index, bool enabled));

    MOCK_METHOD2(CreateTexture, Gfx::Texture(CImage *image, const Gfx::TextureCreateParams &params));
    MOCK_METHOD2(CreateTexture, Gfx::Texture(ImageData *data, const Gfx::TextureCreateParams &params));

    MOCK_METHOD1(DestroyTexture, void(const Gfx::Texture &texture));
    MOCK_METHOD0(DestroyAllTextures, void());

    MOCK_METHOD0(GetMaxTextureStageCount, int());

    MOCK_METHOD2(SetTexture, void(int index, const Gfx::Texture &texture));
    MOCK_METHOD2(SetTexture, void(int index, unsigned int textureId));

    MOCK_METHOD2(SetTextureEnabled, void(int index, bool enabled));

    MOCK_METHOD2(SetTextureStageParams, void(int index, const Gfx::TextureStageParams &params));

    MOCK_METHOD3(SetTextureStageWrap, void(int index, Gfx::TexWrapMode wrapS, Gfx::TexWrapMode wrapT));

    MOCK_METHOD4(DrawPrimitive, void(Gfx::PrimitiveType type, const Gfx::Vertex *vertices, int vertexCount, Gfx::Color color));
    MOCK_METHOD4(DrawPrimitive, void(Gfx::PrimitiveType type, const Gfx::VertexTex2 *vertices, int vertexCount, Gfx::Color color));
    MOCK_METHOD3(DrawPrimitive, void(Gfx::PrimitiveType type, const Gfx::VertexCol *vertices, int vertexCount));

    MOCK_METHOD3(CreateStaticBuffer, unsigned int(Gfx::PrimitiveType primitiveType, const Gfx::Vertex* vertices, int vertexCount));
    MOCK_METHOD3(CreateStaticBuffer, unsigned int(Gfx::PrimitiveType primitiveType, const Gfx::VertexTex2* vertices, int vertexCount));
    MOCK_METHOD3(CreateStaticBuffer, unsigned int(Gfx::PrimitiveType primitiveType, const Gfx::VertexCol* vertices, int vertexCount));

    MOCK_METHOD4(UpdateStaticBuffer, void(unsigned int bufferId, Gfx::PrimitiveType primitiveType, const Gfx::Vertex* vertices, int vertexCount));
    MOCK_METHOD4(UpdateStaticBuffer, void(unsigned int bufferId, Gfx::PrimitiveType primitiveType, const Gfx::VertexTex2* vertices, int vertexCount));
    MOCK_METHOD4(UpdateStaticBuffer, void(unsigned int bufferId, Gfx::PrimitiveType primitiveType, const Gfx::VertexCol* vertices, int vertexCount));

    MOCK_METHOD1(DrawStaticBuffer, void(unsigned int bufferId));

    MOCK_METHOD1(DestroyStaticBuffer, void(unsigned int bufferId));

    MOCK_METHOD2(ComputeSphereVisibility, int(const Math::Vector &center, float radius));

    MOCK_METHOD2(SetRenderState, void(Gfx::RenderState state, bool enabled));
    MOCK_METHOD1(GetRenderState, bool(Gfx::RenderState state));

    MOCK_METHOD1(SetDepthTestFunc, void(Gfx::CompFunc func));

    MOCK_METHOD1(SetDepthBias, void(float factor));

    MOCK_METHOD2(SetAlphaTestFunc, void(Gfx::CompFunc func, float refValue));

    MOCK_METHOD2(SetBlendFunc, void(Gfx::BlendFunc srcBlend, Gfx::BlendFunc dstBlend));

    MOCK_METHOD1(SetClearColor, void(const Gfx::Color &color));

    MOCK_METHOD1(SetGlobalAmbient, void(const Gfx::Color &color));

    MOCK_METHOD5(SetFogParams, void(Gfx::FogMode mode, const Gfx::Color &color, float start, float end, float density));

    MOCK_METHOD1(SetCullMode, void(Gfx::CullMode mode));

    MOCK_METHOD1(SetShadeModel, void(Gfx::ShadeModel model));

    MOCK_METHOD1(SetFillMode, void(Gfx::FillMode mode));

    MOCK_CONST_METHOD0(GetFrameBufferPixels, void*());
};
