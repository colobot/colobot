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

// modelfile.h (aka modfile.h)

#include "graphics/common/engine.h"
#include "graphics/common/vertex.h"
#include "graphics/common/material.h"
#include "math/vector.h"

#include <string>
#include <vector>
#include <iostream>


class CInstanceManager;


namespace Gfx {

/**
  \struct ModelTriangle
  \brief Triangle of a 3D model
  */
struct ModelTriangle
{
    //! 1st vertex
    Gfx::VertexTex2  p1;
    //! 2nd vertex
    Gfx::VertexTex2  p2;
    //! 3rd vertex
    Gfx::VertexTex2  p3;
    //! Material
    Gfx::Material    material;
    //! Name of 1st texture
    std::string      tex1Name;
    //! Name of 2nd texture
    std::string      tex2Name;
    //! Min LOD threshold
    float            min;
    //! Max LOD threshold
    float            max;
    //! Rendering state to be set
    long             state;

    ModelTriangle();
};


/**
  \class CModelFile
  \brief Model file reader/writer

  Allows reading and writing model objects. Models are collections of ModelTriangle structs. */
class CModelFile
{
public:
    CModelFile(CInstanceManager* iMan);
    ~CModelFile();

    //! Returns the last error encountered
    std::string GetError();

    //! Reads a binary Colobot model from file
    bool                 ReadModel(const std::string &filename, bool edit = false, bool meta = true);
    //! Reads a binary Colobot model from stream
    bool                 ReadModel(std::istream &stream, bool edit = false, bool meta = true);
    //! Writes the model to Colobot binary model file
    bool                 WriteModel(const std::string &filename);
    //! Writes the model to Colobot binary model file
    bool                 WriteModel(std::ostream &stream);

    //! Reads a DXF model from file
    bool                 ReadDXF(const std::string &filename, float min, float max);
    //! Reads a DXF model from stream
    bool                 ReadDXF(std::istream &stream, float min, float max);

    //! Returns the number of triangles in model
    int                  GetTriangleCount();
    //! Returns the height of model -- closest point to X and Z coords of \a pos
    float                GetHeight(Math::Vector pos);

    //! Mirrors the model along the Z axis
    void                 Mirror();

    //! Creates an object in the graphics engine from the model
    bool                 CreateEngineObject(int objRank, int addState = 0);

protected:
    //! Adds a triangle to the list
    void                 CreateTriangle(Math::Vector p1, Math::Vector p2, Math::Vector p3, float min, float max);

protected:
    CInstanceManager*    m_iMan;
    CEngine*             m_engine;

    //! Last error
    std::string m_error;

    //! Model triangles
    std::vector<Gfx::ModelTriangle> m_triangles;
};

}; // namespace Gfx
