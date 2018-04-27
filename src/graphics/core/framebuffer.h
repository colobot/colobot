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

/**
* \file graphics/core/framebuffer.h
* \brief Abstract representation of framebuffer and offscreen buffers
*/

#pragma once

namespace Gfx
{

/**
 * \struct FramebufferParams
 * \brief Contains parameters for new framebuffer
 */
struct FramebufferParams
{
    //! Requested width of buffers
    int width = 1024;
    //! Requested height of buffers
    int height = 1024;
    //! Requested depth buffer
    int depth = 16;
    //! Requested number of samples for multisampling
    int samples = 1;
    //! true requests color texture
    bool colorTexture = false;
    //! true requests depth texture
    bool depthTexture = false;

    //! Loads default values
    void LoadDefault()
    {
        *this = FramebufferParams();
    }
};

/**
 * \class CFramebuffer
 * \brief Abstract interface of default framebuffer and offscreen framebuffers
 *
 * This code encapsulates basics of default framebuffer and offscreen buffers
 * and allows offscreen rendering in generic way. CDevice may or may not implement
 * offscreen buffers depending on available hardware but is required to provide
 * default framebuffer implementation. Because of some hardware restrictions
 * and in order to simplify interface, you can't bind/unbind textures from
 * offscreen buffers and you can't change it's parameters.
 */
class CFramebuffer
{
public:
    virtual ~CFramebuffer() {}

    //! Creates this framebuffer
    virtual bool Create() = 0;

    //! Destroys this framebuffer
    virtual void Destroy() = 0;

    //! Returns true if this is default framebuffer
    virtual bool IsDefault() = 0;

    //! Returns width of buffers in this framebuffer
    virtual int GetWidth() = 0;

    //! Returns height of buffers in this framebuffer
    virtual int GetHeight() = 0;

    //! Returns depth size in bits
    virtual int GetDepth() = 0;

    //! Returns number of samples or 1 if multisampling is not supported
    virtual int GetSamples() = 0;

    //! Returns texture that contains color buffer or 0 if not available
    virtual int GetColorTexture() = 0;

    //! Returns texture that contains depth buffer or 0 if not available
    virtual int GetDepthTexture() = 0;

    //! Binds this framebuffer to context
    virtual void Bind() = 0;

    //! Unbinds this framebuffer from context
    virtual void Unbind() = 0;

    //! Copies content of color buffer to screen
    virtual void CopyToScreen(int fromX, int fromY, int fromWidth, int fromHeight, int toX, int toY, int toWidth, int toHeight) = 0;
};


/**
* \class CDefaultFramebuffer
* \brief Concrete implementation of default framebuffer.
*
* This class represents default framebuffer implementation.
*/
class CDefaultFramebuffer : public CFramebuffer
{
private:
    int m_width, m_height, m_depth;

public:
    explicit CDefaultFramebuffer(const FramebufferParams &params);

    //! Creates default framebuffer
    bool Create() override;

    //! Destroys default framebuffer
    void Destroy() override;

    //! Returns true
    bool IsDefault() override;

    //! Returns width of buffers in this framebuffer
    int GetWidth() override;

    //! Returns height of buffers in this framebuffer
    int GetHeight() override;

    //! Returns depth size in bits
    int GetDepth() override;

    //! Returns number of samples or 1 if multisampling is not supported
    int GetSamples() override;

    //! Returns texture that contains color buffer or 0 if not available
    int GetColorTexture() override;

    //! Returns texture that contains depth buffer or 0 if not available
    int GetDepthTexture() override;

    //! Binds this framebuffer to context
    void Bind() override;

    //! Unbinds this framebuffer from context
    void Unbind() override;

    //! Copies content of color buffer to screen
    void CopyToScreen(int fromX, int fromY, int fromWidth, int fromHeight, int toX, int toY, int toWidth, int toHeight) override;
};

} // end of Gfx
