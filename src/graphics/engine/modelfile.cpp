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


#include "graphics/engine/modelfile.h"

#include "common/ioutils.h"
#include "common/logger.h"
#include "common/stringutils.h"

#include "graphics/engine/engine.h"

#include "math/geometry.h"


#include <cstring>

#include <fstream>
#include <sstream>


// Graphics module namespace
namespace Gfx {


bool ReadBinaryVertex(std::istream& stream, Vertex& vertex)
{
    vertex.coord.x     = IOUtils::ReadBinaryFloat(stream);
    vertex.coord.y     = IOUtils::ReadBinaryFloat(stream);
    vertex.coord.z     = IOUtils::ReadBinaryFloat(stream);
    vertex.normal.x    = IOUtils::ReadBinaryFloat(stream);
    vertex.normal.y    = IOUtils::ReadBinaryFloat(stream);
    vertex.normal.z    = IOUtils::ReadBinaryFloat(stream);
    vertex.texCoord.x  = IOUtils::ReadBinaryFloat(stream);
    vertex.texCoord.y  = IOUtils::ReadBinaryFloat(stream);

    return !stream.fail();
}

bool WriteBinaryVertex(Vertex vertex, std::ostream& stream)
{
    IOUtils::WriteBinaryFloat(vertex.coord.x, stream);
    IOUtils::WriteBinaryFloat(vertex.coord.y, stream);
    IOUtils::WriteBinaryFloat(vertex.coord.z, stream);
    IOUtils::WriteBinaryFloat(vertex.normal.x, stream);
    IOUtils::WriteBinaryFloat(vertex.normal.y, stream);
    IOUtils::WriteBinaryFloat(vertex.normal.z, stream);
    IOUtils::WriteBinaryFloat(vertex.texCoord.x, stream);
    IOUtils::WriteBinaryFloat(vertex.texCoord.y, stream);

    return !stream.fail();
}

bool ReadBinaryVertexTex2(std::istream& stream, VertexTex2& vertex)
{
    vertex.coord.x     = IOUtils::ReadBinaryFloat(stream);
    vertex.coord.y     = IOUtils::ReadBinaryFloat(stream);
    vertex.coord.z     = IOUtils::ReadBinaryFloat(stream);
    vertex.normal.x    = IOUtils::ReadBinaryFloat(stream);
    vertex.normal.y    = IOUtils::ReadBinaryFloat(stream);
    vertex.normal.z    = IOUtils::ReadBinaryFloat(stream);
    vertex.texCoord.x  = IOUtils::ReadBinaryFloat(stream);
    vertex.texCoord.y  = IOUtils::ReadBinaryFloat(stream);
    vertex.texCoord2.x = IOUtils::ReadBinaryFloat(stream);
    vertex.texCoord2.y = IOUtils::ReadBinaryFloat(stream);

    return !stream.fail();
}

bool WriteBinaryVertexTex2(VertexTex2 vertex, std::ostream& stream)
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

    return !stream.fail();
}

bool ReadTextVertexTex2(const std::string& text, VertexTex2& vertex)
{
    std::stringstream stream;
    stream.str(text);

    std::string what;

    stream >> what;
    if (what != "c")
        return false;

    stream >> vertex.coord.x >> vertex.coord.y >> vertex.coord.z;

    stream >> what;
    if (what != "n")
        return false;

    stream >> vertex.normal.x >> vertex.normal.y >> vertex.normal.z;

    stream >> what;
    if (what != "t1")
        return false;

    stream >> vertex.texCoord.x >> vertex.texCoord.y;

    stream >> what;
    if (what != "t2")
        return false;

    stream >> vertex.texCoord2.x >> vertex.texCoord2.y;

    return !stream.fail();
}

bool WriteTextVertexTex2(const VertexTex2& vertex, std::ostream& stream)
{
    stream << "c " << vertex.coord.x << " " << vertex.coord.y << " " << vertex.coord.z;
    stream << " n " << vertex.normal.x << " " << vertex.normal.y << " " << vertex.normal.z;
    stream << " t1 " << vertex.texCoord.x << " " << vertex.texCoord.y;
    stream << " t2 " << vertex.texCoord2.x << " " << vertex.texCoord2.y;
    stream << std::endl;

    return !stream.fail();
}

