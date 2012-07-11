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

// modelfile.cpp (aka modfile.cpp)

#include "graphics/common/modelfile.h"

#include "common/iman.h"
#include "common/ioutils.h"
#include "common/stringutils.h"
#include "math/geometry.h"

#include <string.h>

#include <fstream>


//! How big the triangle vector is by default
const int TRIANGLE_PREALLOCATE_COUNT = 2000;

/**
  \struct ModelHeader
  \brief  Header info for model file
 */
struct ModelHeader
{
    //! Revision number
    int     revision;
    //! Version number
    int     version;
    //! Total number of vertices
    int     totalVertices;
    //! Reserved area
    int     reserved[10];

    ModelHeader()
    {
        memset(this, 0, sizeof(*this));
    }
};


struct OldModelTriangle1
{
    char            used;
    char            selected;
    Gfx::Vertex     p1;
    Gfx::Vertex     p2;
    Gfx::Vertex     p3;
    Gfx::Material   material;
    char            texName[20];
    float           min;
    float           max;

    OldModelTriangle1()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct OldModelTriangle2
{
    char            used;
    char            selected;
    Gfx::Vertex     p1;
    Gfx::Vertex     p2;
    Gfx::Vertex     p3;
    Gfx::Material   material;
    char            texName[20];
    float           min;
    float           max;
    long            state;
    short           reserved1;
    short           reserved2;
    short           reserved3;
    short           reserved4;
    OldModelTriangle2()
    {
        memset(this, 0, sizeof(*this));
    }
};


struct NewModelTriangle
{
    char            used;
    char            selected;
    Gfx::VertexTex2 p1;
    Gfx::VertexTex2 p2;
    Gfx::VertexTex2 p3;
    Gfx::Material   material;
    char            texName[20];
    float           min;
    float           max;
    long            state;
    short           texNum2;
    short           reserved2;
    short           reserved3;
    short           reserved4;

