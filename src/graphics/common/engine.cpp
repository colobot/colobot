// * This file is part of the COLOBOT source code
// * Copyright (C) 2001-2008, Daniel ROUX & EPSITEC SA, www.epsitec.ch
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

// engine.cpp

#include "graphics/common/engine.h"

#include "graphics/common/device.h"
#include "math/geometry.h"



Gfx::CEngine::CEngine(CInstanceManager *iMan, CApplication *app)
{
    m_iMan   = iMan;
    m_app    = app;
    m_device = NULL;

    m_wasInit = false;

    // TODO
}

Gfx::CEngine::~CEngine()
{
    m_iMan   = NULL;
    m_app    = NULL;
    m_device = NULL;

    // TODO
}

bool Gfx::CEngine::GetWasInit()
{
    return m_wasInit;
}

std::string Gfx::CEngine::GetError()
{
    return m_error;
}

bool Gfx::CEngine::BeforeCreateInit()
{
    // TODO
    return true;
}

bool Gfx::CEngine::Create()
{
    m_wasInit = true;

    // TODO

    return true;
}

void Gfx::CEngine::Destroy()
{
    // TODO

    m_wasInit = false;
}

void Gfx::CEngine::SetDevice(Gfx::CDevice *device)
{
    m_device = device;
}

Gfx::CDevice* Gfx::CEngine::GetDevice()
{
    return m_device;
}

bool Gfx::CEngine::AfterDeviceSetInit()
{
    m_device->SetClearColor(Gfx::Color(0.0f, 0.0f, 0.0f, 0.0f));

    // TODO

    return true;
}

bool Gfx::CEngine::Render()
{
    m_device->BeginScene();

    Math::Matrix world;
    world.LoadIdentity();
    m_device->SetTransform(Gfx::TRANSFORM_WORLD, world);

    Math::Matrix view;
    view.LoadIdentity();
    m_device->SetTransform(Gfx::TRANSFORM_VIEW, view);

    Math::Matrix proj;
    Math::LoadOrthoProjectionMatrix(proj, -10.0f, 10.0f, -10.0f, 10.0f);
    m_device->SetTransform(Gfx::TRANSFORM_PROJECTION, proj);

    Gfx::VertexCol vertices[3] =
    {
        Gfx::VertexCol(Math::Vector(-2.0f, -1.0f, 0.0f), Gfx::Color(1.0f, 0.0f, 0.0f)),
        Gfx::VertexCol(Math::Vector( 2.0f, -1.0f, 0.0f), Gfx::Color(0.0f, 1.0f, 0.0f)),
        Gfx::VertexCol(Math::Vector( 0.0f,  1.5f, 0.0f), Gfx::Color(0.0f, 0.0f, 1.0f))
    };

    m_device->DrawPrimitive(Gfx::PRIMITIVE_TRIANGLES, vertices, 3);

    m_device->EndScene();

    return true;
}