bool ReadBinaryMaterial(std::istream& stream, Material& material)
{
    material.diffuse.r =  IOUtils::ReadBinaryFloat(stream);
    material.diffuse.g =  IOUtils::ReadBinaryFloat(stream);
    material.diffuse.b =  IOUtils::ReadBinaryFloat(stream);
    material.diffuse.a =  IOUtils::ReadBinaryFloat(stream);

    material.ambient.r =  IOUtils::ReadBinaryFloat(stream);
    material.ambient.g =  IOUtils::ReadBinaryFloat(stream);
    material.ambient.b =  IOUtils::ReadBinaryFloat(stream);
    material.ambient.a =  IOUtils::ReadBinaryFloat(stream);

    material.specular.r = IOUtils::ReadBinaryFloat(stream);
    material.specular.g = IOUtils::ReadBinaryFloat(stream);
    material.specular.b = IOUtils::ReadBinaryFloat(stream);
    material.specular.a = IOUtils::ReadBinaryFloat(stream);

    /* emissive.r = */    IOUtils::ReadBinaryFloat(stream);
    /* emissive.g = */    IOUtils::ReadBinaryFloat(stream);
    /* emissive.b = */    IOUtils::ReadBinaryFloat(stream);
    /* emissive.a = */    IOUtils::ReadBinaryFloat(stream);

    /* power = */         IOUtils::ReadBinaryFloat(stream);

    return !stream.fail();
}

bool WriteBinaryMaterial(const Material& material, std::ostream& stream)
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

    return !stream.fail();
}

bool ReadTextMaterial(const std::string& text, Material& material)
{
    std::stringstream stream;
    stream.str(text);

    std::string what;

    stream >> what;
    if (what != "dif")
        return false;

    stream >> material.diffuse.r
           >> material.diffuse.g
           >> material.diffuse.b
           >> material.diffuse.a;

    stream >> what;
    if (what != "amb")
        return false;

    stream >> material.ambient.r
           >> material.ambient.g
           >> material.ambient.b
           >> material.ambient.a;

    stream >> what;
    if (what != "spc")
        return false;

    stream >> material.specular.r
           >> material.specular.g
           >> material.specular.b
           >> material.specular.a;

    return !stream.fail();
}

bool WriteTextMaterial(const Material& material, std::ostream& stream)
{
    stream << "dif " << material.diffuse.r
           << " " << material.diffuse.g
           << " " << material.diffuse.b
           << " " << material.diffuse.a;

    stream << " amb " << material.ambient.r
           << " " << material.ambient.g
           << " " << material.ambient.b
           << " " << material.ambient.a;

    stream << " spc " << material.specular.r
           << " " << material.specular.g << " "
           << material.specular.b << " "
           << material.specular.a;

    stream << std::endl;

    return !stream.fail();
}

template<typename T>
bool ReadLineValue(std::istream& stream, const std::string& prefix, T& value)
{
    std::string line;
    while (true)
    {
        if (stream.eof() || stream.fail())
            return false;

        std::getline(stream, line);
        if (!line.empty() && line[0] != '#')
            break;
    }

    std::stringstream s;
    s.str(line);

    std::string what;
    s >> what;
    if (what != prefix)
        return false;

    s >> value;

    return true;
}

bool ReadLineString(std::istream& stream, const std::string& prefix, std::string& value)
{
    std::string line;
    while (true)
    {
        if (stream.eof() || stream.fail())
            return false;

        std::getline(stream, line);
        if (!line.empty() && line[0] != '#')
            break;
    }

    std::stringstream s;
    s.str(line);

    std::string what;
    s >> what;
    if (what != prefix)
        return false;

    std::getline(s, value);

    return true;
}


CModelFile::CModelFile()
{
}

CModelFile::~CModelFile()
{
}


/*******************************************************
                      Deprecated formats
 *******************************************************/

/**
 * \struct OldModelHeader
 * \brief Colobot binary model header info
 *
 * @deprecated
 */
struct OldModelHeader
{
    //! Revision number
    int     revision;
    //! Version number
    int     version;
    //! Total number of triangles
    int     totalTriangles;
    //! Reserved area
    int     reserved[10];

    OldModelHeader()
    {
        memset(this, 0, sizeof(*this));
    }
};


/**
 * \struct OldModelTriangle1
 * \brief Colobot binary model file version 1
 *
 * @deprecated
 */
