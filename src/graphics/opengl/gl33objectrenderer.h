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
  * \file graphics/opengl/gl33objectrenderer.h
  * \brief OpenGL 3.3 object renderer
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

class CGL33ObjectRenderer : public CObjectRenderer
{
public:
    CGL33ObjectRenderer(CGL33Device* device);
    virtual ~CGL33ObjectRenderer();

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

    //! Sets primary texture
    virtual void SetPrimaryTexture(const Texture& texture) override;
    //! Sets secondary texture
    virtual void SetSecondaryTexture(const Texture& texture) override;
    //! Sets shadow map
    virtual void SetShadowMap(const Texture& texture) override;

    //! Enables lighting
    virtual void SetLighting(bool enabled) override;
    //! Sets light parameters
    virtual void SetLight(const glm::vec4& position, const float& intensity, const glm::vec3& color) override;
    //! Sets shadow parameters
    virtual void SetShadowParams(int count, const ShadowParam* params) override;

    //! Sets fog parameters
    virtual void SetFog(float min, float max, const glm::vec3& color) override;
    //! Sets alpha scissor
    virtual void SetAlphaScissor(float alpha) override;

    //! Sets cull mode parameters
    virtual void SetCullMode(bool enabled) override;
    //! Sets transparency mode
    virtual void SetTransparency(TransparencyMode mode) override;
    virtual void SetPrimaryTextureEnabled(bool enabled) override;
    //! Sets amount of dirt (second texture) to apply
    virtual void SetDirty(float amount) override;

    //! Draws an object
    virtual void DrawObject(const CVertexBuffer* buffer) override;

private:
    CGL33Device* const m_device;

    // Uniform data
    GLint m_projectionMatrix = -1;
    GLint m_viewMatrix = -1;
    GLint m_cameraMatrix = -1;
    GLint m_shadowMatrix = -1;
    GLint m_modelMatrix = -1;
    GLint m_normalMatrix = -1;
    GLint m_lighting = -1;
    GLint m_lightPosition = -1;
    GLint m_lightIntensity = -1;
    GLint m_lightColor = -1;
    GLint m_fogRange = -1;
    GLint m_fogColor = -1;
    GLint m_color = -1;
    GLint m_primaryEnabled = -1;
    GLint m_dirty = -1;
    GLint m_alphaScissor = -1;

    struct ShadowUniforms
    {
        GLint transform;
        GLint offset;
        GLint scale;
    };

    GLint m_shadowRegions;
    ShadowUniforms m_shadows[4];

    // Shader program
    GLuint m_program = 0;

    // 1x1 white texture
    GLuint m_whiteTexture = 0;
    // Currently bound primary texture
    GLuint m_primaryTexture = 0;
    // Currently bound secondary texture
    GLuint m_secondaryTexture = 0;
    // Currently bound shadow map
    GLuint m_shadowMap = 0;
};

}
