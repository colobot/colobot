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

#include "graphics/model/model_output.h"

#include "common/ioutils.h"

#include "graphics/model/model.h"
#include "graphics/model/model_io_exception.h"
#include "graphics/model/model_io_structs.h"

#include <cassert>
#include <fstream>

namespace Gfx
{

// Private functions
namespace ModelOutput
{
    void WriteTextModel(const CModel& model, std::ostream &stream);
    void WriteTextHeader(const ModelHeaderV3& header, std::ostream &stream);
    void WriteCrashSphere(const ModelCrashSphere& crashSphere, std::ostream &stream);
    void WriteShadowSpot(const ModelShadowSpot& shadowSpot, std::ostream &stream);
    void WriteCameraCollisionSphere(const Math::Sphere& sphere, std::ostream &stream);
    void WriteTextMesh(const CModelMesh* mesh, const std::string& meshName, std::ostream &stream);
    std::string VectorToString(const Math::Vector& vector);
    std::string TransparentModeToString(ModelTransparentMode mode);
    std::string SpecialMarkToString(ModelSpecialMark specialMark);

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
    ModelHeaderV3 header;
    header.version = 3;
    header.totalCrashSpheres = model.GetCrashSphereCount();
    header.hasShadowSpot = model.HasShadowSpot();
    header.hasCameraCollisionSphere = model.HasCameraCollisionSphere();
    header.totalMeshes = model.GetMeshCount();
    WriteTextHeader(header, stream);

    stream << "# MODEL PROPERTIES" << std::endl;

    for (const auto& crashSphere : model.GetCrashSpheres())
        WriteCrashSphere(crashSphere, stream);

    if (model.HasShadowSpot())
        WriteShadowSpot(model.GetShadowSpot(), stream);

    if (model.HasCameraCollisionSphere())
        WriteCameraCollisionSphere(model.GetCameraCollisionSphere(), stream);

    stream << std::endl;

    for (const std::string& meshName : model.GetMeshNames())
    {
        const CModelMesh* mesh = model.GetMesh(meshName);
        assert(mesh != nullptr);
        WriteTextMesh(mesh, meshName, stream);
    }
}

void ModelOutput::WriteTextHeader(const ModelHeaderV3& header, std::ostream &stream)
{
    stream << "# Colobot text model" << std::endl;
    stream << std::endl;
    stream << "### HEAD" << std::endl;
    stream << "version " << header.version << std::endl;
    stream << "total_crash_spheres " << header.totalCrashSpheres << std::endl;
    stream << "has_shadow_spot " << (header.hasShadowSpot ? "Y" : "N") << std::endl;
    stream << "has_camera_collision_sphere " << (header.hasCameraCollisionSphere ? "Y" : "N") << std::endl;
    stream << "total_meshes " << header.totalMeshes << std::endl;
    stream << std::endl;
}

void ModelOutput::WriteCrashSphere(const ModelCrashSphere& crashSphere, std::ostream &stream)
{
    stream << "crash_sphere";
    stream << " pos"
           << " " << crashSphere.position.x
           << " " << crashSphere.position.y
           << " " << crashSphere.position.z;
    stream << " rad " << crashSphere.radius;
    stream << " sound " << crashSphere.sound;
    stream << " hard " << crashSphere.hardness;
    stream << std::endl;
}

void ModelOutput::WriteShadowSpot(const ModelShadowSpot& shadowSpot, std::ostream &stream)
{
    stream << "shadow_spot";
    stream << " rad " << shadowSpot.radius;
    stream << " int " << shadowSpot.intensity;
    stream << std::endl;
}

void ModelOutput::WriteCameraCollisionSphere(const Math::Sphere& sphere, std::ostream &stream)
{
    stream << "camera_collision_sphere ";
    stream << " pos "
           << sphere.pos.x
           << sphere.pos.y
           << sphere.pos.z;
    stream << " rad " << sphere.radius;
    stream << std::endl;
}

void ModelOutput::WriteTextMesh(const CModelMesh* mesh, const std::string& meshName, std::ostream &stream)
{
    stream << "### MESH" << std::endl;
    stream << "mesh " << meshName << std::endl;
    stream << "position " << VectorToString(mesh->GetPosition()) << std::endl;
    stream << "rotation " << VectorToString(mesh->GetRotation()) << std::endl;
    stream << "scale " << VectorToString(mesh->GetScale()) << std::endl;
    stream << "parent " << mesh->GetParent() << std::endl;
    stream << "total_triangles " << mesh->GetTriangleCount() << std::endl;
    stream << std::endl;

    stream << "### MESH TRIANGLES" << std::endl;
    for (const ModelTriangle& t : mesh->GetTriangles())
    {
        stream << "p1 ";
        WriteTextVertexTex2(t.p1, stream);
        stream << "p2 ";
        WriteTextVertexTex2(t.p2, stream);
        stream << "p3 ";
        WriteTextVertexTex2(t.p3, stream);
        stream << "mat ";
        Material material;
        material.ambient = t.ambient;
        material.diffuse = t.diffuse;
        material.specular = t.specular;
        WriteTextMaterial(material, stream);

        stream << "tex1 " << t.tex1Name << std::endl;
        stream << "tex2 " << t.tex2Name << std::endl;
        stream << "var_tex2 " << (t.variableTex2 ? 'Y' : 'N') << std::endl;
        stream << "trans_mode " << TransparentModeToString(t.transparentMode) << std::endl;
        stream << "mark " << SpecialMarkToString(t.specialMark) << std::endl;
        stream << "dbl_side " << (t.doubleSided ? 'Y' : 'N') << std::endl;

        stream << std::endl;
    }
}

std::string ModelOutput::VectorToString(const Math::Vector& vector)
{
    std::ostringstream str;
    str << vector.x << " " << vector.y << " " << vector.z;
    return str.str();
}

std::string ModelOutput::TransparentModeToString(ModelTransparentMode mode)
{
    std::string str;
    switch (mode)
    {
        case ModelTransparentMode::None:
            str = "none";
            break;

        case ModelTransparentMode::AlphaChannel:
            str = "alpha";
            break;

        case ModelTransparentMode::MapBlackToAlpha:
            str = "map_black";
            break;

        case ModelTransparentMode::MapWhiteToAlpha:
            str = "map_white";
            break;
    }
    return str;
}

std::string ModelOutput::SpecialMarkToString(ModelSpecialMark specialMark)
{
    std::string str;
    switch (specialMark)
    {
        case ModelSpecialMark::None:
            str = "none";
            break;

        case ModelSpecialMark::Part1:
            str = "part1";
            break;

        case ModelSpecialMark::Part2:
            str = "part2";
            break;

        case ModelSpecialMark::Part3:
            str = "part3";
            break;
    }
    return str;
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

        strncpy(t.texName, triangle.tex1Name.c_str(), sizeof(t.texName)-1);
        t.texName[sizeof(t.texName)-1] = '\0';

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
