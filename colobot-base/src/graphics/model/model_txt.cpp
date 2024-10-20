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
#include "graphics/model/model_txt.h"

#include "common/ioutils.h"
#include "common/stringutils.h"
#include "common/resources/inputstream.h"

#include "graphics/model/model_io_exception.h"
#include "graphics/model/model_io_structs.h"

using namespace IOUtils;

namespace Gfx::ModelIO
{

void ReadTextModelV1AndV2(CModel& model, std::istream& stream);
void ReadTextModelV3(CModel& model, std::istream& stream);
ModelHeaderV3 ReadTextHeader(std::istream& stream);
std::unique_ptr<CModelMesh> ReadTextMesh(std::istream& stream);

std::string ReadLineString(std::istream& stream, const std::string& expectedPrefix);
void ReadValuePrefix(std::istream& stream, const std::string& expectedPrefix);
Vertex3D ParseVertexTex2(const std::string& text);
LegacyMaterial ParseMaterial(const std::string& text);
glm::vec3 ParseVector(const std::string& text);
ModelCrashSphere ParseCrashSphere(const std::string& text);
ModelShadowSpot ParseShadowSpot(const std::string& text);
Math::Sphere ParseCameraCollisionSphere(const std::string& text);
AlphaMode ParseTransparentMode(const std::string& text);
std::string ParseSpecialMark(const std::string& text);

void ConvertFromOldRenderState(ModelTriangle& triangle, int state);

std::unique_ptr<CModel> ReadTextModel(const std::filesystem::path& path)
{
    CInputStream stream(path);

    int version = 0;
    try
    {
        version = std::stoi(ReadLineString(stream, "version"));
        stream.seekg(std::ios_base::beg);
    }
    catch (const std::exception& e)
    {
        throw CModelIOException(std::string("Error reading version number: ") + e.what());
    }

    auto model = std::make_unique<CModel>();

    if (version == 1 || version == 2)
        ReadTextModelV1AndV2(*model, stream);
    else if (version == 3)
        ReadTextModelV3(*model, stream);
    else
        throw CModelIOException(std::string("Unexpected version number: ") + std::to_string(version));

    return model;
}

void ReadTextModelV1AndV2(CModel& model, std::istream& stream)
{
    ModelHeaderV1AndV2 header;

    try
    {
        header.version = std::stoi(ReadLineString(stream, "version"));
        header.totalTriangles = std::stoi(ReadLineString(stream, "total_triangles"));
    }
    catch (const std::exception& e)
    {
        throw CModelIOException(std::string("Error reading model header: ") + e.what());
    }

    auto mesh = std::make_unique<CModelMesh>();

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
        auto material = ParseMaterial(matText);

        auto diffuse = Gfx::ColorToIntColor(material.diffuse);
        glm::u8vec4 color = { diffuse.r, diffuse.g, diffuse.b, 255 };

        t.p1.color = color;
        t.p2.color = color;
        t.p3.color = color;

        t.tex1Name = ReadLineString(stream, "tex1");
        t.tex2Name = ReadLineString(stream, "tex2");
        std::string varTex2Ch = ReadLineString(stream, "var_tex2");
        t.variableTex2 = varTex2Ch == "Y";

        if (header.version == 1)
            t.lodLevel = static_cast<ModelLODLevel>(std::stoi(ReadLineString(stream, "lod_level")));

        t.state = std::stoi(ReadLineString(stream, "state"));

        if (t.lodLevel == ModelLODLevel::Low ||
            t.lodLevel == ModelLODLevel::Medium)
            continue;

        ModelTriangle triangle;
        triangle.p1 = t.p1;
        triangle.p2 = t.p2;
        triangle.p3 = t.p3;
        triangle.material.albedoTexture = StrUtils::ToPath(t.tex1Name);
        triangle.material.detailTexture = "textures" / StrUtils::ToPath(t.tex2Name);
        triangle.material.variableDetail = t.variableTex2;
        ConvertFromOldRenderState(triangle, t.state);

        mesh->AddTriangle(triangle);
    }

    model.AddMesh("main", std::move(mesh));
}

void ReadTextModelV3(CModel& model, std::istream& stream)
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
        auto mesh = ReadTextMesh(stream);
        model.AddMesh(meshName, std::move(mesh));
    }
}

