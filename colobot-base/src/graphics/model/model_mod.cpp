/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2022, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "graphics/model/model_mod.h"

#include "common/ioutils.h"
#include "common/stringutils.h"
#include "common/resources/inputstream.h"

#include "graphics/model/model_io_exception.h"
#include "graphics/model/model_io_structs.h"

#include <array>
#include <iostream>

using namespace IOUtils;

namespace Gfx::ModelIO
{

std::vector<ModelTriangle> ReadOldModelV1(std::istream& stream, int totalTriangles);
std::vector<ModelTriangle> ReadOldModelV2(std::istream& stream, int totalTriangles);
std::vector<ModelTriangle> ReadOldModelV3(std::istream& stream, int totalTriangles);

Vertex3D ReadBinaryVertex(std::istream& stream);
Vertex3D ReadBinaryVertexTex2(std::istream& stream);
LegacyMaterial ReadBinaryMaterial(std::istream& stream);

void ConvertOldTex1Name(ModelTriangle& triangle, const char* tex1Name);
void ConvertFromOldRenderState(ModelTriangle& triangle, int state);
ModelLODLevel MinMaxToLodLevel(float min, float max);

std::unique_ptr<CModel> ReadOldModel(const std::filesystem::path& path)
{
    CInputStream stream(path);

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

    auto mesh = std::make_unique<CModelMesh>();

    for (const auto& triangle : triangles)
        mesh->AddTriangle(triangle);

    auto model = std::make_unique<CModel>();

    model->AddMesh("main", std::move(mesh));

    return model;
}

std::vector<ModelTriangle> ReadOldModelV1(std::istream& stream, int totalTriangles)
{
    std::vector<ModelTriangle> triangles;

    for (int i = 0; i < totalTriangles; ++i)
    {
        OldModelTriangleV1 t;
        t.used = ReadBinary<1, char>(stream);
        t.selected = ReadBinary<1, char>(stream);

        /* padding */ ReadBinary<2, unsigned int>(stream);

        t.p1 = ReadBinaryVertex(stream);
        t.p2 = ReadBinaryVertex(stream);
        t.p3 = ReadBinaryVertex(stream);

        auto material = ReadBinaryMaterial(stream);
        stream.read(t.texName, 20);
        t.min = ReadBinaryFloat(stream);
        t.max = ReadBinaryFloat(stream);

        ModelLODLevel lodLevel = MinMaxToLodLevel(t.min, t.max);
        if (lodLevel == ModelLODLevel::Low ||
            lodLevel == ModelLODLevel::Medium)
            continue;

        ModelTriangle triangle;
        triangle.p1 = t.p1;
        triangle.p2 = t.p2;
        triangle.p3 = t.p3;

        auto diffuse = Gfx::ColorToIntColor(material.diffuse);
        glm::u8vec4 color = { diffuse.r, diffuse.g, diffuse.b, 255 };

        triangle.p1.color = color;
        triangle.p2.color = color;
        triangle.p3.color = color;

        ConvertOldTex1Name(triangle, t.texName);

        triangles.push_back(triangle);
    }

    return triangles;
}

std::vector<ModelTriangle> ReadOldModelV2(std::istream& stream, int totalTriangles)
{
    std::vector<ModelTriangle> triangles;

    for (int i = 0; i < totalTriangles; ++i)
    {
        OldModelTriangleV2 t;
        t.used = ReadBinary<1, char>(stream);
        t.selected = ReadBinary<1, char>(stream);

        /* padding */ ReadBinary<2, unsigned int>(stream);

        t.p1 = ReadBinaryVertex(stream);
        t.p2 = ReadBinaryVertex(stream);
        t.p3 = ReadBinaryVertex(stream);

        auto material = ReadBinaryMaterial(stream);
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
        triangle.p1 = t.p1;
        triangle.p2 = t.p2;
        triangle.p3 = t.p3;

        auto diffuse = Gfx::ColorToIntColor(material.diffuse);
        glm::u8vec4 color = { diffuse.r, diffuse.g, diffuse.b, 255 };

        triangle.p1.color = color;
        triangle.p2.color = color;
        triangle.p3.color = color;

        ConvertOldTex1Name(triangle, t.texName);

        ConvertFromOldRenderState(triangle, t.state);

        triangles.push_back(triangle);
    }

    return triangles;
}

std::vector<ModelTriangle> ReadOldModelV3(std::istream& stream, int totalTriangles)
{
    std::vector<ModelTriangle> triangles;

    for (int i = 0; i < totalTriangles; ++i)
    {
        OldModelTriangleV3 t;
        t.used = ReadBinary<1, char>(stream);
        t.selected = ReadBinary<1, char>(stream);

        /* padding */ ReadBinary<2, unsigned int>(stream);

        t.p1 = ReadBinaryVertexTex2(stream);
        t.p2 = ReadBinaryVertexTex2(stream);
        t.p3 = ReadBinaryVertexTex2(stream);

        auto material = ReadBinaryMaterial(stream);
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

        auto diffuse = Gfx::ColorToIntColor(material.diffuse);
        glm::u8vec4 color = { diffuse.r, diffuse.g, diffuse.b, 255 };

        triangle.p1.color = color;
        triangle.p2.color = color;
        triangle.p3.color = color;

        ConvertOldTex1Name(triangle, t.texName);

        ConvertFromOldRenderState(triangle, t.state);
        triangle.material.variableDetail = t.texNum2 == 1;

        if (!triangle.material.variableDetail && t.texNum2 != 0)
        {
            std::stringstream ss;
            ss << "dirty" << std::setw(2) << std::setfill('0') << t.texNum2 << ".png";
            triangle.material.detailTexture = "textures" / StrUtils::ToPath(ss.str());
        }

        triangles.push_back(triangle);
    }

    return triangles;
}

ModelLODLevel MinMaxToLodLevel(float min, float max)
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

void ConvertOldTex1Name(ModelTriangle& triangle, const char* tex1Name)
{
    triangle.material.albedoTexture = TempToPath(tex1Name);
    if (triangle.material.albedoTexture.extension() == "bmp" || triangle.material.albedoTexture.extension() == "tga")
    {
        triangle.material.albedoTexture.replace_extension("png");
    }
}

void ConvertFromOldRenderState(ModelTriangle& triangle, int state)
{
    if (triangle.material.albedoTexture == "plant.png" || (state & static_cast<int>(ModelRenderState::Alpha)) != 0)
    {
        triangle.material.alphaMode = AlphaMode::MASK;
        triangle.material.alphaThreshold = 0.5f;
    }
    else
        triangle.material.alphaMode = AlphaMode::NONE;

    if ((state & static_cast<int>(ModelRenderState::Part1)) != 0)
        triangle.material.tag = "tracker_right";
    else if ((state & static_cast<int>(ModelRenderState::Part2)) != 0)
        triangle.material.tag = "tracker_left";
    else if ((state & static_cast<int>(ModelRenderState::Part3)) != 0)
        triangle.material.tag = "energy";
    else
        triangle.material.tag = "";

    bool doubleSided = (state & static_cast<int>(ModelRenderState::TwoFace)) != 0;
    triangle.material.cullFace = doubleSided ? CullFace::NONE : CullFace::BACK;
}

Vertex3D ReadBinaryVertex(std::istream& stream)
{
    Vertex3D vertex;

    vertex.position.x = ReadBinaryFloat(stream);
    vertex.position.y = ReadBinaryFloat(stream);
    vertex.position.z = ReadBinaryFloat(stream);

    vertex.normal.x = ReadBinaryFloat(stream);
    vertex.normal.y = ReadBinaryFloat(stream);
    vertex.normal.z = ReadBinaryFloat(stream);

    vertex.uv.x = ReadBinaryFloat(stream);
    vertex.uv.y = ReadBinaryFloat(stream);

    return vertex;
}

Vertex3D ReadBinaryVertexTex2(std::istream& stream)
{
    Vertex3D vertex;

    vertex.position.x = ReadBinaryFloat(stream);
    vertex.position.y = ReadBinaryFloat(stream);
    vertex.position.z = ReadBinaryFloat(stream);

    vertex.normal.x = ReadBinaryFloat(stream);
    vertex.normal.y = ReadBinaryFloat(stream);
    vertex.normal.z = ReadBinaryFloat(stream);

    vertex.uv.x = ReadBinaryFloat(stream);
    vertex.uv.y = ReadBinaryFloat(stream);

    vertex.uv2.x = ReadBinaryFloat(stream);
    vertex.uv2.y = ReadBinaryFloat(stream);

    return vertex;
}

LegacyMaterial ReadBinaryMaterial(std::istream& stream)
{
    LegacyMaterial material;

    material.diffuse.r = ReadBinaryFloat(stream);
    material.diffuse.g = ReadBinaryFloat(stream);
    material.diffuse.b = ReadBinaryFloat(stream);
    material.diffuse.a = ReadBinaryFloat(stream);

    material.ambient.r = ReadBinaryFloat(stream);
    material.ambient.g = ReadBinaryFloat(stream);
    material.ambient.b = ReadBinaryFloat(stream);
    material.ambient.a = ReadBinaryFloat(stream);

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

}