struct OldModelTriangle1
{
    char            used;
    char            selected;
    Vertex     p1;
    Vertex     p2;
    Vertex     p3;
    Material   material;
    char            texName[20];
    float           min;
    float           max;

    OldModelTriangle1()
    {
        memset(this, 0, sizeof(*this));
    }
};

/**
 * \struct OldModelTriangle2
 * \brief Colobot binary model file version 2
 *
 * @deprecated
 */
struct OldModelTriangle2
{
    char            used;
    char            selected;
    Vertex     p1;
    Vertex     p2;
    Vertex     p3;
    Material   material;
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

/**
 * \struct OldModelTriangle3
 * \brief Colobot binary model file version 3
 *
 * @deprecated
 */
struct OldModelTriangle3
{
    char            used;
    char            selected;
    VertexTex2 p1;
    VertexTex2 p2;
    VertexTex2 p3;
    Material   material;
    char            texName[20];
    float           min;
    float           max;
    long            state;
    short           texNum2;
    short           reserved2;
    short           reserved3;
    short           reserved4;

    OldModelTriangle3()
    {
        memset(this, 0, sizeof(*this));
    }
};

bool CModelFile::ReadModel(const std::string& fileName)
{
    m_triangles.clear();

    std::ifstream stream;
    stream.open(fileName.c_str(), std::ios_base::in | std::ios_base::binary);
    if (!stream.good())
    {
        GetLogger()->Error("Could not open file '%s'\n", fileName.c_str());
        return false;
    }

    return ReadModel(stream);
}

bool CModelFile::ReadModel(std::istream& stream)
{
    m_triangles.clear();

    OldModelHeader header;

    header.revision      = IOUtils::ReadBinary<4, int>(stream);
    header.version       = IOUtils::ReadBinary<4, int>(stream);
    header.totalTriangles = IOUtils::ReadBinary<4, int>(stream);
    for (int i = 0; i < 10; ++i)
        header.reserved[i] = IOUtils::ReadBinary<4, int>(stream);


    if (!stream.good())
    {
        GetLogger()->Error("Error reading model file header\n");
        return false;
    }

    // Old model version #1
    if ( (header.revision == 1) && (header.version == 0) )
    {
        for (int i = 0; i < header.totalTriangles; ++i)
        {
            OldModelTriangle1 t;
            t.used     = IOUtils::ReadBinary<1, char>(stream);
            t.selected = IOUtils::ReadBinary<1, char>(stream);

            /* padding */ IOUtils::ReadBinary<2, unsigned int>(stream);

            ReadBinaryVertex(stream, t.p1);
            ReadBinaryVertex(stream, t.p2);
            ReadBinaryVertex(stream, t.p3);

            ReadBinaryMaterial(stream, t.material);
            stream.read(t.texName, 20);
            t.min = IOUtils::ReadBinaryFloat(stream);
            t.max = IOUtils::ReadBinaryFloat(stream);

            if (stream.fail())
            {
                GetLogger()->Error("Error reading model data\n");
                return false;
            }

            ModelTriangle triangle;
            triangle.p1.FromVertex(t.p1);
            triangle.p2.FromVertex(t.p2);
            triangle.p3.FromVertex(t.p3);

            triangle.material = t.material;
            triangle.tex1Name = std::string(t.texName);
            triangle.lodLevel = MinMaxToLodLevel(t.min, t.max);

            m_triangles.push_back(triangle);
        }
    }
    else if ( header.revision == 1 && header.version == 1 )
    {
        for (int i = 0; i < header.totalTriangles; ++i)
        {
            OldModelTriangle2 t;
            t.used     = IOUtils::ReadBinary<1, char>(stream);
            t.selected = IOUtils::ReadBinary<1, char>(stream);

            /* padding */ IOUtils::ReadBinary<2, unsigned int>(stream);

            ReadBinaryVertex(stream, t.p1);
            ReadBinaryVertex(stream, t.p2);
            ReadBinaryVertex(stream, t.p3);

            ReadBinaryMaterial(stream, t.material);
            stream.read(t.texName, 20);
            t.min = IOUtils::ReadBinaryFloat(stream);
            t.max = IOUtils::ReadBinaryFloat(stream);
            t.state = IOUtils::ReadBinary<4, long>(stream);

            t.reserved1 = IOUtils::ReadBinary<2, short>(stream);
            t.reserved2 = IOUtils::ReadBinary<2, short>(stream);
            t.reserved3 = IOUtils::ReadBinary<2, short>(stream);
            t.reserved4 = IOUtils::ReadBinary<2, short>(stream);

            if (stream.fail())
            {
                GetLogger()->Error("Error reading model data\n");
                return false;
            }

            ModelTriangle triangle;
            triangle.p1.FromVertex(t.p1);
            triangle.p2.FromVertex(t.p2);
            triangle.p3.FromVertex(t.p3);

            triangle.material = t.material;
            triangle.tex1Name = std::string(t.texName);
            triangle.lodLevel = MinMaxToLodLevel(t.min, t.max);
            triangle.state = t.state;

            m_triangles.push_back(triangle);
        }
    }
    else
    {
        for (int i = 0; i < header.totalTriangles; ++i)
        {
            OldModelTriangle3 t;
            t.used     = IOUtils::ReadBinary<1, char>(stream);
            t.selected = IOUtils::ReadBinary<1, char>(stream);

            /* padding */ IOUtils::ReadBinary<2, unsigned int>(stream);

            ReadBinaryVertexTex2(stream, t.p1);
            ReadBinaryVertexTex2(stream, t.p2);
            ReadBinaryVertexTex2(stream, t.p3);

            ReadBinaryMaterial(stream, t.material);
            stream.read(t.texName, 20);
            t.min = IOUtils::ReadBinaryFloat(stream);
            t.max = IOUtils::ReadBinaryFloat(stream);
            t.state = IOUtils::ReadBinary<4, long>(stream);
            t.texNum2 = IOUtils::ReadBinary<2, short>(stream);

            t.reserved2 = IOUtils::ReadBinary<2, short>(stream);
            t.reserved3 = IOUtils::ReadBinary<2, short>(stream);
            t.reserved4 = IOUtils::ReadBinary<2, short>(stream);

            if (stream.fail())
            {
                GetLogger()->Error("Error reading model data\n");
                return false;
            }

            ModelTriangle triangle;
            triangle.p1 = t.p1;
            triangle.p2 = t.p2;
            triangle.p3 = t.p3;

            triangle.material = t.material;
            triangle.tex1Name = std::string(t.texName);
            triangle.lodLevel = MinMaxToLodLevel(t.min, t.max);
            triangle.state = t.state;
            triangle.variableTex2 = t.texNum2 == 1;

            if (!triangle.variableTex2 && t.texNum2 != 0)
            {
                if (t.texNum2 >= 1 && t.texNum2 <= 10)
                    triangle.state |= ENG_RSTATE_DUAL_BLACK;

                if (t.texNum2 >= 11 && t.texNum2 <= 20)
                    triangle.state |= ENG_RSTATE_DUAL_WHITE;

                char tex2Name[20] = { 0 };
                sprintf(tex2Name, "dirty%.2d.png", t.texNum2); // hardcoded as in original code
                triangle.tex2Name = tex2Name;
            }

            m_triangles.push_back(triangle);
       }
    }

    for (int i = 0; i < static_cast<int>( m_triangles.size() ); ++i)
    {
        // All extensions are now png
        m_triangles[i].tex1Name = StrUtils::Replace(m_triangles[i].tex1Name, "bmp", "png");
        m_triangles[i].tex1Name = StrUtils::Replace(m_triangles[i].tex1Name, "tga", "png");

        m_triangles[i].tex2Name = StrUtils::Replace(m_triangles[i].tex2Name, "bmp", "png");
        m_triangles[i].tex2Name = StrUtils::Replace(m_triangles[i].tex2Name, "tga", "png");

        // TODO: fix this in model files
        if (m_triangles[i].tex1Name == "plant.png")
            m_triangles[i].state |= ENG_RSTATE_ALPHA;

        GetLogger()->Trace("ModelTriangle %d\n", i+1);
        std::string s1 = m_triangles[i].p1.ToString();
        GetLogger()->Trace(" p1: %s\n", s1.c_str());
        std::string s2 = m_triangles[i].p2.ToString();
        GetLogger()->Trace(" p2: %s\n", s2.c_str());
        std::string s3 = m_triangles[i].p3.ToString();
        GetLogger()->Trace(" p3: %s\n", s3.c_str());

        std::string d = m_triangles[i].material.diffuse.ToString();
        std::string a = m_triangles[i].material.ambient.ToString();
        std::string s = m_triangles[i].material.specular.ToString();
        GetLogger()->Trace(" mat: d: %s  a: %s  s: %s\n", d.c_str(), a.c_str(), s.c_str());

        GetLogger()->Trace(" tex1: %s  tex2: %s\n", m_triangles[i].tex1Name.c_str(),
                          m_triangles[i].variableTex2 ? "(variable)" : m_triangles[i].tex2Name.c_str());
        GetLogger()->Trace(" lod level: %d\n", m_triangles[i].lodLevel);
        GetLogger()->Trace(" state: %ld\n", m_triangles[i].state);
    }

    return true;
}

bool CModelFile::WriteModel(const std::string& fileName)
{
    std::ofstream stream;
    stream.open(fileName.c_str(), std::ios_base::out | std::ios_base::binary);
    if (!stream.good())
    {
        GetLogger()->Error("Could not open file '%s'\n", fileName.c_str());
        return false;
    }

    return WriteModel(stream);
}

bool CModelFile::WriteModel(std::ostream& stream)
{
    if (m_triangles.size() == 0)
    {
        GetLogger()->Error("Empty model\n");
        return false;
    }

    OldModelHeader header;
    header.revision   = 1;
    header.version  = 2;
    header.totalTriangles = m_triangles.size();

    IOUtils::WriteBinary<4, int>(header.revision,      stream);
    IOUtils::WriteBinary<4, int>(header.version,       stream);
    IOUtils::WriteBinary<4, int>(header.totalTriangles, stream);
    for (int i = 0; i < 10; ++i)
        IOUtils::WriteBinary<4, int>(header.reserved[i], stream);

    for (int i = 0; i < static_cast<int>( m_triangles.size() ); ++i)
    {
        OldModelTriangle3 t;

        t.used = true;

        t.p1 = m_triangles[i].p1;
        t.p2 = m_triangles[i].p2;
        t.p3 = m_triangles[i].p3;

        t.material = m_triangles[i].material;
        strncpy(t.texName, m_triangles[i].tex1Name.c_str(), 20);
        LODLevelToMinMax(m_triangles[i].lodLevel, t.min, t.max);
        t.state = m_triangles[i].state;

        int no = 0;
        if (m_triangles[i].variableTex2)
            no = 1;
        else
            sscanf(m_triangles[i].tex2Name.c_str(), "dirty%d.png", &no); // hardcoded as in the original code

        t.texNum2 = no;


        IOUtils::WriteBinary<1, char>(t.used,     stream);
        IOUtils::WriteBinary<1, char>(t.selected, stream);

        /* padding */ IOUtils::WriteBinary<2, unsigned int>(0, stream);

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

LODLevel CModelFile::MinMaxToLodLevel(float min, float max)
{
    if (min == 0.0f && max == 100.0f)
        return LOD_High;
    else if (min == 100.0f && max == 200.0f)
        return LOD_Medium;
    else if (min == 200.0f && max == 1000000.0f)
        return LOD_Low;
    else if (min == 0.0f && max == 1000000.0f)
        return LOD_Constant;

    return LOD_Constant;
}

void CModelFile::LODLevelToMinMax(LODLevel lodLevel, float& min, float& max)
{
    switch (lodLevel)
    {
        case LOD_High:
            min = 0.0f;
            max = 100.0f;
            break;

        case LOD_Medium:
            min = 100.0f;
            max = 200.0f;
            break;

        case LOD_Low:
            min = 200.0f;
            max = 1000000.0f;
            break;

        case LOD_Constant:
            min = 0.0f;
            max = 1000000.0f;
            break;
    }
}


/*******************************************************
                      New formats
 *******************************************************/

/**
 * \struct NewModelHeader
 * \brief Header for new binary model file
 */
struct NewModelHeader
{
    //! File version (1, 2, ...)
    int version;
    //! Total number of triangles
    int totalTriangles;

    NewModelHeader()
    {
        version = 0;
        totalTriangles = 0;
    }
};

/**
 * \struct NewModelTriangle1
 * \brief Triangle of new binary model file
 *
 * NOTE: at this time, it is identical to ModelTriangle struct, but it may change
 * independently in the future.
 */
struct NewModelTriangle1
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
    //! LOD level
    int              lodLevel;
    //! Rendering state to be set
    int              state;

    NewModelTriangle1()
    {
        variableTex2 = true;
        lodLevel = 0;
        state = 0;
    }
};


bool CModelFile::ReadTextModel(const std::string& fileName)
{
    std::ifstream stream;
    stream.open(fileName.c_str(), std::ios_base::in);
    if (!stream.good())
    {
        GetLogger()->Error("Could not open file '%s'\n", fileName.c_str());
        return false;
    }

    return ReadTextModel(stream);
}

bool CModelFile::ReadTextModel(std::istream& stream)
{
    m_triangles.clear();

    NewModelHeader header;

    bool headOk = ReadLineValue<int>(stream, "version", header.version) &&
                  ReadLineValue<int>(stream, "total_triangles", header.totalTriangles);

    if (!headOk || !stream.good())
    {
        GetLogger()->Error("Error reading model file header\n");
        return false;
    }

    // New model version 1
    if (header.version == 1)
    {
        for (int i = 0; i < header.totalTriangles; ++i)
        {
            NewModelTriangle1 t;

            std::string p1Text, p2Text, p3Text;
            std::string matText;
            char varTex2Ch = 0;

            bool triOk = ReadLineString(stream, "p1", p1Text) &&
                         ReadTextVertexTex2(p1Text, t.p1) &&
                         ReadLineString(stream, "p2", p2Text) &&
                         ReadTextVertexTex2(p2Text, t.p2) &&
                         ReadLineString(stream, "p3", p3Text) &&
                         ReadTextVertexTex2(p3Text, t.p3) &&
                         ReadLineString(stream, "mat", matText) &&
                         ReadTextMaterial(matText, t.material) &&
                         ReadLineValue<std::string>(stream, "tex1", t.tex1Name) &&
                         ReadLineValue<std::string>(stream, "tex2", t.tex2Name) &&
                         ReadLineValue<char>(stream, "var_tex2", varTex2Ch) &&
                         ReadLineValue<int>(stream, "lod_level", t.lodLevel) &&
                         ReadLineValue<int>(stream, "state", t.state);

            if (!triOk || stream.fail())
            {
                GetLogger()->Error("Error reading model data\n");
                return false;
            }

            t.variableTex2 = varTex2Ch == 'Y';


            ModelTriangle triangle;
            triangle.p1 = t.p1;
            triangle.p2 = t.p2;
            triangle.p3 = t.p3;
            triangle.material = t.material;
            triangle.tex1Name = t.tex1Name;
            triangle.tex2Name = t.tex2Name;
            triangle.variableTex2 = t.variableTex2;
            triangle.state = t.state;

            switch (t.lodLevel)
            {
                case 0: triangle.lodLevel = LOD_Constant; break;
                case 1: triangle.lodLevel = LOD_Low;      break;
                case 2: triangle.lodLevel = LOD_Medium;   break;
                case 3: triangle.lodLevel = LOD_High;     break;
                default: break;
            }

            m_triangles.push_back(triangle);

            continue;
        }
    }
    else
    {
        GetLogger()->Error("Unknown model file version\n");
        return false;
    }

    for (int i = 0; i < static_cast<int>( m_triangles.size() ); ++i)
    {
        GetLogger()->Trace("ModelTriangle %d\n", i+1);
        std::string s1 = m_triangles[i].p1.ToString();
        GetLogger()->Trace(" p1: %s\n", s1.c_str());
        std::string s2 = m_triangles[i].p2.ToString();
        GetLogger()->Trace(" p2: %s\n", s2.c_str());
        std::string s3 = m_triangles[i].p3.ToString();
        GetLogger()->Trace(" p3: %s\n", s3.c_str());

        std::string d = m_triangles[i].material.diffuse.ToString();
        std::string a = m_triangles[i].material.ambient.ToString();
        std::string s = m_triangles[i].material.specular.ToString();
        GetLogger()->Trace(" mat: d: %s  a: %s  s: %s\n", d.c_str(), a.c_str(), s.c_str());

        GetLogger()->Trace(" tex1: %s  tex2: %s\n", m_triangles[i].tex1Name.c_str(), m_triangles[i].tex2Name.c_str());
        GetLogger()->Trace(" lod level: %d\n", m_triangles[i].lodLevel);
        GetLogger()->Trace(" state: %ld\n", m_triangles[i].state);
    }

    return true;
}

bool CModelFile::WriteTextModel(const std::string &fileName)
{
    std::ofstream stream;
    stream.open(fileName.c_str(), std::ios_base::out);
    if (!stream.good())
    {
        GetLogger()->Error("Could not open file '%s'\n", fileName.c_str());
        return false;
    }

    return WriteTextModel(stream);
}

bool CModelFile::WriteTextModel(std::ostream& stream)
{
    if (m_triangles.size() == 0)
    {
        GetLogger()->Error("Empty model\n");
        return false;
    }

    NewModelHeader header;

    header.version        = 1;
    header.totalTriangles = m_triangles.size();

    stream << "# Colobot text model" << std::endl;
    stream << std::endl;
    stream << "### HEAD" << std::endl;
    stream << "version " << header.version << std::endl;
    stream << "total_triangles " << header.totalTriangles << std::endl;
    stream << std::endl;
    stream << "### TRIANGLES" << std::endl;

    for (int i = 0; i < static_cast<int>( m_triangles.size() ); ++i)
    {
        NewModelTriangle1 t;

        t.p1 = m_triangles[i].p1;
        t.p2 = m_triangles[i].p2;
        t.p3 = m_triangles[i].p3;
        t.material = m_triangles[i].material;
        t.tex1Name = m_triangles[i].tex1Name;
        t.tex2Name = m_triangles[i].tex2Name;
        t.variableTex2 = m_triangles[i].variableTex2;
        t.state = m_triangles[i].state;

        switch (m_triangles[i].lodLevel)
        {
            case LOD_Constant: t.lodLevel = 0; break;
            case LOD_Low:      t.lodLevel = 1; break;
            case LOD_Medium:   t.lodLevel = 2; break;
            case LOD_High:     t.lodLevel = 3; break;
        }

        stream << "p1 ";
        WriteTextVertexTex2(t.p1, stream);
        stream << "p2 ";
        WriteTextVertexTex2(t.p2, stream);
        stream << "p3 ";
        WriteTextVertexTex2(t.p3, stream);
        stream << "mat ";
        WriteTextMaterial(t.material, stream);

        stream << "tex1 " << t.tex1Name << std::endl;
        stream << "tex2 " << t.tex2Name << std::endl;
        stream << "var_tex2 " << (t.variableTex2 ? 'Y' : 'N') << std::endl;
        stream << "lod_level " << t.lodLevel << std::endl;
        stream << "state " << t.state << std::endl;

        stream << std::endl;

        if (stream.fail())
        {
            GetLogger()->Error("Error writing model file\n");
            return false;
        }
    }

    return true;
}

bool CModelFile::ReadBinaryModel(const std::string& fileName)
{
    std::ifstream stream;
    stream.open(fileName.c_str(), std::ios_base::in | std::ios_base::binary);
    if (!stream.good())
    {
        GetLogger()->Error("Could not open file '%s'\n", fileName.c_str());
        return false;
    }

    return ReadBinaryModel(stream);
}

bool CModelFile::ReadBinaryModel(std::istream& stream)
{
    m_triangles.clear();

    NewModelHeader header;

    header.version        = IOUtils::ReadBinary<4, int>(stream);
    header.totalTriangles = IOUtils::ReadBinary<4, int>(stream);

    if (!stream.good())
    {
        GetLogger()->Error("Error reading model file header\n");
        return false;
    }

    // New model version 1
    if (header.version == 1)
    {
        for (int i = 0; i < header.totalTriangles; ++i)
        {
            NewModelTriangle1 t;

            ReadBinaryVertexTex2(stream, t.p1);
            ReadBinaryVertexTex2(stream, t.p2);
            ReadBinaryVertexTex2(stream, t.p3);
            ReadBinaryMaterial(stream, t.material);
            t.tex1Name = IOUtils::ReadBinaryString<1>(stream);
            t.tex2Name = IOUtils::ReadBinaryString<1>(stream);
            t.variableTex2 = IOUtils::ReadBinaryBool(stream);
            t.lodLevel = IOUtils::ReadBinary<4, int>(stream);
            t.state = IOUtils::ReadBinary<4, int>(stream);

            if (stream.fail())
            {
                GetLogger()->Error("Error reading model data\n");
                return false;
            }

            ModelTriangle triangle;
            triangle.p1 = t.p1;
            triangle.p2 = t.p2;
            triangle.p3 = t.p3;
            triangle.material = t.material;
            triangle.tex1Name = t.tex1Name;
            triangle.tex2Name = t.tex2Name;
            triangle.variableTex2 = t.variableTex2;
            triangle.state = t.state;

            switch (t.lodLevel)
            {
                case 0: triangle.lodLevel = LOD_Constant; break;
                case 1: triangle.lodLevel = LOD_Low;      break;
                case 2: triangle.lodLevel = LOD_Medium;   break;
                case 3: triangle.lodLevel = LOD_High;     break;
                default: break;
            }

            m_triangles.push_back(triangle);
        }
    }
    else
    {
        GetLogger()->Error("Unknown model file version\n");
        return false;
    }

    for (int i = 0; i < static_cast<int>( m_triangles.size() ); ++i)
    {
        GetLogger()->Trace("ModelTriangle %d\n", i+1);
        std::string s1 = m_triangles[i].p1.ToString();
        GetLogger()->Trace(" p1: %s\n", s1.c_str());
        std::string s2 = m_triangles[i].p2.ToString();
        GetLogger()->Trace(" p2: %s\n", s2.c_str());
        std::string s3 = m_triangles[i].p3.ToString();
        GetLogger()->Trace(" p3: %s\n", s3.c_str());

        std::string d = m_triangles[i].material.diffuse.ToString();
        std::string a = m_triangles[i].material.ambient.ToString();
        std::string s = m_triangles[i].material.specular.ToString();
        GetLogger()->Trace(" mat: d: %s  a: %s  s: %s\n", d.c_str(), a.c_str(), s.c_str());

        GetLogger()->Trace(" tex1: %s  tex2: %s\n", m_triangles[i].tex1Name.c_str(), m_triangles[i].tex2Name.c_str());
        GetLogger()->Trace(" lod level: %d\n", m_triangles[i].lodLevel);
        GetLogger()->Trace(" state: %ld\n", m_triangles[i].state);
    }

    return true;
}

bool CModelFile::WriteBinaryModel(const std::string& fileName)
{
    std::ofstream stream;
    stream.open(fileName.c_str(), std::ios_base::out | std::ios_base::binary);
    if (!stream.good())
    {
        GetLogger()->Error("Could not open file '%s'\n", fileName.c_str());
        return false;
    }

    return WriteBinaryModel(stream);
}

bool CModelFile::WriteBinaryModel(std::ostream& stream)
{
    if (m_triangles.size() == 0)
    {
        GetLogger()->Error("Empty model\n");
        return false;
    }

    NewModelHeader header;

    header.version        = 1;
    header.totalTriangles = m_triangles.size();

    IOUtils::WriteBinary<4, int>(header.version, stream);
    IOUtils::WriteBinary<4, int>(header.totalTriangles, stream);

    for (int i = 0; i < static_cast<int>( m_triangles.size() ); ++i)
    {
        NewModelTriangle1 t;

        t.p1 = m_triangles[i].p1;
        t.p2 = m_triangles[i].p2;
        t.p3 = m_triangles[i].p3;
        t.material = m_triangles[i].material;
        t.tex1Name = m_triangles[i].tex1Name;
        t.tex2Name = m_triangles[i].tex2Name;
        t.variableTex2 = m_triangles[i].variableTex2;
        t.state = m_triangles[i].state;

        switch (m_triangles[i].lodLevel)
        {
            case LOD_Constant: t.lodLevel = 0; break;
            case LOD_Low:      t.lodLevel = 1; break;
            case LOD_Medium:   t.lodLevel = 2; break;
            case LOD_High:     t.lodLevel = 3; break;
        }

        WriteBinaryVertexTex2(t.p1, stream);
        WriteBinaryVertexTex2(t.p2, stream);
        WriteBinaryVertexTex2(t.p3, stream);
        WriteBinaryMaterial(t.material, stream);
        IOUtils::WriteBinaryString<1>(t.tex1Name, stream);
        IOUtils::WriteBinaryString<1>(t.tex2Name, stream);
        IOUtils::WriteBinaryBool(t.variableTex2, stream);
        IOUtils::WriteBinary<4, int>(t.lodLevel, stream);
        IOUtils::WriteBinary<4, int>(t.state, stream);

        if (stream.fail())
        {
            GetLogger()->Error("Error writing model file\n");
            return false;
        }
    }

    return true;
}


const std::vector<ModelTriangle>& CModelFile::GetTriangles()
{
    return m_triangles;
}

int CModelFile::GetTriangleCount()
{
    return m_triangles.size();
}


} // namespace Gfx
