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

// pyro.cpp

#include "graphics/engine/pyro.h"

#include "common/logger.h"


Gfx::CPyro::CPyro(CInstanceManager* iMan)
{
    GetLogger()->Trace("CParticle::CPyro() stub!\n");
    // TODO!
}

Gfx::CPyro::~CPyro()
{
    GetLogger()->Trace("CPyro::~CPyro() stub!");
    // TODO!
}

void Gfx::CPyro::DeleteObject(bool all)
{
    GetLogger()->Trace("CPyro::DeleteObject() stub!");
    // TODO!
}

bool Gfx::CPyro::Create(Gfx::PyroType type, CObject* pObj, float force)
{
    GetLogger()->Trace("CPyro::Create() stub!");
    // TODO!
    return true;
}

bool Gfx::CPyro::EventProcess(const Event &event)
{
    GetLogger()->Trace("CPyro::EventProcess() stub!\n");
    // TODO!
    return true;
}

Error Gfx::CPyro::IsEnded()
{
    GetLogger()->Trace("CPyro::IsEnded() stub!\n");
    // TODO!
    return ERR_OK;
}

void Gfx::CPyro::CutObjectLink(CObject* pObj)
{
    GetLogger()->Trace("CPyro::CutObjectLink() stub!\n");
    // TODO!
}

void Gfx::CPyro::DisplayError(PyroType type, CObject* pObj)
{
    GetLogger()->Trace("CPyro::DisplayError() stub!\n");
    // TODO!
}

bool Gfx::CPyro::CreateLight(Math::Vector pos, float height)
{
    GetLogger()->Trace("CPyro::CreateLight() stub!\n");
    // TODO!
    return true;
}

void Gfx::CPyro::DeleteObject(bool primary, bool secondary)
{
    GetLogger()->Trace("CPyro::DeleteObject() stub!\n");
    // TODO!
}

void Gfx::CPyro::CreateTriangle(CObject* pObj, ObjectType oType, int part)
{
    GetLogger()->Trace("CPyro::CreateTriangle() stub!\n");
    // TODO!
}

void Gfx::CPyro::ExploStart()
{
    GetLogger()->Trace("CPyro::ExploStart() stub!\n");
    // TODO!
}
void Gfx::CPyro::ExploTerminate()
{
    GetLogger()->Trace("CPyro::ExploTerminate() stub!\n");
    // TODO!
}

void Gfx::CPyro::BurnStart()
{
    GetLogger()->Trace("CPyro::BurnStart() stub!\n");
    // TODO!
}

void Gfx::CPyro::BurnAddPart(int part, Math::Vector pos, Math::Vector angle)
{
    GetLogger()->Trace("CPyro::BurnAddPart() stub!\n");
    // TODO!
}

void Gfx::CPyro::BurnProgress()
{
    GetLogger()->Trace("CPyro::BurnProgress() stub!\n");
    // TODO!
}

bool Gfx::CPyro::BurnIsKeepPart(int part)
{
    GetLogger()->Trace("CPyro::BurnIsKeepPart() stub!\n");
    // TODO!
    return true;
}

void Gfx::CPyro::BurnTerminate()
{
    GetLogger()->Trace("CPyro::BurnTerminate() stub!\n");
    // TODO!
}

void Gfx::CPyro::FallStart()
{
    GetLogger()->Trace("CPyro::FallStart() stub!\n");
    // TODO!
}

CObject* Gfx::CPyro::FallSearchBeeExplo()
{
    GetLogger()->Trace("CPyro::FallSearchBeeExplo() stub!\n");
    // TODO!
    return nullptr;
}

void Gfx::CPyro::FallProgress(float rTime)
{
    GetLogger()->Trace("CPyro::FallProgress() stub!\n");
    // TODO!
}

Error Gfx::CPyro::FallIsEnded()
{
    GetLogger()->Trace("CPyro::FallIsEnded() stub!\n");
    // TODO!
    return ERR_OK;
}

void Gfx::CPyro::LightOperFlush()
{
    GetLogger()->Trace("CPyro::LightOperFlush() stub!\n");
    // TODO!
}

void Gfx::CPyro::LightOperAdd(float progress, float intensity, float r, float g, float b)
{
    GetLogger()->Trace("CPyro::LightOperAdd() stub!\n");
    // TODO!
}

void Gfx::CPyro::LightOperFrame(float rTime)
{
    GetLogger()->Trace("CPyro::LightOperFrame() stub!\n");
    // TODO!
}
