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

// lightning.cpp (aka blitz.cpp)

#include "graphics/engine/lightning.h"

#include "common/logger.h"


Gfx::CLightning::CLightning(CInstanceManager* iMan, Gfx::CEngine* engine)
{
    GetLogger()->Trace("CLightning::CLightning() stub!\n");
    // TODO!
}

Gfx::CLightning::~CLightning()
{
    GetLogger()->Trace("CLightning::~CLightning() stub!\n");
    // TODO!
}

void Gfx::CLightning::Flush()
{
    GetLogger()->Trace("CLightning::Flush() stub!\n");
    // TODO!
}

bool Gfx::CLightning::EventProcess(const Event &event)
{
    GetLogger()->Trace("CLightning::EventProcess() stub!\n");
    // TODO!
    return true;
}

bool Gfx::CLightning::Create(float sleep, float delay, float magnetic)
{
    GetLogger()->Trace("CLightning::Create() stub!\n");
    // TODO!
    return true;
}

bool Gfx::CLightning::GetStatus(float &sleep, float &delay, float &magnetic, float &progress)
{
    GetLogger()->Trace("CLightning::GetStatus() stub!\n");
    // TODO!
    return true;
}

bool Gfx::CLightning::SetStatus(float sleep, float delay, float magnetic, float progress)
{
    GetLogger()->Trace("CLightning::SetStatus() stub!\n");
    // TODO!
    return true;
}

void Gfx::CLightning::Draw()
{
    GetLogger()->Trace("CLightning::Draw() stub!\n");
    // TODO!
}

bool Gfx::CLightning::EventFrame(const Event &event)
{
    GetLogger()->Trace("CLightning::EventFrame() stub!\n");
    // TODO!
    return true;
}

CObject* Gfx::CLightning::SearchObject(Math::Vector pos)
{
    GetLogger()->Trace("CLightning::SearchObject() stub!\n");
    // TODO!
    return nullptr;
}
