/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "graphics/model/model_mesh.h"

#include <array>

namespace Gfx
{

CVertexProxy::CVertexProxy(CModelPart* part, size_t index)
    : m_part(part), m_index(index)
{

}

const glm::vec3& CVertexProxy::GetPosition() const
{
    return m_part->m_positions.array[m_index];
}

void CVertexProxy::SetPosition(const glm::vec3& position) const
{
    m_part->m_positions.array[m_index] = position;
}

const glm::u8vec4& CVertexProxy::GetColor() const
{
    return m_part->m_colors.array[m_index];
}

void CVertexProxy::SetColor(const glm::u8vec4& color) const
{
    m_part->m_colors.array[m_index] = color;
}

const glm::vec2& CVertexProxy::GetUV1() const
{
    return m_part->m_uvs1.array[m_index];
}

void CVertexProxy::SetUV1(const glm::vec2& uv) const
{
    m_part->m_uvs1.array[m_index] = uv;
}

const glm::vec2& CVertexProxy::GetUV2() const
{
    return m_part->m_uvs2.array[m_index];
}

void CVertexProxy::SetUV2(const glm::vec2& uv) const
{
    m_part->m_uvs2.array[m_index] = uv;
}

const glm::vec3& CVertexProxy::GetNormal() const
{
    return m_part->m_normals.array[m_index];
}

void CVertexProxy::SetNormal(const glm::vec3& normal) const
{
    m_part->m_normals.array[m_index] = normal;
}

const glm::vec4& CVertexProxy::GetTangent() const
{
    return m_part->m_tangents.array[m_index];
}

void CVertexProxy::SetTangent(const glm::vec4& tangent) const
{
    m_part->m_tangents.array[m_index] = tangent;
}

const glm::u8vec4& CVertexProxy::GetBoneIndices() const
{
    return m_part->m_boneIndices.array[m_index];
}

void CVertexProxy::SetBoneIndices(const glm::u8vec4& indices) const
{
    m_part->m_boneIndices.array[m_index] = indices;
}

const glm::vec4& CVertexProxy::GetBoneWeights() const
{
    return m_part->m_boneWeights.array[m_index];
}

void CVertexProxy::SetBoneWeights(const glm::vec4& weights) const
{
    m_part->m_boneWeights.array[m_index] = weights;
}


CModelPart::CModelPart(const Material& material, size_t vertices, size_t indices)
    : m_material(material) {
    m_positions.array.resize(vertices);
    m_indices.array.resize(indices);
}

const Material& CModelPart::GetMaterial() const
{
    return m_material;
}

void CModelPart::SetVertices(size_t count)
{
    m_positions.array.resize(count);

    if (m_colors.enabled) m_colors.array.resize(count);
    if (m_uvs1.enabled) m_uvs1.array.resize(count);
    if (m_uvs2.enabled) m_uvs2.array.resize(count);
    if (m_normals.enabled) m_normals.array.resize(count);
    if (m_tangents.enabled) m_tangents.array.resize(count);
    if (m_boneIndices.enabled) m_boneIndices.array.resize(count);
    if (m_boneWeights.enabled) m_boneWeights.array.resize(count);
}

void CModelPart::SetIndices(size_t count)
{
    m_indices.enabled = count > 0;
    m_indices.array.resize(count);
}

bool CModelPart::Has(VertexAttribute array) const
{
    switch (array)
    {
    case VertexAttribute::COLOR:
        return m_colors.enabled;
    case VertexAttribute::UV1:
        return m_uvs1.enabled;
    case VertexAttribute::UV2:
        return m_uvs2.enabled;
    case VertexAttribute::NORMAL:
        return m_normals.enabled;
    case VertexAttribute::TANGENT:
        return m_tangents.enabled;
    case VertexAttribute::BONE_INDICES:
        return m_boneIndices.enabled;
    case VertexAttribute::BONE_WEIGHTS:
        return m_boneWeights.enabled;
    default:
        return false;
    }
}

void CModelPart::Add(VertexAttribute attribute)
{
    switch (attribute)
    {
    case VertexAttribute::COLOR:
        m_colors.enabled = true;
        m_colors.array.resize(m_positions.array.size());
        break;
    case VertexAttribute::UV1:
        m_uvs1.enabled = true;
        m_uvs1.array.resize(m_positions.array.size());
        break;
    case VertexAttribute::UV2:
        m_uvs2.enabled = true;
        m_uvs2.array.resize(m_positions.array.size());
        break;
    case VertexAttribute::NORMAL:
        m_normals.enabled = true;
        m_normals.array.resize(m_positions.array.size());
        break;
    case VertexAttribute::TANGENT:
        m_tangents.enabled = true;
        m_tangents.array.resize(m_positions.array.size());
        break;
    case VertexAttribute::BONE_INDICES:
        m_boneIndices.enabled = true;
        m_boneIndices.array.resize(m_positions.array.size());
        break;
    case VertexAttribute::BONE_WEIGHTS:
        m_boneWeights.enabled = true;
        m_boneWeights.array.resize(m_positions.array.size());
        return;
    }
}

void CModelPart::Remove(VertexAttribute attribute)
{
    switch (attribute)
    {
    case VertexAttribute::COLOR:
        m_colors.enabled = false;
        m_colors.array.resize(0);
        return;
    case VertexAttribute::UV1:
        m_uvs1.enabled = false;
        m_uvs1.array.resize(0);
        return;
    case VertexAttribute::UV2:
        m_uvs2.enabled = false;
        m_uvs2.array.resize(0);
        return;
    case VertexAttribute::NORMAL:
        m_normals.enabled = false;
        m_normals.array.resize(0);
        return;
    case VertexAttribute::TANGENT:
        m_tangents.enabled = false;
        m_tangents.array.resize(0);
        return;
    case VertexAttribute::BONE_INDICES:
        m_boneIndices.enabled = false;
        m_boneIndices.array.resize(0);
        return;
    case VertexAttribute::BONE_WEIGHTS:
        m_boneWeights.enabled = false;
        m_boneWeights.array.resize(0);
        return;
    }
}

bool CModelPart::IsIndexed() const
{
    return m_indices.enabled;
}

size_t CModelPart::GetVertexCount() const
{
    return m_positions.array.size();
}

size_t CModelPart::GetIndexCount() const
{
    return m_indices.array.size();
}

const std::vector<unsigned int>& CModelPart::GetIndices() const
{
    return m_indices.array;
}

CVertexProxy CModelPart::GetVertex(size_t index)
{
    return CVertexProxy(this, index);
}

std::uint32_t CModelPart::GetIndex(size_t index)
{
    return m_indices.array[index];
}

void CModelPart::AddVertex(const Vertex3D& vertex)
{
    m_positions.array.push_back(vertex.position);

    if (m_colors.enabled) m_colors.array.push_back(vertex.color);
    if (m_uvs1.enabled) m_uvs1.array.push_back(vertex.uv);
    if (m_uvs2.enabled) m_uvs2.array.push_back(vertex.uv2);
    if (m_normals.enabled) m_normals.array.push_back(vertex.normal);
    if (m_tangents.enabled) m_tangents.array.push_back({ 1.0f, 0.0f, 0.0f, 1.0f });
    if (m_boneIndices.enabled) m_boneIndices.array.push_back({ 0, 0, 0, 0 });
    if (m_boneWeights.enabled) m_boneWeights.array.push_back({ 1.0f, 0.0f, 0.0f, 0.0f });
}

void CModelPart::AddIndex(unsigned int index)
{
    if (m_indices.enabled) m_indices.array.push_back(index);
}

void CModelPart::SetIndex(size_t index, unsigned int value)
{
    m_indices.array[index] = value;
}

void CModelPart::GetTriangles(std::vector<Gfx::ModelTriangle>& triangles)
{
    size_t n = IsIndexed()
        ? GetIndexCount()
        : GetVertexCount();

    for (size_t i = 0; i < n - 2; i += 3)
    {
        std::array<Gfx::Vertex3D, 3> verts;

        for (size_t j = 0; j < 3; j++)
        {
            size_t index = IsIndexed()
                ? GetIndex(i + j)
                : i + j;
            auto vertex = GetVertex(index);

            verts[j].position = vertex.GetPosition();

            if (Has(VertexAttribute::COLOR)) verts[j].color = vertex.GetColor();
            else verts[j].color = { 255, 255, 255, 255 };

            if (Has(VertexAttribute::UV1)) verts[j].uv = vertex.GetUV1();
            else verts[j].uv = { 0, 0 };

            if (Has(VertexAttribute::UV2)) verts[j].uv2 = vertex.GetUV2();
            else verts[j].uv2 = { 0, 0 };

            if (Has(VertexAttribute::NORMAL)) verts[j].normal = vertex.GetNormal();
            else verts[j].normal = { 0, 0, 1 };
        }

        triangles.push_back({ verts[0], verts[1], verts[2], GetMaterial() });
    }
}

void CModelMesh::AddTriangle(const ModelTriangle& triangle)
{
    for (auto& part : m_parts)
    {
        if (part->GetMaterial() == triangle.material)
        {
            part->AddVertex(triangle.p1);
            part->AddVertex(triangle.p2);
            part->AddVertex(triangle.p3);
            return;
        }
    }

    auto part = std::make_unique<CModelPart>(triangle.material, 0, 0);

    part->Add(VertexAttribute::COLOR);
    part->Add(VertexAttribute::UV1);
    part->Add(VertexAttribute::UV2);
    part->Add(VertexAttribute::NORMAL);

    part->AddVertex(triangle.p1);
    part->AddVertex(triangle.p2);
    part->AddVertex(triangle.p3);

    m_parts.push_back(std::move(part));
}

void CModelMesh::AddTriangle(const Triangle& triangle, const Material& material)
{
    for (auto& part : m_parts)
    {
        if (part->GetMaterial() == material)
        {
            part->AddVertex(triangle.p1);
            part->AddVertex(triangle.p2);
            part->AddVertex(triangle.p3);
            return;
        }
    }

    auto part = std::make_unique<CModelPart>(material, 0, 0);

    part->Add(VertexAttribute::COLOR);
    part->Add(VertexAttribute::UV1);
    part->Add(VertexAttribute::UV2);
    part->Add(VertexAttribute::NORMAL);

    part->AddVertex(triangle.p1);
    part->AddVertex(triangle.p2);
    part->AddVertex(triangle.p3);

    m_parts.push_back(std::move(part));
}

size_t CModelMesh::GetPartCount() const
{
    return m_parts.size();
}

CModelPart* CModelMesh::GetPart(size_t index) const
{
    return m_parts[index].get();
}

CModelPart* CModelMesh::AddPart(const Material& material)
{
    for (auto& part : m_parts)
    {
        if (part->GetMaterial() == material)
        {
            return part.get();
        }
    }

    auto part = std::make_unique<CModelPart>(material, 0, 0);

    m_parts.push_back(std::move(part));

    return m_parts.back().get();
}

const glm::vec3& CModelMesh::GetPosition() const
{
    return m_position;
}

void CModelMesh::SetPosition(const glm::vec3& position)
{
    m_position = position;
}

const glm::vec3& CModelMesh::GetRotation() const
{
    return m_rotation;
}

void CModelMesh::SetRotation(const glm::vec3& rotation)
{
    m_rotation = rotation;
}

const glm::vec3& CModelMesh::GetScale() const
{
    return m_scale;
}

void CModelMesh::SetScale(const glm::vec3& scale)
{
    m_scale = scale;
}

const std::string& CModelMesh::GetParent() const
{
    return m_parent;
}

void CModelMesh::SetParent(const std::string& parent)
{
    m_parent = parent;
}

std::vector<ModelTriangle> CModelMesh::GetTriangles() const
{
    std::vector<ModelTriangle> triangles;

    for (const auto& part : m_parts)
    {
        part->GetTriangles(triangles);
    }

    return triangles;
}

} // namespace Gfx
