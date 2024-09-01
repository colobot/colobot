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

#pragma once

#include "graphics/model/model_triangle.h"

#include <memory>
#include <vector>

namespace Gfx
{

/**
 * \enum VertexAttribute
 * \brief An enum for vertex attributes
 */
enum class VertexAttribute
{
    COLOR,
    UV1,
    UV2,
    NORMAL,
    TANGENT,
    BONE_INDICES,
    BONE_WEIGHTS,
};

class CModelPart;


/**
 * \class CVertexProxy
 * \brief Proxy object for manipulating vertex data
 */
class CVertexProxy
{
    CModelPart* m_part;
    size_t m_index;

public:
    CVertexProxy(CModelPart* part, size_t index);

    //! Returns vertex position
    const glm::vec3& GetPosition() const;
    //! Sets vertex position
    void SetPosition(const glm::vec3& position) const;

    //! Returns vertex color
    const glm::u8vec4& GetColor() const;
    //! Sets vertex color
    void SetColor(const glm::u8vec4& color) const;

    //! Returns 1st UV
    const glm::vec2& GetUV1() const;
    //! Sets 1st UV
    void SetUV1(const glm::vec2& uv) const;

    //! Returns 2nd UV
    const glm::vec2& GetUV2() const;
    //! Sets 2nd UV
    void SetUV2(const glm::vec2& uv) const;

    //! Returns normal
    const glm::vec3& GetNormal() const;
    //! Sets normal
    void SetNormal(const glm::vec3& normal) const;

    //! Returns tangent
    const glm::vec4& GetTangent() const;
    //! Sets tangent
    void SetTangent(const glm::vec4& tangent) const;

    //! Returns bone indices
    const glm::u8vec4& GetBoneIndices() const;
    //! Sets bone indices
    void SetBoneIndices(const glm::u8vec4& indices) const;

    //! Returns bone weights
    const glm::vec4& GetBoneWeights() const;
    //! Sets bone weights
    void SetBoneWeights(const glm::vec4& weights) const;
};

template<typename T>
struct VertexAttributeArray
{
    bool enabled = false;
    std::vector<T> array = {};
};

/**
 * \class CModelPart
 * \brief Part of mesh with a common material
 */
class CModelPart
{
public:
    //! Creates new part for given material
    CModelPart(const Material& material, size_t vertices = 0, size_t indices = 0);

    //! Returns this part's material
    const Material& GetMaterial() const;

    //! Sets the number of vertices
    void SetVertices(size_t count);
    //! Sets the number of indices
    void SetIndices(size_t count);

    //! Returns whether this mesh part contains specific vertex attribute
    bool Has(VertexAttribute attribute) const;
    //! Adds vertex attribute to this mesh part
    void Add(VertexAttribute attribute);
    //! Removes vertex attribute from this mesh part
    void Remove(VertexAttribute attribute);

    //! Returns true if this part is indexed
    bool IsIndexed() const;
    //! Returns the number of vertices in this part
    size_t GetVertexCount() const;
    //! Returns the number of indices in this part
    size_t GetIndexCount() const;
    //! Returns the indices in this part
    const std::vector<unsigned int>& GetIndices() const;

    //! Returns vertex proxy object for manipulating vertex data
    CVertexProxy GetVertex(size_t index);
    //! Returns vertex index
    std::uint32_t GetIndex(size_t index);

    //! Adds a vertex
    void AddVertex(const Vertex3D& vertex);
    //! Adds an index
    void AddIndex(unsigned int index);

    //! Set index
    void SetIndex(size_t index, unsigned int value);

    //! Fills the array with converted model triangles
    void GetTriangles(std::vector<Gfx::ModelTriangle>& triangles);

    friend class CVertexProxy;

private:
    //! Material
    Material m_material;

    //! Positions
    VertexAttributeArray<glm::vec3> m_positions;
    //! Colors
    VertexAttributeArray<glm::u8vec4> m_colors;
    //! UVs 1
    VertexAttributeArray<glm::vec2> m_uvs1;
    //! UVs 2
    VertexAttributeArray<glm::vec2> m_uvs2;
    //! Normals
    VertexAttributeArray<glm::vec3> m_normals;
    //! Tangents
    VertexAttributeArray<glm::vec4> m_tangents;
    //! Bone indices
    VertexAttributeArray<glm::u8vec4> m_boneIndices;
    //! Bone weights
    VertexAttributeArray<glm::vec4> m_boneWeights;

    //! Indices
    VertexAttributeArray<std::uint32_t> m_indices;
};

/**
 * \class CModelMesh
 * \brief Mesh data saved in model file
 */
class CModelMesh
{
public:
    //! Adds a new triangle
    void AddTriangle(const ModelTriangle& triangle);
    //! Adds a new triangle
    void AddTriangle(const Triangle& triangle, const Material& material);

    //! Returns the number of parts
    size_t GetPartCount() const;
    //! Returns a part with given index
    CModelPart* GetPart(size_t index) const;
    //! Adds a new part with given material or returns an existing one
    CModelPart* AddPart(const Material& material);

    //! Returns the mesh position
    const glm::vec3& GetPosition() const;
    //! Sets the mesh rotation
    void SetPosition(const glm::vec3& position);

    //! Returns the mesh rotation
    const glm::vec3& GetRotation() const;
    //! Sets the mesh rotation
    void SetRotation(const glm::vec3& rotation);

    //! Returns the mesh scale
    const glm::vec3& GetScale() const;
    //! Sets the mesh scale
    void SetScale(const glm::vec3& scale);

    //! Returns the name of parent mesh
    const std::string& GetParent() const;
    //! Sets the name of parent mesh
    void SetParent(const std::string& parent);

    //! Returns all model triangles of this mesh
    std::vector<ModelTriangle> GetTriangles() const;

private:
    std::vector<std::unique_ptr<CModelPart>> m_parts;
    glm::vec3 m_position = { 0, 0, 0 };
    glm::vec3 m_rotation = { 0, 0, 0 };
    glm::vec3 m_scale = { 1, 1, 1 };
    std::string m_parent;
};

} // namespace Gfx
