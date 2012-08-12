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

/**
 * \file graphics/core/vertex.h
 * \brief Vertex structs
 */

#pragma once


#include "graphics/core/color.h"
#include "math/vector.h"
#include "math/point.h"

#include <sstream>

namespace Gfx {

/**
 * \struct Vertex
 * \brief Vertex of a primitive
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


    //! Returns a string "(c: [...], n: [...], tc: [...])"
    inline std::string ToString() const
    {
        std::stringstream s;
        s.precision(3);
        s << "(c: " << coord.ToString() << ", n: " << normal.ToString()
          << ", tc: " << texCoord.ToString() << ")";
        return s.str();
    }
};

/**
 * \struct VertexCol
 * \brief Vertex with color information
 *
 * This structure was created as analog to DirectX's D3DLVERTEX.
 *
 * It contains:
 *  - vertex coordinates (x,y,z) as Math::Vector,
 *  - RGBA color as Gfx::Color,
 *  - RGBA specular color as Gfx::Color,
 *  - texture coordinates (u,v) as Math::Point.
 */
struct VertexCol
{
    Math::Vector coord;
    Gfx::Color color;
    Gfx::Color specular;
    Math::Point texCoord;

    VertexCol(Math::Vector aCoord = Math::Vector(),
              Gfx::Color aColor = Gfx::Color(),
              Gfx::Color aSpecular = Gfx::Color(),
              Math::Point aTexCoord = Math::Point())
        : coord(aCoord), color(aColor), specular(aSpecular), texCoord(aTexCoord) {}

    //! Returns a string "(c: [...], col: [...], sp: [...], tc: [...])"
    inline std::string ToString() const
    {
        std::stringstream s;
        s.precision(3);
        s << "(c: " << coord.ToString() << ", col: " << color.ToString() << ", sp: "
          << specular.ToString() << ", tc: " << texCoord.ToString() << ")";
        return s.str();
    }
};


/**
 * \struct VertexTex2
 * \brief Vertex with secondary texture coordinates
 *
 * In addition to fields from Gfx::Vector, it contains
 * secondary texture coordinates (u2, v2) as Math::Point
 */
struct VertexTex2
{
    Math::Vector coord;
    Math::Vector normal;
    Math::Point texCoord;
    Math::Point texCoord2;

    VertexTex2(Math::Vector aCoord = Math::Vector(),
               Math::Vector aNormal = Math::Vector(),
               Math::Point aTexCoord = Math::Point(),
               Math::Point aTexCoord2 = Math::Point())
        : coord(aCoord), normal(aNormal), texCoord(aTexCoord), texCoord2(aTexCoord2) {}

    //! Sets the fields from Gfx::Vertex with texCoord2 = (0,0)
    void FromVertex(const Gfx::Vertex &v)
    {
        coord = v.coord;
        normal = v.normal;
        texCoord = v.texCoord;
        texCoord2 = Math::Point();
    }

    //! Returns a string "(c: [...], n: [...], tc: [...], tc2: [...])"
    inline std::string ToString() const
    {
        std::stringstream s;
        s.precision(3);
        s << "(c: " << coord.ToString() << ", n: " << normal.ToString()
          << ", tc: " << texCoord.ToString() << ", tc2: " << texCoord2.ToString() << ")";
        return s.str();
    }
};

}; // namespace Gfx
