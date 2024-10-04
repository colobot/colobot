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

 /**
  * \file graphics/opengl33/gl33_ui_renderer.h
  * \brief OpenGL 3.3 UI renderer
  */

#pragma once

#include "graphics/core/renderers.h"

#include <GL/glew.h>

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
    virtual void SetTransparency(TransparencyMode mode) override;

    virtual Vertex2D* BeginPrimitive(PrimitiveType type, int count) override;
    virtual Vertex2D* BeginPrimitives(PrimitiveType type, int drawCount, const int* counts) override;
    virtual bool EndPrimitive() override;

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
    GLuint m_bufferCapacity = 128 * 1024;
    // Buffer offset
    GLsizei m_bufferOffset = 0;

    // Buffer mapping state
    PrimitiveType m_type = {};
    // Number of drawn primitives
    GLuint m_drawCount = 0;
    // Total count of drawn vertices
    GLuint m_currentCount = 0;
    // Starting offset for each drawn primitive
    std::vector<GLint> m_first;
    // Numbers of vertices for each drawn primitive
    std::vector<GLsizei> m_count;
    // True means currently drawing
    bool m_mapped = false;
    // True means mapping failed, using auxiliary buffer
    bool m_backup = false;

    // Buffered vertex data
    std::vector<Vertex2D> m_buffer;

    // Shader program
    GLuint m_program = 0;

    // 1x1 white texture
    GLuint m_whiteTexture = 0;
    // Currently bound texture
    GLuint m_currentTexture = 0;
};

}
