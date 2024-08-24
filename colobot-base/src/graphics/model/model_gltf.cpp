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
#include "graphics/model/model_gltf.h"

#include "common/ioutils.h"
#include "common/logger.h"
#include "common/resources/inputstream.h"

#include "graphics/model/model_io_exception.h"
#include "graphics/model/model_io_structs.h"

#include <vector>
#include <nlohmann/json.hpp>

using namespace IOUtils;

using json = nlohmann::json;

namespace Gfx::ModelIO
{

struct BufferView
{
    int buffer = 0;
    size_t offset = 0;
    size_t length = 0;
};

struct Accessor
{
    int bufferView = 0;
    size_t byteOffset = 0;
    int componentType = 0;
    int count = 0;
    std::string type = "";
};

struct Texture
{
    int sampler = -1;
    int source = 0;
};

struct Image
{
    std::string uri;
};

struct Sampler
{

};

class GLTFLoader
{
public:
    std::unique_ptr<CModel> Load(const std::filesystem::path& path);

private:
    void ReadBuffers();
    void ReadBufferViews();
    void ReadMaterials();
    void ReadAccessors();
    void ReadSamplers();
    void ReadImages();
    void ReadTextures();
    void ReadMeshes();

    std::vector<glm::vec3> ReadPositions(int index);
    std::vector<glm::vec3> ReadNormals(int index);
    std::vector<glm::vec2> ReadUVs(int index);
    std::vector<glm::u8vec4> ReadColors(int index);
    std::vector<unsigned> ReadIndices(int index);

    std::unique_ptr<CModel> m_model;

    std::filesystem::path m_directory;
    json m_root;

