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

// vertex.h

#pragma once


#include "math/vector.h"
#include "math/point.h"

namespace Gfx {

/**
 * \struct Vertex Vertex of a primitive
 *
 * This structure was created as analog to DirectX's D3DVERTEX.
 *
 * It contains:
 *  - vertex coordinates (x,y,z) as Math::Vector,
 *  - normal coordinates (nx,ny,nz) as Math::Vector
 *  - texture coordinates (u,v) as Math::Point.
 */
struct Vertex
{
    Math::Vector coord;
    Math::Vector normal;
    Math::Point texCoord;

    Vertex(Math::Vector aCoord = Math::Vector(),
           Math::Vector aNormal = Math::Vector(),
           Math::Point aTexCoord = Math::Point())
        : coord(aCoord), normal(aNormal), texCoord(aTexCoord) {}
};

/**
 * \struct VertexTex2 Vertex with secondary texture coordinates
 *
 * In addition to fields from Gfx::Vector, it contains
 * secondary texture coordinates (u2, v2) as Math::Point
 */
struct VertexTex2 : public Gfx::Vertex
{
    Math::Point texCoord2;

    VertexTex2(Math::Vector aCoord = Math::Vector(),
               Math::Vector aNormal = Math::Vector(),
               Math::Point aTexCoord = Math::Point(),
               Math::Point aTexCoord2 = Math::Point())
        : Vertex(aCoord, aNormal, aTexCoord), texCoord2(aTexCoord2) {}
};

}; // namespace Gfx
