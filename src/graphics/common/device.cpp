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

// device.cpp


#include "graphics/common/device.h"


void Gfx::DeviceConfig::LoadDefault()
{
    width = 800;
    height = 600;
    bpp = 32;
    fullScreen = false;
    resizeable = false;
    doubleBuf = true;
    noFrame = false;
}

void Gfx::TextureParams::LoadDefault()
{
    minFilter = Gfx::TEX_MIN_FILTER_NEAREST;
    magFilter = Gfx::TEX_MAG_FILTER_NEAREST;

    wrapS = Gfx::TEX_WRAP_REPEAT;
    wrapT = Gfx::TEX_WRAP_REPEAT;

    colorOperation = Gfx::TEX_MIX_OPER_MODULATE;
    colorArg1 = Gfx::TEX_MIX_ARG_CURRENT;
    colorArg2 = Gfx::TEX_MIX_ARG_TEXTURE;

    alphaOperation = Gfx::TEX_MIX_OPER_MODULATE;
    alphaArg1 = Gfx::TEX_MIX_ARG_CURRENT;
    alphaArg2 = Gfx::TEX_MIX_ARG_TEXTURE;
}
