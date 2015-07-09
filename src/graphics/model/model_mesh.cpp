#include "graphics/model/model_mesh.h"

namespace Gfx {

void CModelMesh::AddTriangle(const ModelTriangle& triangle)
{
    m_triangles.push_back(triangle);
}

void CModelMesh::SetTriangles(std::vector<ModelTriangle>&& triangles)
{
    m_triangles = triangles;
}

const std::vector<ModelTriangle>& CModelMesh::GetTriangles() const
{
    return m_triangles;
}

int CModelMesh::GetTriangleCount() const
{
    return m_triangles.size();
}

const Math::Vector& CModelMesh::GetPosition() const
{
    return m_position;
}

void CModelMesh::SetPosition(const Math::Vector& position)
{
    m_position = position;
}

const Math::Vector& CModelMesh::GetRotation() const
{
    return m_rotation;
}

void CModelMesh::SetRotation(const Math::Vector& rotation)
{
    m_rotation = rotation;
}

const Math::Vector& CModelMesh::GetScale() const
{
    return m_scale;
}

void CModelMesh::SetScale(const Math::Vector& scale)
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

} // namespace Gfx
