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

// gldevice.cpp

#include "graphics/opengl/gldevice.h"

#include <GL/gl.h>
#include <GL/glu.h>


void Gfx::GLDeviceConfig::LoadDefault()
{
    Gfx::DeviceConfig::LoadDefault();

    hardwareAccel = true;

    redSize = 8;
    blueSize = 8;
    greenSize = 8;
    alphaSize = 8;
    depthSize = 24;
}

Gfx::CGLDevice::CGLDevice()
{
    m_renderState = 0;
}


Gfx::CGLDevice::~CGLDevice()
{
}

void Gfx::CGLDevice::Initialize()
{
    // TODO
}

void Gfx::CGLDevice::Destroy()
{
    // TODO
}

void Gfx::CGLDevice::BeginScene()
{
    // TODO
}

void Gfx::CGLDevice::EndScene()
{
    // TODO
}

void Gfx::CGLDevice::Clear()
{
    // TODO
}

void Gfx::CGLDevice::SetTransform(Gfx::TransformType type, const Math::Matrix &matrix)
{
    switch (type)
    {
        case Gfx::TRANSFORM_WORLD:
            m_worldMat = matrix;
            // TODO
            break;
        case Gfx::TRANSFORM_VIEW:
            m_viewMat = matrix;
            // TODO
            break;
        case Gfx::TRANSFORM_PROJECTION:
            m_projectionMat = matrix;
            // TODO
            break;
        default:
            assert(false);
    }
}

const Math::Matrix& Gfx::CGLDevice::GetTransform(Gfx::TransformType type)
{
    switch (type)
    {
        case Gfx::TRANSFORM_WORLD:
            return m_worldMat;
        case Gfx::TRANSFORM_VIEW:
            return m_viewMat;
        case Gfx::TRANSFORM_PROJECTION:
            return m_projectionMat;
        default:
            assert(false);
    }

    return m_worldMat; // to avoid warning
}

void Gfx::CGLDevice::MultiplyTransform(Gfx::TransformType type, const Math::Matrix &matrix)
{
    // TODO
}

void Gfx::CGLDevice::SetMaterial(const Gfx::Material &material)
{
    m_material = material;

    // TODO
}

const Gfx::Material& Gfx::CGLDevice::GetMaterial()
{
    return m_material;
}

int Gfx::CGLDevice::GetMaxLightCount()
{
    return m_lights.size();
}

void Gfx::CGLDevice::SetLight(int index, const Gfx::Light &light)
{
    assert(index >= 0);
    assert(index < (int)m_lights.size());

    m_lights[index] = light;

    // TODO
}

const Gfx::Light& Gfx::CGLDevice::GetLight(int index)
{
    assert(index >= 0);
    assert(index < (int)m_lights.size());

    return m_lights[index];
}

void Gfx::CGLDevice::SetLightEnabled(int index, bool enabled)
{
    assert(index >= 0);
    assert(index < (int)m_lightsEnabled.size());

    m_lightsEnabled[index] = enabled;

    // TODO
}

bool Gfx::CGLDevice::GetLightEnabled(int index)
{
    assert(index >= 0);
    assert(index < (int)m_lights.size());

    return m_lightsEnabled[index];
}

int Gfx::CGLDevice::GetMaxTextureCount()
{
    return m_textures.size();
}

const Gfx::Texture& Gfx::CGLDevice::GetTexture(int index)
{
    assert(index >= 0);
    assert(index < (int)m_textures.size());

    return m_textures[index];
}

void Gfx::CGLDevice::SetTexture(int index, const Gfx::Texture &texture)
{
    assert(index >= 0);
    assert(index < (int)m_textures.size());

    m_textures[index] = texture;

    // TODO
}

void Gfx::CGLDevice::SetRenderState(Gfx::RenderState state, bool enabled)
{
    // TODO
}

bool Gfx::CGLDevice::GetRenderState(Gfx::RenderState state)
{
    // TODO
    return false;
}

void Gfx::CGLDevice::DrawPrimitive(Gfx::PrimitiveType, Vertex *vertices, int vertexCount)
{
    // TODO
}

void Gfx::CGLDevice::DrawPrimitive(Gfx::PrimitiveType, VertexTex2 *vertices, int vertexCount)
{
    // TODO
}
