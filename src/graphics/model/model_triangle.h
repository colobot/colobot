/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2021, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#pragma once

#include "graphics/core/color.h"
#include "graphics/core/transparency.h"
#include "graphics/core/vertex.h"

namespace Gfx
{

enum class AlphaMode : unsigned char;

/**
 * \struct ModelTriangle
 * \brief A single triangle in mesh as saved in model file
 */
struct ModelTriangle
{
    //! 1st vertex
    Vertex3D  p1;
    //! 2nd vertex
    Vertex3D  p2;
    //! 3rd vertex
    Vertex3D  p3;
    //! Name of 1st texture
    std::string tex1Name;
    //! Name of 2nd texture
    std::string tex2Name;
    //! If true, 2nd texture will be taken from current engine setting
    bool variableTex2 = false;
    //! Whether to render as double-sided surface
    bool doubleSided = false;
    //! How to deal with texture transparency
    AlphaMode alphaMode = {};
    float alphaThreshold = 0.5f;
    //! Special marking
    std::string tag = "";
};

} // namespace Gfx
