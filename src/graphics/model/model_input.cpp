#include "graphics/model/model_input.h"

#include "common/ioutils.h"
#include "common/logger.h"
#include "common/resources/inputstream.h"

#include "graphics/model/model_io_exception.h"
#include "graphics/model/model_io_structs.h"

#include <fstream>
#include <cstdio>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

namespace Gfx {

// Private functions
namespace ModelInput
{
    void ReadTextModel(CModel &model, std::istream &stream);
    void ReadTextModelV1AndV2(CModel &model, std::istream &stream);
    void ReadTextModelV3(CModel &model, std::istream &stream);

    void ReadBinaryModel(CModel &model, std::istream &stream);
    void ReadBinaryModelV1AndV2(CModel &model, std::istream &stream);
    void ReadBinaryModelV3(CModel &model, std::istream &stream);

    void ReadOldModel(CModel &model, std::istream &stream);
    std::vector<ModelTriangle> ReadOldModelV1(std::istream &stream, int totalTriangles);
    std::vector<ModelTriangle> ReadOldModelV2(std::istream &stream, int totalTriangles);
    std::vector<ModelTriangle> ReadOldModelV3(std::istream &stream, int totalTriangles);

    Vertex ReadBinaryVertex(std::istream& stream);
    VertexTex2 ReadBinaryVertexTex2(std::istream& stream);
    Material ReadBinaryMaterial(std::istream& stream);

    std::string ReadLineString(std::istream& stream, const std::string& prefix);
    VertexTex2 ParseVertexTex2(const std::string& text);
    Material ParseTextMaterial(const std::string& text);

