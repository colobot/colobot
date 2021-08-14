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
  * \file graphics/opengl/gl33renderers.h
  * \brief OpenGL 3.3 renderers
  */

#pragma once

#include "graphics/core/renderers.h"

#include <gl/glew.h>

#include <array>
#include <vector>

// Graphics module namespace
namespace Gfx
{

class CGL33Device;

class CGL33UIRenderer : public CUIRenderer
{
public:
    CGL33UIRenderer(CGL33Device* device);
    virtual ~CGL33UIRenderer();

    virtual void SetProjection(float left, float right, float bottom, float top) override;
    virtual void SetTexture(const Texture& texture) override;
    virtual void SetColor(const glm::vec4& color) override;

    virtual void DrawPrimitive(PrimitiveType type, int count, const Vertex2D* vertices) override;

    virtual void Flush() override;

private:
    void UpdateUniforms();

    CGL33Device* const m_device;

    // Uniform data
    struct Uniforms
    {
        glm::mat4 projectionMatrix;
        glm::vec4 color;
    };
    Uniforms m_uniforms = {};

    // true means uniforms need to be updated
    bool m_uniformsDirty = false;

    // Uniform buffer object
    GLuint m_uniformBuffer = 0;

    // Vertex buffer object
    GLuint m_bufferVBO = 0;
    // Vertex array object
    GLuint m_bufferVAO = 0;
    // VBO capacity
    GLsizei m_bufferCapacity = 4 * 1024;

    // Buffered vertex data
    std::vector<Vertex2D> m_buffer;
    std::vector<GLenum> m_types;
    std::vector<GLint> m_firsts;
    std::vector<GLsizei> m_counts;

    // Shader program
    GLuint m_program = 0;

    // 1x1 white texture
    GLuint m_whiteTexture = 0;
    // Currently bound texture
    GLuint m_currentTexture = 0;
};

class CGL33TerrainRenderer : public CTerrainRenderer
{
public:
    CGL33TerrainRenderer(CGL33Device* device);
    virtual ~CGL33TerrainRenderer();

    virtual void Begin() override;

    virtual void End() override;

    //! Sets projection matrix
    virtual void SetProjectionMatrix(const glm::mat4& matrix) override;
    //! Sets view matrix
    virtual void SetViewMatrix(const glm::mat4& matrix) override;
    //! Sets model matrix
    virtual void SetModelMatrix(const glm::mat4& matrix) override;

    //! Sets primary texture, setting texture 0 means using white texture
    virtual void SetPrimaryTexture(const Texture& texture) override;
    //! Sets secondary texture
    virtual void SetSecondaryTexture(const Texture& texture) override;

    //! Draws terrain object
    virtual void DrawObject(const glm::mat4& matrix, const CVertexBuffer* buffer) override;

    virtual void Flush() override;

private:
    CGL33Device* const m_device;

    // Uniform data
    struct Uniforms
    {
        glm::mat4 projectionMatrix;
        glm::mat4 viewMatrix;
        glm::mat4 modelMatrix;
    };

    Uniforms m_uniforms = {};

    // true means uniforms need to be updated
    bool m_uniformsDirty = false;

    GLuint m_uniformBuffer = 0;

    // Shader program
    GLuint m_program = 0;

    // 1x1 white texture
    GLuint m_whiteTexture = 0;
    // Currently bound primary texture
    GLuint m_primaryTexture = 0;
    // Currently bound secondary texture
    GLuint m_secondaryTexture = 0;
};

} // namespace Gfx
