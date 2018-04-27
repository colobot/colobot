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

#include "graphics/opengl/glframebuffer.h"

#include "common/logger.h"

namespace Gfx
{

// CGLFramebuffer

GLuint CGLFramebuffer::m_currentFBO = 0;

CGLFramebuffer::CGLFramebuffer(const FramebufferParams& params)
    : m_params(params)
{
    m_fbo = 0;
    m_colorRenderbuffer = 0;
    m_colorTexture = 0;
    m_depthRenderbuffer = 0;
    m_depthTexture = 0;
    m_width = 0;
    m_height = 0;
    m_depth = 0;
    m_samples = 0;
}

bool CGLFramebuffer::Create()
{
    if (m_fbo != 0) return false;

    m_width = m_params.width;
    m_height = m_params.height;
    m_depth = m_params.depth;
    m_samples = m_params.samples;

    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // create color texture
    if (m_params.colorTexture)
    {
        GLint previous;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &previous);

        glGenTextures(1, &m_colorTexture);
        glBindTexture(GL_TEXTURE_2D, m_colorTexture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_params.width, m_params.height,
                0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glBindTexture(GL_TEXTURE_2D, previous);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture, 0);
    }
    // create color renderbuffer
    else
    {
        glGenRenderbuffers(1, &m_colorRenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, m_colorRenderbuffer);

        if (m_params.samples > 1)
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_params.samples,
                    GL_RGBA8, m_params.width, m_params.height);
        else
            glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, m_params.width, m_params.height);

        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_colorRenderbuffer);
    }

    GLuint depthFormat = 0;

    switch (m_params.depth)
    {
    case 16: depthFormat = GL_DEPTH_COMPONENT16; break;
    case 24: depthFormat = GL_DEPTH_COMPONENT24; break;
    case 32: depthFormat = GL_DEPTH_COMPONENT32; break;
    default: depthFormat = GL_DEPTH_COMPONENT16; break;
    }

    // create depth texture
    if (m_params.depthTexture)
    {
        GLint previous;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &previous);

        glGenTextures(1, &m_depthTexture);
        glBindTexture(GL_TEXTURE_2D, m_depthTexture);

        glTexImage2D(GL_TEXTURE_2D, 0, depthFormat, m_params.width, m_params.height, 0,
                GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

        float color[] = { 1.0f, 1.0f, 1.0f, 1.0f };

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

        glBindTexture(GL_TEXTURE_2D, previous);

        glFramebufferTexture2D(GL_FRAMEBUFFER,
                GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);
    }
    // create depth renderbuffer
    else
    {
        glGenRenderbuffers(1, &m_depthRenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderbuffer);

        if (m_params.samples > 1)
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_params.samples,
                    depthFormat, m_params.width, m_params.height);
        else
            glRenderbufferStorage(GL_RENDERBUFFER,
                    depthFormat, m_params.width, m_params.height);

        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderbuffer);
    }

    GLuint result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (result != GL_FRAMEBUFFER_COMPLETE)
    {
        GetLogger()->Error("Framebuffer incomplete: ");

        switch (result)
        {
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            GetLogger()->Error("attachment point incomplete");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            GetLogger()->Error("missing attachment");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            GetLogger()->Error("draw buffer has missing color attachments");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            GetLogger()->Error("read buffer has missing color attachments");
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            GetLogger()->Error("unsupported attachment format");
            break;
        }

        GetLogger()->Error("\n");

        Destroy();

        glBindFramebuffer(GL_FRAMEBUFFER, m_currentFBO);
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, m_currentFBO);
    return true;
}

