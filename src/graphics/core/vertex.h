/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
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

/**
 * \file graphics/core/vertex.h
 * \brief Vertex structs
 */

#pragma once


#include "graphics/core/color.h"
#include "graphics/core/type.h"

#include "math/point.h"
#include "math/vector.h"

#include <sstream>
#include <cstdint>


// Graphics module namespace
namespace Gfx
{


/**
* \struct VertexAttribute
* \brief Vertex attribute
*
* This structure contains parameters for a vertex attribute.
*/
struct VertexAttribute
{
    //! true enables vertex attribute
    bool enabled = false;
    //! true means normalized value (integer types only)
    bool normalized = false;
    //! Number of elements in the vertex attribute.
    //! Valid values are 1, 2, 3, and 4. Depends on specific attribute.
    unsigned char size = 0;
    //! Type of values in vertex attribute
    Type type = Type::UBYTE;
    //! Offset to the vertex attribute
    int offset = 0;
    //! Stride of vertex attribute
    int stride = 0;
    //! Default values used when attribute is disabled
    float values[4] = {0.0f, 0.0f, 0.0f, 0.0f};
};

/**
* \struct VertexFormat
* \brief Vertex format
*
* This structure defines vertex formats for generic vertex arrays.
*
* It contains:
*  - vertex coordinate specification
*  - color specification
*  - normal specification
*  - texture coordinate 1 specification
*  - texture coordinate 2 specification
*/
struct VertexFormat
{
    //! Vertex coordinate
    VertexAttribute vertex{};
    //! Color
    VertexAttribute color{};
    //! Normal
    VertexAttribute normal{};
    //! Texture coordinate 1
    VertexAttribute tex1{};
    //! Texture coordinate 2
    VertexAttribute tex2{};
};

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

    explicit Vertex(Math::Vector aCoord = Math::Vector(),
                    Math::Vector aNormal = Math::Vector(),
                    Math::Point aTexCoord = Math::Point())
        : coord(aCoord), normal(aNormal),
          texCoord(aTexCoord) {}


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
 * \brief Colored vertex
 *
 * It contains:
 *  - vertex coordinates (x,y,z) as Math::Vector,
 *  - RGBA color as Color
 */
struct VertexCol
{
    Math::Vector coord;
    Color color;

    VertexCol() = default;

    explicit VertexCol(Math::Vector aCoord,
                       Color aColor = Color())
        : coord(aCoord), color(aColor) {}

    //! Returns a string "(c: [...], col: [...])"
    inline std::string ToString() const
    {
        std::stringstream s;
        s.precision(3);
        s << "(c: " << coord.ToString() << ", col: " << color.ToString() << ")";
        return s.str();
    }
};


/**
 * \struct VertexTex2
 * \brief Vertex with secondary texture coordinates
 *
 * In addition to fields from Vector, it contains
 * secondary texture coordinates (u2, v2) as Math::Point
 */
struct VertexTex2
{
    Math::Vector coord;
    Math::Vector normal;
    Math::Point texCoord;
    Math::Point texCoord2;

    explicit VertexTex2(Math::Vector aCoord = Math::Vector(),
                        Math::Vector aNormal = Math::Vector(),
                        Math::Point aTexCoord = Math::Point(),
                        Math::Point aTexCoord2 = Math::Point())
        : coord(aCoord), normal(aNormal),
          texCoord(aTexCoord), texCoord2(aTexCoord2) {}

    //! Sets the fields from Vertex with texCoord2 = (0,0)
    void FromVertex(const Vertex &v)
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


} // namespace Gfx

