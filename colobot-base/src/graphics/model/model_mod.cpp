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

#include <algorithm>
#include <array>
#include <iostream>
#include <span>

using namespace IOUtils;
using namespace Gfx;

namespace
{

using Polygon = std::vector<glm::vec3>;

struct Clipped
{
    Polygon inside;
    Polygon outside;
};

std::vector<ModelTriangle> ReadOldModelV1(std::istream& stream, int totalTriangles);
std::vector<ModelTriangle> ReadOldModelV2(std::istream& stream, int totalTriangles);
std::vector<ModelTriangle> ReadOldModelV3(std::istream& stream, int totalTriangles);

Vertex3D ReadBinaryVertex(std::istream& stream);
Vertex3D ReadBinaryVertexTex2(std::istream& stream);
LegacyMaterial ReadBinaryMaterial(std::istream& stream);

void ConvertOldTex1Name(ModelTriangle& triangle, const char* tex1Name);
void ConvertFromOldRenderState(ModelTriangle& triangle, int state);
ModelLODLevel MinMaxToLodLevel(float min, float max);

Clipped ClipByValue(std::span<const glm::vec3> polygon, std::span<const float> distances);
std::vector<Triangle> ClipByUVRectangle(const Triangle& triangle, const glm::vec4& region);
std::vector<Triangle> ClipByUVRectangles(const Triangle& triangle, std::span<const glm::vec4> regions);
void AddTriangle(std::vector<ModelTriangle>& triangles, ModelTriangle&& triangle);

}

std::unique_ptr<CModel> Gfx::ModelIO::ReadOldModel(const std::filesystem::path& path)
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

namespace
{

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

        AddTriangle(triangles, std::move(triangle));
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

        AddTriangle(triangles, std::move(triangle));
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

        AddTriangle(triangles, std::move(triangle));
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
    triangle.material.albedoTexture = StrUtils::ToPath(tex1Name);
    if (triangle.material.albedoTexture.extension() == "bmp" || triangle.material.albedoTexture.extension() == "tga")
    {
        triangle.material.albedoTexture.replace_extension("png");
    }
}

void ConvertFromOldRenderState(ModelTriangle& triangle, int state)
{
    if ((state & static_cast<int>(ModelRenderState::Alpha)) != 0)
        triangle.material.alphaMode = AlphaMode::BLEND;
    else
        triangle.material.alphaMode = AlphaMode::NONE;

    if ((state & static_cast<int>(ModelRenderState::Part1)) != 0)
        triangle.material.mark = Mark::RIGHT_TRACKER;
    else if ((state & static_cast<int>(ModelRenderState::Part2)) != 0)
        triangle.material.mark = Mark::LEFT_TRACKER;
    else if ((state & static_cast<int>(ModelRenderState::Part3)) != 0)
        triangle.material.mark = Mark::ENERGY;

    bool doubleSided = (state & static_cast<int>(ModelRenderState::TwoFace)) != 0;
    triangle.material.cullFace = doubleSided ? CullFace::NONE : CullFace::BACK;

    if (triangle.material.albedoTexture == "plant.png")
    {
        triangle.material.baseColor = BaseColor::PLANT;
        triangle.material.alphaMode = AlphaMode::MASK;
        triangle.material.alphaThreshold = 0.5f;
    }
    else if (triangle.material.albedoTexture == "face01.png"
        || triangle.material.albedoTexture == "face02.png"
        || triangle.material.albedoTexture == "face03.png"
        || triangle.material.albedoTexture == "face04.png")
    {
        triangle.material.baseColor = BaseColor::HAIR;
    }
    else if (triangle.material.albedoTexture == "human.png")
    {
        triangle.material.baseColor = BaseColor::SUIT;

        auto middle = (triangle.p1.uv + triangle.p2.uv + triangle.p3.uv) / 3.0f;

        // Make visor transparent
        if (0.0f < middle.x && middle.x < 0.25f
            && 0.0f < middle.y && middle.y < 0.25f)
        {
            triangle.material.alphaMode = AlphaMode::BLEND;
            triangle.material.alphaThreshold = 0.5f;
        }
    }
}


Clipped ClipByValue(std::span<const glm::vec3> polygon, std::span<const float> distances)
{
    // Number of polygon vertices outside
    std::size_t count = std::count_if(distances.begin(), distances.end(),
        [](float distance) { return distance >= 0.0f; });

    if (count == 0) // Entire polygon inside
        return Clipped{ Polygon{ polygon.begin(), polygon.end() }, Polygon{} };
    if (count == polygon.size()) // Entire polygon outside
        return Clipped{ Polygon{}, Polygon{ polygon.begin(), polygon.end() } };

    Clipped clipped;

    for (std::size_t i = 0; i < polygon.size(); i++)
    {
        std::size_t j = (i + 1) % polygon.size();

        const auto& d1 = distances[i];
        const auto& d2 = distances[j];

        if (d1 <= 0.0f) clipped.inside.push_back(polygon[i]);
        if (d1 >= 0.0f) clipped.outside.push_back(polygon[i]);

        if (d1 * d2 < 0.0f)
        {
            float t = d1 / (d1 - d2);

            glm::vec3 inserted = glm::mix(polygon[i], polygon[j], t);

            clipped.inside.push_back(inserted);
            clipped.outside.push_back(inserted);
        }
    }

    return clipped;
}