void CGLFramebuffer::Destroy()
{
    if (m_fbo == 0) return;

    if (m_currentFBO == m_fbo)
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDeleteFramebuffers(1, &m_fbo);
    m_fbo = 0;

    if (m_colorRenderbuffer != 0)
    {
        glDeleteRenderbuffers(1, &m_colorRenderbuffer);
        m_colorRenderbuffer = 0;
    }

    if (m_colorTexture != 0)
    {
        glDeleteTextures(1, &m_colorTexture);
        m_colorTexture = 0;
    }

    if (m_depthRenderbuffer != 0)
    {
        glDeleteRenderbuffers(1, &m_depthRenderbuffer);
        m_depthRenderbuffer = 0;
    }

    if (m_depthTexture != 0)
    {
        glDeleteTextures(1, &m_depthTexture);
        m_depthTexture = 0;
    }

    m_width = 0;
    m_height = 0;
    m_depth = 0;
    m_samples = 0;
}

bool CGLFramebuffer::IsDefault()
{
    return false;
}

//! Returns width of buffers in this framebuffer
int CGLFramebuffer::GetWidth()
{
    return m_width;
}

//! Returns height of buffers in this framebuffer
int CGLFramebuffer::GetHeight()
{
    return m_height;
}

//! Returns depth size in bits
int CGLFramebuffer::GetDepth()
{
    return m_depth;
}

//! Returns number of samples or 1 if multisampling is not supported
int CGLFramebuffer::GetSamples()
{
    return m_samples;
}

//! Returns texture that contains color buffer or 0 if not available
int CGLFramebuffer::GetColorTexture()
{
    return m_colorTexture;
}

//! Returns texture that contains depth buffer or 0 if not available
int CGLFramebuffer::GetDepthTexture()
{
    return m_depthTexture;
}

//! Binds this framebuffer to context
void CGLFramebuffer::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    m_currentFBO = m_fbo;
}

//! Unbinds this framebuffer from context
void CGLFramebuffer::Unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_currentFBO = 0;
}

void CGLFramebuffer::CopyToScreen(int fromX, int fromY, int fromWidth, int fromHeight,
        int toX, int toY, int toWidth, int toHeight)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    glBlitFramebuffer(fromX, fromY, fromX + fromWidth, fromY + fromHeight,
        toX, toY, toX + toWidth, toY + toHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, m_currentFBO);
}

// CGLFramebufferEXT
GLuint CGLFramebufferEXT::m_currentFBO = 0;

CGLFramebufferEXT::CGLFramebufferEXT(const FramebufferParams& params)
    : m_params(params)
{
    m_fbo = 0;
    m_colorRenderbuffer = 0;
    m_colorTexture = 0;
    m_depthRenderbuffer = 0;
    m_depthTexture = 0;
    m_width = 0;
    m_height = 0;
    m_depth = 0;
    m_samples = 0;
}

bool CGLFramebufferEXT::Create()
{
    if (m_fbo != 0) return false;

    m_width = m_params.width;
    m_height = m_params.height;
    m_depth = m_params.depth;
    m_samples = m_params.samples;

    glGenFramebuffersEXT(1, &m_fbo);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);

    // create color texture
    if (m_params.colorTexture)
    {
        GLint previous;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &previous);

        glGenTextures(1, &m_colorTexture);
        glBindTexture(GL_TEXTURE_2D, m_colorTexture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                m_params.width, m_params.height, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glBindTexture(GL_TEXTURE_2D, previous);

        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_colorTexture, 0);
    }
    // create color renderbuffer
    else
    {
        glGenRenderbuffersEXT(1, &m_colorRenderbuffer);
        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_colorRenderbuffer);

        if (m_params.samples > 1)
            glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT,
                    m_params.samples, GL_RGBA8, m_params.width, m_params.height);
        else
            glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGBA8,
                    m_params.width, m_params.height);

        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
                GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, m_colorRenderbuffer);
    }

    GLuint depthFormat = 0;

    switch (m_params.depth)
    {
    case 16: depthFormat = GL_DEPTH_COMPONENT16; break;
    case 24: depthFormat = GL_DEPTH_COMPONENT24; break;
    case 32: depthFormat = GL_DEPTH_COMPONENT32; break;
    default: depthFormat = GL_DEPTH_COMPONENT16; break;
    }

    // create depth texture
    if (m_params.depthTexture)
    {
        GLint previous;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &previous);

        glGenTextures(1, &m_depthTexture);
        glBindTexture(GL_TEXTURE_2D, m_depthTexture);

        glTexImage2D(GL_TEXTURE_2D, 0, depthFormat, m_params.width, m_params.height, 0,
                GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

        float color[] = { 1.0f, 1.0f, 1.0f, 1.0f };

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

        glBindTexture(GL_TEXTURE_2D, previous);

        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, m_depthTexture, 0);
    }
    // create depth renderbuffer
    else
    {
        glGenRenderbuffersEXT(1, &m_depthRenderbuffer);
        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_depthRenderbuffer);

        if (m_params.samples > 1)
            glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT,
                    m_params.samples, depthFormat, m_params.width, m_params.height);
        else
            glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, depthFormat, m_params.width, m_params.height);

        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
                GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, m_depthRenderbuffer);
    }

    GLuint result = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if (result != GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        GetLogger()->Error("Framebuffer incomplete: ");

        switch (result)
        {
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
            GetLogger()->Error("attachment point incomplete");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
            GetLogger()->Error("missing attachment");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
            GetLogger()->Error("incompatible attachment dimensions");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
            GetLogger()->Error("draw buffer has missing color attachments");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
            GetLogger()->Error("read buffer has missing color attachments");
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
            GetLogger()->Error("unsupported attachment format");
            break;
        }

        Destroy();

        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_currentFBO);
        return false;
    }

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_currentFBO);
    return true;
}

