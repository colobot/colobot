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

 /**
  * \file graphics/core/renderers.h
  * \brief Abstract classes representing renderers
  */

#pragma once

#include "graphics/core/vertex.h"

#include <glm/glm.hpp>

// Graphics module namespace
namespace Gfx
{

class CVertexBuffer;
enum PrimitiveType;
struct Texture;

/**
 * \class CRenderer
 * \brief Common abstract interface for renderers
 */
class CRenderer
{
public:
    virtual ~CRenderer() { }

    //! Flush buffered content
    virtual void Flush() = 0;
};

/**
 * \class CRenderer
 * \brief Abstract interface for UI renderers
 */
class CUIRenderer : public CRenderer
{
public:
    virtual ~CUIRenderer() { }

    //! Sets ortographic projection with given parameters
    virtual void SetProjection(float left, float right, float bottom, float top) = 0;
    //! Sets texture, setting texture 0 means using white texture
    virtual void SetTexture(const Texture& texture) = 0;
    //! Sets color
    virtual void SetColor(const glm::vec4& color) = 0;

    //! Draws primitive
    virtual void DrawPrimitive(PrimitiveType type, int count, const Vertex2D* vertices) = 0;
};

class CTerrainRenderer : public CRenderer
{
public:
    virtual ~CTerrainRenderer() { }

    virtual void Begin() = 0;

    virtual void End() = 0;

    //! Sets projection matrix
    virtual void SetProjectionMatrix(const glm::mat4& matrix) = 0;
    //! Sets view matrix
    virtual void SetViewMatrix(const glm::mat4& matrix) = 0;
    //! Sets model matrix
    virtual void SetModelMatrix(const glm::mat4& matrix) = 0;
    //! Sets shadow matrix
    virtual void SetShadowMatrix(const glm::mat4& matrix) = 0;

    //! Sets primary texture, setting texture 0 means using white texture
    virtual void SetPrimaryTexture(const Texture& texture) = 0;
    //! Sets secondary texture
    virtual void SetSecondaryTexture(const Texture& texture) = 0;
    //! Sets shadow map
    virtual void SetShadowMap(const Texture& texture) = 0;

    //! Sets light parameters
    virtual void SetLight(const glm::vec4& position, const float& intensity, const glm::vec3& color) = 0;

    //! Sets fog parameters
    virtual void SetFog(float min, float max, const glm::vec3& color) = 0;

    //! Draws terrain object
    virtual void DrawObject(const glm::mat4& matrix, const CVertexBuffer* buffer) = 0;
};

class CShadowRenderer : public CRenderer
{
public:
    virtual ~CShadowRenderer() { }

    virtual void Begin() = 0;

    virtual void End() = 0;

    //! Sets projection matrix
    virtual void SetProjectionMatrix(const glm::mat4& matrix) = 0;
    //! Sets view matrix
    virtual void SetViewMatrix(const glm::mat4& matrix) = 0;
    //! Sets model matrix
    virtual void SetModelMatrix(const glm::mat4& matrix) = 0;

    //! Sets texture
    virtual void SetTexture(const Texture& texture) = 0;

    //! Draws terrain object
    virtual void DrawObject(const CVertexBuffer* buffer, bool transparent) = 0;
};

} // namespace Gfx
