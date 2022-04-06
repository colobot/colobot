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
enum class CullFace : unsigned char;
enum class PrimitiveType : unsigned char;
enum class TransparencyMode : unsigned char;
struct Texture;

struct ShadowParam
{
    glm::mat4 matrix;
    glm::vec2 uv_offset;
    glm::vec2 uv_scale;
};

/**
 * \class CRenderer
 * \brief Common abstract interface for renderers
 */
class CRenderer
{
public:
    virtual ~CRenderer() { }
};

/**
 * \class CUIRenderer
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
    //! Sets transparency mode
    virtual void SetTransparency(TransparencyMode mode) = 0;

    //! Draws primitive
    virtual void DrawPrimitive(PrimitiveType type, int count, const Vertex2D* vertices) = 0;

    virtual Vertex2D* BeginPrimitive(PrimitiveType type, int count) = 0;
    virtual Vertex2D* BeginPrimitives(PrimitiveType type, int drawCount, const int* counts) = 0;
    virtual bool EndPrimitive() = 0;
};

/**
 * \class CTerrainRenderer
 * \brief Abstract interface for terrain renderers
 */
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

    //! Sets albedo color
    virtual void SetAlbedoColor(const Color& color) = 0;
    //! Sets albedo texture
    virtual void SetAlbedoTexture(const Texture& texture) = 0;
    //! Sets emissive color
    virtual void SetEmissiveColor(const Color& color) = 0;
    //! Sets emissive texture
    virtual void SetEmissiveTexture(const Texture& texture) = 0;
    //! Sets material parameters
    virtual void SetMaterialParams(float roughness, float metalness, float aoStrength) = 0;
    //! Sets material texture
    virtual void SetMaterialTexture(const Texture& texture) = 0;

    //! Sets detail texture
    virtual void SetDetailTexture(const Texture& texture) = 0;
    //! Sets shadow map
    virtual void SetShadowMap(const Texture& texture) = 0;

    //! Sets light parameters
    virtual void SetLight(const glm::vec4& position, const float& intensity, const glm::vec3& color) = 0;
    //! Sets sky parameters
    virtual void SetSky(const Color& color, float intensity) = 0;
    //! Sets shadow parameters
    virtual void SetShadowParams(int count, const ShadowParam* params) = 0;

    //! Sets fog parameters
    virtual void SetFog(float min, float max, const glm::vec3& color) = 0;

    //! Draws terrain object
    virtual void DrawObject(const glm::mat4& matrix, const CVertexBuffer* buffer) = 0;
};

/**
 * \class CObjectRenderer
 * \brief Abstract interface for object renderers
 */
class CObjectRenderer : public CRenderer
{
public:
    virtual ~CObjectRenderer() { }

    virtual void Begin() = 0;

    virtual void End() = 0;

    //! Sets projection matrix
    virtual void SetProjectionMatrix(const glm::mat4& matrix) = 0;
    //! Sets view matrix
    virtual void SetViewMatrix(const glm::mat4& matrix) = 0;
    //! Sets model matrix
    virtual void SetModelMatrix(const glm::mat4& matrix) = 0;

    //! Sets albedo color
    virtual void SetAlbedoColor(const Color& color) = 0;
    //! Sets albedo texture
    virtual void SetAlbedoTexture(const Texture& texture) = 0;
    //! Sets emissive color
    virtual void SetEmissiveColor(const Color& color) = 0;
    //! Sets emissive texture
    virtual void SetEmissiveTexture(const Texture& texture) = 0;
    //! Sets material parameters
    virtual void SetMaterialParams(float roughness, float metalness, float aoStrength) = 0;
    //! Sets material texture
    virtual void SetMaterialTexture(const Texture& texture) = 0;

    //! Sets detail texture
    virtual void SetDetailTexture(const Texture& texture) = 0;
    //! Sets shadow map
    virtual void SetShadowMap(const Texture& texture) = 0;

    //! Enables lighting
    virtual void SetLighting(bool enabled) = 0;
    //! Sets light parameters
    virtual void SetLight(const glm::vec4& position, const float& intensity, const glm::vec3& color) = 0;
    //! Sets sky parameters
    virtual void SetSky(const Color& color, float intensity) = 0;
    //! Sets shadow parameters
    virtual void SetShadowParams(int count, const ShadowParam* params) = 0;

    //! Sets fog parameters
    virtual void SetFog(float min, float max, const glm::vec3& color) = 0;
    //! Sets alpha scissor
    virtual void SetAlphaScissor(float alpha) = 0;

    //! Sets depth test
    virtual void SetDepthTest(bool enabled) = 0;
    //! Sets depth mask
    virtual void SetDepthMask(bool enabled) = 0;
    //! Sets cull face mode
    virtual void SetCullFace(CullFace mode) = 0;
    //! Sets transparency mode
    virtual void SetTransparency(TransparencyMode mode) = 0;

    //! Sets UV transform
    virtual void SetUVTransform(const glm::vec2& offset, const glm::vec2& scale) = 0;

    //! Sets triplanar mode
    virtual void SetTriplanarMode(bool enabled) = 0;
    //! Sets triplanar scale
    virtual void SetTriplanarScale(float scale) = 0;

    //! Draws an object
    virtual void DrawObject(const CVertexBuffer* buffer) = 0;
    //! Draws a primitive
    virtual void DrawPrimitive(PrimitiveType type, int count, const Vertex3D* vertices) = 0;
    //! Draws a set of primitives
    virtual void DrawPrimitives(PrimitiveType type, int drawCount, int count[], const Vertex3D* vertices) = 0;
};

/**
 * \class CParticleRenderer
 * \brief Abstract interface for particle renderers
 */
class CParticleRenderer : public CRenderer
{
public:
    virtual ~CParticleRenderer() { }

    virtual void Begin() = 0;

    virtual void End() = 0;

    //! Sets projection matrix
    virtual void SetProjectionMatrix(const glm::mat4& matrix) = 0;
    //! Sets view matrix
    virtual void SetViewMatrix(const glm::mat4& matrix) = 0;
    //! Sets model matrix
    virtual void SetModelMatrix(const glm::mat4& matrix) = 0;

    //! Sets color
    virtual void SetColor(const glm::vec4& color) = 0;
    //! Sets texture
    virtual void SetTexture(const Texture& texture) = 0;

    //! Sets transparency mode
    virtual void SetTransparency(TransparencyMode mode) = 0;

    //! Draws particles
    virtual void DrawParticle(PrimitiveType type, int count, const Vertex3D* vertices) = 0;
};

/**
 * \class CShadowRenderer
 * \brief Abstract interface for shadow renderers
 */
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

    //! Sets shadow map
    virtual void SetShadowMap(const Texture& texture) = 0;
    //! Sets shadow region
    virtual void SetShadowRegion(const glm::vec2& offset, const glm::vec2& scale) = 0;

    //! Draws terrain object
    virtual void DrawObject(const CVertexBuffer* buffer, bool transparent) = 0;
};

} // namespace Gfx