void CGLFramebufferEXT::Destroy()
{
    if (m_fbo == 0) return;

    if (m_currentFBO == m_fbo)
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    glDeleteFramebuffersEXT(1, &m_fbo);
    m_fbo = 0;

    if (m_colorRenderbuffer != 0)
    {
        glDeleteRenderbuffersEXT(1, &m_colorRenderbuffer);
        m_colorRenderbuffer = 0;
    }

    if (m_colorTexture != 0)
    {
        glDeleteTextures(1, &m_colorTexture);
        m_colorTexture = 0;
    }

    if (m_depthRenderbuffer != 0)
    {
        glDeleteRenderbuffersEXT(1, &m_depthRenderbuffer);
        m_depthRenderbuffer = 0;
    }

    if (m_depthTexture != 0)
    {
        glDeleteTextures(1, &m_depthTexture);
        m_depthTexture = 0;
    }

    m_width = 0;
    m_height = 0;
    m_depth = 0;
    m_samples = 0;
}

bool CGLFramebufferEXT::IsDefault()
{
    return false;
}

//! Returns width of buffers in this framebuffer
int CGLFramebufferEXT::GetWidth()
{
    return m_width;
}

//! Returns height of buffers in this framebuffer
int CGLFramebufferEXT::GetHeight()
{
    return m_height;
}

//! Returns depth size in bits
int CGLFramebufferEXT::GetDepth()
{
    return m_depth;
}

//! Returns number of samples or 1 if multisampling is not supported
int CGLFramebufferEXT::GetSamples()
{
    return m_samples;
}

//! Returns texture that contains color buffer or 0 if not available
int CGLFramebufferEXT::GetColorTexture()
{
    return m_colorTexture;
}

//! Returns texture that contains depth buffer or 0 if not available
int CGLFramebufferEXT::GetDepthTexture()
{
    return m_depthTexture;
}

//! Binds this framebuffer to context
void CGLFramebufferEXT::Bind()
{
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
    m_currentFBO = m_fbo;
}

//! Unbinds this framebuffer from context
void CGLFramebufferEXT::Unbind()
{
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    m_currentFBO = 0;
}

void CGLFramebufferEXT::CopyToScreen(int fromX, int fromY, int fromWidth, int fromHeight,
        int toX, int toY, int toWidth, int toHeight)
{
    glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, m_fbo);
    glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, 0);

    glBlitFramebufferEXT(fromX, fromY, fromX + fromWidth, fromY + fromHeight,
            toX, toY, toX + toWidth, toY + toHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_currentFBO);
}

} // end of Gfx
