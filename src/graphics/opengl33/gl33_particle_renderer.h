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
  * \file graphics/opengl33/gl33_particle_renderer.h
  * \brief OpenGL 3.3 particle renderer
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

class CGL33ParticleRenderer : public CParticleRenderer
{
public:
    CGL33ParticleRenderer(CGL33Device* device);
    virtual ~CGL33ParticleRenderer();

    virtual void Begin() override;

    virtual void End() override;

    //! Sets projection matrix
    virtual void SetProjectionMatrix(const glm::mat4& matrix) override;
    //! Sets view matrix
    virtual void SetViewMatrix(const glm::mat4& matrix) override;
    //! Sets model matrix
    virtual void SetModelMatrix(const glm::mat4& matrix) override;

    //! Sets color
    virtual void SetColor(const glm::vec4& color) override;
    //! Sets texture
    virtual void SetTexture(const Texture& texture) override;

    //! Sets transparency mode
    virtual void SetTransparency(TransparencyMode mode) override;

    //! Draws particles
    virtual void DrawParticle(PrimitiveType type, int count, const Vertex3D* vertices) override;

private:
    CGL33Device* const m_device;

    // Uniform data
    GLint m_projectionMatrix = -1;
    GLint m_viewMatrix = -1;
    GLint m_modelMatrix = -1;
    GLint m_fogRange = -1;
    GLint m_fogColor = -1;
    GLint m_color = -1;
    GLint m_alphaScissor = -1;

    // Shader program
    GLuint m_program = 0;

    // 1x1 white texture
    GLuint m_whiteTexture = 0;
    // Currently bound primary texture
    GLuint m_texture = 0;

    // Vertex buffer object
    GLuint m_bufferVBO = 0;
    // Vertex array object
    GLuint m_bufferVAO = 0;
    // VBO capacity
    GLsizei m_bufferCapacity = 8 * sizeof(Vertex3D);
};

}
