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
    MOCK_METHOD1(GetTransform, const Math::Matrix& (Gfx::TransformType type));
    MOCK_METHOD2(MultiplyTransform, void(Gfx::TransformType type, const Math::Matrix &matrix));

    MOCK_METHOD1(SetMaterial, void(const Gfx::Material &material));
    MOCK_METHOD0(GetMaterial, const Gfx::Material&());

    MOCK_METHOD0(GetMaxLightCount, int());

    MOCK_METHOD2(SetLight, void(int index, const Gfx::Light &light));
    MOCK_METHOD1(GetLight, const Gfx::Light&(int index));

    MOCK_METHOD2(SetLightEnabled, void(int index, bool enabled));
    MOCK_METHOD1(GetLightEnabled, bool(int index));

    MOCK_METHOD2(CreateTexture, Gfx::Texture(CImage *image, const Gfx::TextureCreateParams &params));
    MOCK_METHOD2(CreateTexture, Gfx::Texture(ImageData *data, const Gfx::TextureCreateParams &params));

    MOCK_METHOD1(DestroyTexture, void(const Gfx::Texture &texture));
    MOCK_METHOD0(DestroyAllTextures, void());

    MOCK_METHOD0(GetMaxTextureStageCount, int());

    MOCK_METHOD2(SetTexture, void(int index, const Gfx::Texture &texture));
    MOCK_METHOD2(SetTexture, void(int index, unsigned int textureId));
    MOCK_METHOD1(GetTexture, Gfx::Texture(int index));

    MOCK_METHOD2(SetTextureEnabled, void(int index, bool enabled));
    MOCK_METHOD1(GetTextureEnabled, bool(int index));

    MOCK_METHOD2(SetTextureStageParams, void(int index, const Gfx::TextureStageParams &params));
    MOCK_METHOD1(GetTextureStageParams, Gfx::TextureStageParams(int index));

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
    MOCK_METHOD0(GetDepthTestFunc, Gfx::CompFunc());

    MOCK_METHOD1(SetDepthBias, void(float factor));
    MOCK_METHOD0(GetDepthBias, float());

    MOCK_METHOD2(SetAlphaTestFunc, void(Gfx::CompFunc func, float refValue));
    MOCK_METHOD2(GetAlphaTestFunc, void(Gfx::CompFunc &func, float &refValue));

    MOCK_METHOD2(SetBlendFunc, void(Gfx::BlendFunc srcBlend, Gfx::BlendFunc dstBlend));
    MOCK_METHOD2(GetBlendFunc, void(Gfx::BlendFunc &srcBlend, Gfx::BlendFunc &dstBlend));

    MOCK_METHOD1(SetClearColor, void(const Gfx::Color &color));
    MOCK_METHOD0(GetClearColor, Gfx::Color());

    MOCK_METHOD1(SetGlobalAmbient, void(const Gfx::Color &color));
    MOCK_METHOD0(GetGlobalAmbient, Gfx::Color());

    MOCK_METHOD5(SetFogParams, void(Gfx::FogMode mode, const Gfx::Color &color, float start, float end, float density));
    MOCK_METHOD5(GetFogParams, void(Gfx::FogMode &mode, Gfx::Color &color, float &start, float &end, float &density));

    MOCK_METHOD1(SetCullMode, void(Gfx::CullMode mode));
    MOCK_METHOD0(GetCullMode, Gfx::CullMode());

    MOCK_METHOD1(SetShadeModel, void(Gfx::ShadeModel model));
    MOCK_METHOD0(GetShadeModel, Gfx::ShadeModel());

    MOCK_METHOD1(SetFillMode, void(Gfx::FillMode mode));
    MOCK_METHOD0(GetFillMode, Gfx::FillMode());
};
