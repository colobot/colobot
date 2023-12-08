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
  * \file graphics/opengl33/gl33_shadow_renderer.h
  * \brief OpenGL 3.3 shadow renderer
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

class CGL33ShadowRenderer : public CShadowRenderer
{
public:
    CGL33ShadowRenderer(CGL33Device* device);
    virtual ~CGL33ShadowRenderer();

    virtual void Begin() override;

    virtual void End() override;

    //! Sets projection matrix
    virtual void SetProjectionMatrix(const glm::mat4& matrix) override;
    //! Sets view matrix
    virtual void SetViewMatrix(const glm::mat4& matrix) override;
    //! Sets model matrix
    virtual void SetModelMatrix(const glm::mat4& matrix) override;

    //! Sets texture
    virtual void SetTexture(const Texture& texture) override;

    //! Sets shadow map
    virtual void SetShadowMap(const Texture& texture) override;
    //! Sets shadow region
    virtual void SetShadowRegion(const glm::vec2& offset, const glm::vec2& scale) override;

    //! Draws terrain object
    virtual void DrawObject(const CVertexBuffer* buffer, bool transparent) override;

private:
    CGL33Device* const m_device;

    // Uniform data
    GLint m_projectionMatrix = -1;
    GLint m_viewMatrix = -1;
    GLint m_modelMatrix = -1;
    GLint m_alphaScissor = -1;

    // Shader program
    GLuint m_program = 0;

    // Framebuffer 
    GLuint m_framebuffer = 0;
    int m_width = 0;
    int m_height = 0;
};

} // namespace Gfx