std::vector<Triangle> ClipByUVRectangle(const Triangle& triangle, const glm::vec4& region)
{
    const auto [triangleMinU, triangleMaxU] = std::minmax({ triangle.p1.uv[0], triangle.p2.uv[0], triangle.p3.uv[0] });
    const auto [triangleMinV, triangleMaxV] = std::minmax({ triangle.p1.uv[1], triangle.p2.uv[1], triangle.p3.uv[1] });

    const auto [minU, maxU] = std::minmax({ region[0], region[2] });
    const auto [minV, maxV] = std::minmax({ region[1], region[3] });

    // Discard trivial case of a triangle UVs being outside the region
    if (triangleMinU > maxU) return {};
    if (triangleMaxU < minU) return {};

    if (triangleMinV > maxV) return {};
    if (triangleMaxV < minV) return {};

    // Clipping values calculated using triangle UVs and region UVs
    std::array<glm::vec4, 3> values;

    for (int i = 0; const auto& uv : { triangle.p1.uv, triangle.p2.uv, triangle.p3.uv })
    {
        values[i++] =
        {
            minU - uv[0],
            uv[0] - maxU,
            minV - uv[1],
            uv[1] - maxV
        };
    }

    // Evaluates a value using barycentric coordinates
    const auto evaluate = [](const glm::vec3& coords, auto&& v1, auto&& v2, auto&& v3)
    {
        return coords[0] * v1 + coords[1] * v2 + coords[2] * v3;
    };

    // Inside polygon
    Polygon inside = { glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0) };

    // Outside polygons
    std::vector<Polygon> outside;

    for (int i = 0; i < 4; i++)
    {
        std::vector<float> polygonValues;

        for (std::size_t j = 0; j < inside.size(); j++)
            polygonValues.push_back(evaluate(inside[j], values[0][i], values[1][i], values[2][i]));

        auto clipped = ClipByValue(inside, polygonValues);

        if (clipped.outside.size() > 0) outside.push_back(std::move(clipped.outside));

        inside = std::move(clipped.inside);

        if (inside.size() == 0) break;
    }

    std::vector<Triangle> triangles;

    // Emits a polygon as triangles
    const auto emit = [&](const Polygon& polygon)
    {
        // Interpolate polygon vertices
        std::vector<Vertex3D> vertices;

        for (const auto& point : polygon)
        {
            Vertex3D vertex;

            vertex.position = evaluate(point, triangle.p1.position, triangle.p2.position, triangle.p3.position);
            vertex.color = evaluate(point, glm::vec4{triangle.p1.color}, glm::vec4{triangle.p2.color}, glm::vec4{triangle.p3.color});
            vertex.uv = evaluate(point, triangle.p1.uv, triangle.p2.uv, triangle.p3.uv);
            vertex.uv2 = evaluate(point, triangle.p1.uv2, triangle.p2.uv2, triangle.p3.uv2);
            vertex.normal = evaluate(point, triangle.p1.normal, triangle.p2.normal, triangle.p3.normal);

            vertices.push_back(std::move(vertex));
        }

        // Emit polygon as an expanded triangle fan
        for (std::size_t i = 1; i + 1 < vertices.size(); i++)
        {
            triangles.push_back(Triangle{ vertices[0], vertices[i], vertices[i + 1] });
        }
    };

    // Emit clipped polygons
    if (inside.size() > 0) emit(inside);

    for (const auto& polygon : outside)
        emit(polygon);

    return triangles;
}

std::vector<Triangle> ClipByUVRectangles(const Triangle& triangle, std::span<const glm::vec4> regions)
{
    for (const auto& region : regions)
    {
        auto clipped = ClipByUVRectangle(triangle, region);

        if (clipped.size() > 0) return clipped;
    }

    return {};
}

// Adds a triangle, potentially as a subdivided set of smaller triangles
void AddTriangle(std::vector<ModelTriangle>& triangles, ModelTriangle&& triangle)
{
    if (triangle.material.albedoTexture == "human.png")
    {
        // UV regions for orange bands on the texture
        constexpr auto regions = std::array{
            glm::vec4{ 96.0f / 256.0f, 6.0f / 256.0f, 128.0f / 256.0f, 12.0f / 256.0f },
            glm::vec4{ 134.0f / 256.0f, 32.0f / 256.0f, 140.0f / 256.0f, 64.0f / 256.0f },
            glm::vec4{ 112.0f / 256.0f, 245.0f / 256.0f, 144.0f / 256.0f, 251.0f / 256.0f }
        };

        auto clipped = ClipByUVRectangles(Triangle{ triangle.p1, triangle.p2, triangle.p3 }, regions);

        if (clipped.empty())
        {
            triangles.push_back(std::move(triangle));
            return;
        }
            
        for (const auto& tr : clipped)
        {
            auto middle = (tr.p1.uv + tr.p2.uv + tr.p3.uv) / 3.0f;

            auto inside = std::any_of(regions.begin(), regions.end(), [&](const glm::vec4& region)
            {
                if (middle[0] < region[0]) return false;
                if (middle[1] < region[1]) return false;
                if (middle[0] > region[2]) return false;
                if (middle[1] > region[3]) return false;
                return true;
            });

            auto material = triangle.material;

            // Apply base material color for bands and suit
            if (inside)
                material.baseColor = BaseColor::BAND;
            else
                material.baseColor = BaseColor::SUIT;

            triangles.push_back(ModelTriangle{ tr.p1, tr.p2, tr.p3, material });
        }
    }
    else
    {
        triangles.push_back(std::move(triangle));
    }
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
