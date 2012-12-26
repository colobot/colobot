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

/**
 * \file graphics/engine/modelfile.h
 * \brief Model loading - CModelFile class (aka modfile)
 */

#pragma once


#include "graphics/core/vertex.h"
#include "graphics/core/material.h"

#include "math/vector.h"

#include <string>
#include <vector>
#include <iostream>



// Graphics module namespace
namespace Gfx {


/**
  \struct ModelTriangle
  \brief Triangle of a 3D model
  */
struct ModelTriangle
{
    //! 1st vertex
    VertexTex2  p1;
    //! 2nd vertex
    VertexTex2  p2;
    //! 3rd vertex
    VertexTex2  p3;
    //! Material
    Material    material;
    //! Name of 1st texture
    std::string      tex1Name;
    //! Name of 2nd texture
    std::string      tex2Name;
    //! If true, 2nd texture will be taken from current engine setting
    bool             variableTex2;
    //! Min LOD threshold
    float            min;
    //! Max LOD threshold
    float            max;
    //! Rendering state to be set
    int              state;

    ModelTriangle()
    {
        variableTex2 = true;
        min = max = 0.0f;
        state = 0;
    }
};


/**
 * \class CModelFile
 * \brief Model file reader/writer
 *
 * Allows reading and writing model objects. Models are collections of ModelTriangle structs.
 */
class CModelFile
{
public:
    CModelFile();
    ~CModelFile();

    //! Reads a model in text format from file
    bool                 ReadTextModel(const std::string &fileName);
    //! Reads a model in text format from stream
    bool                 ReadTextModel(std::istream &stream);

    //! Writes the model in text format to a file
    bool                 WriteTextModel(const std::string &fileName);
    //! Writes the model in text format to a stream
    bool                 WriteTextModel(std::ostream &stream);

    //! Reads a model in new binary format from file
    bool                 ReadBinaryModel(const std::string &fileName);
    //! Reads a model in new binary format from stream
    bool                 ReadBinaryModel(std::istream &stream);

    //! Writes the model in binary format to a file
    bool                 WriteBinaryModel(const std::string &fileName);
    //! Writes the model in binary format to a stream
    bool                 WriteBinaryModel(std::ostream &stream);

    //! Reads a binary Colobot model from file
    //! @deprecated
    bool                 ReadModel(const std::string &fileName);
    //! Reads a binary Colobot model from stream
    //! @deprecated
    bool                 ReadModel(std::istream &stream);
    //! Writes the model to Colobot binary model file
    //! @deprecated
    bool                 WriteModel(const std::string &fileName);
    //! Writes the model to Colobot binary model file
    //! @deprecated
    bool                 WriteModel(std::ostream &stream);

    //! Returns the number of triangles in model
    int                  GetTriangleCount();

    //! Returns the triangle vector
    const std::vector<ModelTriangle>& GetTriangles();

protected:
    //! Adds a triangle to the list
    void                 CreateTriangle(Math::Vector p1, Math::Vector p2, Math::Vector p3, float min, float max);

protected:
    //! Model triangles
    std::vector<ModelTriangle> m_triangles;
};

}; // namespace Gfx