    NewModelTriangle()
    {
        memset(this, 0, sizeof(*this));
    }
};


Gfx::Vertex ReadBinaryVertex(std::istream &stream)
{
    Gfx::Vertex result;

    result.coord.x     = IOUtils::ReadBinaryFloat(stream);
    result.coord.y     = IOUtils::ReadBinaryFloat(stream);
    result.coord.z     = IOUtils::ReadBinaryFloat(stream);
    result.normal.x    = IOUtils::ReadBinaryFloat(stream);
    result.normal.y    = IOUtils::ReadBinaryFloat(stream);
    result.normal.z    = IOUtils::ReadBinaryFloat(stream);
    result.texCoord.x  = IOUtils::ReadBinaryFloat(stream);
    result.texCoord.y  = IOUtils::ReadBinaryFloat(stream);

    return result;
}

void WriteBinaryVertex(Gfx::Vertex vertex, std::ostream &stream)
{
    IOUtils::WriteBinaryFloat(vertex.coord.x, stream);
    IOUtils::WriteBinaryFloat(vertex.coord.y, stream);
    IOUtils::WriteBinaryFloat(vertex.coord.z, stream);
    IOUtils::WriteBinaryFloat(vertex.normal.x, stream);
    IOUtils::WriteBinaryFloat(vertex.normal.y, stream);
    IOUtils::WriteBinaryFloat(vertex.normal.z, stream);
    IOUtils::WriteBinaryFloat(vertex.texCoord.x, stream);
    IOUtils::WriteBinaryFloat(vertex.texCoord.y, stream);
}

Gfx::VertexTex2 ReadBinaryVertexTex2(std::istream &stream)
{
    Gfx::VertexTex2 result;

    result.coord.x     = IOUtils::ReadBinaryFloat(stream);
    result.coord.y     = IOUtils::ReadBinaryFloat(stream);
    result.coord.z     = IOUtils::ReadBinaryFloat(stream);
    result.normal.x    = IOUtils::ReadBinaryFloat(stream);
    result.normal.y    = IOUtils::ReadBinaryFloat(stream);
    result.normal.z    = IOUtils::ReadBinaryFloat(stream);
    result.texCoord.x  = IOUtils::ReadBinaryFloat(stream);
    result.texCoord.y  = IOUtils::ReadBinaryFloat(stream);
    result.texCoord2.x = IOUtils::ReadBinaryFloat(stream);
    result.texCoord2.y = IOUtils::ReadBinaryFloat(stream);

    return result;
}

void WriteBinaryVertexTex2(Gfx::VertexTex2 vertex, std::ostream &stream)
{
    IOUtils::WriteBinaryFloat(vertex.coord.x, stream);
    IOUtils::WriteBinaryFloat(vertex.coord.y, stream);
    IOUtils::WriteBinaryFloat(vertex.coord.z, stream);
    IOUtils::WriteBinaryFloat(vertex.normal.x, stream);
    IOUtils::WriteBinaryFloat(vertex.normal.y, stream);
    IOUtils::WriteBinaryFloat(vertex.normal.z, stream);
    IOUtils::WriteBinaryFloat(vertex.texCoord.x, stream);
    IOUtils::WriteBinaryFloat(vertex.texCoord.y, stream);
    IOUtils::WriteBinaryFloat(vertex.texCoord2.x, stream);
    IOUtils::WriteBinaryFloat(vertex.texCoord2.y, stream);
}

Gfx::Material ReadBinaryMaterial(std::istream &stream)
{
    Gfx::Material result;

    result.diffuse.r =  IOUtils::ReadBinaryFloat(stream);
    result.diffuse.g =  IOUtils::ReadBinaryFloat(stream);
    result.diffuse.b =  IOUtils::ReadBinaryFloat(stream);
    result.diffuse.a =  IOUtils::ReadBinaryFloat(stream);

    result.ambient.r =  IOUtils::ReadBinaryFloat(stream);
    result.ambient.g =  IOUtils::ReadBinaryFloat(stream);
    result.ambient.b =  IOUtils::ReadBinaryFloat(stream);
    result.ambient.a =  IOUtils::ReadBinaryFloat(stream);

    result.specular.r = IOUtils::ReadBinaryFloat(stream);
    result.specular.g = IOUtils::ReadBinaryFloat(stream);
    result.specular.b = IOUtils::ReadBinaryFloat(stream);
    result.specular.a = IOUtils::ReadBinaryFloat(stream);

    /* emissive.r = */  IOUtils::ReadBinaryFloat(stream);
    /* emissive.g = */  IOUtils::ReadBinaryFloat(stream);
    /* emissive.b = */  IOUtils::ReadBinaryFloat(stream);
    /* emissive.a = */  IOUtils::ReadBinaryFloat(stream);

    /* power = */       IOUtils::ReadBinaryFloat(stream);

    /* padding? */      IOUtils::ReadBinary<2, unsigned int>(stream);

    return result;
}

void WriteBinaryMaterial(Gfx::Material material, std::ostream &stream)
{
    IOUtils::WriteBinaryFloat(material.diffuse.r, stream);
    IOUtils::WriteBinaryFloat(material.diffuse.g, stream);
    IOUtils::WriteBinaryFloat(material.diffuse.b, stream);
    IOUtils::WriteBinaryFloat(material.diffuse.a, stream);

    IOUtils::WriteBinaryFloat(material.ambient.r, stream);
    IOUtils::WriteBinaryFloat(material.ambient.g, stream);
    IOUtils::WriteBinaryFloat(material.ambient.b, stream);
    IOUtils::WriteBinaryFloat(material.ambient.a, stream);

    IOUtils::WriteBinaryFloat(material.specular.r, stream);
    IOUtils::WriteBinaryFloat(material.specular.g, stream);
    IOUtils::WriteBinaryFloat(material.specular.b, stream);
    IOUtils::WriteBinaryFloat(material.specular.a, stream);

    /* emissive.r */  IOUtils::WriteBinaryFloat(0.0f, stream);
    /* emissive.g */  IOUtils::WriteBinaryFloat(0.0f, stream);
    /* emissive.b */  IOUtils::WriteBinaryFloat(0.0f, stream);
    /* emissive.a */  IOUtils::WriteBinaryFloat(0.0f, stream);

    /* power */       IOUtils::WriteBinaryFloat(0.0f, stream);

    /* padding? */    IOUtils::WriteBinary<2, unsigned int>(0, stream);
}

Gfx::ModelTriangle::ModelTriangle()
{
    min = 0.0f;
    max = 0.0f;
    state = 0L;
}


Gfx::CModelFile::CModelFile(CInstanceManager* iMan)
{
    m_iMan = iMan;

    m_engine = (CEngine*)m_iMan->SearchInstance(CLASS_ENGINE);

    m_triangles.reserve(TRIANGLE_PREALLOCATE_COUNT);
}

Gfx::CModelFile::~CModelFile()
{
}

std::string Gfx::CModelFile::GetError()
{
    return m_error;
}


bool Gfx::CModelFile::ReadModel(const std::string &filename, bool edit, bool meta)
{
    m_triangles.clear();
    m_error = "";

    std::ifstream stream;
    stream.open(filename.c_str(), std::ios_base::in | std::ios_base::binary);
    if (! stream.good())
    {
        m_error = std::string("Could not open file '") + filename + std::string("'");
        return false;
    }

    return ReadModel(stream, edit, meta);
}

bool Gfx::CModelFile::ReadModel(std::istream &stream, bool edit, bool meta)
{
    m_triangles.clear();
    m_error = "";

    // FIXME: for now, reading models only from files, not metafile

    ModelHeader header;

    header.revision      = IOUtils::ReadBinary<4, int>(stream);
    header.version       = IOUtils::ReadBinary<4, int>(stream);
    header.totalVertices = IOUtils::ReadBinary<4, int>(stream);
    for (int i = 0; i < 10; ++i)
        header.reserved[i] = IOUtils::ReadBinary<4, int>(stream);


    if (! stream.good())
    {
        m_error = "Error reading model file header";
        return false;
    }

    // Old model version #1
    if ( (header.revision == 1) && (header.version == 0) )
    {
        for (int i = 0; i < header.totalVertices; ++i)
        {
            OldModelTriangle1 t;
            t.used     = IOUtils::ReadBinary<1, char>(stream);
            t.selected = IOUtils::ReadBinary<1, char>(stream);

            t.p1 = ReadBinaryVertex(stream);
            t.p2 = ReadBinaryVertex(stream);
            t.p3 = ReadBinaryVertex(stream);

            t.material = ReadBinaryMaterial(stream);
            stream.read(t.texName, 20);
            t.min = IOUtils::ReadBinaryFloat(stream);
            t.max = IOUtils::ReadBinaryFloat(stream);

            if (! stream.good())
            {
                m_error = "Error reading model data";
                return false;
            }

            Gfx::ModelTriangle triangle;
            triangle.p1.FromVertex(t.p1);
            triangle.p2.FromVertex(t.p2);
            triangle.p3.FromVertex(t.p3);

            triangle.material = t.material;
            triangle.tex1Name = std::string(t.texName);
            triangle.min = t.min;
            triangle.max = t.max;

            m_triangles.push_back(triangle);
        }
    }
    else if ( header.revision == 1 && header.version == 1 )
    {
        for (int i = 0; i < header.totalVertices; ++i)
        {
            OldModelTriangle2 t;
            t.used     = IOUtils::ReadBinary<1, char>(stream);
            t.selected = IOUtils::ReadBinary<1, char>(stream);

            t.p1 = ReadBinaryVertex(stream);
            t.p2 = ReadBinaryVertex(stream);
            t.p3 = ReadBinaryVertex(stream);

            t.material = ReadBinaryMaterial(stream);
            stream.read(t.texName, 20);
            t.min = IOUtils::ReadBinaryFloat(stream);
            t.max = IOUtils::ReadBinaryFloat(stream);
            t.state = IOUtils::ReadBinary<4, long>(stream);

            t.reserved1 = IOUtils::ReadBinary<2, short>(stream);
            t.reserved2 = IOUtils::ReadBinary<2, short>(stream);
            t.reserved3 = IOUtils::ReadBinary<2, short>(stream);
            t.reserved4 = IOUtils::ReadBinary<2, short>(stream);

            if (! stream.good())
            {
                m_error = "Error reading model data";
                return false;
            }

            Gfx::ModelTriangle triangle;
            triangle.p1.FromVertex(t.p1);
            triangle.p2.FromVertex(t.p2);
            triangle.p3.FromVertex(t.p3);

            triangle.material = t.material;
            triangle.tex1Name = std::string(t.texName);
            triangle.min = t.min;
            triangle.max = t.max;
            triangle.state = t.state;

            m_triangles.push_back(triangle);
        }
    }
    else
    {
        for (int i = 0; i < header.totalVertices; ++i)
        {
            NewModelTriangle t;
            t.used     = IOUtils::ReadBinary<1, char>(stream);
            t.selected = IOUtils::ReadBinary<1, char>(stream);

            t.p1 = ReadBinaryVertexTex2(stream);
            t.p2 = ReadBinaryVertexTex2(stream);
            t.p3 = ReadBinaryVertexTex2(stream);

            t.material = ReadBinaryMaterial(stream);
            stream.read(t.texName, 20);
            t.min = IOUtils::ReadBinaryFloat(stream);
            t.max = IOUtils::ReadBinaryFloat(stream);
            t.state = IOUtils::ReadBinary<4, long>(stream);
            t.texNum2 = IOUtils::ReadBinary<2, short>(stream);

            t.reserved2 = IOUtils::ReadBinary<2, short>(stream);
            t.reserved3 = IOUtils::ReadBinary<2, short>(stream);
            t.reserved4 = IOUtils::ReadBinary<2, short>(stream);

            if (! stream.good())
            {
                m_error = "Error reading model data";
                return false;
            }

            Gfx::ModelTriangle triangle;
            triangle.p1 = t.p1;
            triangle.p2 = t.p2;
            triangle.p3 = t.p3;

            triangle.material = t.material;
            triangle.tex1Name = std::string(t.texName);
            char tex2Name[20] = { 0 };
            triangle.min = t.min;
            triangle.max = t.max;
            triangle.state = t.state;
            sprintf(tex2Name, "dirty%.2d.tga", t.texNum2); // hardcoded as in the original code
            triangle.tex2Name = std::string(tex2Name);

            m_triangles.push_back(triangle);
        }
    }

    for (int i = 0; i < (int) m_triangles.size(); ++i)
        m_triangles[i].tex1Name = StrUtils::Replace(m_triangles[i].tex1Name, "bmp", "tga");

    /*
    if (! edit)
    {
        float limit[2];
        limit[0] = m_engine->RetLimitLOD(0);  // frontier AB as config
        limit[1] = m_engine->RetLimitLOD(1);  // frontier BC as config

        // Standard frontiers -> config.
        for (int i = 0; i < m_triangles.size(); ++i)
        {
            if ( m_triangles[i].min == 0.0f &&
                 m_triangles[i].max == 100.0f )  // resolution A ?
            {
                m_triangles[i].max = limit[0];
            }
            else if ( m_triangles[i].min == 100.0f &&
                      m_triangles[i].max == 200.0f )  // resolution B ?
            {
                m_triangles[i].min = limit[0];
                m_triangles[i].max = limit[1];
            }
            else if ( m_triangles[i].min == 200.0f &&
                      m_triangles[i].max == 1000000.0f )  // resolution C ?
            {
                m_triangles[i].min = limit[1];
            }
        }
    }*/

    return true;
}

bool Gfx::CModelFile::WriteModel(const std::string &filename)
{
    m_error = "";

    std::ofstream stream;
    stream.open(filename.c_str(), std::ios_base::out | std::ios_base::binary);
    if (! stream.good())
    {
        m_error = std::string("Could not open file '") + filename + std::string("'");
        return false;
    }

    return WriteModel(stream);
}

bool Gfx::CModelFile::WriteModel(std::ostream &stream)
{
    m_error = "";

    if (m_triangles.size() == 0)
    {
        m_error = "Empty model";
        return false;
    }

    ModelHeader header;
    header.revision   = 1;
    header.version  = 2;
    header.totalVertices = m_triangles.size();

    IOUtils::WriteBinary<4, int>(header.revision,      stream);
    IOUtils::WriteBinary<4, int>(header.version,       stream);
    IOUtils::WriteBinary<4, int>(header.totalVertices, stream);
    for (int i = 0; i < 10; ++i)
        IOUtils::WriteBinary<4, int>(header.reserved[i], stream);

    for (int i = 0; i < (int)m_triangles.size(); ++i)
    {
        NewModelTriangle t;

        t.used = true;

        t.p1 = m_triangles[i].p1;
        t.p2 = m_triangles[i].p2;
        t.p3 = m_triangles[i].p3;

        t.material = m_triangles[i].material;
        strncpy(t.texName, m_triangles[i].tex1Name.c_str(), 20);
        t.min = m_triangles[i].min;
        t.max = m_triangles[i].max;
        t.state = m_triangles[i].state;
        int no = 0;
        sscanf(m_triangles[i].tex2Name.c_str(), "dirty%d.tga", &no); // hardcoded as in the original code
        t.texNum2 = no;


        IOUtils::WriteBinary<1, char>(t.used,     stream);
        IOUtils::WriteBinary<1, char>(t.selected, stream);

        WriteBinaryVertexTex2(t.p1, stream);
        WriteBinaryVertexTex2(t.p2, stream);
        WriteBinaryVertexTex2(t.p3, stream);

        WriteBinaryMaterial(t.material, stream);
        stream.write(t.texName, 20);
        IOUtils::WriteBinaryFloat(t.min, stream);
        IOUtils::WriteBinaryFloat(t.max, stream);
        IOUtils::WriteBinary<4, long>(t.state, stream);
        IOUtils::WriteBinary<2, short>(t.texNum2, stream);

        IOUtils::WriteBinary<2, short>(t.reserved2, stream);
        IOUtils::WriteBinary<2, short>(t.reserved3, stream);
        IOUtils::WriteBinary<2, short>(t.reserved4, stream);
    }

    return true;
}

bool Gfx::CModelFile::ReadDXF(const std::string &filename, float min, float max)
{
    m_triangles.clear();
    m_error = "";

    std::ifstream stream;
    stream.open(filename.c_str(), std::ios_base::in);
    if (! stream.good())
    {
        m_error = std::string("Couldn't open file '") + filename + std::string("'");
        return false;
    }

    return ReadDXF(stream, min, max);
}

bool Gfx::CModelFile::ReadDXF(std::istream &stream, float min, float max)
{
    m_triangles.clear();
    m_error = "";

    if (! stream.good())
    {
        m_error = "Invalid stream";
        return false;
    }

    // Input state
    bool waitNumVertex = false;
    bool waitNumFace   = false;
    bool waitVertexX   = false;
    bool waitVertexY   = false;
    bool waitVertexZ   = false;
    bool waitFaceX     = false;
    bool waitFaceY     = false;
    bool waitFaceZ     = false;

    // Vertex array
    std::vector<Math::Vector> vertices;
    vertices.reserve(TRIANGLE_PREALLOCATE_COUNT);

    // Number of vertices & faces of the primitive to be read
    int          vertexNum = 0, faceNum = 0;
    // Vertex coords
    Math::Vector coords;
    // Indexes of face (triangle) points
    int          p1 = 0, p2 = 0, p3 = 0;

    // Input line
    std::string line;
    while (! stream.eof() )
    {
        // Read line with command
        std::getline(stream, line);
        int command = StrUtils::FromString<int>(line);

        // Read line with param
        std::getline(stream, line);

        bool ok = true;


        if (command == 66)
        {
            waitNumVertex = true;
        }

        if ( command == 71 && waitNumVertex )
        {
            waitNumVertex = false;
            vertexNum = StrUtils::FromString<int>(line, &ok);
            waitNumFace = true;
        }

        if ( command == 72 && waitNumFace )
        {
            waitNumFace = false;
            faceNum = StrUtils::FromString<int>(line, &ok);
            waitVertexX = true;
        }

        if ( command == 10 && waitVertexX )
        {
            waitVertexX = false;
            coords.x  = StrUtils::FromString<float>(line, &ok);
            waitVertexY = true;
        }

        if ( command == 20 && waitVertexY )
        {
            waitVertexY = false;
            coords.y  = StrUtils::FromString<float>(line, &ok);
            waitVertexZ = true;
        }

        if ( command == 30 && waitVertexZ )
        {
            waitVertexZ = false;
            coords.z = StrUtils::FromString<float>(line, &ok);

            vertexNum --;
            if ( vertexNum >= 0 )
            {
                Math::Vector p(coords.x, coords.z, coords.y);  // permutation of Y and Z!
                vertices.push_back(p);
                waitVertexX = true;
            }
            else
            {
                waitFaceX = true;
            }
        }

        if ( command == 71 && waitFaceX )
        {
            waitFaceX = false;
            p1 = StrUtils::FromString<int>(line, &ok);
            if ( p1 < 0 )  p1 = -p1;
            waitFaceY = true;
        }

        if ( command == 72 && waitFaceY )
        {
            waitFaceY = false;
            p2 = StrUtils::FromString<int>(line, &ok);
            if ( p2 < 0 )  p2 = -p2;
            waitFaceZ = true;
        }

        if ( command == 73 && waitFaceZ )
        {
            waitFaceZ = false;
            p3 = StrUtils::FromString<int>(line, &ok);
            if ( p3 < 0 )  p3 = -p3;

            faceNum --;
            if ( faceNum >= 0 )
            {
                assert( (p1-1 >= 0) && (p1-1 < (int)vertices.size() ) );
                assert( (p2-1 >= 0) && (p2-1 < (int)vertices.size() ) );
                assert( (p3-1 >= 0) && (p3-1 < (int)vertices.size() ) );

                CreateTriangle(vertices[p3-1], vertices[p2-1], vertices[p1-1], min, max);
                waitFaceX = true;
            }
        }

        if (! ok)
        {
            m_error = "Error reading data";
            return false;
        }

    }

    return true;
}

bool Gfx::CModelFile::CreateEngineObject(int objRank, int addState)
{
    /*char    texName1[20];
    char    texName2[20];
    int     texNum, i, state;

    for (int i = 0; i < m_trianglesUsed; i++)
    {
        if (! m_triangles[i].used)  continue;

        state = m_triangles[i].state;
        strcpy(texName1, m_triangles[i].texName);
        texName2[0] = 0;

        if ( strcmp(texName1, "plant.tga") == 0 ) // ???
        {
            state |= D3DSTATEALPHA;
        }

        if ( m_triangles[i].texNum2 != 0 )
        {
            if ( m_triangles[i].texNum2 == 1 )
            {
                texNum = m_engine->RetSecondTexture();
            }
            else
            {
                texNum = m_triangles[i].texNum2;
            }

            if ( texNum >= 1 && texNum <= 10 )
            {
                state |= D3DSTATEDUALb;
            }
            if ( texNum >= 11 && texNum <= 20 )
            {
                state |= D3DSTATEDUALw;
            }
            sprintf(texName2, "dirty%.2d.tga", texNum); // ???
        }

        m_engine->AddTriangle(objRank, &m_triangles[i].p1, 3,
                              m_triangles[i].material,
                              state + addState,
                              texName1, texName2,
                              m_triangles[i].min,
                              m_triangles[i].max, false);
    }*/
    return true;
}

void Gfx::CModelFile::Mirror()
{
    for (int i = 0; i < (int)m_triangles.size(); i++)
    {
        Gfx::VertexTex2  t = m_triangles[i].p1;
        m_triangles[i].p1 = m_triangles[i].p2;
        m_triangles[i].p2 = t;

        m_triangles[i].p1.coord.z = -m_triangles[i].p1.coord.z;
        m_triangles[i].p2.coord.z = -m_triangles[i].p2.coord.z;
        m_triangles[i].p3.coord.z = -m_triangles[i].p3.coord.z;

        m_triangles[i].p1.normal.z = -m_triangles[i].p1.normal.z;
        m_triangles[i].p2.normal.z = -m_triangles[i].p2.normal.z;
        m_triangles[i].p3.normal.z = -m_triangles[i].p3.normal.z;
    }
}

int Gfx::CModelFile::GetTriangleCount()
{
    return m_triangles.size();
}

float Gfx::CModelFile::GetHeight(Math::Vector pos)
{
    float limit = 5.0f;

    for (int i = 0; i < (int)m_triangles.size(); i++)
    {
        if ( fabs(pos.x - m_triangles[i].p1.coord.x) < limit &&
             fabs(pos.z - m_triangles[i].p1.coord.z) < limit )
            return m_triangles[i].p1.coord.y;

        if ( fabs(pos.x - m_triangles[i].p2.coord.x) < limit &&
             fabs(pos.z - m_triangles[i].p2.coord.z) < limit )
            return m_triangles[i].p2.coord.y;

        if ( fabs(pos.x - m_triangles[i].p3.coord.x) < limit &&
             fabs(pos.z - m_triangles[i].p3.coord.z) < limit )
            return m_triangles[i].p3.coord.y;
    }

    return 0.0f;
}

void Gfx::CModelFile::CreateTriangle(Math::Vector p1, Math::Vector p2, Math::Vector p3, float min, float max)
{
    Gfx::ModelTriangle triangle;

    Math::Vector n = Math::NormalToPlane(p3, p2, p1);
    triangle.p1 = Gfx::VertexTex2(p1, n);
    triangle.p2 = Gfx::VertexTex2(p2, n);
    triangle.p3 = Gfx::VertexTex2(p3, n);

    triangle.material.diffuse = Gfx::Color(1.0f, 1.0f, 1.0f, 0.0f);
    triangle.material.ambient = Gfx::Color(0.5f, 0.5f, 0.5f, 0.0f);

    triangle.min = min;
    triangle.max = max;

    m_triangles.push_back(triangle);
}