    std::vector<std::vector<char>> m_buffers;
    std::vector<BufferView> m_bufferViews;
    std::vector<Material> m_materials;
    std::vector<Accessor> m_accessors;
    std::vector<Texture> m_textures;
    std::vector<Image> m_images;
    std::vector<Sampler> m_samplers;
};

std::unique_ptr<CModel> ReadGLTFModel(const std::filesystem::path& path)
{
    GLTFLoader loader;

    return loader.Load(path);
}

std::unique_ptr<CModel> GLTFLoader::Load(const std::filesystem::path& path)
{
    m_directory = path.parent_path();

    CInputStream stream(path);

    stream >> m_root;

    m_model = std::make_unique<CModel>();

    ReadBuffers();
    ReadBufferViews();
    ReadAccessors();
    ReadSamplers();
    ReadImages();
    ReadTextures();
    ReadMaterials();
    ReadMeshes();

    return std::move(m_model);
}

void GLTFLoader::ReadBuffers()
{
    m_buffers.clear();

    for (const auto& node : m_root["buffers"])
    {
        size_t length = node["byteLength"].get<int>();

        std::vector<char> buffer(length);

        if (node.contains("uri"))
        {
            auto uri = m_directory / node["uri"].get<std::string>();

            CInputStream stream(uri);

            stream.read(buffer.data(), buffer.size());
        }
        else
        {
            GetLogger()->Error("Base64 not yet supported");
        }

        m_buffers.emplace_back(std::move(buffer));
    }
}

void GLTFLoader::ReadBufferViews()
{
    m_bufferViews.clear();

    for (const auto& node : m_root["bufferViews"])
    {
        BufferView bufferView{};

        bufferView.buffer = node["buffer"].get<int>();
        bufferView.offset = node["byteOffset"].get<int>();
        bufferView.length = node["byteLength"].get<int>();

        m_bufferViews.push_back(bufferView);
    }
}

void GLTFLoader::ReadMaterials()
{
    m_materials.clear();

    for (const auto& material : m_root["materials"])
    {
        Material mat;

        if (material.contains("doubleSided"))
        {
            mat.cullFace = material["doubleSided"].get<bool>() ? CullFace::NONE : CullFace::BACK;
        }

        if (material.contains("extras"))
        {
            const auto& extras = material["extras"];

            if (extras.contains("dirt"))
            {
                int dirt = extras["dirt"].get<int>();

                std::string texName = std::string("dirty0") + char('0' + dirt) + ".png";

                mat.detailTexture = texName;
            }

            if (extras.contains("tag"))
            {
                mat.tag = extras["tag"].get<std::string>();
            }

            if (extras.contains("recolor"))
            {
                mat.recolor = extras["recolor"].get<std::string>();
            }

            if (extras.contains("recolor_ref"))
            {
                const auto& color = extras["recolor_ref"];

                float r = color[0];
                float g = color[1];
                float b = color[2];

                mat.recolorReference = Color(r, g, b);
            }

            if (extras.contains("recolor_threshold"))
            {
                mat.recolorThreshold = extras["recolor_threshold"].get<float>();
            }
        }

        if (material.contains("emissiveFactor"))
        {
            const auto& color = material["emissiveFactor"];

            mat.emissiveColor = {
                color[0].get<float>(),
                color[1].get<float>(),
                color[2].get<float>(),
                0.0
            };

            mat.emissiveColor = Gfx::ToLinear(mat.emissiveColor);
        }
        else
        {
            mat.emissiveColor = { 0.0, 0.0, 0.0, 0.0 };
        }

        if (material.contains("emissiveTexture"))
        {
            const auto& tex = material["emissiveTexture"];

            if (tex.contains("index"))
            {
                int index = tex["index"].get<int>();

                const auto& texture = m_textures[index];

                const auto& image = m_images[texture.source];

                mat.emissiveTexture = image.uri;
            }
        }

        if (material.contains("pbrMetallicRoughness"))
        {
            const auto& pbr = material["pbrMetallicRoughness"];

            if (pbr.contains("baseColorFactor"))
            {
                const auto& color = pbr["baseColorFactor"];
                
                mat.albedoColor = {
                    color[0].get<float>(),
                    color[1].get<float>(),
                    color[2].get<float>(),
                    color[3].get<float>()
                };

                mat.albedoColor = Gfx::ToLinear(mat.albedoColor);
            }
            else
            {
                mat.albedoColor = { 1.0, 1.0, 1.0, 1.0 };
            }

            if (pbr.contains("baseColorTexture"))
            {
                const auto& tex = pbr["baseColorTexture"];

                if (tex.contains("index"))
                {
                    int index = tex["index"].get<int>();

                    const auto& texture = m_textures[index];

                    const auto& image = m_images[texture.source];

                    mat.albedoTexture = image.uri;
                }
            }

            if (pbr.contains("metallicFactor"))
            {
                mat.metalness = pbr["metallicFactor"].get<float>();
            }
            else
            {
                mat.metalness = 1.0;
            }

            if (pbr.contains("roughnessFactor"))
            {
                mat.roughness = pbr["roughnessFactor"].get<float>();
            }
            else
            {
                mat.roughness = 1.0;
            }

            if (pbr.contains("metallicRoughnessTexture"))
            {
                const auto& tex = pbr["metallicRoughnessTexture"];

                if (tex.contains("index"))
                {
                    int index = tex["index"].get<int>();

                    const auto& texture = m_textures[index];

                    const auto& image = m_images[texture.source];

                    mat.materialTexture = image.uri;
                }
            }

            if (pbr.contains("occlusionTexture"))
            {
                const auto& tex = pbr["occlusionTexture"];

                if (tex.contains("index"))
                {
                    int index = tex["index"].get<int>();

                    const auto& texture = m_textures[index];

                    [[maybe_unused]] const auto& image = m_images[texture.source];

                    if (tex.contains("strength"))
                    {
                        mat.aoStrength = tex["strength"].get<float>();
                    }
                    else
                    {
                        mat.aoStrength = 1.0f;
                    }
                }
            }
        }

        if (material.contains("normalTexture"))
        {
            const auto& tex = material["normalTexture"];

            if (tex.contains("index"))
            {
                int index = tex["index"].get<int>();

                const auto& texture = m_textures[index];

                const auto& image = m_images[texture.source];

                mat.normalTexture = image.uri;
            }
        }

        if (material.contains("alphaMode"))
        {
            auto mode = material["alphaMode"].get<std::string>();

            if (mode == "OPAQUE")
                mat.alphaMode = AlphaMode::NONE;
            else if (mode == "MASK")
                mat.alphaMode = AlphaMode::MASK;
            else if (mode == "BLEND")
                mat.alphaMode = AlphaMode::BLEND;
        }

        if (material.contains("alphaCutoff"))
        {
            mat.alphaThreshold = material["alphaCutoff"].get<float>();
        }

        m_materials.push_back(mat);
    }
}

void GLTFLoader::ReadAccessors()
{
    m_accessors.clear();

    for (const auto& node : m_root["accessors"])
    {
        Accessor accessor{};

        accessor.bufferView = node["bufferView"].get<int>();

        if (node.contains("byteOffset"))
            accessor.byteOffset = node["byteOffset"].get<int>();
        else
            accessor.byteOffset = 0;
        accessor.count = node["count"].get<int>();
        accessor.componentType = node["componentType"].get<int>();
        accessor.type = node["type"].get<std::string>();

        m_accessors.push_back(accessor);
    }
}

void GLTFLoader::ReadSamplers()
{
    m_samplers.clear();

    for ([[maybe_unused]] const auto& node : m_root["samplers"])
    {
        Sampler sampler{};

        m_samplers.push_back(sampler);
    }
}

void GLTFLoader::ReadImages()
{
    m_images.clear();

    for (const auto& node : m_root["images"])
    {
        Image image{};

        if (node.contains("uri"))
        {
            image.uri = node["uri"].get<std::string>();
        }

        m_images.push_back(image);
    }
}

void GLTFLoader::ReadTextures()
{
    m_textures.clear();

    for (const auto& node : m_root["textures"])
    {
        Texture texture{};

        if (node.contains("sampler"))
            texture.sampler = node["sampler"].get<int>();

        texture.source = node["source"].get<int>();

        m_textures.push_back(texture);
    }
}

void GLTFLoader::ReadMeshes()
{
    m_model = std::make_unique<CModel>();

    for (const auto& node : m_root["meshes"])
    {
        auto name = node["name"].get<std::string>();

        auto mesh = std::make_unique<CModelMesh>();

        for (const auto& primitive : node["primitives"])
        {
            const auto& material = m_materials[primitive["material"].get<int>()];

            auto part = mesh->AddPart(material);

            if (primitive.contains("attributes"))
            {
                const auto& attributes = primitive["attributes"];

                auto positions = ReadPositions(attributes["POSITION"].get<int>());

                if (positions.empty()) continue;

                part->SetVertices(positions.size());

                for (size_t i = 0; i < positions.size(); i++)
                    part->GetVertex(i).SetPosition(positions[i] * glm::vec3(1.0f, 1.0f, -1.0f));

                if (attributes.contains("NORMAL"))
                {
                    part->Add(VertexAttribute::NORMAL);

                    auto normals = ReadNormals(attributes["NORMAL"].get<int>());

                    for (size_t i = 0; i < normals.size(); i++)
                        part->GetVertex(i).SetNormal(normals[i] * glm::vec3(1.0f, 1.0f, -1.0f));
                }

                if (attributes.contains("TEXCOORD_0"))
                {
                    part->Add(VertexAttribute::UV1);

                    auto uvs = ReadUVs(attributes["TEXCOORD_0"].get<int>());

                    for (size_t i = 0; i < uvs.size(); i++)
                        part->GetVertex(i).SetUV1(uvs[i]);
                }

                if (attributes.contains("TEXCOORD_1"))
                {
                    part->Add(VertexAttribute::UV2);

                    auto uvs = ReadUVs(attributes["TEXCOORD_1"].get<int>());

                    for (size_t i = 0; i < uvs.size(); i++)
                        part->GetVertex(i).SetUV2(uvs[i]);
                }

                if (attributes.contains("COLOR_0"))
                {
                    part->Add(VertexAttribute::COLOR);

                    auto colors = ReadColors(attributes["COLOR_0"].get<int>());

                    for (size_t i = 0; i < colors.size(); i++)
                        part->GetVertex(i).SetColor(colors[i]);
                }
            }

            if (primitive.contains("indices"))
            {
                auto indices = ReadIndices(primitive["indices"].get<int>());

                part->SetIndices(indices.size());

                for (unsigned i = 0; i < indices.size(); i += 3)
                {
                    part->SetIndex(i + 0, indices[i + 0]);
                    part->SetIndex(i + 1, indices[i + 2]);
                    part->SetIndex(i + 2, indices[i + 1]);
                }
            }
            else
            {
                size_t vertices = part->GetVertexCount();

                part->SetIndices(vertices);

                for (unsigned i = 0; i < vertices; i += 3)
                {
                    part->SetIndex(i + 0, i + 0);
                    part->SetIndex(i + 1, i + 2);
                    part->SetIndex(i + 2, i + 1);
                }
            }
        }

        if (mesh->GetPartCount() > 0)
            m_model->AddMesh(name, std::move(mesh));
    }
}

std::vector<glm::vec3> GLTFLoader::ReadPositions(int index)
{
    const auto& accessor = m_accessors[index];

    std::vector<glm::vec3> positions(accessor.count);

    const auto& bufferView = m_bufferViews[accessor.bufferView];

    const auto& buffer = m_buffers[bufferView.buffer];

    auto data = reinterpret_cast<const glm::vec3*>(buffer.data() + bufferView.offset);

    for (int i = 0; i < accessor.count; i++)
        positions[i] = data[i];

    return positions;
}

std::vector<glm::vec3> GLTFLoader::ReadNormals(int index)
{
    const auto& accessor = m_accessors[index];

    std::vector<glm::vec3> normals(accessor.count);

    const auto& bufferView = m_bufferViews[accessor.bufferView];

    const auto& buffer = m_buffers[bufferView.buffer];

    auto data = reinterpret_cast<const glm::vec3*>(buffer.data() + bufferView.offset);

    for (int i = 0; i < accessor.count; i++)
        normals[i] = data[i];

    return normals;
}

std::vector<glm::vec2> GLTFLoader::ReadUVs(int index)
{
    const auto& accessor = m_accessors[index];

    std::vector<glm::vec2> uvs(accessor.count);

    const auto& bufferView = m_bufferViews[accessor.bufferView];

    const auto& buffer = m_buffers[bufferView.buffer];

    if (accessor.componentType == 5126)
    {
        auto data = reinterpret_cast<const glm::vec2*>(buffer.data() + bufferView.offset);

        for (int i = 0; i < accessor.count; i++)
            uvs[i] = data[i];
    }
    else
    {
        GetLogger()->Error("Invalid UV type: %%", accessor.componentType);
    }

    return uvs;
}

std::vector<glm::u8vec4> GLTFLoader::ReadColors(int index)
{
    const auto& accessor = m_accessors[index];

    std::vector<glm::u8vec4> colors(accessor.count);

    const auto& bufferView = m_bufferViews[accessor.bufferView];

    const auto& buffer = m_buffers[bufferView.buffer];

    if (accessor.componentType == 5121)
    {
        auto data = reinterpret_cast<const glm::u8vec4*>(buffer.data() + bufferView.offset);

        for (int i = 0; i < accessor.count; i++)
            colors[i] = data[i];
    }
    else if (accessor.componentType == 5123)
    {
        auto data = reinterpret_cast<const glm::u16vec4*>(buffer.data() + bufferView.offset);

        for (int i = 0; i < accessor.count; i++)
            colors[i] = glm::u8vec4(data[i] / glm::u16vec4(256));
    }
    else if (accessor.componentType == 5126)
    {
        auto data = reinterpret_cast<const glm::vec4*>(buffer.data() + bufferView.offset);

        for (int i = 0; i < accessor.count; i++)
            colors[i] = glm::u8vec4(data[i] * 255.0f);
    }
    else
    {
        GetLogger()->Error("Invalid color type: %%", accessor.componentType);
    }

    // Fix for bug in Blender where it exports vertex colors in sRGB instead of linear space
    for (size_t i = 0; i < colors.size(); i++)
    {
        auto color = Gfx::IntColorToColor(Gfx::IntColor(colors[i]));

        color = Gfx::ToLinear(color);

        colors[i] = Gfx::ColorToIntColor(color);
    }

    return colors;
}

std::vector<unsigned> GLTFLoader::ReadIndices(int index)
{
    const auto& accessor = m_accessors[index];

    std::vector<unsigned> indices(accessor.count);

    const auto& bufferView = m_bufferViews[accessor.bufferView];

    const auto& buffer = m_buffers[bufferView.buffer];

    // Unsigned byte components
    if (accessor.componentType == 5121)
    {
        auto data = reinterpret_cast<const uint8_t*>(buffer.data() + bufferView.offset);

        for (int i = 0; i < accessor.count; i++)
            indices[i] = data[i];
    }
    // Unsigned short components
    else if (accessor.componentType == 5123)
    {
        auto data = reinterpret_cast<const uint16_t*>(buffer.data() + bufferView.offset);

        for (int i = 0; i < accessor.count; i++)
            indices[i] = data[i];
    }
    // Unsigned int components
    else if (accessor.componentType == 5125)
    {
        auto data = reinterpret_cast<const uint32_t*>(buffer.data() + bufferView.offset);

        for (int i = 0; i < accessor.count; i++)
            indices[i] = data[i];
    }

    return indices;
}

}
