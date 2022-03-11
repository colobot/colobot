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

#include "graphics/model/model_mesh.h"

namespace Gfx
{

CModelPart::CModelPart(const Material& material)
    : m_material(material) {}

const Material& CModelPart::GetMaterial() const
{
    return m_material;
}

bool CModelPart::IsIndexed() const
{
    return !m_indices.empty();
}

size_t CModelPart::GetVertexCount() const
{
    return m_vertices.size();
}

const std::vector<Vertex3D>& CModelPart::GetVertices() const
{
    return m_vertices;
}

size_t CModelPart::GetIndexCount() const
{
    return m_indices.size();
}

const std::vector<unsigned int>& CModelPart::GetIndices() const
{
    return m_indices;
}

void CModelPart::AddVertex(const Vertex3D& vertex)
{
    m_vertices.push_back(vertex);
}

void CModelPart::AddIndex(unsigned int index)
{
    m_indices.push_back(index);
}

void CModelMesh::AddTriangle(const ModelTriangle& triangle)
{
    for (auto& part : m_parts)
    {
        if (part.GetMaterial() == triangle.material)
        {
            part.AddVertex(triangle.p1);
            part.AddVertex(triangle.p2);
            part.AddVertex(triangle.p3);
            return;
        }
    }

    CModelPart part(triangle.material);

    part.AddVertex(triangle.p1);
    part.AddVertex(triangle.p2);
    part.AddVertex(triangle.p3);

    m_parts.emplace_back(part);
}

void CModelMesh::AddTriangle(const Triangle& triangle, const Material& material)
{
    for (auto& part : m_parts)
    {
        if (part.GetMaterial() == material)
        {
            part.AddVertex(triangle.p1);
            part.AddVertex(triangle.p2);
            part.AddVertex(triangle.p3);
            return;
        }
    }

    CModelPart part(material);

    part.AddVertex(triangle.p1);
    part.AddVertex(triangle.p2);
    part.AddVertex(triangle.p3);

    m_parts.emplace_back(part);
}

size_t CModelMesh::GetPartCount() const
{
    return m_parts.size();
}

const CModelPart& CModelMesh::GetPart(size_t index) const
{
    return m_parts[index];
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
        if (part.IsIndexed())
        {
            const auto& vertices = part.GetVertices();
            const auto& indices = part.GetIndices();

            for (size_t i = 0; i < indices.size() - 2; i += 3)
            {
                triangles.push_back({
                    vertices[indices[i]],
                    vertices[indices[i + 1]],
                    vertices[indices[i + 2]],
                    part.GetMaterial()
                });
            }
        }
        else
        {
            const auto& vertices = part.GetVertices();

            for (size_t i = 0; i < vertices.size() - 2; i += 3)
            {
                triangles.push_back({
                    vertices[i],
                    vertices[i + 1],
                    vertices[i + 2],
                    part.GetMaterial()
                });
            }
        }
    }

    return triangles;
}

} // namespace Gfx
