/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
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

#include "graphics/core/framebuffer.h"

#include "graphics/opengl/glutil.h"

namespace Gfx
{

/**
 * \class CGLFramebuffer
 * \brief Implementation of CFramebuffer interface in OpenGL 3.0+
 *
 * Provides the concrete implementation of core framebuffers.
 * Can be used in OpenGL 3.0+ and with ARB_framebuffer_object supported.
*/
class CGLFramebuffer : public CFramebuffer
{
protected:
    FramebufferParams m_params;

    int m_width, m_height, m_depth, m_samples;

    GLuint m_fbo;
    GLuint m_colorRenderbuffer;
    GLuint m_colorTexture;
    GLuint m_depthRenderbuffer;
    GLuint m_depthTexture;

    static GLuint m_currentFBO;

public:
    CGLFramebuffer(const FramebufferParams& params);

    bool Create() override;

    void Destroy() override;

    bool IsDefault() override;

    int GetWidth() override;

    int GetHeight() override;

    int GetDepth() override;

    int GetSamples() override;

    int GetColorTexture() override;

    int GetDepthTexture() override;

    void Bind() override;

    void Unbind() override;

    void CopyToScreen(int fromX, int fromY, int fromWidth, int fromHeight, int toX, int toY, int toWidth, int toHeight) override;
};

/**
* \class CGLFramebuffer
* \brief Implementation of CFramebuffer interface in legacy OpenGL
*
* Provides the concrete implementation of extension framebuffers.
* Can be used with EXT_framebuffer_object supported.
*/
class CGLFramebufferEXT : public CFramebuffer
{
protected:
    FramebufferParams m_params;

    int m_width, m_height, m_depth, m_samples;

    GLuint m_fbo;
    GLuint m_colorRenderbuffer;
    GLuint m_colorTexture;
    GLuint m_depthRenderbuffer;
    GLuint m_depthTexture;

    static GLuint m_currentFBO;

public:
    CGLFramebufferEXT(const FramebufferParams& params);

    bool Create() override;

    void Destroy() override;

    bool IsDefault() override;

    int GetWidth() override;

    int GetHeight() override;

    int GetDepth() override;

    int GetSamples() override;

    int GetColorTexture() override;

    int GetDepthTexture() override;

    void Bind() override;

    void Unbind() override;

    void CopyToScreen(int fromX, int fromY, int fromWidth, int fromHeight, int toX, int toY, int toWidth, int toHeight) override;
};

} // end of Gfx