ModelHeaderV3 ReadTextHeader(std::istream& stream)
{
    ModelHeaderV3 header;
    header.version = std::stoi(ReadLineString(stream, "version"));
    header.totalCrashSpheres = std::stoi(ReadLineString(stream, "total_crash_spheres"));
    header.hasShadowSpot = ReadLineString(stream, "has_shadow_spot") == std::string("Y");
    header.hasCameraCollisionSphere = ReadLineString(stream, "has_camera_collision_sphere") == std::string("Y");
    header.totalMeshes = std::stoi(ReadLineString(stream, "total_meshes"));
    return header;
}

std::unique_ptr<CModelMesh> ReadTextMesh(std::istream& stream)
{
    auto mesh = std::make_unique<CModelMesh>();

    mesh->SetPosition(ParseVector(ReadLineString(stream, "position")));
    mesh->SetRotation(ParseVector(ReadLineString(stream, "rotation")));
    mesh->SetScale(ParseVector(ReadLineString(stream, "scale")));
    mesh->SetParent(ReadLineString(stream, "parent"));

    int totalTriangles = std::stoi(ReadLineString(stream, "total_triangles"));

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
        LegacyMaterial mat = ParseMaterial(matText);

        auto diffuse = Gfx::ColorToIntColor(mat.diffuse);
        glm::u8vec4 color = { diffuse.r, diffuse.g, diffuse.b, 255 };

        t.p1.color = color;
        t.p2.color = color;
        t.p3.color = color;

        t.material.albedoTexture = StrUtils::ToPath(ReadLineString(stream, "tex1"));
        t.material.detailTexture = StrUtils::ToPath(ReadLineString(stream, "tex2"));
        t.material.variableDetail = ReadLineString(stream, "var_tex2") == std::string("Y");

        t.material.alphaMode = ParseTransparentMode(ReadLineString(stream, "trans_mode"));
        t.material.tag = ParseSpecialMark(ReadLineString(stream, "mark"));
        bool doubleSided = ReadLineString(stream, "dbl_side") == std::string("Y");
        t.material.cullFace = doubleSided ? CullFace::NONE : CullFace::BACK;

        if (t.material.alphaMode != AlphaMode::NONE)
            t.material.alphaThreshold = 0.5f;

        mesh->AddTriangle(t);
    }

    return mesh;
}




std::string ReadLineString(std::istream& stream, const std::string& expectedPrefix)
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

void ReadValuePrefix(std::istream& stream, const std::string& expectedPrefix)
{
    std::string prefix;
    stream >> prefix;
    if (prefix != expectedPrefix)
        throw CModelIOException(std::string("Unexpected prefix: '") + prefix + "', expected was: '" + expectedPrefix + "'");
}

Vertex3D ParseVertexTex2(const std::string& text)
{
    Vertex3D vertex;

    std::stringstream stream;
    stream.str(text);

    ReadValuePrefix(stream, "c");
    stream >> vertex.position.x >> vertex.position.y >> vertex.position.z;

    ReadValuePrefix(stream, "n");
    stream >> vertex.normal.x >> vertex.normal.y >> vertex.normal.z;

    ReadValuePrefix(stream, "t1");
    stream >> vertex.uv.x >> vertex.uv.y;

    ReadValuePrefix(stream, "t2");
    stream >> vertex.uv2.x >> vertex.uv2.y;

    return vertex;
}

LegacyMaterial ParseMaterial(const std::string& text)
{
    LegacyMaterial material;

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

glm::vec3 ParseVector(const std::string& text)
{
    glm::vec3 vector = { 0, 0, 0 };

    std::stringstream stream;
    stream.str(text);

    stream >> vector.x >> vector.y >> vector.z;

    return vector;
}

ModelCrashSphere ParseCrashSphere(const std::string& text)
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

ModelShadowSpot ParseShadowSpot(const std::string& text)
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

Math::Sphere ParseCameraCollisionSphere(const std::string& text)
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

AlphaMode ParseTransparentMode(const std::string& text)
{
    if (text == "none")
        return AlphaMode::NONE;
    else if (text == "alpha")
        return AlphaMode::MASK;
    else
        return AlphaMode::NONE;
}

std::string ParseSpecialMark(const std::string& text)
{
    if (text == "none")
        return "";
    else if (text == "part1")
        return "tracker_right";
    else if (text == "part2")
        return "tracker_left";
    else if (text == "part3")
        return "energy";
    else
        throw CModelIOException(std::string("Unexpected special mark: '") + text + "'");
}

}