    void ConvertOldTex1Name(ModelTriangle& triangle, const char* tex1Name);
    void ConvertFromOldRenderState(ModelTriangle& triangle, int state);
    ModelLODLevel MinMaxToLodLevel(float min, float max);
}

using namespace IOUtils;

CModel ModelInput::Read(std::istream &stream, ModelFormat format)
{
    stream.exceptions(std::ios_base::failbit | std::ios_base::badbit);

    CModel model;

    try
    {
        switch (format)
        {
            case ModelFormat::Binary:
                ReadBinaryModel(model, stream);
                break;

            case ModelFormat::Text:
                ReadTextModel(model, stream);
                break;

            case ModelFormat::Old:
                ReadOldModel(model, stream);
                break;
        }
    }
    catch (const CModelIOException& e)
    {
        throw;
    }
    catch (const std::exception& e)
    {
        throw CModelIOException(std::string("Other error while reading model data: ") + e.what());
    }

    return model;
}

void ModelInput::ReadBinaryModel(CModel &model, std::istream &stream)
{
    int version = 0;
    try
    {
        version = ReadBinary<4, int>(stream);
        stream.seekg(std::ios_base::beg);
    }
    catch (const std::exception& e)
    {
        throw CModelIOException(std::string("Error reading version number: ") + e.what());
    }

    if (version == 1 || version == 2)
        ReadBinaryModelV1AndV2(model, stream);
    else if (version == 3)
        ReadBinaryModelV3(model, stream);
    else
        throw CModelIOException(std::string("Unexpected version number: ") + boost::lexical_cast<std::string>(version));
}

void ModelInput::ReadBinaryModelV1AndV2(CModel &model, std::istream &stream)
{
    ModelHeaderV1AndV2 header;
    try
    {
        header.version        = ReadBinary<4, int>(stream);
        header.totalTriangles = ReadBinary<4, int>(stream);
    }
    catch (const std::exception& e)
    {
        throw CModelIOException(std::string("Error reading model file header: ") + e.what());
    }

    CModelMesh mesh;

    try
    {
        for (int i = 0; i < header.totalTriangles; ++i)
        {
            ModelTriangleV1AndV2 t;

            t.p1 = ReadBinaryVertexTex2(stream);
            t.p2 = ReadBinaryVertexTex2(stream);
            t.p3 = ReadBinaryVertexTex2(stream);
            t.material = ReadBinaryMaterial(stream);
            t.tex1Name = ReadBinaryString<1>(stream);
            t.tex2Name = ReadBinaryString<1>(stream);
            t.variableTex2 = ReadBinaryBool(stream);

            if (header.version == 1)
                t.lodLevel = static_cast<ModelLODLevel>( ReadBinary<4, int>(stream) );

            t.state = ReadBinary<4, int>(stream);

            if (t.lodLevel == ModelLODLevel::Low ||
                t.lodLevel == ModelLODLevel::Medium)
                continue;

            ModelTriangle triangle;
            triangle.p1 = t.p1;
            triangle.p2 = t.p2;
            triangle.p3 = t.p3;
            triangle.diffuse = t.material.diffuse;
            triangle.specular = t.material.specular;
            triangle.ambient = t.material.ambient;
            triangle.tex1Name = t.tex1Name;
            triangle.tex2Name = t.tex2Name;
            triangle.variableTex2 = t.variableTex2;
            ConvertFromOldRenderState(triangle, t.state);

            mesh.AddTriangle(triangle);
        }
    }
    catch (const std::exception& e)
    {
        throw CModelIOException(std::string("Error reading model data: ") + e.what());
    }

    model.AddMesh("main", std::move(mesh));
}

void ModelInput::ReadBinaryModelV3(CModel &model, std::istream &stream)
{
    // TODO...
}

void ModelInput::ReadTextModel(CModel &model, std::istream &stream)
{
    int version = 0;
    try
    {
        version = boost::lexical_cast<int>(ReadLineString(stream, "version"));
        stream.seekg(std::ios_base::beg);
    }
    catch (const std::exception& e)
    {
        throw CModelIOException(std::string("Error reading version number: ") + e.what());
    }

    if (version == 1 || version == 2)
        ReadTextModelV1AndV2(model, stream);
    else if (version == 3)
        ReadTextModelV3(model, stream);
    else
        throw CModelIOException(std::string("Unexpected version number: ") + boost::lexical_cast<std::string>(version));
}

void ModelInput::ReadTextModelV1AndV2(CModel &model, std::istream &stream)
{
    ModelHeaderV1AndV2 header;

    try
    {
        header.version = boost::lexical_cast<int>(ReadLineString(stream, "version"));
        header.totalTriangles = boost::lexical_cast<int>(ReadLineString(stream, "total_triangles"));
    }
    catch (const std::exception& e)
    {
        throw CModelIOException(std::string("Error reading model header: ") + e.what());
    }

    CModelMesh mesh;

    for (int i = 0; i < header.totalTriangles; ++i)
    {
        ModelTriangleV1AndV2 t;

        std::string p1Text = ReadLineString(stream, "p1");
        t.p1 = ParseVertexTex2(p1Text);
        std::string p2Text = ReadLineString(stream, "p2");
        t.p2 = ParseVertexTex2(p2Text);
        std::string p3Text = ReadLineString(stream, "p3");
        t.p3 = ParseVertexTex2(p3Text);

        std::string matText = ReadLineString(stream, "mat");
        t.material = ParseTextMaterial(matText);

        t.tex1Name = ReadLineString(stream, "tex1");
        t.tex2Name = ReadLineString(stream, "tex2");
        std::string varTex2Ch = ReadLineString(stream, "var_tex2");
        t.variableTex2 = varTex2Ch == "Y";

        if (header.version == 1)
            t.lodLevel = static_cast<ModelLODLevel>( boost::lexical_cast<int>(ReadLineString(stream, "lod_level")) );

        t.state = boost::lexical_cast<int>(ReadLineString(stream, "state"));

        if (t.lodLevel == ModelLODLevel::Low ||
            t.lodLevel == ModelLODLevel::Medium)
            continue;

        ModelTriangle triangle;
        triangle.p1 = t.p1;
        triangle.p2 = t.p2;
        triangle.p3 = t.p3;
        triangle.ambient = t.material.ambient;
        triangle.diffuse = t.material.diffuse;
        triangle.specular = t.material.specular;
        triangle.tex1Name = t.tex1Name;
        triangle.tex2Name = t.tex2Name;
        triangle.variableTex2 = t.variableTex2;
        ConvertFromOldRenderState(triangle, t.state);

        mesh.AddTriangle(triangle);
    }

    model.AddMesh("main", std::move(mesh));
}

void ModelInput::ReadTextModelV3(CModel &model, std::istream &stream)
{
    // TODO...
}

void ModelInput::ReadOldModel(CModel &model, std::istream &stream)
{
    OldModelHeader header;

    try
    {
        header.revision = ReadBinary<4, int>(stream);
        header.version = ReadBinary<4, int>(stream);
        header.totalTriangles = ReadBinary<4, int>(stream);
        for (int i = 0; i < 10; ++i)
            header.reserved[i] = ReadBinary<4, int>(stream);
    }
    catch (const std::exception& e)
    {
        throw CModelIOException(std::string("Error reading model file header: ") + e.what());
    }

    std::vector<ModelTriangle> triangles;

    try
    {
        if (header.revision == 1 && header.version == 0)
        {
            triangles = ReadOldModelV1(stream, header.totalTriangles);
        }
        else if (header.revision == 1 && header.version == 1)
        {
            triangles = ReadOldModelV2(stream, header.totalTriangles);
        }
        else
        {
            triangles = ReadOldModelV3(stream, header.totalTriangles);
        }
    }
    catch (const std::exception& e)
    {
        throw CModelIOException(std::string("Error reading model triangles: ") + e.what());
    }

    CModelMesh mesh;
    mesh.SetTriangles(std::move(triangles));

    model.AddMesh("main", std::move(mesh));
}

std::vector<ModelTriangle> ModelInput::ReadOldModelV1(std::istream &stream, int totalTriangles)
{
    std::vector<ModelTriangle> triangles;

    for (int i = 0; i < totalTriangles; ++i)
    {
        OldModelTriangleV1 t;
        t.used     = ReadBinary<1, char>(stream);
        t.selected = ReadBinary<1, char>(stream);

        /* padding */ ReadBinary<2, unsigned int>(stream);

        t.p1 = ReadBinaryVertex(stream);
        t.p2 = ReadBinaryVertex(stream);
        t.p3 = ReadBinaryVertex(stream);

        t.material = ReadBinaryMaterial(stream);
        stream.read(t.texName, 20);
        t.min = ReadBinaryFloat(stream);
        t.max = ReadBinaryFloat(stream);

        ModelLODLevel lodLevel = MinMaxToLodLevel(t.min, t.max);
        if (lodLevel == ModelLODLevel::Low ||
            lodLevel == ModelLODLevel::Medium)
            continue;

        ModelTriangle triangle;
        triangle.p1.FromVertex(t.p1);
        triangle.p2.FromVertex(t.p2);
        triangle.p3.FromVertex(t.p3);

        triangle.ambient = t.material.ambient;
        triangle.diffuse = t.material.diffuse;
        triangle.specular = t.material.specular;
        ConvertOldTex1Name(triangle, t.texName);

        triangles.push_back(triangle);
    }

    return triangles;
}

std::vector<ModelTriangle> ModelInput::ReadOldModelV2(std::istream &stream, int totalTriangles)
{
    std::vector<ModelTriangle> triangles;

    for (int i = 0; i < totalTriangles; ++i)
    {
        OldModelTriangleV2 t;
        t.used     = ReadBinary<1, char>(stream);
        t.selected = ReadBinary<1, char>(stream);

        /* padding */ ReadBinary<2, unsigned int>(stream);

        t.p1 = ReadBinaryVertex(stream);
        t.p2 = ReadBinaryVertex(stream);
        t.p3 = ReadBinaryVertex(stream);

        t.material = ReadBinaryMaterial(stream);
        stream.read(t.texName, 20);
        t.min = ReadBinaryFloat(stream);
        t.max = ReadBinaryFloat(stream);
        t.state = ReadBinary<4, long>(stream);

        t.reserved1 = ReadBinary<2, short>(stream);
        t.reserved2 = ReadBinary<2, short>(stream);
        t.reserved3 = ReadBinary<2, short>(stream);
        t.reserved4 = ReadBinary<2, short>(stream);

        ModelLODLevel lodLevel = MinMaxToLodLevel(t.min, t.max);
        if (lodLevel == ModelLODLevel::Low ||
            lodLevel == ModelLODLevel::Medium)
            continue;

        ModelTriangle triangle;
        triangle.p1.FromVertex(t.p1);
        triangle.p2.FromVertex(t.p2);
        triangle.p3.FromVertex(t.p3);

        triangle.ambient = t.material.ambient;
        triangle.diffuse = t.material.diffuse;
        triangle.specular = t.material.specular;
        ConvertOldTex1Name(triangle, t.texName);

        ConvertFromOldRenderState(triangle, t.state);

        triangles.push_back(triangle);
    }

    return triangles;
}

std::vector<ModelTriangle> ModelInput::ReadOldModelV3(std::istream &stream, int totalTriangles)
{
    std::vector<ModelTriangle> triangles;

    for (int i = 0; i < totalTriangles; ++i)
    {
        OldModelTriangleV3 t;
        t.used     = ReadBinary<1, char>(stream);
        t.selected = ReadBinary<1, char>(stream);

        /* padding */ ReadBinary<2, unsigned int>(stream);

        t.p1 = ReadBinaryVertexTex2(stream);
        t.p2 = ReadBinaryVertexTex2(stream);
        t.p3 = ReadBinaryVertexTex2(stream);

        t.material = ReadBinaryMaterial(stream);
        stream.read(t.texName, 20);
        t.min = ReadBinaryFloat(stream);
        t.max = ReadBinaryFloat(stream);
        t.state = ReadBinary<4, long>(stream);
        t.texNum2 = ReadBinary<2, short>(stream);

        t.reserved2 = ReadBinary<2, short>(stream);
        t.reserved3 = ReadBinary<2, short>(stream);
        t.reserved4 = ReadBinary<2, short>(stream);

        ModelLODLevel lodLevel = MinMaxToLodLevel(t.min, t.max);
        if (lodLevel == ModelLODLevel::Low ||
            lodLevel == ModelLODLevel::Medium)
            continue;

        ModelTriangle triangle;
        triangle.p1 = t.p1;
        triangle.p2 = t.p2;
        triangle.p3 = t.p3;

        triangle.ambient = t.material.ambient;
        triangle.diffuse = t.material.diffuse;
        triangle.specular = t.material.specular;
        ConvertOldTex1Name(triangle, t.texName);

        ConvertFromOldRenderState(triangle, t.state);
        triangle.variableTex2 = t.texNum2 == 1;

        if (!triangle.variableTex2 && t.texNum2 != 0)
        {
            char tex2Name[20] = { 0 };
            std::sprintf(tex2Name, "dirty%.2d.png", t.texNum2);
            triangle.tex2Name = tex2Name;
        }

        triangles.push_back(triangle);
    }

    return triangles;
}

ModelLODLevel ModelInput::MinMaxToLodLevel(float min, float max)
{
    if (min == 0.0f && max == 100.0f)
        return ModelLODLevel::High;
    else if (min == 100.0f && max == 200.0f)
        return ModelLODLevel::Medium;
    else if (min == 200.0f && max == 1000000.0f)
        return ModelLODLevel::Low;
    else if (min == 0.0f && max == 1000000.0f)
        return ModelLODLevel::Constant;

    return ModelLODLevel::Constant;
}

void ModelInput::ConvertOldTex1Name(ModelTriangle& triangle, const char* tex1Name)
{
    triangle.tex1Name = tex1Name;
    boost::replace_all(triangle.tex1Name, "bmp", "png");
    boost::replace_all(triangle.tex1Name, "tga", "png");
}

void ModelInput::ConvertFromOldRenderState(ModelTriangle& triangle, int state)
{
    if (triangle.tex1Name == "plant.png" || (state & static_cast<int>(ModelRenderState::Alpha)) != 0)
        triangle.transparentMode = ModelTransparentMode::AlphaChannel;
    else if ((state & static_cast<int>(ModelRenderState::TTextureBlack)) != 0)
        triangle.transparentMode = ModelTransparentMode::MapBlackToAlpha;
    else if ((state & static_cast<int>(ModelRenderState::TTextureWhite)) != 0)
        triangle.transparentMode = ModelTransparentMode::MapWhiteToAlpha;
    else
        triangle.transparentMode = ModelTransparentMode::None;

    if ((state & static_cast<int>(ModelRenderState::Part1)) != 0)
        triangle.specialMark = ModelSpecialMark::Part1;
    else if ((state & static_cast<int>(ModelRenderState::Part2)) != 0)
        triangle.specialMark = ModelSpecialMark::Part2;
    else if ((state & static_cast<int>(ModelRenderState::Part3)) != 0)
        triangle.specialMark = ModelSpecialMark::Part3;
    else
        triangle.specialMark = ModelSpecialMark::None;

    triangle.doubleSided = (state & static_cast<int>(ModelRenderState::TwoFace)) != 0;
}

Vertex ModelInput::ReadBinaryVertex(std::istream& stream)
{
    Vertex vertex;

    vertex.coord.x     = ReadBinaryFloat(stream);
    vertex.coord.y     = ReadBinaryFloat(stream);
    vertex.coord.z     = ReadBinaryFloat(stream);

    vertex.normal.x    = ReadBinaryFloat(stream);
    vertex.normal.y    = ReadBinaryFloat(stream);
    vertex.normal.z    = ReadBinaryFloat(stream);

    vertex.texCoord.x  = ReadBinaryFloat(stream);
    vertex.texCoord.y  = ReadBinaryFloat(stream);

    return vertex;
}

VertexTex2 ModelInput::ReadBinaryVertexTex2(std::istream& stream)
{
    VertexTex2 vertex;

    vertex.coord.x     = ReadBinaryFloat(stream);
    vertex.coord.y     = ReadBinaryFloat(stream);
    vertex.coord.z     = ReadBinaryFloat(stream);

    vertex.normal.x    = ReadBinaryFloat(stream);
    vertex.normal.y    = ReadBinaryFloat(stream);
    vertex.normal.z    = ReadBinaryFloat(stream);

    vertex.texCoord.x  = ReadBinaryFloat(stream);
    vertex.texCoord.y  = ReadBinaryFloat(stream);

    vertex.texCoord2.x = ReadBinaryFloat(stream);
    vertex.texCoord2.y = ReadBinaryFloat(stream);

    return vertex;
}

Material ModelInput::ReadBinaryMaterial(std::istream& stream)
{
    Material material;

    material.diffuse.r =  ReadBinaryFloat(stream);
    material.diffuse.g =  ReadBinaryFloat(stream);
    material.diffuse.b =  ReadBinaryFloat(stream);
    material.diffuse.a =  ReadBinaryFloat(stream);

    material.ambient.r =  ReadBinaryFloat(stream);
    material.ambient.g =  ReadBinaryFloat(stream);
    material.ambient.b =  ReadBinaryFloat(stream);
    material.ambient.a =  ReadBinaryFloat(stream);

    material.specular.r = ReadBinaryFloat(stream);
    material.specular.g = ReadBinaryFloat(stream);
    material.specular.b = ReadBinaryFloat(stream);
    material.specular.a = ReadBinaryFloat(stream);

    /* emissive.r = */    ReadBinaryFloat(stream);
    /* emissive.g = */    ReadBinaryFloat(stream);
    /* emissive.b = */    ReadBinaryFloat(stream);
    /* emissive.a = */    ReadBinaryFloat(stream);

    /* power = */         ReadBinaryFloat(stream);

    return material;
}


std::string ModelInput::ReadLineString(std::istream& stream, const std::string& expectedPrefix)
{
    std::string line;
    while (true)
    {
        if (stream.eof())
            throw CModelIOException("Unexpected EOF");

        std::getline(stream, line);
        if (!line.empty() && line[0] != '#')
            break;
    }

    std::stringstream s;
    s.str(line);

    std::string prefix;
    s >> prefix;
    if (prefix != expectedPrefix)
        throw CModelIOException(std::string("Unexpected prefix: '") + prefix + "'");

    std::string value;
    std::getline(s, value);
    boost::trim_left(value);

    return value;
}

VertexTex2 ModelInput::ParseVertexTex2(const std::string& text)
{
    VertexTex2 vertex;

    std::stringstream stream;
    stream.str(text);

    std::string prefix;

    stream >> prefix;
    if (prefix != "c")
        throw CModelIOException(std::string("Unexpected prefix: '") + prefix + "'");

    stream >> vertex.coord.x >> vertex.coord.y >> vertex.coord.z;

    stream >> prefix;
    if (prefix != "n")
        throw CModelIOException(std::string("Unexpected prefix: '") + prefix + "'");

    stream >> vertex.normal.x >> vertex.normal.y >> vertex.normal.z;

    stream >> prefix;
    if (prefix != "t1")
        throw CModelIOException(std::string("Unexpected prefix: '") + prefix + "'");

    stream >> vertex.texCoord.x >> vertex.texCoord.y;

    stream >> prefix;
    if (prefix != "t2")
        throw CModelIOException(std::string("Unexpected prefix: '") + prefix + "'");

    stream >> vertex.texCoord2.x >> vertex.texCoord2.y;

    return vertex;
}

Material ModelInput::ParseTextMaterial(const std::string& text)
{
    Material material;

    std::stringstream stream;
    stream.str(text);

    std::string prefix;

    stream >> prefix;
    if (prefix != "dif")
        throw CModelIOException(std::string("Unexpected prefix: '") + prefix + "'");

    stream >> material.diffuse.r
           >> material.diffuse.g
           >> material.diffuse.b
           >> material.diffuse.a;

    stream >> prefix;
    if (prefix != "amb")
        throw CModelIOException(std::string("Unexpected prefix: '") + prefix + "'");

    stream >> material.ambient.r
           >> material.ambient.g
           >> material.ambient.b
           >> material.ambient.a;

    stream >> prefix;
    if (prefix != "spc")
        throw CModelIOException(std::string("Unexpected prefix: '") + prefix + "'");

    stream >> material.specular.r
           >> material.specular.g
           >> material.specular.b
           >> material.specular.a;

    return material;
}

} // namespace Gfx
