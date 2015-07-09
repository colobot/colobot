#include "graphics/model/model_output.h"

#include "common/ioutils.h"

#include "graphics/model/model_io_exception.h"
#include "graphics/model/model_io_structs.h"

#include "graphics/model/model.h"

#include <fstream>

namespace Gfx {

// Private functions
namespace ModelOutput
{
    void WriteTextModel(const CModel& model, std::ostream &stream);

    void WriteBinaryModel(const CModel& model, std::ostream &stream);

    void WriteOldModel(const CModel& model, std::ostream &stream);

    int ConvertToOldState(const ModelTriangle& triangle);

    void WriteBinaryVertexTex2(VertexTex2 vertex, std::ostream &stream);
    void WriteBinaryMaterial(const Material& material, std::ostream &stream);

    void WriteTextVertexTex2(const VertexTex2& vertex, std::ostream &stream);
    void WriteTextMaterial(const Material& material, std::ostream &stream);
}

using namespace IOUtils;

void ModelOutput::Write(const CModel& model, std::ostream &stream, ModelFormat format)
{
    stream.exceptions(std::ios_base::failbit | std::ios_base::badbit);

    try
    {
        switch (format)
        {
            case ModelFormat::Text:
                WriteTextModel(model, stream);
                break;

            case ModelFormat::Binary:
                WriteBinaryModel(model, stream);
                break;

            case ModelFormat::Old:
                WriteOldModel(model, stream);
                break;
        }
    }
    catch (const CModelIOException& e)
    {
        throw;
    }
    catch (const std::exception& e)
    {
        throw CModelIOException(std::string("Error saving model data: ") + e.what());
    }
}

void ModelOutput::WriteTextModel(const CModel& model, std::ostream &stream)
{
    const CModelMesh* mesh = model.GetMesh("main");
    if (mesh == nullptr)
        throw CModelIOException("No main mesh found in model");

    ModelHeaderV1AndV2 header;

    header.version = 2;
    header.totalTriangles = mesh->GetTriangleCount();

    stream << "# Colobot text model" << std::endl;
    stream << std::endl;
    stream << "### HEAD" << std::endl;
    stream << "version " << header.version << std::endl;
    stream << "total_triangles " << header.totalTriangles << std::endl;
    stream << std::endl;
    stream << "### TRIANGLES" << std::endl;

    for (const ModelTriangle& triangle : mesh->GetTriangles())
    {
        ModelTriangleV1AndV2 t;

        t.p1 = triangle.p1;
        t.p2 = triangle.p2;
        t.p3 = triangle.p3;
        t.material.ambient = triangle.ambient;
        t.material.diffuse = triangle.diffuse;
        t.material.specular = triangle.specular;
        t.tex1Name = triangle.tex1Name;
        t.tex2Name = triangle.tex2Name;
        t.variableTex2 = triangle.variableTex2;
        t.state = ConvertToOldState(triangle);

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
        stream << "state " << t.state << std::endl;

        stream << std::endl;
    }
}

void ModelOutput::WriteBinaryModel(const CModel& model, std::ostream &stream)
{
    const CModelMesh* mesh = model.GetMesh("main");
    if (mesh == nullptr)
        throw CModelIOException("No main mesh found in model");

    ModelHeaderV1AndV2 header;

    header.version = 2;
    header.totalTriangles = mesh->GetTriangleCount();

    WriteBinary<4, int>(header.version, stream);
    WriteBinary<4, int>(header.totalTriangles, stream);

    for (const ModelTriangle& triangle : mesh->GetTriangles())
    {
        ModelTriangleV1AndV2 t;

        t.p1 = triangle.p1;
        t.p2 = triangle.p2;
        t.p3 = triangle.p3;
        t.material.ambient = triangle.ambient;
        t.material.diffuse = triangle.diffuse;
        t.material.specular = triangle.specular;
        t.tex1Name = triangle.tex1Name;
        t.tex2Name = triangle.tex2Name;
        t.variableTex2 = triangle.variableTex2;
        t.state = ConvertToOldState(triangle);

        WriteBinaryVertexTex2(t.p1, stream);
        WriteBinaryVertexTex2(t.p2, stream);
        WriteBinaryVertexTex2(t.p3, stream);
        WriteBinaryMaterial(t.material, stream);
        WriteBinaryString<1>(t.tex1Name, stream);
        WriteBinaryString<1>(t.tex2Name, stream);
        WriteBinaryBool(t.variableTex2, stream);
        WriteBinary<4, int>(t.state, stream);
    }
}

void ModelOutput::WriteOldModel(const CModel& model, std::ostream &stream)
{
    const CModelMesh* mesh = model.GetMesh("main");
    if (mesh == nullptr)
        throw CModelIOException("No main mesh found in model");

    OldModelHeader header;
    header.revision = 1;
    header.version = 2;
    header.totalTriangles = mesh->GetTriangleCount();

    WriteBinary<4, int>(header.revision, stream);
    WriteBinary<4, int>(header.version, stream);
    WriteBinary<4, int>(header.totalTriangles, stream);
    for (int i = 0; i < 10; ++i)
        WriteBinary<4, int>(header.reserved[i], stream);

    for (const ModelTriangle& triangle : mesh->GetTriangles())
    {
        OldModelTriangleV3 t;

        t.used = true;

        t.p1 = triangle.p1;
        t.p2 = triangle.p2;
        t.p3 = triangle.p3;

        t.material.ambient = triangle.ambient;
        t.material.diffuse = triangle.diffuse;
        t.material.specular = triangle.specular;
        strncpy(t.texName, triangle.tex1Name.c_str(), 20);
        t.min = 0.0f;
        t.max = 1000000.0f;
        t.state = ConvertToOldState(triangle);

        int no = 0;
        if (triangle.variableTex2)
            no = 1;
        else
            std::sscanf(triangle.tex2Name.c_str(), "dirty%d.png", &no);

        t.texNum2 = no;


        WriteBinary<1, char>(t.used,     stream);
        WriteBinary<1, char>(t.selected, stream);

        /* padding */ WriteBinary<2, unsigned int>(0, stream);

        WriteBinaryVertexTex2(t.p1, stream);
        WriteBinaryVertexTex2(t.p2, stream);
        WriteBinaryVertexTex2(t.p3, stream);

        WriteBinaryMaterial(t.material, stream);
        stream.write(t.texName, 20);
        WriteBinaryFloat(t.min, stream);
        WriteBinaryFloat(t.max, stream);
        WriteBinary<4, long>(t.state, stream);
        WriteBinary<2, short>(t.texNum2, stream);

        WriteBinary<2, short>(t.reserved2, stream);
        WriteBinary<2, short>(t.reserved3, stream);
        WriteBinary<2, short>(t.reserved4, stream);
    }
}

int ModelOutput::ConvertToOldState(const ModelTriangle& triangle)
{
    int state = 0;

    switch (triangle.transparentMode)
    {
        case ModelTransparentMode::None:
            break;

        case ModelTransparentMode::AlphaChannel:
            state |= static_cast<int>(ModelRenderState::Alpha);
            break;

        case ModelTransparentMode::MapBlackToAlpha:
            state |= static_cast<int>(ModelRenderState::TTextureBlack);
            break;

        case ModelTransparentMode::MapWhiteToAlpha:
            state |= static_cast<int>(ModelRenderState::TTextureWhite);
            break;
    }

    switch (triangle.specialMark)
    {
        case ModelSpecialMark::None:
            break;

        case ModelSpecialMark::Part1:
            state |= static_cast<int>(ModelRenderState::Part1);
            break;

        case ModelSpecialMark::Part2:
            state |= static_cast<int>(ModelRenderState::Part2);
            break;

        case ModelSpecialMark::Part3:
            state |= static_cast<int>(ModelRenderState::Part3);
            break;
    }

    if (triangle.doubleSided)
        state |= static_cast<int>(ModelRenderState::TwoFace);

    return state;
}

void ModelOutput::WriteBinaryVertexTex2(VertexTex2 vertex, std::ostream &stream)
{
    WriteBinaryFloat(vertex.coord.x, stream);
    WriteBinaryFloat(vertex.coord.y, stream);
    WriteBinaryFloat(vertex.coord.z, stream);
    WriteBinaryFloat(vertex.normal.x, stream);
    WriteBinaryFloat(vertex.normal.y, stream);
    WriteBinaryFloat(vertex.normal.z, stream);
    WriteBinaryFloat(vertex.texCoord.x, stream);
    WriteBinaryFloat(vertex.texCoord.y, stream);
    WriteBinaryFloat(vertex.texCoord2.x, stream);
    WriteBinaryFloat(vertex.texCoord2.y, stream);
}

void ModelOutput::WriteBinaryMaterial(const Material& material, std::ostream &stream)
{
    WriteBinaryFloat(material.diffuse.r, stream);
    WriteBinaryFloat(material.diffuse.g, stream);
    WriteBinaryFloat(material.diffuse.b, stream);
    WriteBinaryFloat(material.diffuse.a, stream);

    WriteBinaryFloat(material.ambient.r, stream);
    WriteBinaryFloat(material.ambient.g, stream);
    WriteBinaryFloat(material.ambient.b, stream);
    WriteBinaryFloat(material.ambient.a, stream);

    WriteBinaryFloat(material.specular.r, stream);
    WriteBinaryFloat(material.specular.g, stream);
    WriteBinaryFloat(material.specular.b, stream);
    WriteBinaryFloat(material.specular.a, stream);

    /* emissive.r */  WriteBinaryFloat(0.0f, stream);
    /* emissive.g */  WriteBinaryFloat(0.0f, stream);
    /* emissive.b */  WriteBinaryFloat(0.0f, stream);
    /* emissive.a */  WriteBinaryFloat(0.0f, stream);

    /* power */       WriteBinaryFloat(0.0f, stream);
}

void ModelOutput::WriteTextVertexTex2(const VertexTex2& vertex, std::ostream &stream)
{
    stream << "c " << vertex.coord.x << " " << vertex.coord.y << " " << vertex.coord.z;
    stream << " n " << vertex.normal.x << " " << vertex.normal.y << " " << vertex.normal.z;
    stream << " t1 " << vertex.texCoord.x << " " << vertex.texCoord.y;
    stream << " t2 " << vertex.texCoord2.x << " " << vertex.texCoord2.y;
    stream << std::endl;
}

void ModelOutput::WriteTextMaterial(const Material& material, std::ostream &stream)
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
}


} // namespace Gfx
