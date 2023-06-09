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

#include "core/stringutils.h"

#include "graphics/model/model_input.h"

#include "common/ioutils.h"
#include "common/logger.h"

#include "common/resources/inputstream.h"

#include "graphics/model/model_io_exception.h"
#include "graphics/model/model_io_structs.h"

#include <fstream>
#include <cstdio>

namespace Gfx
{

// Private functions
namespace ModelInput
{
    void ReadTextModel(CModel &model, std::istream &stream);
    void ReadTextModelV1AndV2(CModel &model, std::istream &stream);

    void ReadTextModelV3(CModel &model, std::istream &stream);
    ModelHeaderV3 ReadTextHeader(std::istream &stream);
    CModelMesh ReadTextMesh(std::istream &stream);

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

    std::string ReadLineString(std::istream& stream, const std::string& expectedPrefix);
    void ReadValuePrefix(std::istream& stream, const std::string& expectedPrefix);
    VertexTex2 ParseVertexTex2(const std::string& text);
    Material ParseMaterial(const std::string& text);
    Math::Vector ParseVector(const std::string& text);
    ModelCrashSphere ParseCrashSphere(const std::string& text);
    ModelShadowSpot ParseShadowSpot(const std::string& text);
    Math::Sphere ParseCameraCollisionSphere(const std::string& text);
    ModelTransparentMode ParseTransparentMode(const std::string& text);
    ModelSpecialMark ParseSpecialMark(const std::string& text);

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
        throw CModelIOException(std::string("Unexpected version number: ") + StrUtils::ToString(version));
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
        version = StrUtils::FromString<int>(ReadLineString(stream, "version"));
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
        throw CModelIOException(std::string("Unexpected version number: ") + StrUtils::ToString(version));
}

void ModelInput::ReadTextModelV1AndV2(CModel &model, std::istream &stream)
{
    ModelHeaderV1AndV2 header;

    try
    {
        header.version = StrUtils::FromString<int>(ReadLineString(stream, "version"));
        header.totalTriangles = StrUtils::FromString<int>(ReadLineString(stream, "total_triangles"));
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
        t.material = ParseMaterial(matText);

        t.tex1Name = ReadLineString(stream, "tex1");
        t.tex2Name = ReadLineString(stream, "tex2");
        std::string varTex2Ch = ReadLineString(stream, "var_tex2");
        t.variableTex2 = varTex2Ch == "Y";

        if (header.version == 1)
            t.lodLevel = static_cast<ModelLODLevel>( StrUtils::FromString<int>(ReadLineString(stream, "lod_level")) );

        t.state = StrUtils::FromString<int>(ReadLineString(stream, "state"));

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
    ModelHeaderV3 header;

    try
    {
        header = ReadTextHeader(stream);
    }
    catch (const std::exception& e)
    {
        throw CModelIOException(std::string("Error reading model header: ") + e.what());
    }

    for (int i = 0; i < header.totalCrashSpheres; ++i)
    {
        auto crashSphere = ParseCrashSphere(ReadLineString(stream, "crash_sphere"));
        model.AddCrashSphere(crashSphere);
    }

    if (header.hasShadowSpot)
    {
        auto shadowSpot = ParseShadowSpot(ReadLineString(stream, "shadow_spot"));
        model.SetShadowSpot(shadowSpot);
    }

    if (header.hasCameraCollisionSphere)
    {
        auto sphere = ParseCameraCollisionSphere(ReadLineString(stream, "camera_collision_sphere"));
        model.SetCameraCollisionSphere(sphere);
    }

    for (int i = 0; i < header.totalMeshes; ++i)
    {
        std::string meshName = ReadLineString(stream, "mesh");
        CModelMesh mesh = ReadTextMesh(stream);
        model.AddMesh(meshName, std::move(mesh));
    }
}

ModelHeaderV3 ModelInput::ReadTextHeader(std::istream &stream)
{
    ModelHeaderV3 header;
    header.version = StrUtils::FromString<int>(ReadLineString(stream, "version"));
    header.totalCrashSpheres = StrUtils::FromString<int>(ReadLineString(stream, "total_crash_spheres"));
    header.hasShadowSpot = ReadLineString(stream, "has_shadow_spot") == std::string("Y");
    header.hasCameraCollisionSphere = ReadLineString(stream, "has_camera_collision_sphere") == std::string("Y");
    header.totalMeshes = StrUtils::FromString<int>(ReadLineString(stream, "total_meshes"));
    return header;
}

CModelMesh ModelInput::ReadTextMesh(std::istream& stream)
{
    CModelMesh mesh;

    mesh.SetPosition(ParseVector(ReadLineString(stream, "position")));
    mesh.SetRotation(ParseVector(ReadLineString(stream, "rotation")));
    mesh.SetScale(ParseVector(ReadLineString(stream, "scale")));
    mesh.SetParent(ReadLineString(stream, "parent"));

    int totalTriangles = StrUtils::FromString<int>(ReadLineString(stream, "total_triangles"));

    for (int i = 0; i < totalTriangles; ++i)
    {
        ModelTriangleV3 t;

        std::string p1Text = ReadLineString(stream, "p1");
        t.p1 = ParseVertexTex2(p1Text);
        std::string p2Text = ReadLineString(stream, "p2");
        t.p2 = ParseVertexTex2(p2Text);
        std::string p3Text = ReadLineString(stream, "p3");
        t.p3 = ParseVertexTex2(p3Text);

        std::string matText = ReadLineString(stream, "mat");
        Material mat = ParseMaterial(matText);
        t.ambient = mat.ambient;
        t.diffuse = mat.diffuse;
        t.specular = mat.specular;

        t.tex1Name = ReadLineString(stream, "tex1");
        t.tex2Name = ReadLineString(stream, "tex2");
        t.variableTex2 = ReadLineString(stream, "var_tex2") == std::string("Y");

        t.transparentMode = ParseTransparentMode(ReadLineString(stream, "trans_mode"));
        t.specialMark = ParseSpecialMark(ReadLineString(stream, "mark"));
        t.doubleSided = ReadLineString(stream, "dbl_side") == std::string("Y");

        mesh.AddTriangle(t);
    }

    return mesh;
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
    triangle.tex1Name = StrUtils::Replace(triangle.tex1Name, "bmp", "png");
    triangle.tex1Name = StrUtils::Replace(triangle.tex1Name, "tga", "png");
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
        StrUtils::TrimRight(line);
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
    StrUtils::TrimLeft(value);

    return value;
}

void ModelInput::ReadValuePrefix(std::istream& stream, const std::string& expectedPrefix)
{
    std::string prefix;
    stream >> prefix;
    if (prefix != expectedPrefix)
        throw CModelIOException(std::string("Unexpected prefix: '") + prefix + "', expected was: '" + expectedPrefix + "'");
}

VertexTex2 ModelInput::ParseVertexTex2(const std::string& text)
{
    VertexTex2 vertex;

    std::stringstream stream;
    stream.str(text);

    ReadValuePrefix(stream, "c");
    stream >> vertex.coord.x >> vertex.coord.y >> vertex.coord.z;

    ReadValuePrefix(stream, "n");
    stream >> vertex.normal.x >> vertex.normal.y >> vertex.normal.z;

    ReadValuePrefix(stream, "t1");
    stream >> vertex.texCoord.x >> vertex.texCoord.y;

    ReadValuePrefix(stream, "t2");
    stream >> vertex.texCoord2.x >> vertex.texCoord2.y;

    return vertex;
}

Material ModelInput::ParseMaterial(const std::string& text)
{
    Material material;

    std::stringstream stream;
    stream.str(text);

    ReadValuePrefix(stream, "dif");
    stream >> material.diffuse.r
           >> material.diffuse.g
           >> material.diffuse.b
           >> material.diffuse.a;

    ReadValuePrefix(stream, "amb");
    stream >> material.ambient.r
           >> material.ambient.g
           >> material.ambient.b
           >> material.ambient.a;

    ReadValuePrefix(stream, "spc");
    stream >> material.specular.r
           >> material.specular.g
           >> material.specular.b
           >> material.specular.a;

    return material;
}

Math::Vector ModelInput::ParseVector(const std::string& text)
{
    Math::Vector vector;

    std::stringstream stream;
    stream.str(text);

    stream >> vector.x >> vector.y >> vector.z;

    return vector;
}

ModelCrashSphere ModelInput::ParseCrashSphere(const std::string& text)
{
    ModelCrashSphere crashSphere;

    std::stringstream stream;
    stream.str(text);

    ReadValuePrefix(stream, "pos");
    stream >> crashSphere.position.x
           >> crashSphere.position.y
           >> crashSphere.position.z;

    ReadValuePrefix(stream, "rad");
    stream >> crashSphere.radius;

    ReadValuePrefix(stream, "sound");
    stream >> crashSphere.sound;

    ReadValuePrefix(stream, "hard");
    stream >> crashSphere.hardness;

    return crashSphere;
}

ModelShadowSpot ModelInput::ParseShadowSpot(const std::string& text)
{
    ModelShadowSpot shadowSpot;

    std::stringstream stream;
    stream.str(text);

    ReadValuePrefix(stream, "rad");
    stream >> shadowSpot.radius;

    ReadValuePrefix(stream, "int");
    stream >> shadowSpot.intensity;

    return shadowSpot;
}

Math::Sphere ModelInput::ParseCameraCollisionSphere(const std::string& text)
{
    Math::Sphere sphere;

    std::stringstream stream;
    stream.str(text);

    ReadValuePrefix(stream, "pos");
    stream >> sphere.pos.x
           >> sphere.pos.y
           >> sphere.pos.z;

    ReadValuePrefix(stream, "rad");
    stream >> sphere.radius;

    return sphere;
}

ModelTransparentMode ModelInput::ParseTransparentMode(const std::string& text)
{
    if (text == "none")
        return ModelTransparentMode::None;
    else if (text == "alpha")
        return ModelTransparentMode::AlphaChannel;
    else if (text == "map_black")
        return ModelTransparentMode::MapBlackToAlpha;
    else if (text == "map_white")
        return ModelTransparentMode::MapWhiteToAlpha;
    else
        throw CModelIOException(std::string("Unexpected transparent mode: '") + text + "'");
}

ModelSpecialMark ModelInput::ParseSpecialMark(const std::string& text)
{
    if (text == "none")
        return ModelSpecialMark::None;
    else if (text == "part1")
        return ModelSpecialMark::Part1;
    else if (text == "part2")
        return ModelSpecialMark::Part2;
    else if (text == "part3")
        return ModelSpecialMark::Part3;
    else
        throw CModelIOException(std::string("Unexpected special mark: '") + text + "'");
}

} // namespace Gfx
