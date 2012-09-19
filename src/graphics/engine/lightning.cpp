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


#include "graphics/engine/lightning.h"

#include "common/logger.h"


// Graphics module namespace
namespace Gfx {


CLightning::CLightning(CInstanceManager* iMan, CEngine* engine)
{
    GetLogger()->Trace("CLightning::CLightning() stub!\n");
    // TODO!
}

CLightning::~CLightning()
{
    GetLogger()->Trace("CLightning::~CLightning() stub!\n");
    // TODO!
}

void CLightning::Flush()
{
    GetLogger()->Trace("CLightning::Flush() stub!\n");
    // TODO!
}

bool CLightning::EventProcess(const Event &event)
{
    GetLogger()->Trace("CLightning::EventProcess() stub!\n");
    // TODO!
    return true;
}

bool CLightning::Create(float sleep, float delay, float magnetic)
{
    GetLogger()->Trace("CLightning::Create() stub!\n");
    // TODO!
    return true;
}

bool CLightning::GetStatus(float &sleep, float &delay, float &magnetic, float &progress)
{
    GetLogger()->Trace("CLightning::GetStatus() stub!\n");
    // TODO!
    return true;
}

bool CLightning::SetStatus(float sleep, float delay, float magnetic, float progress)
{
    GetLogger()->Trace("CLightning::SetStatus() stub!\n");
    // TODO!
    return true;
}

void CLightning::Draw()
{
    GetLogger()->Trace("CLightning::Draw() stub!\n");
    // TODO!
}

bool CLightning::EventFrame(const Event &event)
{
    GetLogger()->Trace("CLightning::EventFrame() stub!\n");
    // TODO!
    return true;
}

CObject* CLightning::SearchObject(Math::Vector pos)
{
    GetLogger()->Trace("CLightning::SearchObject() stub!\n");
    // TODO!
    return nullptr;
}


} // namespace Gfx
